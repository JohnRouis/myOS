#include "tinyOS.h"
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "LED.h"

tTask tTask1;
tTask tTask2;
tTask tTask3;
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];

int task1Flag = 0;
void task1Entry(void* param)
{
//	RCC->APB2ENR = 0x00000010;
//	GPIOC->CRH = 0x00300000;
//	

	OLED_Init();
	
	
	for(;;)
	{
		OLED_ShowString(1, 1, "MYOS");
		tTaskDelay(5);
	}
}

int task2Flag = 0;
void task2Entry(void* param)
{
	
	LED13_init();
	for(;;)
	{
		LED13(0);
		tTaskDelay(5);
	}
}

//int task3Flag = 0;
//void task3Entry(void* param)
//{
//	for(;;)
//	{
//		task3Flag = 1;
//		tTaskDelay(1);
//		task3Flag = 0;
//		tTaskDelay(1);
//	}
//}

void tInitApp(void)
{
	tTaskInit(&tTask1, task1Entry, (void*)0x11111111, 0, task1Env, sizeof(task1Env));
	tTaskInit(&tTask2, task2Entry, (void*)0x22222222, 1, task2Env, sizeof(task2Env));
//	tTaskInit(&tTask3, task3Entry, (void*)0x33333333, 1, task3Env, sizeof(task3Env));
}
