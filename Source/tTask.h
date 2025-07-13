#ifndef TTASK_H
#define TTASK_H

//��ʱ״̬
#define TINYOS_TASK_STATE_RDY         0
#define TINYOS_TASK_STATE_DESTROYED  (1 << 0)//ɾ����־λ
#define TINYOS_TASK_STATE_DELAYED    (1 << 1)//��������ʱ״̬
#define TINYOS_TASK_STATE_SUSPEND    (1 << 2)
#define TINYOS_TASK_WAIT_MASK        (0xFF << 16)//�ȴ���־

//cortex-m3��ջ��Ԫ���ͣ���ջ��Ԫ��СΪ32λ
typedef uint32_t tTaskStack;

struct  _tEvent;//ǰ������

typedef struct _tTask
{
    tTaskStack* stack;

    uint32_t delayTicks;//������ʱ����

    tNode linkNode;//�������ӽ��

    tNode delayNode;//��ʱ���

    uint32_t prio;//��������ȼ�

    uint32_t state;//����ǰ״̬

    uint32_t slice;//��ǰʣ��ʱ��Ƭ

    uint32_t suspendCount;//������Ĵ���

    void (*clean) (void* param);//����ɾ��ʱ��������

    void* cleanParam;//���ݸ��������Ĳ���u

    uint8_t requestDeleteFlag;//����ɾ����־ ��0��ʾ����ɾ��

    struct _tEvent* waitEvent;//�������ڵȴ����¼�����
    
    void* eventMsg;//�ȴ��¼��Ĵ洢λ��

    uint32_t waitEventResult;//�ȴ��¼����
}tTask;

typedef struct _tTaskInfo
{
    uint32_t delayTicks;
    uint32_t prio;
    uint32_t state;
    uint32_t slice;
    uint32_t suspendCount;
}tTaskInfo;

void tTaskInit(tTask* task, void (*entry)(void*), void* param, uint32_t prio, uint32_t* stack);

void tTaskSuspend(tTask* task);

void tTaskWakeUp(tTask* task);

void tTaskSetCleanCallFun(tTask* task, void (*clean)(void* paran), void* param);

void tTaskForceDelete(tTask* task);

void tTaskRequestDelete(tTask* task);

uint8_t tTaskIsRequestedDeleted(void);

void tTaskDeleteSelf(void);

void tTaskGetInfo(tTask* task, tTaskInfo* info);

#endif
