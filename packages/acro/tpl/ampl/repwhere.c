/****************************************************************
Copyright (C) 1997 Lucent Technologies
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

#include "asl.h"

 void
#ifdef KR_headers
report_where(asl) ASL *asl;
#else
report_where(ASL *asl)
#endif
{
	int i, j, k, k1;
	static char *what[2] = { "constraint", "objective" };
	static char *nfmt[2] = { "%d: ", "function: " };
	char *b, buf[512];
	FILE *f;

	fflush(stdout);
	need_nl = 0;
	fprintf(Stderr, "Error evaluating ");

#define next_line fgets(buf,sizeof(buf),f)

	if (i = cv_index) {
		strcpy(stub_end, ".fix");
		j = 0;
		if (f = fopen(filename, "r")) {
			for(;;) {
				if (!next_line)
					goto eof;
				for(b = buf; *b; b++)
					if (*b == '=') {
						while(++j < i)
							if (!next_line)
								goto eof;
						b = buf;
						while(*b && *b != '=')
							b++;
						if (*b != '=' || b < buf + 2)
							j = 0;
						else
							b[-1] = 0;
						goto eof;
						}
				}
 eof:
			fclose(f);
			}
		if (j == i)
			fprintf(Stderr, "var %s: ", buf);
		else
			fprintf(Stderr, "\"var =\" definition %d: ", i);
		goto ret;
		}

	k = k1 = 0;
	if ((i = co_index) < 0) {
		k = 1;
		i = n_con -i - 1;
		if (n_obj <= 1)
			k1 = 1;
		}
	fprintf(Stderr, "%s ", what[k]);
	if (maxrownamelen) {
		strcpy(stub_end, ".row");
		if (f = fopen(filename, "r")) {
			for(j = 0; j <= i; j++)
				if (!next_line)
					break;
			fclose(f);
			if (j > i) {
				for(b = buf; *b; b++)
					if (*b == '\n') {
						*b = 0;
						break;
						}
				fprintf(Stderr, "%s: ", buf);
				goto ret;
				}
			}
		}
	fprintf(Stderr, nfmt[k1], i + 1);
 ret:
	errno = 0;	/* in case it was set by fopen */
	fflush(Stderr);
	}
