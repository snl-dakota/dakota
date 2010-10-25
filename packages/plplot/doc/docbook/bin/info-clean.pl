#!/usr/bin/perl
 
$inpart = 0;
$intoc = 0;
while (<>) {
  if (/(<!DOCTYPE.*)\[(.*)/) {
    print "$1 \"foobar\" [$2\n" ; next}
  if (/(.*)<part>(.*)/) {
    $inpart = 1;
    print "$1<!--<part>";
    $_ = "$2\n";
    redo;
  }
  if ($inpart and m{(.*)</title>(.*)}) {
    $inpart = 0;
    print "$1</title>-->";
    $_ = "$2\n";
    redo;
  }
  if (m{(.*)</part>(.*)}) {
    $inpart = 1;
    print "$1<!--</part>-->";
    $_ = "$2\n";
    redo;
  }
  if (/(.*)<toc>(.*)/) {
    $intoc = 1;
    print "$1<!--<toc>";
    $_ = "$2\n";
    redo;
  }
  if (m{(.*)</toc>(.*)}) {
    $intoc = 0;
    print "$1</toc>-->";
    $_ = "$2\n";
    redo;
  }

  # The hacks below are necessary because docbook2texixml does
  # not know hot to include system entities
  if (m{(.*)<!ENTITY % ([^\s]+) SYSTEM "([^"]+)">(.*)}) {
    $sysent{$2} = $3;
    print "$1";
    $_ = "$4\n";
    redo;
  }
  if (m{(.*)%([^;]+);(.*)}) {
    print "$1";
    $tmp = "$3\n";
    print `cat $sysent{$2}`
      if (not m{%Math-Entities;});
    $_ = $tmp;
    redo
  }

  print;
}

