#include "ThreadPoolWin32.h"
#include <process.h>
#include <stdio.h>

#define Max(a, b)	((a) > (b) ? (a) : (b))

#ifdef _DEBUG

// Unfortunately there is no real API for setting the name of a thread in Windows.
// However, as mentioned in several MSDN articles it's possible to do it through
// the operating system's exception mechanic as described here:
// http://msdn.microsoft.com/en-us/library/windows/desktop/ee416321(v=vs.85).aspx

struct ThreadNameInfo
{
	DWORD	type;	// Must be 0x1000.
	LPCSTR	name;	// Pointer to name (in user address space).
	DWORD	id;		// Thread ID (-1 = caller thread).
	DWORD	flags;	// Reserved for future use, must be zero.

	ThreadNameInfo(LPCSTR n, DWORD i = -1) : type(0x1000), name(n), id(i), flags(0)	{ }
};

static void SetThreadName(LPCSTR name, DWORD id = -1)
{
	ThreadNameInfo info(name, id);
	__try { RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (DWORD*)&info ); }
	__except(EXCEPTION_CONTINUE_EXECUTION) { }
}

// Use names for synchronization objects when running in debug mode.
#define DEBUG_NAME(x)	x

#else // !_DEBUG

#define DEBUG_NAME(x)	0

#endif // !_DEBUG


bool ThreadPool::Init(unsigned numReserved)
{
	do
	{
		// Detect the number of hardware threads that are available, and if we don't
		// have enough it's better for performance if we just fail instead.
		unsigned numCpus = GetNumCpuCores();
		if (numCpus <= Max(numReserved, 1))
			break;

		// Allocate space to hold the queue of thread pointers.
		numThreads = numCpus;
		threads = new WorkThread *[numThreads];
		if (!threads)
			break;

		threadHandles = new HANDLE [numThreads];
		if (!threadHandles)
			break;

		// Allocate synchronization objects.
		emptySlotSemaphore = CreateSemaphore(NULL, numThreads, numThreads, DEBUG_NAME("EmptySlot"));
		workToDoSemaphore = CreateSemaphore(NULL, 0, numThreads, DEBUG_NAME("WorkToDo"));
		exitEvent = CreateEvent(NULL, TRUE, FALSE, DEBUG_NAME("ExitEvent"));
		emptyEvent = CreateEvent(NULL, TRUE, FALSE, DEBUG_NAME("EmptyEvent"));
		if (!emptySlotSemaphore || !workToDoSemaphore || !exitEvent || !emptyEvent)
			break;

		// We need a lock to protect the counters.
		InitializeCriticalSection(&lock);

		// Initialize the threads in a way that is compatible with the C RunTime (ie
		// use _beginthread* instead of CreateThread).
		// The idea is for these jobs to be fairly small both in execution time
		// and memory usage, so it should be enough with a small stack (the
		// internal block size is 32kb).
		unsigned t;
		const unsigned kStackSize = 1 * (32 * 1024);
		for (t = 0; t < numThreads; t++)
		{
			unsigned id;
			threadHandles[t] = (HANDLE)_beginthreadex(NULL, kStackSize, ThreadMain, this, 0, &id);
			if (!threadHandles[t])
				break;

#ifdef _DEBUG
			// Let the thread start up and then give it a name to help when debugging.
			Sleep(1);
			char name[64];
			sprintf_s(name, 64, "Work thread %d", t);
			SetThreadName(name, id);
#endif // _DEBUG
		}

		// We need all the threads to succeed.
		return (t == numThreads);
	}
	while (0);

	Destroy();
	return false;
}

void ThreadPool::Destroy()
{
	// Stop new jobs from being added.
	if (numThreads > 0)
	{
		unsigned _numThreads = numThreads;
		numThreads = 0;

		// Signal that we're about to exit, don't accept new jobs and
		// wait for all work threads to finish.
		if (exitEvent)
			SetEvent(exitEvent);

		// All threads will send an event (with their handle) as they exit.
		WaitForMultipleObjects(_numThreads, threadHandles, TRUE, INFINITE);

		// Free all resources.
		DeleteCriticalSection(&lock);
		CloseHandle(emptySlotSemaphore);
		CloseHandle(workToDoSemaphore);
		CloseHandle(exitEvent);
		CloseHandle(emptyEvent);

		delete [] threads;
		delete [] threadHandles;

		// Clear out the current state.
		Reset();
	}
}

bool ThreadPool::AddToJobQueue(void (f)(int), int p)
{
	// Use a thin class to hold the state data needed for the task.
	// As long as we're not Out Of Memory we should be fine.
	// ToDo: This should be from a data pool instead.
	WorkThread *newJob = new WorkThread(f, p);
	if (!newJob)
		return false;

	// Decrement the counter for empty slots if possible, or wait until
	// once becomes free.
	// ToDo: ? will this fail if we've been signalled to exit ?
	InterlockedIncrement(&numQueuedJobs);
	if (WAIT_OBJECT_0 != WaitForSingleObject(emptySlotSemaphore, INFINITE))
	{
		delete newJob;
		return false;
	}

	Push(newJob);

	// And we're ready to accept the next job.
	return true;
}

void ThreadPool::WaitForJobsToFinish()
{
	WaitForSingleObject(emptyEvent, INFINITE);
}

unsigned ThreadPool::GetNumCpuCores()
{
	// We only want real CPU cores when determining the amount of worker
	// threads to create. Hyper Threading and Simultaneous Multi-Threading
	// share the execution units and caches of one core and can in the best
	// case improve performance by 10-20% if the instruction and memory
	// access patterns are compatible, but can in the worst case seriously
	// impact the performance negatively. The safest assumption is to have
	// one worker thread per CPU core.
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	return systemInfo.dwNumberOfProcessors;
	// ToDo!
}

void ThreadPool::Push(WorkThread *job)
{
	AutoLock autoLock(&lock);

	threads[topIndex++] = job;
	if (topIndex >= numThreads)
		topIndex = 0;

	// Increment the semaphore count.
	ReleaseSemaphore(workToDoSemaphore, 1, NULL);
}

ThreadPool::WorkThread *ThreadPool::Pop()
{
	AutoLock autoLock(&lock);

	WorkThread *job = threads[bottomIndex];
	threads[bottomIndex++] = NULL;
	if (bottomIndex >= numThreads)
		bottomIndex = 0;

	// Increment the semaphore count.
	ReleaseSemaphore(emptySlotSemaphore, 1, NULL);

	return job;
}

unsigned __stdcall ThreadPool::ThreadMain(void *self)
{
	ThreadPool *pool = (ThreadPool *)self;

	HANDLE waitHandles[2];
	waitHandles[0] = pool->workToDoSemaphore;
	waitHandles[1] = pool->exitEvent;

	// Wait for any work to become available (and decrement the count).
	// Also wait for an exit event.
	for (;;)
	{
		switch (WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE))
		{
		case WAIT_OBJECT_0 + 0:
			// Get the next job.
			pool->Pop()->Process();
			InterlockedDecrement(&pool->numQueuedJobs);
			if (pool->numQueuedJobs == 0)
				SetEvent(pool->emptyEvent);
			break;

		case WAIT_OBJECT_0 + 1:
			// Exit event was received.
			return 0;

		default:
			// ToDo: try to signal error?
			// Exit thread with error code.
			return 1;
		}
	}
}
