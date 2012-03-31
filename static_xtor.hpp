///
/// /file
///
/// /brief Class static_xtor (interface)
/// /author Alex Turbov

#ifndef TCL_STATIC_XTOR_INCLUDED
#define TCL_STATIC_XTOR_INCLUDED

namespace tcl {

///
/// @brief Static constructor/destructor idiom
///
/// To use static constructor/destructor user must inherit static_xtor
/// instantiated with derived class name (yeah, CTRP used). Also derived
/// class have to define 2 static functions named @c static_ctor and @c static_dtor
/// in private/protected class section. And finally to grant access to thouse
/// functions, class @c static_xtor<Derived>::access must be declared as friend.
///
/// @code
/// template <typename FinalHandler>
/// class message_handler_base
///   : public tcl::static_xtor<message_handler_base>        // Provides static initialization
///   , public message_handler_iface                         // Provides polimorphic behaviour
/// {
///      friend class tcl::static_xtor<message_handler_base>::access;
///      static void static_ctor()
///      {
///          // Register concrete message handler (FinalHandler)
///          // in a private static map of handlers...
///      }
///      static void static_dtor()
///      {
///      }
/// };
/// @endcode
///
template <typename Derived, typename Target = Derived>
class static_xtor
{
protected:
    /// @todo It would be nice to be able to recognize if
    /// @c Target class really has both required static methods
    /// (@c static_ctor and/or @c static_dtor), and if some missed
    /// do not try to call it... Its actually easy to implement, @b BUT
    /// nowadays top gcc 4.4.2 has a bug: if helper is template
    /// @c has_static_static_ctor (which can easily be generated) 
    /// returns false (true expected) for end-user classes 
    /// (which are really define required function).
    class access
    {
    public:
        access()
        {
            Target::static_ctor();
        }
        ~access()
        {
            Target::static_dtor();
        }
    };

    virtual ~static_xtor() {}  // get rid of gcc warnings

private:
    static access s_helper;

    // We use two overlapped ways to guarantee that s_helper will be created and 
    // initialized. 

    // 1. Static function that access s_helper.
    // And template instantiated with pointer to static function that access s_helper.
    static void use_helper()
    {
        (void)s_helper;
    }

    template <void(*)()>
    struct helper2 {};

    helper2<&static_xtor::use_helper> s_helper2;

    // 2. Virtual function that access s_helper. This is more robust and actually works 
    // almost always.
    virtual void use_helper2()
    {
        (void)s_helper;
    }
};

template <typename Derived, typename Target>
typename static_xtor<Derived, Target>::access static_xtor<Derived, Target>::s_helper;

}                                                           // namespace tcl
#endif                                                      // TCL_STATIC_XTOR_INCLUDED
