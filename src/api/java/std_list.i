/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/* -----------------------------------------------------------------------------
 * std_list.i
 * ----------------------------------------------------------------------------- */

/* This adds only basic support for std::list to Java. Ideally there would be a
 * way to get a Java iterator for the list. */

%{
#include <list>
%}

namespace std {

    template<class T> class list {
      public:
        typedef size_t size_type;
        typedef T value_type;
        typedef const value_type& const_reference;
        list();
        list(size_type n);
        size_type size() const;
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        void reverse();
        %rename(addFirst) push_front;
        void push_front(const value_type& x);
        %rename(addLast) push_back;
        void push_back(const value_type& x);
        %rename(getFirst) front;
        const_reference front();
        %rename(getLast) back;
        const_reference back();
        %rename(clearLast) pop_back;
        void pop_back();
        %rename(clearFirst) pop_front;
        void pop_front();
   };
}
