#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>
//cortex-m3��ջ��Ԫ���ͣ���ջ��Ԫ��СΪ32λ
typedef uint32_t tTaskStack;

typedef struct tTask
{
    tTaskStack* stack;

    uint32_t delayTicks;//������ʱ����
}tTask;

extern tTask* currentTask;

extern tTask* nextTask;

void tTaskSwitch(void);

void tTaskRunFirst(void);

#endif
