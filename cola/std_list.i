/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * std_list.i
 * ----------------------------------------------------------------------------- */

%include <std_common.i>

%{
#include <list>
#include <stdexcept>
%}

namespace std {
    
    template<class T> class list {
      public:
        typedef size_t size_type;
        typedef T value_type;
        typedef const value_type& const_reference;
        list();
        size_type size() const;
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %rename(add) push_back;
        void push_back(const value_type& x);
   };
}

%define specialize_std_list(T)
#warning "specialize_std_list - specialization for type T no longer needed"
%enddef

