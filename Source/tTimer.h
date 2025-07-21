#ifndef TTIMER_H
#define TTIMER_H

#include "tConfig.h"
#include "tEvent.h"

/*
** Description: 定时器的状态
*/
typedef enum _tTimerState
{
    tTimerCreated,//定时器已创建

    tTimerStarted,//定时器已启动

    tTimerRunning,//定时器正在执行回调函数

    tTimerStopped,//定时器已停止

    tTimerDestroyed,//定时器已销毁
}tTimerState;

typedef struct _tTimer
{
    tNode linkNode;//链接结点

    uint32_t startDelayTicks;//初次启动后的ticks数

    uint32_t durationTicks;//周期定时的周期ticks数

    uint32_t delayTicks;//当前定时器递减计数值

    void (*timerFunc) (void* arg);//定时器回调函数

    void* arg;//传递给回调函数的参数

    uint32_t config;//定时器配置参数

    tTimerState state;//定时器状态
}tTimer;

//选择软件一点还是硬件一点的定时器
#define TIMER_CONFIG_TYPE_HARD   (1 << 0)
#define TIMER_CONFIG_TYPE_SOFT   (0 << 0)

void tTimerInit(tTimer* timer, uint32_t delayTicks, uint32_t durationTicks, void (*timerFunc) (void* arg), void* arg, uint32_t config);

void tTimerStart(tTimer* timer);

void tTimerStop(tTimer* timer);

void tTimerModuleInit(void);

void tTimerMoudleTickNotify(void);

#endif
