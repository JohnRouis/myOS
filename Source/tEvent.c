#include "tinyOS.h"

void tEventInit(tEvent* event, tEventType type)
{
    event->type = tEventTypeUnkonwn;
    tListInit(&event->waitList);
}

/* 事件控制块的等待 */
void tEventWait(tEvent* event, tTask* task, void* msg, uint32_t state, uint32_t timeout)
{
    uint32_t status = tTaskEnterCritical();

    task->state = state;//标记任务处于某种事件的状态

    task->waitEvent = event;//任务等待的事件结构

    task->eventMsg = msg;//任务等待事件的消息存储位置

    task->waitEventResult = tErrorNoError;//清空事件等待结果

    tTaskSchedUnRdy(task);//从就绪队列中移除

    tListAddLast(&event->waitList, &task->linkNode);//插入事件等待队列中

    if(timeout)
    {
        tTimeTaskWait(task, timeout);//有延时设置,插入到延时队列当中
    }

    tTaskExitCritical(status);
}

/* 唤醒事件 */
tTask* tEventWakeUp(tEvent* event, void* msg, uint32_t result)
{
    tNode* node;
    tTask* task = (tTask*)0;

    uint32_t status = tTaskEnterCritical();

    if((node = tListRemoveFirst(&event->waitList)) != (tNode*)0)
    {
        task = (tTask*)tNodeParent(node, tTask, linkNode);//插入用的是链接结点，找到对应任务1先
        
        task->waitEvent = (tEvent*)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~TINYOS_TASK_WAIT_MASK;

        //如果申请了超时等待,从延时队列中移除
        if(task->delayTicks != 0)
        {
            tTimeTaskWakeUp(task);
        }

        //加入就绪队列
        tTaskSchedRdy(task);
    }

    tTaskExitCritical(status);
		
		return task;
}

/*
** Description: 唤醒固定任务
*/
void tEventWakeUpTask(tEvent* event, tTask* task, void* msg, uint32_t result)
{
    uint32_t status = tTaskEnterCritical();

    tListRemove(&event->waitList, &task->linkNode);

    //设置收到的消息 结构 清除相应等待标志位
    task->waitEvent = (tEvent*)0;
    task->eventMsg = msg;
    task->waitEventResult = result;
    task->state &= ~TINYOS_TASK_WAIT_MASK;

    if(task->delayTicks != 0)//任务申请了超时等待,从延时队列中清除
    {
        tTimeTaskWakeUp(task);
    }

    tTaskSchedRdy(task);//任务加入就绪队列中

    tTaskExitCritical(status);
}

/* 从等待队列中强制移除 */
void tEventRemoveTask(tTask* task, void* msg, uint32_t result)
{
    uint32_t status = tTaskEnterCritical();

    tListRemove(&task->waitEvent->waitList, &task->linkNode);//任务从等待队列中1移除

    task->waitEvent = (tEvent*)0;
    task->eventMsg = msg;
    task->waitEventResult = result;
    task->state &= ~TINYOS_TASK_WAIT_MASK;

    tTaskExitCritical(status);
}

/* 清空所有等待中的任务，返回唤醒数量 */
uint32_t tEventRemoveAll(tEvent* event, void* msg, uint32_t result)
{
    tNode* node;

    uint32_t count = 0;

    uint32_t status = tTaskEnterCritical();

    count = tListCount(&event->waitList);
    
    while ((node = tListRemoveFirst(&event->waitList)) != (tNode*)0)
    {
        tTask* task = (tTask*)tNodeParent(node, tTask, linkNode);
        task->waitEvent = (tEvent*)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~TINYOS_TASK_WAIT_MASK;

        if(task->delayTicks != 0)//申请超时等待则从延时队列中移除
        {
            tTimeTaskWakeUp(task);
        }
        tTaskSchedRdy(task);
    }
    

    tTaskExitCritical(status);
    return count;
}

/* 接口:查询事件控制块中的任务数量 */
uint32_t tEventWaitCount(tEvent* event)
{
    uint32_t count = 0;

    uint32_t status = tTaskEnterCritical();

    count = tListCount(&event->waitList);

    tTaskExitCritical(status);

    return count;
}
