///
/// /file
///
/// /brief Helper to bind pointer to member function and weak pointer.
/// /author Taras Kozlov
/// /todo Use Boost.Move to achieve smooth support for old and new compilers
/// /todo Add usage example

#if !defined(BOOST_PP_IS_ITERATING)
#if !defined(TCL_WEAK_PTR_CLOSURE_INCLUDED)

#define TCL_WEAK_PTR_CLOSURE_INCLUDED

#include <boost/config.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/utility/enable_if.hpp>

#include <boost/function_types/result_type.hpp>
#include <boost/function_types/is_member_function_pointer.hpp>

#ifdef BOOST_NO_VARIADIC_TEMPLATES
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#endif                                                      // BOOST_NO_VARIADIC_TEMPLATES

#ifndef TCL_MAX_CLOSURE_PARAMS
#define TCL_MAX_CLOSURE_PARAMS 10
#endif

// Standard library support for shared_ptr and weak_ptr.
// \todo Move this to separate header
#if defined(__GNUC__)
# if ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3)) || (__GNUC__ > 4)
#  if defined(__GXX_EXPERIMENTAL_CXX0X__)
#   define TCL_HAS_STD_SHARED_PTR
#  endif // defined(__GXX_EXPERIMENTAL_CXX0X__)
# endif // ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3)) || (__GNUC__ > 4)
#endif // defined(__GNUC__)
#if defined(BOOST_MSVC)
# if (_MSC_VER >= 1600)
#  define TCL_HAS_STD_SHARED_PTR
# endif // (_MSC_VER >= 1600)
#endif // defined(BOOST_MSVC)

// Forward declare possible smart pointers
namespace boost {
    template <typename T> class shared_ptr;
    template <typename T> class weak_ptr;
}

#ifdef TCL_HAS_STD_SHARED_PTR
#include <memory>
#endif

namespace tcl { 

/// @brief Define translation from user type to shared and weak pointer types.
///
/// Currently implementation assumes that shared pointer is implicitly convertable to
/// weak pointer, and weak pointer has @c lock method that return shared pointer.
template<typename SmartPtr>
struct smart_ptr_traits;

template<typename T>
struct smart_ptr_traits< boost::shared_ptr<T> >
{
    typedef boost::shared_ptr<T> shared_ptr_type;
    typedef boost::weak_ptr<T> weak_ptr_type;
};

template<typename T>
struct smart_ptr_traits< boost::weak_ptr<T> >
{
    typedef boost::shared_ptr<T> shared_ptr_type;
    typedef boost::weak_ptr<T> weak_ptr_type;
};

template<typename T>
struct smart_ptr_traits< std::shared_ptr<T> >
{
    typedef std::shared_ptr<T> shared_ptr_type;
    typedef std::weak_ptr<T> weak_ptr_type;
};

template<typename T>
struct smart_ptr_traits< std::weak_ptr<T> >
{
    typedef std::shared_ptr<T> shared_ptr_type;
    typedef std::weak_ptr<T> weak_ptr_type;
};
    
namespace detail {

/// @brief Bound pointer to member function and weak_ptr.
///
/// Contains a variadic arg operator()(Args&& args...). 
/// This operator try to lock weak pointer and if succedeed then call specified member function. 
/// If lock failed(object already has been destroyed) then do nothing and return 
/// default constructed return type. Return type is deduced from MemFun type.
template <typename MemFun, typename SharedOrWeakPtr>
struct weak_binder
{
    // member function pointer type
    typedef MemFun                                                 
        member_function_type;

    // eval member function and thus operator() result type
    typedef typename boost::function_types::result_type<member_function_type>::type 
        result_type;

    // eval weak pointer type
    typedef typename smart_ptr_traits<SharedOrWeakPtr>::weak_ptr_type
        weak_ptr_type;

    // eval shared pointer type
    typedef typename smart_ptr_traits<SharedOrWeakPtr>::shared_ptr_type
        shared_ptr_type;

    // instance data
    const member_function_type m_memfun_ptr;
    weak_ptr_type              m_wptr;

    // constructor
    weak_binder(member_function_type memfun, const weak_ptr_type& wptr)
      : m_memfun_ptr(memfun)
      , m_wptr(wptr)
    {}

    // overloaded implementation for operator()(...)
#ifdef BOOST_NO_VARIADIC_TEMPLATES

    // generate set of operator()() with params from 0 to TCL_MAX_CLOSURE_PARAMS
#    define BOOST_PP_ITERATION_PARAMS_1 (3, (0, TCL_MAX_CLOSURE_PARAMS, "weak_ptr_closure.hpp"))
#    include BOOST_PP_ITERATE()

#else

    template<typename... Args>    
    result_type operator()(Args&&... args) const
    {
        if (shared_ptr_type ptr = m_wptr.lock())
            return (ptr.get()->*m_memfun_ptr)(std::forward<Args>(args)...);

        return result_type();
    }

#endif
};

}                                                           // namespace detail

/// Helper function to make @c weak_binder instances
/// @overload
template <typename MemFun, typename SharedOrWeakPtr>
inline detail::weak_binder<MemFun, SharedOrWeakPtr>
weak_ptr_closure(MemFun memfun, SharedOrWeakPtr smart_ptr)
{
    // assure that we have pointer to member function
    BOOST_MPL_ASSERT((boost::function_types::is_member_function_pointer<MemFun>));

    // shared ptr will be implicitly converted to weak ptr here
    return detail::weak_binder<MemFun, SharedOrWeakPtr>(memfun, smart_ptr);
}

}                                                         // namespace tcl

#  endif                                                  // TCL_WEAK_PTR_CLOSURE_INCLUDED

#else                                                     // !defined(BOOST_PP_ITERATING)

#  if BOOST_PP_ITERATION() == 0

/// \todo Add non-const operators as needed
result_type operator()() const
{
    if (shared_ptr_type ptr = m_wptr.lock())
        return (ptr.get()->*m_memfun_ptr)();

    return result_type();
}

#  else

template <BOOST_PP_ENUM_PARAMS(BOOST_PP_ITERATION(), typename P)>
result_type operator()(BOOST_PP_ENUM_BINARY_PARAMS(BOOST_PP_ITERATION(), P, &p)) const
{
    if (shared_ptr_type ptr = m_wptr.lock())
        return (ptr.get()->*m_memfun_ptr)(BOOST_PP_ENUM_PARAMS(BOOST_PP_ITERATION(), p));

    return result_type();
}

#  endif

#endif                                                      // defined(BOOST_PP_ITERATING)
