/* $Id: pltek.c 3186 2006-02-15 18:17:33Z slbrow $
 *
 *  Review a Tektronix vector file.
 *  from 'scan', by Clair Nielsen, LANL.
 *  Modifications by Maurice LeBrun, IFS.
 *
 *  This version works well with xterm and at least one vt100/tek emulator
 *  I've tried.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

static void describe(void);
static void tek_mode(int mode);

#define ESC  27				/* ESCape character */

#define ALPHA_MODE	0		/* switch to alpha mode */
#define GRAPH_MODE	1 		/* switch to graph mode */
#define BUFSZ	1024
#define MAXPAGES 1024

enum { unknown, xterm, tekterm } termtype = unknown;


int
main(int argc, char *argv[])
{
    FILE *fd;
    int i, j, nb, npage, ipage, ifirst, oldpage;
    int istop;
    long start[MAXPAGES];		/* start (offset) of each page */
    char buf[BUFSZ], xtra, lastchar = '\0';
    char c, ibuf[128], *t;

    if (argc < 2) {
	describe();
	exit(1);
    }
    if ((fd = fopen(argv[1], "r")) == NULL) {
	printf("Cannot open %s\n", argv[1]);
	exit(1);
    }
   
    if ( (t = getenv("TERM")) != NULL ) {      
	if ( strcmp("xterm", t) == 0 ) { 
	    termtype = xterm;	 
	} else if (!strncmp("tek",t,3) ||
		   !strncmp("401",t,3) ||
		   !strncmp("410",t,3) ) {
	    termtype = tekterm;
	}
    }

/* Find out how many pages there are in file. */

    ipage = 0;
    start[0] = 0;
    for (i = 0; i < MAXPAGES; i++) {
	nb = fread(buf, 1, BUFSZ, fd);
	if (nb <= 0)
	    break;
	ifirst = 0;
	for (j = 0; j < nb; j++) {
	    if ((lastchar = buf[j]) == '\f') {
		ifirst = j - 1;
		ipage++;
		start[ipage] = BUFSZ * i + ifirst + 2;
	    }
	}
    }

/* don't count a FF at the end of the file as a separate page */

    if (lastchar == '\f')
	ipage--;

    npage = ipage + 1;

/* Loop until the user quits */

    ipage = 0;
    while (1) {
	oldpage = ipage;
	printf("Page %d/%d> ", ipage, npage);

	if (!fgets(ibuf, 128, stdin))
	    break;

	c = ibuf[0];

/* User input a page number or a return */
/* A carriage return in response to the prompt proceeds to the next page. */

	if (c == '\n') {	 	/* CR = increment by one page */
	    ipage++;
	} else if (c == '+') {		/* +<n> = increment by <n> pages */
	    ipage += atoi(ibuf+1);
	} else if (c == '-') {		/* -<n> = decrement by <n> pages */
	    ipage -= atoi(ibuf+1);
	} else if (isdigit(c)) {	/* <n> = goto page <n> */
	    ipage = atoi(ibuf);
	} else if (c == '\f') {
	    ;				/* FF = redraw the last plot */
	} else if (c == 'q') {		/* q = quit */
	    break;
	} else {			/* h, ? or garbage = give help */
	    tek_mode(ALPHA_MODE);
	    describe();    
	    continue;
	}

/* Bounds checking */

	if (ipage > npage) {
	    ipage = npage;
	    if (ipage == oldpage)
		continue;
	} else if (ipage < 0) {
	    ipage = 1;
	} else if (ipage == 0) {
	    continue;			/* page 0 does not exist */
	}

/* Time to actually plot something. */

	tek_mode(GRAPH_MODE);
	istop = fseek(fd, start[ipage - 1], 0);
	xtra = '\0';
	istop = 0;
	for (i = 0; i < 8196; i++) {	/* less than 8MB per page! */
	    if (xtra) {
		fwrite(&xtra, 1, 1, stdout);
		xtra = '\0';
	    }
	    nb = fread(buf, 1, BUFSZ, fd);
	    if (nb <= 0)
		break;
	    ifirst = 0;
	    for (j = 0; j < nb; j++) {
		if (buf[j] == '\f') {
		    fwrite(&buf[ifirst], 1, j - ifirst, stdout);
		    fflush(stdout);
		    istop = 1;
		    break;
		}
	    }
	    if (istop)
		break;
	    if (buf[nb] == ESC) {
		xtra = ESC;
		j--;
	    }
	    fwrite(&buf[ifirst], 1, j - ifirst, stdout);
	}
	if ( termtype == xterm ) 
	    tek_mode(ALPHA_MODE);
    }
    tek_mode(ALPHA_MODE);		/* how to kill an xterm Tek window? */
    fclose(fd);
    exit(0);
}

/*----------------------------------------------------------------------*\
 *  tek_mode()
 *
 *  switch to alpha/graph mode
\*----------------------------------------------------------------------*/

static int currentmode = ALPHA_MODE;
static void 
tek_mode (int mode)
{
    if ( mode == GRAPH_MODE ) {
	if ( currentmode == ALPHA_MODE ) {
	    switch(termtype) {
	    case tekterm:
		break;
	    case xterm:
		printf("\033[?38h");		/* open graphics window */
		break;
	    default:
		printf("\033[?38h");		/* open graphics window */
	    }
	    printf("\035");			/* Enter Vector mode = GS */
	}
	printf("\033\f");			/* clear screen = ESC FF */
	fflush(stdout);      
	currentmode = mode;
    } else if ( mode == ALPHA_MODE ) {
	switch(termtype) {
	case tekterm:
	    printf("\037\030");		/* Enter Alpha mode = US CAN */
	    break;
	case xterm:
	    printf("\033\003");		/* VT mode (xterm) = ESC ETX */
	    break;
	default:
	    printf("\033\003");		/* VT mode (xterm) = ESC ETX */
	}
	fflush(stdout);      
	currentmode = mode;
    }
}

/*----------------------------------------------------------------------*\
 *  describe()
 *
 *  Print help message.
 *  Note: if this message starts to exceed 512 bytes, may need to split
 *  since some compilers can't handle strings that long.
\*----------------------------------------------------------------------*/

static void 
describe (void)
{
    fputs("\
Usage: pltek filename \n\
At the prompt, the following replies are recognized:\n\
   h,?	  Give this help message.\n\
    q	  Quit program.\n\
   <n>	  Go to the specified page number.\n\
   -<n>   Go back <n> pages.\n\
   +<n>   Go forward <n> pages.\n\
 <Return> Go to the next page.\n\n",
	  stdout);
}
