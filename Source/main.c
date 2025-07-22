#include "tinyOS.h"
#include "ARMCM3.h"

tTask* currentTask;

tTask* nextTask;

tTask* idleTask;

tList taskTable[TINYOS_PRO_COUNT];//就绪队列

uint8_t schedLockCount;//调度锁计数器

uint32_t tickCount;//时钟节拍计数

tBitmap taskPrioBitmap;//任务优先级标志位置结构

tList tTaskDelayedList;//延时队列

uint32_t idleCount;//空闲任务计数 用于CPU使用百分比测量

uint32_t idleMaxCount;//空闲任务最大计数值

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1

static void initCpuUsageStat(void);
static void checkCpuUsage(void);
static void cpuUsageSyncWithSysTick(void);

#endif

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
#if TINYOS_ENABLE_HOOKS == 1
		tHooksTaskSwitch(currentTask, nextTask);
#endif
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

/*
** Description: 时钟节拍统计数初始化
*/
void tTimeTickInit(void)
{
	tickCount = 0;
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

	tickCount++;

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	checkCpuUsage();//检查CPU使用率
#endif

	tTaskExitCritical(status);

#if TINYOS_ENABLE_TIMER == 1
	tTimerMoudleTickNotify();//定时器通知调用
#endif

#if TINYOS_ENABLE_HOOKS == 1
	tHooksSysTick();
#endif

	tTaskSched();
}

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1

static float cpuUsage;//CPU使用率统计
static uint32_t enableCpuUsageState;//是否使能CPU统计功能

/*
** Description: 初始化CPU统计相关变量
*/
static void initCpuUsageStat(void)
{
	idleCount = 0;
	idleMaxCount = 0;
	cpuUsage = 0.0f;
	enableCpuUsageState = 0;
}

/*
** Description: 检查CPU使用率
*/
static void checkCpuUsage(void)
{
	if(enableCpuUsageState == 0)//与空闲任务的cpu统计同步
	{
		enableCpuUsageState = 1;
		tickCount = 0;
		return;
	}

	if(tickCount == TICKS_PER_SEC)
	{
		idleMaxCount = idleCount;
		idleCount = 0;

		tTaskSchedEnable();//计数完毕，开始进行任务调度
	}
	else if(tickCount % TICKS_PER_SEC == 0)
	{
		cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
		idleCount = 0;
	}
}

/*
** Description: 阻塞等待同步
*/
static void cpuUsageSyncWithSysTick(void)
{
	while (enableCpuUsageState == 0)
	{
		;;
	}
	
}

/*
** Description: 获取CPU利用率
*/
float tCpuUsageGet(void)
{
	float usage = 0;

	uint32_t status = tTaskEnterCritical();
	usage = cpuUsage;
	tTaskExitCritical(status);

	return usage;
}
#endif

tTask tTaskIdle;
tTaskStack idleTaskEnv[TINYOS_IDLETASK_STACK_SIZE];

void idleTaskEntry(void* param)
{
	tTaskSchedDisable();//禁止调度，为空闲任务留下100tick的时间计算CPU占比

	tInitApp();//应用任务初始化

#if TINYOS_ENABLE_TIMER == 1
	tTimerInitTask();//软件定时器初始化
#endif

	tSetSysTickPeriod(TINYOS_SYSTICK_MS);//初始化时钟节拍

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	cpuUsageSyncWithSysTick();//等待与时钟同步
#endif

	for(;;)
	{
		uint32_t status = tTaskEnterCritical();

		idleCount++;

		tTaskExitCritical(status);
#if TINYOS_ENABLE_HOOKS == 1
		tHooksCpuIdle();
#endif
	}
}

int main(void)
{
	tTaskSchedInit();//调度锁初始化

	tTaskDelayedInit();//初始化延时队列

#if TINYOS_ENABLE_TIMER == 1
	tTimerModuleInit();//定时器模块初始化
#endif

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	initCpuUsageStat();//初始化CPU统计
#endif

	tTimeTickInit();//时钟节拍初始化

	tTaskInit(&tTaskIdle, idleTaskEntry, (void*)0, TINYOS_PRO_COUNT - 1, idleTaskEnv, TINYOS_IDLETASK_STACK_SIZE);

	nextTask = tTaskHighestReady();//找到最高优先级任务

	tTaskRunFirst();

	return 0;
}
