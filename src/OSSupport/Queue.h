
// Queue.h

// Implements the cQueue class representing a thread safe queue

#pragma once

/*
Items can be added multiple times to a queue, there are two functions for
adding, EnqueueItem() and EnqueueItemIfNotPresent(). The first one always
enqueues the specified item, the second one checks if the item is already
present and only queues it if it isn't.

Usage:
To create a queue of type T, instantiate a cQueue<T> object. You can also
modify the behavior of the queue when deleting items and when adding items
that are already in the queue by providing a second parameter, a class that
implements the functions Delete() and Combine(). An example is given in
cQueueFuncs and is used as the default behavior. */

#include <mutex>
#include <condition_variable>

/** This empty struct allows for the callback functions to be inlined */
template <class T>
struct cQueueFuncs
{
public:

	/** Called when an Item is deleted from the queue without being returned */
	static void Delete(T &) {}

	/** Called when an Item is inserted with EnqueueItemIfNotPresent and there is another equal value already inserted */
	static void Combine(T & a_existing, const T & a_new)
	{
		UNUSED(a_existing);
		UNUSED(a_new);
	}
};





template <class ItemType, class Funcs = cQueueFuncs<ItemType> >
class cQueue
{
	// The actual storage type for the queue
	using QueueType = std::list<ItemType>;

	using cLockGuard = std::lock_guard<std::mutex>;
	using cUniqueLock = std::unique_lock<std::mutex>;
	
public:

	cQueue() = default;

	/** Enqueues an item to the queue, may block if other threads are accessing the queue. */
	void EnqueueItem(ItemType a_Item)
	{
		// Do allocations outside of critical section
		QueueType temp;
		temp.push_back(std::move(a_Item));
		{
			cLockGuard Lock(m_CS);
			m_Contents.splice(m_Contents.end(), temp);
		}
		m_evtAdded.notify_one();
	}


	/** Enqueues an item in the queue if not already present (as determined by operator ==). Blocks other threads from accessing the queue.
	Returns true if the item was Enqueued. */
	bool EnqueueItemIfNotPresent(ItemType a_Item)
	{
		{
			cLockGuard Lock(m_CS);
			auto itr = std::find(m_Contents.begin(), m_Contents.end(), a_Item);
			if (itr != m_Contents.end())
			{
				Funcs::Combine(*itr, a_Item);
				return false;
			}
			m_Contents.push_back(std::move(a_Item));
		}
		m_evtAdded.notify_one();
		return true;
	}


	/** Dequeues an item from the queue if any are present.
	Returns true if successful. Value of item is undefined if dequeuing was unsuccessful. */
	bool TryDequeueItem(ItemType & a_Item)
	{
		QueueType temp;
		{
			cLockGuard Lock(m_CS);
			if (m_Contents.empty())
			{
				return false;
			}
			// Take out the first node from the list
			temp.splice(temp.begin(), m_Contents, m_Contents.begin());
		}
		
		m_evtRemoved.notify_all();
		a_Item = std::move(temp.front());
		return true;
	}


	/** Dequeues an item from the queue, blocking until an item is available. */
	ItemType DequeueItem()
	{
		QueueType temp;
		{
			cUniqueLock Lock(m_CS);
			m_evtAdded.wait(Lock, [this]() { return !m_Contents.empty(); });
			// Take out the first node from the list
			temp.splice(temp.begin(), m_Contents, m_Contents.begin());
		}
		m_evtRemoved.notify_all();
		a_Item = std::move(temp.front());
		return true;
	}


	/** Dequeues an item from the queue, blocking until an item is available or the predicate returns true.
	Returns true if successful. Value of item is undefined if dequeuing was unsuccessful. */
	template <class Predicate>
	bool DequeueItem(ItemType & a_Item, Predicate a_Pred)
	{
		QueueType temp;
		{
			cUniqueLock Lock(m_CS);
			m_evtAdded.wait(Lock, [this, a_Pred]() { return (!m_Contents.empty() || a_Pred()); });

			if (m_Contents.empty())
			{
				// We were woken by the predicate
				return false;
			}
			// Take out the first node from the list
			temp.splice(temp.begin(), m_Contents, m_Contents.begin());
		}
		m_evtRemoved.notify_all();
		a_Item = std::move(temp.front());
		return true;
	}


	/** Blocks until the queue is empty. */
	void BlockTillEmpty(void)
	{
		cUniqueLock Lock(m_CS);
		m_evtRemoved.wait(Lock, [this]() { return m_Contents.empty(); });
	}


	/** Removes all Items from the Queue, calling Delete on each of them. */
	void Clear(void)
	{
		QueueType temp;
		{
			cLockGuard Lock(m_CS);
			std::swap(temp, m_Contents);
		}

		m_evtRemoved.notify_all();
		
		for (auto & Item : temp)
		{
			Funcs::Delete(Item);
		}
	}

	/** Returns the size at time of being called.
	Do not use to determine whether to call DequeueItem(), use TryDequeueItem() instead */
	size_t Size(void)
	{
		cLockGuard Lock(m_CS);
		return m_Contents.size();
	}


	/** Removes the item from the queue. If there are multiple such items, only the first one is removed.
	Returns true if the item has been removed, false if no such item found. */
	bool Remove(const ItemType& a_Item)
	{
		{
			cLockGuard Lock(m_CS);
			auto itr = std::find(m_Contents.begin(), m_Contents.end(), a_Item);
			if (itr == m_Contents.end())
			{
				return false;
			}
			m_Contents.erase(itr);
		}

		m_evtRemoved.notify_all();
		return true;
	}


	/** Removes all items for which the predicate returns true. */
	template <class Predicate>
	void RemoveIf(Predicate a_Predicate)
	{
		cLockGuard Lock(m_CS);
		for (auto itr = m_Contents.begin(); itr != m_Contents.end();)
		{
			if (a_Predicate(*itr))
			{
				itr = m_Contents.erase(itr);
				m_evtRemoved.notify_all();
			}
			else
			{
				++itr;
			}
		}  // for itr - m_Contents[]
	}

	/** Wake those trying to dequeue items. */
	void Wake()
	{
		m_evtAdded.notify_all();
	}

private:
	/** The contents of the queue */
	QueueType m_Contents;

	/** Mutex that protects access to the queue contents */
	std::mutex m_CS;

	/** Condition that is signalled when an item is added */
	std::condition_variable m_evtAdded;

	/** Condition that is signalled when an item is removed (both dequeued or erased) */
	std::condition_variable m_evtRemoved;
};




