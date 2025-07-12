#ifndef TSEM_H
#define TSEM_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tSem
{
    tEvent event;//�¼����ƿ�

    uint32_t count;//��ǰ����

    uint32_t maxCount;//��������
}tSem;

void tSemInit(tSem* sem, uint32_t startCount, uint32_t maxCount);

#endif
