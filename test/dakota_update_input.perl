#!/usr/bin/perl
#######################################################################
# This perl script parses one or more Dakota input files and replaces
# outdated keywords with new ones.  The script supports different
# modes according to the command line arguments:
#
# -- multiple file mode: updates all dakota*.in files in CWD
#
#      dakota_update_input.perl
#
# -- single file mode: updates filename.in
#
#      dakota_update_input.perl filename.in
#
#######################################################################
#  Usage:	dakota_update_input.perl <filename>
#  Owner:	Mike Eldred
#  Date:	October 2005
#######################################################################

# retrieve command line arguments
if (@ARGV) {
  if (@ARGV[0] =~ /dakota_\w+\.in/) {
    @testin = @ARGV[0];
  }
  else {
    die "Usage: dakota_update_input.perl <filename>";
  }
}
else {
  @testin = <dakota_*.in>;
}

# update each input file
foreach $file (@testin) {

  # open original input file
  open (ORIG_INPUT, "$file") || die "cannot open original input file $file\n$!";
  print "Updating $file\n";

  # open modified input file
  $modinp = "$file.mod";
  open (MOD_INPUT, ">$modinp") ||
    die "cannot open modified input file $modinp\n$!";

  # read original input file until EOF
  while (<ORIG_INPUT>) {

    # perform keyword substitutions
    if (/((?:cd|dd|nu|lnu|uu|luu|tu|eu|bu|gau|guu|fu|wu|hu|iu|cs|ds)v_)(\w+)/) {
      print "Removing $1 from $2\n";
      s/$1//g;
    }
    #elsif (/descriptor /) {
    #  print "Making descriptor plural\n";
    #  s/descriptor /descriptors/g;
    #}
    #elsif(/descriptor\s+/) {
    #  print "Making descriptor plural\n";
    #  s/descriptor/descriptors/g;
    #}
    elsif (/fd_step_size/) {
      print "Replacing fd_step_size with fd_grad_step_size\n";
      s/fd_step_size/fd_grad_step_size/g;
    }
    elsif (/application/) {
      print "Removing application\n";
      s/application//g;
    }
    elsif (/no_taylor/) {
      print "Replacing no_taylor with no_approx\n";
      s/no_taylor/no_approx/g;
    }
    elsif (/layered/) {
      print "Replacing layered with surrogate\n";
      s/layered/surrogate/g;
      # TO DO: consolidate approx interface/layered model -> surrogate model
    }
    elsif (/multi_level/) {
      print "Replacing multi_level with hybrid\n";
      s/multi_level/hybrid/g;
    }
    elsif (/adaptive_hybrid/) {
      print "Replacing adaptive_hybrid with adaptive\n";
      s/adaptive_hybrid/adaptive/g;
    }
    elsif (/opt_under_uncertainty/) {
      print "Replacing opt_under_uncertainty with single_method\n";
      s/opt_under_uncertainty/single_method/g;
      while ($_ !~ /opt_method_pointer/) {
	print MOD_INPUT;
	$_ = <ORIG_INPUT>; # grab next line
      }
      print "Replacing opt_method_pointer with method_pointer\n";
      s/opt_method_pointer/method_pointer/g;
    }
    elsif (/surrogate_based_opt|surrogate_update/) {
      # This logic assumes that no strategy independent controls follow
      # opt_method_pointer, but strategy dependent controls may precede it.
      print "Replacing SBO/SU strategy spec with single_method\n";
      # replace strategy name
      if (s/surrogate_based_opt/single_method/g)
	{ $sbo = 1; }
      elsif (s/surrogate_update/single_method/g)
	{ $sbo = 2; }
      print MOD_INPUT;
      $_ = <ORIG_INPUT>;
      # process independent/dependent controls prior to opt_method_pointer
      splice @strat2iter, 0;
      while ($_ !~ /opt_method_pointer/) {
	if (/graphics|iterator_/) # print as part of strategy spec
	  { print MOD_INPUT; }
	else # store for later printing as part of SBLO method spec
	  { push @strat2iter, $_; }
	$_ = <ORIG_INPUT>;
      }
      # store strategy::opt_method_pointer location and text for restoration
      $omp_pos  = tell;
      $omp_line = $_;
      # extract approx method id
      ($approx_method) = $_ =~ /opt_method_pointer\s*=\s*'(\w+)'/;
      # have strategy point to new SBLO/SBGO method spec
      if ($sbo == 1) {
	print "Augmenting with new SBLO method spec\n";
	print MOD_INPUT "\tmethod_pointer = 'SBLO'\n\nmethod,\n";
	print MOD_INPUT "\tid_method = 'SBLO'\n\tsurrogate_based_local\n";
      }
      elsif ($sbo == 2) {
	print "Augmenting with new SBGO method spec\n";
	print MOD_INPUT "\tmethod_pointer = 'SBGO'\n\nmethod,\n";
	print MOD_INPUT "\tid_method = 'SBGO'\n\tsurrogate_based_global\n";
      }
      # scan ahead for model pointer spec within approx method
      # and print it out within the new SBLO method spec
      while ($_ !~ /id_method\s*=\s*'$approx_method'/)
	{ $_ = <ORIG_INPUT>; }
      while ($_ !~ /model_pointer\s*=\s*/)
	{ $_ = <ORIG_INPUT>; }
      # print this model_pointer as part of new SBLO method spec
      print "Moving model_pointer spec to new local/global SBO method spec\n";
      print MOD_INPUT;
      # reset file pointer and input
      seek ORIG_INPUT, $omp_pos, 0;
      $_ = $omp_line;
      # update syntax from strategy->method ptr to method->sub-method ptr
      s/opt_method_pointer/approx_method_pointer/g;
      print MOD_INPUT;
      # now dump stored SBLO-dependent controls stored earlier
      print MOD_INPUT @strat2iter;
      $_ = <ORIG_INPUT>;
      # remove redundant model_pointer
      while ($_ !~ /id_method\s*=\s*'$approx_method'/)
	{ print MOD_INPUT; $_ = <ORIG_INPUT>; }
      while ($_ !~ /model_pointer\s*=\s*/)
	{ print MOD_INPUT; $_ = <ORIG_INPUT>; }
      $_ = <ORIG_INPUT>; # skip found line
    }
    elsif (/_region_threshold/) { # expand and contract thresholds
      print "Replacing _region_threshold with _threshold\n";
      s/_region_threshold/_threshold/g;
    }
    elsif (/_dist_\w+_bound/) { # uncertain variable distribution bounds
      print "Replacing _dist_(lower,upper)_bound with _(lower,upper)_bound\n";
      s/_dist_lower_bound/_lower_bound/g;
      s/_dist_upper_bound/_upper_bound/g;
    }
    elsif (/nond_reliability/) { # changed method name
      print "Replacing nond_reliability with nond_local_reliability\n";
      s/nond_reliability/nond_local_reliability/g;
    }
    #elsif (//) {
    #  print "Replacing  with ";
    #  s///;
    #}
    print MOD_INPUT;
  }

  # cleanup
  close (ORIG_INPUT);
  close (MOD_INPUT);
  rename $modinp, $file;
}
print "Updates complete\n";
