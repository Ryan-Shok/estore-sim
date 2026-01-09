#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "EStore.h"
#include "TaskQueue.h"
#include "sthread.h"
#include "RequestGenerator.h"


class Simulation {
public:
    TaskQueue supplierTasks;
    TaskQueue customerTasks;
    EStore store;

    int maxTasks;
    int numSuppliers;
    int numCustomers;
    bool fineMode;

    explicit Simulation(bool useFineMode) : store(useFineMode) { }
};

/*
 * ------------------------------------------------------------------
 * supplierGenerator --
 *
 *      The supplier generator thread. The argument is a pointer to
 *      the shared Simulation object.
 *
 *      Enqueue arg->maxTasks requests to the supplier queue, then
 *      stop all supplier threads by enqueuing arg->numSuppliers
 *      stop requests.
 *
 *      Use a SupplierRequestGenerator to generate and enqueue
 *      requests.
 *
 *      This thread should exit when done.
 *
 * Results:
 *      Does not return. Exit instead.
 *
 * ------------------------------------------------------------------
 */
static void*
supplierGenerator(void* arg)
{
    // create a new supplier request generator from the provided simulator
    Simulation* sim = ((Simulation*)arg);
    SupplierRequestGenerator supplyGen(&(sim->supplierTasks));

    // enqueue the max amount of tasks and thread stoppers
    supplyGen.enqueueTasks(sim->maxTasks, &(sim->store));
    supplyGen.enqueueStops(sim->numSuppliers);
    sthread_exit();
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * customerGenerator --
 *
 *      The customer generator thread. The argument is a pointer to
 *      the shared Simulation object.
 *
 *      Enqueue arg->maxTasks requests to the customer queue, then
 *      stop all customer threads by enqueuing arg->numCustomers
 *      stop requests.
 *
 *      Use a CustomerRequestGenerator to generate and enqueue
 *      requests.  For the fineMode argument to the constructor
 *      of CustomerRequestGenerator, use the output of
 *      store.fineModeEnabled() method, where store is a field
 *      in the Simulation class.
 *
 *      This thread should exit when done.
 *
 * Results:
 *      Does not return. Exit instead.
 *
 * ------------------------------------------------------------------
 */
static void*
customerGenerator(void* arg)
{
    // create a new customer request generator object from the provided simulation
    Simulation* sim = ((Simulation*)arg);
    CustomerRequestGenerator customerGen(&(sim->customerTasks), sim->store.fineModeEnabled());

    // enqueue the max amounts of tasks and thread stoppers
    customerGen.enqueueTasks(sim->maxTasks, &(sim->store));
    customerGen.enqueueStops(sim->numCustomers);
    sthread_exit();
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * supplier --
 *
 *      The main supplier thread. The argument is a pointer to the
 *      shared Simulation object.
 *
 *      Dequeue Tasks from the supplier queue and execute them.
 *
 * Results:
 *      Does not return.
 *
 * ------------------------------------------------------------------
 */
static void*
supplier(void* arg)
{
    Simulation* sim = ((Simulation*)arg);

    // for each of these supplier threads dequeue a task and handle it
    Task task = sim->supplierTasks.dequeue();
    task.handler(task.arg);
    sthread_exit();
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * customer --
 *
 *      The main customer thread. The argument is a pointer to the
 *      shared Simulation object.
 *
 *      Dequeue Tasks from the customer queue and execute them.
 *
 * Results:
 *      Does not return.
 *
 * ------------------------------------------------------------------
 */
static void*
customer(void* arg)
{
    Simulation* sim = ((Simulation*)arg);

    // for each of these customer threads dequeue a task and handle it
    Task task = sim->customerTasks.dequeue();
    task.handler(task.arg);
    sthread_exit();
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * startSimulation --
 *      Create a new Simulation object. This object will serve as
 *      the shared state for the simulation.
 *
 *      Create the following threads:
 *          - 1 supplier generator thread.
 *          - 1 customer generator thread.
 *          - numSuppliers supplier threads.
 *          - numCustomers customer threads.
 *
 *      After creating the worker threads, the main thread
 *      should wait until all of them exit, at which point it
 *      should return.
 *
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
static void
startSimulation(int numSuppliers, int numCustomers, int maxTasks, bool useFineMode)
{
    // initialize the simulation
    Simulation sharedSim(useFineMode);
    sharedSim.numSuppliers = numSuppliers;
    sharedSim.numCustomers = numCustomers;
    sharedSim.maxTasks = maxTasks;
    sharedSim.fineMode = useFineMode;

    // create generator threads
    sthread_t supplierGen;
    sthread_t customerGen;
    sthread_create(&supplierGen, supplierGenerator, &sharedSim);
    sthread_create(&customerGen, customerGenerator, &sharedSim);

    // create worker threads
    sthread_t supplierArr[numSuppliers];
    sthread_t customerArr[numCustomers];
    for (int i = 0; i < numSuppliers; i++)
    {
        sthread_t supplierThread;
        sthread_create(&supplierThread, supplier, &sharedSim);
        supplierArr[i] = supplierThread;
    }
    for (int i = 0; i < numCustomers; i++)
    {
        sthread_t customerThread;
        sthread_create(&customerThread, customer, &sharedSim);
        customerArr[i] = customerThread;
    }

    // join the threads to wait for their completions
    sthread_join(supplierGen);
    sthread_join(customerGen);
    for (int i = 0; i < numSuppliers; i++)
    {
        sthread_join(supplierArr[i]);
    }
    for (int i = 0; i < numCustomers; i++)
    {
        sthread_join(customerArr[i]);
    }

}

int main(int argc, char** argv)
{
    bool useFineMode = false;

    // Seed the random number generator.
    srand(time(NULL));

    if (argc > 1)
        useFineMode = strcmp(argv[1], "--fine") == 0;
    startSimulation(10, 10, 100, useFineMode);
    return 0;
}
