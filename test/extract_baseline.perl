#!/usr/bin/perl

# Extract the baseline for one test file from the specified baseline file
if (scalar(@ARGV) < 2 || (scalar(@ARGV) > 3) ) {
  die "Usage: extract_baseline.perl dakota_input_filename dakota_base_file " . 
      "baseline_fragment_output";
}

# Input processing
my $testin    = @ARGV[0]; # dakota input filename, e.g. dakota_dace.in
my $base_file = @ARGV[1]; # baseline file name, e.g. dakota_[p]base.test
my $out_file  = "${testin}.base";
if (scalar(@ARGV) == 3) { 
  $out_file  = @ARGV[2]; # reduced baseline file name, e.g. dakota_dace.base
}

open (my $DAKOTA_BASE, $base_file) || die "Error: Cannot open file $base_file" ;
$test_found = 0;
while ($line = <$DAKOTA_BASE>) {

  my $mod_testin = $testin;
  $mod_testin =~ s/\./\\\./g; # escape the "." in dakota_*.in

  if ($line =~ /$mod_testin/) {
    $test_found = 1;
    extract_test_baseline($DAKOTA_BASE, $out_file);
  }

  # stop processing 
  last if ($test_found == 1);

}
close ($DAKOTA_BASE);

if ($test_found == 0) {
  die "Error: Test ${testin} not found in baseline ${base_file}\n";
}


sub extract_test_baseline {
  
  my ($file_handle, $out_file) = @_;

  open (BASELINE_OUT, ">${out_file}") || 
      die "Error: can't write to ${out_file}\n";

  while (my $line = <$file_handle>) {

    # stop when we encounter the next test file
    last if ($line =~ /dakota_\w+\.in/);

    print BASELINE_OUT $line;
  }
  
  close (BASELINE_OUT);

}
