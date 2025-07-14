#include "tMemBlock.h"
#include "tinyOS.h"

void tMemBlockInit(tMemBlock* memBlock, uint8_t* memStart, uint32_t blockSize, uint32_t blockCnt)
{
    uint8_t* memBlockStart = (uint8_t*)memStart;
    uint8_t* memBlockEnd = memBlockStart + blockSize * blockCnt;

    if(blockSize < sizeof(tNode))//存储块需要放置链接指针,空间至少要比tNode大才行
    {
        return;
    }

    tEventInit(&memBlock->event, tEventTypeMemBlock);

    memBlock->memStart = memStart;
    memBlock->blockSize = blockSize;
    memBlock->maxCount = blockCnt;

    tListInit(&memBlock->blockList);//初始化列表
    while (memBlockStart < memBlockEnd)//将这些相同大小的存储块链接起来
    {
        tNodeInit((tNode*)memBlockStart);
        tListAddLast(&memBlock->blockList, (tNode*)memBlockStart);//链接到链表上
        memBlockStart += blockSize;//下一位
    }
    
}
