#include "tinyOS.h"

void tTaskDelay(uint32_t delay)
{
	uint32_t status = tTaskEnterCritical();

	tTimeTaskWait(currentTask, delay);//设置延时，插入队列

	tTaskSchedUnRdy(currentTask);//将任务从就绪列表中移除

	tTaskSched();

	tTaskExitCritical(status);
}

