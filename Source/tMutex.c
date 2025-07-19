#include "tMutex.h"
#include "tinyOS.h"

void tMutexInit(tMutex* mutex)
{
    tEventInit(&mutex->event, tEventTypeMutex);

    mutex->lockedCount = 0;
    mutex->owner = (tTask*)0;
    mutex->ownerOriginalPrio = TINYOS_PRO_COUNT;//最低优先级31,填32为了搞一个无效值
}

/*
**Description: 获取信号量
*/
uint32_t tMutexWait(tMutex* mutex, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    if(mutex->lockedCount <= 0)//没有锁定时
    {
        mutex->owner = currentTask;//锁定
        mutex->ownerOriginalPrio = currentTask->prio;//记录当前优先级
        mutex->lockedCount++;

        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else//已经被锁定了
    {
        if(mutex->owner == currentTask)//而且锁定的是当前任务
        {
            mutex->lockedCount++;
            tTaskExitCritical(status);
            return tErrorNoError;
        }
        else//锁定的不是当前任务 需要判断
        {
        //查看当前主任务的优先级
            if(currentTask->prio < mutex->owner->prio)//当前任务比主任务优先级更高
            {   //优先级继承方式处理
                tTask* owner = mutex->owner;

                //提升当前主任务优先级
                if(owner->state == TINYOS_TASK_STATE_RDY)//仍处于就绪队列中的话 更改任务在就绪表中的位置
                {
                    tTaskSchedUnRdy(owner);
                    owner->prio = currentTask->prio;//提升优先级
                    tTaskSchedRdy(owner);
                }
                else
                {
                    owner->prio = currentTask->prio;//其他状态就只修改优先级
                }
            }
            //进入等待队列中
            tEventWait(&mutex->event, currentTask, (void*)0, tEventTypeMutex, waitTicks);
            tTaskExitCritical(status);
            //当前任务已经等待了，执行一次调度
            tTaskSched();
            return currentTask->waitEventResult;
        }
    }
}

/*
** Description: 获取信号量,如果已经被锁定,立刻返回
*/
uint32_t tMutexNoWaitGet(tMutex* mutex)
{
    uint32_t status = tTaskEnterCritical();

    if(mutex->lockedCount <= 0)//还没锁定
    {
        mutex->owner = currentTask;
        mutex->ownerOriginalPrio = currentTask->prio;
        mutex->lockedCount++;

        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else
    {
        if(mutex->owner == currentTask)//锁定且为当前任务
        {
            mutex->lockedCount++;//拥有者再次wait，进行增加计数
            tTaskExitCritical(status);
            return tErrorNoError;
        }
        tTaskExitCritical(status);
        return tErrorResourceUnavaliable;
    }
}

/*
** Description: 通知操作,只能由拥有者释放
*/
uint32_t tMutexNotify(tMutex* mutex)
{
    uint32_t status = tTaskEnterCritical();

    if(mutex->lockedCount <= 0)//信号量未被锁定,直接退出
    {
        tTaskExitCritical(status);
        return tErrorNoError;
    }

    if(mutex->owner != currentTask)//不是拥有者释放,为非法行为
    {
        tTaskExitCritical(status);
        return tErrorOwner;
    }

    if(--mutex->lockedCount > 0)//递减后仍大于0,不需要唤醒等待任务
    {
        tTaskExitCritical(status);
        return tErrorNoError;
    }

    //拥有者释放,就看有无发生优先级继承机制
    if(mutex->ownerOriginalPrio != mutex->owner->prio)
    {
        //任务处于就绪状态,更改任务在就绪列表中的位置
        if(mutex->owner->state != TINYOS_TASK_STATE_RDY)
        {
            tTaskSchedUnRdy(mutex->owner);
            currentTask->prio = mutex->ownerOriginalPrio;
            tTaskSchedRdy(mutex->owner);
        }
        else//其他状态,就只修改优先级
        {
            currentTask->prio = mutex->ownerOriginalPrio;
        }
    }
    //看有没有任务在等待
    if(tEventWaitCount(&mutex->event) > 0)
    {
        //有就唤醒首部任务
        tTask* task = tEventWakeUp(&mutex->event, (void*)0, tErrorNoError);

        mutex->owner = task;//切换
        mutex->ownerOriginalPrio = task->prio;
        mutex->lockedCount++;
        //如果拥有者优先级更高,执行调度切换
        if(task->prio < currentTask->prio)
        {
            tTaskSched();
        }
    }
    tTaskExitCritical(status);
    return tErrorNoError;
}

/*
**Description: 销毁互斥信号量
*/
uint32_t tMutexDestroy(tMutex* mutex)
{
    uint32_t count = 0;
    uint32_t status = tTaskEnterCritical();

    //信号量被锁定了,没有锁定说明没有任务等待,就不用处理
    if(mutex->lockedCount > 0)
    {
        //有没有优先级继承发生 有就恢复拥有者优先级
        if(mutex->ownerOriginalPrio != mutex->owner->prio)
        {
            //处于就绪队列 更改任务在就绪队列中的位置
            if(mutex->owner->state == TINYOS_TASK_STATE_RDY)
            {
                tTaskSchedUnRdy(mutex->owner);
                mutex->owner->prio = mutex->ownerOriginalPrio;
                tTaskSchedRdy(mutex->owner);
            }
            else//其他状态只修改优先级
            {
                mutex->owner->prio = mutex->ownerOriginalPrio;
            }
        }
        count = tEventRemoveAll(&mutex->event, (void*)0, tErrorDel);
        if(count > 0)//如果有任务就绪,进行一次调度
        {
            tTaskSched();
        }
    }
    tTaskExitCritical(status);
    return count;
}

/*
** Description: 查询信号量接口
*/
void tMutexGetInfo(tMutex* mutex, tMutexInfo* info)
{
    uint32_t status = tTaskEnterCritical();

    info->taskCount = tEventWaitCount(&mutex->event);
    info->ownerPrio = mutex->ownerOriginalPrio;
    if(mutex->owner != (tTask*)0)
    {
        info->inheritedPrio = mutex->owner->prio;
    }
    else
    {
        info->inheritedPrio = TINYOS_PRO_COUNT;//设为32 无效值
    }
    info->owner = mutex->owner;
    info->lockedCount = mutex->lockedCount;

    tTaskExitCritical(status);
}
