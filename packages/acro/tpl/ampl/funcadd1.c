/****************************************************************
Copyright (C) 1998, 1999, 2000 Lucent Technologies
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

#ifdef NO_FUNCADD
#include "funcadd.h"

char *ix_details_ASL[] = {0};

 void
#ifdef KR_headers
funcadd(ae) AmplExports *ae;
#else
funcadd(AmplExports *ae)
#endif
{ ae = ae; /* shut up non-use warning */ }

#else

#ifdef _WIN32
#undef WIN32
#define WIN32
#endif

#ifdef WIN32
#include "windows.h"
#undef Char
#endif

#define _POSIX_SOURCE	/* for HP-UX */

#include "funcadd.h"
#include "string.h"

#ifdef Old_APPLE	/* formerly __APPLE__, for earlier versions of Mac OS X */
#define FUNCADD "_funcadd_ASL"
#endif
#ifndef FUNCADD
#define FUNCADD "funcadd_ASL"
#endif

#ifdef __cplusplus
extern "C" {
extern int libload_ASL(AmplExports *ae, char *s, int ns, int warn);
#endif

typedef void Funcadd ANSI((AmplExports*));

extern Char *mymalloc_ASL ANSI((size_t));
#undef mymalloc
#define mymalloc(x) mymalloc_ASL((size_t)(x))
#ifndef KR_headers
extern void free(void*);
#endif

char *ix_details_ASL[] = {
	"? {show -i options}",
	"- {do not import functions: do not access amplfunc.dll}",
	"dir {look for amplfunc.dll in directory dir}",
	"file {import functions from file rather than amplfunc.dll}",
	"",
	"If no -i option appears but $ampl_funclibs is set, assume",
	"-i $ampl_funclibs.  Otherwise, if $AMPLFUNC is set, assume",
	"-i $AMPLFUNC.  Otherwise look for amplfunc.dll in the",
	"directory that is current when execution begins.",
	"",
	"-ix and -i x are treated alike.",
	0 };
#define afdll afdll_ASL
extern int aflibname_ASL ANSI((AmplExports*, char*, char*, int, Funcadd*, int, void(*)(void*), void*));
extern char *i_option_ASL;

#ifdef __cplusplus
	}
#endif

static int first = 1;

#ifdef WIN32

#define SLASH '\\'
char afdll[] = "\\amplfunc.dll";
typedef HINSTANCE shl_t;
#define dlopen(x,y) LoadLibrary(x)
#define find_dlsym(a,b,c) (a = (Funcadd*)GetProcAddress(b,c))
#define dlclose(x) FreeLibrary((HMODULE)x)
#define NO_DLERROR
#define reg_file(x) 1

 static int
Abspath(char *s)
{
	int c = *s;
	if ((c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
	 && s[1] == ':'
	 && (c = s[2]) == '\\' || c == '/')
		return 1;
	return 0;
	}

#else /* !WIN32 */

#define SLASH '/'

char afdll[] = "/amplfunc.dll";

#define Abspath(s) (*(s) == '/')

#ifdef KR_headers
extern char *getcwd();
#else
#include "unistd.h"	/* for getcwd */
#endif
#define GetCurrentDirectory(a,b) getcwd(b,(int)(a))

#include <sys/types.h>
#include <sys/stat.h>

 static int
#ifdef KR_headers
reg_file(name) char *name;
#else
reg_file(char *name)
#endif
{
	struct stat sb;
	return stat(name,&sb) ? 0 : S_ISREG(sb.st_mode);
	}

#ifdef __hpux
#include "dl.h"
#define dlopen(x,y) shl_load(x, BIND_IMMEDIATE, 0)
#define find_dlsym(a,b,c) !shl_findsym(&b, c, TYPE_PROCEDURE, &a)
#define dlclose(x) shl_unload((shl_t)x)
#define NO_DLERROR
#else
#ifdef Old_APPLE
#include <mach-o/dyld.h>
typedef struct {
	NSObjectFileImage ofi;
	NSModule m;
	char *name;
	} NS_pair;
typedef NS_pair *shl_t;

 static void*
find_sym_addr(NS_pair *p, const char *name)
{
	NSSymbol nss;

	if (nss = NSLookupSymbolInModule(p->m, name))
		return NSAddressOfSymbol(nss);
	return 0;
	}

#define find_dlsym(a,b,c) (a = find_sym_addr(b,c))

 static void
dlclose(NS_pair *p)
{
	if (NSUnLinkModule(p->m, NSUNLINKMODULE_OPTION_NONE))
		NSDestroyObjectFileImage(p->ofi);
	free(p);
	}
#define NO_DLERROR
#else
#include "dlfcn.h"
typedef void *shl_t;
#define find_dlsym(a,b,c) (a = (Funcadd*)dlsym(b,c))
#ifdef sun
#ifndef RTLD_NOW
#define RTLD_NOW RTLD_LAZY
#endif
#endif /* sun */
#endif /* Old_APPLE */
#endif /* __hpux */
#endif /* WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

 static shl_t
#ifdef KR_headers
dl_open(ae, name, warned) AmplExports *ae; char *name; int *warned;
#else
dl_open(AmplExports *ae, char *name, int *warned)
#endif
{
/* This is a bit aggressive, but a first cut */
#if defined(WIN32) || defined(AMPL_HAVE_DLOPEN)
	shl_t h;
	FILE *f;
#ifndef KR_headers
	const
#endif
	      char *s;

#ifdef Old_APPLE
	NS_pair p;
	NSObjectFileImageReturnCode irc;
	irc = NSCreateObjectFileImageFromFile(name,&p.ofi);
	h = 0;
	if (irc == NSObjectFileImageSuccess) {
		p.m = NSLinkModule(p.ofi, name,
			  NSLINKMODULE_OPTION_BINDNOW
			| NSLINKMODULE_OPTION_PRIVATE
			| NSLINKMODULE_OPTION_RETURN_ON_ERROR);
		if (!p.m)
			fprintf(stderr, "NSLinkModule(\"%s\") failed.\n", name);
		else {
			h = (NS_pair*)mymalloc(sizeof(NS_pair) + strlen(name) + 1);
			strcpy(p.name = (char*)(h+1), name);
			memcpy(h, &p, sizeof(NS_pair));
			}
		}
	else if (irc != NSObjectFileImageAccess)
		fprintf(stderr,
			"return %d from NSCreateObjectFileImageFromFile(\"%s\")\n",
			irc, name);
#else
	h = dlopen(name, RTLD_NOW);
#endif
	if (!h && warned && (f = fopen(name,"rb"))) {
		fclose(f);
		if (reg_file(name)) {
			*warned = 1;
#ifdef NO_DLERROR
			fprintf(Stderr, "Cannot load library %s.\n", name);
#else
			fprintf(Stderr, "Cannot load library %s", name);
			s = dlerror();
			fprintf(Stderr, s ? ":\n%s\n" : ".\n", s);
#endif
			}
		}
	return h;
#else /* !( #if defined(WIN32) || defined(HAVE_DLOPEN) ) */
	return 0;
#endif
	}

 static void
#ifdef KR_headers
dl_close(h) void *h;
#else
dl_close(void *h)
#endif
{
#ifdef CLOSE_AT_RESET
	first = 1;
#endif

#if defined(WIN32) || defined(AMPL_HAVE_DLOPEN)
	if (h)
		dlclose(h);
#endif
	}

 int
#ifdef KR_headers
libload_ASL(ae, s, ns, warn) AmplExports *ae; char *s; int ns; int warn;
#else
libload_ASL(AmplExports *ae, char *s, int ns, int warn)
#endif
{
#if defined(WIN32) || defined(AMPL_HAVE_DLOPEN)
	Funcadd *fa;
	char buf0[2048], *buf;
	int rc, warned;
	shl_t h;
	unsigned int n, nx;

	nx = 0;
	buf = buf0;
	if (!Abspath(s)) {
		if (!GetCurrentDirectory(sizeof(buf0),buf0))
			return 2;
		nx = strlen(buf0);
		}
	n = ns + sizeof(afdll) + nx;
	if (n > sizeof(buf0)) {
		buf = (char*)mymalloc(n);
		if (nx)
			memcpy(buf, buf0, nx);
		}
	if (nx)
		buf[nx++] = SLASH;
	strncpy(buf+nx, s, ns);
	buf[nx+ns] = 0;
	warned = 0;
	if (h = dl_open(ae, buf, &warned)) {
 found:
		if (find_dlsym(fa, h, FUNCADD)
		 || find_dlsym(fa, h, "funcadd")) {
			rc = 0;
#ifdef CLOSE_AT_RESET
			aflibname_ASL(ae,buf,s,ns,fa,0,dl_close,h);
				/* -DCLOSE_AT_RESET is for use in shared */
				/* libraries, such as MATLAB mex functions, */
				/* that may be loaded and unloaded several */
				/* times during execution of the program. */
#else
			aflibname_ASL(ae,buf,s,ns,fa,1,dl_close,h);
#endif
			}
		else {
			fprintf(stderr, "Could not find funcadd in %s\n", buf);
			dl_close(h);
			rc = 3;
			}
		}
	else if (warn) {
		if (!warned) {
			strcpy(buf+nx+ns, afdll);
			if (h = dl_open(ae, buf, &warned))
				goto found;
			}
		if (warned)
			rc = 2;
		else
			goto notfound;
		}
	else {
 notfound:
		if (warn)
			fprintf(Stderr, "Cannot find library %.*s\nor %.*s%s\n",
				ns, s, ns, s, afdll);
		rc = 1;
		}
	if (buf != buf0)
		free(buf);
	return rc;
#else /* !( #if defined(WIN32) || defined(HAVE_DLOPEN) ) */
	return 3;
#endif
	}

 static void
#ifdef KR_headers
libloop(ae, s) AmplExports *ae; char *s;
#else
libloop(AmplExports *ae, char *s)
#endif
{
	char *s1;
	int ns;

	for(;; s = s1) {
		while(*s <= ' ')
			if (!*s++)
				return;
		for(s1 = s; *++s1 >= ' '; );
		while(s1[-1] == ' ')
			--s1;
		ns = s1 - s;
		libload_ASL(ae, s, ns, 1);
		}
	}

 void
#ifdef KR_headers
funcadd(ae) AmplExports *ae;
#else
funcadd(AmplExports *ae)
#endif
{
	char *s;

	if (first) {
		first = 0;
		if (s = i_option_ASL) {
			if (!*s || *s == '-' && !s[1])
				return;
			libloop(ae, s);
			}
		else
			libload_ASL(ae, afdll+1, (int)sizeof(afdll)-2, 0);
		}
	}
#ifdef __cplusplus
}
#endif

#endif /* NO_FUNCADD */
