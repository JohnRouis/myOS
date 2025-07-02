#include "tinyOS.h"

void tEventInit(tEvent* event, tEventType type)
{
    event->type = tEventTypeUnkonwn;
    tListInit(&event->waitList);
}
