#ifndef TMBOX_H
#define TMBOX_H

#include "tConfig.h"

#include "tEvent.h"

#define tMBOXSendFront   0x01//������������ͷ��
#define tMBOXSendNormal  0x00//����������������

typedef struct _tMbox
{
    tEvent event;//�¼����ƿ�

    uint32_t count;//��Ϣ����

    uint32_t read;//������

    uint32_t write;//д����

    uint32_t maxCount;//���������Ϣ��

    void** msgBuffer;//��Ϣ�洢������
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
