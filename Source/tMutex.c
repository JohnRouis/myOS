#include "tMutex.h"
#include "tinyOS.h"

void tMutexInit(tMutex* mutex)
{
    tEventInit(&mutex->event, tEventTypeMutex);

    mutex->lockedCount = 0;
    mutex->owner = (tTask*)0;
    mutex->ownerOriginalPrio = TINYOS_PRO_COUNT;//������ȼ�31,��32Ϊ�˸�һ����Чֵ
}

/*
**Description: ��ȡ�ź���
*/
uint32_t tMutexWait(tMutex* mutex, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    if(mutex->lockedCount <= 0)//û������ʱ
    {
        mutex->owner = currentTask;//����
        mutex->ownerOriginalPrio = currentTask->prio;//��¼��ǰ���ȼ�
        mutex->lockedCount++;

        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else//�Ѿ���������
    {
        if(mutex->owner == currentTask)//�����������ǵ�ǰ����
        {
            mutex->lockedCount++;
            tTaskExitCritical(status);
            return tErrorNoError;
        }
        else//�����Ĳ��ǵ�ǰ���� ��Ҫ�ж�
        {
        //�鿴��ǰ����������ȼ�
            if(currentTask->prio < mutex->owner->prio)//��ǰ��������������ȼ�����
            {   //���ȼ��̳з�ʽ����
                tTask* owner = mutex->owner;

                //������ǰ���������ȼ�
                if(owner->state == TINYOS_TASK_STATE_RDY)//�Դ��ھ��������еĻ� ���������ھ������е�λ��
                {
                    tTaskSchedUnRdy(owner);
                    owner->prio = currentTask->prio;//�������ȼ�
                    tTaskSchedRdy(owner);
                }
                else
                {
                    owner->prio = currentTask->prio;//����״̬��ֻ�޸����ȼ�
                }
            }
            //����ȴ�������
            tEventWait(&mutex->event, currentTask, (void*)0, tEventTypeMutex, waitTicks);
            tTaskExitCritical(status);
            //��ǰ�����Ѿ��ȴ��ˣ�ִ��һ�ε���
            tTaskSched();
            return currentTask->waitEventResult;
        }
    }
}

/*
** Description: ��ȡ�ź���,����Ѿ�������,���̷���
*/
uint32_t tMutexNoWaitGet(tMutex* mutex)
{
    uint32_t status = tTaskEnterCritical();

    if(mutex->lockedCount <= 0)//��û����
    {
        mutex->owner = currentTask;
        mutex->ownerOriginalPrio = currentTask->prio;
        mutex->lockedCount++;

        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else
    {
        if(mutex->owner == currentTask)//������Ϊ��ǰ����
        {
            mutex->lockedCount++;//ӵ�����ٴ�wait���������Ӽ���
            tTaskExitCritical(status);
            return tErrorNoError;
        }
        tTaskExitCritical(status);
        return tErrorResourceUnavaliable;
    }
}

/*
** Description: ֪ͨ����,ֻ����ӵ�����ͷ�
*/
uint32_t tMutexNotify(tMutex* mutex)
{
    uint32_t status = tTaskEnterCritical();

    if(mutex->lockedCount <= 0)//�ź���δ������,ֱ���˳�
    {
        tTaskExitCritical(status);
        return tErrorNoError;
    }

    if(mutex->owner != currentTask)//����ӵ�����ͷ�,Ϊ�Ƿ���Ϊ
    {
        tTaskExitCritical(status);
        return tErrorOwner;
    }

    if(--mutex->lockedCount > 0)//�ݼ����Դ���0,����Ҫ���ѵȴ�����
    {
        tTaskExitCritical(status);
        return tErrorNoError;
    }

    //ӵ�����ͷ�,�Ϳ����޷������ȼ��̳л���
    if(mutex->ownerOriginalPrio != mutex->owner->prio)
    {
        //�����ھ���״̬,���������ھ����б��е�λ��
        if(mutex->owner->state != TINYOS_TASK_STATE_RDY)
        {
            tTaskSchedUnRdy(mutex->owner);
            currentTask->prio = mutex->ownerOriginalPrio;
            tTaskSchedRdy(mutex->owner);
        }
        else//����״̬,��ֻ�޸����ȼ�
        {
            currentTask->prio = mutex->ownerOriginalPrio;
        }
    }
    //����û�������ڵȴ�
    if(tEventWaitCount(&mutex->event) > 0)
    {
        //�оͻ����ײ�����
        tTask* task = tEventWakeUp(&mutex->event, (void*)0, tErrorNoError);

        mutex->owner = task;//�л�
        mutex->ownerOriginalPrio = task->prio;
        mutex->lockedCount++;
        //���ӵ�������ȼ�����,ִ�е����л�
        if(task->prio < currentTask->prio)
        {
            tTaskSched();
        }
    }
    tTaskExitCritical(status);
    return tErrorNoError;
}

/*
**Description: ���ٻ����ź���
*/
uint32_t tMutexDestroy(tMutex* mutex)
{
    uint32_t count = 0;
    uint32_t status = tTaskEnterCritical();

    //�ź�����������,û������˵��û������ȴ�,�Ͳ��ô���
    if(mutex->lockedCount > 0)
    {
        //��û�����ȼ��̳з��� �оͻָ�ӵ�������ȼ�
        if(mutex->ownerOriginalPrio != mutex->owner->prio)
        {
            //���ھ������� ���������ھ��������е�λ��
            if(mutex->owner->state == TINYOS_TASK_STATE_RDY)
            {
                tTaskSchedUnRdy(mutex->owner);
                mutex->owner->prio = mutex->ownerOriginalPrio;
                tTaskSchedRdy(mutex->owner);
            }
            else//����״ֻ̬�޸����ȼ�
            {
                mutex->owner->prio = mutex->ownerOriginalPrio;
            }
        }
        count = tEventRemoveAll(&mutex->event, (void*)0, tErrorDel);
        if(count > 0)//������������,����һ�ε���
        {
            tTaskSched();
        }
    }
    tTaskExitCritical(status);
    return count;
}

/*
** Description: ��ѯ�ź����ӿ�
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
        info->inheritedPrio = TINYOS_PRO_COUNT;//��Ϊ32 ��Чֵ
    }
    info->owner = mutex->owner;
    info->lockedCount = mutex->lockedCount;

    tTaskExitCritical(status);
}
