#ifndef TCONFIG_H
#define TCONFIG_H

#define TINYOS_PRO_COUNT           32
#define TINYOS_SLICE_MAX           10 //ÿ�������������ʱ��Ƭ����

#define TINYOS_IDLETASK_STACK_SIZE 1024

#define TINYOS_TIMERTASK_STACK_SIZE 1024//��ʱ�������ջ��С
#define TINYOS_TIMERTASK_PRIO      1//��ʱ���������ȼ�

#define TINYOS_SYSTICK_MS          10//ʱ�ӽ���

//�ں˹��ܲü�
#define TINYOS_ENABLE_SEM           0
#define TINYOS_ENABLE_MUTEX         0
#define TINYOS_ENABLE_FLAGGROUP     0
#define TINYOS_ENABLE_MBOX          0
#define TINYOS_ENABLE_MEMBLOCK      0
#define TINYOS_ENABLE_TIMER         0
#define TINYOS_ENABLE_CPUUSAGE_STAT 0
#define TINYOS_ENABLE_HOOKS         0

#endif