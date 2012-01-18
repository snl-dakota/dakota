#!/usr/bin/perl
#
#  A Perl script to diff the baseline dakota results file with a newly
#  generated dakota results file.  The script searches for specific
#  strings within the results file so that the diff can be done on the
#  correct values. For example the keyword Objective is found, then
#  the value associated with the Objective Function is differenced.
#  Uses the compare function to compare numerical values within a
#  specified epsilon.
#
#########################################################################
#
# Usage:
#   dakota_diff dakota_input_filename dakota_base_file dakota_test_output
#########################################################################
#read in the arguments
$testin = @ARGV[0]; # dakota input filename,          e.g. dakota_dace.in
$file1  = @ARGV[1]; # baseline file name,             e.g. dakota_base.test
$file2  = @ARGV[2]; # reduced test results file name, e.g. dakota_dace.tst

open (DAKOTA_BASE, $file1) || die "cannot open file $!" ;
open (DAKOTA_TEST, $file2) || die "cannot open file $!" ;

# exit code tracking the worst test result status (see dakota_test.perl)
my $exitcode = 0;

# numerical field in exponential notation
$expo = "-?\\d\\.\\d+e(?:\\+|-)\\d+"; 
# invalid numerical field
$naninf = "-?([Nn][Aa][Nn]|[Ii][Nn][Ff])";
# numerical field printed as exponential (may contain NaN/Inf)
$e = "($expo|$naninf)";
# numerical field in integer notation
$i = "-?\\d+";                     
$test_num = 0;
$test_found = 0;
$test_diff = 0;
$test_fail = 0;

$mod_testin = $testin;
$mod_testin =~ s/\./\\\./g; # escape the "." in dakota_*.in
#print "$mod_testin\n";

while ( $test_found == 0 && defined ($base = <DAKOTA_BASE>) ) {

  # baseline test results are delimited by the input file name, so advance the
  # baseline to the point matching the test filename.  If the baseline lacks
  # a particular test, no report will be generated for that test.

  if ($base =~ /$mod_testin/) {
    $test_found = 1;
    print "$testin\n";
    while (defined($test = <DAKOTA_TEST>)) {

      # increment baseline in direct synchronization with test file
      $base = <DAKOTA_BASE>;

      ####################
      # Vector extractions
      ####################

      # General
      while ( ($base =~ /^<<<<< Best [ \w\(\)]+=$/) &&
              ($test =~ /^<<<<< Best [ \w\(\)]+=$/) ) {
        if ($base != $test) {
          print "Error: mismatch in data header between baseline and test\n";
          exit(-1);
        }
	$b_hdr = $base; # save header in case of diffs
	$t_hdr = $test; # save header in case of diffs
        $first_diff = 0;
        $base = <DAKOTA_BASE>; # grab next line
        $test = <DAKOTA_TEST>; # grab next line
        while ( ( ($t_val) = $test =~ /^\s+($e|$i)/ ) &&
                ( ($b_val) = $base =~ /^\s+($e|$i)/ ) ) {
          if (diff($t_val, $b_val)) {
            $test_diff = 1;
	    if ($first_diff == 0) {
              $first_diff = 1;
              push @base_diffs, $b_hdr;
              push @test_diffs, $t_hdr;
	    }
            push @base_diffs, $base;
            push @test_diffs, $test;
          }
          $base = <DAKOTA_BASE>; # grab next line
          $test = <DAKOTA_TEST>; # grab next line
        }
      }

      # SBO
      #if ( ($base =~ /^SBO Final Design Variables$/) &&
      #     ($test =~ /^SBO Final Design Variables$/) ) {
      #	 $b_hdr = $base; # save header in case of diffs
      #	 $t_hdr = $test; # save header in case of diffs
      #  $first_diff = 0;
      #  $base = <DAKOTA_BASE>; # grab next line
      #  $test = <DAKOTA_TEST>; # grab next line
      #  while ( ( ($t_val) = $test =~ /^\s+\w+\s+=\s+($e)$/ ) &&
      #          ( ($b_val) = $base =~ /^\s+\w+\s+=\s+($e)$/ ) ) {
      #    if (diff($t_val, $b_val)) {
      #      $test_diff = 1;
      #	     if ($first_diff == 0) {
      #        $first_diff = 1;
      #        push @base_diffs, $b_hdr;
      #        push @test_diffs, $t_hdr;
      #	     }
      #      push @base_diffs, $base;
      #      push @test_diffs, $test;
      #    }
      #    $base = <DAKOTA_BASE>; # grab next line
      #    $test = <DAKOTA_TEST>; # grab next line
      #  }
      #}

      # PCE
      if ( ($base =~ /^Polynomial Chaos coefficients for/) &&
           ($test =~ /^Polynomial Chaos coefficients for/) ) {
	$b_hdr = $base; # save header in case of diffs
	$t_hdr = $test; # save header in case of diffs
        $first_diff = 0;
        $base = <DAKOTA_BASE>; # grab next line
        $test = <DAKOTA_TEST>; # grab next line
	while ( ( ($t_val) = $test =~ /^\s+($e)/ ) &&
		( ($b_val) = $base =~ /^\s+($e)/ ) ) {
	  if (diff($t_val, $b_val)) {
	    $test_diff = 1;
	    if ($first_diff == 0) {
	      $first_diff = 1;
	      push @base_diffs, $b_hdr;
	      push @test_diffs, $t_hdr;
	    }
	    push @base_diffs, $base;
	    push @test_diffs, $test;
	  }
	  $base = <DAKOTA_BASE>; # grab next line
	  $test = <DAKOTA_TEST>; # grab next line
	}
      }

      # UQ mappings and indices
      while ( ($base =~ /^(\s+(Response Level|Resp Level Set)\s+Probability Level\s+Reliability Index\s+General Rel Index|\s+Response Level\s+Belief (Prob Level|Gen Rel Lev)\s+Plaus (Prob Level|Gen Rel Lev)|\s+(Probability|General Rel) Level\s+Belief Resp Level\s+Plaus Resp Level|[ \w]+Correlation Matrix[ \w]+input[ \w]+output\w*:|\w+ Sobol indices:|(Moment-based statistics|95% confidence intervals) for each response function:)$/o) &&
	      ($test =~ /^(\s+(Response Level|Resp Level Set)\s+Probability Level\s+Reliability Index\s+General Rel Index|\s+Response Level\s+Belief (Prob Level|Gen Rel Lev)\s+Plaus (Prob Level|Gen Rel Lev)|\s+(Probability|General Rel) Level\s+Belief Resp Level\s+Plaus Resp Level|[ \w]+Correlation Matrix[ \w]+input[ \w]+output\w*:|\w+ Sobol indices:|(Moment-based statistics|95% confidence intervals) for each response function:)$/o) ) {
	$b_hdr1 = $base;         # save headers in case of diffs
	$b_hdr2 = <DAKOTA_BASE>; # save headers in case of diffs
	$t_hdr1 = $test;         # save headers in case of diffs
	$t_hdr2 = <DAKOTA_TEST>; # save headers in case of diffs
        $first_diff = 0;
        $base = <DAKOTA_BASE>; # grab next line
        $test = <DAKOTA_TEST>; # grab next line
	while ( ( (@t_val) = $test =~ /\s+($e)/go ) &&
		( (@b_val) = $base =~ /\s+($e)/go ) ) {
	  if ($#t_val != $#b_val) {
	    print "Error: mismatch in number of entries in UQ results table between baseline and test\n";
	    exit(-1);
	  }
	  $row_diff = 0;
	  for ($count=0; $count<=$#t_val; $count++) {
	    if (diff($t_val[$count], $b_val[$count])) {
	      $test_diff = 1;
	      $row_diff = 1;
	    }
	  }
	  if ($row_diff == 1) {
	    if ($first_diff == 0) {
	      $first_diff = 1;
	      push @base_diffs, $b_hdr1;
	      push @base_diffs, $b_hdr2;
	      push @test_diffs, $t_hdr1;
	      push @test_diffs, $t_hdr2;
	    }
	    push @base_diffs, $base;
	    push @test_diffs, $test;
	  }
          $base = <DAKOTA_BASE>; # grab next line
          $test = <DAKOTA_TEST>; # grab next line
        }
      }

      # Test number header
      while ( ( ($t_test_num) = $test =~ /^Test Number (\d+)/ ) &&
              ( ($b_test_num) = $base =~ /^Test Number (\d+)/ ) ) {

	# the previous test is now complete, generate the summary for it.
        if ($t_test_num > 0) {
          if ($test_diff == 0 && $test_fail == 0) {
            print "PASS test $test_num\n";
          }
          elsif ($test_diff == 1) {
            print "DIFF test $test_num\n";
            $exitcode = 80 if $exitcode < 80;
            foreach (@base_diffs) {
              print "base< $_";
            }
	    print "---\n";
            foreach (@test_diffs) {
              print "test> $_";
            }
	    @base_diffs = ();
	    @test_diffs = ();
            $test_diff = 0;
          }
	  #else: FAIL header for this test has already been output
        }

	# verify consistent test_num in base and test
        if ($b_test_num != $t_test_num) {
          print "Error: mismatch in Test Number between baseline and test\n";
          exit(-1);
        }
        else {
          $test_num = $t_test_num;
        }
	# TODO: could test that the code and message are the same
	($test_code, $msg) = $test =~ /failed with exit code (\d+)([\(\) \w]*)$/;
	($base_code) = $base =~ /failed with exit code (\d+)$/;
 	if ($test_code) {
	  $test_fail = 1;
	  # determine if test failure is consistent in baseline
	  if ($base_code) {
	    print "FAIL test $test_num (consistent with baseline)\n";
            $exitcode = 90 if $exitcode < 90;
	    # increment to next test (required for while loop)
            $test = <DAKOTA_TEST>;
	    $base = <DAKOTA_BASE>;
	  }
	  else {
	    if (100 <= $test_code && $test_code <= 104) {
	      print "FAIL test $test_num$msg\n";
	      $exitcode = $test_code if $exitcode < $test_code;
	    }
	    else {
	      print "FAIL test $test_num (*** new ***)\n";
	      $exitcode = 105 if $exitcode < 105;
	    }
            # Offset problem.  Advance $test and $base to next test.
            $test = <DAKOTA_TEST>;
	    $base = <DAKOTA_BASE>;
	    while (defined($base) && $base !~ /^Test Number \d+/) {
              $base = <DAKOTA_BASE>;
            }
	  }
	}
	elsif ($base_code) {
	  $test_fail = 1;
	  print "PASS test $test_num (baseline failure: nothing to diff)\n";
          # Offset problem.  Advance $base and $test to next test.
          $base = <DAKOTA_BASE>;
	  $test = <DAKOTA_TEST>;
          while (defined($test) && $test !~ /^Test Number \d+/) {
            $test = <DAKOTA_TEST>;
          }
	}
	else {
	  #print "both tests succeeded\n";
	  $test_fail = 0;
	  # Now exit this while loop and compare the test results.
	  # NOTE: we don't want to increment to the next line (one way to break
	  # out of the while loop) since this could conflict with a Best data
	  # vector extraction that immediately followed (the advance at the
	  # bottom of the main loop would go one line too far).
	  last; # exit this while loop w/o incrementing the file pointers
	}
      }

      ###############################################################
      # These tests must follow the vector extractions since the file
      # pointer has been advanced at the end of the vector extraction
      ###############################################################

      # General
      if ( ( ($t_tev, $t_nev, $t_dev) = $test =~
        /^<<<<< Function evaluation summary[ \w\(\)]*: (\d+) total \((\d+) new, (\d+) duplicate\)$/ ) &&
	   ( ($b_tev, $b_nev, $b_dev) = $base =~
        /^<<<<< Function evaluation summary[ \w\(\)]*: (\d+) total \((\d+) new, (\d+) duplicate\)$/ ) ) {
        if ( diff($t_tev, $b_tev) || diff($t_nev, $b_nev) ||
             diff($t_dev, $b_dev) ) {
          $test_diff = 1;
          push @base_diffs, $base;
          push @test_diffs, $test;
        }
      }
      elsif ( ( ($t_vt, $t_vn, $t_vd, $t_gt, $t_gn, $t_gd, $t_ht, $t_hn, $t_hd)
	= $test =~ /^\s*\w+: (\d+) val \((\d+) n, (\d+) d\), (\d+) grad \((\d+) n, (\d+) d\), (\d+) Hess \((\d+) n, (\d+) d\)$/ ) &&
	      ( ($b_vt, $b_vn, $b_vd, $b_gt, $b_gn, $b_gd, $b_ht, $b_hn, $b_hd)
	= $base =~ /^\s*\w+: (\d+) val \((\d+) n, (\d+) d\), (\d+) grad \((\d+) n, (\d+) d\), (\d+) Hess \((\d+) n, (\d+) d\)$/ ) ) {
        if ( diff($t_vt, $b_vt) || diff($t_vn, $b_vn) || diff($t_vd, $b_vd) ||
	     diff($t_gt, $b_gt) || diff($t_gn, $b_gn) || diff($t_gd, $b_gd) ||
	     diff($t_ht, $b_ht) || diff($t_hn, $b_hn) || diff($t_hd, $b_hd) ) {
          $test_diff = 1;
          push @base_diffs, $base;
          push @test_diffs, $test;
        }
      }
      elsif ( ( ($t_bd) = $test =~
        /^<<<<< Best data captured at function evaluation (\d+)$/ ) &&
              ( ($b_bd) = $base =~
        /^<<<<< Best data captured at function evaluation (\d+)$/ ) ) {
        if ( diff($t_bd, $b_bd) ) {
          $test_diff = 1;
          push @base_diffs, $base;
          push @test_diffs, $test;
        }
      }
      elsif (
	$test =~ /^<<<<< Best data captured at function evaluation \d+$/ &&
        $base =~ /^<<<<< Best data not found in evaluation cache$/ ) {
	$test_diff = 1;
	push @base_diffs, $base;
	push @test_diffs, $test;
      }
      elsif (
	$test =~ /^<<<<< Best data not found in evaluation cache$/ &&
        $base =~ /^<<<<< Best data captured at function evaluation \d+$/ ) {
	$test_diff = 1;
	push @base_diffs, $base;
	push @test_diffs, $test;
      }

      # MV
      elsif ( ( ($t_val) = $test =~
        /^\s+(?:Approximate Mean Response|Approximate Standard Deviation of Response|Importance Factor for variable \w+)\s+=\s+($e)$/ ) &&
	      ( ($b_val) = $base =~
        /^\s+(?:Approximate Mean Response|Approximate Standard Deviation of Response|Importance Factor for variable \w+)\s+=\s+($e)$/ ) ) {
        if ( diff($t_val, $b_val) ) {
          $test_diff = 1;
          push @base_diffs, $base;
          push @test_diffs, $test;
        }
      }

      # LHS/MC
      elsif ( ( ($t_mu, $t_sig, $t_cov) = $test =~
        /^\w+:\s+Mean =\s+($e)\s+Std. Dev. =\s+($e)\s+Coeff. of Variation =\s+($e)$/ ) &&
	      ( ($b_mu, $b_sig, $b_cov) = $base =~
        /^\w+:\s+Mean =\s+($e)\s+Std. Dev. =\s+($e)\s+Coeff. of Variation =\s+($e)$/ ) ) {
        if ( diff($t_mu, $b_mu) || diff($t_sig, $b_sig) || diff($t_cov, $b_cov) ) {
          $test_diff = 1;
          push @base_diffs, $base;
          push @test_diffs, $test;
        }
      }
      elsif ( ( ($t_mu_lb, $t_mu_ub, $t_sig_lb, $t_sig_ub)
		= $test =~ /^\w+:\s+Mean = \(\s+($e),\s+($e)\s+\), Std Dev = \(\s+($e),\s+($e)\s+\)$/ ) &&
	      ( ($b_mu_lb, $b_mu_ub, $b_sig_lb, $b_sig_ub)
		= $base =~ /^\w+:\s+Mean = \(\s+($e),\s+($e)\s+\), Std Dev = \(\s+($e),\s+($e)\s+\)$/ ) ) {
        if ( diff($t_mu_lb, $b_mu_lb) || diff($t_mu_ub, $b_mu_ub) || diff($t_sig_lb, $b_sig_lb) || diff($t_sig_ub, $b_sig_ub) ) {
          $test_diff = 1;
          push @base_diffs, $base;
          push @test_diffs, $test;
        }
      }
      elsif ( ( ($t_min, $t_max) = $test =~
		/^\w+:\s+Min =\s+($e)\s+Max =\s+($e)$/ ) &&
	      ( ($b_min, $b_max) = $base =~
		/^\w+:\s+Min =\s+($e)\s+Max =\s+($e)$/ ) ) {
        if ( diff($t_min, $b_min) || diff($t_max, $b_max) ) {
          $test_diff = 1;
          push @base_diffs, $base;
          push @test_diffs, $test;
        }
      }
      # Sampling-based VBD
      #elsif ( ( ($t_si, $t_ti)
      #		= $test =~ /^\s+\w+:\s+Si =\s+($e)\s+Ti =\s+($e)$/ ) &&
      #	      ( ($b_si, $b_ti)
      #		= $base =~ /^\s+\w+:\s+Si =\s+($e)\s+Ti =\s+($e)$/ ) ) {
      #  if ( diff($t_si, $b_si) || diff($t_ti, $b_ti) ) {
      #    $test_diff = 1;
      #    push @base_diffs, $base;
      #    push @test_diffs, $test;
      #  }
      #}
      # SBO
      #elsif ( ( ($t_iter) = $test =~ /^SBO Iterations =\s+(\d+)$/ ) &&
      #        ( ($b_iter) = $base =~ /^SBO Iterations =\s+(\d+)$/ ) ) {
      #  if ( diff($t_iter, $b_iter) ) {
      #    $test_diff = 1;
      #    push @base_diffs, $base;
      #    push @test_diffs, $test;
      #  }
      #}
      elsif ( ( ($t_tev, $t_nev, $t_dev) = $test =~
        /^  \w+ evaluations: (\d+) total \((\d+) new, (\d+) duplicate\)$/ )
	   && ( ($b_tev, $b_nev, $b_dev) = $base =~
        /^  \w+ evaluations: (\d+) total \((\d+) new, (\d+) duplicate\)$/ ) ) {
        if ( diff($t_tev, $b_tev) || diff($t_nev, $b_nev) ||
             diff($t_dev, $b_dev) ) {
          $test_diff = 1;
          push @base_diffs, $base;
          push @test_diffs, $test;
        }
      }
      elsif ( ( ($t_val) = $test =~
        /^  (?:Objective Function\s+\d*|Ineq Constraint\s+\d+|Eq Constraint\s+\d+)\s+=\s+($e)$/ ) &&
	      ( ($b_val) = $base =~
        /^  (?:Objective Function\s+\d*|Ineq Constraint\s+\d+|Eq Constraint\s+\d+)\s+=\s+($e)$/ ) ){
        if ( diff($t_val, $b_val) ) {
          $test_diff = 1;
          push @base_diffs, $base;
          push @test_diffs, $test;
        }
      }
    }

    # the final test is now complete, generate the summary for it.
    if ($test_diff == 0 && $test_fail == 0) {
      print "PASS test $test_num\n";
    }
    elsif ($test_diff == 1) {
      print "DIFF test $test_num\n";
      foreach (@base_diffs) {
        print "base< $_";
      }
      print "---\n";
      foreach (@test_diffs) {
        print "test> $_";
      }
      @base_diffs = ();
      @test_diffs = ();
      $test_diff = 0;
    }
    #else: FAIL header for this test has already been output
    print "\n"; # extra carriage return to separate input file reports
  }
}
close (DAKOTA_BASE);
close (DAKOTA_TEST);

exit $exitcode;


# subroutine diff assesses whether two numbers differ by more than an epsilon
sub diff {
  # $_[0] = test value
  # $_[1] = baseline value

  #print "Diffing $_[0] and $_[1]\n";

  # for nan or inf, require exact string match
  if ( $_[0] =~ /$naninf/ || $_[1] =~ /$naninf/ ) {
    if ( $_[0] ne $_[1] ) {
      return 1;
    }
    else {
      return 0;
    }
  }

  $SMALL       = 1.e-8; # use absolute difference for small values
  $ABS_EPSILON = 5.e-9; # allow up to a quarter of the total abs() interval
  $REL_EPSILON = 1.e-4; # allow up to 0.01% numerical differences
  if ( (abs($_[0]) < $SMALL) || (abs($_[1]) < $SMALL) ) {
    $differ = abs($_[0] - $_[1]);     # absolute difference
    if ($differ > $ABS_EPSILON) {
      return 1;
    }
  }
  else {
    $differ = abs($_[0]/$_[1] - 1.0); # relative difference
    if ($differ > $REL_EPSILON) {
      return 1;
    }
  }

  return 0;
}
