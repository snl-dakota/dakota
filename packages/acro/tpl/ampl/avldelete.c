/****************************************************************
Copyright (C) 2007 David M. Gay

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,

provided that the above copyright notice appear in all copies and that
both that the copyright notice and this permission notice and warranty
disclaimer appear in supporting documentation.

The author disclaims all warranties with regard to this software,
including all implied warranties of merchantability and fitness.
In no event shall the author be liable for any special, indirect or
consequential damages or any damages whatsoever resulting from loss of
use, data or profits, whether in an action of contract, negligence or
other tortious action, arising out of or in connection with the use or
performance of this software.
****************************************************************/

#define AVL_TREE_JUST_IMPL_DECLS
#include "avltree.c"

 static int
avl_delhelp(Node **pA, AVL_Tree *T, const Element **pE, int rt)
{
	Node *A, *B, *C, *D, *E, **pB, **pC;
	int hB, hC, hD, hE;

	A = *pA;
	if (rt) {
		pB = &A->left;
		pC = &A->right;
		}
	else {
		pB = &A->right;
		pC = &A->left;
		}
	if (!*pB) {
		*pA = *pC;
		*pE = A->elem;
		A->elem = 0;
		A->right = 0;
		A->left = T->efree;
		T->efree = A;
		}
	else if (!avl_delhelp(pB, T, pE, rt))
		return 0;
	else {
		hB = hC = 0;
		if ((B = *pB))
			hB = B->height;
		if ((C = *pC))
			hC = C->height;
		if (hB >= hC - 1)
			return 0;
		hD = hE = 0;
		if (rt) {
			if ((D = C->left))
				hD = D->height;
			if ((E = C->right))
				hE = E->height;
			if (hE > hD) {
				/* easy rotation */
				C->left = A;
				A->right = D;
				A->height = hD + 1;
				*pA = C;
				return 1;
				}
			/* hard rotation */
			A->right = D->left;
			A->height = hB + 1;
			C->left = D->right;
			C->height = D->height++;
			D->right = C;
			D->left = A;
			*pA = D;
			}
		else {
			if ((D = C->right))
				hD = D->height;
			if ((E = C->left))
				hE = E->height;
			if (hE > hD) {
				/* easy rotation */
				C->right = A;
				A->left = D;
				A->height = hD + 1;
				*pA = C;
				return 1;
				}
			/* hard rotation */
			A->left = D->right;
			A->height = hB + 1;
			C->right = D->left;
			C->height = D->height++;
			D->left = C;
			D->right = A;
			*pA = D;
			}
		}
	return 1;
	}

 static int
avl_finddel(const Element *e, Node **pThis, AVL_Tree *T, const Element **found)
{
	/* return 1 if height decreases, else 0 */
	/* *pThis = this node (possibly NULL). */
	/* *found is set to 0 if e does not occur in the tree, */
	/* and is otherwise set to the matching element. */

	Node *A, *B, *C, *D, *E;
	int c, hB, hC, hD, hE, rv;

	if (!(A = *pThis))
		return 0;
	c = (*T->cmp)(T->v, e, A->elem);
	if (c < 0) {
		if (!A->left)
			return 0;
		if (!avl_finddel(e, &A->left, T, found))
			return 0;
		hB = hC = 0;
		if ((C = A->right))
			hC = C->height;
		if ((B = A->left))
			hB = B->height;
		if (hB >= hC - 1) {
			if (hB >= hC)
				--A->height;
			return 0;
			}
		hD = hE = 0;
		if ((D = C->left))
			hD = D->height;
		if ((E = C->right))
			hE = E->height;
		if (hE > hD) {
			/* easy rotation */
			C->left = A;
			A->right = D;
			A->height = hD + 1;
			*pThis = C;
			return 1;
			}
		/* hard rotation */
		A->right = D->left;
		A->height = hB + 1;
		C->left = D->right;
		C->height = D->height++;
		D->right = C;
		D->left = A;
		*pThis = D;
		}
	else if (c > 0) {
		if (!A->right)
			return 0;
		if (!avl_finddel(e, &A->right, T, found))
			return 0;
		hB = hC = 0;
		if ((C = A->left))
			hC = C->height;
		if ((B = A->right))
			hB = B->height;
		if (hB >= hC - 1) {
			if (hB >= hC)
				--A->height;
			return 0;
			}
		hD = hE = 0;
		if ((D = C->right))
			hD = D->height;
		if ((E = C->left))
			hE = E->height;
		if (hE > hD) {
			/* easy rotation */
			C->right = A;
			A->left = D;
			A->height = hD + 1;
			*pThis = C;
			return 1;
			}
		/* hard rotation */
		A->left = D->right;
		A->height = hB + 1;
		C->right = D->left;
		C->height = D->height++;
		D->left = C;
		D->right = A;
		*pThis = D;
		}
	else /* c == 0 */ {
		*found = A->elem;
		hB = hC = 0;
		if ((B = A->left))
			hB = B->height;
		if ((C = A->right))
			hC = C->height;
		if (hB > hC) {
			A->height -= (rv = avl_delhelp(&A->left, T, &A->elem, 0));
			return rv;
			}
		if (hC > hB) {
			A->height -= (rv = avl_delhelp(&A->right, T, &A->elem, 1));
			return rv;
			}
		if (B) {
			avl_delhelp(&A->left, T, &A->elem, 0);
			return 0;
			}
		A->elem = 0;
		*pThis = 0;
		A->left = T->efree;
		T->efree = A;
		}
	return 1;
	}

 const Element *
AVL_delete(const Element *e, AVL_Tree *T)
{
	const Element *rv;

	rv = 0;
	avl_finddel(e, &T->Top, T, &rv);
	if (rv)
		--T->nelem;
	return rv;
	}
