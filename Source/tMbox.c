#include "tMbox.h"
#include "tinyOS.h"

/* 邮箱初始化 */
void tMboxInit(tMbox* mbox, void** msgBuffer, uint32_t maxCount)
{
    tEventInit(&mbox->event, tEventTypeMbox);

    mbox->msgBuffer = msgBuffer;
    mbox->maxCount = maxCount;
    mbox->read = 0;
    mbox->write = 0;
    mbox->count = 0;
}

/* 
** 等待邮箱,获取一则消息 
** parameter:msg 消息缓存区
*/
uint32_t tMboxWait(tMbox* mbox, void** msg, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    if(mbox->count > 0)//看消息计数大于0不
    {
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];//取出一个然后索引往后挪

        if(mbox->read >= mbox->maxCount)//边界回绕
        {
            mbox->read = 0;
        }
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else
    {
        tEventWait(&mbox->event, currentTask, (void*)0, tEventTypeMbox, waitTicks);//没有就将任务插入等待队列中
        tTaskExitCritical(status);

        tTaskSched();//调度一次,换成其他任务

        *msg = currentTask->eventMsg;//如果邮箱有信息发到,就会重新回到这个函数位置,这个时候就需要将邮件信息存好

        return currentTask->waitEventResult;

    }
}

/*
** descripttion: 邮箱没消息时选择不等待
** 
*/
uint32_t tMboxNoWaitGet(tMbox* mbox, void** msg)
{
    uint32_t status = tTaskEnterCritical();

    if(mbox->count > 0)
    {
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];

        if(mbox->read >= mbox->maxCount)
        {
            mbox->read = 0;
        }
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else//没有的话直接返回资源不可调用提示
    {
        tTaskExitCritical(status);
        return tErrorResourceUnavaliable;
    }
}
/*
** Description: 邮箱通知操作,通知有消息可用,唤醒等待队列中的一个任务,或者将消息插入到邮箱中
*/
uint32_t tMboxNotify(tMbox* mbox, void* msg, uint32_t notifyOption)
{
    uint32_t status = tTaskEnterCritical();

    if(tEventWaitCount(&mbox->event) > 0)//检查是否有任务等待
    {
        tTask* task = tEventWakeUp(&mbox->event, (void*)msg, tErrorNoError);//有就唤醒首部任务

        if(task->prio < currentTask->prio)//如果这个任务的优先级更高则调度过去
        {
            tTaskSched();
        }
    }
    else//没有任务等待,将消息插入到缓冲区中
    {
        if(mbox->count >= mbox->maxCount)
        {
            tTaskExitCritical(status);
            return tErrorResourceFull;
        }
        if(notifyOption & tMBOXSendFront)
        {
            if(mbox->read <= 0)
            {
                mbox->read = mbox->maxCount - 1;//环形结构保持
            }
            else
            {
                --mbox->read;
            }
            mbox->msgBuffer[mbox->read] = msg;
        }
        else
        {
            mbox->msgBuffer[mbox->write++] = msg;
            if(mbox->write >= mbox->maxCount)
            {
                mbox->write = 0;
            }
        }
        mbox->count++;
    }
    tTaskExitCritical(status);
    return tErrorNoError;
}

/*
** Description: 清空邮箱消息
*/
void tMboxFlush(tMbox* mbox)
{
    uint32_t status = tTaskEnterCritical();

    if(tEventWaitCount(&mbox->event) == 0)//没有任务等待就需要判断一下，如果还有任务等待，说明邮箱已经是空了
    {
        mbox->read = 0;
        mbox->write = 0;
        mbox->count = 0;
    }

    tTaskExitCritical(status);
}

/*
**Description: 销毁邮箱
*/
uint32_t  tMboxDestroy(tMbox* mbox)
{
    uint32_t status = tTaskEnterCritical();
    //清除事件控制块所有任务
    uint32_t count = tEventRemoveAll(&mbox->event, (void*)0, tErrorDel);
    tTaskExitCritical(status);

    if(count > 0)
    {
        tTaskSched();
    }
    return count;
}

/*
** Description:查询状态信息
*/
void tMboxGetInfo(tMbox* mbox, tMboxInfo* info)
{
    uint32_t status = tTaskEnterCritical();

    info->count = mbox->count;
    info->maxCount = mbox->maxCount;
    info->taskCount = tEventWaitCount(&mbox->event);

    tTaskExitCritical(status);
}
