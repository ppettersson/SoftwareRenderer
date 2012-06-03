#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <windows.h>

class ThreadPool
{
public:
	static ThreadPool &GetInstance()							{ static ThreadPool instance; return instance; }

	// Try to initialize the thread pool with as many threads as possible.
	// Only actual cpu cores are used to determine the number of threads.
	// If not enough hardware thread are found to justify a thread pool this
	// call will fail and if this class is used it'll act as a simple conduit
	// that runs each added job serially instead with a minimal impact on
	// performance.
	//
	// numReserved can be used to limit the number of worker thread. This can
	// be useful if one or more threads are already allocated and are expected
	// to run at full capacity for the length of the program.
	bool Init(unsigned numReserved = 0);

	// Stop accepting new jobs. Wait for all running jobs to finish and then
	// shut down the thread pool.
	void Destroy();

	// Run a workload through the thread pool if possible or fall back to
	// running it directly if not.
	__forceinline void Run(void (func)(int), int param)
	{
		// Try to add this task to the job queue.
		if (!numThreads || !AddToJobQueue(func, param))
		{
			// Fall back to run it serially instead.
			func(param);
		}
	}

	// Wait for all currently running workloads to finish before returning.
	__forceinline void Wait()
	{
		// Only wait if the thread pool is running.
		if (numThreads)
			WaitForJobsToFinish();
	}
	
private:
	struct WorkThread
	{
		void	(*func)(int);									// Callback function.
		int		param;											// Parameter to the callback function.

		WorkThread(void (f)(int), int p) : func(f), param(p)	{ }
		virtual void Process()									{ func(param); 	delete this; }
	};

	// Convenience class to automatically unlock when going out of scope.
	struct AutoLock
	{
		CRITICAL_SECTION *lock;

		AutoLock(CRITICAL_SECTION *l) : lock(l)					{ EnterCriticalSection(lock); }
		~AutoLock()												{ LeaveCriticalSection(lock); }
	};


	// Active state.
	unsigned			numThreads;								// Number of available hardware threads in the pool
	volatile LONG		numQueuedJobs;							// Number of currently queued jobs that are either in flight or waiting to be started.
	int					topIndex;								// ?
	int					bottomIndex;							// ?
	WorkThread			**threads;								// Currently active jobs.
	HANDLE				*threadHandles;							// Handles for each thread.

	// Synchronization.
	HANDLE				emptySlotSemaphore;						// ?
	HANDLE				workToDoSemaphore;						// ?
	HANDLE				exitEvent;								// Send from the pool to all worker threads to not accept any new jobs and terminate thread.
	HANDLE				emptyEvent;								// Sent from the worker thread that decrements numQueuedJobs to zero to the pool.
	CRITICAL_SECTION	lock;									// Used to synchronize access to the active state.


	ThreadPool()												{ Reset(); }
	~ThreadPool()												{ Destroy(); }

	void Reset()												{ ZeroMemory(this, sizeof(*this)); }

	unsigned GetNumCpuCores();

	bool AddToJobQueue(void (f)(int), int p);
	void WaitForJobsToFinish();

	void Push(WorkThread *job);
	WorkThread *Pop();

	static unsigned __stdcall ThreadMain(void *self);
};

#endif // THREAD_POOL_H
