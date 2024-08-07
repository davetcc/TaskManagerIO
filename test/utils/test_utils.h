#ifndef TASKMANGERIO_TEST_UTILS_H
#define TASKMANGERIO_TEST_UTILS_H

#include <Arduino.h>
#include <unity.h>
#include <IoLogging.h>

void dumpTasks();

#define MILLIS_ALLOWANCE 2000
#define MICROS_ALLOWANCE 200

extern bool scheduled;
extern bool scheduled2ndJob;
extern unsigned long microsStarted;
extern unsigned long microsExecuted;
extern unsigned long microsExecuted2ndJob;
extern int count1;
extern int count2;
extern uint8_t pinNo;

class EnsureExecutionWithin {
private:
    unsigned long startMillis;
    unsigned long maximumTime;
public:
    explicit EnsureExecutionWithin(unsigned long maxWaiting) {
        startMillis = millis();
        maximumTime = maxWaiting;
    }
    bool ensureTimely() {
        if(millis() > (startMillis + maximumTime)) {
            serdebugF2("TEST EXECUTION OUT OF RANGE ", int(millis() - startMillis));
            return false;
        }

        serdebugF2("Test execution within range: ", int(millis() - startMillis));
        return true;
    }
};

class TimingHelpFixture {
public:
    void setup() {
        taskManager.reset();
        scheduled = scheduled2ndJob = false;
        microsExecuted = microsExecuted2ndJob = 0;
        microsStarted = micros();
        count1 = count2 = 0;
    }

    void assertThatTaskRunsOnTime(uint32_t minExpected, uint32_t allowanceOver) {
        unsigned long startTime = millis();

        // wait until the task is marked as scheduled.
        while(!scheduled && (millis() - startTime) < 5000) {
            taskManager.yieldForMicros(10000);
        }

        // we must have called the job.
        TEST_ASSERT_TRUE(scheduled);

        // print information to help with diagnostics.
        serdebugF4("Scheduled: ", microsStarted, " exec:", microsExecuted);
        unsigned long howLong = microsExecuted - microsStarted;
        serdebugF4(" difference ", howLong, "  expected:", minExpected);

        // check that it has been executed within the allowable window.
        TEST_ASSERT_UINT32_WITHIN(allowanceOver, minExpected, howLong);
    }

    void assertThatSecondJobRan(unsigned long minExpected, unsigned long allowanceOver) {
        // it is assumed that the first job has been waited for and that this should
        // have been run first (ie the shorter task)

        // print information to help with diagnostics.
        serdebugF4("Scheduled: ", microsStarted, " ExecAt: ", microsExecuted2ndJob);
        long howLong = microsExecuted2ndJob - microsStarted;
        serdebugF4(" difference ", howLong, " - expected ", minExpected);

        TEST_ASSERT_TRUE(scheduled2ndJob);
        TEST_ASSERT_UINT32_WITHIN(allowanceOver, minExpected, howLong);
    }

    void assertTasksSpacesTaken(int taken) {
        char sz[32];
        int cnt = 0;
        char* taskData = taskManager.checkAvailableSlots(sz, sizeof sz);
        while(*taskData) {
            if(*taskData != 'F') cnt++;
            ++taskData;
        }
        serdebugF2("Tasks free ", cnt);
        TEST_ASSERT_EQUAL(taken, cnt);
    }
};

void dumpTasks() {
    serdebugF("Dumping the task queue contents");
    TimerTask* task = taskManager.getFirstTask();
    while(task) {
        serdebugF4(" - Task schedule ", task->microsFromNow(), task->isRepeating() ? " Repeating ":" Once ", (task->isMicrosSchedule() ? " Micros " : " Millis "));
        serdebug(task->isInUse() ? " InUse":" Free");
        if(task->getNext() == task) {
            serdebugF("!!!Infinite loop found!!!");
        }
        task = task->getNext();
    }
}

#endif //TASKMANGERIO_TEST_UTILS_H
