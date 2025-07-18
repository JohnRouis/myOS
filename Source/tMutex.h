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

void tMutexInit(tMutex* mutex);

#endif
