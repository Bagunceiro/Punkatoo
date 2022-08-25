#pragma once

#include <Arduino.h>

/**
 * @class P2Task
 * @brief Encapsulates an RTOS task
 */
class P2Task
{
public:
    /**
     * @param n A name for reporting purposes
     * @param stack Stack size in bytes
     */
    P2Task(const String &n, const int stack);
    virtual ~P2Task();
    /**
     * @brief Start the task up
     * @param priority The RTOS task priority to be assigned
     * @todo Shift priority defintion to constructor (or stack size to here)?
     */
    bool start(uint8_t priority);
    /**
     * @brief main loop (actual implementation by sub-classes)
     * @return Should process continue (true = yes)? "no" is not yet implented.
     */
    virtual bool operator()() = 0;
    /**
     * @brief loop of the running RTOS task.
     *
     * Repeatedly calls operator() and in between monitors the stack high water mark
     *
     * @param tsk Which P2Task are we talking about
     * @todo implement handling of return from operator()()
     */
    static void loop(void *tsk);

private:
    /** @brief RTOS task handle */
    TaskHandle_t taskHandle;
    /** @brief Name for reporting purposes */
    String name;
    /** @brief size of the stack */
    int stackSize;
};