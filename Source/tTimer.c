#include "tinyOS.h"
#include "tTimer.h"

static tList tTimerHardList;//"硬"件定时器列表
static tList tTimerSoftList;//软件定时器列表 定时器任务1和其他任务访问,中断不允许访问此列表

static tSem tTimerProtectSem;//访问软件定时器的计数信号量
static tSem tTimerTickSem;//软件定时器任务与中断同步的计数信号量

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

/*
** Description: 启动定时器 将定时器任务插入到相应列表中
*/
void tTimerStart(tTimer* timer)
{
    switch (timer->state)
    {
    case tTimerCreated:
    case tTimerStopped:
        timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;//不等于0使用start否则使用duration
        timer->state = tTimerStarted;
        if (timer->config & TIMER_CONFIG_TYPE_HARD)//如果是硬件定时器,加入对应定时器列表
        {
            /* code */
            uint32_t status = tTaskEnterCritical();//在时钟节拍中断中处理,需要进入临界区防护
            tListAddLast(&tTimerHardList, &timer->linkNode);//加入列表中
            tTaskExitCritical(status);
        }
        else
        {
            //软件定时器,先获取信号量,以处理此时定时器任务和在同一时间访问定时器列表的软件定时器而导致的冲突问题
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
** Description: 定时器停止 从列表中移除
*/
void tTimerStop(tTimer* timer)
{
    switch (timer->state)
    {
    case tTimerCreated:
    case tTimerRunning:
        if(timer->config & TIMER_CONFIG_TYPE_HARD)//已经启动,判断定时器类型,从相应延时列表中移除
        {
            uint32_t status = tTaskEnterCritical();
            tListRemove(&tTimerHardList, &timer->linkNode);
            tTaskExitCritical(status);
        }
        else//软件定时器,
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
** Description: 销毁定时器
*/
void tTimerDestroy(tTimer* timer)
{
    tTimerStop(timer);//将定时器从列表中移除

    timer->state = tTimerDestroyed;//状态标记
}

/*
** Description: 定时器查询接口
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
** Description: 功能辅助函数,遍历定时器列表,调用各个定时器的处理函数
*/
static void tTimerCallFuncList(tList* timerList)
{
    tNode* node;

    for (node = timerList->headNode.nextNode; node != &(timerList->headNode); node = node->nextNode)
    {
        tTimer* timer = tNodeParent(node, tTimer, linkNode);

        //延时已到,调用定时器处理函数
        if((timer->delayTicks == 0) || (--timer->delayTicks == 0))
        {
            timer->state = tTimerRunning;//切换为正在运行

            timer->timerFunc(timer->arg);//调用处理函数

            timer->state = tTimerStarted;//切换为已启动状态

            if(timer->durationTicks > 0)//如果是周期性,重复延时计数值
            {
                timer->delayTicks = timer->durationTicks;
            }
            else//一次性定时器,终止
            {
                tListRemove(timerList, &timer->linkNode);
                timer->state = tTimerStopped;
            }
        }
    }
    
}

static tTask tTimerTask;//处理定时器列表的任务
static tTaskStack tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE];

static void tTimerSoftTask(void* param)
{
    for(;;)
    {
        tSemWait(&tTimerTickSem, 0);//等待系统节拍发送的中断事件信号 下面才能恢复运行

        tSemWait(&tTimerProtectSem, 0);//获取定时器列表的访问权限

        tTimerCallFuncList(&tTimerSoftList);//处理软件定时器列表

        tSemNotify(&tTimerProtectSem);//释放访问权限
    }
}

/*
** Description: 定时器模块初始化
*/
void tTimerModuleInit(void)
{
    tListInit(&tTimerHardList);
    tListInit(&tTimerSoftList);
    tSemInit(&tTimerProtectSem, 1, 1);
    tSemInit(&tTimerTickSem, 0, 0);
}

/*
** Description: 定时器任务初始化
*/
void tTimerInitTask(void)
{
#if TINYOS_TIMERTASK_PRIO >= (TINYOS_PRO_COUNT - 1)
    #error "The proprity of timer task must be greater than (TINYOS_PRO_COUNT - 1)"
#endif
    tTaskInit(&tTimerTask, tTimerSoftTask, (void*)0, TINYOS_TIMERTASK_PRIO, tTimerTaskStack, TINYOS_TIMERTASK_STACK_SIZE);
}

/*
** Description: 通知定时器模块,系统tick增加
*/
void tTimerMoudleTickNotify(void)
{
    uint32_t status = tTaskEnterCritical();

    tTimerCallFuncList(&tTimerHardList);//处理硬件定时器列表

    tTaskExitCritical(status);

    tSemNotify(&tTimerTickSem);//通知软定时器节拍变化
}
