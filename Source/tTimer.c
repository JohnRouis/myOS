#include "tinyOS.h"
#include "tTimer.h"

static tList tTimerHardList;//"Ӳ"����ʱ���б�
static tList tTimerSoftList;//�����ʱ���б� ��ʱ������1�������������,�жϲ�������ʴ��б�

static tSem tTimerProtectSem;//���������ʱ���ļ����ź���
static tSem tTimerTickSem;//�����ʱ���������ж�ͬ���ļ����ź���

void tTimerInit(tTimer* timer, uint32_t delayTicks, uint32_t durationTicks, void (*timerFunc) (void* arg), void* arg, uint32_t config)
{
    tNodeInit(&timer->linkNode);
    timer->startDelayTicks = delayTicks;
    timer->durationTicks = durationTicks;
    timer->timerFunc = timerFunc;
    timer->arg = arg;
    timer->config = config;

    if(delayTicks == 0)//�����ʼ������ʱΪ0,��ʹ��������ʱֵ
    {
        timer->delayTicks = durationTicks;
    }
    else
    {
        timer->delayTicks = timer->startDelayTicks;
    }
    timer->state = tTimerCreated;//״̬�궨
}

/*
** Description: ������ʱ�� ����ʱ��������뵽��Ӧ�б���
*/
void tTimerStart(tTimer* timer)
{
    switch (timer->state)
    {
    case tTimerCreated:
    case tTimerStopped:
        timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;//������0ʹ��start����ʹ��duration
        timer->state = tTimerStarted;
        if (timer->config & TIMER_CONFIG_TYPE_HARD)//�����Ӳ����ʱ��,�����Ӧ��ʱ���б�
        {
            /* code */
            uint32_t status = tTaskEnterCritical();//��ʱ�ӽ����ж��д���,��Ҫ�����ٽ�������
            tListAddLast(&tTimerHardList, &timer->linkNode);//�����б���
            tTaskExitCritical(status);
        }
        else
        {
            //�����ʱ��,�Ȼ�ȡ�ź���,�Դ����ʱ��ʱ���������ͬһʱ����ʶ�ʱ���б�������ʱ�������µĳ�ͻ����
            tSemWait(&tTimerProtectSem, 0);
            tListAddLast(&tTimerSoftList, &timer->linkNode);
            tSemNotify(&tTimerProtectSem);
        }
        break;
    
    default:
        break;
    }
}

/*
** Description: ��ʱ��ֹͣ ���б����Ƴ�
*/
void tTimerStop(tTimer* timer)
{
    switch (timer->state)
    {
    case tTimerCreated:
    case tTimerRunning:
        if(timer->config & TIMER_CONFIG_TYPE_HARD)//�Ѿ�����,�ж϶�ʱ������,����Ӧ��ʱ�б����Ƴ�
        {
            uint32_t status = tTaskEnterCritical();
            tListRemove(&tTimerHardList, &timer->linkNode);
            tTaskExitCritical(status);
        }
        else//�����ʱ��,
        {
            tSemWait(&tTimerProtectSem, 0);
            tListRemove(&tTimerSoftList, &timer->linkNode);
            tSemNotify(&tTimerProtectSem);
        }
        timer->state = tTimerStopped;
        break;
    
    default:
        break;
    }
}

/*
** Description: ���ٶ�ʱ��
*/
void tTimerDestroy(tTimer* timer)
{
    tTimerStop(timer);//����ʱ�����б����Ƴ�

    timer->state = tTimerDestroyed;//״̬���
}

/*
** Description: ��ʱ����ѯ�ӿ�
*/
void tTimerGetInfo(tTimer* timer, tTimerInfo* info)
{
    uint32_t status = tTaskEnterCritical();

    info->startDelayTicks = timer->startDelayTicks;
    info->durationTicks = timer->durationTicks;
    info->timerFunc = timer->timerFunc;
    info->arg = timer->arg;
    info->config = timer->config;
    info->state = timer->state;

    tTaskExitCritical(status);
}

/*
** Description: ���ܸ�������,������ʱ���б�,���ø�����ʱ���Ĵ�����
*/
static void tTimerCallFuncList(tList* timerList)
{
    tNode* node;

    for (node = timerList->headNode.nextNode; node != &(timerList->headNode); node = node->nextNode)
    {
        tTimer* timer = tNodeParent(node, tTimer, linkNode);

        //��ʱ�ѵ�,���ö�ʱ��������
        if((timer->delayTicks == 0) || (--timer->delayTicks == 0))
        {
            timer->state = tTimerRunning;//�л�Ϊ��������

            timer->timerFunc(timer->arg);//���ô�����

            timer->state = tTimerStarted;//�л�Ϊ������״̬

            if(timer->durationTicks > 0)//�����������,�ظ���ʱ����ֵ
            {
                timer->delayTicks = timer->durationTicks;
            }
            else//һ���Զ�ʱ��,��ֹ
            {
                tListRemove(timerList, &timer->linkNode);
                timer->state = tTimerStopped;
            }
        }
    }
    
}

static tTask tTimerTask;//����ʱ���б������
static tTaskStack tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE];

static void tTimerSoftTask(void* param)
{
    for(;;)
    {
        tSemWait(&tTimerTickSem, 0);//�ȴ�ϵͳ���ķ��͵��ж��¼��ź� ������ָܻ�����

        tSemWait(&tTimerProtectSem, 0);//��ȡ��ʱ���б�ķ���Ȩ��

        tTimerCallFuncList(&tTimerSoftList);//���������ʱ���б�

        tSemNotify(&tTimerProtectSem);//�ͷŷ���Ȩ��
    }
}

/*
** Description: ��ʱ��ģ���ʼ��
*/
void tTimerModuleInit(void)
{
    tListInit(&tTimerHardList);
    tListInit(&tTimerSoftList);
    tSemInit(&tTimerProtectSem, 1, 1);
    tSemInit(&tTimerTickSem, 0, 0);
}

/*
** Description: ��ʱ�������ʼ��
*/
void tTimerInitTask(void)
{
#if TINYOS_TIMERTASK_PRIO >= (TINYOS_PRO_COUNT - 1)
    #error "The proprity of timer task must be greater than (TINYOS_PRO_COUNT - 1)"
#endif
    tTaskInit(&tTimerTask, tTimerSoftTask, (void*)0, TINYOS_TIMERTASK_PRIO, tTimerTaskStack, TINYOS_TIMERTASK_STACK_SIZE);
}

/*
** Description: ֪ͨ��ʱ��ģ��,ϵͳtick����
*/
void tTimerMoudleTickNotify(void)
{
    uint32_t status = tTaskEnterCritical();

    tTimerCallFuncList(&tTimerHardList);//����Ӳ����ʱ���б�

    tTaskExitCritical(status);

    tSemNotify(&tTimerTickSem);//֪ͨ��ʱ�����ı仯
}
