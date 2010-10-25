#ifndef	_choosecol_h_
#define	_choosecol_h_

/*
 * Not really a `converter', but useful anyway:
 * Compute a color that is `factor' lighter than `base'
 * factor = 1.0 means same color.
 */
Boolean choose_color(
#if NeedFunctionPrototypes
    Widget self,
    double factor,
    Pixel base,
    Pixel* result
#endif
);

void choose_xcc_color(
#if NeedFunctionPrototypes
    Widget self,
    XCC xcc,
    double factor,
    Pixel base,
    Pixel* result
#endif
);

#endif /* _choosecol_h_ */
