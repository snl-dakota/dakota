/*
## Copyright (C) 1998-2003 Joao Cardoso.
## 
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2 of the License, or (at your
## option) any later version.
## 
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## This file is part of plplot_octave.
*/

/*
 * Add online help to functions, parsing 'tmp_stub' and looking for
 * function definitions; for each function found, scans directory
 * 'plplot_octave_txt' for a file with the same basename as function
 * and extension '.txt'; if found, incorporates it as the function
 * online help, else, looks in 'plplot.doc' for a one-line description
 * of the function and adds it. If not even found in 'plplot.doc', ask
 * the user to write the docs! No free lunches!
 */

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include  <sys/stat.h>
#include  "errno.h"

int
main()
{
  char	*p1, *p2;
  char	doc[150][1024], b[1024], tok[80];
  int		item=0, j;
  FILE	*fp, *fp1;
  struct stat buf;
	
  if ((fp = fopen("plplot_octave_txt/plplot.doc","r")) == NULL) {
    perror("plplot.doc not found:");
    exit(1);
  }

  while (! feof(fp)){
    fgets(b, sizeof(b), fp);
    if (strchr(b, '-'))
      strcpy(doc[item++], b);
  }
  fclose(fp);

  if ((fp = fopen("tmp_stub", "r")) == NULL) {
    perror("tmp_stub not found:");
    exit(1);
  }

  while (! feof(fp)){
    fgets(b, sizeof(b), fp);
    if ((p2 = strchr(b, '('))){		/* function ... = ...( */
      p1 = p2;
      while(*p1-- != ' ');
      p1 += 2;
      if (*(p1+1) == '_')			/* c_... */
	p1+=2;
      strncpy(tok, p1, p2-p1);
      *(tok+(int)(p2-p1))='\0';
      printf("%s", b);
      fgets(b, sizeof(b), fp);
      printf("%s%%\n", b);	/* % function ... = ...(*/
				
      sprintf(b,"plplot_octave_txt/%s.txt", tok);
      if (stat(b, &buf) && errno == ENOENT) {
	fprintf(stderr,"%s not found, trying plplot.doc... ", b);
	strcat(tok, "\t");
	for (j=0; j<item; j++){
	  if (strncmp(doc[j], tok, strlen(tok)) == 0){
	    printf("%% %s", &doc[j][strlen(tok)+4]);	/* strip func --*/
	    break;
	  }
	}
	if (j == item) {
	  fprintf(stderr,"%s not found\n", tok);
	  printf("%% No online help available. Help me, write and submit the documentation, or at least write a one line descriptive text.\n");
	  
	} else
	  fprintf(stderr,"%s OK\n", tok);
      } else {
	printf("%%   Original PLplot call documentation:\n%%\n");
	fp1 = fopen(b,"r");
	while(!feof(fp1)) {
	  fgets(b, sizeof(b), fp1);
	  printf("%% %s", b);
	}
	fclose(fp1);
      }
      fgets(b, sizeof(b), fp);	/* % No doc...*/
      fgets(b, sizeof(b), fp);
      printf("%s", b);          /* plplot_oct...*/
      fgets(b, sizeof(b), fp);
      printf("%s\n", b);        /* endfunction*/
    }
  }
  fclose(fp);
  exit(0);
}
