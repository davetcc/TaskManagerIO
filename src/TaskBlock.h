//
// Created by David Cherry on 28/08/2020.
//

#ifndef _TASKMANAGERIO_TASKBLOCK_H_
#define _TASKMANAGERIO_TASKBLOCK_H_

#include <TaskPlatformDeps.h>
#include <TaskTypes.h>

/**
 * This is an internal class, and users of the library generally don't see it.
 *
 * Task manager can never deallocate memory that it has already allocated for tasks, this is in order to make thread
 * safety much easier. Given this we allocate tasks in blocks of DEFAULT_TASK_SIZE and each tranche contains it's start
 * and end point in the "array". DEFAULT task size is set to 20 on 32 bit hardware where the size is negligible, 10
 * on MEGA2560 and all other AVR boards default to 6. We allow up to 8 tranches on AVR and up to 16 on 32 bit boards.
 * This should provide more than enough tasks for most boards.
 */
class TaskBlock {
private:
    TimerTask tasks[DEFAULT_TASK_SIZE];
    const taskid_t first;
    const taskid_t arraySize;
public:
    explicit TaskBlock(taskid_t first_) : first(first_), arraySize(DEFAULT_TASK_SIZE) {}

    /**
     * Checks if taskId is contained within this block
     * @param task the task ID to check
     * @return true if contained, otherwise false
     */
    bool isTaskContained(taskid_t task) const {
        return task >= first && task < (first + arraySize);
    };

    TimerTask* getContainedTask(taskid_t task) {
        return isTaskContained(task) ? &tasks[task - first] : nullptr;
    }

    void clearAll() {
        for(taskid_t i=0; i<arraySize;i++) {
            tasks[i].clear();
        }
    }

    taskid_t allocateTask() {
        for(taskid_t i=0; i<arraySize; i++) {
            if(tasks[i].allocateIfPossible()) {
                return i + first;
            }
        }
        return TASKMGR_INVALIDID;
    }

    taskid_t lastSlot() const {
        return first + arraySize - 1;
    }

    taskid_t firstSlot() const {
        return first;
    }
};


#endif //_TASKMANAGERIO_TASKBLOCK_H_
