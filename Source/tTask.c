#include "tinyOS.h"

void tTaskInit(tTask* task, void (*entry)(void*), void* param, uint32_t prio, uint32_t* stack, uint32_t size)
{
    uint32_t* stackTop;//堆栈指针
    task->stackBase = stack;//起始地址赋值
    task->stackSize = size;
    memset(stack, 0, size);

    stackTop = stack + size / sizeof(tTaskStack);//堆栈指针初始化

	*(--stackTop) = (unsigned long)(1 << 24);
	*(--stackTop) = (unsigned long)entry;
	*(--stackTop) = (unsigned long)0x14;
	*(--stackTop) = (unsigned long)0x12;
	*(--stackTop) = (unsigned long)0x3;
	*(--stackTop) = (unsigned long)0x2;
	*(--stackTop) = (unsigned long)0x1;
	*(--stackTop) = (unsigned long)param;
	*(--stackTop) = (unsigned long)0x11;
	*(--stackTop) = (unsigned long)0x10;
	*(--stackTop) = (unsigned long)0x9;
	*(--stackTop) = (unsigned long)0x8;
	*(--stackTop) = (unsigned long)0x7;
	*(--stackTop) = (unsigned long)0x6;
	*(--stackTop) = (unsigned long)0x5;
	*(--stackTop) = (unsigned long)0x4;

	task->stack = stackTop;//更新堆栈指针位置
	task->delayTicks = 0;
	task->prio = prio;
	task->state = TINYOS_TASK_STATE_RDY;
	task->slice = TINYOS_SLICE_MAX;
    task->clean = (void(*)(void*))0;
    task->cleanParam = (void*)0;
    task->requestDeleteFlag = 0;

    task->waitEvent = (tEvent*)0;//没有等待事件
    task->eventMsg = (void*)0;
    task->waitEventResult = tErrorNoError;//没有等待事件错误

	tNodeInit(&(task->linkNode));
	tNodeInit(&(task->delayNode)); 

	tTaskSchedRdy(task);
}
//任务挂起函数
void tTaskSuspend(tTask* task)
{
    uint32_t status = tTaskEnterCritical();

    if (!(task->state & TINYOS_TASK_STATE_DELAYED))//不对已经进入延时状态的任务挂起
    {
        if(++task->suspendCount <= 1)//挂起次数1次
        {
            task->state |= TINYOS_TASK_STATE_SUSPEND;
            tTaskSchedUnRdy(task);//直接从就绪列表中移除
            if(task == currentTask)//挂起的是当前任务，切换到其他任务
            {
                tTaskSched();
            }
        }
    }
    

    tTaskExitCritical(status);
}
//任务唤醒
void tTaskWakeUp(tTask* task)
{
    uint32_t status = tTaskEnterCritical();

    if(task->state & TINYOS_TASK_STATE_SUSPEND)//检查任务是否处于挂起状态
    {
        if(--task->suspendCount == 0)
        {
            task->state &= -TINYOS_TASK_STATE_SUSPEND;//清除挂起标志

            tTaskSchedRdy(task);//放回就绪队列中

            tTaskSched();//唤醒过程中可能有更高优先级任务就绪，进行一次调度
        }
    }

    tTaskExitCritical(status);
}
//删除设置
void tTaskSetCleanCallFun(tTask* task, void (*clean)(void* paran), void* param)
{
    task->clean = clean;
    task->cleanParam = param;
}
/* 强制删除 */
void tTaskForceDelete(tTask* task)
{
    uint32_t status = tTaskEnterCritical();

    if(task->state & TINYOS_TASK_STATE_DELAYED)//如果处于延时队列中,从延时队列中删除
    {
        tTimeTaskRemove(task);
    }
    else if (!(task->state & TINYOS_TASK_STATE_SUSPEND))//不在挂起状态中，那就是在就绪态，从就绪列表中移除
    {
        tTaskSchedRemove(task);
    }

    if(task->clean)//如果有清理的回调函数，调用
    {
        task->clean(task->cleanParam);
    }

    if(currentTask == task)//被删的是自己，切换到另一个任务
    {
        tTaskSched();
    }

    tTaskExitCritical(status);
}

/* 请求删除 */
void tTaskRequestDelete(tTask* task)
{
    uint32_t status = tTaskEnterCritical();

    task->requestDeleteFlag = 1;

    tTaskExitCritical(status);
}

/* 检查函数,是否已经被请求删除自己 */
uint8_t tTaskIsRequestedDeleted(void)
{
    uint8_t delete;

    uint32_t status = tTaskEnterCritical();

    delete = currentTask->requestDeleteFlag;

    tTaskExitCritical(status);

    return delete;
}

/* 删除自己 */
void tTaskDeleteSelf(void)
{
    uint32_t status = tTaskEnterCritical();
    
    tTaskSchedRemove(currentTask);//从就绪队列中移除

    if(currentTask->clean)//如果有清理函数，调用
    {
        currentTask->clean(currentTask->cleanParam);
    }

    tTaskSched();

    tTaskExitCritical(status);
}
/* 任务信息的获取 */
void tTaskGetInfo(tTask* task, tTaskInfo* info)
{
    uint32_t* stackEnd;//末端指针

    uint32_t status = tTaskEnterCritical();

    info->delayTicks = task->delayTicks;

    info->prio = task->prio;

    info->state = task->state;

    info->slice = task->slice;

    info->suspendCount = task->suspendCount;

    //计算堆栈使用量
    info->stackFree = 0;
    stackEnd = task->stackBase;//末端赋值
    while ((*stackEnd++ == 0) && (stackEnd <= task->stackBase + task->stackSize / sizeof(tTaskStack)))
    {
        info->stackFree++;
    }
    
    //从单元数转成字节数
    info->stackFree *= sizeof(tTaskStack);

    tTaskExitCritical(status);
}
