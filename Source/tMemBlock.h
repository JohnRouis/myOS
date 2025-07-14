#ifndef TMEMBLOCK_H
#define TMEMBLOCK_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tMemBlock
{
    tEvent event;//�¼����ƿ�

    void* memStart;//�洢���׵�ַ

    uint32_t blockSize;//ÿ���洢��Ĵ�С

    uint32_t maxCount;//�ܴ洢�����

    tList blockList;//�洢���б�

}tMemBlock;

void tMemBlockInit(tMemBlock* memBlock, uint8_t* memStart, uint32_t blockSize, uint32_t blockCnt);


#endif
