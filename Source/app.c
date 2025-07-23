#include "tinyOS.h"
#include "stm32f10x.h"                  // Device header


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
//	GPIOC->ODR = 0x00002000;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
	//GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	for(;;)
	{
		
	}
}

//int task2Flag = 0;
//void task2Entry(void* param)
//{
//	for(;;)
//	{
//		task2Flag = 1;
//		tTaskDelay(1);
//		task2Flag = 0;
//		tTaskDelay(1);
//	}
//}

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
//	tTaskInit(&tTask2, task2Entry, (void*)0x22222222, 1, task2Env, sizeof(task2Env));
//	tTaskInit(&tTask3, task3Entry, (void*)0x33333333, 1, task3Env, sizeof(task3Env));
}
