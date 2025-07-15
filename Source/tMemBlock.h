#ifndef TMEMBLOCK_H
#define TMEMBLOCK_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tMemBlock
{
    tEvent event;//事件控制块

    void* memStart;//存储块首地址

    uint32_t blockSize;//每个存储块的大小

    uint32_t maxCount;//总存储块个数

    tList blockList;//存储块列表

}tMemBlock;

typedef struct _tMemBlockInfo
{
    uint32_t count;//当前存储块块数

    uint32_t maxCount;//最大存储块个数

    uint32_t blockSize;//每个存储块的大小

    uint32_t taskCount;//当前等待的任务计数
}tMemBlockInfo;


void tMemBlockInit(tMemBlock* memBlock, uint8_t* memStart, uint32_t blockSize, uint32_t blockCnt);

uint32_t tMemBlockWait(tMemBlock* memBlock, uint8_t** mem, uint32_t waitTicks);

uint32_t tMemBlockNoWaitGet(tMemBlock* memBlock, uint8_t** mem);

void tMemBlockNotify(tMemBlock* memBlock, uint8_t* mem);

void tMemBlockGetInfo(tMemBlock* memBlock, tMemBlockInfo* info);

uint32_t tMemBlockDestroy(tMemBlock* memBlock);

#endif
