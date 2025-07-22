#ifndef TMUTEX_H
#define TMUTEX_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tMutex
{
    tEvent event;//�¼����ƿ�

    uint32_t lockedCount;//�ѱ������Ĵ���

    tTask* owner;//ӵ����

    uint32_t ownerOriginalPrio;//ӵ���ߵ�ԭʼ���ȼ�
}tMutex;

typedef struct _tMutexInfo
{
    uint32_t taskCount;//�ȴ���������

    uint32_t ownerPrio;//ӵ���ߵ����ȼ�

    uint32_t inheritedPrio;//�̳����ȼ�

    tTask* owner;//��ǰ�ź���ӵ����

    uint32_t lockedCount;//��������
}tMutexInfo;


void tMutexInit(tMutex* mutex);

uint32_t tMutexWait(tMutex* mutex, uint32_t waitTicks);

uint32_t tMutexNoWaitGet(tMutex* mutex);

uint32_t tMutexNotify(tMutex* mutex);

uint32_t tMutexDestroy(tMutex* mutex);

void tMutexGetInfo(tMutex* mutex, tMutexInfo* info);

#endif
