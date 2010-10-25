/* $Id: xform.c 3186 2006-02-15 18:17:33Z slbrow $

	Example linear transformation function for contour plotter.
*/

#include "plplotP.h"

extern PLFLT tr[];

void 
xform(PLFLT x, PLFLT y, PLFLT * tx, PLFLT * ty)
{
    *tx = tr[0] * x + tr[1] * y + tr[2];
    *ty = tr[3] * x + tr[4] * y + tr[5];
}
