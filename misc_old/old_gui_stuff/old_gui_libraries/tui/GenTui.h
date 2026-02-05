class MutexRaw;

typedef MutexRaw* Mutex;

Mutex NewMutex();
void LockMutex(Mutex m);
void ReleaseMutex(Mutex m);
void DeleteMutex(Mutex m);

class MutexLocker
{
	Mutex m;

	MutexLocker(const MutexLocker&);
	void operator = (const MutexLocker&);

public:
	MutexLocker(Mutex m_):m(m_)
	{
		if(m)
			LockMutex(m);
	}
	
	~MutexLocker()
	{
		if(m)
			ReleaseMutex(m);
	}
};



struct ThreadFunctor
{
	virtual void Apply()=0;

	virtual ~ThreadFunctor(){};
};

void SystemNewThread(ThreadFunctor* pF);

struct ThreadFunctor0: public ThreadFunctor
{
	void (*fn)();


	ThreadFunctor0(void (*fn_)())
		:fn(fn_){}

	/*virtual*/ void Apply()
	{
		fn();
	}
};

inline void NewThread(void (*fn)())
{
	ThreadFunctor* pF = new ThreadFunctor0(fn);
	SystemNewThread(pF);
}

template <class T0>
struct ThreadFunctor1: public ThreadFunctor
{
	void (*fn)(T0);

	T0 arg0;

	ThreadFunctor1(void (*fn_)(T0), T0 arg0_)
		:fn(fn_), arg0(arg0_){}

	/*virtual*/ void Apply()
	{
		fn(arg0);
	}
};

template <class T0>
void NewThread(void (*fn)(T0), T0 arg0)
{
	ThreadFunctor* pF = new ThreadFunctor1<T0>(fn, arg0);
	SystemNewThread(pF);
}

template <class T0, class T1>
struct ThreadFunctor2: public ThreadFunctor
{
	void (*fn)(T0, T1);

	T0 arg0;
	T1 arg1;

	ThreadFunctor2(void (*fn_)(T0, T1), T0 arg0_, T1 arg1_)
		:fn(fn_), arg0(arg0_), arg1(arg1_){}

	/*virtual*/ void Apply()
	{
		fn(arg0, arg1);
	}
};

template <class T0, class T1>
void NewThread(void (*fn)(T0, T1), T0 arg0, T1 arg1)
{
	ThreadFunctor* pF = new ThreadFunctor2<T0, T1>(fn, arg0, arg1);
	SystemNewThread(pF);
}

template <class T0, class T1, class T2>
struct ThreadFunctor3: public ThreadFunctor
{
	void (*fn)(T0, T1, T2);

	T0 arg0;
	T1 arg1;
	T2 arg2;

	ThreadFunctor3(void (*fn_)(T0, T1, T2), T0 arg0_, T1 arg1_, T2 arg2_)
		:fn(fn_), arg0(arg0_), arg1(arg1_), arg2(arg2_){}

	/*virtual*/ void Apply()
	{
		fn(arg0, arg1, arg2);
	}
};

template <class T0, class T1, class T2>
void NewThread(void (*fn)(T0, T1, T2), T0 arg0, T1 arg1, T2 arg2)
{
	ThreadFunctor* pF = new ThreadFunctor3<T0, T1, T2>(fn, arg0, arg1, arg2);
	SystemNewThread(pF);
}

template <class T0, class T1, class T2, class T3>
struct ThreadFunctor4: public ThreadFunctor
{
	void (*fn)(T0, T1, T2, T3);

	T0 arg0;
	T1 arg1;
	T2 arg2;
	T3 arg3;

	ThreadFunctor4(void (*fn_)(T0, T1, T2, T3), T0 arg0_, T1 arg1_, T2 arg2_, T3 arg3_)
		:fn(fn_), arg0(arg0_), arg1(arg1_), arg2(arg2_), arg3(arg3_){}

	/*virtual*/ void Apply()
	{
		fn(arg0, arg1, arg2, arg3);
	}
};

template <class T0, class T1, class T2, class T3>
void NewThread(void (*fn)(T0, T1, T2, T3), T0 arg0, T1 arg1, T2 arg2, T3 arg3)
{
	ThreadFunctor* pF = new ThreadFunctor4<T0, T1, T2, T3>(fn, arg0, arg1, arg2, arg3);
	SystemNewThread(pF);
}

template <class T0, class T1, class T2, class T3, class T4>
struct ThreadFunctor5: public ThreadFunctor
{
	void (*fn)(T0, T1, T2, T3, T4);

	T0 arg0;
	T1 arg1;
	T2 arg2;
	T3 arg3;
	T4 arg4;

	ThreadFunctor5(void (*fn_)(T0, T1, T2, T3, T4), T0 arg0_, T1 arg1_, T2 arg2_, T3 arg3_, T4 arg4_)
		:fn(fn_), arg0(arg0_), arg1(arg1_), arg2(arg2_), arg3(arg3_), arg4(arg4_){}

	/*virtual*/ void Apply()
	{
		fn(arg0, arg1, arg2, arg3, arg4);
	}
};

template <class T0, class T1, class T2, class T3, class T4>
void NewThread(void (*fn)(T0, T1, T2, T3, T4), T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
	ThreadFunctor* pF = new ThreadFunctor5<T0, T1, T2, T3, T4>(fn, arg0, arg1, arg2, arg3, arg4);
	SystemNewThread(pF);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
struct ThreadFunctor6: public ThreadFunctor
{
	void (*fn)(T0, T1, T2, T3, T4, T5);

	T0 arg0;
	T1 arg1;
	T2 arg2;
	T3 arg3;
	T4 arg4;
	T5 arg5;

	ThreadFunctor6(void (*fn_)(T0, T1, T2, T3, T4, T5), T0 arg0_, T1 arg1_, T2 arg2_, T3 arg3_, T4 arg4_, T5 arg5_)
		:fn(fn_), arg0(arg0_), arg1(arg1_), arg2(arg2_), arg3(arg3_), arg4(arg4_), arg5(arg5_){}

	/*virtual*/ void Apply()
	{
		fn(arg0, arg1, arg2, arg3, arg4, arg5);
	}
};

template <class T0, class T1, class T2, class T3, class T4, class T5>
void NewThread(void (*fn)(T0, T1, T2, T3, T4, T5), T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
	ThreadFunctor* pF = new ThreadFunctor6<T0, T1, T2, T3, T4, T5>(fn, arg0, arg1, arg2, arg3, arg4, arg5);
	SystemNewThread(pF);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
struct ThreadFunctor7: public ThreadFunctor
{
	void (*fn)(T0, T1, T2, T3, T4, T5, T6);

	T0 arg0;
	T1 arg1;
	T2 arg2;
	T3 arg3;
	T4 arg4;
	T5 arg5;
	T6 arg6;

	ThreadFunctor7(void (*fn_)(T0, T1, T2, T3, T4, T5, T6), T0 arg0_, T1 arg1_, T2 arg2_, T3 arg3_, T4 arg4_, T5 arg5_, T6 arg6_)
		:fn(fn_), arg0(arg0_), arg1(arg1_), arg2(arg2_), arg3(arg3_), arg4(arg4_), arg5(arg5_), arg6(arg6_){}

	/*virtual*/ void Apply()
	{
		fn(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
	}
};

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
void NewThread(void (*fn)(T0, T1, T2, T3, T4, T5, T6), T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
{
	ThreadFunctor* pF = new ThreadFunctor7<T0, T1, T2, T3, T4, T5, T6>(fn, arg0, arg1, arg2, arg3, arg4, arg5, arg6);
	SystemNewThread(pF);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
struct ThreadFunctor8: public ThreadFunctor
{
	void (*fn)(T0, T1, T2, T3, T4, T5, T6, T7);

	T0 arg0;
	T1 arg1;
	T2 arg2;
	T3 arg3;
	T4 arg4;
	T5 arg5;
	T6 arg6;
	T7 arg7;

	ThreadFunctor8(void (*fn_)(T0, T1, T2, T3, T4, T5, T6, T7), T0 arg0_, T1 arg1_, T2 arg2_, T3 arg3_, T4 arg4_, T5 arg5_, T6 arg6_, T7 arg7_)
		:fn(fn_), arg0(arg0_), arg1(arg1_), arg2(arg2_), arg3(arg3_), arg4(arg4_), arg5(arg5_), arg6(arg6_), arg7(arg7_){}

	/*virtual*/ void Apply()
	{
		fn(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
	}
};

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
void NewThread(void (*fn)(T0, T1, T2, T3, T4, T5, T6, T7), T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7)
{
	ThreadFunctor* pF = new ThreadFunctor8<T0, T1, T2, T3, T4, T5, T6, T7>(fn, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
	SystemNewThread(pF);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
struct ThreadFunctor9: public ThreadFunctor
{
	void (*fn)(T0, T1, T2, T3, T4, T5, T6, T7, T8);

	T0 arg0;
	T1 arg1;
	T2 arg2;
	T3 arg3;
	T4 arg4;
	T5 arg5;
	T6 arg6;
	T7 arg7;
	T8 arg8;

	ThreadFunctor9(void (*fn_)(T0, T1, T2, T3, T4, T5, T6, T7, T8), T0 arg0_, T1 arg1_, T2 arg2_, T3 arg3_, T4 arg4_, T5 arg5_, T6 arg6_, T7 arg7_, T8 arg8_)
		:fn(fn_), arg0(arg0_), arg1(arg1_), arg2(arg2_), arg3(arg3_), arg4(arg4_), arg5(arg5_), arg6(arg6_), arg7(arg7_), arg8(arg8_){}

	/*virtual*/ void Apply()
	{
		fn(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
	}
};

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
void NewThread(void (*fn)(T0, T1, T2, T3, T4, T5, T6, T7, T8), T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8)
{
	ThreadFunctor* pF = new ThreadFunctor9<T0, T1, T2, T3, T4, T5, T6, T7, T8>(fn, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
	SystemNewThread(pF);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
struct ThreadFunctor10: public ThreadFunctor
{
	void (*fn)(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9);

	T0 arg0;
	T1 arg1;
	T2 arg2;
	T3 arg3;
	T4 arg4;
	T5 arg5;
	T6 arg6;
	T7 arg7;
	T8 arg8;
	T9 arg9;

	ThreadFunctor10(void (*fn_)(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9), T0 arg0_, T1 arg1_, T2 arg2_, T3 arg3_, T4 arg4_, T5 arg5_, T6 arg6_, T7 arg7_, T8 arg8_, T9 arg9_)
		:fn(fn_), arg0(arg0_), arg1(arg1_), arg2(arg2_), arg3(arg3_), arg4(arg4_), arg5(arg5_), arg6(arg6_), arg7(arg7_), arg8(arg8_), arg9(arg9_){}

	/*virtual*/ void Apply()
	{
		fn(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
	}
};

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
void NewThread(void (*fn)(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9), T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9)
{
	ThreadFunctor* pF = new ThreadFunctor10<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(fn, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
	SystemNewThread(pF);
}

