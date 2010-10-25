#! /usr/bin/env perl
 
# Get symbols exported by the PLplot library and output only those that are
# included in the public header file.
#
# Rafael Laboissiere <rafael@users.sf.net> on Sun May 23 16:24:02 CEST 2004
# This file is part of PLplot and release under the terms of the GNU GPL.
# No warranties.

die "Usage:\n  $0 header.h\n"
  if scalar @ARGV != 1;

my ($header) = @ARGV;

open (HEADER, "< $header")
  or die "$0: cannot open file $header";

my $in_struct = 0;
my $buf = "";
  
while (<HEADER>) {
  $in_struct = 1
    if /^typedef struct {/;
  if ($in_struct) {
    $buf .= $_;
    if (/^}/) {
      print "$buf\n";
      $in_struct = 0;
      $buf = "";
    }
  }
}
