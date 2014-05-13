/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <stdio.h>

/**********************************************************************/
/* container.c - container optimization example                       */
/**********************************************************************/
int main(int argc, char **argv)
{
  FILE *fileptr;
  double fval[2], D, H;
  int i, num_vars, num_fns, req[2];
  char *infile, *outfile, in_str[81];
  char *valtag[] = {"area\n", "volume_constraint\n"};
  const double PI = 3.14159265358979;

  /* assign the input and output file names from the command line */
  infile  = argv[1];
  outfile = argv[2];

  /******************************/
  /* read the input from DAKOTA */
  /******************************/
  fileptr = fopen(infile,"r");

  /* get the number of variables */
  fscanf(fileptr,"%d %80s",&num_vars,in_str);

  /* get the values of the variables and the associated tag names */
  fscanf(fileptr,"%lf %80s",&H,in_str);
  fscanf(fileptr,"%lf %80s",&D,in_str);

  /* get the number of functions*/
  fscanf(fileptr,"%d %80s",&num_fns,in_str);

  /* get the evaluation type request */
  for (i=0; i<num_fns; i++)
    fscanf(fileptr,"%d %80s",&req[i],in_str);

  fclose(fileptr);

  /********************************************************/
  /* compute the objective function and constraint values */
  /********************************************************/
  if (req[0]==1)
    fval[0]=0.644*PI*D*D+1.04*PI*D*H;
  if (req[1]==1)
    fval[1]=0.25*PI*H*D*D-63.525;

  /****************************************/
  /* write the response output for DAKOTA */
  /****************************************/
  fileptr = fopen(outfile,"w");

  for (i=0; i<num_fns; i++)
    if (req[i])
      fprintf(fileptr,"%23.15e %s",fval[i],valtag[i]);

  fclose(fileptr);
  return 0;
}
