/*********************************************************************
Copyright 2008, 2010 Sandia Corporation.  Under the terms of Contract
DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
retains certain rights in this software.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of Sandia Corporation nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

/* dakreorder.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int NIDR_disallow_missing_start;
extern void nidr_parse(const char*, FILE*);
extern int nidr_save_exedir(const char*, int);
extern FILE *nidrin;

char nidr_please_refer[] =
	"Please refer to the DAKOTA Reference and/or User's Manual or the\n"
	"dakota.input.summary file distributed with this executable.";

 static char *progname, version[];

 static int
report_version(void)
{
	printf("%s", version+1);
	return 0;
	}

 static int
usage(int rc)
{
	fprintf(rc ? stderr : stdout, "Usage: %s [options]"
#ifdef NO_NIDR_keywds0
	" specsum"
#endif
	" [infile [outfile]]\n\
	to reorder DAKOTA input file infile (or stdin if infile is not given)\n\
	to an order that permits simple parsing, with abbreviated keywords\n\
	expanded to their full forms.\nOptions:\n\t"
#ifndef NO_NIDR_keywds0
	"-1 ==> put data for a keyword on one line\n\
	-c ==> retain comments\n\
	-j specsum ==> read grammer specification from file specsum\n\
		(written by \"nidrgen -j specsum dakota.input.nspec\")\n\
	-p ==> replace aliases by the \"preferred\" name\n\t"
#endif
	"-v ==> report version\n", progname);
	return rc;
	}

 static int specadj(char*s);

 int
main(int argc, char **argv)
{
	FILE *f;
	char buf[8], *b, *s;
	int comkeep, oneline, prefer;

	nidr_save_exedir(progname = *argv, 0); /* 0 ==> no $PATH adjustment */
#ifdef NO_NIDR_keywds0
	comkeep = oneline = prefer = 1;
#else
	comkeep = oneline = prefer = 0;
#endif
nextarg:
	while((s = *++argv) && *s == '-') {
		for(;;)
			switch(*++s) {
			  case 0: goto nextarg;
#ifndef NO_NIDR_keywds0
			  case '1': ++oneline; continue;
			  case 'c': ++comkeep; continue;
			  case 'j': if (*++s || (s = *++argv)) {
					if (specadj(s))
						return 1;
					goto nextarg;
					}
				    goto usage1;
			  case 'p': ++prefer;  continue;
#endif
			  case 'h': return usage(!s[1] || !strcmp(s,"help") ? 0 : 1);
#ifdef NIDR_DYLIB_DEBUG
				/* -s is a debugging option, not mentioned in usage() */
			  case 's': nidr_set_strict(1); goto nextarg;
#endif
			  case 'v': return report_version();
			  case '?': return usage(s[1] != 0);
			  case '-': if (!s[1]) {
					s = *++argv;
					goto optsdone;
					}
				if (!strcmp(s,"-help"))
					return usage(0);
				if (!strcmp(s,"-version"))
					return report_version();
			  usage1:
			  default:  return usage(1);
			  }
		}
 optsdone:
	/* default nidrin (set by nidr-scanner) is stdin */
	f = stdout;
	NIDR_disallow_missing_start = 0;
#ifdef NO_NIDR_keywds0
	if (!s || specadj(s))
		return usage(1);
	s = *++argv;
#endif
	if (s) {
		if (argv[1] && argv[2])
			return usage(1);
		if (!(nidrin = fopen(s,"r"))) {
			fprintf(stderr, "%s: Cannot open input file \"%s\"\n",
				progname, s);
			return 1;
			}
		if ((b = argv[1]) && !(f = fopen(b, "w"))) {
			fprintf(stderr, "%s: Cannot open output file \"%s\"\n", progname, b);
			return 1;
			}
		}
	b = buf;
	if (oneline + comkeep + prefer) {
		*b++ = '-';
		if (oneline)
			*b++ = '1';
		if (comkeep)
			*b++ = 'c';
		if (prefer)
			*b++ = 'p';
		*b = 0;
		}
	nidr_parse(buf, f);
	return 0;
	}

#include "nidr.h"

#ifdef NO_NIDR_keywds0
KeyWord Dakota_Keyword_Top = {"KeywordTop"};
 static char version[] = "\ndakreord version 20100604\n";
#else
#include "NIDR_keywds0.h"
 static char version[] = "\ndakreorder version " NSPEC_DATE "\n";
#endif

 static void*
Malloc(size_t len)
{
	void *rv = malloc(len);
	if (!rv) {
		fprintf(stderr, "%s:  malloc(%lu) failure!\n",
			progname, (unsigned long)len);
		exit(1);
		}
	return rv;
	}

 static void
swap4(void *v)
{
	int c;
	char *s = (char*)v;
	c = s[3];
	s[3] = s[0];
	s[0] = c;
	c = s[2];
	s[2] = s[1];
	s[1] = c;
	}

 static void
swap8(void *v)
{
	int c;
	char *s, *t;
	s = (char*)v;
	t = s + 7;
	do {
		c = *s;
		*s++ = *t;
		*t-- = c;
		} while(s < t);
	}

 static int
specadj(char *specsum)
{
	FILE *f;
	KeyWord *kw, *kw0;
	KwpHead head;
	Kwpack *p, *p0;
	Kwpack0 *q;
	Uint *u, *u0;
	const char *strtab;
	double *d, *fptab;
	int swap;
	size_t L, L1;

	f = fopen(specsum, "rb");
	if (!f) {
		fprintf(stderr, "%s:  cannot open \"%s\"\n", progname, specsum);
		return 1;
		}
	fread(&head, sizeof(head), 1, f);
	swap = 0;
	if (head.bkind != 0x1234abcd) {
		swap4(&head.bkind);
		if (head.bkind != 0x1234abcd) {
 badfile:
			fprintf(stderr, "%s:  invalid specsum file \"%s\"\n",
				progname, specsum);
			return 1;
			}
		swap8(&head.fpkind);
		swap = 1;
		swap4(&head.nkw);
		swap4(&head.strtab_offset);
		swap4(&head.kw_offset);
		swap4(&head.end_offset);
		}
	if (head.fpkind != 1e13)
		goto badfile;
	L = head.end_offset;
	L1 = head.kw_offset + (head.nkw+1)*sizeof(KeyWord);
	if (L1 < L)
		L1 = L;
	fptab = (double*)Malloc(L1);
	fread(fptab, L, 1, f);
	fclose(f);
	p = p0 = (Kwpack*)((char*)fptab + head.kw_offset);
	strtab = (const char*)fptab + head.strtab_offset;
	kw0 = (KeyWord*)p;
	kw = ++kw0 + head.nkw;
	p += head.nkw;
	if (head.pad == 0) {
		q = (Kwpack0*)p0 + head.nkw;
		p = (Kwpack*)q;
		}
	if (swap) {
		u0 = (Uint*)p0;
		u = (Uint*)p;
		while(u > u0)
			swap4(--u);
		d = fptab + head.strtab_offset/sizeof(double);
		while(d > fptab)
			swap8(--d);
		}
	if (head.pad == 0)
		while(kw > kw0) { /* no DYNLIB data */
			--kw;
			--q;
			kw->name = q->name ? strtab + q->name : 0;
			kw->kind = q->kind;
			kw->nkw = q->nkw;
			kw->alt = q->alt;
			kw->req = q->req;
			kw->kw = q->kw ? kw0 + q->kw : 0;
			kw->Lb = fptab[q->Lb];
			kw->Ub = fptab[q->Ub];
			kw->paoff = q->poff;
			kw->f.start = kw->f.final = 0;
			kw->f.vs = kw->f.vf = 0;
			kw->comment = 0;
			kw->kwnext = 0;
			}
	else
		while(kw > kw0) {
			--kw;
			--p;
			kw->name = p->name ? strtab + p->name : 0;
			kw->kind = p->kind;
			kw->nkw = p->nkw;
			kw->alt = p->alt;
			kw->req = p->req;
			kw->kw = p->kw ? kw0 + p->kw : 0;
			kw->Lb = fptab[p->Lb];
			kw->Ub = fptab[p->Ub];
			kw->paoff = p->poff;
			kw->f.start = kw->f.final = 0;
			kw->f.vs = kw->f.vf = 0;
			kw->comment = 0;
			kw->kwnext = 0;
			if (kw->kind & KWKind_Dynlib)
				kw->f.vf = (void*)(strtab + p->dylib);
			}
	kw = &Dakota_Keyword_Top;
	kw->nkw = kw0->nkw;
	kw->kw = kw0->kw;
	kw->alt = kw0->alt;
	kw->req = kw0->req;
	return 0;
	}
