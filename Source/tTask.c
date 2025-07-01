#include "tinyOS.h"

void tTaskInit(tTask* task, void (*entry)(void*), void* param, uint32_t prio, uint32_t* stack)
{
	*(--stack) = (unsigned long)(1 << 24);
	*(--stack) = (unsigned long)entry;
	*(--stack) = (unsigned long)0x14;
	*(--stack) = (unsigned long)0x12;
	*(--stack) = (unsigned long)0x3;
	*(--stack) = (unsigned long)0x2;
	*(--stack) = (unsigned long)0x1;
	*(--stack) = (unsigned long)param;
	*(--stack) = (unsigned long)0x11;
	*(--stack) = (unsigned long)0x10;
	*(--stack) = (unsigned long)0x9;
	*(--stack) = (unsigned long)0x8;
	*(--stack) = (unsigned long)0x7;
	*(--stack) = (unsigned long)0x6;
	*(--stack) = (unsigned long)0x5;
	*(--stack) = (unsigned long)0x4;

	task->stack = stack;//���¶�ջָ��λ��
	task->delayTicks = 0;
	task->prio = prio;
	task->state = TINYOS_TASK_STATE_RDY;
	task->slice = TINYOS_SLICE_MAX;
    task->clean = (void(*)(void*))0;
    task->cleanParam = (void*)0;
    task->requestDeleteFlag = 0;

	tNodeInit(&(task->linkNode));
	tNodeInit(&(task->delayNode)); 

	tTaskSchedRdy(task);
}
//���������
void tTaskSuspend(tTask* task)
{
    uint32_t status = tTaskEnterCritical();

    if (!(task->state & TINYOS_TASK_STATE_DELAYED))//�����Ѿ�������ʱ״̬���������
    {
        if(++task->suspendCount <= 1)//�������1��
        {
            task->state |= TINYOS_TASK_STATE_SUSPEND;
            tTaskSchedUnRdy(task);//ֱ�ӴӾ����б����Ƴ�
            if(task == currentTask)//������ǵ�ǰ�����л�����������
            {
                tTaskSched();
            }
        }
    }
    

    tTaskExitCritical(status);
}
//������
void tTaskWakeUp(tTask* task)
{
    uint32_t status = tTaskEnterCritical();

    if(task->state & TINYOS_TASK_STATE_SUSPEND)//��������Ƿ��ڹ���״̬
    {
        if(--task->suspendCount == 0)
        {
            task->state &= -TINYOS_TASK_STATE_SUSPEND;//��������־

            tTaskSchedRdy(task);//�Żؾ���������

            tTaskSched();//���ѹ����п����и������ȼ��������������һ�ε���
        }
    }

    tTaskExitCritical(status);
}
//ɾ������
void tTaskSetCleanCallFun(tTask* task, void (*clean)(void* paran), void* param)
{
    task->clean = clean;
    task->cleanParam = param;
}
/* ǿ��ɾ�� */
void tTaskForceDelete(tTask* task)
{
    uint32_t status = tTaskEnterCritical();

    if(task->state & TINYOS_TASK_STATE_DELAYED)//���������ʱ������,����ʱ������ɾ��
    {
        tTimeTaskRemove(task);
    }
    else if (!(task->state & TINYOS_TASK_STATE_SUSPEND))//���ڹ���״̬�У��Ǿ����ھ���̬���Ӿ����б����Ƴ�
    {
        tTaskSchedRemove(task);
    }

    if(task->clean)//���������Ļص�����������
    {
        task->clean(task->cleanParam);
    }

    if(currentTask == task)//��ɾ�����Լ����л�����һ������
    {
        tTaskSched();
    }

    tTaskExitCritical(status);
}

/* ����ɾ�� */
void tTaskRequestDelete(tTask* task)
{
    uint32_t status = tTaskEnterCritical();

    task->requestDeleteFlag = 1;

    tTaskExitCritical(status);
}

/* ��麯��,�Ƿ��Ѿ�������ɾ���Լ� */
uint8_t tTaskIsRequestedDeleted(void)
{
    uint8_t delete;

    uint32_t status = tTaskEnterCritical();

    delete = currentTask->requestDeleteFlag;

    tTaskExitCritical(status);

    return delete;
}

/* ɾ���Լ� */
void tTaskDeleteSelf(void)
{
    uint32_t status = tTaskEnterCritical();
    
    tTaskSchedRemove(currentTask);//�Ӿ����������Ƴ�

    if(currentTask->clean)//�����������������
    {
        currentTask->clean(currentTask->cleanParam);
    }

    tTaskSched();

    tTaskExitCritical(status);
}
/* ������Ϣ�Ļ�ȡ */
void tTaskGetInfo(tTask* task, tTaskInfo* info)
{
    uint32_t status = tTaskEnterCritical();

    info->delayTicks = task->delayTicks;

    info->prio = task->prio;

    info->state = task->state;

    info->slice = task->slice;

    info->suspendCount = task->suspendCount;

    tTaskExitCritical(status);
}
