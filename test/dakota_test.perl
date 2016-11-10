#!/usr/bin/perl

# See documentation at end, or run dakota_test.perl --man
#  dakota_test.perl [options] [filename(s)] [test_number]

#use strict;
#use warnings 'all';
use Getopt::Long;
use Pod::Usage;
use File::Basename;
use File::Copy;
use File::Path 'rmtree';
use File::Spec;
use POSIX "sys_wait_h";
use POSIX "uname";
use Cwd 'abs_path';
use Config;

my $DTP_DEBUG = 0;  # set to 1 to debug

# set default options (global to this script)
my $baseline_filename = "";  # default is dakota_[p]base.test.new
my $bin_dir = "";            # default binary location is pwd (none)
my $bin_ext = "";            # default extension is empty
my @dakota_config = ();      # CMake/#define configuration of Dakota itself
my $extract_filename = "";   # default is dakota_*.in_
my $input_dir = "";          # default test file source is pwd
my $label_regex = "";        # regular expression to filter based on labels
my $mode = "run";            # modes are run, base, extract, test_props
my $output_dir = "";         # default output is pwd
my $parallelism = "serial";  # whether DAKOTA runs in parallel
my $save_output = 0;         # whether to save the .out, .err. .in_, etc.
my @test_inputs = ();        # input files to run or extract
my $test_num = undef;        # undef since can be zero
my $test_props_dir = "";     # write test properties to this directory
my $using_aprun = 0;
my $run_valgrind = 0;        # boolean for whether to run valgrind
my $vg_extra_args = "";      # append args from DAKOTA_TEST_VALGRIND_EXTRA_ARGS

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

# regular expressions for matching and extracting test results
# invalid numerical field
my $naninf = "-?(?:[Nn][Aa][Nn]|[Ii][Nn][Ff]|1\\.#INF|1\\.#IND|1\\.#QNAN(?:0+e\\+000))";
my $e = "-?\\d\\.\\d+e(?:\\+|-)\\d+"; # numerical field: exponential
my $f = "-?\\d+\\.?\\d*";             # numerical field: floating point
my $i = "-?\\d+";                     # numerical field: integer notation
my $ui = "\\d+";                      # numerical field: unsigned integer
my $s = "[A-Za-z0-9_-]+";             # alphanumeric (plus _ and -) string

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
print "Testing executables in $bin_dir, PATH=$env_path\n" if $DTP_DEBUG; 

if (defined $test_num) {
  print "Testing in $parallelism, mode = $mode, test_num = $test_num\n";
}
else {
  print "Testing in $parallelism, mode = $mode\n";
}

if ($mode eq "base" || $mode eq "run") {
  manage_parallelism();
  @dakota_config = check_dakota_config();
}

# create new baseline file for output from all tests
if ($mode eq "base") {
  open (TEST_OUT, ">${output_dir}${baseline_filename}") ||
    die "Error: cannot open ${output_dir}${baseline_filename}\n$!";
}

if ($mode eq "test_props") {
  open (PROPERTIES_OUT, ">${test_props_dir}/dakota_tests.props");
  open (USEREXAMPLES_OUT, ">${test_props_dir}/dakota_usersexamples.props");
}



# for each input file perform test actions
foreach my $file (@test_inputs) {

  # populate hash from test-selections to test-options for the whole file
  # TODO: store all options for a test by number at parse time
  %test_opts = ();  # global variable
  my ($max_serial, $max_parallel) = parse_test_options($file);
  if ($DTP_DEBUG) {
    use Data::Dumper;
    print "DEBUG: All test options for $file:\n";
    print Dumper(\%test_opts);
  }
  if ($mode eq "test_props") {
    write_test_options($file);
    next;
  }

  # skip whole test file if regex specified and doesn't match labels
  next if (!check_labels()) ;

  # determine test range, possibly a single subtest
  my $last_test = ($parallelism eq "parallel") ? $max_parallel : $max_serial;
  my @test_range = (0..${last_test});
  if (defined $test_num) {
    if ($test_num < 0 || $test_num > ${last_test}) {
      die "Test number ${test_num} not found in $file\n";
    }
    @test_range = (${test_num}..${test_num})
  }

  print "Testing $file\n";  # name of source test input file

  # get the intermediate file names
  my($base_filename, $output, $error, $input, $test, $restart_file) = 
    get_filenames($file);

  if ($mode eq "base" && ${last_test} >= 0) {
    # annotate baseline with test filenames (if there exist ser/par subtests)
    print TEST_OUT "$base_filename\n";
  }
  elsif ($mode eq "run") { 
    # if normal test mode, open individual test output file
    open (TEST_OUT, ">$test") || die "cannot open output file $test\n$!";
  }

  # Multiple tests are defined within one input file.  A specific test
  # is indicated by a comment #[sp][0-9]+: a #, followed by s or p for
  # serial or parallel, followed by an integer, such as #s3.  If no
  # markings are found, a single default serial test s0 is assumed.
  # If only p0 is found, it is assumed a default parallel test.
  # Therefore files with p0 must mark s0 if it is to exist as a
  # default test (can be marked in a stripped header comment line)

  # For each test, this loop uncomments lines needed for a test, and
  # comments out lines not needed for a test.  In some cases, the #0
  # test is not marked (the #1-#n tests can be additions to the #0
  # test, rather than substitutions; in this case the #0 shared parts
  # cannot be marked since the logic is to comment out all marked #0
  # lines for the #1-#n tests).

  # restart options used to unlink file conditionally; declare outside loop:
  my $restart = "";   # no restart options by default (opts: read, write, none)
  foreach my $cnt (@test_range) {

    # define a serial- or parallel-qualified count
    my $ser_par_cnt = ($parallelism eq "parallel") ? "p$cnt" : "s$cnt";

    # trailing delimiter is important to avoid matching #nn with #n
    my $test0_tag = "(\\s|,)#s0(\\s|,)";
    # if no serial test 0, #p0 may be an uncommented parallel test
    $test0_tag = "(\\s|,)#p0(\\s|,)" if ($max_serial < 0); 
    my $test_tag = "(\\s|,)#${ser_par_cnt}(\\s|,)";
    
    # per-test options for dakota exec, arguments, and input file
    my $dakota_command = get_test_option_value($cnt, "ExecCmd", "dakota");
    my $dakota_args = get_test_option_value($cnt, "ExecArgs", "");
    # default is dakota_input.in_
    my $dakota_input = get_test_option_value($cnt, "InputFile", "$input");

    # Default is to write a unique restart per test, named for the test input
    # no restart options by default
    $restart = get_test_option_value($cnt, "Restart", "");
    my $restart_command = parse_restart_command($restart, $restart_file);

    # per-test default for number of CPUs in parallel test = 0
    my $num_proc = get_test_option_value($cnt, "MPIProcs", 0);

    # log file to examine for output; output will still go to $output,
    # but then we'll check this file for diffs
    my $check_output = get_test_option_value($cnt, "CheckOutput", "$output");

    # per-test timeout parameters (in seconds): these may be overridden by
    # individual test inputs:
    # delay before checking for file size changes (60 sec)
    # test terminated if output stagnant for this time
    my $delay = get_test_option_value($cnt, "TimeoutDelay", 60);
    # absolute timeout for a single job (20 minutes)
    my $timeout = get_test_option_value($cnt, "TimeoutAbsolute", 1200);

    # open original input file
    open (INPUT_MASTER, "$file") ||
      die "cannot open original input file $file\n$!";
    # open temporary input file
    open (INPUT_TMP, ">$input") || die "cannot open temp file $input\n$!";

    my $last_line_blank = 0;  # for detecting subsequent blank lines

    # read input file until EOF
    while (<INPUT_MASTER>) { # read each line of file

      # no further processing for #@ (test annotation) lines
      next if /^#@/;

      # Extracts a particular test (using pretty output) for inclusion in docs.
      # Does not deactivate graphics.
      if ($mode eq "extract") {

	# if line contains $test_num tag, then comment/uncomment
	if (/$test0_tag/) {   # line is initially uncommented
	  if (/$test_tag/) {  # leave uncommented
	    s/#[sp]\d+,?//g;    # remove tags
	    s/\s*([\r\n])/$1/g; # remove trailing whitespace, leave newline
	    print INPUT_TMP;
	    $last_line_blank = 0;
	  }
	  # else don't output inactive line to STDOUT
	}
	elsif (/$test_tag/) { # line is initially commented
	  s/^#//;             # uncomment line
	  s/#[sp]\d+,?//g;      # remove tags
	  s/\s*([\r\n])/$1/g;   # remove trailing whitespace, leave newline
	  print INPUT_TMP;
	  $last_line_blank = 0;
	}
	elsif (/^#/) {        # inactive line: do not output to STDOUT
	}
	else {                # active line not tagged by test number
	  s/\s*([\r\n])/$1/g;   # remove trailing whitespace, leave newline
	  # in extract mode, don't print subsequent blank lines
	  if (/^\s*$/) {
	    print INPUT_TMP if ($last_line_blank == 0);
	    $last_line_blank = 1;
	  }
	  else {
	    print INPUT_TMP;
	    $last_line_blank = 0;
	  }
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
	    print INPUT_TMP;
	  }
	  else {               # comment it out
	    print INPUT_TMP "#$_";
	  }
	}
	elsif (/$test_tag/) {  # line is initially commented
	  s/#//;               # uncomment line
	  print INPUT_TMP;
	}
	else {                 # line is not tagged by test number
	  print INPUT_TMP;
	}
      }

    }  # end read each line of file

    # close both files
    close (INPUT_MASTER); # or could rewind it
    close (INPUT_TMP);

    # nothing more to do for this input file if extracting
    next if ($mode eq "extract");

    # Run the test
    print "Test Number $cnt ";

    # skip if subtest $cnt not enabled in this Dakota configuration
    my $enable_test = check_required_configs($cnt);
    if (! $enable_test) {
      print "skipped\n";
      print TEST_OUT "Test Number $cnt skipped\n";
      next;
    }

    # For workdir tests, need to remove trydir*
    # TODO: generalize to pre- and post-processing steps
    if ( $file eq "dakota_workdir.in" ) {
      my @trydirlist = glob("trydir*");
      for my $tdir (@trydirlist) {
	rmtree $tdir;
      }
    }

    my $test_command = 
        form_test_command($cnt, $num_proc, $dakota_command, $dakota_args,
			  $restart_command, $dakota_input, $output, $error);

    my $pt_code = protected_test($test_command, $output, $delay, $timeout);

    # Catalog data from each run, if requested
    if ($save_output) {
      copy("$dakota_input", "${dakota_input}.${cnt}") if (-e "$dakota_input");
      copy("$output", "${output}.${cnt}") if (-e "$output");
      copy("$error", "${error}.${cnt}") if (-e "$error");
      copy("$restart_file", "${restart_file}.${cnt}") if (-e "$restart_file");
      copy("dakota_tabular.dat", "dakota_tabular.dat.${cnt}") if (-e "dakota_tabular.dat");
    }

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
      parse_test_output($check_output);
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

  }  # foreach(test)


  if ($mode eq "run" && -e $test) { 
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
  # remove unneeded files
  if ($mode ne "extract") {
    unlink $input;
    unlink $output;
    unlink $error;
    # Remove restart if not explicitly requested
    if ( ! ($restart =~ /write/) ) {
      unlink $restart_file;
    }
  }

} # end foreach file

if ($mode eq "test_props") {
  close(PROPERTIES_OUT);
  close(USEREXAMPLES_OUT);
}

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
  my $opt_save_output = 0;
  my $opt_man = 0;
  my $opt_parallel = 0;
  my $opt_valgrind = 0;

  # Process long options
  GetOptions('base'           => \$opt_base,
  	     'bin-dir=s'      => \$bin_dir,
	     'bin-ext=s'      => \$bin_ext,
  	     'extract'        => \$opt_extract,
  	     'file-base=s'    => \$baseline_filename,
  	     'file-extract=s' => \$extract_filename,
  	     'help|?'         => \$opt_help,
  	     'input-dir=s'    => \$input_dir,
	     'label-regex=s'  => \$label_regex,
	     'save-output'    => \$opt_save_output,
  	     'man'            => \$opt_man,
  	     'output-dir=s'   => \$output_dir,
  	     'parallel'       => \$opt_parallel,
	     'test-properties=s' => \$test_props_dir,
	     'valgrind'       => \$opt_valgrind
	     ) || pod2usage(1);
  pod2usage(0) if $opt_help;
  pod2usage(-exitstatus => 0, -verbose => 2) if $opt_man;
  
  # parallel options
  if ($opt_parallel) {
    $parallelism = "parallel";
  }
  
  # extraction and baseline options
  if ($test_props_dir) {
    # short-circuits any other mode
    $mode = "test_props";
  }
  elsif ($opt_extract || $extract_filename) {
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
        $baseline_filename = "dakota_pbase.test.new";
      }
      else {
        $baseline_filename = "dakota_base.test.new";
      }
    }
  }

  # cleanup options
  if (${opt_save_output} || $ENV{'DAKOTA_TEST_SAVE_OUTPUT'}) {
    $save_output = 1;
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

  if (${opt_valgrind} || $ENV{'DAKOTA_TEST_VALGRIND'}) {
    $run_valgrind = 1;
    if (${opt_parallel} || ${using_aprun}) {
      die "Error: cannot use valgrind in parallel or aprun mode";
    }
    if ($ENV{'DAKOTA_TEST_VALGRIND'}) {
      $vg_extra_args = $ENV{'DAKOTA_TEST_VALGRIND_EXTRA_ARGS'};
    }
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
# May set environment variables and global option variable using_aprun. 
sub manage_parallelism {

  # Create a machines file for platforms where this is needed.
  # (always on AIX; if parallel on others)
  my ($sysname, $nodename, $release, $version, $machine) = POSIX::uname();
  if ( $sysname =~ /AIX/ ||
       ( $parallelism eq "parallel" && $sysname =~ /SunOS/) ) {
    open (MACHINEFILE, ">machines") || die "cannot open machines\n$!";
    for (my $count = 0; $count < 12; $count++) {
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

  # Detect launch within a job on a Cray XC system. These systems
  # can run MOAB, PBS (only with MOAB?), or SLURM
  if (exists $ENV{CRAYPE_VERSION} && 
	( exists $ENV{MOAB_JOBNAME} || 
	  exists $ENV{PBS_VERSION} ||   
	  exists $ENV{SLURM_JOB_ID} )) {
    $using_aprun = 1;
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

# GOALS:
# Be able to extract all test properties for use in configuring CTest
# Be able to get one test's properties when running a single test or test file
# Be able to apply them to a given test by number

# Read input file and parse any contiguous header lines beginning with #@
# Create hash from test selections to key/value pairs
# #@[whitespace]<test-selection>[whitespace]:[whitespace]kw1=val1 kw2=val2
#
# TODO: unroll test selection if contains comma
sub parse_test_options {

  my $file = shift(@_);
  my $max_serial = -1;
  my $max_parallel = -1;
  open (INPUT_FILE, "$file") ||
      die "cannot open Dakota input file $file\n$!";

  while (my $line = <INPUT_FILE>) {
    # Parse test annotations
    if ($line =~ /^#@/) {

      # match a single test selection (could combine into two expressions)
      # double \\ since expanding the string in the regex below
      # support *, s*, p*, 3, s3, p4; not combining regexs to show priority
      my $test_select_re = "\\*|[sp]\\*|[0-9]+|[sp][0-9]+";
      # /^#@\s*(${test_select_re})\s*:\s*(.+)/

      # skip taxonomy entries which look like [ cat:subcat ]
      my $taxonomy_metadata =  "\\[\\s*[\\w:]+\\s*\\]";
      if ($line =~ 
          /^#@\s*(${test_select_re}\s*:)?\s*(${taxonomy_metadata})\s*$/ ) {
        print "Skipping taxonomy line: $line" if ${DTP_DEBUG};
        next;
      }

      # now match selection: key/value pairs; re-match leading comment
      # to be safe (differentiating from comments)
      if ($line =~ /^#@\s*(${test_select_re})\s*:\s*(.+)/) {
	my $test_selection = $1;
	my $test_options = $2;
	#$test_opts{${test_selection}} = { parse_key_val(${test_options})};
	# merge key/values with any existing in the options hash
	my %hash_tmp = parse_key_val(${test_options});
	while (($key, $value) = each %hash_tmp) {
	  # if key already appeared, append with comma
	  if (exists $test_opts{${test_selection}}{$key}) {
	    $test_opts{${test_selection}}{$key} = 
		"$test_opts{${test_selection}}{$key},$value";
	  }
	  else {
	    $test_opts{${test_selection}}{$key} = $value;
	  }
	}
      }

    }
    # Count tests in this file
    # TODO: store list of all tests and verify contiguous
    while ($line =~ /#[sp]\d+/g) {
      my $match = $&;
      # iterate the matches
      if ($match =~ /#p(\d+)/) {
	if ($1 > ${max_parallel}) {
	  $max_parallel = $1;
	}
      }
      elsif ($match =~ /#s(\d+)/){
	if ($1 > ${max_serial}) {
	  $max_serial = $1;
	}
      }
    }
  }  # while INPUT_FILE

  close (INPUT_FILE);

  # if no explicit parallel test, and no serial test, assume a default
  # serial test
  $max_serial = 0 if ($max_parallel == -1 && $max_serial == -1);

  # save total serial and parallel count into the properties
  $test_opts{"s*"}{"Count"} = $max_serial;
  $test_opts{"p*"}{"Count"} = $max_parallel;
  return ($max_serial, $max_parallel);
}


sub parse_key_val {

  # Regular expressions for key/value pairs
  my $check_output_re = "(CheckOutput)='(.*)'";
  my $dak_config_re = "(DakotaConfig)=(.*)";                # multi-valued
  my $depends_on_re = "(DependsOn)=([sp][0-9]+)";
  my $exec_args_re = "(ExecArgs)='(.*)'";
  my $exec_cmd_re = "(ExecCmd)='(.*)'";
  my $input_file_re = "(InputFile)='(.*)'";
  my $label_re = "(Label)s?=(.*)";                          # multi-valued
  my $mpi_procs_re = "(MPIProcs)=([0-9]+)";
  my $req_files_re = "(ReqFiles)=(.*)";
  my $restart_re = "(Restart)=(read|write|none)";
  my $timeout_absolute_re = "(TimeoutAbsolute)=([0-9]+)";
  my $timeout_delay_re = "(TimeoutDelay)=([0-9]+)";
  my $user_man_re = "(UserMan)=(\\w+)";
  my $will_fail_re = "(WillFail)=(true)";

  # TODO: cleanup workdir option
  ##RemoveFilesBefore, RemoveFilesAfter
      

  @all_keyval_re = (
    "${check_output_re}",
    "${dak_config_re}",
    "${depends_on_re}",
    "${exec_args_re}",
    "${exec_cmd_re}",
    "${input_file_re}",
    "${label_re}",
    "${mpi_procs_re}",
    "${req_files_re}",
    "${restart_re}",
    "${timeout_absolute_re}",
    "${timeout_delay_re}",
    "${user_man_re}",
    "${will_fail_re}"
  );

  # TODO: allow append of multiple of same option split across line
  # TODO: more efficient data structure than double loop

  # this function receives a space-separated list of key/val pairs
  # tokenize on space, respecting quotes for filenames, arguments
  my $line = shift(@_);
  # trim leading and trailing whitespace
  $line =~ s/^\s+|\s+$//g;
  use Text::ParseWords;  # See Also Regexp::Common::balanced, delimited
  @keyval_list = parse_line(" ", 1, $line);  

  my %keyval_hash = ();
  foreach my $kv (@keyval_list) {
    my $matched = 0;
    foreach my $kv_re (@all_keyval_re) {
      if ( $kv =~ /$kv_re/) {
	$keyval_hash{$1} = $2;
	$matched = 1;
      }
    }
    die "\nError: Invalid option '$kv' in line:\n  ${line}\n" if (!$matched);
  }

  return %keyval_hash;
}


# return bool as to whether we skip this test based on configuration
# uses global @dakota_config
sub check_required_configs() {
  my ($cnt) = @_;
  my $enable_test = 1;  # whether to enable this test based on config

  if (@dakota_config) {
  
    # get a comma-separated list of required configs
    my $req_configs = get_test_option_value($cnt, "DakotaConfig", "");
    
    # for now these can't be multivalued...
    my @rc_list = split(',', $req_configs);
    foreach my $rc (@rc_list) {
      if (! grep {$_ eq $rc} @dakota_config) {
	# Dakota does not have the required configuration
	print "Skipping test due to ${rc}\n";
	$enable_test = 0;
      }
    }
  }

  return $enable_test;
}


# Check if any of the current test labels match a user-provided
# regular expression.  For now this is only checking on a per-file
# basis (*, s*, p*) to mimic CTest behavior
sub check_labels() {

  my $enable_test = 1;  # whether to enable this test (default yes if no regex)

  if (${label_regex}) {
    $enable_test = 0;  # only enable this test if a label is matched

    # get a comma-separated list of assigned labels (-1 so * and s*/p* only)
    my $quiet = 1;
    my $test_labels = get_test_option_value(-1, "Label", "", $quiet);
   
    my @tl_list = split(',', $test_labels);
    foreach my $tl (@tl_list) {
      if (${tl} =~ /${label_regex}/) {
	#print "Including test since ${tl} matched ${label_regex}\n";
	$enable_test = 1;
	last;
      }
    }
  }
  
  return $enable_test;
}


# find a final value for a test option, going from general * to
# specific [ps]<int>; uses global parallelism
sub get_test_option_value() {

  my ($cnt, $key, $default, $quiet) = @_;
  

  my $value = $default;    
  my $ser_par = ($parallelism eq "parallel") ? "p" : "s"; 
  # Successively overwrite if the options is found
  # TODO: append if the option can have multiple values (HAVE_*, Label)
  foreach my $test_select ("*", "${ser_par}*", "${cnt}", "${ser_par}${cnt}") {
    if (exists $test_opts{${test_select}}{$key}) {
      $value = "$test_opts{${test_select}}{$key}";
    }
  }
  if ( !$quiet && $value ne $default) {
    print "Using test option ${key} = ${value}\n";
  }
  return $value;
}


# Print per-input file test options to file PROPERTIES_OUT in format 
#   dakota_input_file:serial|parallel: key=value
# (one line per key/value entry).  For now, aggregate test options
# across all serial (parallel) tests in the file yield
#
# TODO: print number of parallel/serial tests found in file
# TODO: detect duplicates/conflicts
# TODO: write on per-test basis
sub write_test_options {
  
  my ($input_file) = @_;
  substr($input_file,-3, 3) = "";  # trim .in
 
  # TODO: print all?  Need another hash with all possible properties
  # to regex matches
  foreach my $property 
      ("ReqFiles", "DakotaConfig", "Label", "Count") {
    my $prop_ser = "";
    my $prop_par = "";
    my %uniq_serial;    # properties that apply to serial tests
    my %uniq_parallel;  # properties that apply to parallel tests
    # iterate all test selection types
    while ( my ($test_select, $opts) = each %test_opts) {
      if (exists $test_opts{$test_select}{"$property"}){
	my $prop_vals = $test_opts{$test_select}{"$property"};
	# *, s*, s[0-9]+, [0-9]+
	if ($test_select =~ /^s?[\*0-9]+$/) {
	  $uniq_serial{$prop_vals} = 1;
	}
	# *, p*, p[0-9]+, [0-9]+
	if ($test_select =~ /^p?[\*0-9]+$/) {
	  $uniq_parallel{$prop_vals} = 1;
	}
      }
    }
    # using comma delimiter to help with cmake interpretation
    foreach my $prop_vals ( keys %uniq_serial ) {
      $prop_ser = $prop_ser ? "$prop_ser,$prop_vals" : "$property=$prop_vals";
    }
    print(PROPERTIES_OUT "${input_file}:serial: ${prop_ser}\n") if $prop_ser;

    foreach my $prop_vals ( keys %uniq_parallel ) {
      # using comma to help with cmake interpretation
      $prop_par = $prop_par ? "$prop_par,$prop_vals" : "$property=$prop_vals";
    }
    print(PROPERTIES_OUT "${input_file}:parallel: ${prop_par}\n") if $prop_par;
  }
  
  # match s[0-9]+: UserMan=extracted_file
  my $property = "UserMan";    
  # iterate all test selection types
  while ( my ($test_select, $opts) = each %test_opts) {
    if (exists $test_opts{$test_select}{"$property"}){
	my $prop_vals = $test_opts{$test_select}{"$property"};
	# Only support s[0-9]+ (serial) for user manual
	if ($test_select =~ /^s([\*0-9]+)$/) {
	  # prop_vals should be the output file name
	  # write extracted_file source_file test_num
	  print (USEREXAMPLES_OUT "${prop_vals} ${input_file} ${1}\n");
	}
    }
  }

}


# Determine how Dakota is configured (CMake options, OS) from
# Makefile.export in either build or install tree
sub check_dakota_config {

  my $makefile_export = "";
  if (-f "../src/Makefile.export.Dakota") {
    $makefile_export = "../src/Makefile.export.Dakota";
  }
  elsif (-f "../include/Makefile.export.Dakota") {
    $makefile_export = "../include/Makefile.export.Dakota";
  }
  return if (! ${makefile_export});
  
  open( my $fh, '<', $makefile_export );
  if (! $fh) {
    print "Warning: couldn't open ${makefile_export}.\n";
    return;
  }
  my $all_defines="";
  while ( my $line = <$fh> ) {
    if ( $line =~ /Dakota_DEFINES=(.+)/ ) {
      $all_defines = $1;
      last;
    }
  }
  close $fh;
  my @dakota_defs = split(" ", $all_defines);
  foreach my $dd (@dakota_defs) {
    # Remove leading -D and any whitespace
    $dd =~ s/\s*-D(.*)\s*/$1/g;
  }
  # Add CMake equivalents for operating system
  push(@dakota_defs, "WIN32") if ($Config{osname} =~ /MSWin/);
  push(@dakota_defs, "UNIX") if (!($Config{osname} =~ /MSWin/));

  return @dakota_defs;
}


# Compute full restart option string from passed restart option and filename
sub parse_restart_command() {
  my ($restart_option, $restart_file) = @_;
  # this is also the default for /write/
  my $restart_command = "-write_restart $restart_file";
  if ($restart_option eq "read") {
    # always write a named file if we're reading one
    $restart_command = 
	"-read_restart $restart_file -write_restart $restart_file";
    print "Restart file: reading and writing $restart_file\n";
  }
  elsif ($restart_option eq "none") {
    $restart_command = "";
    print "Restart file: explicitly removing restart arguments\n";
  }	
  return $restart_command;
}



# -----------------
# EXECUTION Helpers
# -----------------

# Assemble the test command based on active options;
# relies on global $parallelism 
sub form_test_command {

  my ($cnt, $num_proc, $dakota_command, $dakota_args, $restart_command,
      $dakota_input, $output, $error) = @_;
  
  my $fulldakota = "${bin_dir}${dakota_command}${bin_ext} ${dakota_args} $restart_command $dakota_input";
  my $redir = "> $output 2> $error";     


  # default serial command
  my $test_command = "$fulldakota $redir";
  # prepend a valgrind command
  # TODO: $cnt
  if ($run_valgrind) {
    my $vg_file = basename(${dakota_input}, ".in_") . ".${cnt}.vg";
    # Default output is for XML to integrate with build system
    # Default is to check for memory errors and leaks, but not track origins
    # Use DAKOTA_TEST_VALGRIND_EXTRA_OPTS to override any of these settings
    # These are one per line to facilitate comment/uncomment:
    my $vg_cmd = "valgrind --tool=memcheck "
	. "--log-file=${vg_file}.log "
	. "--xml=yes --xml-file=${vg_file}.xml "
	. "--child-silent-after-fork=yes "
	. "--leak-check=full "
##	. "--track-origins=yes "
	. "${vg_extra_args}";
    $test_command = "${vg_cmd} ${test_command}";
  }

  # If testing within a Cray XC job, aprun the test and force Dakota into serial mode
  if( $using_aprun && $parallelism eq "serial") {
    $test_command = "aprun -e DAKOTA_RUN_PARALLEL=F -n 1 $test_command";
  }
 

  if ($parallelism eq "parallel") {
    my ($sysname, $nodename, $release, $version, $machine) = POSIX::uname();
    # parallel test
    if ( $sysname =~ /AIX/) {
      # TODO: perform poekill after specified time
      $test_command = "poe $fulldakota -procs $num_proc $redir";
    }
    elsif ($sysname =~ /SunOS/) {
      $test_command = "mprun -np $num_proc $fulldakota $redir";
    }
    elsif($using_aprun == 1) {
      $test_command = "aprun -n $num_proc $fulldakota $redir";
    } 
    else
    { 
      # default for Linux
      $test_command = 
        "mpirun -np $num_proc $fulldakota $redir";
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

    # Make sure CTRL-C kills the child process group 
    local $SIG{INT} = sub { kill -9, $pid;
			    $exitcode = 103 << 8;
			    print ("aborted\n");
			    die "User CTRL-C abort";
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
    #kill -9, $pid;
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
      # We used to call setpgrp to avoid zombies with MPICH mpirun,
      # but opposite seems to be happening with more recent versions.
      #if ( $Config{osname} !~ /MSWin/ ) {
      #  setpgrp(0,0); # This sets process group so I can kill this + children
      #}
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
      while (/^\s+($e|$i|$s)/) {
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
    if (/^<<<<< Equivalent number of high fidelity evaluations/) {
      print;
      print TEST_OUT;
    }
    
    if (/(Mean =|Approximate Mean Response|Approximate Standard Deviation of Response|Importance Factor for|Si =)/) {
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
    
    if (/(Moment|Sample moment) statistics for each (response function|posterior variable):/) {
      print;
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line (Mean/StdDev/Skew/Kurt header)
      print;
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line (secondary tag header or table data)
      if (/^\s*\w+$/) { # 2 sets of moments (e.g. PCE/SC w/ exp _and_ numerical)
        while (/^\s*\w+$/) {
          $_ = <OUTPUT>; # grab next line (table data)
          while (/\s+$e/) {
    	  print;
    	  print TEST_OUT;
    	  $_ = <OUTPUT>; # grab next line
          }
        }
      }
      else { # 1 set of moments (e.g. PCE/SC w/ expansion _or_ numerical)
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

    while (/^\w+ Sobol' indices:/) {
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

    # BMA: Bayesian methods might have just "Response Level  Probability Level", 
    # so Reliability Index  General Rel Index is optional
    while (/^(\s+(Response Level|Resp Level Set)\s+Probability Level(\s+Reliability Index\s+General Rel Index)?|\s+Response Level\s+Belief (Prob Level|Gen Rel Lev)\s+Plaus (Prob Level|Gen Rel Lev)|\s+(Probability|General Rel) Level\s+Belief Resp Level\s+Plaus Resp Level|\s+Bin Lower\s+Bin Upper\s+Density Value|[ \w]+Correlation Matrix[ \w]+input[ \w]+output\w*:)$/) {
      print;
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line
      print;
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line
      while (/\s+($e|$naninf)/) {  # correlations may contain nan/inf
        print;
        print TEST_OUT;
        $_ = <OUTPUT>; # grab next line
      }
    }

    while (/^Surrogate quality metrics/) {
      print;
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line
      while (/^\s*${s}\s*($e|$naninf).*/) {  # may contain nan/inf
        print;
        print TEST_OUT;
        $_ = <OUTPUT>; # grab next line
      }
    }

    while (/^Wilks Statistics for/) {
      print;
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line
      $_ = <OUTPUT>; # grab next line
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line
      print TEST_OUT;
      $_ = <OUTPUT>; # grab next line
      print TEST_OUT;
      while (/\s+$e/) {
#      while (/\s*${s}\s*($e|$naninf)/) {  # may contain nan/inf
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

=item B<--label-regex=regular_expression>

only run test files with global label (*, p*, s*) matching given
regular_expression; this is applied only on a per-file basis, not to
skip individual tests

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

=item B<--save-output>

save test input, output, error, and restart of the last subtest run;
or set environment variable DAKOTA_TEST_SAVE_OUTPUT

=item B<--test-properties=props_dir>

write dakota_tests.props and dakota_usersexamples.props for specified
tests to specified directory; short circuits any other modes or
requests

=item B<--valgrind>

prepend Dakota command with valgrind executable and default options;
alternately set environment variable DAKOTA_TEST_VALGRIND.
DAKOTA_TEST_VALGRIND_EXTRA_ARGS will append args to valgrind,
overriding the defaults.

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

=head1 TEST FILE MARKUP

=over 4

=item B<Overview>

Test markups are placed in a header section of dakota_*.in files on
contiguous lines starting with #@ comments.  Each markup consists of
<test-selection>: <key/value pairs>.  For example, to specify options
for all parallel tests, with an override for a specific test:

  #@ Example comment about parallel markup
  #@ p*: MPIProcs=2 
  #@ p4: TimeoutAbsolute=1200 TimeoutDelay=60

A test selection may have multiple key/value pairs, but only one test
selection is allowed per line.

The markup header is terminated by any line not starting with #@.  #@
lines will always be omitted from extracted tests.

=item B<Test Selection>

Test properties are applied in the following order:

=over 2

=item 1. *  : all tests in this file

=item 2. s* : all serial (p*, parallel) tests in this file

=item 3. 4  : specific test (both serial and parallel versions)

=item 4. s7 : specific serial (p7, parallel) test

=back

=item B<Supported Properties>

See the regular expressions in dakota_test.perl for a complete list of
supported options.

=over 2

=item CheckOutput='<filename>'

diff Dakota output from <filename> instead of dakota_input.out; the
single quotes are required to allow spaces in <filename>

=item DakotaConfig=<dakota-define1>[,<dakota-define2>]

only enable the test(s) if Dakota was configured with <dakota-define1>,
e.g, HAVE_NPSOL;DAKOTA_HAVE_MPI; separate multiple with comma

=item DependsOn=<test-selection>

require the test(s) to run after test-dep, where <test-selection> is a
specific test such as s4 or p3

=item ExecArgs='<command-line-args>'

run the executable with <command-line-args>; the single quotes are
required to allow space-separated args

=item ExecCmd='<executable>'

run <executable> instead of dakota; the single quotes are required to
allow space-separated args

=item InputFile='<dakota-input>'

run with <dakota-input> instead of the default dakota_input.in; the
single quotes are required to allow space-separated args

=item Label=<label1>[,<label2>]

apply CTest labels, e.g., test categories: SmokeTest, AcceptanceTest;
capability categories: OptimizationTest, or maturity: Experimental

=item MPIProcs=<int-procs>

invoke mpirun with -np <int-procs>

=item Restart=(read|write|none)

run the test(s) reading from dakota_input.rst, writing to
dakota_input.rst, or explicitly with no restart

=item TimeoutAbsolute=<int-seconds>

terminate each test if total test time exceeds <int-seconds>

=item TimeoutDelay=<int-seconds>

terminate each test if output has not changed in <int-seconds>

=item UserMan=extracted_filename

the specified test (must be a single test selection) will be extracted
to extracted_filename for inclusion in Dakota User's Manual

=item WillFail=true

this test will fail, but report as a PASS

=back

=back

=cut
