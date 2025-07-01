#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>
#include "tConfig.h"
#include "tLib.h"
//延时状态
#define TINYOS_TASK_STATE_RDY         0
#define TINYOS_TASK_STATE_DESTROYED  (1 << 0)//删除标志位
#define TINYOS_TASK_STATE_DELAYED    (1 << 1)//任务处于延时状态
#define TINYOS_TASK_STATE_SUSPEND    (1 << 2)

//cortex-m3堆栈单元类型，堆栈单元大小为32位
typedef uint32_t tTaskStack;

typedef struct tTask
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

}tTask;

extern tTask* currentTask;

extern tTask* nextTask;

void tTaskSwitch(void);

void tTaskRunFirst(void);

void tTaskSchedInit(void);

void tTaskSchedDisable(void);

void tTaskSchedEnable(void);

void tTaskSchedRemove(tTask* task);

uint32_t tTaskEnterCritical(void);

void tTaskExitCritical(uint32_t status);

void tTaskSched(void);

void tTaskSchedRdy(tTask* task);

void tTaskSchedUnRdy(tTask* task);

void tTaskSystemTickHandler();

void tTaskDelay(uint32_t delay);
typedef struct _tTaskInfo
{
    uint32_t delayTicks;
    uint32_t prio;
    uint32_t state;
    uint32_t slice;
    uint32_t suspendCount;
}tTaskInfo;

void tTimeTaskWait(tTask* task, uint32_t ticks);

void tTimeTaskWakeUp(tTask* task);

void tTimeTaskRemove(tTask* task);

void tTaskInit(tTask* task, void (*entry)(void*), void* param, uint32_t prio, uint32_t* stack);

void tSetSysTickPeriod(uint32_t ms);

void tInitApp(void);

void tTaskSetCleanCallFun(tTask* task, void (*clean)(void* paran), void* param);

void tTaskForceDelete(tTask* task);

void tTaskRequestDelete(tTask* task);

uint8_t tTaskIsRequestedDeleted(void);

void tTaskDeleteSelf(void);

void tTaskGetInfo(tTask* task, tTaskInfo* info);

#endif
