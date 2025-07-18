#include "tFlagGroup.h"
#include "tinyOS.h"

void tFlagGroupInit(tFlagGroup* flagGroup, uint32_t flags)
{
    tEventInit(&flagGroup->event, tEventTypeFlagGroup);
    flagGroup->flags = flags;
}

/* 
** Description: 辅助函数,负责检查并消耗事件标志
*/
static uint32_t tFlagGroupCheckAndConsume(tFlagGroup* flagGroup, uint32_t type, uint32_t* flags)
{
    //先将参数接收
    uint32_t srcFlags = *flags;
    uint32_t isSet = type & TFLAGGROUP_SET;//置1还是置0
    uint32_t isAll = type & TFLAGGROUP_ALL;//部分还是全部
    uint32_t isConsume = type & TFLAGGROUP_CONSUME;

    uint32_t calcFlag = isSet ? (flagGroup->flags & srcFlags) : (~flagGroup->flags & srcFlags);

    if(((isAll != 0) && (calcFlag == srcFlags)) || ((isAll == 0) && (calcFlag != 0)))
    {
        if(isConsume)
        {
            if(isSet)
            {
                //清除为1的标志位
                flagGroup->flags &= ~srcFlags;
            }
            else
            {
                //清除为0的标志位
                flagGroup->flags |= srcFlags;
            }
        }
        *flags = calcFlag;
        return tErrorNoError;
    }
    *flags = calcFlag;
    return tErrorResourceUnavaliable;
}

/*
** Description: 等待事件标志组
*/
uint32_t tFlagGroupWait(tFlagGroup* flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t* resultFlag, uint32_t waitTicks)
{
    uint32_t result;
    uint32_t flags = requestFlag;

    uint32_t status = tTaskEnterCritical();
    result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);//检查标志位并置1或0
    if(result != tErrorNoError)//不满足条件,插入队列等待
    {
        currentTask->waitFlagsType = waitType;
        currentTask->eventFlags = requestFlag;
        tEventWait(&flagGroup->event, currentTask, (void*)0, tEventTypeFlagGroup, waitTicks);

        tTaskExitCritical(status);

        tTaskSched();//挂起后进行一次调度

        *resultFlag = currentTask->eventFlags;//归来后记录值
        result = currentTask->waitEventResult;
    }
    else
    {
        *resultFlag = flags;
        tTaskExitCritical(status);
    }
    return result;
}

/*
** Description: 获取事件标志,不等待型
*/
uint32_t tFlagGroupNoWaitGet(tFlagGroup* flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t* resultFlag)
{
    uint32_t flags = requestFlag;

    uint32_t status = tTaskEnterCritical();

    uint32_t result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);//直接获取,有就有,没有就直接走

    tTaskExitCritical(status);

    *resultFlag = flags;
    return status;
}

/*
** Description: 通知事件标志组中的任务有新标志发生
*/
void tFlagGroupNotify(tFlagGroup* flagGroup, uint8_t isSet, uint32_t flags)
{
    tList* waitList;
    tNode* node;
    tNode* nextNode;
    uint8_t sched = 0;//调度标志位

    uint32_t status = tTaskEnterCritical();

    if(isSet)
    {
        flagGroup->flags |= flags;//置1
    }
    else
    {
        flagGroup->flags &= ~flags;//置0
    }
    //遍历所有任务,获取满足条件的任务,加入到移除列表中
    waitList = &flagGroup->event.waitList;
    for(node = waitList->headNode.nextNode; node != &(waitList->headNode); node = nextNode)
    {
        uint32_t result;
        tTask* task = tNodeParent(node, tTask, linkNode);
        uint32_t flags = task->eventFlags;
        nextNode = node->nextNode;
        //检查标志
        result = tFlagGroupCheckAndConsume(flagGroup, task->waitFlagsType, &flags);
        if(result == tErrorNoError)//对上了就唤醒
        {
            task->eventFlags = flags;
            tEventWakeUpTask(&flagGroup->event, task, (void*)0, tErrorNoError);
            sched = 1;
        }
    }
    if(sched == 1)//需要调度
    {
        tTaskSched();
    }
    tTaskExitCritical(status);
}

/*
** Description: 查询事件标志组的状态信息
*/
void tFlagGroupGetInfo(tFlagGroup* flagGroup, tFlagGroupInfo* info)
{
    uint32_t status = tTaskEnterCritical();

    info->flags = flagGroup->flags;
    info->taskCount = tEventWaitCount(&flagGroup->event);

    tTaskExitCritical(status);
}

/*
** Description: 销毁事件标志组
*/
uint32_t tFlagGroupDestroy(tFlagGroup* flagGroup)
{
    uint32_t status = tTaskEnterCritical();

    uint32_t count = tEventRemoveAll(&flagGroup->event, (void*)0, tErrorDel);

    tTaskExitCritical(status);

    if(count > 0)//如果还有任务在就绪状态,执行调度
    {
        tTaskSched();
    }
    return count;
}
