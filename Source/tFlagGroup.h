#ifndef TFLAGGROUP_H
#define TFLAGGROUP_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tFlagGroup
{
    tEvent event;//事件控制块

    uint32_t flags;//当前事件标志
}tFlagGroup;

void tFlagGroupInit(tFlagGroup* flagGroup, uint32_t flags);

#endif
