/*
	pmdefs.h
	Geoffrey Furnish
	22 September 1991
	
	This file contains definitions of constants and structures which
	are need by the PLPLOT 

	metafile writer and renderer.

	25 March 1992
	VERSION 1.0
*/

/* Symbolic constants needed in this file.
   Probably best to not exceed 32767 with these two. 
   WARNING: If you change these, it will affect how old metafiles are
   plotted, so be careful. */

#define PLMETA_X	10000
#define PLMETA_Y	10000

/* These numbers are supposed to be in dots/mm.  Since I am using a very
   large virtual display space, these need to be pretty big. */

#define PIXEL_RES_X	42
#define PIXEL_RES_Y	56


/* Structures and such for specifying the available commands. */

typedef int COMMAND;	/* Use for commands without paramaters. */
#define UCHAR unsigned char

/* 
	The available commands are ...

	*** NOTICE !!! ***
	If you change ANY of the following, you will wreck backward
	compatibility with old metafiles.  You may add, but do
	NOT delete !!!
*/

#define INITIALIZE 1
#define CLOSE 2
#define SWITCH_TO_TEXT 3
#define SWITCH_TO_GRAPH 4
#define CLEAR 5
#define PAGE 6
#define NEW_COLOR 7
#define NEW_WIDTH 8
#define LINE 9
#define LINETO 10
#define ESCAPE 11
#define ESC_NOOP 12
#define ESC_RGB 13

#define	STACKSIZE 8192

#define PIPE_BUFFER_SIZE	1000
#define PIPE_NAME	"\\pipe\\pmplplot"

typedef struct {
    PLINT a;
} PMSTUFF;

/*
	The following are various things needed to get my resources
	to work correctly with pmserv.
*/

#define ID_RESOURCE 1

#define IDM_ADVANCE	1
#define IDM_CONTROLS	2

#define IDM_AUTO_ADVANCE	20
