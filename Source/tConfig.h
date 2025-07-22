#ifndef TCONFIG_H
#define TCONFIG_H

#define TINYOS_PRO_COUNT           32
#define TINYOS_SLICE_MAX           10 //每个任务最大运行时间片计数

#define TINYOS_IDLETASK_STACK_SIZE 1024

#define TINYOS_TIMERTASK_STACK_SIZE 1024//定时器任务堆栈大小
#define TINYOS_TIMERTASK_PRIO      1//定时器任务优先级

#define TINYOS_SYSTICK_MS          10//时钟节拍

//内核功能裁剪
#define TINYOS_ENABLE_SEM           0
#define TINYOS_ENABLE_MUTEX         0
#define TINYOS_ENABLE_FLAGGROUP     0
#define TINYOS_ENABLE_MBOX          0
#define TINYOS_ENABLE_MEMBLOCK      0
#define TINYOS_ENABLE_TIMER         0
#define TINYOS_ENABLE_CPUUSAGE_STAT 0
#define TINYOS_ENABLE_HOOKS         0

#endif