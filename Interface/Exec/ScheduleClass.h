/*=+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * File: ScheduleClass.h
 *
 * ScheduleClass is the interface for dividing the scheduler into seperate
 * scheduler classes, namely - RT, CFS, RR.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef EXEC_SCHEDULECLASS_H
#define EXEC_SCHEDULECLASS_H

#include <Exec/KTask.h>
#include <Synch/Spinlock.h>
#include <Util/Stack.h>

namespace HAL
{
	struct Domain;
}

/**
 * KSCHED_ROLLER  -
 *
 * Summary:
 * The kernel scheduler roller (or KSCHED_ROLLER) is a type used for interfacing
 * with various scheduler classes. It contains the functions (ptr) for operating with
 * the scheduler. It is present on every CPU.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
struct _KSCHED_ROLLER {
	unsigned long RunnerLoad;
	SPIN_LOCK RunqueueLock;

	STACK RqTransferBuffer;
	unsigned long RqLoadRequired;
	unsigned long RqTransferDiff;
	
	KRUNNABLE *(*ScheduleRunner)(
		TIME curTime,
		struct Processor *
	);/* Actual scheduling algorithm*/

	void *(*SaveRunner)(
		TIME curTime,
		struct Processor *
	);/* Save state for switching to other scheduler */

	/* Clock-tick occured, thus, update the runner (if required) */
	KRUNNABLE *(*UpdateRunner)(TIME curTime, struct Processor *);

	/* Used for inserting a new runner */
	void (*InsertRunner) (KRUNNABLE *runner, struct Processor *);

	/* Used for removing a runner */
	void (*RemoveRunner) (KRUNNABLE *runner);

	/* Used for transfering of load (SMP). */
	void (*TransferRunners) (struct _KSCHED_ROLLER *roller); 
} KSCHED_ROLLER;

typedef
struct
{
	unsigned int DomainLoad;
	TIME BalanceInterval;
} KSCHED_ROLLER_DOMAIN;

namespace Executable
{
class RunqueueBalancer;
typedef unsigned long ScheduleClass;

enum
{
	ROUND_ROBIN = 0,
	COMPLETELY_FAIR = 1
};

/**
 * Class: ScheduleRoller
 *
 * Summary:
 * Rolls schedules by allocating tasks to the core-scheduler. It is basically
 * a interface that a scheduler-module must implement to extend the base
 * scheduling mechanism. It is stored on a per-cpu basis which requires
 * seperate runqueues per-processor.
 *
 * Functions:
 * allocate - get any task to run on the cpu
 * free - take-back a task after a time-interval of executing it
 * add - spawn a new task in the system
 * remove - destroy the task from the system
 * transfer - move tasks to the other roller with the loaded transfer-config
 *
 * Author: Shukant Pal
 */
class ScheduleRoller
{
public:
	unsigned long getLoad()
	{
		return (load);
	}

	void request(unsigned long rload)
	{
		loadRequested = rload;
	}

	/*
	 * Adds the unhosted task into the processor's runqueue in the
	 * scheduling class. It can be called from outside a interrupt
	 * context.
	 */
	virtual KTask *add(KTask *newTask) = 0;

	/*
	 * Rolls the next task that should be executed when this class
	 * was picked for scheduling (and the previous one was diff.)
	 */
	virtual KTask *allocate(TIME t, Processor *cpu) = 0;

	/*
	 * Rolls the next task assuming that the previous class of tasks
	 * running was the same.
	 */
	virtual KTask *update(TIME t, Processor *cpu) = 0;

	/*
	 * Takes the tasks running back into the runqueue, allowing a
	 * diff. scheduling class to execute now.
	 */
	virtual void free(TIME at, Processor *cpu) = 0;

	/*
	 * Removes the task from the runqueue permanently allowing it to be
	 * shifted or destroyed. This can be called outside of a interrupt
	 * context.
	 */
	virtual void remove(KTask *tTask) = 0;

	/*
	 * Sends a 'Accept' request to 'proc' with a list of tasks which (on
	 * transferring) can balancing the load b/w the client & donor domains
	 * (which should be under the same parent & this cpu should come under
	 * the donor).
	 */
	virtual void send(Processor *proc, CircularList &list, unsigned long delta) = 0;

	virtual void *transfer(ScheduleRoller& idle) = 0;

	/*
	 * Take a part of the list of tasks given, cutting off the given load
	 * from the chain. This is due to the fact that it is possible -
	 * load != no. of tasks
	 *
	 * It returns the task after that last one taken by the cpu.
	 */
	virtual KTask *recieve(KTask *first, KTask *last, unsigned long load) = 0;
protected:
	ScheduleRoller();
	virtual ~ScheduleRoller();
public:
	unsigned long load;// current load
	unsigned long loadRequested;// requested-load for transfer
	unsigned long transferDelta;// delta on transfer
	Stack transferBuffer;// buffer for incoming tasks
	Spinlock lock;

	friend class RunqueueBalancer;
};

struct ScheduleDomain
{
	long load;// task-load for the domain
	TIME balanceDelta;// timestamp for the next balance

	ScheduleDomain()
	{
		load = 0;
		balanceDelta = 0;
	}
};

}

#endif /* Exec/ScheduleClass.h */
