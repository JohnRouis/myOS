#include "tSem.h"
#include "tinyOS.h"

#if TINYOS_ENABLE_SEM == 1

void tSemInit(tSem* sem, uint32_t startCount, uint32_t maxCount)
{
    tEventInit(&sem->event, tEventTypeSem);
    sem->maxCount = maxCount;

    if(maxCount == 0)
    {
        sem->count = startCount;
    }
    else
    {
        sem->count = (startCount > maxCount) ? maxCount : startCount;
    }
}

/* 等待信号量 */
uint32_t tSemWait(tSem* sem, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    if(sem->count > 0)//如果还有资源
    {
        --sem->count;
        tTaskExitCritical(status);//直接退出
        return tErrorNoError;
    }
    else
    {   
        tEventWait(&sem->event, currentTask, (void*)0, tEventTypeSem, waitTicks);//将任务插入到事件队列中
        tTaskExitCritical(status);
        //进行一次事件调度，方便切换到其他任务
        tTaskSched();
        return currentTask->waitEventResult;
    }
}
/* 获取信号量 不等待版本 立即返回 */
uint32_t tSemNoWaitGet(tSem* sem)
{
    uint32_t status = tTaskEnterCritical();

    if(sem->count > 0)//有资源可用
    {
        --sem->count;
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else//没有就直接返回
    {
        tTaskExitCritical(status);
        return tErrorResourceUnavaliable;//资源不可用
    }
}

/* 通知信号量接口 */
void tSemNotify(tSem* sem)
{
    uint32_t status = tTaskEnterCritical();

    if(tEventWaitCount(&sem->event) > 0)//看有没有任务在等待
    { 
        tTask* task = tEventWakeUp(&sem->event, (void*)0, tErrorNoError);//有就唤醒队列首部的任务
        if(task->prio < currentTask->prio)//如果任务优先级更高就执行
        {
            tTaskSched();
        }
        else
        {
            ++sem->count;
            if((sem->maxCount != 0) && (sem->count > sem->maxCount))
            {
                sem->count = sem->maxCount;
            }
        }
    }

    tTaskExitCritical(status);
}

/* 查询接口 */
void tSemGetInfo(tSem* sem, tSemInfo* info)
{
    uint32_t status = tTaskEnterCritical();

    info->count = sem->count;
    info->maxCount = sem->maxCount;
    info->taskCount = tEventWaitCount(&sem->event);

    tTaskExitCritical(status);
}

/* 删除信号量接口 */
uint32_t tSemDestroy(tSem* sem)
{
    uint32_t status = tTaskEnterCritical();

    uint32_t count = tEventRemoveAll(&sem->event, (void*)0, tErrorDel);
    sem->count = 0;
    tTaskExitCritical(status);

    //清空过程中如果还有任务就绪,进行一次调度
    if(count > 0)
    {
        tTaskSched();
    }

    return count;
}

#endif
