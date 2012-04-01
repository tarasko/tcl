/// \file construct_destroy.hpp
/// Define \c construct, \c construct_array, \c destroy, \c destroy_array templates
/// that allocates/deallocates memory from provided allocator and explicitly
/// create/destroy objects on that memory.
///
/// \c construct and \c construct_array forwards additional parameters to object
/// constructor. If compiler doesn`t support variadic templates then it is allowed to
/// forward only up to TCL_ALLOCATORS_CONSTRUCT_MAX_PARAMS parameters.
/// By default it is 5. Redefine it if you need more.
///
/// Both \c construct and \c construct_array provides strong exception guarantee and
/// behave like operator new. I.e when object constructor throw exception,
/// \c construct_array call destructor for all previosly constructed objects, and then
/// both functions will deallocate memory and rethrow exception
///
/// \todo Add noexcept to destroy and destroy_array

#if !BOOST_PP_IS_ITERATING

#ifndef TCL_ALLOCATORS_CONSTRUCT_DESTROY_HPP
#define TCL_ALLOCATORS_CONSTRUCT_DESTROY_HPP

#include <boost/config.hpp>

#ifdef BOOST_NO_VARIADIC_TEMPLATES

#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>

#ifndef TCL_ALLOCATORS_CONSTRUCT_MAX_PARAMS
#define TCL_ALLOCATORS_CONSTRUCT_MAX_PARAMS 5
#endif

#else  // #ifdef BOOST_NO_VARIADIC_TEMPLATES

#include <utility>

#endif // #ifdef BOOST_NO_VARIADIC_TEMPLATES

namespace tcl { namespace allocators {

template<typename Allocator>
void destroy(Allocator& al, typename Allocator::pointer p)
{
    typedef typename Allocator::value_type value_type;
    p->~value_type();
    al.deallocate(p, 1);
}

template<typename Allocator>
void destroy_array(
    Allocator& al
  , typename Allocator::pointer p
  , typename Allocator::size_type array_size
  )
{
    typedef typename Allocator::size_type  size_type;
    typedef typename Allocator::value_type value_type;

    for(size_type i = 0; i < array_size; ++i)
        p[i].~value_type();

    al.deallocate(p, array_size);
}

#ifdef BOOST_NO_VARIADIC_TEMPLATES

#define RVAL_CAST(Z, N, ARG) static_cast<Arg ## N &&>(arg ## N)

#define BOOST_PP_ITERATION_PARAMS_1 (3, (0, TCL_ALLOCATORS_CONSTRUCT_MAX_PARAMS, "construct_destroy.hpp"))
#include BOOST_PP_ITERATE()

#undef RVAL_CAST

#else // #ifdef BOOST_NO_VARIADIC_TEMPLATES

template<typename Allocator, typename ... Args>
typename Allocator::pointer
construct(
    Allocator& allocator
  , Args&& ... args
  )
{
    typedef typename Allocator::pointer pointer;
    typedef typename Allocator::value_type value_type;

    pointer p = allocator.allocate(1);
    try {
        new (p) value_type(std::forward<Args>(args) ...);
    }
    catch(...) {
        allocator.deallocate(p, 1);
        throw;
    }

    return p;
}

template<typename Allocator, typename ... Args>
typename Allocator::pointer
construct_array(
    Allocator& allocator
  , typename Allocator::size_type array_size
  , Args&& ... args
  )
{
    typedef typename Allocator::pointer    pointer;
    typedef typename Allocator::value_type value_type;
    typedef typename Allocator::size_type  size_type;

    pointer p = allocator.allocate(array_size);
    size_type i = 0;
    try {
        for (; i < array_size; ++i)
            new (p + i) value_type(std::forward<Args>(args) ...);
    }
    catch(...) {
        // In case of exception we must call destructors for previously
        // constructed objects
        for (size_type k = 0; k < i; ++k)
            p[i].~value_type();

        allocator.deallocate(p, array_size);
        throw;
    }

    return p;
}

#endif // #ifdef BOOST_NO_VARIADIC_TEMPLATES

}} // namespace tcl { namespace allocators {

#endif // #ifndef TCL_ALLOCATORS_CONSTRUCT_DESTROY_HPP

#else // #if !BOOST_PP_IS_ITERATING

template<typename Allocator BOOST_PP_COMMA_IF(BOOST_PP_ITERATION()) BOOST_PP_ENUM_PARAMS(BOOST_PP_ITERATION(), typename Arg)>
typename Allocator::pointer
construct(
    Allocator& allocator
    BOOST_PP_COMMA_IF(BOOST_PP_ITERATION())
    BOOST_PP_ENUM_BINARY_PARAMS(BOOST_PP_ITERATION(), Arg, &&arg)
  )
{
    typedef typename Allocator::pointer pointer;
    typedef typename Allocator::value_type value_type;

    pointer p = allocator.allocate(1);
    try {
        new (p) value_type(BOOST_PP_ENUM(BOOST_PP_ITERATION(), RVAL_CAST, zzz));
    }
    catch(...) {
        allocator.deallocate(p, 1);
        throw;
    }

    return p;
}

template<typename Allocator BOOST_PP_COMMA_IF(BOOST_PP_ITERATION()) BOOST_PP_ENUM_PARAMS(BOOST_PP_ITERATION(), typename Arg)>
typename Allocator::pointer
construct_array(
    Allocator& allocator
  , typename Allocator::size_type array_size
    BOOST_PP_COMMA_IF(BOOST_PP_ITERATION())
    BOOST_PP_ENUM_BINARY_PARAMS(BOOST_PP_ITERATION(), Arg, &&arg)
  )
{
    typedef typename Allocator::pointer    pointer;
    typedef typename Allocator::value_type value_type;
    typedef typename Allocator::size_type  size_type;

    pointer p = allocator.allocate(array_size);
    size_type i = 0;
    try {
        for (; i < array_size; ++i)
            new (p + i) value_type(BOOST_PP_ENUM(BOOST_PP_ITERATION(), RVAL_CAST, zzz));
    }
    catch(...) {
        // In case of exception we must call destructors for previously
        // constructed objects
        for (size_type k = 0; k < i; ++k)
            p[i].~value_type();

        allocator.deallocate(p, array_size);
        throw;
    }

    return p;
}

#endif // #if !BOOST_PP_IS_ITERATING
