

#include "Globals.h"
#include "OSSupport/Latch.h"

namespace
{
	/** Atomically decrements a_Value and returns the new value. */
	ptrdiff_t Decrement(std::atomic<ptrdiff_t>& a_Value, ptrdiff_t a_Decrement)
	{
		auto OldVal = a_Value.fetch_sub(a_Decrement);
		return OldVal - a_Decrement;
	}

	using UniqueLock = std::unique_lock<std::mutex>;
}





void cLatch::CountDownAndWait(ptrdiff_t a_Decrement)
{
	if (Decrement(m_Count, a_Decrement) <= 0)
	{
		// This action has readied the latch
		m_CondFinished.notify_all();
		return;
	}

	// Wait until ready
	UniqueLock Lock(m_CS);
	m_CondFinished.wait(Lock, [this] { return IsReady(); });
}





void cLatch::CountDown(ptrdiff_t a_Decrement)
{
	if (Decrement(m_Count, a_Decrement) <= 0)
	{
		// This action has readied the latch
		m_CondFinished.notify_all();
	}
}





bool cLatch::IsReady() const
{
	return (m_Count <= 0);
}





void cLatch::Wait() const
{
	if (IsReady())
	{
		return;  // Already ready
	}

	// Wait until ready
	UniqueLock Lock(m_CS);
	m_CondFinished.wait(Lock, [this] { return IsReady(); });
}
