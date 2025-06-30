#include "tinyOS.h"

tTask tTask1;
tTask tTask2;
tTask tTask3;
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];

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

void delay(void)
{
	for(int i = 0; i < 0xff; i++){}
}

int task2Flag = 0;
void task2Entry(void* param)
{
	for(;;)
	{
		task2Flag = 1;
		delay();
		task2Flag = 0;
		delay();
	}
}

int task3Flag = 0;
void task3Entry(void* param)
{
	for(;;)
	{
		task3Flag = 1;
		delay();
		task3Flag = 0;
		delay();
	}
}

void tInitApp(void)
{
    tTaskInit(&tTask1, task1Entry, (void*)0x11111111, 0, &task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void*)0x22222222, 1, &task2Env[1024]);
	tTaskInit(&tTask3, task3Entry, (void*)0x33333333, 1, &task3Env[1024]);
}