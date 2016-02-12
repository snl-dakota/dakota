#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "Converters.h"


Boolean cvtStringToStringArray(display, args, num_args, from, to,
			       converter_data)
    Display *display;
    XrmValuePtr args;
    Cardinal *num_args;
    XrmValuePtr from;
    XrmValuePtr to;
    XtPointer *converter_data;
{
    String t, s;
    StringArray a = NULL;
    Cardinal i;
    char delim;

    if (*num_args != 0)
	XtAppErrorMsg(XtDisplayToApplicationContext(display),
		      "cvtStringToStringArray", "wrongParameters",
		      "XtToolkitError",
		      "String to StringArray conversion needs no arguments",
		      (String*) NULL, (Cardinal*) NULL);

    delim = ((String) from->addr)[0];
    s = XtNewString((String) from->addr + 1);
    i = 0;
    while (s && *s) {
	t = strchr(s, delim);
        if (t) *t = '\0';
	a = (StringArray) XtRealloc((String) a, (i + 1) * sizeof(*a));
	a[i] = s;
	i++;
        s = t ? t + 1 : NULL;
    }
    a = (StringArray) XtRealloc((String) a, (i + 1) * sizeof(*a));
    a[i] = NULL;
    done_bert(StringArray, a);
}


StringArray newStringArray(a)
    StringArray a;
{
    Cardinal n, i;
    StringArray s;

    if (!a) return NULL;
    for (s = a, n = 0; s[n]; n++) ;
    s = (StringArray) XtMalloc((n + 1) * sizeof(*s));
    for (i = 0; i < n; i++) s[i] = XtNewString(a[i]);
    s[n] = NULL;
    return s;
}


void freeStringArray(a)
    StringArray a;
{
    Cardinal i;

    if (!a) return;
    for (i = 0; a[i]; i++) XtFree(a[i]);
    XtFree((String) a);
}



