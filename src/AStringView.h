

namespace cpp17
{
	/** An incomplete implementation of c++17's std::basic_string_view. */
	template <class CharT, class Traits = std::char_traits<CharT>>
	class basic_string_view
	{
	public:
		static const size_t npos = std::numeric_limits<size_t>::max();

		basic_string_view():
			m_Begin(nullptr),
			m_Size(0)
		{
		}

		basic_string_view(const CharT * a_String, size_t a_Size):
			m_Begin(a_String),
			m_Size(a_Size)
		{
		}

		basic_string_view(const CharT * a_CString):
			m_Begin(a_CString),
			m_Size(Traits::length(a_CString))
		{
		}

		CharT operator [] (size_t a_Pos) const
		{
			return m_Begin[a_Pos];
		}

		const CharT * data() const
		{
			return m_Begin;
		}

		size_t size() const
		{
			return m_Size;
		}

		size_t length() const
		{
			return m_Size;
		}

		bool empty() const
		{
			return (m_Size == 0);
		}

		const CharT * begin() const
		{
			return m_Begin;
		}

		const CharT * end() const
		{
			return m_Begin + m_Size;
		}

		void remove_prefix(size_t a_Count)
		{
			m_Begin += a_Count;
			m_Size -= a_Count;
		}

		void remove_suffix(size_t a_Count)
		{
			ASSERT(a_Count <= m_Size);
			m_Size -= a_Count;
		}

		basic_string_view substr(size_t pos = 0, size_t count = npos) const
		{
			size_t NewSize = (count == npos) ? m_Size - pos : count;
			return AStringView(m_Begin + pos, NewSize);
		}

		int compare(basic_string_view a_CompareTo) const
		{
			size_t CommonSize = std::min(m_Size, a_CompareTo.m_Size);
			int CommonCmp = Traits::compare(m_Begin, a_CompareTo.m_Begin, CommonSize);
			if (CommonCmp == 0)
			{
				if (m_Size < a_CompareTo.m_Size)
				{
					return -1;
				}
				else if (m_Size > a_CompareTo.m_Size)
				{
					return 1;
				}
				else  // m_Size == a_CompareTo.m_Size
				{
					return 0;
				}
			}
			return CommonCmp;
		}

		size_t find(CharT a_Char, size_t a_StartPos = 0) const NOEXCEPT
		{
			auto SearchStart = m_Begin + a_StartPos;
			return (Traits::find(SearchStart, a_Size - a_StartPos, a_Char) - SearchStart);
		}

	private:
		const CharT * m_Begin;
		size_t m_Size;
	};

	// Define all the comparison operators
	#pragma push_macro("STRING_VIEW_COMPARE")
	#undef STRING_VIEW_COMPARE

	#define STRING_VIEW_COMPARE(op) \
		template <class CharT, class Traits> bool operator op ( \
			const basic_string_view<CharT, Traits> & a_Lhs, \
			const basic_string_view<CharT, Traits> & a_Rhs \
		) NOEXCEPT \
		{ \
			return (a_Lhs.compare(a_Rhs) op 0); \
		}
	
	STRING_VIEW_COMPARE(==);
	STRING_VIEW_COMPARE(!=);
	STRING_VIEW_COMPARE(< );
	STRING_VIEW_COMPARE(<=);
	STRING_VIEW_COMPARE(> );
	STRING_VIEW_COMPARE(>=);

	#pragma pop_macro("STRING_VIEW_COMPARE")
	
	// Convenience aliases
	using string_view    = basic_string_view<char>;
	using wstring_view 	 = basic_string_view<wchar_t>;
	using u16string_view = basic_string_view<char16_t>;
	using u32string_view = basic_string_view<char32_t>;

	namespace literals
	{
		string_view operator ""_sv(const char * a_String, size_t a_Size) NOEXCEPT
		{
			return { a_String, a_Size };
		}
	}
}  // namespace cpp17



/** Wrapper around cpp17::string_view to add conversions from string.
These would normally be defined as conversion operators in std::string itself. */
class AStringRef:
	public cpp17::string_view
{
	using Super = cpp17::string_view;
public:

	using Super::Super;

	AStringRef(const AString & a_String):
		Super(a_String.data(), a_String.size())
	{
	}

	operator AString () const
	{
		return { data(), size() };
	}
};




int foo()
{
	using namespace cpp17::literals;
	auto a = "aosjdhasdjh"_sv;

	return a[6];
}

