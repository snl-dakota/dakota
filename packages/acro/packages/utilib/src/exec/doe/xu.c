/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */

/* The C source code for 
Xu, H. (2002). An algorithm for constructing orthogonal and nearly-orthogoanl arrays with mixed levels and small runs.  Technometrics, 44, No 4, 356-368.

Note: 1. The criterion is called K2 (instead of J2).
    2. The program can be used to construct mixed-level supersaturated designs.
    3. The code can be called by R or Splus (see woa.s).


The research was supported by the National Science Foundation under DMS Grant No. 0072489 and 0204009.

Author: Hongquan Xu (hqxu@stat.ucla.edu)
*/
 
#include <utilib_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define min(x,y)  ((x) < (y) ? (x) : (y))
#define max(x,y)  ((x) > (y) ? (x) : (y))  

/* woa  parameters: OA'(m, s_1 ...s_n, 2)
long x[n*m];   n factors, m runs , by column.
long y[n*m];   used bu MaxOA() to save the best x[][] 
long xk[m];    temp 1 factor, m runs 
long c[m*m];   coincidence matrix, c[i][i] is unused
long s[n];     levels 
long w[n];     weights
long *nLoop, *nTry, *nTry2;  iteration status
*/

/* return a balanced column of length m and s levels*/
void RandomOA1(long x[], long m, long s)
{
  long i, nCur=0, no;

  /* get a random permutation of 0,1,...,m-1 */
  while (nCur < m) {
    #if defined( __MINGW32__) || defined(_MSC_VER)
    no = (unsigned long) rand() % m;
    #else
    no = (unsigned long) random() % m;
    #endif
    for (i=0; i<nCur; i++)  if (no == x[i]) break;
    if (i >= nCur)   x[nCur++] = no;
  }
  /* change to level s */
  for (i=0; i<m; i++) x[i] = x[i] % s;
}

void PrintXX(long* x, long n, long m)
{
  long i,j, *xi=x;
  for (i=0; i<n; i++, xi+=m) {
     printf("\n"); 
     for(j=0; j<m; j++) printf("%ld ", xi[j]); 
  }
}

void PrintX(long* x, long n, long m)
{
  long i,j;
  for (i=0; i<m; i++) {
     printf("\n"); 
     for(j=0; j<n; j++) 
       printf("%ld ", x[i + j*m]); 
  }
}
/* lower bound of K2, achieved by OA */
/* OABound=[(sum t[i])^2 + sum (s[i]-1)*(t[i])^2 - m*(sum w[i])^2]/2,
   where t[i]=m*w[i]/s[i] */
void OABound(long*bound, long*s, long*w, long n, long m)
{
  long   i, t, sumt,  sumst2, sumw;
  for(sumt=sumst2=sumw=0, i=0; i<n; i++){
    t = m*w[i]/s[i];
    sumt += t;  sumst2 += (s[i]-1)*t*t;  sumw += w[i];
    bound[i] = (sumt*sumt + sumst2 - m*sumw*sumw)/2;
   }
}
/* SSDBound=[(sum t[i])^2 + [sum (s[i]-1)*t[i]]^2/(m-1) - m*(sum w[i])^2]/2,
   where t[i]=m*w[i]/s[i] */
void SSDBound(long*bound, long*s, long*w, long n, long m)
{
  long   i, t, sumt,  sumst, sumw;
  for(sumt=sumst=sumw=0, i=0; i<n; i++){
    t = m*w[i]/s[i];
    sumt += t;  sumst += (s[i]-1)*t;  sumw += w[i];
    bound[i] =  (sumt*sumt + sumst*sumst/(m-1) - m*sumw*sumw)/2;
   }
}

long CalcC(long*x, long*c, long*w, long n, long m) 
{
  long i, j, k, *ci, K2;

  /* using x[n][m] to calculate c[m][m] and K2
     note that c[i][i] is unused */
  K2 = 0;  /* K2 =  sum_{i > j} c[i][j]^2 */
  for (ci=c,i=0; i<m; i++, ci+=m){
    for (j=0; j<i; j++){
      ci[j] = 0;
      for (k=0; k<n; k++) if(x[k*m+i] == x[k*m+j]) ci[j] += w[k];
      c[j*m+i] = ci[j];
      K2 += ci[j]*ci[j]; 
    }
  }
  return K2;
}
/* k: current column, sk: level, wk: weight, bound: max(OAbound, SSDbound) */
long AddOAColumn(long*x, long*c, long*xk, long sk, long wk, long k, long m, 
       long bound, long K2Prev,  long nTry)
{
  long  i, j, a, b, swapa, swapb;
  long  K2, K2Best=0, Sdif, SdifBest, try;
  long  *ci, *ca, *cb,  mwk=m*wk*wk*(m/sk-1)/2;

  /* for each randomly selected balanced column, exchange rows in the column
     until no improvement.  If it is orthogonal to others, break; else repeat
     nTry times and save the best to x[k][] and update c[][] */

  for (try=0; try < max(nTry,1); try++){ /* at least one try */

    /* Step 1:  initiation */
    RandomOA1(xk, m, sk);
    /* update K2 */
    K2 = K2Prev + mwk;
    /*printf("HERE %d %d %d %d %d %d\n", K2, K2Prev, mwk, m, wk, sk);*/
    for (ci=c,i=0; i<m; i++, ci+=m){
      for (j=0; j<i; j++)  if( xk[i] == xk[j] ) K2 +=2*ci[j]*wk;
    }
 
    /* Step 2: iterate until no improvement */
    if(K2 > bound){
      do{
	SdifBest =0;  swapa = swapb =0;  
	/* find a pair (a,b) which reduces K2 most */
	for ( a=1, ca=c+m; a<m; a++,ca+=m){
	  for(b=0,cb=c; b<a; b++,cb+=m){
	    if (xk[a] == xk[b] )  continue;
	    
	    Sdif = 0;
	    for (j=0; j<m; j++){
	      if (j==a || j==b)  continue;
	      if ( xk[j] == xk[a] )     Sdif += ca[j] - cb[j];
	      else if ( xk[j] == xk[b] )     Sdif += cb[j] - ca[j];
	    } /* for j */
	    
	    if (Sdif > SdifBest){
	      SdifBest = Sdif;     swapa = a;   swapb = b;
	    }
	  } /* for b */
	} /* for a */
	
	/*  Step 3: swap xk[a] and xk[b], update  K2 */
	if (SdifBest > 0){
	  a =swapa;  b=swapb;
	  swapa = xk[a];  xk[a] = xk[b];  xk[b] = swapa;
	  K2 -= 2*SdifBest*wk;
	}	
      } while(K2 > bound && SdifBest > 0);
    } /* end if */
    
    /* save the best result to x[k][] */
    if (K2 < K2Best || try==0){
      K2Best = K2;
      for (i=0; i<m; i++) x[k*m+i] = xk[i];   /* save xk[]to x[k][] */
    }
    if (K2Best <= bound)  break;  /* find an best column */
    
  } /* for(try) */
 
  /* Step 4: update c[m][m] by x[k][], return K2Best */
  for (i=0; i<m; i++){
    for (j=0; j<i; j++) if( x[k*m+i] == x[k*m+j] ) c[i*m+j]= c[j*m+i] += wk;
  }

  return K2Best;
}

/* woa: called by Splus or main function */
void woa(long* x, long* s, long*w, long* oabound, long* ssdbound, 
	 long* c, long* xk, long* y,
	 long* pK2, long* nColInit, long* nCol, long* nRow, 
	 long* pLoop, long* pTry, long* pTry2, double*pdfTime)
{
  long  n0=*nColInit,  n=*nCol, m=*nRow,  nTry;
  long  i, j, nOACol, K2, loop, K2Best, nOAColMax, bound;
  time_t  tStartTime = time(NULL);
#ifdef RTHRTH  
  unsigned seed = (unsigned)time(NULL);
  srandom( seed );
#endif
  #if defined(__MINGW32__) || defined(_MSC_VER)
  srand(987654321);
  #else
  srandom (987654321);
  #endif
  
  /* initial the first two columns */
  if(n0 < 2){
    n0 = 2;
    for(i=0; i<m; i++) x[0*m+i] = (i / (m/s[0]) ) % s[0];
    for(i=0; i<m; i++) x[1*m+i] = i % s[1];
  }
  /* compute the lower bounds */
  OABound(oabound,s,w,n,m);
  SSDBound(ssdbound, s,w,n,m);
  #if 0
  int ii=0;
  for (ii=0; ii<n; ii++) {
    printf("%d %d %d\n", ii, oabound[ii], ssdbound[ii]);
    }
  #endif

  for(loop=0; loop <*pLoop; loop++){ 

    /* initial the coincidence matrix c[][] */
    K2 = CalcC(x, c, w, n0, m);
    if(K2 == oabound[n0-1]) { 
      nOACol = n0;     nTry=*pTry; 
    }
    else{ /* not an OA */
      nOACol = 0;   nTry=*pTry2;
    }
    
   /* sequentially add columns by minizing K2 */
    for (i=n0; i<n; i++) {
      bound = max(oabound[i], ssdbound[i]);
      /* x[i][] and c[][] are updated after adding a column */
      K2 = AddOAColumn(x, c, xk, s[i], w[i], i, m, bound, K2,  nTry);
      /* increase nOACol if add an OA column, reduce nTry to nTry2 otherwise */
      if ( K2 == oabound[i] ) 	++nOACol;
      else nTry = *pTry2;  /* second try */
    }
    
    /* save x[][] to y[][] if it is the first time 
       or if there is a smaller K2 or if more OA columns,  */
    if (loop==0 || (*pTry2>0 && K2 < K2Best) 
	|| (*pTry2<=0 && nOAColMax < nOACol) ){ 
      nOAColMax = nOACol;  K2Best = K2;
      for (i=0; i<n; i++) for (j=0; j<m; j++)  y[i*m+j] = x[i*m+j];
    }
    if (K2Best <= bound)  break;  /* find an best design */
  }

  /* copy y[][] to x[][] */
  for (i=0; i<n; i++) for (j=0; j<m; j++)  x[i*m+j] = y[i*m+j];

  /* seconds elapsed */
  *pdfTime = difftime(time(NULL), tStartTime);

  /* pass back K2 and nOACol */
  *pK2 = K2Best;   *nColInit=nOAColMax;  *pLoop=loop;
}

/* miscellaneous and main functions */
void MsgExit(char*msg)
{
	fprintf(stderr,"\n");
	fprintf(stderr,msg);
	fprintf(stderr,"\n");
	exit(-1);
}
void *CallocMem(size_t nelem, size_t elsize)
{
  char *ptr;
  long size = (long)nelem * elsize;
  ptr = (char*)calloc(nelem, elsize);
  if (ptr == NULL)	MsgExit("calloc fail");
  while (size)	ptr[--size] = '\0';
  return ptr;
}

int main(int argc, char*argv[])
{
  long  i=0, n0=0, n=0, m=0,  nLoop, nTry2, nTry ;
  long  *x, *c, *s, *xk, *y, *oabound, *ssdbound, K2;
  double dfTime = (double) time(NULL);

  /* initiate parameters (n, m, s[]) */
  if (argc == 2) { 
    FILE* file;
    char line[200];
    s = CallocMem(1000, sizeof(long));
    file = fopen (argv[1], "r");
    if (!file) {
      printf ("unable to open specified file - exiting\n");
      exit(1);
      }
     
    fgets (line, 199, file);   
    sscanf (line, "%ld", &m);
  
    fgets (line, 199, file);
    sscanf (line, "%ld", &n);
    
    i = 0;
    while (fgets(line, 199, file)) {
      sscanf (line, "%ld", &s[i++]);
    }   
  }    
  else if (argc == 1) {
   printf("\nwoa: Sequentially adding (nearly) orthogonal columns"
     "\nUsage:"
     "\nwoa filename"
	   "\nwoa runs factors [s1 [s2 ...] ]"
	   "\nwoa runs factors  Try  [s1 [s2 ...] ]"
	   "\n  Try (default 100) > 11"
	  "\nAuthor: Hongquan Xu  (hqxu@umich.edu)  March 2000\n");
    return (-1);
  }
 
  /* process the command line parameters */
if (argc > 2) {  
   m = atol(argv[1]);  
   n = atol(argv[2]);  
   s = CallocMem(n, sizeof(long));        
/*    nLoop = atol(argv[3]); */
  }
  nLoop = 2;


  if (n<2 || m<2) MsgExit("n and m must be >= 2");
  x = CallocMem(n*m, sizeof(long));
  y = CallocMem(n*m, sizeof(long));
  xk = CallocMem(m, sizeof(long));
  oabound = CallocMem(n, sizeof(long));
  ssdbound = CallocMem(n, sizeof(long));
  c = CallocMem(m*m, sizeof(long)); 
  nTry2 = nTry= 100;
  
  if(argc > 3){
    if(atol(argv[3]) > 11){
      nTry2 = nTry = atol(argv[3]);  
      for(i=0; i<n && i<argc-4; i++) s[i] = atol(argv[i+4]);
    }
    else {
      for(i=0; i<n && i<argc-3; i++) s[i] = atol(argv[i+3]);
    }
  }

  if(i==0) { /* no levels specfied */
    s[0]=2;  /* default level */
    i=1;
  }
  for( ; i<n; i++) s[i] = s[i-1]; 
  if(nLoop < 0) { nLoop=-nLoop;  nTry2=0; }  /* oa only */
 
  /* call woa, with w[i]=s[i] */
  woa(x,s,s,oabound, ssdbound, c, xk,y, &K2, &n0, &n, &m, &nLoop, &nTry, &nTry2,  &dfTime);

  printf("\n#Runs=%ld, #levels=%ld,  levels=", m, n);
  for (i=0; i<n; i++) printf(" %ld", s[i]);
/*  PrintXX(x,n,m); */

  printf("\nBound=%ld, K2=%ld, nOACol=%ld, loop=%ld, Try=%ld, Try2=%ld, time=%.0lfs\n", 
	 max(oabound[n-1], ssdbound[n-1]), K2, n0, nLoop, nTry, nTry2, dfTime);
  PrintX(x, n, m);
  PrintX(c, m, m);
  printf ("\n");
  return (0);
}

