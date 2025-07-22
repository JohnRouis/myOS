#ifndef TMUTEX_H
#define TMUTEX_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tMutex
{
    tEvent event;//事件控制块

    uint32_t lockedCount;//已被锁定的次数

    tTask* owner;//拥有者

    uint32_t ownerOriginalPrio;//拥有者的原始优先级
}tMutex;

typedef struct _tMutexInfo
{
    uint32_t taskCount;//等待任务数量

    uint32_t ownerPrio;//拥有者的优先级

    uint32_t inheritedPrio;//继承优先级

    tTask* owner;//当前信号量拥有者

    uint32_t lockedCount;//锁定次数
}tMutexInfo;


void tMutexInit(tMutex* mutex);

uint32_t tMutexWait(tMutex* mutex, uint32_t waitTicks);

uint32_t tMutexNoWaitGet(tMutex* mutex);

uint32_t tMutexNotify(tMutex* mutex);

uint32_t tMutexDestroy(tMutex* mutex);

void tMutexGetInfo(tMutex* mutex, tMutexInfo* info);

#endif
