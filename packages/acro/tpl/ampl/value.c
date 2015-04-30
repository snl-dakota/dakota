/****************************************************************
Copyright (C) 1997, 2001 Lucent Technologies
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of Lucent or any of its entities
not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
****************************************************************/

#include "getstub.h"

 char *
#ifdef KR_headers
C_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
C_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	ASL *asl;
	char *s, *s1, *z, *zap;
	unsigned char *rv;
	unsigned int nq, q;

	asl = oi->asl;
	rv = (unsigned char *)value;
	if (*rv == '?' && rv[1] <= ' ') {
		zap = 0;
		if (!(s = *(char**)kw->info))
			s = "<NULL>";
		else {
			for(nq = 0, s1 = s; *s1; ++s1)
				if (*s1 == '"')
					++nq;
			z = zap = (char*)Malloc((s1-s) + nq + 3);
			*z++ = '"';
			while(*s) {
				if (*s == '"')
					*z++ = '"';
				*z++ = *s++;
				}
			*z++ = '"';
			*z = 0;
			s = zap;
			}
		printf("%s%s%s\n", kw->name, oi->eqsign, s);
		if (zap)
			free(zap);
		oi->option_echo &= ~ASL_OI_echothis;
		return value+1;
		}
	switch(q = *rv) {
	  case '"':
	  case '\'':
		while(*++rv && (*rv != q || *++rv == q));
		break;
	  default:
		q = 0;
		while(*++rv > ' ');
		if (!*rv) {
			*(char**)kw->info = value;
			goto done;
			}
	  }
	*(char**)kw->info = s = (char*)M1alloc((char*)rv - value + 1);
	if (q) {
		while(*++value) {
			if (*value == q) {
				if (*++value != q)
					break;
				}
			*s++ = *value;
			}
		}
	else
		while(*(unsigned char*)value > ' ')
			*s++ = *value++;
	*s = 0;
 done:
	return (char*)rv;
	}

 char *
#ifdef KR_headers
CK_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
CK_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	C_Known *c = (C_Known*)kw->info;
	Not_Used(oi);
	*c->valp = c->val;
	return value;
	}

 char *
#ifdef KR_headers
Dval_ASL(oi, kw, value, v) Option_Info *oi; keyword *kw; char *value; real *v;
#else
Dval_ASL(Option_Info *oi, keyword *kw, char *value, real *v)
#endif
{
	char buf[32], *rv;
	real t;

	if (*value == '?') {
		g_fmt(buf, *v);
		printf("%s%s%s\n", kw->name, oi->eqsign, buf);
		oi->option_echo &= ~ASL_OI_echothis;
		return value+1;
		}
	t = strtod(value, &rv);
	if (*(unsigned char*)rv <= ' ')
		*v = t;
	else
		rv = badval_ASL(oi,kw,value,rv);
	return rv;
	}

 char *
#ifdef KR_headers
DA_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
DA_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	return Dval_ASL(oi, kw, value,
		(double *)((char*)oi->asl + Intcast kw->info));
	}

 char *
#ifdef KR_headers
DK_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
DK_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	D_Known *c = (D_Known*)kw->info;
	Not_Used(oi);
	*c->valp = c->val;
	return value;
	}

 char *
#ifdef KR_headers
D_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
D_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	return Dval_ASL(oi, kw, value, (real*)kw->info);
	}

 char *
#ifdef KR_headers
DU_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
DU_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	return Dval_ASL(oi, kw, value, (real*)(oi->uinfo + Intcast kw->info));
	}

 char *
#ifdef KR_headers
LK_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
LK_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	L_Known *c = (L_Known*)kw->info;
	Not_Used(oi);
	*c->valp = c->val;
	return value;
	}

 char *
#ifdef KR_headers
Lval_ASL(oi, kw, value, v) Option_Info *oi; keyword *kw; char *value; Long *v;
#else
Lval_ASL(Option_Info *oi, keyword *kw, char *value, Long *v)
#endif
{
	char *rv;
	Long t;

	if (*value == '?') {
		printf("%s%s%ld\n", kw->name, oi->eqsign, (long)*v);
		oi->option_echo &= ~ASL_OI_echothis;
		return value+1;
		}
	t = (Long)strtol(value, &rv, 10);
	if (*(unsigned char*)rv <= ' ')
		*v = t;
	else
		rv = badval_ASL(oi,kw,value,rv);
	return rv;
	}

 char *
#ifdef KR_headers
L_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
L_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	return Lval_ASL(oi, kw, value, (Long *)kw->info);
	}

 char *
#ifdef KR_headers
LU_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
LU_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	return Lval_ASL(oi, kw, value, (Long *)(oi->uinfo + Intcast kw->info));
	}

 char *
#ifdef KR_headers
Ival_ASL(oi, kw, value, v) Option_Info *oi; keyword *kw; char *value; int *v;
#else
Ival_ASL(Option_Info *oi, keyword *kw, char *value, int *v)
#endif
{
	char *rv;
	int t;

	if (*value == '?') {
		printf("%s%s%d\n", kw->name, oi->eqsign, *v);
		oi->option_echo &= ~ASL_OI_echothis;
		return value+1;
		}
	t = (int)strtol(value, &rv, 10);
	if (*(unsigned char*)rv <= ' ')
		*v = t;
	else
		rv = badval_ASL(oi,kw,value,rv);
	return rv;
	}

 char *
#ifdef KR_headers
IA_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
IA_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	return Ival_ASL(oi, kw, value, (int *)((char*)oi->asl + Intcast kw->info));
	}

 char *
#ifdef KR_headers
IK_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
IK_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	I_Known *c = (I_Known*)kw->info;
	Not_Used(oi);
	*c->valp = c->val;
	return value;
	}

 char *
#ifdef KR_headers
IK0_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
IK0_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	Not_Used(oi);
	*(int*)kw->info = 0;
	return value;
	}

 char *
#ifdef KR_headers
IK1_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
IK1_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	Not_Used(oi);
	*(int*)kw->info = 1;
	return value;
	}

 char *
#ifdef KR_headers
IU_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
IU_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	return Ival_ASL(oi, kw, value, (int *)(oi->uinfo + Intcast kw->info));
	}

 char *
#ifdef KR_headers
I_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
I_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	return Ival_ASL(oi, kw, value, (int*)kw->info);
	}

 char *
#ifdef KR_headers
WS_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
WS_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	return Ival_ASL(oi, kw, value, &oi->wantsol);
	}

 char *
#ifdef KR_headers
SU_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
SU_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	char *rv;
	short *v = (short *)(oi->uinfo + Intcast kw->info);
	int L = *v;
	rv = Ival_ASL(oi, kw, value, &L);
	*v = L;
	return rv;
	}

 char *
#ifdef KR_headers
FI_val(oi, kw, value) Option_Info *oi; keyword *kw; char *value;
#else
FI_val(Option_Info *oi, keyword *kw, char *value)
#endif
{
	Long L = *(fint*)kw->info;
	char *rv = Lval_ASL(oi, kw, value, &L);
	*(fint*)kw->info = L;
	return rv;
	}

/* Affected by ASL update of 20020503 */
