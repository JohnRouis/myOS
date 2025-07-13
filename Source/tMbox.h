#ifndef TMBOX_H
#define TMBOX_H

#include "tConfig.h"

#include "tEvent.h"

typedef struct _tMbox
{
    tEvent event;//�¼����ƿ�

    uint32_t count;//��Ϣ����

    uint32_t read;//������

    uint32_t write;//д����

    uint32_t maxCount;//���������Ϣ��

    void** msgBuffer;//��Ϣ�洢������
}tMbox;

void tMboxInit(tMbox* mbox, void** msgBuffer, uint32_t maxCount);

uint32_t tMboxWait(tMbox* mbox, void** msg, uint32_t waitTicks);

#endif
