#ifndef TMBOX_H
#define TMBOX_H

#include "tConfig.h"

#include "tEvent.h"

typedef struct _tMbox
{
    tEvent event;//事件控制块

    uint32_t count;//消息数量

    uint32_t read;//读索引

    uint32_t write;//写索引

    uint32_t maxCount;//最大容纳消息量

    void** msgBuffer;//消息存储缓存区
}tMbox;

void tMboxInit(tMbox* mbox, void** msgBuffer, uint32_t maxCount);

uint32_t tMboxWait(tMbox* mbox, void** msg, uint32_t waitTicks);

#endif
