#include "tinyOS.h"
#include "ARMCM3.h"

tTask* currentTask;

tTask* nextTask;

tTask* idleTask;

tList taskTable[TINYOS_PRO_COUNT];//��������

uint8_t schedLockCount;//������������

uint32_t tickCount;//ʱ�ӽ��ļ���

tBitmap taskPrioBitmap;//�������ȼ���־λ�ýṹ

tList tTaskDelayedList;//��ʱ����

uint32_t idleCount;//����������� ����CPUʹ�ðٷֱȲ���

uint32_t idleMaxCount;//��������������ֵ

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1

static void initCpuUsageStat(void);
static void checkCpuUsage(void);
static void cpuUsageSyncWithSysTick(void);

#endif

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
#if TINYOS_ENABLE_HOOKS == 1
		tHooksTaskSwitch(currentTask, nextTask);
#endif
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

/*
** Description: ʱ�ӽ���ͳ������ʼ��
*/
void tTimeTickInit(void)
{
	tickCount = 0;
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

	tickCount++;

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	checkCpuUsage();//���CPUʹ����
#endif

	tTaskExitCritical(status);

#if TINYOS_ENABLE_TIMER == 1
	tTimerMoudleTickNotify();//��ʱ��֪ͨ����
#endif

#if TINYOS_ENABLE_HOOKS == 1
	tHooksSysTick();
#endif

	tTaskSched();
}

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1

static float cpuUsage;//CPUʹ����ͳ��
static uint32_t enableCpuUsageState;//�Ƿ�ʹ��CPUͳ�ƹ���

/*
** Description: ��ʼ��CPUͳ����ر���
*/
static void initCpuUsageStat(void)
{
	idleCount = 0;
	idleMaxCount = 0;
	cpuUsage = 0.0f;
	enableCpuUsageState = 0;
}

/*
** Description: ���CPUʹ����
*/
static void checkCpuUsage(void)
{
	if(enableCpuUsageState == 0)//����������cpuͳ��ͬ��
	{
		enableCpuUsageState = 1;
		tickCount = 0;
		return;
	}

	if(tickCount == TICKS_PER_SEC)
	{
		idleMaxCount = idleCount;
		idleCount = 0;

		tTaskSchedEnable();//������ϣ���ʼ�����������
	}
	else if(tickCount % TICKS_PER_SEC == 0)
	{
		cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
		idleCount = 0;
	}
}

/*
** Description: �����ȴ�ͬ��
*/
static void cpuUsageSyncWithSysTick(void)
{
	while (enableCpuUsageState == 0)
	{
		;;
	}
	
}

/*
** Description: ��ȡCPU������
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
	tTaskSchedDisable();//��ֹ���ȣ�Ϊ������������100tick��ʱ�����CPUռ��

	tInitApp();//Ӧ�������ʼ��

#if TINYOS_ENABLE_TIMER == 1
	tTimerInitTask();//�����ʱ����ʼ��
#endif

	tSetSysTickPeriod(TINYOS_SYSTICK_MS);//��ʼ��ʱ�ӽ���

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	cpuUsageSyncWithSysTick();//�ȴ���ʱ��ͬ��
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
	tTaskSchedInit();//��������ʼ��

	tTaskDelayedInit();//��ʼ����ʱ����

#if TINYOS_ENABLE_TIMER == 1
	tTimerModuleInit();//��ʱ��ģ���ʼ��
#endif

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	initCpuUsageStat();//��ʼ��CPUͳ��
#endif

	tTimeTickInit();//ʱ�ӽ��ĳ�ʼ��

	tTaskInit(&tTaskIdle, idleTaskEntry, (void*)0, TINYOS_PRO_COUNT - 1, idleTaskEnv, TINYOS_IDLETASK_STACK_SIZE);

	nextTask = tTaskHighestReady();//�ҵ�������ȼ�����

	tTaskRunFirst();

	return 0;
}
