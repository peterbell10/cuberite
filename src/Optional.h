

#include <memory>


namespace cpp17
{
	struct nullopt_t {} nullopt;

	struct bad_optional_access:
		public std::exception
	{
		using std::exception::exception;
	};

	/** Slightly non-standard implementation of std::optional.
	Doesn't support constructor or assignment forwarding.
	e.g. optional<AString>{"Some string"}
	Will construct a temprorary AString to pass to optional's constructor. */
	template <typename T>
	class optional
	{
	public:

		optional(): m_HasValue(false) {}
		optional(cpp17::nullopt_t) : optional() {}

		optional(const optional & a_Other):
			optional()
		{
			if (a_Other.m_HasValue)
			{
				unchecked_emplace(a_Other.unchecked_value());
			}
		}

		optional(optional && a_Other):
			optional()
		{
			if (a_Other.m_HasValue)
			{
				unchecked_emplace(std::move(a_Other.unchecked_value()));
			}
		}

		~optional()
		{
			reset();
		}

		optional & operator = (const optional & a_OtherOptional)
		{
			reset();
			if (a_OtherOptional.m_HasValue)
			{
				unchecked_emplace(a_OtherOptional.unchecked_value());
			}
			return *this;
		}

		optional & operator = (optional && a_OtherOptional)
		{
			reset();
			if (a_OtherOptional.m_HasValue)
			{
				unchecked_emplace(std::move(a_OtherOptional.unchecked_value()));
			}
			return *this;
		}

		optional & operator = (const T & a_OtherValue)
		{
			if (m_HasValue)
			{
				unchecked_value() = a_OtherValue;
			}
			else
			{
				unchecked_emplace(a_OtherValue);
			}
			return *this;
		}


		optional & operator = (T && a_OtherValue)
		{
			if (m_HasValue)
			{
				unchecked_value() = std::move(a_OtherValue);
			}
			else
			{
				unchecked_emplace(std::move(a_OtherValue));
			}
			return *this;
		}

		optional & operator = (cpp17::nullopt_t)
		{
			reset();
			return *this;
		}

		bool has_value() const { return m_HasValue; }
		explicit operator bool() const { return m_HasValue; }

		T & operator * () &                { return unchecked_value(); }
		T && operator * () &&              { return unchecked_value(); }
		const T & operator * () const   &  { return unchecked_value(); }
		const T && operator * () const  && { return unchecked_value(); }

		T & operator -> () &                { return unchecked_value(); }
		T && operator -> () &&              { return unchecked_value(); }
		const T & operator -> () const   &  { return unchecked_value(); }
		const T && operator -> () const  && { return unchecked_value(); }

		T & value() &
		{
			if (!m_HasValue)
			{
				throw bad_optional_access();
			}
			return unchecked_value();
		}

		T && value() &&
		{
			if (!m_HasValue)
			{
				throw bad_optional_access();
			}
			return unchecked_value();
		}

		const T & value() const  &
		{
			if (!m_HasValue)
			{
				throw bad_optional_access();
			}
			return unchecked_value();
		}

		const T && value() const  &&
		{
			if (!m_HasValue)
			{
				throw bad_optional_access();
			}
			return unchecked_value();
		}

		template <typename U>
		T value_or(U && a_Default_value) const
		{
			if (m_HasValue)
			{
				return unchecked_value();
			}
			else
			{
				return std::forward<U>(a_DefaultValue);
			}
		}

		template <typename... Args>
		void emplace(Args && ... a_Args)
		{
			reset();
			unchecked_emplace(std::forward<Args>(a_Args)...);
		}

		void reset()
		{
			if (m_HasValue)
			{
				unchecked_value().~T();
				m_HasValue = false;
			}
		}

		void swap(optional & a_Other)
		{
			// Either the primary has a value or neither does
			optional & primary   = m_HasValue ? *this : a_Other;
			optional & secondary = m_HasValue ? a_Other : *this;

			if (primary.m_HasValue)
			{
				if (secondary.m_HasValue)
				{
					std::swap(unchecked_value(), a_Other.unchecked_value());
				}
				else
				{
					secondary.unchecked_emplace(std::move(primary.unchecked_value()));
					primary.reset();
				}
			}
			else
			{
				// Neither has a value so nothing needs to change
			}
		}

	private:
		using buff_type = typename std::aligned_union<sizeof(T), T>::type;

		/** Constructs a new element inplace.
		@pre m_HasValue must be false. */
		template <typename... Args>
		void unchecked_emplace(Args && ... a_Args)
		{
			// Guard against macro new
			#pragma push_macro("new")
			new(&m_ValueBuffer) T(std::forward<Args>(a_Args)...);
			#pragma pop_macro("new")

			m_HasValue = true;
		}

		/** Returns the value contained by this optional.
		@pre m_HasValue must be true. */
		T & unchecked_value() & NOEXCEPT                { return reinterpret_cast<        (T) (&) >(m_ValueBuffer); }
		T & unchecked_value() && NOEXCEPT               { return reinterpret_cast<        (T) (&&)>(m_ValueBuffer); }
		const T & unchecked_value() const  & NOEXCEPT   { return reinterpret_cast<(const) (T) (&) >(m_ValueBuffer); }
		const T && unchecked_value() const  && NOEXCEPT { return reinterpret_cast<(const) (T) (&&)>(m_ValueBuffer); }

		bool m_HasValue;
		buff_type m_ValueBuffer;  // Contains a valid T iff m_HasValue is true
	};


}  // namespace cpp17
