#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>

#include "tConfig.h"

#include "tLib.h"

#include "tEvent.h"

#include "tTask.h"

#include "tSem.h"

#include "tMbox.h"

#include "tMemBlock.h"

#include "tFlagGroup.h"

#include "tMutex.h"

typedef enum _tError
{
    tErrorNoError = 0,
    tErrorTimeout,
    tErrorResourceUnavaliable,
    tErrorDel,
    tErrorResourceFull,
}tErroe;


extern tTask* currentTask;

extern tTask* nextTask;

void tTaskSwitch(void);

void tTaskRunFirst(void);

void tTaskSchedInit(void);

void tTaskSchedDisable(void);

void tTaskSchedEnable(void);

void tTaskSchedRemove(tTask* task);

uint32_t tTaskEnterCritical(void);

void tTaskExitCritical(uint32_t status);

void tTaskSched(void);

void tTaskSchedRdy(tTask* task);

void tTaskSchedUnRdy(tTask* task);

void tTaskSystemTickHandler();

void tTaskDelay(uint32_t delay);



void tTimeTaskWait(tTask* task, uint32_t ticks);

void tTimeTaskWakeUp(tTask* task);

void tTimeTaskRemove(tTask* task);



void tSetSysTickPeriod(uint32_t ms);

void tInitApp(void);



#endif
