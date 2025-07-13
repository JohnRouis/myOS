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
