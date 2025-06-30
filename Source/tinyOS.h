#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>
#include "tConfig.h"
#include "tLib.h"
//��ʱ״̬
#define TINYOS_TASK_STATE_RDY         0
#define TINYOS_TASK_STATE__DELAYED   (1 << 1)//��������ʱ״̬

//cortex-m3��ջ��Ԫ���ͣ���ջ��Ԫ��СΪ32λ
typedef uint32_t tTaskStack;

typedef struct tTask
{
    tTaskStack* stack;

    uint32_t delayTicks;//������ʱ����

    tNode linkNode;//�������ӽ��

    tNode delayNode;//��ʱ���

    uint32_t prio;//��������ȼ�

    uint32_t state;//����ǰ״̬

    uint32_t slice;//��ǰʣ��ʱ��Ƭ
}tTask;

extern tTask* currentTask;

extern tTask* nextTask;

void tTaskSwitch(void);

void tTaskRunFirst(void);

void tTaskSchedInit(void);

void tTaskSchedDisable(void);

void tTaskSchedEnable(void);

uint32_t tTaskEnterCritical(void);

void tTaskExitCritical(uint32_t status);

void tTaskSched(void);

void tTaskSchedRdy(tTask* task);

void tTaskSchedUnRdy(tTask* task);

void tTaskSystemTickHandler();

void tTaskDelay(uint32_t delay);

void tTimeTaskWait(tTask* task, uint32_t ticks);

void tTimeTaskWakeUp(tTask* task);

void tTaskInit(tTask* task, void (*entry)(void*), void* param, uint32_t prio, uint32_t* stack);

void tSetSysTickPeriod(uint32_t ms);


#endif
