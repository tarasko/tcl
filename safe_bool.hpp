///
/// /file
///
/// /brief Implementation helper for safe_bool idiom
/// /author Taras Kozlov 

#ifndef TCL_SAFE_BOOL_INCLUDED
#define TCL_SAFE_BOOL_INCLUDED

namespace tcl {

/// \brief Implements safe bool idiom.
/// 
/// This template add safe conversion to bool to derived class. It avoid any 
/// possiblity of unwanted usage in comparisons. Read more about safe bool 
/// idiom on http://www.artima.com/cppsource/safebool.html
/// Derived class must provide boolean_test method which perform actual test
/// is object true or false.
/// 
/// \code 
/// class testable : public safe_bool<testable> 
/// {
///     friend class safe_bool<testable>;
/// 
///     bool boolean_test() const 
///     {
///         return m_val;
///     }
/// 
///     bool m_val;
///
/// public: 
///     testable(bool val) : m_val(val) {}
/// };
/// 
/// int main() 
/// {
///     testable sb1(true);
///     testable sb2(false);
/// 
///     cout << (!sb1 ? true : false) << endl;
///     cout << (!sb2 ? true : false) << endl;
/// 
///     return 0;
/// }
/// \endcode
/// 
/// \tparam T - derived class.
template<class T>
class safe_bool 
{
protected:
    typedef void (safe_bool::*unspecified_bool_type)() const;

    void this_type_does_not_support_comparisons() const {}

    // prevent from creating and deleting such class manually
    safe_bool() {}
    safe_bool(const safe_bool&) {}
    ~safe_bool() {}

public:
    operator unspecified_bool_type() const 
    {
        return static_cast<const T*>(this)->boolean_test() ? 
            &safe_bool::this_type_does_not_support_comparisons : 0;
    }
};

/// \brief Protect safe_bool instances from comparisons
template<typename T>
bool operator!=(const safe_bool<T>& sb1, const safe_bool<T>& sb2) 
{
    sb1.this_type_does_not_support_comparisons();
    return false;
}

/// \brief Protect safe_bool instances from comparisons
template<typename T>
bool operator==(const safe_bool<T>& sb1, const safe_bool<T>& sb2) 
{
    sb1.this_type_does_not_support_comparisons();
    return false;
}

}                                                           // namespace tcl

#endif                                                      // TCL_SAFE_BOOL_INCLUDED
