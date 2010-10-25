#!/usr/bin/perl
#
# This perl script runs one or more Dakota regression tests, captures the
# output, and compares the results against a baseline.  The script may
# execute multiple tests for each input file as denoted by the #n comments.
#
# This script supports several different modes according to the command
# line arguments:
#
# -- serial multi-file mode
#
#      dakota_test.perl
#      -> executes all serial tests and compares results to the baseline
#
# -- parallel multi-file mode
#
#      dakota_test.perl parallel
#      -> executes all parallel tests and compares results to the baseline
#
# -- serial single-file mode
#
#      dakota_test.perl filename.in
#      -> executes only the serial tests defined in filename.in
#
# -- parallel single-file mode
#
#      dakota_test.perl parallel filename.in
#      -> executes only the parallel tests defined in filename.in
#
# -- serial single-file nth-test
#
#      dakota_test.perl filename.in 3
#      -> execute the nth serial test defined in filename.in
#
# -- parallel single-file nth-test
#
#      dakota_test.perl parallel filename.in 3
#      -> execute the nth parallel test defined in filename.in
#
# -- serial single-file extract nth-test
#
#      dakota_test.perl extract filename.in 3
#      -> extracts, but does not execute, the nth serial test defined
#         in filename.in
#
# -- parallel single-file extract nth-test
#
#      dakota_test.perl parallel extract filename.in 3
#      -> extracts, but does not execute, the nth parallel test defined
#         in filename.in
#
# -- serial baseline
#
#      dakota_test.perl base
#      -> executes all the serial tests in the current directory
#         and creates a new baseline file, dakota_base.test
#
# -- parallel baseline
#
#      dakota_test.perl parallel base
#      -> executes all the parallel tests in the current directory
#         and creates a new baseline file, dakota_pbase.test
#
# Baseline creating guidelines:
#
# To create a new serial baseline :
# 1. Run dakota_test.perl in baseline serial mode
#      dakota_test.perl base
# 2. Review results in dakota_base.test to make sure all tests executed
#    correctly. Making sure any expected changes are present and are reasonable.
# 3. copy the dakota_base.test file to the test/sqa directory.
#      cp dakota_base.test sqa
# 4. commit the update dakota_base.test to Subversion
#      svn commit
#
# To create a new parallel baseline :
# 1. Run dakota_test.perl in baseline parallel mode
#      dakota_test.perl parallel base
# 2. Review results in dakota_pbase.test .to make sure all tests executed
#    correctly. Making sure any expected changes are present and are reasonable.
# 3. Copy the dakota_pbase.test file to the test/sqa directory.
#      cp dakota_pbase.test sqa
# 4. commit the update dakota_pbase.test to Subversion
#      svn commit
#
#######################################################################
#
#  Usage: dakota_test.perl <parallel> <base,extract> <filename> <#>
#
#######################################################################

use POSIX "sys_wait_h";

# clean up diffs file
if (-e dakota_diffs.out) {
  unlink dakota_diffs.out;
}

# determine $parallel and $base based on command line arguments
$parallel = 0; # $parallel = 0 (serial) or 1 (parallel)
$mode     = 0; # $mode = 0 (normal), 1 (extract), or 2 (create baseline)
$nth_test = 0;
$argcnt   = 0;
if (@ARGV) {
  if (@ARGV[0] =~ /parallel/) {
    $parallel = 1;
    $argcnt++;
  }
  if (@ARGV[$argcnt] =~ /base/) {
    $mode = 2;
    $argcnt++;
    @testin = <dakota_*.in>;
  }
  elsif (@ARGV[$argcnt] =~ /extract/) {
    $mode = 1;
    $argcnt++;
    if (@ARGV[$argcnt] =~ /dakota_\w+\.in/) {
      @testin = @ARGV[$argcnt];
      $argcnt++;
      if (@ARGV[$argcnt] =~ /[0-9]+/) { # check for trailing test number
	$test_num = @ARGV[$argcnt];
	$nth_test = 1;
      }
      else {
	die "Test number required for nth-test extract mode";
      }
    }
    else {
      die "Input filename required for nth-test extract mode";
    }
  }
  elsif (@ARGV[$argcnt] =~ /dakota_\w+\.in/) {
    while (@ARGV[$argcnt] =~ /dakota_\w+\.in/) {
      push @testin, @ARGV[$argcnt];
      $argcnt++;
    }
    if (@ARGV[$argcnt] =~ /[0-9]+/) { # check for trailing test number
      $test_num = @ARGV[$argcnt];
      $nth_test = 1;
    }
  }
  else {
    @testin = <dakota_*.in>;
  }
}
else {
  @testin = <dakota_*.in>;
}
if ($nth_test == 1) {
  print "Testing parallel = $parallel, mode = $mode, test_num = $test_num\n";
}
else {
  print "Testing parallel = $parallel, mode = $mode\n";
}

# Create a new baseline file for output.  Define filename based on mode.
if ($mode == 2) {
  if ($parallel == 1) {
    open (TEST_OUT, ">dakota_pbase.test") ||
      die "cannot open dakota_pbase.test\n$!";
  }
  else {
    open (TEST_OUT, ">dakota_base.test") ||
      die "cannot open dakota_base.test\n$!";
  }
}

# If either serial or parallel, and running on a torque/moab platform,
# this script should have been called inside a 'qsub'.  If not, don't run 
# the tests.  If okay, set $qsub flag for mpiexec parallel choice later.
# Similar behavior for slurm environment and $slurm flag.
$qsub = 0;
$slurm = 0;
if ( $ENV{SNLSYSTEM} eq "tlcc" ) {
   $slurm = 1;
   if ( $mode != 1 && !$ENV{SLURM_JOB_ID} ) {
     die "On this cluster, must run dakota tests with sbatch \n";
   }
}
elsif ( -d $ENV{TORQUEHOME} ) {
   $qsub = 1;
   if ( $mode != 1 && !-e $ENV{PBS_NODEFILE} ) {
     die "On this cluster, must run dakota tests with qsub \n";
   }
}

# If parallel mode, create a machines file for platforms where this is needed.
$uname = `uname`;
if ( $mode != 1 && ($uname =~ /AIX/ ||
     ( $parallel == 1 &&
       ($uname =~ /Darwin/ || $uname =~ /Linux/ || $uname =~ /SunOS/) ) ) ) {
  $nodename = `uname -n`;
  open (MACHINEFILE, ">machines") || die "cannot open machines\n$!";
  for ($count = 0; $count < 10; $count++) {
    print MACHINEFILE "$nodename";
  }
  close (MACHINEFILE);
  if ( $uname =~ /AIX/ ) {
    # Setting environment variables for POE (for serial and parallel runs).
    $ENV{MP_EUILIB} = "ip";
    $ENV{MP_INFOLEVEL} = "0";
    $ENV{MP_HOSTFILE} = "machines";
    $ENV{MP_NODES} = "1";
    $ENV{MP_RESD} = "no";
    $ENV{MP_SHARED_MEMORY} = "yes";
  }
}

# OpenMPI fork warning will trip exit code 104 below, so disable when testing
if ( $parallel == 1 ) {
  $ENV{'OMPI_MCA_mpi_warn_on_fork'} = '0';
}

# for each input file do test
$e = "-?\\d\\.\\d+e(?:\\+|-)\\d+";# numerical field in exponential notation
$f = "-?\\d+\\.?\\d*";            # numerical field in floating point notation
$i = "-?\\d+";                    # numerical field in integer notation
$ui = "\\d+";                     # numerical field in unsigned integer notation
foreach $file (@testin) {

  # create necessary filenames
  $output = $file;
  $error  = $file;
  $tempin = $file;
  $test   = $file;
  $restart_file = $file;
  substr($output, -2, 2) = "out";
  substr($error, -2, 2)  = "err";
  substr($tempin, -2, 2) = "in_";
  substr($test,   -2, 2) = "tst";
  substr($restart_file, -2, 2) = "rst";

  # output name of test input file
  print "Testing $file\n";

  if ($mode == 2) { # annotate baseline with test filenames
    print TEST_OUT "$file\n";
  }
  elsif ($mode == 0) { # if not a baseline, open individual test output file
    open (TEST_OUT, ">$test") || die "cannot open output file $test\n$!";
  }

  # In order to define multiple tests within one input file, we utilize
  # a # followed by a number, such as #3, to denote a specific test.
  # Check input file for multiple tests, uncomment lines needed for a
  # test, and comment out lines not needed for a test.
  $cnt = 0;
  if ( $nth_test == 1 ) {
    $cnt = $test_num;
  }
  $proc = 0;
  $found = 1;
  $output_generated = 0;
  # pass through the loop at least twice since, in some cases, the #0 test is
  # not marked (the #1-#n tests can be additions to the #0 test, rather than
  # substitutions; in this case the #0 shared parts cannot be marked since the
  # logic is to comment out all marked #0 lines for the #1-#n tests).
  while ( $found == 1 || ( $parallel == 0 && $nth_test == 0 && $cnt <= 1 ) ) {

    $found = 0;

    # open original input file
    open (INPUT_MASTER, "$file") ||
      die "cannot open original input file $file\n$!";
    # open temporary input file
    open (INPUT_TMP, ">$tempin") || die "cannot open temp file $tempin\n$!";

    # trailing delimiter is important to avoid matching #nn with #n
    $test0_tag = "(\\s|,)#0(\\s|,|\\\\)";
    if ($parallel == 1) {
      $test_tag = "(\\s|,)#p$cnt(\\s|,|\\\\)";
    }
    else {
      $test_tag = "(\\s|,)#$cnt(\\s|,|\\\\)";
    }

    # defaults for dakota command, input, restart, and timeout
    $dakota_command = "dakota";
    $dakota_input = $tempin;
    $restart_command = "";
    # test timeout parameters (in seconds): these may be overridden by
    # individual test inputs through tdMM,taNN for delay and absolute timeout,
    # respectively
    $delay = 60;      # delay before checking for file size changes (60 sec)
                      # test terminated if output stagnant for this time
    $timeout = 1200;  # absolute timeout for a single job (20 minutes)

    # read input file until EOF
    while (<INPUT_MASTER>) { # read each line of file

      # get the # of processors for the parallel test
      if ($parallel == 1 && /p$cnt=($ui)/ ) {
	$proc = $1;
	print "No. of processors = $proc\n";
      }

      # allow override of the default DAKOTA command with
      # D='alternate_command -with_args'
      if ( ($parallel == 1 && /(#|\s+)p$cnt=D\'([\w\- ]*)\'/ ) ||
	   ($parallel == 0 &&  /(#|\s+)$cnt=D\'([\w\- ]*)\'/ ) ) {
	$dakota_command = $2;
	print "Using alternate dakota command \'$dakota_command\'\n";
      }

      # allow override of the default test input file name with
      # I='alternate.in'
      # allows use of empty filename or alternate to dakota_test.in_
      if ( ($parallel == 1 && /(#|\s+)p$cnt=I\'([\w\- ]*)\'/ ) ||
	   ($parallel == 0 &&  /(#|\s+)$cnt=I\'([\w\- ]*)\'/ ) ) {
	$dakota_input = $2;
	print "Using alternate dakota input file name \'$dakota_input\'\n";
      }

      # determine if restart file read/write is needed (R[rws]+) for this test
      # TODO: allow specification of name to read/write for each test
      #       and allow stop specification
      if ( ($parallel == 1 && /(#|\s+)p$cnt=R([rws]+)/ ) ||
	   ($parallel == 0 &&  /(#|\s+)$cnt=R([rws]+)/ ) ) {
	$restart = $2;
	if ( $restart =~ /r/ && $restart =~ /w/ ) {
	  $restart_command =
	    "-read_restart $restart_file -write_restart $restart_file";
	  print "Restart file: reading and writing $restart_file\n";
        }
	elsif ( $restart =~ /w/ ) {
	  $restart_command = "-write_restart $restart_file";
	  print "Restart file: writing $restart_file\n";
	}
	elsif ( $restart =~ /r/ ) {
	  $restart_command = "-read_restart $restart_file";
	  print "Restart file: reading $restart_file\n";
        }	
	else {
	  print "Restart file: invalid option $restart; disabled\n";
	  $restart = 0;
        }
      }

      # get any adjustments to output delay (TDmm) or absolute timeout (TAnn)
      if ( ($parallel == 1 && /(#|\s+)p$cnt=TD($ui)/ ) ||
	   ($parallel == 0 &&  /(#|\s+)$cnt=TD($ui)/ ) ) {
	$delay = $2;
	print "Output delay overridden to $delay seconds\n";
      }
      if ( ($parallel == 1 && /(#|\s+)p$cnt=TA($ui)/ ) ||
	   ($parallel == 0 &&  /(#|\s+)$cnt=TA($ui)/ ) ) {
	$timeout = $2;
	print "Test timeout overridden to $timeout seconds\n";
      }

      # Extracts a particular test (using pretty output) for inclusion in docs.
      # Does not deactivate graphics.  Does not set $found -> test is not
      # executed and loop exits after one pass.
      if ($mode == 1) {

	# if line contains $test_num tag, then comment/uncomment
	if (/$test0_tag/) {   # line is initially uncommented
	  if (/$test_tag/) {  # leave uncommented
	    s/#p?\d+,?//g;    # remove tags
	    print INPUT_TMP;
	  }
	  # else don't output inactive line to STDOUT
	}
	elsif (/$test_tag/) { # line is initially commented
	  s/^#//;             # uncomment line
	  s/#p?\d+,?//g;      # remove tags
	  print INPUT_TMP;
	}
	elsif (/^#/) {        # inactive line: do not output to STDOUT
	}
	else {                # active line not tagged by test number
	  print INPUT_TMP;
	}
      }
      # runs a particular test (normal output)
      else {

	# turn off graphics for all test files
	if ( s/\sgraphics(\s|,)/# graphics\n/) {
	  print INPUT_TMP;
	}
	# if line contains $cnt tag, then comment/uncomment
	elsif (/$test0_tag/) { # line is initially uncommented
	  if (/$test_tag/) {   # leave uncommented
	    $found = 1;
	    print INPUT_TMP;
	  }
	  else {               # comment it out
	    print INPUT_TMP "#$_";
	  }
	}
	elsif (/$test_tag/) {  # line is initially commented
	  $found = 1;
	  s/#//;               # uncomment line
	  print INPUT_TMP;
	}
	else {                 # line is not tagged by test number
	  print INPUT_TMP;
	}
      }
    }
    # print extra carriage return in case last stored line has newline escape
    print INPUT_TMP "\n";

    # close both files
    close (INPUT_MASTER); # or could rewind it
    close (INPUT_TMP);
    #system("cp $tempin $file.$cnt"); # for debugging

    # if a new test series was found run test, else stop checking this
    # input file for new tests

    if ( $found == 1 || ( $cnt == 0 && $parallel == 0 && $mode != 1 ) ) {

      my $pt_code;
      print "Test Number $cnt ";
      if ($parallel == 1) { # parallel test
	if ( $qsub == 1 || $slurm == 1 ) {
          #print "mpiexec -np $proc $dakota_command $dakota_input > $output\n";
          $pt_code = protected_test("mpiexec -np $proc $dakota_command $restart_command $dakota_input > $output 2> $error\n");
        }
        elsif ($uname =~ /IRIX64/) {
	  #print "mpirun -np $proc $dakota_command $dakota_input > $output\n";
	  $pt_code = protected_test("mpirun -np $proc $dakota_command $restart_command $dakota_input > $output 2> $error");
	}
	elsif ($uname =~ /AIX/) {
	  #print "poe $dakota_command $dakota_input -procs $proc > $output\n";
	  # TODO: perform poekill after specified time
	  $pt_code = protected_test("poe $dakota_command $restart_command $dakota_input -procs $proc > $output 2> $error");
	}
	elsif ($uname =~ /SunOS/) {
	  #print "mprun -np $proc $dakota_command $dakota_input > $output\n";
	  $pt_code = protected_test("mprun -np $proc $dakota_command $restart_command $dakota_input > $output 2> $error");
	}
	else { # default for Linux, ...
	  #print "mpirun -np $proc -machinefile machines $dakota_command $dakota_input > $output\n";
	  $pt_code = protected_test("mpirun -np $proc -machinefile machines $dakota_command $restart_command $dakota_input > $output 2> $error");
	}
      }
      else { # serial test
        #print "\n./$dakota_command $restart_command $dakota_input > $output 2> $error\n";
        $pt_code = protected_test("./$dakota_command $restart_command $dakota_input > $output 2> $error");
      }
      $output_generated = 1;

      # Uncomment these lines to catalog data from each run
      #rename "dakota_tabular.dat", "dakota_tabular.dat.$cnt";
      #rename $output, $output.$cnt;

      # parse out return codes from $? (the Perl $CHILD_ERROR special variable)
      # TODO: instead use POSIX W*() functions to check status
      # Note: mpirun does not seem to reliably return error codes for
      #       MPI_Abort'ed runs
      $exit_value  = $pt_code >> 8;
      $signal_num  = $pt_code & 127;
      $dumped_core = $pt_code & 128;
      #print "[exit = $exit_value, signal = $signal_num, core = $dumped_core ";
      #print "protected test code = $pt_code] ";

      # If there's anything from stderr, check it first, since MPI
      # might return $exit_code = 0 though there was an error on a child
      if ($exit_value == 0 && open (ERROR_FILE, $error)) {
        while (<ERROR_FILE>) {
	  if (/error/i || /abort/i) {
	    $exit_value = 104;
	    last;
	  }
        }
        close (ERROR_FILE);
      }

      # iff the test succeeded, parse out the results subset of interest
      if ($exit_value == 0) {

	print "succeeded\n";
        print TEST_OUT "Test Number $cnt succeeded\n";

	# open newly created output file and baseline file
	open (OUTPUT, "$output") || die "cannot open output file $output\n$!";
	# search for key phrases to create baseline
	while (<OUTPUT>) {

	  # ********************************
	  # *** Standard results summary ***
	  # ********************************

	  # Capture evaluation summaries
	  while (/^<<<<< Function evaluation summary/) {
            # DMD (05/01/2006): the following line may be needed if 
	    # 0 function evaluations are performed (i.e., an error has occurred)
	    # and DAKOTA still returns with exit code 0
            # ($numevals) = /summary: (\d+)/;
	    print;
	    print TEST_OUT;
	    $_ = <OUTPUT>; # grab next line
            # Capture detailed summaries (if output set to verbose)
	    while (/^\s*\w+: $ui val/) {
	      print;
	      print TEST_OUT;
	      $_ = <OUTPUT>; # grab next line
	    }
	  }
	  # Capture final results: best parameters, samples,
	  # objective functions, constraints, residual terms, etc.
	  while (/^<<<<< Best [ \w\(\)]+=$/) {
	    print;
	    print TEST_OUT;
	    $_ = <OUTPUT>; # grab next line
	    while (/^\s+($e|$i)/) {
	      print;
	      print TEST_OUT;
	      $_ = <OUTPUT>; # grab next line
	    }
	  }
	  if (/^<<<<< Best data (captured at|not found in)/) {
	    print;
	    print TEST_OUT;
	  }

	  # *********************************
	  # *** SBO/SBOUU results summary ***
	  # *********************************
	  #if (/^SBO (Final Design Variables|Iterations = $ui|Final Truth Response Values)$/) {
	  #  print;
	  #  print TEST_OUT;
	  #  $_ = <OUTPUT>; # grab next line
	  #  while (/^[ \w]+(=|:)\s+($e|$ui)/) {
	  #    print;
	  #    print TEST_OUT;
	  #    $_ = <OUTPUT>; # grab next line
	  #  }
	  #}

	  # ********************************************
	  # *** Concurrent strategy summaries        ***
          # *** (not critical for serial, but needed ***
          # *** for concurrent iterator parallel)    ***
	  # ********************************************
	  if (/^<<<<< Results summary:$/) {
	    print;
	    print TEST_OUT;
	    $_ = <OUTPUT>; # grab header
	    print;
	    print TEST_OUT;
	    $_ = <OUTPUT>; # grab next line
	    while (/^\s+$ui(\s+($e|$f)){2,}/) {
	      print;
	      print TEST_OUT;
	      $_ = <OUTPUT>; # grab next line
	    }
	  }

	  # ********************
	  # TO DO: MINLP summary
	  # ********************

	  # ***********************************************
	  # *** UQ sampling/reliability results summary ***
	  # ***********************************************
	  if (/(Mean =|Approximate Mean Response|Approximate Standard Deviation of Response|Importance Factor for variable|Si =)/) {
	    print;
	    print TEST_OUT;
	  }

	  if (/^\w+:\s+Min =\s+$e\s+Max =\s+$e$/) {
	    print;
	    print TEST_OUT;
	  }

	  while (/^response_fn_(\d+) Sobol|total Sobol /) {
	    print;
	    print TEST_OUT;
	    $_ = <OUTPUT>; # grab next line (header)
	    while (/^\s+$e/) {
	      print;
	      print TEST_OUT;
	      $_ = <OUTPUT>; # grab next line
	    }
	  }

	  while (/^Polynomial Chaos coefficients for/) {
	    print;
	    print TEST_OUT;
	    $_ = <OUTPUT>; # grab next line (header)
	    $_ = <OUTPUT>; # grab next line (header)
	    $_ = <OUTPUT>; # grab next line
	    while (/^\s+$e/) {
	      print;
	      print TEST_OUT;
	      $_ = <OUTPUT>; # grab next line
	    }
	  }

	  while (/^(\s+Response Level\s+Probability Level\s+Reliability Index\s+General Rel Index|\s+Response Level\s+Belief (Prob Level|Gen Rel Lev)\s+Plaus (Prob Level|Gen Rel Lev)|\s+(Probability|General Rel) Level\s+Belief Resp Level\s+Plaus Resp Level|[ \w]+Correlation Matrix[ \w]+input[ \w]+output\w*:)$/) {
	    print;
	    print TEST_OUT;
	    $_ = <OUTPUT>; # grab next line
	    print;
	    print TEST_OUT;
	    $_ = <OUTPUT>; # grab next line
	    while (/\s+$e/) {
	      print;
	      print TEST_OUT;
	      $_ = <OUTPUT>; # grab next line
	    }
	  }
	}
        # DMD (05/01/2006): the following "if" line may be needed if
	# 0 function evaluations are performed (i.e., an error has occurred)
	# and DAKOTA still returns with exit code 0
        #if ($numevals == 0) {
	#  print "<<<<< Best data captured at function evaluation 0\n";
	#  print TEST_OUT "<<<<< Best data captured at function evaluation 0\n";
        #}
	close(OUTPUT);
      }
      else {

        # if the test failed, don't parse out any results
	print "failed with exit code $exit_value";
	print TEST_OUT "Test Number $cnt failed with exit code $exit_value";
	my $error_msg;
	if ($exit_value == 100) {
	  $error_msg = " (timed out)\n";
        }
	elsif ($exit_value == 101) {
	  $error_msg = " (stale output)\n";
        }
	elsif ($exit_value == 102) {
	  $error_msg = " (no output)\n";
        }
	elsif ($exit_value == 103) {
	  $error_msg = " (abort SIGINT)\n";
        }
	elsif ($exit_value == 104) {
	  $error_msg = " (stderr contained error or abort)\n";
        }
	else {
	  $error_msg = "\n";
	}
	print "$error_msg";
	print TEST_OUT "$error_msg";

      }
    }

    if ($nth_test == 1) {
      $found = 0;      # exit loop after one test
    }
    else {
      $cnt = $cnt + 1; # increment test counter
    }
  }

  # Note: this does not currently support nth_test diffing.
  if ($mode == 0 && $output_generated == 1) { # if normal mode, generate diffs
    close(TEST_OUT);
    # diff the test output against the base output and save to a file
    if ($parallel == 1) {
      system("sqa/dakota_diff.perl $file sqa/dakota_pbase.test $test >> dakota_pdiffs.out");
    }
    else {
      system("sqa/dakota_diff.perl $file sqa/dakota_base.test $test >> dakota_diffs.out");
    }
  }
  # remove unneeded files (especially $tempin since the last instance of this
  # file corresponds to the #(n+1) tests for which $found == false).
  if ($mode != 1) {
    unlink $tempin;
    unlink $output;
    unlink $error;
  }
}
if ($mode == 2) {
  close(TEST_OUT);
}
print "Testing Script Complete.\n";

# The following subroutines (protected_test and fork_dakota) adapted from Xyce
# test harness, with thanks to Todd Coffey
#
# Note that $? is expected to be 16 bits, with exit code in the leftmost 8 bits
# This function returns: $? from waitpid if fork and child exited, $? from
# system, or one of the following codes in the leftmost 8 bits
#   100: killed due to timeout
#   101: killed due to stale output
#   102: killed due to no output
#   103: aborted by SIG_INT
sub protected_test
{
  my ($test_command, $pid, $exitcode, $t0, $tn, $tnp1, $dead_pid, $elapsed_time,
      $incr_time, $filesize, $filesize_old);

  ($test_command) = @_;
  $pid = fork_dakota($test_command);

  $exitcode = undef;
  # fork failed, use legacy system call
  if ($pid == -1) {
    print("fork failure, using system call to invoke dakota");
    system("$test_command");
    $exitcode = $?;
  }
  else {

    local $SIG{INT} = sub { kill -9, $pid;
			    $exitcode = 103 << 8;
			    print ("aborted\n");
			    return ($exitcode);  };

    $t0 = time;
    $tn = $t0;
    # TODO: consider kill -15?
    while (not defined $exitcode)
    {
      $dead_pid = waitpid($pid,&WNOHANG); # == $pid on completion
      if ($dead_pid == $pid) {
	# return the exit code from the child process
	$exitcode = $?;
      }
      $tnp1 = time;
      $elapsed_time = $tnp1 - $t0;
      if ($elapsed_time >= $timeout) {
	print("killing $pid due to timeout of $timeout seconds\n");
	kill -9, $pid; # TERM -> process group
	$dead_pid = waitpid($pid,0);
	$exitcode = 100 << 8;
      }
      $incr_time = $tnp1-$tn;
      if ($incr_time >= $delay) {
	$tn = time;
	if ( -f "$output" ){
	  $filesize = (stat("$output"))[7];
	  if (defined ($filesize_old) and ($filesize <= $filesize_old)) {
	    print("killing $pid -- $output unchanged for $delay seconds\n");
	    kill -9, $pid; # TERM -> process group
	    $dead_pid = waitpid($pid,0);
	    $exitcode = 101 << 8;
	  }
	  $filesize_old = $filesize;
	}
	else {
	  print("killing $pid -- $output not created in $delay seconds\n");
	  kill -9, $pid; # TERM -> process group
	  $dead_pid = waitpid($pid,0);
	  $exitcode = 102 << 8;
	}
      }
      select(undef, undef, undef, 0.1); # non-cpu intensive sleep of 0.1 sec
    }
    # We have now exited the Dakota run.  To handle cases where mpich's mpirun
    # lets child processes sit around, kill the whole process group.
    kill -9, $pid;
  }
  return $exitcode;
}

# attempt to fork and set process group
sub fork_dakota
{
  my ($test_command) = @_;
  my $pid;

  fork:
  {
    if ($pid=fork) {
      # parent
      return $pid;
    }
    elsif (defined $pid) {
      setpgrp(0,0); # This sets process group so I can kill this + children
      exec "$test_command";
      exit 0; # this is for when exec fails
    }
    elsif ($! == EAGAIN) {
      sleep 3;
      redo fork;
    }
    else {
      # We could not fork for some reason
      return -1;
    }
  }
}
