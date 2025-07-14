#ifndef TMBOX_H
#define TMBOX_H

#include "tConfig.h"

#include "tEvent.h"

#define tMBOXSendFront   0x01//发送至缓存区头部
#define tMBOXSendNormal  0x00//正常发送至缓存区

typedef struct _tMbox
{
    tEvent event;//事件控制块

    uint32_t count;//消息数量

    uint32_t read;//读索引

    uint32_t write;//写索引

    uint32_t maxCount;//最大容纳消息量

    void** msgBuffer;//消息存储缓存区
}tMbox;

typedef struct _tMboxInfo
{
    uint32_t  count;
    uint32_t maxCount;
    uint32_t taskCount;
}tMboxInfo;


void tMboxInit(tMbox* mbox, void** msgBuffer, uint32_t maxCount);

uint32_t tMboxWait(tMbox* mbox, void** msg, uint32_t waitTicks);

uint32_t tMboxNoWaitGet(tMbox* mbox, void** msg);

uint32_t tMboxNotify(tMbox* mbox, void* msg, uint32_t notifyOption);

void tMboxFlush(tMbox* mbox);

uint32_t  tMboxDestroy(tMbox* mbox);

void tMboxGetInfo(tMbox* mbox, tMboxInfo* info);

#endif
