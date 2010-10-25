#!/usr/bin/perl
#
# This is a PERL script to 
#
$pwd = `pwd`;
chomp($pwd);
$ENV{PATH} = $ENV{PATH} . ":.:$pwd/../../acro/packages/utilib/src/doe";

if ($ARGV[0] eq "--debug") {
  $debug="--debug";
  shift @ARGV;
} else {
  $debug="";
}

foreach $file (@ARGV) {
  print "runexp $debug $file\n";
  }
