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

/* 
** Description: 等待存储块
*/
uint32_t tMemBlockWait(tMemBlock* memBlock, uint8_t** mem, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    if(tListCount(&memBlock->blockList) > 0)//检查有没有存储块
    {
        *mem = (uint8_t*)tListRemoveFirst(&memBlock->blockList);//取出一个
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else
    {
        tEventWait(&memBlock->event, currentTask, (void*)0, tEventTypeMemBlock, waitTicks);
        tTaskExitCritical(status);

        tTaskSched();//调度到其他任务,剩下的回来后再执行

        *mem = currentTask->eventMsg;//

        return currentTask->waitEventResult;//取出等待结果
    }
}

/* 
** Description: 等待存储块,得不到就不等了版本
*/
uint32_t tMemBlockNoWaitGet(tMemBlock* memBlock, uint8_t** mem)
{
    uint32_t status = tTaskEnterCritical();

    if(tListCount(&memBlock->blockList) > 0)
    {
        *mem = (uint8_t*)tListRemoveFirst(&memBlock->blockList);
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else
    {
        tTaskExitCritical(status);//退出
        return tErrorResourceUnavaliable;//资源不可调用
    }
}

/*
** Description: 通知存储块可用
*/
void tMemBlockNotify(tMemBlock* memBlock, uint8_t* mem)
{
    uint32_t status = tTaskEnterCritical();

    if(tEventWaitCount(&memBlock->event) > 0)//检查有没有任务在等待
    {
        tTask* task = tEventWakeUp(&memBlock->event, (void*)mem, tErrorNoError);

        if(task->prio < currentTask->prio)//如果优先级更高
        {
            tTaskSched();//开始调度
        }
    }
    else//没有任务等待,将存储块插入等待队列中
    {
        tListAddLast(&memBlock->blockList, (tNode*)mem);
    }

    tTaskExitCritical(status);
}

/*
** Description: 查询存储块结构体状态信息
*/
void tMemBlockGetInfo(tMemBlock* memBlock, tMemBlockInfo* info)
{
    uint32_t status = tTaskEnterCritical();

    info->count = tListCount(&memBlock->blockList);
    info->maxCount = memBlock->maxCount;
    info->blockSize = memBlock->blockSize;
    info->taskCount = tEventWaitCount(&memBlock->event);

    tTaskExitCritical(status);
}

/*
** Description: 销毁控制块
*/
uint32_t tMemBlockDestroy(tMemBlock* memBlock)
{   
    uint32_t status = tTaskEnterCritical();

    uint32_t count = tEventRemoveAll(&memBlock->blockList, (void*)0, tErrorDel);//清空所有控制块任务

    tTaskExitCritical(status);

    if(count > 0)//如果还有任务剩下
    {
        tTaskSched();//进行一次任务调度
    }
    return count;
}
