#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>
#include "tConfig.h"
#include "tLib.h"

//cortex-m3��ջ��Ԫ���ͣ���ջ��Ԫ��СΪ32λ
typedef uint32_t tTaskStack;

typedef struct tTask
{
    tTaskStack* stack;

    uint32_t delayTicks;//������ʱ����

    uint32_t prio;//��������ȼ�
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

#endif
