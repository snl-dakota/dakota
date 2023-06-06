#!/usr/bin/perl

# See documentation at end, or run dprepro --man
#  dprepro [options] parameters_file template_input_file new_input_file

#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

use Getopt::Long;
use Pod::Usage;
# use the following for additional trig functions and floor/ceil/round
#use Math::Trig;
#use POSIX;

# default delimiters are curly braces { }
my $ld = "\{";
my $rd = "\}";
# default output does not reformat numeric fields
my $global_output_format;
undef $global_output_format;

# process options for delimiters, format, and check argument validity
process_command_line();

$params_file   = $ARGV[0]; # DAKOTA parameters file (aprepro or standard format)
$template_file = $ARGV[1]; # template simulation input file
$new_file      = $ARGV[2]; # new simulation input file with insertions


# Regular expressions for numeric fields
$e  = "-?(?:\\d+\\.?\\d*|\\.\\d+)[eEdD](?:\\+|-)?\\d+"; # exponential notation
$f  = "-?\\d+\\.\\d*|-?\\.\\d+";                        # floating point
$i  = "-?\\d+";                                         # integer
$ui = "\\d+";                                           # unsigned integer
# Note: these should be protected within () due to OR's
$t  = "\\w+(?::\\w+)*"; # tag (colon separated alphanumeric blocks:)
$s  = "\"*$t\"*";       # string (tag with optional double quotes;
                        #         Dakota only uses double quotes, not single)
$v  = "$e|$f|$i|$s";    # value (numeric field: exponential, float, int, or 
                        # quoted string, with same admissible characters as tag)

# Regular expression for sprintf format strings.
# See: http://www.perlmonks.org/?node_id=20519
$p  = "%[#0+ -]?\\d*\\.?\\d*[csduoxefgXEGpn]";

# Regular expression for an even number of backslashes
my $enbs  = "(?<!\\\\)(\\\\\\\\)*";

################################
# Process DAKOTA parameters file
################################

# Open parameters file for input.
open (DAKOTA_PARAMS, "<$params_file") || die "Can't open $params_file: $!";

# Read parameters file, extract tags and corresponding values in either aprepro
# "{ tag = value }" format or standard "value tag" format, and store the
# tag/value pairs in %values_p1.  Numeric fields from DAKOTA are either integral
# or exponential notation and there is only one tag/value pair per line
# (exception: standard format up to DAKOTA v3.3 uses "# variables # functions"
# on the first line).  However, to accomodate the possibility that parameters
# files may be generated from other sources, floats are also supported and
# multiple tag/value pairs are allowed per line.  Compound expressions are not
# currently allowed.
while (<DAKOTA_PARAMS>) { # read each line of file, one at a time
  # extract tag/value fields allowing multiple matches per line in either format
  foreach $field (m/(?:$v)\s+(?:$t)|\{\s*(?:$t)\s*=\s*(?:$v)\s*\}/go) {
    # extract tag/value pair from each field
    if ( ( ($value, $tag) = ($field =~ m/^($v)\s+($t)$/o) ) ||       # Standard
         ( ($tag, $value) = ($field =~ m/^\{\s*($t)\s*=\s*($v)\s*\}$/o))){# Apr
      # No longer convert exponents as dprepro mainly intended for Dakota
      # and not compatible with string-valued variables
      #$value =~ s/[dD]/e/o;      # convert any F77 dbl prec exponents
      if ( $value =~ m/^\"*($t)\"*$/o ) {
        $value = $1;  # remove quotes that appeared in the params file
      }
      $values_p1{$tag} = $value; # store in hash
    }
  }
}
close (DAKOTA_PARAMS);


#################################################################
# Process template simulation file and create new simulation file
#################################################################

# Open the template simulation file for input.
open (TEMPLATE_SIMULATION, "<$template_file") ||
  die "Can't open $template_file: $!";
# Open the new simulation file for output.
open (NEW_SIMULATION, ">$new_file") || die "Can't create $new_file: $!";

# Read each line of template_file, find the {} fields, process any
# assignments or expressions, and substitute the corresponding values.
# Print each line with substitution to new_file.  The DAKOTA parameters
# file assignments (%values_p1 = precedence 1) take precedence over any
# duplicate template file assignments (%values_p2 = precedence 2) in
# order to allow the flexibility to define defaults in the template
# file which can then be overridden by a particular parameters file.
while (<TEMPLATE_SIMULATION>) {

  # a match that ignores escaped delimiters via $enbs  
  # the delimiters should already be escaped
  # ".+?" provides a minimal match on the field content
  my $escaped_match = "${ld}\\s*(.+?\\s*${enbs})${rd}"; 

  # Extract each (likely {}) delimited match from this line
  while (m/${enbs}${escaped_match}/go) {

    # $1 is a subexpression of $enbs; set field to the group from $escaped_match
    my $field_and_format = $2;

    # strip off any trailing whitespace ahead of time to simplify below
    $field_and_format =~ s/\s+$//o;

    # unescape any escaped characters _inside_ the field
    $field_and_format =~ s/\\(.)/$1/go;
    
    # split field and (optional) format
    $field_and_format =~ m/^(.*?)(?:,($p))?$/;
    $field=$1;
    $format=$2;

    # Case 1: test for simple tag match "{tag}"
    if ($field =~ m/^($t)$/o) {
      
      if ( defined ( $value = $values_p1{$field} ) ||
           defined ( $value = $values_p2{$field} ) ) { # or exists $values{$tag}
        my $fv = format_value($value,$format);
	s/($enbs)$escaped_match/$1${fv}/;
      }
    }

    # Case 2: test for assignment "{tag = field}"
    elsif ( ($tag, $assign) = ($field =~ m/^($t)\s*=\s*(.+?)$/o) ) {

      # If tag defined in values_p1 (parameters file), use that value,
      # otherwise insert (possibly evaluated) $assign in values_p2 for
      # later use
      if ( not defined ( $value = $values_p1{$tag} ) ) {

        # Case 2a: assignment of numerical value
        # ($v = exponential notation $e, floating point $f, or integer $i)
        if ($assign =~ m/^($v)$/o) {
	  # No longer convert exponents as dprepro mainly intended for Dakota
	  # and not compatible with string-valued variables
          #$assign =~ s/[dD]/e/o;               # convert F77 dbl prec exponents
	  $value = $assign;
        }

        # Case 2b: assignment of expression.  Evaluate $assign by replacing any
        # known tags with their values and then eval the remaining expression.
        else {
          foreach $exptag ($assign =~ m/\b(\w*[a-zA-Z_]+\w*)\b/go) {
            if ( defined ( $value = $values_p1{$exptag} ) ||
                 defined ( $value = $values_p2{$exptag} ) ) {
              $assign =~ s/$exptag/$value/;
            }
          }
          $value = eval $assign;
          if ($@) { die "Eval error: $@"; }
        }
        if ( $value =~ m/^\"*($t)\"*$/o ) {
	  $value = $1;  # remove quotes that appeared in the assignment
	}
	$values_p2{$tag} = $value;          # store in priority 2 hash

      }
      my $fv = format_value($value,$format);
      s/($enbs)$escaped_match/$1${fv}/; # replace assignment with value
    }

    # Case 3: assume general expression in all remaining $field matches.
    # Evaluate $field by replacing any known tags with their values and
    # then eval the remaining expression.
    else {
      foreach $tag ($field =~ m/\b(\w*[a-zA-Z_]+\w*)\b/go) {
        if ( defined ( $value = $values_p1{$tag} ) ||
             defined ( $value = $values_p2{$tag} ) ) {
          $field =~ s/$tag/$value/;
        }
      }
      my $evalfield = eval $field;
      if ($@) { die "Eval error: $@"; }
      my $fv = format_value($evalfield,$format);
      s/($enbs)$escaped_match/$1${fv}/;
    }
  }

  # unescape any escaped characters _outside_ the fields
  s/\\(.)/$1/go;

  # output the processed line to the new simulation file
  print NEW_SIMULATION $_;
}
close (TEMPLATE_SIMULATION);
close (NEW_SIMULATION);

# Print %values for debugging purposes.
#foreach $tag (sort keys %values_p1) {
#  print $tag, " ", $values_p1{$tag}, "\n";
#}


##########################################
# End script, begin subroutine definitions
##########################################

# Intrinsic numeric operators include +,-,*,/,**,%,<<,>>,sqrt(),abs(),
# sin(),cos(),atan2(),exp(),log(),int(),hex(),oct(),rand(),srand().

# Augment these with others (adapted/extended from bprepro by Bob Walton).
# NOTE: convert from degs to rads by pi/180 = (pi/4)/45 = atan2(1,1)/45
#       convert from rads to degs by 180/pi = 45/atan2(1,1)

# additional logarithmic functions

sub log10 { log($_[0])/log(10) }

# additional trigonometric functions with radian input

sub tan { sin($_[0])/cos($_[0]) }

sub cot { cos($_[0])/sin($_[0]) }

sub csc { 1/sin($_[0]) }

sub sec { 1/cos($_[0]) }

# trigonometric functions with degree input

sub sind { sin($_[0]*atan2(1,1)/45) }

sub cosd { cos($_[0]*atan2(1,1)/45) }

sub tand { tan($_[0]*atan2(1,1)/45) }

sub cotd { cot($_[0]*atan2(1,1)/45) }

sub cscd { 1/sin($_[0]*atan2(1,1)/45) }

sub secd { 1/cos($_[0]*atan2(1,1)/45) }

# inverse trigonometric functions returning radians

sub asin {
  if (abs($_[0]) > 1) { die "input out of range in asin()\n"; }
  atan2($_[0],sqrt(1-$_[0]**2));
}

sub acos {
  if (abs($_[0]) > 1) { die "input out of range in acos()\n"; }
  atan2(sqrt(1-$_[0]**2),$_[0]);
}

sub atan { atan2($_[0],1) }

# inverse trigonometric functions returning degrees

sub asind { asin($_[0])*45/atan2(1,1) }

sub acosd { acos($_[0])*45/atan2(1,1) }

sub atand { atan2($_[0],1)*45/atan2(1,1) }

sub atan2d { atan2($_[0],$_[1])*45/atan2(1,1) }

# hyperbolic functions

sub sinh { (exp($_[0]) - exp(-$_[0]))/2 }

sub cosh { (exp($_[0]) + exp(-$_[0]))/2 }

sub tanh { sinh($_[0])/cosh($_[0]) }

sub coth {
  if ($_[0] == 0) { die "input out of range in coth()\n"; }
  cosh($_[0])/sinh($_[0]);
}

sub csch {
  if ($_[0] == 0) { die "input out of range in csch()\n"; }
  1/sinh($_[0]);
}

sub sech { 1/cosh($_[0]) }

# inverse hyperbolic functions

sub asinh { log($_[0] + sqrt($_[0]**2 + 1)) }

sub acosh {
  if ($_[0] < 1) { die "input out of range in acosh()\n"; }
  log($_[0] + sqrt($_[0]**2 - 1));
}

sub atanh {
  if (abs($_[0]) >= 1) { die "input out of range in atanh()\n"; }
  log((1+$_[0])/(1-$_[0]))/2;
}



# Uses and modifies global @ARGV.  May set global option variables:
#   ld, rd, global_output_format
sub process_command_line {

  my $opt_left_delim, $opt_right_delim, $opt_output_format, $opt_help, $opt_man;

  # Process long options
  GetOptions('left-delimiter=s'   => \$opt_left_delim,
  	     'right-delimiter=s'  => \$opt_right_delim,
  	     'output-format=s'    => \$opt_output_format,
  	     'help|?'             => \$opt_help,
  	     'man'                => \$opt_man
	     ) || pod2usage(1);
  pod2usage(0) if $opt_help;
  pod2usage(-exitstatus => 0, -verbose => 2) if $opt_man;
  
  if ($opt_left_delim || $opt_right_delim) {
    if (!$opt_right_delim || !$opt_left_delim) {   
      die "Error: both left and right delimiter must be specified";
    }
    # set user delimiters, 
    # globally escaping metacharacters as necessary
    ($ld = $opt_left_delim) =~ s/([\\\.\^\$\*\+\?\{\}\[\]\(\)\|])/\\$1/go;
    ($rd = $opt_right_delim) =~ s/([\\\.\^\$\*\+\?\{\}\[\]\(\)\|])/\\$1/go;
  }

  if ($opt_output_format) {
    # TODO: escape metacharacters?
    $global_output_format = $opt_output_format;
  }

  # Check for correct number of command line arguments
  pod2usage(1) if( @ARGV != 3 );

}


# If a user-specified output format is active, apply it; otherwise
# return argument 
sub format_value {

  my $local_value = shift(@_);  
  my $local_format = shift(@_);
 
  # optional $local_format for the field, extracted from template,
  # takes precendence over optional $output_format defined by 
  # command line option

  if (defined $local_format) {
    return(sprintf($local_format, $local_value));
  } elsif (defined $global_output_format) {
    return(sprintf($global_output_format, $local_value));
  }
  else {
    return($local_value);
  }

}

# dprepro plain old documentation, for pod2usage

=head1 NAME

dprepro - A Perl pre-processor for manipulating input files with DAKOTA.

=head1 SYNOPSIS

dprepro [options] parameters_file template_input_file new_input_file

=head1 OPTIONS

=over 4

=item B<--help>

print brief help message
 
=item B<--man>  

print full manual page and exit

=item B<--left-delimiter=string, --right-delimiter=string>

override the default curly brace delimiters { and }; these may need to
be quoted or escaped on the command line, e.g.,
  require escaping with backslash: ` $ # 
  require quotes or escaping: \ ' ( ) |

=item B<--output-format=string>

use the specified C-style format specification string as the 
default for numeric output in all substitutions; will be overridden
by field-specific format strings; may need to be quoted or escaped

=back

=head1 DESCRIPTION

Reads the variable tags and values from the parameters_file and then
replaces each appearance of "{tag}" in the template_input_file with
its associated value in order to create the new_input_file.  The
parameters_file written by Dakota may either be in standard format
(using "value tag" constructs) or in "aprepro" format (using "{ tag =
value }" constructs), and the variable tags used inside
template_input_file must match the variable descriptors specified in
the Dakota input file.  Supports assignments and numerical expressions
in the template file; the parameters file takes precedence in the
case of duplicate assignments (so that template file assignments can
be treated as defaults to be overridden). Also supports per-tag format
strings, e.g. {tag,%5.3f}, which override the format string provided
as a command line option.

To leave a literal delimiter, e.g., brace or backslash, in the
template file without its enclosing contents being treated as a
dprepro expression for substitution, escape it with backslash, e.g.,
\{ dontsub \} or \\ dontsub \\.

=over 4

=item B<basic operation>
 
dprepro params.in app.template app.input

=item B<specifying output format>

dprepro --output-format='%10.5f' params.in app.template app.input

Alternately, specify per-field formatting within the delimiters in the
templaqte file:
  {DakotaParam1,%10.5f}
  {DakotaParam2,%06d}

=item B<specifying double brackets as delimiters>

dprepro --left-delimiter='[[' --right-delimiter=']]' params.in app.template app.input


=back

=cut

