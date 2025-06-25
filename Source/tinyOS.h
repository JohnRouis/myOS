#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>
//cortex-m3堆栈单元类型，堆栈单元大小为32位
typedef uint32_t tTaskStack;

typedef struct tTask
{
    tTaskStack* stack;
}tTask;

extern tTask* currentTask;

extern tTask* nextTask;

void tTaskSwitch(void);

void tTaskRunFirst(void);

#endif
