#!/usr/bin/perl -w

# Get current year, if not given as argument
$year = (defined $ARGV[0]) ? $ARGV[0] : `date '+%Y'`;
chomp $year;

# CVS root
$cvsroot = "/cvsroot/plplot/plplot";

# Get list of CVS files
@files = ();
print STDERR "Getting CVS files list...";
open (STATUS, "cvs status 2>/dev/null|")
  or die "Cannot run cvs status.  Au revoir.";
while (<STATUS>) {
  push (@files, $1)
    if (m{Repository revision:[^/]+$cvsroot/(.*)(?=,v)});
}
print STDERR " done\n";

print STDERR "Getting cvs logs...\n";

for $file (@files) {
  my %names;
  open (LOG, "cvs log $file 2>/dev/null |")
    or die "Cannot run cvs log for $file.  Au revoir.";
  while (<LOG>) {
    $names{$1} = 1
      if (m{^date: $year/.*author: (.*)(?=;  state)});
  }
  my @authors = sort keys %names;
  print STDOUT "$file: " . join (", ", @authors) . "\n"
    if scalar @authors > 0;
}
