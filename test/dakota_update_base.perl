#!/usr/bin/perl
#######################################################################
# This perl script parses baselines and replaces outdated formats
# with new ones.  The script supports different modes according to
# the command line arguments:
#
# -- multiple file mode: updates all dakota_*.test files in CWD
#
#      dakota_update_base.perl
#
# -- single file mode: updates filename.test
#
#      dakota_update_base.perl filename.test
#
#######################################################################
#  Usage:	dakota_update_base.perl <filename>
#  Owner:	Mike Eldred
#  Date:	January 2008
#######################################################################

# retrieve command line arguments
if (@ARGV) {
  if (@ARGV[0] =~ /dakota_\w+\.test/) {
    @testin = @ARGV[0];
  }
  else {
    die "Usage: dakota_update_base.perl <dakota_*.test>";
  }
}
else {
  @testin = <dakota_*.test>;
}

$e = "-?\\d\\.\\d+e(?:\\+|-)\\d+"; # exponential
$i = "-?\\d+";                     # integer
$ui = "\\d+";                      # unsigned integer

# update each base file
foreach $file (@testin) {

  # open original base file
  open (ORIG_BASE, "$file") || die "cannot open original base file $file\n$!";
  print "Updating $file\n";

  # open modified base file
  $modbase = "$file.mod";
  open (MOD_BASE, ">$modbase") ||
    die "cannot open modified base file $modbase\n$!";

  # read original base file until EOF
  while (<ORIG_BASE>) {

    # perform keyword substitutions
    if ( /^SBO Iterations = $ui/ )
      { print "Removing SBO Iterations\n"; }
    elsif ( ($tag) = /^  (\w+) evaluations:/ ) {
      print "Replacing SBO evaluation summary for $tag\n";
      s/^  $tag evaluations:/<<<<< Function evaluation summary ($tag):/g;
      print MOD_BASE;
    }
    elsif ( /^SBO Final Design Variables/ ) {
      print "Replacing SBO Final Design Variables\n";
      s/^SBO Final Design Variables/<<<<< Best parameters          =/g;
      print MOD_BASE;
      $_ = <ORIG_BASE>; # grab next line
      while ( ($tag, $value) = /^  (\w+)\s+= (\s?$e)/ ) {
	s/^  \w+\s+= \s?$e/                     $value $tag/g;
	print MOD_BASE;
	$_ = <ORIG_BASE>; # grab next line
      }
      print "Replacing SBO Final Truth Response Values\n";
      s/^SBO Final Truth Response Values/<<<<< Best objective function  =/g;
      print MOD_BASE;
      $_ = <ORIG_BASE>; # grab next line
      ($value) = /^  Objective Function\s+= (\s?$e)/;
      s/^  Objective Function   = \s?$e/                     $value/g;
      print MOD_BASE;
      $_ = <ORIG_BASE>; # grab next line
      if (/^  Ineq Constraint $ui\s+=/)
	{ print MOD_BASE "<<<<< Best constraint values   =\n"; }
      while ( ($value) = /^  Ineq Constraint $ui\s+= (\s?$e)/ ) {
	s/^  Ineq Constraint $ui\s+= \s?$e/                     $value/g;
	print MOD_BASE;
	$_ = <ORIG_BASE>; # grab next line
      }
      print MOD_BASE;
    }
    else
      { print MOD_BASE; }
  }

  # cleanup
  close (ORIG_BASE);
  close (MOD_BASE);
  rename $modbase, $file;
}
print "Updates complete\n";
