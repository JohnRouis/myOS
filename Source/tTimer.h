#ifndef TTIMER_H
#define TTIMER_H

#include "tConfig.h"
#include "tEvent.h"

/*
** Description: ��ʱ����״̬
*/
typedef enum _tTimerState
{
    tTimerCreated,//��ʱ���Ѵ���

    tTimerStarted,//��ʱ��������

    tTimerRunning,//��ʱ������ִ�лص�����

    tTimerStopped,//��ʱ����ֹͣ

    tTimerDestroyed,//��ʱ��������
}tTimerState;

typedef struct _tTimer
{
    tNode linkNode;//���ӽ��

    uint32_t startDelayTicks;//�����������ticks��

    uint32_t durationTicks;//���ڶ�ʱ������ticks��

    uint32_t delayTicks;//��ǰ��ʱ���ݼ�����ֵ

    void (*timerFunc) (void* arg);//��ʱ���ص�����

    void* arg;//���ݸ��ص������Ĳ���

    uint32_t config;//��ʱ�����ò���

    tTimerState state;//��ʱ��״̬
}tTimer;

//ѡ�����һ�㻹��Ӳ��һ��Ķ�ʱ��
#define TIMER_CONFIG_TYPE_HARD   (1 << 0)
#define TIMER_CONFIG_TYPE_SOFT   (0 << 0)

void tTimerInit(tTimer* timer, uint32_t delayTicks, uint32_t durationTicks, void (*timerFunc) (void* arg), void* arg, uint32_t config);

void tTimerStart(tTimer* timer);

void tTimerStop(tTimer* timer);

void tTimerModuleInit(void);

void tTimerMoudleTickNotify(void);

#endif
