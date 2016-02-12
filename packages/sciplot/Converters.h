#ifndef _Converters_h
#define _Converters_h

/* #ifdef HAVE_CONFIG_H
#include "motif_config.h"
#endif */ /* HAVE_CONFIG_H */

#include "long.h"
#include "icon.h"
#include "XCC.h"
#include "choosecol.h"
#include "strarray.h"
#include "StrToPmap.h"
#include "Pen.h"
#include "str2color.h"

#define done_bert(type, value) \
    do {\
	if (to->addr != NULL) {\
	    if (to->size < sizeof(type)) {\
	        to->size = sizeof(type);\
	        return False;\
	    }\
	    *(type*)(to->addr) = (value);\
        } else {\
	    static type static_val;\
	    static_val = (value);\
	    to->addr = (XtPointer)&static_val;\
        }\
        to->size = sizeof(type);\
        return True;\
    } while (0)

#define done_bob(type, value) \
   {                                    \
       if (toVal->addr != NULL) {       \
          if (toVal->size < sizeof(type)) {\
             toVal->size = sizeof(type);\
             return False;              \
          }                             \
          *(type*)(toVal->addr) = (value);\
       }                                \
       else {                           \
          static type static_val;       \
          static_val = (value);         \
          toVal->addr = (XPointer)&static_val;\
       }                                \
       toVal->size = sizeof(type);      \
   }

#endif /* _Converters_h */
