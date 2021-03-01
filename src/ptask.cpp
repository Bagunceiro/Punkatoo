#include "ptask.h"
#include "config.h"
#include "eventlog.h"

PTask::PTask(const String& n, const int stack)
{
    name = n;
    stackSize = stack;
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
        delay(1);
        int hwmnow = uxTaskGetStackHighWaterMark(pThis->taskHandle);
        if (hwmnow < hwm) // log maximum stack depth
        {
            hwm = hwmnow;
            Event e;
            e.enqueue(String("Task " + pThis->name + " stack free " + String(hwm)));
        }
    }
}

bool PTask::start(uint8_t priority)
{
    serr.println(String("Creating task for ") + name);
    xTaskCreate(
        loop,          // Function to implement the task
        name.c_str(),  // Name of the task
        stackSize,     // Stack size in words
        this,          // Task input parameter
        priority,      // Priority of the task
        &taskHandle    // Task handle.
    );

    return true;
}