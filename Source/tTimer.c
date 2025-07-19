#include "tinyOS.h"
#include "tTimer.h"

void tTimerInit(tTimer* timer, uint32_t delayTicks, uint32_t durationTicks, void (*timerFunc) (void* arg), void* arg, uint32_t config)
{
    tNodeInit(&timer->linkNode);
    timer->startDelayTicks = delayTicks;
    timer->durationTicks = durationTicks;
    timer->timerFunc = timerFunc;
    timer->arg = arg;
    timer->config = config;

    if(delayTicks == 0)//如果初始启动延时为0,则使用周期延时值
    {
        timer->delayTicks = durationTicks;
    }
    else
    {
        timer->delayTicks = timer->startDelayTicks;
    }
    timer->state = tTimerCreated;//状态标定
}