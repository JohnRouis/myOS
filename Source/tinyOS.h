#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>
#include "tConfig.h"
#include "tLib.h"
//��ʱ״̬
#define TINYOS_TASK_STATE_RDY         0
#define TINYOS_TASK_STATE_DESTROYED  (1 << 0)//ɾ����־λ
#define TINYOS_TASK_STATE_DELAYED    (1 << 1)//��������ʱ״̬
#define TINYOS_TASK_STATE_SUSPEND    (1 << 2)

//cortex-m3��ջ��Ԫ���ͣ���ջ��Ԫ��СΪ32λ
typedef uint32_t tTaskStack;

typedef struct tTask
{
    tTaskStack* stack;

    uint32_t delayTicks;//������ʱ����

    tNode linkNode;//�������ӽ��

    tNode delayNode;//��ʱ���

    uint32_t prio;//��������ȼ�

    uint32_t state;//����ǰ״̬

    uint32_t slice;//��ǰʣ��ʱ��Ƭ

    uint32_t suspendCount;//������Ĵ���

    void (*clean) (void* param);//����ɾ��ʱ��������

    void* cleanParam;//���ݸ��������Ĳ���u

    uint8_t requestDeleteFlag;//����ɾ����־ ��0��ʾ����ɾ��

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
