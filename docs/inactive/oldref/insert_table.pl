#!/usr/bin/perl

# Script to help insert a table into the reference manual
#
# Requires Perl List/MoreUtils, install perl-List-MoreUtils on RHEL6, e.g.
#
# The Ref_File.dox and specified chapter must correspond, e.g.,
#   insert_table.pl Ref_Method.dox 5 2
#   insert_table.pl Ref_Variables.dox 7 2
# because Method tables are Chapter 5 and Variables in 7.  
#
# This would increment by one all table numbers from 5.LAST down to 5.2
# (7.LAST down to 7.2) in all Ref_*.dox files.

# It would be easy to modify this to remove a table as well...

use File::Copy;
use List::MoreUtils qw(uniq);

if (@ARGV && @ARGV[0] =~ /Ref_\w+\.dox/ && 
    @ARGV[1] =~ /[\d]+/ && @ARGV[2] =~ /[\d]+/) {
  $ref_file = @ARGV[0];
  $chapter = @ARGV[1];
  $tablenum = @ARGV[2];
}
else {
  die "Usage to insert table before chapter.tablenum in ALL Ref_*.dox files:\n  insert_table.pl Ref_File.dox chapter table_num\n";
  usage();
}

# Read the reference file
open (REF_INPUT, "$ref_file") ||
    die "ERROR: Cannot open $ref_file\n$!";
my $holdTerminator = $/;
undef $/;
my $ref_contents = <REF_INPUT>;
$/ = $holdTerminator;
close(REF_INPUT);

# Find all occurences of T${chapter}d${table} and  T${chapter}.${table}
# They must be the same or a table is missing its anchor
while($ref_contents =~ m/Table ${chapter}\.([0-9]+)/g) {
  push @table_names, $1;
}
my @uniq_names = uniq sort {$a <=> $b} @table_names;

while($ref_contents =~ m/T${chapter}d([0-9]+)/g) {
  push @table_refs, $1;
}
my @uniq_refs = uniq sort {$a <=> $b} @table_refs;

if (@uniq_names == 0 || @uniq_refs == 0) {
  die "\nError: No valid tables found.  Perhaps specified file and chapter don't align?\n";
}

print "Numbers from names are: @uniq_names\n";
print "Numbers from refs are: @uniq_refs\n";
if (@uniq_names == @uniq_refs) {
  print "Info: table names and references match up.\n";
}
else {
  die "\nError: mismatch in table names and references!\n";
}

# names and refs are valid, sorted numbers of tables
# find the last table number
my $lastnum = @uniq_names[-1];

print "\nIncrementing numbers of Tables ${chapter}.${tablenum} through ${chapter}.${lastnum}\n  to make room for new Table ${chapter}.${tablenum}.\n";

# Open each Ref file, renumber from last to tablenum, write out
foreach $ref_file (<Ref_*.dox>) {

  # Read the reference file
  open (REF_INPUT, "$ref_file") ||
    die "\nERROR: Cannot open $ref_file\n$!";
  my $holdTerminator = $/;
  undef $/;
  my $dox_contents = <REF_INPUT>;
  $/ = $holdTerminator;
  close(REF_INPUT);

  # open original input file
  print "Updating ${ref_file} with backup ${ref_file}.bak\n";
  copy("${ref_file}", "${ref_file}.bak") or die "Copy to ${ref_file}.bak failed: $!";

  foreach $oldnum (reverse(${tablenum}..${lastnum})) {

    # Increment the table numbers
    my $newnum = $oldnum + 1; 
    my $newstr = "${chapter}\.${newnum}";

    #print "Sending ${chapter}\.${oldnum} to ${newstr}\n";
 
    # Match chapter and table, but don't allow a digit to follow (couldn't get working)
    # For example to match 5.3, but not 5.30
    #my $oldre = "${chapter}\.${oldnum}\D";

    # replace table names in \htmlonly
    #$dox_contents =~ s/Table ${chapter}\.${oldnum}(\s+)/Table ${chapter}\.${newnum}\1/g;

    # replace quoted table names
    #$dox_contents =~ s/"Table ${chapter}\.${oldnum}"/"Table ${chapter}\.${newnum}"/g;

    # Table 5.3, followed by mandatory (quote or space) to avoid replacing 5.30
    $dox_contents =~ s/Table ${chapter}\.${oldnum}(["\s]+)/Table ${newstr}\1/g;
 
    # replace quoted table numbers: caution, could replace Figure is quoted, but no current occurences
    $dox_contents =~ s/"${chapter}\.${oldnum}"/"${newstr}"/g;

    # replace table anchors and references, requiring surrounding space
    $dox_contents =~ s/(\s+)T${chapter}d${oldnum}(\s+)/\1T${chapter}d${newnum}\2/g;

  }

  open (REF_OUTPUT, ">$ref_file") ||
    die "\nERROR: Cannot open $ref_file\n$!";
  print REF_OUTPUT $dox_contents;
  close(REF_OUTPUT);

}

print "Updates complete\n";



