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

typedef struct _tMemBlockInfo
{
    uint32_t count;//��ǰ�洢�����

    uint32_t maxCount;//���洢�����

    uint32_t blockSize;//ÿ���洢��Ĵ�С

    uint32_t taskCount;//��ǰ�ȴ����������
}tMemBlockInfo;


void tMemBlockInit(tMemBlock* memBlock, uint8_t* memStart, uint32_t blockSize, uint32_t blockCnt);

uint32_t tMemBlockWait(tMemBlock* memBlock, uint8_t** mem, uint32_t waitTicks);

uint32_t tMemBlockNoWaitGet(tMemBlock* memBlock, uint8_t** mem);

void tMemBlockNotify(tMemBlock* memBlock, uint8_t* mem);

void tMemBlockGetInfo(tMemBlock* memBlock, tMemBlockInfo* info);

uint32_t tMemBlockDestroy(tMemBlock* memBlock);

#endif
