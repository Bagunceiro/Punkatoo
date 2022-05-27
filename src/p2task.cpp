#include "p2task.h"
#include "config.h"
#include "eventlog.h"

P2Task::P2Task(const String &n, const int stack)
{
    name = n;
    stackSize = stack;
}

P2Task::~P2Task()
{
}

void P2Task::loop(void *ctlr)
{
    P2Task *pThis = reinterpret_cast<P2Task *>(ctlr);
    int hwm = uxTaskGetStackHighWaterMark(pThis->taskHandle);
    while (true)
    {
        (*pThis)();
        delay(1);
        int hwmnow = uxTaskGetStackHighWaterMark(pThis->taskHandle);
        if (hwmnow < hwm) // log maximum stack depth
        {
            hwm = hwmnow;
            serr.printf("Task %s HWM=%d\n", pThis->name.c_str(), hwm);
            if (hwm < 500) // Arbitrary warning limit
            {
                Event e;
                e.enqueue(String("Task " + pThis->name + " HWM=" + String(hwm)));
            }
        }
    }
}

bool P2Task::start(uint8_t priority)
{
    // serr.println(String("Creating task for ") + name);
    xTaskCreate(
        loop,         // Function to implement the task
        name.c_str(), // Name of the task
        stackSize,    // Stack size in words
        this,         // Task input parameter
        priority,     // Priority of the task
        &taskHandle   // Task handle.
    );

    return true;
}