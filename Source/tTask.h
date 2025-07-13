#ifndef TTASK_H
#define TTASK_H

//延时状态
#define TINYOS_TASK_STATE_RDY         0
#define TINYOS_TASK_STATE_DESTROYED  (1 << 0)//删除标志位
#define TINYOS_TASK_STATE_DELAYED    (1 << 1)//任务处于延时状态
#define TINYOS_TASK_STATE_SUSPEND    (1 << 2)
#define TINYOS_TASK_WAIT_MASK        (0xFF << 16)//等待标志

//cortex-m3堆栈单元类型，堆栈单元大小为32位
typedef uint32_t tTaskStack;

struct  _tEvent;//前置声明

typedef struct _tTask
{
    tTaskStack* stack;

    uint32_t delayTicks;//任务延时计数

    tNode linkNode;//任务链接结点

    tNode delayNode;//延时结点

    uint32_t prio;//任务的优先级

    uint32_t state;//任务当前状态

    uint32_t slice;//当前剩余时间片

    uint32_t suspendCount;//被挂起的次数

    void (*clean) (void* param);//任务删除时的清理函数

    void* cleanParam;//传递给清理函数的参数u

    uint8_t requestDeleteFlag;//请求删除标志 非0表示请求删除

    struct _tEvent* waitEvent;//任务正在等待的事件类型
    
    void* eventMsg;//等待事件的存储位置

    uint32_t waitEventResult;//等待事件结果
}tTask;

typedef struct _tTaskInfo
{
    uint32_t delayTicks;
    uint32_t prio;
    uint32_t state;
    uint32_t slice;
    uint32_t suspendCount;
}tTaskInfo;

void tTaskInit(tTask* task, void (*entry)(void*), void* param, uint32_t prio, uint32_t* stack);

void tTaskSuspend(tTask* task);

void tTaskWakeUp(tTask* task);

void tTaskSetCleanCallFun(tTask* task, void (*clean)(void* paran), void* param);

void tTaskForceDelete(tTask* task);

void tTaskRequestDelete(tTask* task);

uint8_t tTaskIsRequestedDeleted(void);

void tTaskDeleteSelf(void);

void tTaskGetInfo(tTask* task, tTaskInfo* info);

#endif
