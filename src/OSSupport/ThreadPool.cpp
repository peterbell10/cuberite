// ThreadPool.cpp

// Implements the cThreadPool class representing a task scheduler with a dynamically sized pool of threads. 


#include "Globals.h"

#include "IsThread.h"
#include "ThreadPool.h"



class cThreadPool::cWorkerThread :
	public cIsThread
{
public:

	cWorkerThread(const AString & a_WorkerName, cThreadPool & a_Parent);
	~cWorkerThread();

	/** Request the thread stop and block till the thread has joined. */
	void Stop();

	/** Request the thread stop withou waiting for the thread to join. */
	void RequestStop();

protected:

	/** Process work items until the thread is stopped. */
	virtual void Execute() override final;

private:
	/** The Pool this is operating in. */
	cThreadPool & m_Parent;
};


////////////////////////////////////////////////////////////////////////////////
// cThreadPool::cWorkerThread:


cThreadPool::cWorkerThread::cWorkerThread(const AString & a_WorkerName, cThreadPool & a_Parent) :
	cIsThread(a_WorkerName),
	m_Parent(a_Parent)
{
}





cThreadPool::cWorkerThread::~cWorkerThread()
{
	Stop();
}





void cThreadPool::cWorkerThread::Stop()
{
	RequestStop();
	cIsThread::Stop();
}





void cThreadPool::cWorkerThread::RequestStop()
{
	m_ShouldTerminate = true;
	m_Parent.m_SharedWork.Wake();
}





void cThreadPool::cWorkerThread::Execute()
{
	cThreadPool::cWork Work;
	while (!m_ShouldTerminate)
	{
		// Execute the next task in the queue
		if (m_Parent.m_SharedWork.DequeueItem(Work, [this]() {return m_ShouldTerminate.load(); }))
		{
			Work();
		}		
	}
}





////////////////////////////////////////////////////////////////////////////////
// cThreadPool:

cThreadPool::cThreadPool(size_t a_InitialSize, AString a_PoolName):
	m_PoolName(std::move(a_PoolName))
{
	Resize(a_InitialSize);
}





cThreadPool::~cThreadPool()
{
	WaitForFinish();
	JoinAll();
}




void cThreadPool::WaitForFinish()
{
	m_SharedWork.BlockTillEmpty();
}





bool cThreadPool::Submit(cWork a_Work)
{
	m_SharedWork.EnqueueItem(std::move(a_Work));
	return true;
}





size_t cThreadPool::Size() const
{
	std::lock_guard<std::mutex> Lock(m_CS);
	return m_Threads.size();
}





void cThreadPool::Resize(size_t a_NewSize)
{
	std::lock_guard<std::mutex> Lock(m_CS);
	size_t CurSize = m_Threads.size();
	if (a_NewSize < CurSize)
	{
		// Need to join some threads
		// First notify workers that are being teminated without blocking
		for (size_t i = a_NewSize; i != CurSize; ++i)
		{
			m_Threads[i]->RequestStop();
		}

		// Then actually join in the destuctors
		m_Threads.resize(a_NewSize);
	}
	else if (a_NewSize > CurSize)
	{
		// Need to spin up some new threads
		AString ThreadName;
		m_Threads.reserve(a_NewSize);
		for (size_t i = CurSize; i != a_NewSize; ++i)
		{
			Printf(ThreadName, "%s:" SIZE_T_FMT, m_PoolName.c_str(), i);
			m_Threads.push_back(cpp14::make_unique<cWorkerThread>(ThreadName, *this));
			m_Threads[i]->Start();
		}
	}
}




