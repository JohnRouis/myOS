#include "tinyOS.h"
#include "ARMCM3.h"
tTask* currentTask;

tTask* nextTask;

tTask* idleTask;

tTask* taskTable[2];

uint8_t schedLockCount;//调度锁计数器

void tTaskInit(tTask* task, void (*entry)(void*), void* param, uint32_t* stack)
{
	*(--stack) = (unsigned long)(1 << 24);
	*(--stack) = (unsigned long)entry;
	*(--stack) = (unsigned long)0x14;
	*(--stack) = (unsigned long)0x12;
	*(--stack) = (unsigned long)0x3;
	*(--stack) = (unsigned long)0x2;
	*(--stack) = (unsigned long)0x1;
	*(--stack) = (unsigned long)param;
	*(--stack) = (unsigned long)0x11;
	*(--stack) = (unsigned long)0x10;
	*(--stack) = (unsigned long)0x9;
	*(--stack) = (unsigned long)0x8;
	*(--stack) = (unsigned long)0x7;
	*(--stack) = (unsigned long)0x6;
	*(--stack) = (unsigned long)0x5;
	*(--stack) = (unsigned long)0x4;

	task->stack = stack;//更新堆栈指针位置
	task->delayTicks = 0;
}

void tTaskSchedInit(void)
{
	schedLockCount = 0;
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
void tTaskSched(void)
{   //进入临界区，保护任务调度和切换期间，不会因为发生中断而导致更改
	uint32_t status = tTaskEnterCritical();

	if(currentTask == idleTask)//如果时空闲任务，执行其中一个任务即可
	{
		if(taskTable[0]->delayTicks == 0)
		{
			nextTask = taskTable[0];
		}
		else if(taskTable[1]->delayTicks == 0)
		{
			nextTask == taskTable[1];
		}
		else
		{
			tTaskExitCritical(status);
			return;
		}
	}
	else//不是空闲任务，检查下一个任务去
	{
		if(currentTask == taskTable[0])
		{
			if(taskTable[1]->delayTicks == 0)//下一个任务行，那就执行
			{
				nextTask = taskTable[1];
			}
			else if(currentTask->delayTicks != 0)//下一个任务还在延时而且
			{
				nextTask = idleTask;
			}
			else
			{
				tTaskExitCritical(status);
				return;
			}
		}
		else if(currentTask == taskTable[1])
		{
			if(taskTable[0]->delayTicks == 0)//下一个任务行，那就执行
			{
				nextTask = taskTable[0];
			}
			else if(currentTask->delayTicks != 0)//下一个任务还在延时而且
			{
				nextTask = idleTask;
			}
			else
			{
				tTaskExitCritical(status);
				return;
			}
		}
	}
	tTaskSwitch();
	//推出临界区
	tTaskExitCritical(status);
}

void tTaskSystemTickHandler()
{
	int i;
	uint32_t status = tTaskEnterCritical();
	for(i = 0; i < 2; i++)
	{
		if(taskTable[i]->delayTicks > 0)
		{
			taskTable[i]->delayTicks--;
		}
	}
	tTaskExitCritical(status);

	tTaskSched();
}

void tTaskDelay(uint32_t delay)
{
	uint32_t status = tTaskEnterCritical();

	currentTask->delayTicks = delay;

	tTaskExitCritical(status);
	
	tTaskSched();
}

void tSetSysTickPeriod(uint32_t ms)
{
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1;
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler()
{
	tTaskSystemTickHandler();
}

int task1Flag = 0;
void task1Entry(void* param)
{
	tSetSysTickPeriod(10);
	for(;;)
	{
		task1Flag = 1;
		tTaskDelay(1);
		task1Flag = 0;
		tTaskDelay(1);
	}
}

int task2Flag = 0;
void task2Entry(void* param)
{
	for(;;)
	{
		task2Flag = 1;
		tTaskDelay(1);
		task2Flag = 0;
		tTaskDelay(1);
	}
}

void idleTaskEntry(void* param)
{
	for(;;)
	{

	}
}

tTask tTask1;
tTask tTask2;
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];

tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];

int main(void)
{
	tTaskInit(&tTask1, task1Entry, (void*)0x11111111, &task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void*)0x22222222, &task2Env[1024]);
	tTaskInit(&tTaskIdle, idleTaskEntry, (void*)0, &idleTaskEnv[1024]);

	taskTable[0] = &tTask1;
	taskTable[1] = &tTask2;
	idleTask = &tTaskIdle;

	nextTask = taskTable[0];

	tTaskRunFirst();

	return 0;
}
