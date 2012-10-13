/**	@file	Scope guard facilities.
 *	A scope guard is a class that takes care of the resource cleanup - you 
 *	acquire/initialize a resource and pass the responsibility of cleaning up 
 *	the resource to the scope guard that triggers the cleanup in any case 
 *	(i.e. on premature return in a function, when errors are thrown or also 
 *	on successful execution).
 *	
 *	See Andrei's and Petru Marginean's CUJ article
 *	http://www.cuj.com/documents/s=8000/cujcexp1812alexandr/alexandr.htm
 *	and the loki library: 
 *	http://loki-lib.sourceforge.net/index.php?n=Idioms.ScopeGuardPointer
 *	
 *	This implementation provides only a scope guard taking nullary functors.
 *	Andrei's and Marginean's original implementation takes n-ary functors 
 *	and eventually binds the necessary arguments.
 *	
 *	I think that the scope guard is just a trigger for a cleanup functor and 
 *	shouldn't take care of binding a functor's arguments.
 *	This design decision separates the creation and handling of functors from 
 *	the scope guard functionality and passes the responsibility (basically) to 
 *	create the right functors to the programmer.
 *	There exist plenty of possibilities to create nullary functors like from the 
 *	stl (std::bind1st, ...), sigc++ (sigc::bind) or boost (boost::bind) 
 *	(the one from boost are already available in TR1).
 *	
 *	However, this implementation offers an easy possibility to create nullary 
 *	functors by providing a macro that hooks into the tr1/boost bind functionality.
 *	
 *	@author	kj
 *	@date	2007-04-11	kj	created
 */

/**	@def	SCOPE_GUARD_HAVE_TR1_BIND
 *	Define this macro if you have a compiler already providing std::tr1::bind.
 *	If you don't define it, you must have boost installed.
 */


#ifndef _SCOPE_GUARD_H_
#define _SCOPE_GUARD_H_


#include <cstdlib> // std::size_t

#ifdef SCOPE_GUARD_HAVE_TR1_BIND
#  include <tr1/functional>

// copied from boost
#  define SCOPE_GUARD_JOIN( X, Y ) SCOPE_GUARD_DO_JOIN( X, Y )
#  define SCOPE_GUARD_DO_JOIN( X, Y ) SCOPE_GUARD_DO_JOIN2(X,Y)
#  define SCOPE_GUARD_DO_JOIN2( X, Y ) X##Y

#else // SCOPE_GUARD_HAVE_TR1_BIND, don't have tr1
#  include <boost/config.hpp> // BOOST_JOIN
#  include <boost/bind.hpp>

#  define SCOPE_GUARD_JOIN BOOST_JOIN
#endif // SCOPE_GUARD_HAVE_TR1_BIND


namespace util
{

/**	@short Base class used by all scope_guard implementations.  All commonly used
 *	functions are in this class (e.g. - dismiss and safe_execute).
 */
class scope_guard_base/*: nonassignable, nonheapcreatable, nonpointeraliasing*/
{
    /// Copy-assignment operator is not implemented and private.
    scope_guard_base& operator =(const scope_guard_base&);
	/// can't create on the heap
	void* operator new(std::size_t);
	/// can't pointer alias
	scope_guard_base* operator &();
	/// can't const pointer alias
	const scope_guard_base* operator &() const;


protected:
    scope_guard_base() throw()
		: m_dismissed() 
    {}

    ~scope_guard_base()
    {}

    /// Copy-ctor takes over responsibility from other scope_guard.
    scope_guard_base(const scope_guard_base& other) throw() 
        : m_dismissed(other.m_dismissed)
    {
        other.dismiss();
    }

    template<typename T_janitor>
    static void safe_execute(T_janitor& j) throw() 
    {
        if (!j.m_dismissed)
		{
            try {	j.execute();	}
            catch(...)
            {}
		}
	}
    
public:
    void dismiss() const throw() 
    {
        m_dismissed = true;
    }


private:
    mutable bool m_dismissed;
};


/**	
 *	Implementation class for a nullary functor (without parameters).
 *	scope_guard0 ignores any value returned from the
 *	call within the execute functor.
 *	
 *	This class has a single standalone helper function, make_guard which
 *	creates and returns a scope_guard, and a helper macro, MAKE_GUARD
 *	which helps in creating a nullary functor.
 */
template<typename T_functor>
class scope_guard0: public scope_guard_base
{
public:
    explicit scope_guard0(T_functor functor)
		: m_functor(functor)
    {}

    ~scope_guard0() throw() 
    {
        scope_guard_base::safe_execute(*this);
    }

    void execute() 
    {
        m_functor();
    }

protected:
	T_functor m_functor;
};


/**	@short	Factory template function to create easily a scope guard 
 *	from a nullary functor.
 *	
 *	Note that you must pass a nullary functor. There are plenty
 *	possilibities for functor creation in the stl, sigc++ or boost.
 *	Or you can use the MAKE_GUARD macro.
 */
template<typename T_functor> 
inline scope_guard0<T_functor> 
make_guard(T_functor functor)
{
	return scope_guard0<T_functor>(functor);
}


/**	@short	The scope guard variable type to use;
 *	
 *	A const reference ensures execution of the correct dtor.
 *	Use it like this:
 *	@code
 *	scope_guard g = MAKE_GUARD((&fclose, hFile));
 *	@endcode
 */
typedef const scope_guard_base& scope_guard;


} // namespace sg



/**	@def SOME_SCOPE_GUARD
 *	@short	Macro to create a unique (per line) anonymous scope guard variable.
 *	
 *	Use this macro if you don't need a variable to refer to the scope guard, 
 *	but want to have control over the creation of the nullary functor.
 *	
 *	@code
 *	// want to have a unique anonymous scope guard and 
 *	// have a functor that takes no arguments and we don't want to use 
 *	// ON_SCOPE_EXIT that wraps our functor in a boost bind functor.
 *	SOME_SCOPE_GUARD = make_guard(no_arg_functor);
 *	// want to have a unique anonymous scope guard but also want to control
 *	// the creation of the functor to trigger
 *	SOME_SCOPE_GUARD = make_guard(sigc::bind(&fclose, hFile));
 *	// more explicit version than with ON_SCOPE_EXIT (see below)
 *	SOME_SCOPE_GUARD = MAKE_GUARD((&fclose, hFile));
 *	@endcode
 */
#if (defined _MSC_VER) && (_MSC_VER < 1300)
// __LINE__ macro broken when -ZI is used see Q199057
#pragma message("Note that we can't create unique anonymous scope guards when -ZI is used (program database with edit and continue)")
#pragma message("because the __LINE__ macro is broken, see Q199057")
#define SOME_SCOPE_GUARD\
	::util::scope_guard SCOPE_GUARD_JOIN(scopeGuard, __LINE__)

#elif (defined _MSC_VER)
// __LINE__ macro broken when -ZI is used see Q199057
// but we can use __COUNTER__ instead
#define SOME_SCOPE_GUARD\
	::util::scope_guard SCOPE_GUARD_JOIN(scopeGuard, __COUNTER__)

#else
#define SOME_SCOPE_GUARD\
	::util::scope_guard SCOPE_GUARD_JOIN(scopeGuard, __LINE__)

#endif


/**	@short	Macro to easily create a unique (per line) anonymous scope guard.
 *	
 *	Use this macro if you don't need a variable to refer to the scope guard.
 *	@code
 *	ON_SCOPE_EXIT((&fclose, hFile));
 *	@endcode
 *	
 *	@note	You can pass nullary functors, too.
 */
#define ON_SCOPE_EXIT\
	SOME_SCOPE_GUARD = MAKE_GUARD

/**	@def	MAKE_GUARD
 *	@short	Macro to ease the creation of a scope guard for functors with arguments.
 *	
 *	@note	This macro hooks into the tr1 or boost bind facility, so you can 
 *			pass nullary functors, too.
 *	@note	This macro expects ONE argument - the tuple consisting of the 
 *			functor and eventual arguments, enclosed in parantheses (The 
 *			preprocessor will see then just one argument).
 *	e.g.
 *	@code
 *	// note the extra parantheses for the tuple
 *	scope_guard g = MAKE_GUARD((&fclose, hFile));
 *	// nullary functor
 *	scope_guard g = MAKE_GUARD((nullary_functor));
 *	@endcode
 */
#ifdef SCOPE_GUARD_HAVE_TR1_BIND
// create a tr1::bind function call from the tuple 
#define MAKE_GUARD(fun_n_args_tuple)\
	::util::make_guard(::std::tr1::bind fun_n_args_tuple)

#else
// create a boost::bind function call from the tuple 
#define MAKE_GUARD(fun_n_args_tuple)\
	::util::make_guard(::boost::bind fun_n_args_tuple)

#endif // SCOPE_GUARD_HAVE_TR1_BIND, MAKE_GUARD


#endif // end file guardian
