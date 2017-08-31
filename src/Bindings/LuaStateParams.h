// LuaStateParams.h

// Declares the cLuaStateParams class that provides high-level functions for reading API parameters from Lua state

#pragma once





#include "LuaState.h"
#include "tolua++/include/tolua++.h"





namespace Detail
{
	template <typename... Ts> struct TypeList {};

	template <size_t I, typename Types> struct GetImpl {};

	template <size_t I, typename Head, typename... Tail>
	struct GetImpl<I, TypeList<Head, Tail...>>:
		GetImpl<I - 1, TypeList<Tail...>> // Keep iterating over the list
	{
	};

	// Terminating case
	template <typename Head, typename... Tail>
	struct GetImpl<0, TypeList<Head, Tail...>>
	{
		using type = Head;
	};

	/** Aliases the Ith type in the TypeList Types */
	template <size_t I, typename Types>
	using Get = typename GetImpl<I, Types>::type;


	
	// C++14 TODO: Replace with std::index_sequence
	template <size_t... Is>	struct IndexSequence {};

	/** Append to the end of an IndexSequence */
	template <typename Sequence, size_t New>
	struct AppendIndex;

	template <size_t New, size_t... Curr>
	struct AppendIndex<IndexSequence<Curr...>, New>
	{
		using type = IndexSequence<Curr..., New>;
	};

	template <size_t I>
	struct MakeIndexSequenceImpl:
		AppendIndex<typename MakeIndexSequenceImpl<I - 1>::type, I - 1>
	{
	};

	template<>
	struct MakeIndexSequenceImpl<0>
	{
		using type = IndexSequence<>;
	};

	/** Build and IndexSequence of all values from 0 to I - 1 (i.e. IndexSequence<0, 1, .., I-1>) */
	template <size_t I>
	using MakeIndexSequence = typename MakeIndexSequenceImpl<I>::type;


	// LambdaSignature implementation adapted from the answer to this SO post:
	// https://stackoverflow.com/questions/11893141/

	template <typename Result, typename... Args>
	struct SignatureDescription
	{
		using ResultType = Result;
		using ArgumentTypes = TypeList<Args...>;
	};

	/** Extracts result type and argument types from a pmf type. */
	template<typename T> struct LambdaSignatureImpl {};

	template<typename C, typename R, typename... A>
	struct LambdaSignatureImpl<R(C::*)(A...)>:
		SignatureDescription<R, A...>
	{
	};

	template<typename C, typename R, typename... A>
	struct LambdaSignatureImpl<R(C::*)(A...) const>:
		SignatureDescription<R, A...>
	{
	};

	template<typename C, typename R, typename... A>
	struct LambdaSignatureImpl<R(C::*)(A...) volatile>:
		SignatureDescription<R, A...>
	{
	};

	template<typename C, typename R, typename... A>
	struct LambdaSignatureImpl<R(C::*)(A...) const volatile>:
		SignatureDescription<R, A...>
	{
	};

	/** Provides a description of the call signature for T where T has an unambiguous operator().
	i.e. Must not be overloaded or templated meaning generic lambdas will not work.
	Provides type aliases:
		- ResultType: the return type of the function.
		- ArgumentTypes: a TypeList<> of the argument types of the function. */
	template<typename T>
	struct LambdaSignature:
		// Pass the pmf type of the call operator
		LambdaSignatureImpl<decltype(&std::remove_reference<T>::type::operator())>
	{
	};



	/** Aliases the given type after stripping references and top level cv qualifiers.*/
	template <typename T>
	using RemoveCVR =
		typename std::remove_cv<
			typename std::remove_reference<T>::type
		>::type;



	/** Utility struct that provides the functionality to cLuaStateParams::GetTypeDescription.
	Supports decorators such as cLuaStateParams::cSelf<T>.
	Must be a struct in order to support decorators, and must not be inside a class (gcc / clang complain about that).
	The general declaration must not be used by the code, all types need a specialization returning the correct type. */
	template <typename T>
	struct TypeDescription
	{
		/* If compiler complains on the previous line, you need to make sure that the type
		passed as the template parameter to this structure has an appropriate TypeDescription specialization.
		Usually these are created automatically for all API classes by ToLua++ in the LuaStateParams_TypeDescs.inc file.
		For basic types, the specializations are below.
		*/
		static const char * desc() = delete;
	};

	// Specializations are defined after the cLuaStateParams class declaration
}





/** A namespace-like class for reading parameters to API functions from Lua, using templates.
The Call() function is the main entrypoint for this, and does all the work.
Example usage:
int res = cLuaStateParams::Read(LuaState,
	[](cLuaStateParams::cSelf<cItem> Item, Param1, Param2, Param3)
	{
		Item->DoSomething(Param1, Param2, Param3);
		return 0;
	},
	[](cLuaStateParams::cSelf<cItem> Item, ParamA, ParamB)
	{
		Item->DoSomething(ParamA, ParamB);
		return 0;
	}
);
*/
class cLuaStateParams
{
	/** A dumb smart-pointer style wrapper around a bare pointer. */
	template <typename T>
	class cWrappedPtr
	{
	public:
		typedef T ParentType;
		T * m_Ptr;

		T & operator * () const { return *m_Ptr; }

		T * operator -> () const { return m_Ptr; }

		T * get() const { return m_Ptr; }
	};

public:
	/** A wrapper used in cLuaStateParams::Call() to signalize that the value must be a non-nil pointer when read from Lua. */
	template <typename T>
	class cNonNil:
		public cWrappedPtr<T>
	{
	};


	/** A wrapper used in cLuaStateParams::Call() to signalize that the value is a "self" - it must be a non-nil pointer when read from Lua,
	and has a special error message when mismatched. */
	template <typename T>
	class cSelf:
		public cWrappedPtr<T>
	{
	};


	/** A wrapper used in cLuaStateParams::Call() to signalize that the value is a "static self",
	it must be the table representing the class T when read from Lua. */
	template <class T>
	class cStaticSelf:
		public cWrappedPtr<T>
	{
	};



protected:

	/** Type to bundle the signature information together with the reference itself. */
	template <typename Lambda>
	struct LambdaDescription:
		Detail::LambdaSignature<Lambda>
	{
		Lambda && m_Lambda;

		LambdaDescription(Lambda && a_Lambda):
			m_Lambda{ std::forward<Lambda>(a_Lambda) }
		{
		}
	};


	/** Helper to create a LambdaDescription with type deduction. */
	template <typename Lambda>
	static LambdaDescription<Lambda> MakeLambdaDesc(Lambda && a_Lambda)
	{
		return { std::forward<Lambda>(a_Lambda) };
	}


	/** Attempts to match the params on the Lua stack to the API function overloads given (recursively).
	a_CurOverload is the index of the currently processed overload (recursion level),
	Returns the index of the overload that matches the parameters.
	If no overloads match, returns -1. */
	template <typename LambdaDesc, typename... OtherLambdaDescs>
	static int CallInternal(cLuaState & a_LuaState, LambdaDesc a_LD, OtherLambdaDescs... a_OtherLDs)
	{
		// Try to read this overload
		if (CheckSingleOverload(a_LuaState, typename LambdaDesc::ArgumentTypes{}))
		{
			return CallSingleOverload(a_LuaState, a_LD.m_Lambda, typename LambdaDesc::ArgumentTypes{});
		}

		// Try the next overload:
		return CallInternal(a_LuaState, a_OtherLDs...);
	}


	/** Terminator for the template-based recursion of the function above - for a single overload. */
	static int CallInternal(cLuaState & a_LuaState)
	{
		// No more overloads, report failure
		return -1;
	}



	/** Helper struct to implement iterating over std::tuple elements */
	template <size_t N>
	using SizeT = std::integral_constant<size_t, N>;



	/** Attempts to match the params on the Lua stack to the given API function overload.
	Returns true if compatible, false if not. */
	template <typename... Args>
	static bool CheckSingleOverload(cLuaState & a_LuaState, Detail::TypeList<Args...> a_ArgsTypes)
	{
		// Check that there exactly as many params as the tuple items:
		if (!lua_isnone(a_LuaState, sizeof...(Args) + 1))
		{
			// Too many params
			return false;
		}
		if (lua_isnone(a_LuaState, sizeof...(Args)))
		{
			// Too few params
			return false;
		}

		// Read the tuple, compile-time-recursively:
		return CheckSingleOverloadRecurse(a_LuaState, a_ArgsTypes, SizeT<sizeof...(Args)>());
	}



	/** Attempts to match the params on the Lua stack to the given API function overload.
	The compile-time recursive worker implementation of ReadSingleOverload, recurses by the number of elements in the overload tuple.
	Returns true on success, false on failure. */
	template <typename... Args, size_t N>
	static bool CheckSingleOverloadRecurse(cLuaState & a_LuaState, Detail::TypeList<Args...>, SizeT<N>)
	{
		// First check the params from the lower indices:
		if (!CheckSingleOverloadRecurse(a_LuaState, Detail::TypeList<Args...>{}, SizeT<N - 1>{}))
		{
			return false;
		}

		// Then check the params match at our index:
		using Type = Detail::RemoveCVR<Detail::Get<N - 1, Detail::TypeList<Args...>>>;
		Type temp;
		return GetStackValue(a_LuaState, N, temp);
	}


	/** Terminator for the above compile-time-recursive function. */
	template <typename... Args>
	static bool CheckSingleOverloadRecurse(cLuaState & a_LuaState, Detail::TypeList<Args...>, SizeT<0>)
	{
		return true;
	}

	template<typename Lambda, typename Types, size_t... Is>
	static int CallSingleOverloadImpl(cLuaState & a_LuaState, Lambda && a_Lambda, Types, Detail::IndexSequence<Is...>)
	{
		return a_Lambda(GetValue<Is, Types>(a_LuaState)...);
	}

	template<typename Lambda, typename... Args>
	static int CallSingleOverload(cLuaState & a_LuaState, Lambda && a_Lambda, Detail::TypeList<Args...>)
	{
		return CallSingleOverloadImpl(
			a_LuaState,
			std::forward<Lambda>(a_Lambda),
			Detail::TypeList<Args...>{},
			Detail::MakeIndexSequence<sizeof...(Args)>{}
		);
	}


	template <size_t I, typename Types>
	static auto GetValue(cLuaState & a_LuaState)
		-> Detail::RemoveCVR<Detail::Get<I, Types>>
	{
		using Type = Detail::RemoveCVR<Detail::Get<I, Types>>;
		Type Val;
		int StackPos = static_cast<int>(I + 1);  // From 0 based to 1 based indexing
		bool Succeeded = GetStackValue(a_LuaState, StackPos, Val);
		UNUSED(Succeeded);
		ASSERT(Succeeded);  // Should be checked before the call
		return Val;
	}


	/** Reads one value from the Lua stack.
	Returns true on success, false on failure.
	The hard work is delegated into cLuaState that already has this function, but we need to specialize it for decorators (cNonNil, cSelf etc.). */
	template <typename T>
	static bool GetStackValue(cLuaState & a_LuaState, int a_StackPos, T & a_ReturnedVal)
	{
		return a_LuaState.GetStackValue(a_StackPos, a_ReturnedVal);
	}


	/** Reads one value from the Lua stack.
	Returns true on success, false on failure.
	Specialization: Retrieves a value that should represent a non-nil pointer. Used primarily for ReadParams(... nonNil(...)) */
	template <typename T>
	static bool GetStackValue(cLuaState & a_LuaState, int a_StackPos, cNonNil<T> & a_ReturnedVal)
	{
		auto res = GetStackValue(a_LuaState, a_StackPos, a_ReturnedVal.m_Ptr);
		if (res)
		{
			if (a_ReturnedVal.m_Ptr == nullptr)
			{
				return false;
			}
		}
		return res;
	}


	/** Reads one value from the Lua stack.
	Returns true on success, false on failure.
	Specialization: Retrieves a value that should represent a pointer to self - must not be nil. Used primarily for ReadParams(self(...)) */
	template <typename T>
	static bool GetStackValue(cLuaState & a_LuaState, int a_StackPos, cSelf<T> & a_ReturnedVal)
	{
		auto res = GetStackValue(a_LuaState, a_StackPos, a_ReturnedVal.m_Ptr);
		if (res)
		{
			if (a_ReturnedVal.m_Ptr == nullptr)
			{
				return false;
			}
		}
		return res;
	}


	/** Reads one value from the Lua stack.
	Returns true on success, false on failure.
	Specialization: The value should represent a class and has no useful meaning, it is not read, just type-checked.
	Used primarily for ReadParams(staticSelf(...)) */
	template <typename T>
	static bool GetStackValue(cLuaState & a_LuaState, int a_StackPos, cStaticSelf<T> & a_ReturnedVal)
	{
		return (CheckValueType(a_LuaState, a_StackPos, a_ReturnedVal).empty());
	}



	/** Raises a Lua error that the parameters don't match the overloads.
	Builds and logs the whole error message, including the reason why each overload wasn't matched. */
	template <typename... ArgTypes>
	static void RaiseError(cLuaState & a_LuaState, ArgTypes...)
	{
		auto matcherMsgs = BuildMatcherErrorMessages(a_LuaState, ArgTypes{}...);
		a_LuaState.ApiParamError("Parameters don't match function signatures:\n%s",
			StringJoin(matcherMsgs, "\n\t").c_str()
		);
	}



	/** Returns a vector of string, each item representing a single overload's signature
	and the error message from the matcher why the signature cannot be used. */
	template <typename... Args, typename... OtherOverloads>
	static AStringVector BuildMatcherErrorMessages(
		cLuaState & a_LuaState,
		Detail::TypeList<Args...>,
		OtherOverloads...
	)
	{
		auto res = BuildMatcherErrorMessages(a_LuaState, OtherOverloads{}...);
		auto signature = BuildSingleOverloadDescription<Args...>();
		auto msg = GetMatcherErrorMessage(a_LuaState, Detail::TypeList<Args...>{});
		res.emplace_back(std::move(Printf("(%s): %s", signature.c_str(), msg.c_str())));
		return res;
	}


	/** Terminator for the function above. */
	static AStringVector BuildMatcherErrorMessages(cLuaState & a_LuaState)
	{
		return AStringVector();
	}



	/** Returns the error message why the specified overload signature cannot be used for current params. */
	template <typename... T>
	static AString GetMatcherErrorMessage(cLuaState & a_LuaState, Detail::TypeList<T...>)
	{
		if (!lua_isnone(a_LuaState, static_cast<int>(sizeof...(T) + 1)))
		{
			return Printf("There are more parameters present (%d) than the signature allows (%u)",
				lua_gettop(a_LuaState),
				static_cast<unsigned>(sizeof...(T))
			);
		}
		if (lua_isnone(a_LuaState, static_cast<int>(sizeof...(T))))
		{
			return Printf("There are not enough parameters present (%d) to match the signature (%u).",
				lua_gettop(a_LuaState),
				static_cast<unsigned>(sizeof...(T))
			);
		}
		return GetParamMatchError(a_LuaState, Detail::TypeList<T...>{}, SizeT<sizeof...(T)>());
	}



	/** Returns the string describing why the specified overload signature doesn't match current params.
	Checks each individual param using compile-time recursion.
	Doesn't check param end (checked by GetMatcherErrorMessage(), which is the only one calling this function). */
	template <typename... T, size_t N>
	static AString GetParamMatchError(cLuaState & a_LuaState, Detail::TypeList<T...>, SizeT<N>)
	{
		// Try to read the param into a dummy variable of the proper type:
		using Type = Detail::RemoveCVR<Detail::Get<N - 1, Detail::TypeList<T...>>>;
		Type Dummy;
		auto res = CheckValueType(a_LuaState, static_cast<int>(N), Dummy);
		if (!res.empty())
		{
			return Printf("Parameter %u: %s", static_cast<unsigned>(N), res.c_str());
		}

		// Reading succeeded, try the next param:
		return GetParamMatchError(a_LuaState, Detail::TypeList<T...>{}, SizeT<N - 1>{});
	}


	/** Terminator for the above function
	We've checked all the params, so this should never be reached. */
	template <typename... T>
	static AString GetParamMatchError(cLuaState & a_LuaState, Detail::TypeList<T...>, SizeT<0>)
	{
		return "[internal matcher error - no reason for mismatch can be found]";
	}



	/** Returns an error message if the value on the specified index on the Lua stack is of the wrong (template) type.
	If the type matches, returns an empty string.
	This is the generic version for regular values. */
	template <typename T>
	static AString CheckValueType(cLuaState & a_LuaState, int a_StackPos, const T & a_Dest)
	{
		typename std::remove_reference<T>::type dummy;
		if (!GetStackValue(a_LuaState, a_StackPos, dummy))
		{
			return Printf("Mismatch, expected %s, got %s",
				GetTypeDescription<T>().c_str(),
				a_LuaState.GetTypeText(a_StackPos).c_str()
			);
		}
		return AString();
	}


	/** Returns an error message if the value on the specified index on the Lua stack is of the wrong (template) type.
	This is the specialization for "cNonNil"-decorated values. */
	template <typename T>
	static AString CheckValueType(cLuaState & a_LuaState, int a_StackPos, const cNonNil<T> & a_Dest)
	{
		if (lua_isnil(a_LuaState, a_StackPos))
		{
			return Printf("Expected a non-nil instance of %s, got a nil",
				GetTypeDescription<T>().c_str(),
				a_LuaState.GetTypeText(a_StackPos).c_str()
			);
		}
		return CheckValueType(a_LuaState, a_StackPos, a_Dest.m_Ptr);
	}


	/** Returns an error message if the value on the specified index on the Lua stack is of the wrong (template) type.
	This is the specialization for "cSelf"-decorated values. */
	template <typename T>
	static AString CheckValueType(cLuaState & a_LuaState, int a_StackPos, const cSelf<T> & a_Dest)
	{
		if (lua_isnil(a_LuaState, a_StackPos))
		{
			return Printf("Expected an instance of %s, got a %s. Did you use the right calling convention?",
				GetTypeDescription<T>().c_str(),
				a_LuaState.GetTypeText(a_StackPos).c_str()
			);
		}
		return CheckValueType(a_LuaState, a_StackPos, a_Dest.m_Ptr);
	}


	/** Returns an error message if the value on the specified index on the Lua stack is of the wrong (template) type.
	This is the specialization for "cStaticSelf"-decorated values. */
	template <typename T>
	static AString CheckValueType(cLuaState & a_LuaState, int a_StackPos, const cStaticSelf<T> & a_Dest)
	{
		tolua_Error err;
		auto type = GetTypeDescription<T>();
		if (lua_isnil(a_LuaState, a_StackPos))
		{
			return Printf("Expected the class %s, got a nil", type.c_str());
		}
		if (tolua_isusertype(a_LuaState, a_StackPos, type.c_str(), 0, &err))
		{
			return Printf("Expected the class %s, got a %s. This function is static, remember to use the right calling convention.",
				type.c_str(),
				a_LuaState.GetTypeText(a_StackPos).c_str()
			);
		}
		return AString();
	}



	/** Returns a string describing the function overload composed of the specified (template) types. */
	template <typename T1, typename T2, typename... T3>
	static AString BuildSingleOverloadDescription()
	{
		return GetTypeDescription<T1>() + ", " + BuildSingleOverloadDescription<T2, T3...>();
	}


	/** Terminator for the above function. */
	template <typename T>
	static AString BuildSingleOverloadDescription()
	{
		return GetTypeDescription<T>();
	}



	/** Returns the Lua type representing the specified C++ type.
	Supports decorators such as cSelf, references and pointers. */
	template <typename T>
	static AString GetTypeDescription(void)
	{
		return Detail::TypeDescription<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::desc();
	}



public:

	/** The main entrypoint for API param reading.
	Tries to match the parameters on the Lua stack onto the specified overloads.
	Each overload is a std::tuple<...> of references which are to be filled with the values read.
	Returns the 0-based index of the first overload that matched the Lua params.
	If no overloads match, raises a Lua error with detailed information about why there was no match.
	The design goals:
		- In case of success, use as little overhead as possible
		- In case of error, provide as much information as possible, even at the cost of huge overhead
	The caller should order the overloads in the order by expected usage - the most used ones first.
	Because the overloads are checked sequentially, this provides the least overhead.
	Uses compile-time template recursion to iterate over all overloads and all their items.

	Call tree:
	Read
	+- ReadInternal <recursive>
	|  +- ReadSingleOverload
	|     +- ReadSingleOverloadRecurse <recursive>
	|        +- GetStackValue
	|           +- cLuaState::GetStackValue
	|           +- CheckValueType [for cStaticSelf]
	+- RaiseError
	|  +- BuildMatcherErrorMessages <recursive>
	|  |  +- BuildSingleOverloadDescription <recursive>
	|  |  |  +- GetTypeDescription
	|  |  |     +- Detail::TypeDescription<T>::desc
	|  |  +- GetMatcherErrorMessage
	|  |     +- GetParamMatchError <recursive>
	|  |        +- CheckValueType
	|  |           +- cLuaState::GetStackValue
	|  +- cLuaState::ApiParamError
	+- return [never reached]
	*/
	template <typename... OverloadLambdas>
	static int Call(cLuaState & a_LuaState, OverloadLambdas &&... a_Overloads)
	{
		// If the reading succeeded, return success
		auto res = CallInternal(a_LuaState, MakeLambdaDesc(a_Overloads)...);
		if (res >= 0)
		{
			return res;
		}

		// The reading failed, raise an error:
		RaiseError(a_LuaState, typename Detail::LambdaSignature<decltype(a_Overloads)>::ArgumentTypes{}...);
		return 0;  // Never reached, but undefined behavior if not present
	}
};





namespace Detail
{
	// Specializations for basic types:
	template <> struct TypeDescription<AString>      { static const char * desc() { return "string"; } };
	template <> struct TypeDescription<const char *> { static const char * desc() { return "string"; } };
	template <> struct TypeDescription<int>          { static const char * desc() { return "number"; } };
	template <> struct TypeDescription<unsigned>     { static const char * desc() { return "number"; } };
	template <> struct TypeDescription<float>        { static const char * desc() { return "number"; } };
	template <> struct TypeDescription<double>       { static const char * desc() { return "number"; } };

	// Specializations for decorated types:
	template <typename T>
	struct TypeDescription<cLuaStateParams::cNonNil<T>>
	{
		static const char * desc() { return TypeDescription<T>::desc(); }
	};

	template <typename T>
	struct TypeDescription<cLuaStateParams::cNonNil<T> &>
	{
		static const char * desc() { return TypeDescription<T>::desc(); }
	};

	template <typename T>
	struct TypeDescription<cLuaStateParams::cSelf<T>>
	{
		static const char * desc() { return TypeDescription<T>::desc(); }
	};

	template <typename T>
	struct TypeDescription<cLuaStateParams::cSelf<T> &>
	{
		static const char * desc() { return TypeDescription<T>::desc(); }
	};

	template <typename T>
	struct TypeDescription<cLuaStateParams::cStaticSelf<T>>
	{
		static const char * desc() { return TypeDescription<T>::desc(); }
	};

	template <typename T>
	struct TypeDescription<cLuaStateParams::cStaticSelf<T> &>
	{
		static const char * desc() { return TypeDescription<T>::desc(); }
	};

	// Include the TypeDecription<T> specializations generated for all known API classes:
	#include "LuaStateParams_TypeDescs.inc"
}




