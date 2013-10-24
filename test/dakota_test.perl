#!/usr/bin/perl

# See documentation at end, or run dakota_test.perl --man
#  dakota_test.perl [options] [filename(s)] [test_number]

#use strict;
#use warnings 'all';
use Getopt::Long;
use Pod::Usage;
use File::Basename;
use File::Path 'rmtree';
use File::Spec;
use POSIX "sys_wait_h";
use POSIX "uname";
use Cwd 'abs_path';
use Config;

# set default options (global to this script)
my $baseline_filename = "";  # default is dakota_[p]base.test.new
my $bin_dir = "";            # default binary location is pwd (none)
my $bin_ext = "";            # default extension is empty
my $extract_filename = "";   # default is dakota_*.in_
my $input_dir = "";          # default test file source is pwd
my $mode = "run";            # modes are run, base, extract
my $output_dir = "";         # default output is pwd
my $parallelism = "serial";  # whether DAKOTA runs in parallel
my @test_inputs = ();        # input files to run or extract
my $test_num = undef;        # undef since can be zero
my $using_qsub = 0;
my $using_slurm = 0;

# Use default extension .exe on Windows and Cygwin
if ( $Config{osname} =~ /MSWin/ || $Config{osname} =~ /cygwin/ ) {
  $bin_ext = ".exe";
}

# exit code summarizing worst test condition found (higher is nominally "worse")
# might need to change if we go back to baselines that permit failures
#   0 PASS
#  70 PASS, but baseline lookup failure (unused)
#  80 DIFF
#  90 FAIL, consistent with baseline
#  93 FAIL (ran, but missing baseline)
#  96 FAIL (ran, but baseline says failure)
# 100 killed due to timeout
# 101 killed due to stale output
# 102 killed due to no output
# 103 aborted by SIG_INT
# 104 error or abort appeared in output
# 105 unknown or other FAIL
my $summary_exitcode = 0;

# numeric regular expressions
my $e = "-?\\d\\.\\d+e(?:\\+|-)\\d+"; # numerical field: exponential
my $f = "-?\\d+\\.?\\d*";             # numerical field: floating point
my $i = "-?\\d+";                     # numerical field: integer notation
my $ui = "\\d+";                      # numerical field: unsigned integer

# command line processing may adjust above global variables
process_command_line();

# optionally prepend bin-dir to the PATH
if ("${bin_dir}" gt "") {
  if ($Config{osname} =~ /MSWin/) {
    $ENV{'PATH'} = "${bin_dir};$ENV{'PATH'}"; 
  }
  else {
    $ENV{'PATH'} = "${bin_dir}:$ENV{'PATH'}"; 
  }
}
my $env_path = $ENV{'PATH'}; 
print "Testing executables in $bin_dir, PATH=$env_path\n"; 

if (defined $test_num) {
  print "Testing in $parallelism, mode = $mode, test_num = $test_num\n";
}
else {
  print "Testing in $parallelism, mode = $mode\n";
}

if ($mode ne "extract") {
  manage_parallelism();
}

# create new baseline file for output from all tests
if ($mode eq "base") {
  open (TEST_OUT, ">${output_dir}${baseline_filename}") ||
    die "Error: cannot open ${output_dir}${baseline_filename}\n$!";
}

# for each input file perform test actions
foreach my $file (@test_inputs) {

  print "Testing $file\n";  # name of source test input file

  # get the intermediate file names
  my($base_filename, $output, $error, $input, $test, $restart_file) = 
    get_filenames($file);

  if ($mode eq "base") {
    # annotate baseline with test filenames
    print TEST_OUT "$base_filename\n";
  }
  elsif ($mode eq "run") { 
    # if normal test mode, open individual test output file
    open (TEST_OUT, ">$test") || die "cannot open output file $test\n$!";
  }

  # In order to define multiple tests within one input file, we utilize
  # a # followed by a number, such as #3, to denote a specific test.
  # Check input file for multiple tests, uncomment lines needed for a
  # test, and comment out lines not needed for a test.
  my $cnt = ( defined $test_num ) ? $test_num : 0;
  my $num_proc = 0;   # number of CPUs for a given test
  my $restart = "";   # no restart by default
  my $found = 1;
  my $output_generated = 0;
  # pass through the loop at least twice since, in some cases, the #0 test is
  # not marked (the #1-#n tests can be additions to the #0 test, rather than
  # substitutions; in this case the #0 shared parts cannot be marked since the
  # logic is to comment out all marked #0 lines for the #1-#n tests).
  while ( $found == 1 || 
	  ( $parallelism eq "serial" && !defined $test_num && $cnt <= 1 ) ) {

    $found = 0;

    # open original input file
    open (INPUT_MASTER, "$file") ||
      die "cannot open original input file $file\n$!";
    # open temporary input file
    open (INPUT_TMP, ">$input") || die "cannot open temp file $input\n$!";

    # define a parallel-qualified count to be $cnt or p$cnt
    my $pq_cnt = ($parallelism eq "parallel") ? "p$cnt" : "$cnt";

    # trailing delimiter is important to avoid matching #nn with #n
    my $test0_tag = "(\\s|,)#0(\\s|,|\\\\)";
    my $test_tag = "(\\s|,)#$pq_cnt(\\s|,|\\\\)";

    # per-test defaults for dakota command, input, restart, and timeout
    my $dakota_command = "dakota";
    my $dakota_args = "";
    my $dakota_input = $input;
    # Default is to write a unique restart per test, named for the test input
    my $restart = "";
    my $restart_command = "-write_restart $restart_file";
    # test timeout parameters (in seconds): these may be overridden by
    # individual test inputs through tdMM,taNN for delay and absolute timeout,
    # respectively
    my $delay = 60;      # delay before checking for file size changes (60 sec)
                         # test terminated if output stagnant for this time
    my $timeout = 1200;  # absolute timeout for a single job (20 minutes)

    # read input file until EOF
    while (<INPUT_MASTER>) { # read each line of file

      # parse out number of CPUs for parallel test
      parse_num_proc($_, $cnt, \$num_proc);

      # parse out dakota command line arguments
      parse_dakota_command($_, $pq_cnt, $restart_file,
                           \$dakota_command, \$dakota_args, \$dakota_input, 
			   \$restart, \$restart_command);

      # parse out timeout and delay options
      parse_timeout($_, $cnt, \$delay, \$timeout);

      # Extracts a particular test (using pretty output) for inclusion in docs.
      # Does not deactivate graphics.  Does not set $found, such that the test
      # is not executed and the loop exits after one pass.
      if ($mode eq "extract") {

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

    }  # end read each line of file

    # print extra carriage return in case last stored line has newline escape
    print INPUT_TMP "\n";

    # close both files
    close (INPUT_MASTER); # or could rewind it
    close (INPUT_TMP);

    # if a new test series was found run test, else stop checking this
    # input file for new tests

    if ( $found == 1 || 
	 ( $cnt == 0 && $parallelism eq "serial" && $mode ne "extract" ) ) {

      print "Test Number $cnt ";

      # For workdir tests, need to remove trydir*
      if ( $file eq "dakota_workdir.in" ) {
	my @trydirlist = glob("trydir*");
	for my $tdir (@trydirlist) {
	  rmtree $tdir;
	}
      }

      my $test_command = 
        form_test_command($num_proc, $dakota_command, $dakota_args,
			  $restart_command, $dakota_input, $output, $error);

      my $pt_code = protected_test($test_command, $output, $delay, $timeout);
      $output_generated = 1;

      # Uncomment these lines to catalog data from each run
      #rename $dakota_input, "$file.$cnt";
      #system("cp $output $output.$cnt");# leave existing $output for processing
      #rename "dakota_tabular.dat", "dakota_tabular.dat.$cnt";

      # parse out return codes from $? (the Perl $CHILD_ERROR special variable)
      # TODO: instead use POSIX W*() functions to check status
      # Note: mpirun does not seem to reliably return error codes for
      #       MPI_Abort'ed runs
      my $exit_value  = $pt_code >> 8;
      my $signal_num  = $pt_code & 127;
      my $dumped_core = $pt_code & 128;
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
	parse_test_output($output);
      }
      else {
        # if the test failed, don't parse out any results
	print "failed with exit code $exit_value";
	print TEST_OUT "Test Number $cnt failed with exit code $exit_value";
	append_error_message($exit_value);
      }

      # For workdir tests, need to remove trydir*
      if ( $file eq "dakota_workdir.in" ) {
	my @trydirlist = glob("trydir*");
	for my $tdir (@trydirlist) {
	  rmtree $tdir;
	}
      }

    }

    # TODO: error if a specified test not found
    if (defined $test_num) {
      $found = 0;      # exit loop after one test
    }
    else {
      $cnt = $cnt + 1; # increment test counter
    }

  }  # end while tests remain


  # Note: this does not currently support nth_test diffing.
  if ($mode eq "run" && $output_generated == 1) { 
    # if normal mode, generate diffs
    close(TEST_OUT);
    my $diff_path = abs_path($0);
    $diff_path = dirname($diff_path);
    my $diff_script = File::Spec->catfile($diff_path, "dakota_diff.perl");
    # diff the test output against the base output and save to a file
    my $perlexe = $Config{perlpath};
    if ($parallelism eq "parallel") {
      system("${perlexe} ${diff_script} $base_filename $input_dir" . 
	     "dakota_pbase.test $test >> $output_dir" . 
	     "dakota_pdiffs.out");
    }
    else {
      system("${perlexe} ${diff_script} $base_filename $input_dir" . 
	     "dakota_base.test $test >> $output_dir" .
	     "dakota_diffs.out");
    }

    # track the worst exit code seen; have to shift since this is the
    # output of wait
    my $dd_exitcode = $? >> 8;
    $summary_exitcode = $dd_exitcode if $summary_exitcode < $dd_exitcode;
  }
  # remove unneeded files (especially $input since the last instance of this
  # file corresponds to the #(n+1) tests for which $found == false).
  if ($mode ne "extract") {
    unlink $input;
    unlink $output;
    unlink $error;
    # Remove restart if not explicitly requested
    if ( ! ($restart =~ /w/) ) {
      unlink $restart_file;
    }
  }


} # end foreach file

if ($mode eq "base") {
  close(TEST_OUT);
}

print "Testing Script Complete.\n";

exit $summary_exitcode;

# ---------------
# STARTUP Helpers
# ---------------


# Uses global @ARGV.  May set global option variables:
#   mode, parallelism, test_num
#   directories input_dir and output_dir
#   file names baseline_filename and extract_filename
#   array test_inputs
sub process_command_line {

  # Was Cmake used to substitute the source directory name? Can't just use
  # the literal as it will be replaced.  Allow user override by long opt.
  my $cmake_source_dir = "@CMAKE_CURRENT_SOURCE_DIR@";
  if ($cmake_source_dir !~ /^@/ && 
      $cmake_source_dir !~ /CMAKE_CURRENT_SOURCE_DIR@$/) {
    $input_dir = ${cmake_source_dir};
  } 

  my $opt_base = 0;
  my $opt_extract = 0;
  my $opt_help = 0;
  my $opt_man = 0;
  my $opt_parallel = 0;

  # Process long options
  GetOptions('base'           => \$opt_base,
  	     'bin-dir=s'      => \$bin_dir,
	     'bin-ext=s'      => \$bin_ext,
  	     'extract'        => \$opt_extract,
  	     'file-base=s'    => \$baseline_filename,
  	     'file-extract=s' => \$extract_filename,
  	     'help|?'         => \$opt_help,
  	     'input-dir=s'    => \$input_dir,
  	     'man'            => \$opt_man,
  	     'output-dir=s'   => \$output_dir,
  	     'parallel'       => \$opt_parallel
	     ) || pod2usage(1);
  pod2usage(0) if $opt_help;
  pod2usage(-exitstatus => 0, -verbose => 2) if $opt_man;
  
  # parallel options
  if ($opt_parallel) {
    $parallelism = "parallel";
  }
  
  # extraction and baseline options
  if ($opt_extract || $extract_filename) {
    if ($opt_base || $baseline_filename) {
      die "Error: cannot specify --base* and --extract* together";
    }
    $mode = "extract";
  }
  elsif ($opt_base || $baseline_filename) {
    $mode = "base";
    # default baseline filenames
    if (! $baseline_filename) {
      if ($parallelism eq "parallel") {
        $baseline_filename = "dakota.pbase.test.new";
      }
      else {
        $baseline_filename = "dakota.base.test.new";
      }
    }
  }

  # executable extension must have leading dot
  if ( length($bin_ext) > 0 && substr($bin_ext, 0, 1) ne ".") {
    $bin_ext = ".${bin_ext}"
  }

  # directory options
  # (only append a slash if using an alternate path, so local tests still work)
  if ($bin_dir) {
    $bin_dir .= "/";
  }
  if ($input_dir) {
    $input_dir .= "/";
  }
  if ($output_dir) {
    $output_dir .= "/";
  }
  
  # parse any remaining command-line arguments
  if (@ARGV) {
    if ($mode eq "base") {
      die "Error: cannot specify files or individual tests in baseline mode";
    }
    foreach my $arg (@ARGV) {
      if ($arg =~ /^[0-9]+$/) {      
        # a specified test number (exact match on integers)
        if (defined $test_num) {
          die "Error: only one test number may be specified";
        }
        $test_num = $arg;
      }
      else {
        # Assume argument specifies input file(s).
	#
	# Depending on location of test inputs (in or out of source),
	# arg might be a literal glob expression "dakota_d*.in" or
	# might be a single input file from a specified list.  Use
	# glob to handle both cases.
        push @test_inputs, glob("$input_dir" . "$arg");
      }
    }
  }
  else {
    # default for all test modes is all dakota_*.in files in $input_dir
    @test_inputs = glob("$input_dir" . "dakota_*.in");
  }
  
  # extract mode needs a number; if no filename specified, do all
  if ($mode eq "extract" && !defined $test_num) {
    die "Error: test number required for nth-test extract mode";
  }

}


# Set options specific to parallel runs or platforms and create machinefile. 
# Uses global parallelism variable
# May set environment variables and global option variables: 
#   using_qsub, using_slurm
sub manage_parallelism {

  # TODO: better detect SLURM and PBS clusters

  # If either serial or parallel, and running on a torque/moab platform,
  # this script should have been called inside a 'qsub'.  If not, don't run 
  # the tests.  If okay, set $using_qsub flag for mpiexec parallel choice later.
  # Similar behavior for slurm environment and $using_slurm flag.
  if ( defined $ENV{SNLSYSTEM} && $ENV{SNLSYSTEM} eq "tlcc" ) {
     $using_slurm = 1;
     if ( ! $ENV{SLURM_JOB_ID} ) {
       die "On this cluster, must run dakota tests with sbatch \n";
     }
  }
  elsif ( defined $ENV{TORQUEHOME} && -d $ENV{TORQUEHOME} ) {
     $using_qsub = 1;
     if ( !-e $ENV{PBS_NODEFILE} ) {
       die "On this cluster, must run dakota tests with qsub \n";
     }
  }
  
  # Create a machines file for platforms where this is needed.
  # (always on AIX; if parallel on others)
  my ($sysname, $nodename, $release, $version, $machine) = POSIX::uname();
  if ( $sysname =~ /AIX/ ||
       ( $parallelism eq "parallel" &&
         ($sysname =~ /Darwin/ || $sysname =~ /Linux/ || $sysname =~ /SunOS/) ) ) {
    open (MACHINEFILE, ">machines") || die "cannot open machines\n$!";
    for (my $count = 0; $count < 10; $count++) {
      print MACHINEFILE "$nodename\n";
    }
    close (MACHINEFILE);
    if ( $sysname =~ /AIX/ ) {
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
  if ( $parallelism eq "parallel" ) {
    $ENV{'OMPI_MCA_mpi_warn_on_fork'} = '0';
  }
}


# Take a (potentially fully-qualified) test input file name and create all
# needed output file names, possibly fully-qualified; uses optional
# (global) variables $output_dir and $extract_filename
sub get_filenames {
  my $qualified_filename = shift(@_);
  my $output_filename = "";
  my($filename, $directories, $suffix) = fileparse($qualified_filename);
  if ($output_dir) {
    $output_filename = "$output_dir" . "$filename";
  } 
  else {
    # use the base file name if no output dir specified (output to pwd)
    $output_filename = "$filename";
  }

  # create necessary filenames, then change suffixes
  my $output = $output_filename;
  my $error = $output_filename;
  my $input = $output_filename;
  my $test = $output_filename;
  my $restart_file = $output_filename;

  substr($output,-2, 2) = "out";
  substr($error, -2, 2) = "err";
  substr($input, -2, 2) = "in_";
  substr($test,  -2, 2) = "tst";
  substr($restart_file, -2, 2) = "rst";

  # allow override of the extract name used
  if ($extract_filename) {
    $input = "$output_dir" . "$extract_filename";
  }

  return ($filename, $output, $error, $input, $test, $restart_file);
}



# ------------------------
# INPUT file parse helpers
# ------------------------


# Parse an input file line for processor counts
# Uses globals parallelism, ui and optionally sets $num_proc by reference
sub parse_num_proc {

  my ($line, $cnt, $ref_num_proc) = @_;

  # get the # of processors for the parallel test
  if ($parallelism eq "parallel" && $line =~ /p$cnt=($ui)/ ) {
    ${$ref_num_proc} = $1;
    print "No. of processors = ${$ref_num_proc}\n";
  }

}


# Parse an input file line for optional DAKOTA commands
# Takes [p]$cnt (qualified with p if needed)
# Sets commands by reference
sub parse_dakota_command {


  my ($line, $pq_cnt, $restart_file, $ref_dakota_command, ,$ref_dakota_args,
      $ref_dakota_input, $ref_restart, $ref_restart_command) = @_;

  # allow override of the default DAKOTA command with
  # D='alternate_command -with_args'
  if ( $line =~ /(#|\s+)$pq_cnt=D\'([\w\-]*)\'/ ) {
    ${$ref_dakota_command} = $2;
    print "Using alternate dakota command \'${$ref_dakota_command}\'\n";
  }

  # allow optional dakota command arguments with 
  # DA='-with_args'
  if ( $line =~ /(#|\s+)$pq_cnt=DA\'([\w\-]*)\'/ ) {
    ${$ref_dakota_args} = $2;
    print "Using alternate dakota args \'${$ref_dakota_args}\'\n";
  }

  # allow override of the default test input file name with
  # I='alternate.in'
  # allows use of empty filename or alternate to dakota_test.in_
  if ( $line =~ /(#|\s+)$pq_cnt=I\'([\w\- ]*)\'/ ) {
    ${$ref_dakota_input} = $2;
    print "Using alternate dakota input file name \'${$ref_dakota_input}\'\n";
  }

  # determine if restart file read/write is needed (R[rwsn]+) for this test
  # TODO: allow specification of name to read/write for each test
  #       and allow stop specification
  if ( $line =~ /(#|\s+)$pq_cnt=R([rwsn]+)/ ) {
    ${$ref_restart} = $2;
    if ( (${$ref_restart} =~ /r/ || ${$ref_restart} =~ /w/) && 
	 ${$ref_restart} =~ /n/ ) {
      die "Restart file: (n)one option can't be used with (r)ead or (w)rite; " .
	  "exiting\n";	    
    }
    elsif ( ${$ref_restart} =~ /r/ ) {
      # always write a named file if we're reading one (r or rw case)
      ${$ref_restart_command} =
        "-read_restart $restart_file -write_restart $restart_file";
      print "Restart file: reading and writing $restart_file\n";
    }
    elsif ( ${$ref_restart} =~ /w/ ) {
      ${$ref_restart_command} = "-write_restart $restart_file";
      print "Restart file: writing $restart_file\n";
    }
    elsif ( ${$ref_restart} =~ /n/ ) {
      ${$ref_restart_command} = "";
      print "Restart file: explicitly removing restart arguments\n";
    }	
    else {
      print "Restart file: invalid option ${$ref_restart}; " . 
	    "default writing to $restart_file\n";
      ${$ref_restart} = "";
    }
  }

}


# Parse an input file line for optional timeout
# Takes [p]$cnt (qualified with p if needed)
# Sets timeouts by reference
sub parse_timeout {

  my ($line, $pq_cnt, $ref_delay, $ref_timeout) = @_;

  # get any adjustments to output delay (TDmm) or absolute timeout (TAnn)
  if ( $line =~ /(#|\s+)$pq_cnt=TD($ui)/ ) {
    ${$ref_delay} = $2;
    print "Output delay overridden to ${$ref_delay} seconds\n";
  }
  if ( $line =~ /(#|\s+)$pq_cnt=TA($ui)/ ) {
    ${$ref_timeout} = $2;
    print "Test timeout overridden to ${$ref_timeout} seconds\n";
  }

}



# -----------------
# EXECUTION Helpers
# -----------------

# Assemble the test command based on active options;
# relies on global $parallelism 
sub form_test_command {

  my ($num_proc, $dakota_command, $dakota_args, $restart_command,
      $dakota_input, $output, $error) = @_;

  my $fulldakota = "${bin_dir}${dakota_command}${bin_ext} ${dakota_args} $restart_command $dakota_input";
  my $redir = "> $output 2> $error";     

  # default serial command
  my $test_command = "$fulldakota $redir";
 
  if ($parallelism eq "parallel") {
    my ($sysname, $nodename, $release, $version, $machine) = POSIX::uname();
    # parallel test
    if ( $using_qsub == 1 || $using_slurm == 1 ) {
      $test_command = "mpiexec -np $num_proc $fulldakota $redir\n";
    }
    elsif ($sysname =~ /AIX/) {
      # TODO: perform poekill after specified time
      $test_command = "poe $fulldakota -procs $num_proc $redir";
    }
    elsif ($sysname =~ /SunOS/) {
      $test_command = "mprun -np $num_proc $fulldakota $redir";
    }
    else { 
      # default for Linux workstation
      $test_command = 
        "mpirun -np $num_proc -machinefile machines $fulldakota $redir";
    }
  }

  return $test_command;

}


# The following subroutines (protected_test and fork_dakota) adapted from Xyce
# test harness, with thanks to Todd Coffey
#
# 
# protected_test(test_command, output_file) # output file to monitor
#   uses global variables timeout and delay
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
  my ($test_command, $output_file, $pid, $exitcode, $t0, $tn, $tnp1, $dead_pid,
      $elapsed_time, $incr_time, $filesize, $filesize_old, $delay, $timeout);

  ($test_command, $output_file, $delay, $timeout) = @_;

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
	if ( -f "$output_file" ){
	  $filesize = (stat("$output_file"))[7];
	  if (defined ($filesize_old) and ($filesize <= $filesize_old)) {
	    print("killing $pid -- $output_file unchanged for $delay seconds\n");
	    kill -9, $pid; # TERM -> process group
	    $dead_pid = waitpid($pid,0);
	    $exitcode = 101 << 8;
	  }
	  $filesize_old = $filesize;
	}
	else {
	  print("killing $pid -- $output_file not created in $delay seconds\n");
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
      if ( $Config{osname} !~ /MSWin/ ) {
        setpgrp(0,0); # This sets process group so I can kill this + children
      }
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



# -------------------------
# OUTPUT file parse helpers
# -------------------------


# Process the test output, matching on relevant content on which to diff
sub parse_test_output {

  my ($output) = @_;

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

    #if (/^Condition number for LLS using LAPACK/) {
    #  print;
    #  print TEST_OUT;
    #}
    
    if (/Moment-based statistics for each response function:/) {
      print;
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line (Mean/StdDev/Skew/Kurt header)
      print;
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line (secondary tag header or table data)
      if (/^\s*\w+$/) { # PCE w/ expansion _and_ numerical moments
        while (/^\s*\w+$/) {
          $_ = <OUTPUT>; # grab next line (table data)
          while (/\s+$e/) {
    	  print;
    	  print TEST_OUT;
    	  $_ = <OUTPUT>; # grab next line
          }
        }
      }
      else { # PCE/SC w/ expansion _or_ numerical moments
        while (/\s+$e/) {
    	print;
          print TEST_OUT;
          $_ = <OUTPUT>; # grab next line
        }
      }
    }

    if (/95% confidence intervals for each response function:/) {
      print;
      print TEST_OUT;
      $_ = <OUTPUT>; #grab next line(LowerCI_mean/UpperCI_mean/LowerCI_stdev/UpperCI_stdev)
      print;
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line (secondary tag header or table data)
      while (/\s+$e/) {
        print;
        print TEST_OUT;
        $_ = <OUTPUT>; # grab next line
      }
    }

    while (/^\w+ Sobol indices:/) {
      print;
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line (header)
      print;
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line (table data)
      while (/^\s+$e/) {
        print;
        print TEST_OUT;
        $_ = <OUTPUT>; # grab next line
      }
      if (/Interaction/) { # header of optional section
        #print;          # process as one big table in dakota_diff.perl
        #print TEST_OUT; # process as one big table in dakota_diff.perl
        $_ = <OUTPUT>; # grab next line (table data)
        while (/^\s+$e/) {
          print;
          print TEST_OUT;
          $_ = <OUTPUT>; # grab next line
        }
      }
    }
    
    while (/of Polynomial Chaos Expansion for/) {
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

    while (/^(\s+(Response Level|Resp Level Set)\s+Probability Level\s+Reliability Index\s+General Rel Index|\s+Response Level\s+Belief (Prob Level|Gen Rel Lev)\s+Plaus (Prob Level|Gen Rel Lev)|\s+(Probability|General Rel) Level\s+Belief Resp Level\s+Plaus Resp Level|[ \w]+Correlation Matrix[ \w]+input[ \w]+output\w*:)$/) {
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

  } # end while output lines

  # DMD (05/01/2006): the following "if" line may be needed if
  # 0 function evaluations are performed (i.e., an error has occurred)
  # and DAKOTA still returns with exit code 0
  #if ($numevals == 0) {
  #  print "<<<<< Best data captured at function evaluation 0\n";
  #  print TEST_OUT "<<<<< Best data captured at function evaluation 0\n";
  #}
  close(OUTPUT);

}


# For a test throwing an error, append a string error message to the file
sub append_error_message {

  my ($exit_value) = @_;

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




# dakota_test.perl plain old documentation, for pod2usage

=head1 NAME

dakota_test.perl - Run or extract DAKOTA regression tests

=head1 SYNOPSIS

dakota_test.perl [options] [filename(s)] [test_number]

=head1 OPTIONS

=over 4

=item B<--help>

print brief help message
 
=item B<--man>  

print full manual page and exit

=item B<--parallel>

run parallel tests

=item B<--base>                  

create baseline file (default dakota_[p]base.test.new); cannot be
specified with extract options

=item B<--file-base=filename>    

create specified baseline file; implies --base

=item B<--extract>

extract test specified by test_number (required) from each
testfilename to testfilename.in_; cannot be specified with base
options

=item B<--file-extract=filename> 

extract test specified by test_number (required) from specified single
test to specified filename; cannot be specified with base options

=item B<--bin-ext=extension> 

specify the extension to append to any executable names (default .exe
on Windows and Cygwin, empty on all other platforms)

=item B<--bin-dir=filepath>

directory containing executables such as dakota

=item B<--input-dir=filepath>

directory containing test inputs and baselines

=item B<--output-dir=filepath>

for generated intermediate, diff, and baseline files

=back

=head1 DESCRIPTION

This perl script runs one or more Dakota regression tests, captures
the output, and compares the results against a baseline.  The script
may execute multiple tests for each input file as denoted by the #n
comments.

This script supports several different modes according to the command
line arguments:

=over 4

=item B<all file run mode>
 
dakota_test.perl [--parallel]

executes all serial [parallel] tests in dakota_*.in and compares
results to the relevant baseline

=item B<specified file(s) run mode>

dakota_test.perl [--parallel] dakota_f1.in [dakota_f2.in] [dakota_f*.in]

executes the serial [parallel] tests only in the specified file names
/ globs and compares results to the relevant baseline

=item B<specified file(s) run nth-test>

dakota_test.perl [--parallel] dakota_f1.in [dakota_f2.in] [dakota_f*.in] 3

execute the nth serial [parallel] test defined in each of the
specified filenames / globs (baseline compare only for 0th test)

=item B<single-file extract nth-test>

dakota_test.perl [--parallel] extract dakota_filename.in 3

extracts, but does not execute, the nth serial [parallel] test defined
in dakota_filename.in, writing dakota_filename.in_

=item B<baseline generation>

dakota_test.perl [--parallel] --base

executes all the serial [parallel] tests in the test input directory
and creates a new baseline file, dakota_[p]base.test.new

=item B<Baseline creation guidelines>

To create a new serial [parallel] baseline:

1. Run dakota_test.perl in baseline serial [parallel] mode
     dakota_test.perl [--parallel] --base

2. Review results in dakota_[p]base.test.new to make sure all tests
   executed correctly. Making sure any expected changes are present
   and are reasonable.

3. Copy the dakota_[p]base.test.new file
     cp dakota_[p]base.test.new dakota_[p]base.test

4. Commit the updated dakota_[p]base.test to the repository

=back

=cut

