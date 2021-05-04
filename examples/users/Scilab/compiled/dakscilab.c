#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <api_scilab.h>
#include <call_scilab.h>
#include <MALLOC.h>

static void Squawk(const char *fmt, ...)
{ 
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "dakscilab: ");
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  
}

void * Malloc(size_t len)
{ 
  void *rv = malloc(len);
  if (!rv)
    {
      Squawk("malloc(%lu) failure!\n", (unsigned long)len);
      exit(1);
    }

  return rv;
  
}

static int usage(int rc)
{
  char *prgnm = strdup("dakscilab");
  
  fprintf(rc ? stderr : stdout, "usage: %s [-s] [-si arg] [-sf arg] [-f arg] [-fp arg] [-fpp arg] arg\n\n"
	  "Option -s ==> start server that talks with SCILAB and\n"
	  "talks to other instances of %s over pipes that by default\n"
	  "are named dakscilab_pipe1 and dakscilab_pipe2; if shell variable\n"
	  "DAKSCILAB_PIPE is set, they are named ${DAKSCILAB_PIPE}1 and\n"
	  "${DAKSCILAB_PIPE}2.  If files with these names already exist,\n"
	  "%s complains and gives return code 1; otherwise it creates\n"
	  "the named pipes, starts SCILAB, and passes any arguments\n"
	  "after the \"-s\" to SCILAB for it to interpret as though\n"
	  "they were typed at the SCILAB prompt.\n\n"
	  "When invoked as an analysis_program by DAKOTA, the invocation is\n\n"
	  "\t%s parameters_file results_file\n\n"
	  "This invocation causes %s to talk over the named pipes with\n"
	  "a server instance of itself, so SCILAB sees assignments to x and\n"
	  "asv, followed by one or more of\n\n\t\"y = f(x);\"\n"
	  "\typ = fp(x);\n\typp = fpp(x);\n\n"
	  "(depending on what DAKOTA wants: functions, derivatives, or\n"
	  "second derivatives), after which the server retrieves y,\n"
	  "yp = y' = Jacobian of f w.r.t. x, and/or ypp = Hessians of\n"
	  "y w.r.t. x.  The server writes a results_file for DAKOTA and\n"
	  "(over the second named pipe) tells the analyis_program instance\n"
	  "to exit.  If the named pipes do not exist, %s itself\n"
	  "initially invokes\n\n\t%s -s\n\n"
	  "Writing \"quit\" or an empty file to the first named pipe, as in\n\n"
	  "\techo quit > dakscilab_pipe1\nor\n"
	  "\tcat /dev/null >dakscilab_pipe1\n\n"
	  "causes the server to remove the named pipes, shut SCILAB down\n"
	  "and exit.\n"
	  "Other options:\n"
	  "-f cmd_name   - command to execute to load the objective function (default: \"exec f.sci\")\n"
	  "-fp cmd_name  - command to execute to load the gradient function (default: \"exec fp.sci\")\n"
	  "-fpp cmd_name - command to execute to load the hessian function (default: \"exec fpp.sci\")\n"
	  "-si cmd_name  - command to execute after Scilab initialization (default: none)\n"
	  "-sf cmd_name  - command to execute before Scilab termination (default: none)\n"
	  "You can also send some strings to the Scilab server via ${DAKSCILAB_PIP}1. For example:\n"
	  "dgb cmd_name  - cmd_name is executed by the Scilab server\n"
	  "quit          - the Scilab server is terminated\n", prgnm, prgnm, prgnm, prgnm, prgnm, prgnm, prgnm);
  free(prgnm);
  return rc;
}

#define stderr stderr /* for the following */
typedef double real;
/**** Stuff modified from dakampl.c -- could it go into a common file? ****/

typedef struct FileInfo 
{
  FILE *f;
  char *fname;
  char *se;
  char *stub;
  char *nlext;
  long lineno;
  char buf[1024];
} FileInfo;

static int BadOpen(const char *what, const char *name)
{
  Squawk("Unable to open %s file \"%s\"\n", what, name);
  return 1;
}

static void BadRead(FileInfo *fi)
{
  Squawk("Unable to read line %ld of file \"%s\"\n",
	 fi->lineno, fi->fname);
  exit(1);
}

static void BadFmt(FileInfo *fi)
{
  Squawk("Bad format, line %ld of file \"%s\":\n%s\n",
	 fi->lineno, fi->fname);
  exit(1);
}

int Iread(FileInfo *fi, const char *expected)
{
  int rv, test;
  char *s, *se;
  printf("expected1 : %s\n",expected);
  ++(fi->lineno);
  if (!fgets(s = fi->buf, sizeof(fi->buf), fi->f)) BadRead(fi);

  rv = (int)strtol(s, &se, 10);
  if (s == se || *se > ' ') BadFmt(fi);
  fi->se = se;

  while(*se <= ' ' && *se)
    {
      printf("se se se se se : %s\n",se);
      printf("expected2 : %s\n",expected);
      //test = strcmp(se,expected);
      //printf("comparaison : %i",test);
      se++;
      if (expected)
	{
	  if ((strcmp(se,expected)) == 0)
	    {
	      fprintf(stderr, "dakscilab: Expected \"%s\", but got \"%s\"\n",
		      expected, se);
	      BadFmt(fi);
	    }
	}
    }
  printf("rv rv rv rv rv : %d\n\n",rv);
  return rv;
}

double Dread(FileInfo *fi)
{
  char *s, *se;
  double rv;
  
  ++fi->lineno;
  
  if (!fgets(s = fi->buf, sizeof(fi->buf), fi->f))
    {
      BadRead(fi);
    }
  
  rv = strtod(s, &se);
	
  if (s == se || *se > ' ')
    {
      BadFmt(fi);
    }
  
  fi->se = se;
  return rv;
}

static void grdout(FILE *f, int n1, int *p, real *g)
{
  int i;
  
  fprintf(f, "[");
  
  for(i = 0; i < n1; i++)
    {
      fprintf(f, " %.17g", g[p[i]]);
    }
  
  fprintf(f, " ]\n");
}

static void hesout(FILE *f, int n1, int *p, real *h)
{
  int i, j;

  fprintf(f, "[[");
  
  for(i = 0; i < n1; i++, h += n1) 
    {
      for(j = 0; j < n1; j++)
	{
	  fprintf(f, " %.17g", h[p[j]]);
	}
    }
	
  fprintf(f, " ]]\n");
}

static int exists(const char *fname, int squawk)
{
  struct stat stb;
  const char *msg[2] = { "already exists", "is not a named pipe" };
  
  if (!stat(fname, &stb)) 
    {
      if (S_ISFIFO(stb.st_mode))
	{
	  return 2;
	}	
      if (squawk)
	{
	  Squawk("\"%s\" %s.\n", fname, msg[squawk-1]);
	}
      return 1;
    }
  
  return 0;
}

/**** End of stuff from dakampl.c  ****/
/**** but process() contains some stuff from main() from dakampl.c */

static int process(int ep, char *parname, char *resname)
{
  SciErr sciErr;
  FILE *f;
  FileInfo fi;
  char *s;
  int *asv, asvkind[3], i, j, k, m1, n1, n2, nd, *p, rc;
  int nbrow, nbcol;
  real *x, *x_tmp;


  rc = 1;		
  if (!(fi.f = fopen(parname,"r")))
    {
      BadOpen("parameters_file", parname);
      goto done1;
    }
  
  fi.fname = s;
  fi.lineno = 0;

  n1 = Iread(&fi, "variables");
	
  x = (real *)Malloc(n1*sizeof(real));
	
  for(i = 0; i < n1; i++)
    {
      x[i] = Dread(&fi);
    }
  
  sciErr = createNamedMatrixOfDouble(pvApiCtx, "x", n1, 1, x);
  
  if(sciErr.iErr)
    {
      printError(&sciErr, 0);
      return sciErr.iErr;
    }

  free(x);
	
  m1 = Iread(&fi, "functions");

  asvkind[0] = asvkind[1] = asvkind[2] = 0;
  asv = (int*)Malloc(m1*sizeof(int));
  p = (int*)Malloc(n1*sizeof(int));

  for(i = 0; i < m1; i++) 
    {
      asv[i] = j = Iread(&fi,0);
      printf("asv[i] : %d\n",asv[i]);
      for(k = 0; k < 3 && j; k++, j >>= 1)
	{
	  if (j & 1)
	    {
	      ++asvkind[k];
	    }
	}	
    }
	
  nd = Iread(&fi, "derivative_variables");
	
  for(i = 0; i < nd; i++)
    {
      p[i] = Iread(&fi, 0) - 1;
    }
  
  fclose(fi.f);
  f = fopen(resname, "w");
	
  if (!f)
    {
      BadOpen("results_file", resname);
      goto done1;
    }

  x = (real *)Malloc(3*sizeof(real));
	
  for(i = 0; i < 3; i++)
    {
      x[i] = asvkind[i];
    }
  
  sciErr = createNamedMatrixOfDouble(pvApiCtx, "asv", 1, 3, x);

  if(sciErr.iErr)
    {
      printError(&sciErr, 0);
      return sciErr.iErr;
    }
  
  free(x);
  if (asvkind[0])
    {
      SendScilabJob("y = f(x);");
      sciErr = readNamedMatrixOfDouble(pvApiCtx, "y", &nbrow, &nbcol, NULL);
      
      if(sciErr.iErr)
	{
	  printError(&sciErr, 0);
	}
      
      x = (real *)Malloc(nbrow*nbcol*sizeof(real));
      sciErr = readNamedMatrixOfDouble(pvApiCtx, "y", &nbrow, &nbcol, x);
      
      if(sciErr.iErr)
	{
	  printError(&sciErr, 0);
	}
		
      if (!x)
	{
	  Squawk("Scilab didn't return y = f(x)\n");
	  goto done;
	}
      
      for(i = 0; i < m1; i++)
	{
	  if (asv[i] & 1)
	    {
	      fprintf(f, " %.17g\n", x[i]);
	    }
	}	
      free(x);
    }

  if (asvkind[1])
    {
      SendScilabJob("yp = fp(x);");

      sciErr = readNamedMatrixOfDouble(pvApiCtx, "yp", &nbrow, &nbcol, NULL);
      
      if(sciErr.iErr)
	{
	  printError(&sciErr, 0);
	}
      
      x = (real *)Malloc(nbrow*nbcol*sizeof(real));
      sciErr = readNamedMatrixOfDouble(pvApiCtx, "yp", &nbrow, &nbcol, x);
      
      if(sciErr.iErr)
	{
	  printError(&sciErr, 0);
	}
      
      if (!x)
	{
	  Squawk("Scilab didn't return yp = fp(x)\n");
	  goto done;
	}

      x_tmp = x;
      for(i = 0; i < m1; i++, x_tmp += n1)
	if (asv[i] & 2)
	  grdout(f, n1, p, x_tmp);

      free(x);
    }

  if (asvkind[2])
    {
      SendScilabJob("ypp = fpp(x);");

      sciErr = readNamedMatrixOfDouble(pvApiCtx, "ypp", &nbrow, &nbcol, NULL);
      
      if(sciErr.iErr)
	{
	  printError(&sciErr, 0);
	}
      
      x = (real *)Malloc(nbrow*nbcol*sizeof(real));
      sciErr = readNamedMatrixOfDouble(pvApiCtx, "yp", &nbrow, &nbcol, x);
      
      if(sciErr.iErr)
	{
	  printError(&sciErr, 0);
	}
      
      if (!x)
	{
	  Squawk("SCILAB didn't return ypp = fpp(x)\n");
	  goto done;
	}
		
      n2 = n1*n1;

      x_tmp = x;
      for(i = 0; i < m1; i++, x_tmp += n2)
	{
	  if (asv[i] & 4)
	    {
	      hesout(f, n1, p, x_tmp);
	    }
	}

      free(x);
    }
  rc = 0;
 done:
  fclose(f);
 done1:
  free(asv);
  return rc;
}

void trim(char *s)
{
  char *s0 = s;
  
  while(*s)
    {
      ++s;		
    }
  while(s > s0 && s[-1] <= ' ')
    {
      --s;
    }
  
  *s = 0;
}

static int mkfifo_fail(const char *pname)
{
  Squawk("Could not create \"%s\" as named pipe.\n", pname);
  return 1;
}

static int server(char *pname[2], int p[2], char *scriptINIT, char *scriptFINAL, char ** function)
{
  int ep,ts;
  FILE *f;
  char buf[4096], buf1[4096], *msg, *s, *t, *buff_script = NULL;
  int rc;

#ifdef DEBUG
  printf("here 1\n");
#endif

  if (exists(pname[0], 1) || exists(pname[1], 1))
    {
      return 1;
    }
	
#ifdef DEBUG
  printf("here 2\n");
#endif

  if (mkfifo(pname[0], 0600))
    {
      return mkfifo_fail(pname[0]);
    }
	
#ifdef DEBUG
  printf("here 3\n");
#endif

  if (mkfifo(pname[1], 0600))
    {
      unlink(pname[0]);
      return mkfifo_fail(pname[1]);
    }
	
  // SCILAB CALLING
	
#ifdef _MSC_VER
  ep = StartScilab(NULL,NULL,NULL);
#else
  ep = StartScilab(getenv("SCI"),NULL,NULL);
#endif
  if (ep == FALSE)
    {
      Squawk("Could not start the Scilab server.\n");
      return 1;
    }
  
  if (scriptINIT)
    {
      printf("Initialisation script : %s\n",scriptINIT);
      SendScilabJob(scriptINIT);
    }

  // SCILAB TASKS	
  if (function[0])
    {
      SendScilabJob(function[0]);
    }
  else
    {
      SendScilabJob("exec f.sci;");
    }

  if (function[1])
    {
      SendScilabJob(function[1]);
    }
  else
    {
      SendScilabJob("exec fp.sci;");
    }
  
  if (function[2])
    {
      SendScilabJob(function[2]);
    }
  else
    {
      SendScilabJob("exec fpp.sci;");
    }
  
  if (p[1] >= 0)
    {
      close(p[0]);
      write(p[1], "OK\n", 3);
      close(p[1]);
    }
 	
  rc = 1;
	
  for(;;)
    {
      f = fopen(pname[0], "r");
      
      if (!f)
	{
	  break;
	}
      s = fgets(buf, sizeof(buf), f);
      if (!s)
      {
	fclose(f);
	break;
      }
      
      trim(s);
 

      if (!*s)
	{
	  Squawk("server empty parameters_file name\n");
	bailout:
	  fclose(f);
	  break;
	}

      if (!strncmp(s,"dbg",3))
	{
	  SendScilabJob(s+4);
	  fclose(f);
	  continue;
	}

      if (!strcmp(s,"quit"))
	{
	  printf("dakscilab: leaving server\n");
	  rc = 0;
	  goto bailout;
	}
      
      t = fgets(buf1, sizeof(buf1), f);
      
      fclose(f);
      
      if (!t)
	{
	  Squawk("server expected 2 lines from \"%s\"; only got 1.\n",
		 pname[0]);
	  break;
      }
      
      trim(t);
      
      msg = process(ep, s, t) ? "evaluation error" : "results_file written";
      
      f = fopen(pname[1],"w");
      
      if (!f)
	{
	  Squawk("Could not open pipe2 file \"%s\"\n", pname[1]);
	  break;
	}
      
      fprintf(f, "%s\n", msg);
      fclose(f);
    }
  
  // SCILAB ENDING
  
  if (scriptFINAL)
    {
      printf("Final script : %s\n",scriptFINAL);
      SendScilabJob(scriptFINAL);
    }
  
  ts = TerminateScilab(NULL); // Argument NULL : Scilab will use the default path
  if (ts == FALSE)
    {
      Squawk("Error while closing the Scilab server\n");
      return 2;
    }
  
  unlink(pname[0]);
  unlink(pname[1]);
  return rc;

}

//#define DEBUG 1

int main(int argc, char **argv, char *const envp[])
{
  FILE *f;
  char *av[3], buf[80], *pb, *pname[2], *s, *s2, *scriptInit = NULL, *scriptFinal = NULL;
  char *function[3];
  int ex[2], i, idx, j, k, p[2], ep, ts, mark = 0;
  pid_t pid;
  size_t L, L2;

  if (!(pb = getenv("DAKSCILAB_PIPE")))
    {
      pb = "dakscilab_pipe";
    }

  L = strlen(pb);
  L2 = L + 2;
  pname[0] = (char*)Malloc(L2*sizeof(char));
  pname[1] = (char*)Malloc(L2*sizeof(char));
  strcpy(s = pname[0], pb);
  s[L] = '1';
  s[L+1] = 0;
  strcpy(s = pname[1], pb);
  s[L] = '2';
  s[L+1] = 0;

#ifdef DEBUG
  printf("DEBUG: current path: %s\n",get_current_dir_name());
  printf("       pname[0] = %s\n", pname[0]);
  printf("       pname[1] = %s\n", pname[1]);
#endif

  function[0] = NULL;
  function[1] = NULL;
  function[2] = NULL;

  if (argc < 2)
    {
      return usage(1);
    }

  buf[0] = 0;
  idx = 1;

  for(idx=1;idx<argc;idx++)
    {
      if (strcmp(argv[idx],"-h")==0)
	{
	  return usage(0);
	}
      else if (strcmp(argv[idx],"-d")==0)
	{
	  mark = 1;
	  continue;
	}
      else if (strcmp(argv[idx],"-si")==0)
	{
	  idx++;
	  scriptInit = argv[idx];
	  continue;
	}
      else if (strcmp(argv[idx],"-sf")==0)
	{
	  idx++;
	  scriptFinal = argv[idx];
	  continue;
	}
      else if (strcmp(argv[idx],"-fpp")==0)
	{
	  idx++;	  
	  function[2] = argv[idx];
#ifdef DEBUG
	  printf("DEBUG : function de 2 : %s\n",function[2]);
#endif
	  continue;
	}
      else if (strcmp(argv[idx],"-fp")==0)
	{
	  idx++;	  
	  function[1] = argv[idx];
#ifdef DEBUG
	  printf("DEBUG : function de 1 : %s\n",function[1]);
#endif
	  continue;
	}
      else if (strcmp(argv[idx],"-f")==0)
	{
	  idx++;
	  function[0] = argv[idx];
#ifdef DEBUG
	  printf("DEBUG : function de 0 : %s\n",function[0]);
#endif
	  continue;
	}
      else if (strcmp(argv[idx],"-s")==0)
	{
	  mark = 1;
	  
	  p[0] = p[1] = -1;
	    
	  if (pipe(p))
	    {
	      perror("pipe() failure");
	      return 1;
	    }

	  if ((pid = fork()))
	    {
	      close(p[1]);
	      k = read(p[0], buf, sizeof(buf));
	      close(p[0]);
	      
	      function[0] = NULL;
	      function[1] = NULL;
	      function[2] = NULL;
	      
	      if (k == 3 && !strncmp(buf,"OK\n",k))
		{
		  return 0;
		}
	      
	      Squawk("Failed to start SCILAB engine\n");
	      return 1;
	    }
	  return server(pname, p, NULL, NULL, function);
	}
      else
	{
	  break;
	}
    }

#ifdef DEBUG
  printf("DEBUG: argc = %d idx = %d argc - idx = %d\n", argc, idx, argc -idx);
#endif

  if (argc - idx != 2)
    {
      Squawk("expected two arguments, parameters_file and results_file.\n");
      return usage(1);
    }
	
  if ((ex[0] = exists(pname[0], 2)) == 1) 
    {
      Squawk("\"%s\" is not a named pipe.\n", pname[0]);
    bailout:
      printf("%s\n",argv[2]);
      creat(argv[2], 0666); /* empty file for DAKOTA to read */
      return 1;
    }
	
  if ((ex[1] = exists(pname[1], 2)) == 1)
    {
      Squawk("\"%s\" is not a named pipe.\n", pname[1]);
      goto bailout;
    }
  
  if ((k = ex[0] + ex[1]) == 2)
    {
      i = ex[1] == 2;
      j = 1 - i;
      Squawk("\"%s\" is a named pipe, but \"%s\" is not.\n",
	     pname[i], pname[j]);
      goto bailout;
    }
  
  if (!k)
    {
      if (pipe(p))
	{
	  perror("pipe() failure");
	  return 1;
	}
      
      pid = fork();
      if (!pid)
	{
	  // Server start
	  if (mark) 
	    {
	      if (scriptInit && scriptFinal)
		{
		  return server(pname, p, scriptInit, scriptFinal, function);
		}
	      else if(scriptInit)
		{
		  return server(pname, p, scriptInit, NULL, function);
		}
	      else if(scriptFinal)
		{
		  return server(pname, p, NULL, scriptFinal, function);
		}
	      else
		{
		  return server(pname, p, NULL, NULL, function);
		}
	    }
	}
 
      close(p[1]);
      k = read(p[0], buf, sizeof(buf));
      close(p[0]);

#ifdef DEBUG
      printf("DEBUG: k = %d\n",k);
      printf("       bug = %s\n",buf);
      printf("       p[0] = %d\n", p[0]);
      printf("       p[1] = %d\n", p[1]);
#endif

      if (k != 3  || strncmp(buf,"OK\n",k)
	  || exists(pname[0], 0) != 2
	  || exists(pname[1], 0) != 2)
	{
	  Squawk("failed to start self as server\n");
	  goto bailout;
	}
    }
  
  f = fopen(pname[0], "w");
  
  if (!f)
    {
      Squawk("Could not open \"%s\"\n", pname[0]);
      goto bailout;
    }
  
  fprintf(f, "%s\n%s\n", argv[argc-2], argv[argc-1]);
  fclose(f);
  f = fopen(pname[1], "r");
  s = fgets(buf, sizeof(buf), f);
  fclose(f);
  
  if (!s)
    {
      Squawk("empty reply from server\n");
      goto bailout;
    }
  
  if (strcmp(s, "evaluation error\n"))
    {
      return 1;
    }
  
  if (strcmp(s, "results_file written\n"))
    {
      Squawk("bad reply from server\n");
      goto bailout;
    }
  
  return 0;
}
