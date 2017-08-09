#pragma once

// Guard against macro new
#pragma push_macro("new")
#undef new


namespace Detail
{
	template <class ... Types>
	struct TypeList {};

	template <size_t CurIdx, class SearchType, class Types>
	struct FindTypeImpl;

	template <size_t CurIdx, class SearchType, class HeadType, class ... TailTypes>
	struct FindTypeImpl<CurIdx, SearchType, TypeList<HeadType, TailTypes...>>:
		FindTypeImpl<CurIdx + 1, SearchType, TypeList<TailTypes...>>
	{
	};

	template <size_t CurIdx, class SearchType, class ... TailTypes>
	struct FindTypeImpl<CurIdx, SearchType, TypeList<SearchType, TailTypes...>>:
		std::integral_constant<size_t, CurIdx>
	{
	};

	template <size_t CurIdx, class SearchType>
	struct FindTypeImpl<CurIdx, SearchType, TypeList<>>
	{  // Don't set a value for SFINAE friendlyness
	};

	template <class SearchType, class Types>
	using FindType = FindTypeImpl<0, SearchType, Types>;
}





/** A "simple" variant implementation that only allows visitation. */
template <class ... Types>
class cBasicVariant
{
	using ListOfTypes = Detail::TypeList<Types...>;
public:

	template <class T, size_t Idx = Detail::FindType<T, ListOfTypes>::value>
	cBasicVariant(T a_Value):
		m_TypeIndex(Idx)
	{
		new(&m_Storage) T(std::move(a_Value));
	}

	~cBasicVariant()
	{
		struct sDestroyer
		{
			template <class T>
			void operator () (T & a_Value) { a_Value.~T(); }
		};

		Visit(sDestroyer{});
	}

private:

	#pragma push_macro("VISIT_OVERLOAD")
	#undef VISIT_OVERLOAD
    #define VISIT_OVERLOAD(QUAL)                                                                        \
    public:                                                                                             \
        template <class Visitor>                                                                        \
        void Visit(Visitor && a_Visitor) QUAL                                                           \
        {                                                                                               \
            DoVisit(0, std::forward<Visitor>(a_Visitor), ListOfTypes{});                                \
        }                                                                                               \
                                                                                                        \
    private:                                                                                            \
        template <class Visitor, class HeadType, class ... TailTypes>                                   \
        void DoVisit(size_t a_Idx, Visitor && a_Visitor, Detail::TypeList<HeadType, TailTypes...>) QUAL \
        {                                                                                               \
            if (a_Idx != 0)                                                                             \
            {                                                                                           \
                DoVisit(a_Idx - 1, std::forward<Visitor>(a_Visitor), Detail::TypeList<TailTypes...>{}); \
            }                                                                                           \
            else                                                                                        \
            {                                                                                           \
                a_Visitor(reinterpret_cast<HeadType QUAL>(m_Storage));                                  \
            }                                                                                           \
        }                                                                                               \
                                                                                                        \
        template <class Visitor>                                                                        \
        void DoVisit(size_t a_Idx, Visitor && a_Visitor, Detail::TypeList<>) QUAL                       \
        {                                                                                               \
            ASSERT(!"This point should never be reached");                                              \
		}

	// Create all the Visit overloads
	VISIT_OVERLOAD(&);
	VISIT_OVERLOAD(&&);
	VISIT_OVERLOAD(const &);
	VISIT_OVERLOAD(const &&);
	VISIT_OVERLOAD(volatile &);
	VISIT_OVERLOAD(volatile &&);
	VISIT_OVERLOAD(const volatile &);
	VISIT_OVERLOAD(const volatile &&);

	#pragma pop_macro("VISIT_OVERLOAD")

	using cStorage = typename std::aligned_union<0, Types...>::type;
	cStorage m_Storage;
	size_t m_TypeIndex;
};




#pragma pop_macro("new")
