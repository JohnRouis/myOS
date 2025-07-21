#include "tinyOS.h"
#include "ARMCM3.h"

tTask* currentTask;

tTask* nextTask;

tTask* idleTask;

tList taskTable[TINYOS_PRO_COUNT];//��������

uint8_t schedLockCount;//������������

tBitmap taskPrioBitmap;//�������ȼ���־λ�ýṹ

tList tTaskDelayedList;//��ʱ����


tTask* tTaskHighestReady(void)
{
	uint32_t  highestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
	tNode* node = tListFirst(&taskTable[highestPrio]);//�Ӿ����������ҵ�������ȼ�����
	return (tTask*)tNodeParent(node, tTask, linkNode);
}

void tTaskSchedInit(void)
{
	schedLockCount = 0;
	int i;
	for ( i = 0; i < TINYOS_PRO_COUNT; i++)//��ʼ��������������
	{
		tListInit(&taskTable[i]);
	}
	
}
//��ֹ������� ����
void tTaskSchedDisable(void)
{
	uint32_t status = tTaskEnterCritical();//���ʹ���������Ҫ�����ٽ���
	if(schedLockCount < 255)
	{
		schedLockCount++;
	}
	tTaskExitCritical(status);
}
//����������� ����
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

/* ����������Ϊ����̬ */
void tTaskSchedRdy(tTask* task)
{
	tListAddLast(&taskTable[task->prio], &(task->linkNode));
	tBitmapSet(&taskPrioBitmap, task->prio);
}

/* ������Ӿ����б����Ƴ� */
void tTaskSchedUnRdy(tTask*  task)
{
	tListRemove(&taskTable[task->prio], &(task->linkNode));
	if(tListCount(&taskTable[task->prio]) == 0)//�����п��ܴ��ڶ������,ֻ��û������ʱ�����λͼ���
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
	tTask* tempTask;//��ʱ����
	uint32_t status = tTaskEnterCritical();

	if(schedLockCount > 0)//�������������������е���
	{
		tTaskExitCritical(status);
		return;
	}
	//�ҵ�������ȼ����������ȼ��ȵ�ǰ�Ļ��ߣ��л����������
	tempTask = tTaskHighestReady();
	if(tempTask != currentTask)
	{
		nextTask = tempTask;
		tTaskSwitch();
	}

	tTaskExitCritical(status);
}

/* ��ʼ����ʱ���� */
void tTaskDelayedInit(void)
{
	tListInit(&tTaskDelayedList);
}
/* �����������ʱ������ */
void tTimeTaskWait(tTask* task, uint32_t ticks)
{
	task->delayTicks = ticks;
	tListAddLast(&tTaskDelayedList, &(task->delayNode));//�������
	task->state |= TINYOS_TASK_STATE_DELAYED;//״̬�޸�
}

/* �������ʱ�������Ƴ� */
void tTimeTaskWakeUp(tTask* task)
{
	tListRemove(&tTaskDelayedList, &(task->delayNode));
	task->state &= ~TINYOS_TASK_STATE_DELAYED;//�����־λ
}

void tTimeTaskRemove(tTask* task)
{
	tListRemove(&tTaskDelayedList, &(task->delayNode));
}

/* ʱ�ӽ��Ĵ��� */
void tTaskSystemTickHandler()
{
	tNode* node;

	uint32_t status = tTaskEnterCritical();

	for(node = tTaskDelayedList.headNode.nextNode; node != &(tTaskDelayedList.headNode); node = node->nextNode)
	{
		tTask* task = tNodeParent(node, tTask, delayNode); //��ȡ����ṹ
		if(--task->delayTicks == 0)
		{
			if(task->waitEvent)//�����Դ��ڵȴ��¼�״̬,���¼��ȴ������л���
			{
				tEventRemoveTask(task, (void*)0, tErrorTimeout);
			}

			tTimeTaskWakeUp(task);//�Ӷ������Ƴ�
			tTaskSchedRdy(task);//�ָ�������״̬
		}
	}

	if(--currentTask->slice == 0)
	{
		if(tListCount(&taskTable[currentTask->prio]) > 0)
		{
			tListRemoveFirst(&taskTable[currentTask->prio]);//�׽ڵ��Ƴ�
			tListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));//�Ѹո�Ų���Ľ��Żص�����ȥ
			currentTask->slice = TINYOS_SLICE_MAX;//���ü�����
		}
	}

	tTaskExitCritical(status);

	tTimerMoudleTickNotify();//��ʱ��֪ͨ����

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
	tTaskSchedInit();//��������ʼ��

	tTaskDelayedInit();//��ʼ����ʱ����

	tTimerModuleInit();//��ʱ��ģ���ʼ��

	tInitApp();

	tTaskInit(&tTaskIdle, idleTaskEntry, (void*)0, TINYOS_PRO_COUNT - 1, &idleTaskEnv[TINYOS_IDLETASK_STACK_SIZE]);

	nextTask = tTaskHighestReady();//�ҵ�������ȼ�����

	tTaskRunFirst();

	return 0;
}
