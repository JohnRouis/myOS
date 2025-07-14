#include "tMbox.h"
#include "tinyOS.h"

/* �����ʼ�� */
void tMboxInit(tMbox* mbox, void** msgBuffer, uint32_t maxCount)
{
    tEventInit(&mbox->event, tEventTypeMbox);

    mbox->msgBuffer = msgBuffer;
    mbox->maxCount = maxCount;
    mbox->read = 0;
    mbox->write = 0;
    mbox->count = 0;
}

/* 
** �ȴ�����,��ȡһ����Ϣ 
** parameter:msg ��Ϣ������
*/
uint32_t tMboxWait(tMbox* mbox, void** msg, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    if(mbox->count > 0)//����Ϣ��������0��
    {
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];//ȡ��һ��Ȼ����������Ų

        if(mbox->read >= mbox->maxCount)//�߽����
        {
            mbox->read = 0;
        }
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else
    {
        tEventWait(&mbox->event, currentTask, (void*)0, tEventTypeMbox, waitTicks);//û�оͽ��������ȴ�������
        tTaskExitCritical(status);

        tTaskSched();//����һ��,������������

        *msg = currentTask->eventMsg;//�����������Ϣ����,�ͻ����»ص��������λ��,���ʱ�����Ҫ���ʼ���Ϣ���

        return currentTask->waitEventResult;

    }
}

/*
** descripttion: ����û��Ϣʱѡ�񲻵ȴ�
** 
*/
uint32_t tMboxNoWaitGet(tMbox* mbox, void** msg)
{
    uint32_t status = tTaskEnterCritical();

    if(mbox->count > 0)
    {
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];

        if(mbox->read >= mbox->maxCount)
        {
            mbox->read = 0;
        }
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else//û�еĻ�ֱ�ӷ�����Դ���ɵ�����ʾ
    {
        tTaskExitCritical(status);
        return tErrorResourceUnavaliable;
    }
}
/*
** Description: ����֪ͨ����,֪ͨ����Ϣ����,���ѵȴ������е�һ������,���߽���Ϣ���뵽������
*/
uint32_t tMboxNotify(tMbox* mbox, void* msg, uint32_t notifyOption)
{
    uint32_t status = tTaskEnterCritical();

    if(tEventWaitCount(&mbox->event) > 0)//����Ƿ�������ȴ�
    {
        tTask* task = tEventWakeUp(&mbox->event, (void*)msg, tErrorNoError);//�оͻ����ײ�����

        if(task->prio < currentTask->prio)//��������������ȼ���������ȹ�ȥ
        {
            tTaskSched();
        }
    }
    else//û������ȴ�,����Ϣ���뵽��������
    {
        if(mbox->count >= mbox->maxCount)
        {
            tTaskExitCritical(status);
            return tErrorResourceFull;
        }
        if(notifyOption & tMBOXSendFront)
        {
            if(mbox->read <= 0)
            {
                mbox->read = mbox->maxCount - 1;//���νṹ����
            }
            else
            {
                --mbox->read;
            }
            mbox->msgBuffer[mbox->read] = msg;
        }
        else
        {
            mbox->msgBuffer[mbox->write++] = msg;
            if(mbox->write >= mbox->maxCount)
            {
                mbox->write = 0;
            }
        }
        mbox->count++;
    }
    tTaskExitCritical(status);
    return tErrorNoError;
}

/*
** Description: ���������Ϣ
*/
void tMboxFlush(tMbox* mbox)
{
    uint32_t status = tTaskEnterCritical();

    if(tEventWaitCount(&mbox->event) == 0)//û������ȴ�����Ҫ�ж�һ�£������������ȴ���˵�������Ѿ��ǿ���
    {
        mbox->read = 0;
        mbox->write = 0;
        mbox->count = 0;
    }

    tTaskExitCritical(status);
}

/*
**Description: ��������
*/
uint32_t  tMboxDestroy(tMbox* mbox)
{
    uint32_t status = tTaskEnterCritical();
    //����¼����ƿ���������
    uint32_t count = tEventRemoveAll(&mbox->event, (void*)0, tErrorDel);
    tTaskExitCritical(status);

    if(count > 0)
    {
        tTaskSched();
    }
    return count;
}

/*
** Description:��ѯ״̬��Ϣ
*/
void tMboxGetInfo(tMbox* mbox, tMboxInfo* info)
{
    uint32_t status = tTaskEnterCritical();

    info->count = mbox->count;
    info->maxCount = mbox->maxCount;
    info->taskCount = tEventWaitCount(&mbox->event);

    tTaskExitCritical(status);
}
