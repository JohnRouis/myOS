#ifndef TSEM_H
#define TSEM_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tSem
{
    tEvent event;//事件控制块

    uint32_t count;//当前计数

    uint32_t maxCount;//最大计数量
}tSem;

void tSemInit(tSem* sem, uint32_t startCount, uint32_t maxCount);

#endif
