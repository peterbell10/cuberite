// ThreadPool.h

// Declares the cThreadPool class representing a task scheduler with a dynamically sized pool of threads. 

#pragma once


#include <functional>
#include <mutex>
#include <thread>

#include "Queue.h"






class cThreadPool
{
public:

	using cWork = std::function<void()>;

	cThreadPool(size_t a_InitialSize, AString a_PoolName);
	~cThreadPool();

	/** Blocks until all remaining tasks have been assigned to a thread. */
	void WaitForFinish();

	/** Block until all threads have been stopped. */
	void JoinAll() { Resize(0); }
	
	/** Submit a new task to be executed in the pool. */
	bool Submit(cWork a_Work);

	/** Returns true if there are no threads in the pool. */
	bool Empty() const { return (Size() == 0); }

	/** Returns the current number of threads in the pool. */
	size_t Size() const;

	/** Alters the size of the pool, creating or joining threads as necessary. */
	void Resize(size_t a_NewSize);
	
private:
	/** Class representing a thread in the pool. */
	class cWorkerThread;

	using cWorkerPtr = std::unique_ptr<cWorkerThread>;

	/** Mutex protecting access to m_Threads (but not blocking the threads). */
	mutable std::mutex m_CS;

	/** The threads in the pool. */
	std::vector<cWorkerPtr> m_Threads;

	/** The queue of tasks waiting to be executed. */
	cQueue<cWork> m_SharedWork;

	/** Name of the pool. Used to name worker threads. */
	const AString m_PoolName;
};