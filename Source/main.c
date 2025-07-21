#include "tinyOS.h"
#include "ARMCM3.h"

tTask* currentTask;

tTask* nextTask;

tTask* idleTask;

tList taskTable[TINYOS_PRO_COUNT];//就绪队列

uint8_t schedLockCount;//调度锁计数器

tBitmap taskPrioBitmap;//任务优先级标志位置结构

tList tTaskDelayedList;//延时队列


tTask* tTaskHighestReady(void)
{
	uint32_t  highestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
	tNode* node = tListFirst(&taskTable[highestPrio]);//从就绪队列中找到最高优先级任务
	return (tTask*)tNodeParent(node, tTask, linkNode);
}

void tTaskSchedInit(void)
{
	schedLockCount = 0;
	int i;
	for ( i = 0; i < TINYOS_PRO_COUNT; i++)//初始化就绪队列链表
	{
		tListInit(&taskTable[i]);
	}
	
}
//禁止任务调度 上锁
void tTaskSchedDisable(void)
{
	uint32_t status = tTaskEnterCritical();//访问公共变量需要进入临界区
	if(schedLockCount < 255)
	{
		schedLockCount++;
	}
	tTaskExitCritical(status);
}
//允许任务调度 解锁
void tTaskSchedEnable(void)
{
	uint32_t status = tTaskEnterCritical();
	if(schedLockCount > 0)
	{
		if(--schedLockCount == 0)
		{
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
}

/* 将任务设置为就绪态 */
void tTaskSchedRdy(tTask* task)
{
	tListAddLast(&taskTable[task->prio], &(task->linkNode));
	tBitmapSet(&taskPrioBitmap, task->prio);
}

/* 将任务从就绪列表中移除 */
void tTaskSchedUnRdy(tTask*  task)
{
	tListRemove(&taskTable[task->prio], &(task->linkNode));
	if(tListCount(&taskTable[task->prio]) == 0)//队列中可能存在多个任务,只有没有任务时才清除位图标记
	{
		tBitmapClear(&taskPrioBitmap, task->prio);
	}
}

void tTaskSchedRemove(tTask* task)
{
	tListRemove(&taskTable[task->prio], &(task->linkNode));
	if(tListCount(&taskTable[task->prio]) == 0)
	{
		tBitmapClear(&taskPrioBitmap, task->prio);
	}
}

void tTaskSched(void)
{   
	tTask* tempTask;//临时变量
	uint32_t status = tTaskEnterCritical();

	if(schedLockCount > 0)//调度器已上锁，不进行调度
	{
		tTaskExitCritical(status);
		return;
	}
	//找到最高优先级的任务，优先级比当前的还高，切换到这个任务
	tempTask = tTaskHighestReady();
	if(tempTask != currentTask)
	{
		nextTask = tempTask;
		tTaskSwitch();
	}

	tTaskExitCritical(status);
}

/* 初始化延时任务 */
void tTaskDelayedInit(void)
{
	tListInit(&tTaskDelayedList);
}
/* 将任务加入延时队列中 */
void tTimeTaskWait(tTask* task, uint32_t ticks)
{
	task->delayTicks = ticks;
	tListAddLast(&tTaskDelayedList, &(task->delayNode));//插入队列
	task->state |= TINYOS_TASK_STATE_DELAYED;//状态修改
}

/* 任务从延时队列中移除 */
void tTimeTaskWakeUp(tTask* task)
{
	tListRemove(&tTaskDelayedList, &(task->delayNode));
	task->state &= ~TINYOS_TASK_STATE_DELAYED;//清除标志位
}

void tTimeTaskRemove(tTask* task)
{
	tListRemove(&tTaskDelayedList, &(task->delayNode));
}

/* 时钟节拍处理 */
void tTaskSystemTickHandler()
{
	tNode* node;

	uint32_t status = tTaskEnterCritical();

	for(node = tTaskDelayedList.headNode.nextNode; node != &(tTaskDelayedList.headNode); node = node->nextNode)
	{
		tTask* task = tNodeParent(node, tTask, delayNode); //获取任务结构
		if(--task->delayTicks == 0)
		{
			if(task->waitEvent)//任务仍处于等待事件状态,从事件等待队列中唤醒
			{
				tEventRemoveTask(task, (void*)0, tErrorTimeout);
			}

			tTimeTaskWakeUp(task);//从队列中移除
			tTaskSchedRdy(task);//恢复到就绪状态
		}
	}

	if(--currentTask->slice == 0)
	{
		if(tListCount(&taskTable[currentTask->prio]) > 0)
		{
			tListRemoveFirst(&taskTable[currentTask->prio]);//首节点移除
			tListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));//把刚刚挪掉的结点放回到后面去
			currentTask->slice = TINYOS_SLICE_MAX;//重置计数器
		}
	}

	tTaskExitCritical(status);

	tTimerMoudleTickNotify();//定时器通知调用

	tTaskSched();
}

void idleTaskEntry(void* param)
{
	for(;;)
	{

	}
}

tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];

int main(void)
{
	tTaskSchedInit();//调度锁初始化

	tTaskDelayedInit();//初始化延时队列

	tTimerModuleInit();//定时器模块初始化

	tInitApp();

	tTaskInit(&tTaskIdle, idleTaskEntry, (void*)0, TINYOS_PRO_COUNT - 1, &idleTaskEnv[TINYOS_IDLETASK_STACK_SIZE]);

	nextTask = tTaskHighestReady();//找到最高优先级任务

	tTaskRunFirst();

	return 0;
}
