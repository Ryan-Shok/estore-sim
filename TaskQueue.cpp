#include "TaskQueue.h"
#include "sthread.h"
#include <queue>

TaskQueue::
TaskQueue()
{
    smutex_init(&mutex);
    scond_init(&cond);
    queueSize = 0;
}

TaskQueue::
~TaskQueue()
{
    smutex_destroy(&mutex);
    scond_destroy(&cond);
}

/*
 * ------------------------------------------------------------------
 * size --
 *
 *      Return the current size of the queue.
 *
 * Results:
 *      The size of the queue.
 *
 * ------------------------------------------------------------------
 */
int TaskQueue::
size()
{
    smutex_lock(&mutex);
    int tmpSize = queueSize;
    smutex_unlock(&mutex);
    return tmpSize;
}

/*
 * ------------------------------------------------------------------
 * empty --
 *
 *      Return whether or not the queue is empty.
 *
 * Results:
 *      true if the queue is empty and false otherwise.
 *
 * ------------------------------------------------------------------
 */
bool TaskQueue::
empty()
{
    smutex_lock(&mutex);
    bool empty = false;
    // check if the queue is empty
    if (queueSize == 0)
    {
        empty = true;
    }
    smutex_unlock(&mutex);
    return empty; // Keep compiler happy until routine done.
}

/*
 * ------------------------------------------------------------------
 * enqueue --
 *
 *      Insert the task at the back of the queue.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void TaskQueue::
enqueue(Task task)
{
    smutex_lock(&mutex);
    // enqueue a task
    queue.push(task);
    // increase the size
    queueSize++;

    // wake any waiters
    scond_broadcast(&cond, &mutex);
    smutex_unlock(&mutex);
}

/*
 * ------------------------------------------------------------------
 * dequeue --
 *
 *      Remove the Task at the front of the queue and return it.
 *      If the queue is empty, block until a Task is inserted.
 *
 * Results:
 *      The Task at the front of the queue.
 *
 * ------------------------------------------------------------------
 */
Task TaskQueue::
dequeue()
{
    smutex_lock(&mutex);
    while (queueSize == 0)
    {
        // wait until there is a task to dequeue
        scond_wait(&cond, &mutex);
    }
    // dequeue the task and return it
    Task task = queue.front();
    queue.pop();
    // decrease the size
    queueSize--;
    
    // wake any waiters
    scond_broadcast(&cond, &mutex);
    smutex_unlock(&mutex);
    return task; // Keep compiler happy until routine done.
}


