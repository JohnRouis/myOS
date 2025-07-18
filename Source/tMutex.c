#include "tMutex.h"
#include "tinyOS.h"

void tMutexInit(tMutex* mutex)
{
    tEventInit(&mutex->event, tEventTypeMutex);

    mutex->lockedCount = 0;
    mutex->owner = (tTask*)0;
    mutex->ownerOriginalPrio = TINYOS_PRO_COUNT;//最低优先级31,填32为了搞一个无效值
}
