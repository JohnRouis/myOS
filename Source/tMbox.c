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
