#include "tFlagGroup.h"
#include "tinyOS.h"

void tFlagGroupInit(tFlagGroup* flagGroup, uint32_t flags)
{
    tEventInit(&flagGroup->event, tEventTypeFlagGroup);
    flagGroup->flags = flags;
}

/* 
** Description: ��������,�����鲢�����¼���־
*/
static uint32_t tFlagGroupCheckAndConsume(tFlagGroup* flagGroup, uint32_t type, uint32_t* flags)
{
    //�Ƚ���������
    uint32_t srcFlags = *flags;
    uint32_t isSet = type & TFLAGGROUP_SET;//��1������0
    uint32_t isAll = type & TFLAGGROUP_ALL;//���ֻ���ȫ��
    uint32_t isConsume = type & TFLAGGROUP_CONSUME;

    uint32_t calcFlag = isSet ? (flagGroup->flags & srcFlags) : (~flagGroup->flags & srcFlags);

    if(((isAll != 0) && (calcFlag == srcFlags)) || ((isAll == 0) && (calcFlag != 0)))
    {
        if(isConsume)
        {
            if(isSet)
            {
                //���Ϊ1�ı�־λ
                flagGroup->flags &= ~srcFlags;
            }
            else
            {
                //���Ϊ0�ı�־λ
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
** Description: �ȴ��¼���־��
*/
uint32_t tFlagGroupWait(tFlagGroup* flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t* resultFlag, uint32_t waitTicks)
{
    uint32_t result;
    uint32_t flags = requestFlag;

    uint32_t status = tTaskEnterCritical();
    result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);//����־λ����1��0
    if(result != tErrorNoError)//����������,������еȴ�
    {
        currentTask->waitFlagsType = waitType;
        currentTask->eventFlags = requestFlag;
        tEventWait(&flagGroup->event, currentTask, (void*)0, tEventTypeFlagGroup, waitTicks);

        tTaskExitCritical(status);

        tTaskSched();//��������һ�ε���

        *resultFlag = currentTask->eventFlags;//�������¼ֵ
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
** Description: ��ȡ�¼���־,���ȴ���
*/
uint32_t tFlagGroupNoWaitGet(tFlagGroup* flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t* resultFlag)
{
    uint32_t flags = requestFlag;

    uint32_t status = tTaskEnterCritical();

    uint32_t result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);//ֱ�ӻ�ȡ,�о���,û�о�ֱ����

    tTaskExitCritical(status);

    *resultFlag = flags;
    return status;
}

/*
** Description: ֪ͨ�¼���־���е��������±�־����
*/
void tFlagGroupNotify(tFlagGroup* flagGroup, uint8_t isSet, uint32_t flags)
{
    tList* waitList;
    tNode* node;
    tNode* nextNode;
    uint8_t sched = 0;//���ȱ�־λ

    uint32_t status = tTaskEnterCritical();

    if(isSet)
    {
        flagGroup->flags |= flags;//��1
    }
    else
    {
        flagGroup->flags &= ~flags;//��0
    }
    //������������,��ȡ��������������,���뵽�Ƴ��б���
    waitList = &flagGroup->event.waitList;
    for(node = waitList->headNode.nextNode; node != &(waitList->headNode); node = nextNode)
    {
        uint32_t result;
        tTask* task = tNodeParent(node, tTask, linkNode);
        uint32_t flags = task->eventFlags;
        nextNode = node->nextNode;
        //����־
        result = tFlagGroupCheckAndConsume(flagGroup, task->waitFlagsType, &flags);
        if(result == tErrorNoError)//�����˾ͻ���
        {
            task->eventFlags = flags;
            tEventWakeUpTask(&flagGroup->event, task, (void*)0, tErrorNoError);
            sched = 1;
        }
    }
    if(sched == 1)//��Ҫ����
    {
        tTaskSched();
    }
    tTaskExitCritical(status);
}

/*
** Description: ��ѯ�¼���־���״̬��Ϣ
*/
void tFlagGroupGetInfo(tFlagGroup* flagGroup, tFlagGroupInfo* info)
{
    uint32_t status = tTaskEnterCritical();

    info->flags = flagGroup->flags;
    info->taskCount = tEventWaitCount(&flagGroup->event);

    tTaskExitCritical(status);
}

/*
** Description: �����¼���־��
*/
uint32_t tFlagGroupDestroy(tFlagGroup* flagGroup)
{
    uint32_t status = tTaskEnterCritical();

    uint32_t count = tEventRemoveAll(&flagGroup->event, (void*)0, tErrorDel);

    tTaskExitCritical(status);

    if(count > 0)//������������ھ���״̬,ִ�е���
    {
        tTaskSched();
    }
    return count;
}
