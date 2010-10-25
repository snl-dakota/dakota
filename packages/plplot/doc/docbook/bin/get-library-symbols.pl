#! /usr/bin/env perl
 
# Get symbols exported by the PLplot library and output only those that are
# included in the public header file.
#
# Rafael Laboissiere <rafael@users.sf.net> on Sun May 23 16:24:02 CEST 2004
# This file is part of PLplot and release under the terms of the GNU GPL.
# No warranties.

my $nargin = scalar @ARGV;

die "Usage:\n  $0 library.so header.h [n_columns]\n"
  if ($nargin != 2 and $nargin != 3);

my ($library, $header, $columns) = @ARGV;
$columns ||= 2;

die "$0: n_columns must be >= 1"
  if $columns < 1;

open (HEADER, "< $header")
  or die "$0: cannot open file $header";
my @hlines = <HEADER>;
close HEADER;

my @extsym = `nm $library`
  or die "$0: cannot get external symbols of library $library";

my @symbols = ();

my $maxlen = 0;

map {    
  my $s = $_;
  if (grep {/$s/} @hlines) {
    push (@symbols, $s);
    my $len = length ($s);
    $maxlen = $len 
      if $len > $maxlen;
  }
} sort map {
  chomp;
  my $s = (split (" "))[2];
  $s =~ s/^c_//;
  $s;
} grep {/ T /} @extsym;

if ($columns == 1) {

  # Simple output if one column requested
  print join ("\n", @symbols);

} else {
    
  my $nsym = scalar @symbols;
  my $percol = $nsym / $columns;
  $percol = int ($percol) + 1
    if $percol != int ($percol);
  for (my $i = 0; $i < $percol; $i++) {
    for (my $j = 0; $j < $columns; $j++) {
      my $k = $j * $percol + $i;
      if ($k < $nsym) {
        my $sym = $symbols[$k];
        print $sym;
	print (" " x ($maxlen - length ($sym) + 2))
	  if $j < $columns - 1;
      }
    }
    print "\n";
  }
}
