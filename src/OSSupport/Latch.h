




/** A one use thread synchronisation point.
The latch is initialised with a count.
The count is decremented by each call to CountDown() or CountDownAndWait() until the count is 0.
Once the count reaches 0, the latch unlocks and any thread waiting on the latch will wake. */
class cLatch
{
public:
	/** Create a latch with initial count a_Count. */
	explicit cLatch(ptrdiff_t a_Count):
		m_Count{a_Count}
	{
	}

	/** Decrement count by the value given and block until the latch is ready.
	\param a_Decrement must be less than the latch count. */
	void CountDownAndWait(ptrdiff_t a_Decrement = 1);

	/** Decrement count by the value given.
	\param a_Decrement must be less than the latch count. */
	void CountDown(ptrdiff_t a_Decrement = 1);

	/** Checks if the count has reached 0. */
	bool IsReady() const;

	/** Blocks until the latch is ready.
	If the latch counter is 0 already this returns immediately. */
	void Wait() const;

private:
	/** Present count. Latch is ready when m_Count == 0. */
	std::atomic<ptrdiff_t> m_Count;
	/** Mutex for use with the condition variable. */
	mutable std::mutex m_CS;
	/** Condition variable notified when m_Count is decremented to 0. */
	mutable std::condition_variable m_CondFinished;
};
