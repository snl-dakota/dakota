/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/* memmon.c
 * Stefan Chakerian
 * Sandia National Labs
 * Jan 2007
 */

/*
Run and monitor a program for memory usage.

Requires /proc filesystem, although getmem() can be written
for other systems, too.

Usage: memmon [options] executable args ...  see showhelp()

-l sets kernel resource limits, causing malloc to fail.  Default is unset.
-v, -vv, -q do standard things.

*/

#define KB 1024L
#define MB (KB * 1024L)
#define GB (MB * 1024L)
#define LINESIZE 1024

#if defined(unix) || defined(linux) || defined(__CYGWIN__)
static char myname[LINESIZE];
static char errmsg[LINESIZE];
#endif


#if defined(linux) || defined(__CYGWIN__)

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


extern int optind, opterr, optopt;        /* for getopt */

static char logfile[LINESIZE] = "";
static FILE *log;
static int verbose = 0, logging=0;
static long maxmem = 0;
static pid_t childpid;


void showhelp() {
  printf("Usage: %s [options] /full/path/executable args ...\n", myname);
  printf("Report memory usage information for process, in particular the maximum\nmemory used.  The commandline must be the full path to the executable.\n");
  printf("\n");
  printf("  -t, --time=LIMIT\t\tset poll time (in milliseconds) (default=1ms)\n");
  printf("  -l, --limit=SIZE\t\tset hard limit of SIZE KB for memory\n\t\t\t\t  consumption (default is none)\n");
  printf("  -k, --kb=SIZE\t\t\tsend signal %d (SIGXCPU) if memory usage is\n\t\t\t\t  greater than SIZE kilobytes\n",
          SIGXCPU);
  printf("  -m, --mb=SIZE\t\t\tsend %d if memory usage > size megabytes\n", SIGXCPU);
  printf("  -g, --gb=SIZE\t\t\tsend %d if memory usage > size gigabytes\n", SIGXCPU);
  printf("  -s, --signal=SIG\t\tsend this signal instead of %d\n", SIGXCPU);
  printf("  -o, --logfile=FILENAME\tsend output to logfile\n");
  printf("  -i, --interval=TIME\t\tmilliseconds between periodic status printouts\n");
  printf("  -v, --verbose\t\t\tmore output\n");
  printf("  -q, --quiet\t\t\tless output (default)\n");
  printf("\n");
  return;
}


/*
 * returns amount of memory consumed by pid
 * works on Linux, other OSes will have other methods
 * -1 on error (will hopefully work properly even with unsigned long retval)
 */
long
getmem(pid_t child) {
  static char procfile[LINESIZE];
  FILE *fd;
  long mem = -1L;
  long tmp;
  char item[256];

  snprintf(procfile, LINESIZE, "/proc/%d/status", child);

  if ((fd = fopen(procfile, "r")) == NULL)
    return -1;

/* Not all processes have a VmSize, e.g. Zombies */
  while (fscanf(fd, "%255s %ld %*[^\n]", item, &tmp) != EOF) {
    if (strcmp(item,"VmSize:") == 0) {

       mem = tmp;
       /*printf("HERE %ld\n",mem);*/
       break;
    }
  }
  fclose(fd);
  return mem;
}


/* exit signal, either SIGCHLD, SIGINT, or SIGTERM
 * can call with arg 0 manually
 */
void die(int sig) {
  if (sig && verbose > 1)
    fprintf(stderr, "Received signal %d\n", sig);
  if ((sig == SIGCHLD) || (sig == 0)) {
    if (verbose)
      fprintf(log, "memmon: Maximum memory used: %ld KB\n", maxmem );
    else
      fprintf(log, "memmon: %ld KB used\n", maxmem );
  }
  waitpid(childpid,0,0);
  exit(0);
}


int main(int argc, char **argv) {
  unsigned int poll = 1;        /* millisec */
  unsigned long limit = 0;        /* memory max before signal (long long?) */
  unsigned long rlimit = 0;        /* memory limit before malloc fails */
  long curmem = 0;
  /*unsigned long curtime = 0;*/
  unsigned long interval = 0;
  int sig = SIGXCPU;
  int c;
  struct sigaction nanny1, nanny2;
  struct timeval tp;                /* to get that for a running process, only terminated */
  long long ssec, csec;                /* start millisec, cur millisec */
  int numintervals=0;

  snprintf(myname, LINESIZE, "%s", argv[0]);
  log = stdout;



/* Parse command line
 */
  for (;;) {
    int optidx = 0;
    static struct option long_options[] = {
      {"time",1,0,'t'},                /* poll time in msec */
      {"poll",1,0,'t'},                /* poll time in msec */
      {"limit",1,0,'l'},        /* set malloc to fail (in kb) */
      {"mem",1,0,'k'},                /* memory maximum to send signal (in kb) */
      {"kb",1,0,'k'},                /* memory maximum to send signal (in kb) */
      {"mb",1,0,'m'},                /* memory maximum to send signal (in mb) */
      {"gb",1,0,'g'},                /* memory maximum to send signal (in gb) */
      {"signal",1,0,'s'},        /* int signal to send if mem exceeded (SIGXCPU) */
      {"help",0,0,'h'},                /* */
      {"verbose",0,0,'v'},        /* */
      {"quiet",0,0,'q'},        /* */
      {"log",1,0,'o'},                /* send normal output to logfile instead of stdout */
      {"logfile",1,0,'o'},        /* synonym for log */
      {"interval",1,0,'i'},        /* print memory usage every N milliseconds */
      {0,0,0,0}
    };


    c = getopt_long (argc, argv, "+hvqp:t:l:g:m:k:s:i:o:", long_options, &optidx);

    if (c == -1) {
      break;
    }

    switch(c) {
    case 't':
    case 'p':
      poll = atoi(optarg);
      break;
    case 'l':
      rlimit = atol(optarg) * KB;
      break;
    case 'k':
      limit = atol(optarg) * KB;
      break;
    case 'm':
      limit = atol(optarg) * MB;
      break;
    case 'g':
      limit = atol(optarg) * GB;
      break;
    case 's':
      sig = atoi(optarg);
      break;
    case 'h':
      showhelp();
      exit(0);
    case 'q':
      verbose = 0;
      break;
    case 'v':
      verbose++;
      break;
    case 'i':
      interval = atol(optarg); /* interval time in milliseconds */
      break;
    case 'o':
      strncpy(logfile, optarg, LINESIZE-1);
      if (!logging && (log = fopen(logfile, "w+")) == NULL) {
        snprintf(errmsg, LINESIZE, "%s: fopen failed for %s", myname,logfile);
        perror(errmsg);
        exit(1);
      }
      logging=1;
      break;
    default:
      showhelp();
      exit(1);
    }
  }

  if (argc < 2 || argc == optind) {
    showhelp();
    exit(1);
  }

  /* start the clock */
  gettimeofday(&tp, NULL);
  ssec = (long long) tp.tv_sec * 1000L;
  ssec += (tp.tv_usec + 500) / 1000;

  if ((childpid = fork())) {                /* parent */

    if (verbose) {
      int ac;
      fprintf(log, "memmon: %s: poll %d, limit %lu KB, signal %d\n", myname, poll,
          (limit + KB - 1)/KB, sig);
      fprintf(log, "memmon: logfile %s, interval %lu ms\n", logging?logfile:"none", interval);
      if (rlimit)
        fprintf(log, "memmon: hard limit %lu KB\n", (rlimit + KB - 1)/KB);
      fprintf(log, "command:");
      for (ac = optind; ac < argc; ++ac)
        fprintf(log, " %s", argv[ac]);
      fprintf(log, "\n");
    }

    /* install signal handlers to monitor child process */

    nanny1.sa_handler = &die;
    sigemptyset(&nanny1.sa_mask);
    if (sigaction(SIGINT, &nanny1, NULL) < 0) {
      perror("sigint handler failed");
      exit(1);
    }
    if (sigaction(SIGTERM, &nanny1, NULL) < 0) {
      perror("sigterm handler failed");
      exit(1);
    }

    nanny2.sa_handler = &die;
    nanny2.sa_flags = SA_NOCLDSTOP;
    sigemptyset(&nanny2.sa_mask);
    if (sigaction(SIGCHLD, &nanny2, NULL) < 0) {
      perror("sigchld handler failed");
      exit(1);
    }


    if (verbose)
       fprintf(log, "%s: processing file /proc/%d/status\n", myname, childpid);

    for (;;) {
      usleep( poll * 1000 );
      curmem = getmem(childpid);
      if (curmem > maxmem)
        maxmem = curmem;
      /*printf("THERE %ld %ld\n", maxmem, curmem);*/
      if (curmem < 0) {
        int status;
        waitpid(childpid, &status, WNOHANG);
        if (! WIFEXITED(status)) {
           fprintf(stderr, "%s: getmem failed\n", myname);
           }
        die(-1);
      }
      if (interval) {
        gettimeofday(&tp, NULL);
        csec = (long long) tp.tv_sec * 1000L;
        csec += (tp.tv_usec + 500) / 1000;

        if ((unsigned long long)(csec - ssec) >= interval * numintervals) {
          fprintf(log, "%llu %ld\n", csec - ssec, curmem);
          numintervals++;
        }
      }
      if (limit && ((unsigned long)curmem) > limit) {
        if (verbose)
          fprintf(stderr,
          "%s: Error: memory exceeded (%lu > %ld), sending signal %d to %d\n",
                myname, curmem, limit, sig, childpid);
        else
          fprintf(stderr, "%s: Error: memory exceeded\n", myname);
        kill (childpid, sig);
        break;
      }
    }
    die(0);
  } else {                        /* child - execv the remainder of line */
    if (logging)
      fclose(log);

    if (rlimit) {
      struct rlimit rlp;
      getrlimit(RLIMIT_AS, &rlp);
      rlp.rlim_cur = rlp.rlim_max = (rlim_t) rlimit;
      setrlimit(RLIMIT_AS, &rlp);
    }

    execv(argv[optind], argv + optind);
    /* notreached, normally */
    snprintf(errmsg, LINESIZE, "%s: execv failed for %s", myname, argv[optind]);
    perror(errmsg);
    killpg(getpgrp(), SIGINT);
  }

  exit(0);

}
#elif defined(unix)

#include <stdio.h>
#if defined(sun)
#  include <stdlib.h>
#else
#  include <getopt.h>
#endif

void showhelp() {
  printf("memmon requires proc filesystem - full help available on those systems\n");
}

int main(int argc, char **argv) {
  extern char *optarg;
  extern int optind, optopt, opterr;
  int c;

  fprintf(stderr, "%s currently supported by linux - launching job only\n", argv[0]);

  opterr=0;

  while ((c = getopt (argc, argv, "hvqp:t:l:g:m:k:s:i:o:")) != -1)
    if (c == 'h') {
      showhelp();
      exit(0);
    }

  if (argc < 2 || argc == optind) {
    showhelp();
    exit(1);
  }

  execv(argv[optind], argv + optind);        /* launch job */
  snprintf(errmsg, LINESIZE, "%s: execv failed for %s", myname, argv[optind]);
  perror(errmsg);
  exit(1);
}


#else
/* #elif defined(__MINGW32__) */

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {

    (void)argc;
    fprintf(stderr, "%s is not supported on this platform\n", argv[0]);
    exit(1);
}

#endif
