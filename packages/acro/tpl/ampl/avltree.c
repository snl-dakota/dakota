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

#include "avltree.h"
#include <stdlib.h> /* for free() */
#include <string.h>

#ifndef AVL_memgulp
#define AVL_memgulp 256
#endif

 typedef struct AVL_Mblk AVL_Mblk;
 typedef struct Node Node;

 struct
Node {
	const Element *elem;
	Node *left, *right;
	int height;
	};

 struct
AVL_Mblk {
	AVL_Mblk *next;
	Node x[AVL_memgulp];
	};

 struct
AVL_Tree {
	Node *Top;
	Node *efree;
	AVL_Mblk *mb;
	size_t nelem;
	AVL_Elcomp cmp;
	void *v;
	void *(*Malloc)(size_t);
	void (*Free)(void*);
	};

#ifndef AVL_TREE_JUST_IMPL_DECLS

 AVL_Tree*
AVL_Tree_alloc2(void *v, AVL_Elcomp cmp, void *(*Malloc)(size_t), void (*Free)(void*))
{
	AVL_Mblk *mb;
	AVL_Tree *T;
	Node *N, *Ne;
	size_t L;

	mb = (AVL_Mblk*)Malloc(L = sizeof(AVL_Tree) + sizeof(AVL_Mblk));
	memset(mb, 0, L);
	T = (AVL_Tree*)(mb + 1);
	T->cmp = cmp;
	T->v = v;
	T->mb = mb;
	T->efree = N = mb->x;
	Ne = N + AVL_memgulp - 1;
	while(N < Ne)
		N = N->left = N + 1;
	T->Malloc = Malloc;
	T->Free = Free;
	return T;
	}

 AVL_Tree*
AVL_Tree_alloc(void *v, AVL_Elcomp cmp, void *(*Malloc)(size_t))
{ return AVL_Tree_alloc2(v, cmp, Malloc, free); }

 void
AVL_Tree_free(AVL_Tree **Tp)
{
	AVL_Mblk *mb, *mb1;
	AVL_Tree *T;

	if ((T = *Tp)) {
		*Tp = 0;
		mb1 = T->mb;
		while((mb = mb1)) {
			mb1 = mb->next;
			T->Free(mb);
			}
		}
	}

 static Node *
Node_alloc(AVL_Tree *T)
{
	AVL_Mblk *mb;
	Node *N, *Ne, *Nrv;

	mb = (AVL_Mblk*)T->Malloc(sizeof(AVL_Mblk));
	memset(mb, 0, sizeof(AVL_Mblk));
	mb->next = T->mb;
	T->mb = mb;
	N = Nrv = mb->x;
	Ne = N++ + AVL_memgulp - 1;
	T->efree = N;
	while(N < Ne)
		N = N->left = N + 1;
	return Nrv;
	}

 const Element *
AVL_find(const Element *e, AVL_Tree *T)
{
	Node *N;
	int c;

	if ((N = T->Top)) {
 top:
		if (!(c = (*T->cmp)(T->v, e, N->elem)))
			return N->elem;
		if (c < 0) {
			if ((N = N->left))
				goto top;
			return 0;
			}
		if ((N = N->right))
			goto top;
		}
	return 0;
	}

 void *
AVL_setv(AVL_Tree *T, void *v)
{
	void *rv = T->v;
	T->v = v;
	return rv;
	}

 static int
avl_findins(const Element *e, Node **pThis, AVL_Tree *T, const Element **found)
{
	/* return 1 if height increases, else 0 */
	/* *pThis = this node (possibly NULL). */
	/* *found is set to 0 if e does not occur in the tree, */
	/* and is otherwise set to the matching element. */

	Node *A, *B, *C, *D, *E, *N;
	int c, hB, hC, hD, hE;

	if (!(A = *pThis)) {
		if ((N = T->efree)) {
			T->efree = N->left;
			N->left = 0;
			}
		else
			N = Node_alloc(T);
		N->elem = e;
		*pThis = N;
		return 0;
		}
	c = (*T->cmp)(T->v, e, A->elem);
	if (c < 0) {
		if (!A->left) {
			if ((N = T->efree)) {
				T->efree = N->left;
				N->left = 0;
				}
			else
				N = Node_alloc(T);
			N->elem = e;
			A->left = N;
			if (A->right)
				return 0;
			return A->height = 1;
			}
		if (!avl_findins(e, &A->left, T, found))
			return 0;
		C = A->left;
		hC = C->height;
		hB = 0;
		if ((B = A->right))
			hB = B->height;
		if (hC <= hB)
			return 0;
		if (hC == hB + 1) {
			A->height = hB + 2;
			return 1;
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
			return 0;
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
	else if (c > 0) {
		if (!A->right) {
			if ((N = T->efree)) {
				T->efree = N->left;
				N->left = 0;
				}
			else
				N = Node_alloc(T);
			N->elem = e;
			A->right = N;
			if (A->left)
				return 0;
			return A->height = 1;
			}
		if (!avl_findins(e, &A->right, T, found))
			return 0;
		C = A->right;
		hC = C->height;
		hB = 0;
		if ((B = A->left))
			hB = B->height;
		if (hC <= hB)
			return 0;
		if (hC == hB + 1) {
			A->height = hB + 2;
			return 1;
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
			return 0;
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
	else /* c == 0 */
		*found = A->elem;
	return 0;
	}

 const Element *
AVL_insert(const Element *e, AVL_Tree *T)
{
	const Element *rv;

	rv = 0;
	avl_findins(e, &T->Top, T, &rv);
	if (!rv)
		++T->nelem;
	return rv;
	}

 size_t
AVL_Tree_size(AVL_Tree *T)
{
	return T->nelem;
	}

 static int
avl_visit1(void *v, Node *N, AVL_Visitor V)
{
	Node *N1;
	int rv;
 top:
	if ((N1 = N->left))
		avl_visit1(v, N1, V);
	if ((rv = (*V)(v, N->elem)))
		return rv;
	if ((N = N->right))
		goto top;
	return 0;
	}

 int
AVL_visit(void *v, AVL_Tree *T, AVL_Visitor V)
{
	if (!T->Top)
		return 0;
	return avl_visit1(v, T->Top, V);
	}

#endif /*AVL_TREE_JUST_IMPL_DECLS*/
