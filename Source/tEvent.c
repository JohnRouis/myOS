#include "tinyOS.h"

void tEventInit(tEvent* event, tEventType type)
{
    event->type = tEventTypeUnkonwn;
    tListInit(&event->waitList);
}

/* �¼����ƿ�ĵȴ� */
void tEventWait(tEvent* event, tTask* task, void* msg, uint32_t state, uint32_t timeout)
{
    uint32_t status = tTaskEnterCritical();

    task->state = state;//���������ĳ���¼���״̬

    task->waitEvent = event;//����ȴ����¼��ṹ

    task->eventMsg = msg;//����ȴ��¼�����Ϣ�洢λ��

    task->waitEventResult = tErrorNoError;//����¼��ȴ����

    tTaskSchedUnRdy(task);//�Ӿ����������Ƴ�

    tListAddLast(&event->waitList, &task->linkNode);//�����¼��ȴ�������

    if(timeout)
    {
        tTimeTaskWait(task, timeout);//����ʱ����,���뵽��ʱ���е���
    }

    tTaskExitCritical(status);
}

/* �����¼� */
tTask* tEventWakeUp(tEvent* event, void* msg, uint32_t result)
{
    tNode* node;
    tTask* task = (tTask*)0;

    uint32_t status = tTaskEnterCritical();

    if((node = tListRemoveFirst(&event->waitList)) != (tNode*)0)
    {
        task = (tTask*)tNodeParent(node, tTask, linkNode);//�����õ������ӽ�㣬�ҵ���Ӧ����1��
        
        task->waitEvent = (tEvent*)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~TINYOS_TASK_WAIT_MASK;

        //��������˳�ʱ�ȴ�,����ʱ�������Ƴ�
        if(task->delayTicks != 0)
        {
            tTimeTaskWakeUp(task);
        }

        //�����������
        tTaskSchedRdy(task);
    }

    tTaskExitCritical(status);
		
		return task;
}

/*
** Description: ���ѹ̶�����
*/
void tEventWakeUpTask(tEvent* event, tTask* task, void* msg, uint32_t result)
{
    uint32_t status = tTaskEnterCritical();

    tListRemove(&event->waitList, &task->linkNode);

    //�����յ�����Ϣ �ṹ �����Ӧ�ȴ���־λ
    task->waitEvent = (tEvent*)0;
    task->eventMsg = msg;
    task->waitEventResult = result;
    task->state &= ~TINYOS_TASK_WAIT_MASK;

    if(task->delayTicks != 0)//���������˳�ʱ�ȴ�,����ʱ���������
    {
        tTimeTaskWakeUp(task);
    }

    tTaskSchedRdy(task);//����������������

    tTaskExitCritical(status);
}

/* �ӵȴ�������ǿ���Ƴ� */
void tEventRemoveTask(tTask* task, void* msg, uint32_t result)
{
    uint32_t status = tTaskEnterCritical();

    tListRemove(&task->waitEvent->waitList, &task->linkNode);//����ӵȴ�������1�Ƴ�

    task->waitEvent = (tEvent*)0;
    task->eventMsg = msg;
    task->waitEventResult = result;
    task->state &= ~TINYOS_TASK_WAIT_MASK;

    tTaskExitCritical(status);
}

/* ������еȴ��е����񣬷��ػ������� */
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

        if(task->delayTicks != 0)//���볬ʱ�ȴ������ʱ�������Ƴ�
        {
            tTimeTaskWakeUp(task);
        }
        tTaskSchedRdy(task);
    }
    

    tTaskExitCritical(status);
    return count;
}

/* �ӿ�:��ѯ�¼����ƿ��е��������� */
uint32_t tEventWaitCount(tEvent* event)
{
    uint32_t count = 0;

    uint32_t status = tTaskEnterCritical();

    count = tListCount(&event->waitList);

    tTaskExitCritical(status);

    return count;
}
