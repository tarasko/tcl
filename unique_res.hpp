///
/// /file
///
/// /brief General RAII wrapper with move semantic for uniform resource.
///
/// /author Taras Kozlov 

#ifndef TCL_UNIQUE_RES_INCLUDED
#define TCL_UNIQUE_RES_INCLUDED

#include "safe_bool.hpp"

#include <boost/typeof/typeof.hpp>
#include <boost/move/move.hpp>

namespace tcl {

/// \brief Define moveable-only type that exploits RAII to free wrapped resource on scope exit.
///
/// @c Object of this type can be used in boolean context, it become false when value
/// is equal to NullValue.@n
/// @c Object of this type provide @c ptr method to couple with some stupid functions,
/// like OpenProcessToken that return resource within params.
/// Example:
/// @code 
///
/// typedef TCL_UNIQUE_RES(HANDLE, NULL, CloseHandle) handle;
/// handle h1(CreateEvent(...));
/// h1.reset(CreateEvent(...)) // free previously stored handle, assign new
///
/// {
///     handle h2;                     // default constructible
///     h2.reset(CreateEvent(...));    // assign handle
///     SetEvent(h2.get());            // access to stored handle
///
///     if (h2) {                      // works as boolean
///         ...
///     }
/// } // free handle on scope exit using CloseHandle
///
/// // using ptr method to couple will some stupid API
/// handle h3;
/// OpenProcessToken(GetCurrentProcess(), GENERIC_READ, h3.ptr());
/// 
/// // can be moved and returned from function
/// handle create_event() 
/// {
///     // create, do something and return
///     handle h = CreateEvent(...);
///     SetEvent(h.get());
///     return boost::move(h);           // std::move can be used too if available
///     ...
///     return handle(CreateEvent(...)); // this is possible 
///     ...
///     return CreateEvent(...);         // and this, it is equivalent to previous one
/// }
///
/// handle_t ev(create_event());     // move constructor used
/// ev = create_event();             // move assignment operator used
///
/// HANDLE h = ev.release();
/// assert(!ev);                     // check that resource is NULL after release
///
/// @endcode
///
/// @tparam Type - resource type
/// @tparam NullValueType - type of NullValue parameter
/// @tparam NullValue - value that means invalid or not existent resource
/// @tparam CleanerType - type of Cleaner parameter
/// @tparam Cleaner - Cleaner(val), free resource val of type Type
///
///
template<
    typename T
  , typename NullValueType
  , NullValueType NullValue
  , typename CleanerType
  , CleanerType Cleaner
  >
class unique_res : public ::tcl::safe_bool< unique_res<T, NullValueType, NullValue, CleanerType, Cleaner> > 
{
    typedef unique_res<T, NullValueType, NullValue, CleanerType, Cleaner> self_type;

    BOOST_MOVABLE_BUT_NOT_COPYABLE(unique_res)
    struct proxy;

public:
    typedef T value_type;

    static value_type null_value() 
    {
        return static_cast<T>(NullValue);
    }

    /// Constructor
    unique_res(value_type v = null_value()) : val_(v) 
    {
    }

    /// Destructor
    ~unique_res()
    {
        if (null_value() != val_)
            Cleaner(val_);
    }

    /// Move constructor 
    unique_res(BOOST_RV_REF(unique_res) other) : val_(other.val_)
    {
        other.val_ = null_value();
    }

    /// Move assignment
    unique_res& operator=(BOOST_RV_REF(unique_res) other)
    {
        val_ = other.val_;
        other.val_ = null_value();
        return *this;
    }

    /// Resource accessor
    value_type get() const 
    {
        return val_;
    }

    /// Return proxy type that is implicitly convertible to pointer to value_type
    /// Upon destruction proxy check if contained value has changed and assign 
    /// new value to parent unique_res object.
    proxy ptr() 
    {
        return proxy(this);
    }

    /// Free managed resource using cleaner, assign new resource
    void reset(value_type val = null_value()) 
    {
        if (null_value() != val_)
            Cleaner(val_);

        val_ = val;
    }

    /// Release ownership of managed resource, return resource
    value_type release() 
    {
        value_type tmp = val_;
        val_ = null_value();
        return tmp;
    }

private:
#ifdef _MSC_VER
    friend ::tcl::safe_bool<self_type>;
#else
    friend class ::tcl::safe_bool<self_type>;
#endif

    bool boolean_test() const 
    {
        return null_value() != val_;
    }

    // convertible to T*, object of proxy returned by ptr method
    struct proxy
    {
        proxy(unique_res* r) : r_(r), val_(r->get()) 
        {
        }

        ~proxy() 
        {
            if (r_->get() != val_)
                r_->reset(val_);
        }

        operator value_type*() 
        {
            return &val_;
        }

    private:
        unique_res* r_;
        value_type val_;
    };

    value_type val_;
};

}                                                           // namespace tcl

/// Syntactic sugar macro for defining unique_res type
#define TCL_UNIQUE_RES(Type, NullValue, Cleaner) ::tcl::unique_res<Type, BOOST_TYPEOF_TPL(NullValue), NullValue, BOOST_TYPEOF_TPL(&Cleaner), &Cleaner>

#endif 												        // TCL_UNIQUE_RES_INCLUDED
