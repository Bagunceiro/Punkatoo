#include "ptask.h"
#include "config.h"

PTask::PTask()
{
}

PTask::~PTask()
{
}

void PTask::loop(void *ctlr)
{
    PTask *pThis = reinterpret_cast<PTask *>(ctlr);
    int hwm = uxTaskGetStackHighWaterMark(pThis->taskHandle);
    while (true)
    {
        (*pThis)();
        delay(10);
        int hwmnow = uxTaskGetStackHighWaterMark(pThis->taskHandle);
        if (hwmnow < hwm)
        {
            hwm = hwmnow;
            serr.printf("task HWM = %d\n", hwm);
        }
    }
}

bool PTask::start(uint8_t priority)
{
    xTaskCreate(
        loop,       // Function to implement the task
        "irTask",   // Name of the task
        4000,       // Stack size in words
        this,       // Task input parameter
        priority,   // Priority of the task
        &taskHandle // Task handle.
    );

    return true;
}