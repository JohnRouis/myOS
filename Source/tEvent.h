#ifndef TEVENT_H
#define TEVENT_H

#include "tConfig.h"
#include "tLib.h"
#include "tTask.h"

/*事件类型*/
typedef enum _tEventType
{
    tEventTypeUnkonwn = (0 << 16),
    tEventTypeSem = (1 << 16),
    tEventTypeMbox = (2 << 16),
    tEventTypeMemBlock = (3 << 16),
    tEventTypeFlagGroup = (4 << 16),
    tEventTypeMutex = (5 << 16),
}tEventType;

typedef struct _tEvent
{
    tEventType type;
    tList waitList;
}tEvent;

void tEventInit(tEvent* event, tEventType type);

void tEventWait(tEvent* event, tTask* task, void* msg, uint32_t state, uint32_t timeout);

tTask* tEventWakeUp(tEvent* event, void* msg, uint32_t result);

void tEventWakeUpTask(tEvent* event, tTask* task, void* msg, uint32_t result);

void tEventRemoveTask(tTask* task, void* msg, uint32_t result);

uint32_t tEventRemoveAll(tEvent* event, void* msg, uint32_t result);

uint32_t tEventWaitCount(tEvent* event);

#endif
