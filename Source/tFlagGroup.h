#ifndef TFLAGGROUP_H
#define TFLAGGROUP_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tFlagGroup
{
    tEvent event;//�¼����ƿ�

    uint32_t flags;//��ǰ�¼���־
}tFlagGroup;

void tFlagGroupInit(tFlagGroup* flagGroup, uint32_t flags);

#endif
