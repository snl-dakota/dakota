/******************************************************************************
 *
 * File:           istack.h
 *
 * Created:        06/06/2001
 *
 * Author:         Pavel Sakov
 *                 CSIRO Marine Research
 *
 * Purpose:        Header for handling stack of integers.
 *
 * Description:    None
 *
 * Revisions:      None
 *
 *****************************************************************************/

#if !defined(_ISTACK_H)
#define _ISTACK_H

typedef struct {
    int n;
    int nallocated;
    int* v;
} istack;

int istack_contains(istack* s, int v);
istack* istack_create();
void istack_destroy(istack* s);
void istack_push(istack* s, int v);
int istack_pop(istack* s);
void istack_reset(istack* s);

#endif
