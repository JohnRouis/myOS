#include "tinyOS.h"

void tTaskDelay(uint32_t delay)
{
	uint32_t status = tTaskEnterCritical();

	tTimeTaskWait(currentTask, delay);//������ʱ���������

	tTaskSchedUnRdy(currentTask);//������Ӿ����б����Ƴ�

	tTaskSched();

	tTaskExitCritical(status);
}

