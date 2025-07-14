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

void tMemBlockInit(tMemBlock* memBlock, uint8_t* memStart, uint32_t blockSize, uint32_t blockCnt);


#endif
