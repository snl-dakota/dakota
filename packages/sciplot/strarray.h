#ifndef _strarray_h_
#define _strarray_h_
/*
   StringArray
   ===========
   The type |StringArray| represents an array of |String|s, with the
   proviso that by convention the last member of a |StringArray| is
   always a |NULL| pointer. There is a converter that can construct a
   |StringArray| from a single string.


   cvtStringToStringArray
   ======================
   The converter from |String| to |StringArray| makes a copy of the
   passed string and then replaces all occurences of the delimiter
   with a nul byte. The |StringArray| is filled with pointers to the
   parts of the string.

   The delimiter character is the first character in the string.


   newStringArray
   ==============
   The function |newStringArray| makes a copy of a |StringArray|. It
   allocates new space for the array itself and for the strings that
   it contains.


   freeStringArray
   ===============
   |freeStringArray| deallocates the array and all strings it
   contains. Note that this works for StringArrays produced with
   |newStringArray|, but not for those created by
   |cvtStringToStringArray|!

*/


typedef String * StringArray;

Boolean cvtStringToStringArray(
#if NeedFunctionPrototypes
    Display *display,
    XrmValuePtr args,
    Cardinal *num_args,
    XrmValuePtr from,
    XrmValuePtr to,
    XtPointer *converter_data
#endif
);


StringArray newStringArray(
#if NeedFunctionPrototypes
    StringArray a
#endif
);


void freeStringArray(
#if NeedFunctionPrototypes
    StringArray a
#endif
);


#endif /* _strarray_h_ */
