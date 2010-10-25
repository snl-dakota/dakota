#!/usr/bin/perl
# File: api2man.pl
# Description: Convert the PLplot API chapter (file api.xml of the DocBook
#              manual) into a series of man pages.
# Author: Rafael Laboissière <rafael@debian.org>
# Created on: Mon Dec  4 16:35:32 CET 2000
# Last modified on: Thu Mar 20 11:30:25 CET 2003
# $Id: api2text.pl 3186 2006-02-15 18:17:33Z slbrow $
#
# Copyright (C) 2000, 2003  Rafael Laboissiere
#
# This script relies on the present structure of the API chapter (file
# ../src/api.xml), where each API function is documented in its own
# section.  Here is the typical structure of a section:
#
#   <sect1 id="NAME">
#     <title><function>NAME</function>: DESCRIPTION</title>
#
#     <para>
#       <funcsynopsis>
#       <funcprototype>
# 	<funcdef>
# 	  <function>NAME</function>
# 	</funcdef>
#         <paramdef><parameter>ARG1</parameter></paramdef>
#         <paramdef><parameter>ARG2</parameter></paramdef>
#          ...
#       </funcprototype>
#       </funcsynopsis>
#     </para>
#
#     <para>
#       DESCRIPTION
#     </para>
#
#     <variablelist>
#       <varlistentry>
# 	<term>
# 	  <parameter>ARG1</parameter>
# 	  TYPE
# 	</term>
# 	<listitem>
# 	  <para>
# 	    ARG1 DESCRIPTION
# 	  </para>
# 	</listitem>
#       </varlistentry>
#       ...
#     </variablelist>
#
#   </sect1>

#  Call this script by ginving the master file (typically
#  plplotdoc.xml) as the first argument, the API chapter file
#  (typically api.xml) as second argument, and the man volume
#  (typicvally 3plplot) as third.

use XML::Parser;
use XML::DOM;
use Text::Wrap;
$Text::Wrap::columns = 75;
use Text::Tabs;
$tabstop = 4;

$api = "";
open (MASTER, "< $ARGV[0]");
while (<MASTER>) {
  if (/^(<!DOCTYPE.*)\[/) {
    $api .= "$1 \"\" [\n";
  }
  elsif (/^<\?xml/) {
    $api .= '<?xml version="1.0" standalone="yes"?>
';
  }
  elsif (/^<!ENTITY pl/) {
    $api .= $_;
  }
}
$api .= "<!ENTITY amp '#38;#38;'>
<!ENTITY deg ' degrees'>
<!ENTITY gt '&#x003E;'>
<!ENTITY leq '&#38;#60;='>
<!ENTITY lt '&#38;#60;'>
<!ENTITY ndash '--'>
]>\n";
close MASTER;

open (API, "< $ARGV[1]");
$/ = undef;
$api .= <API>;
close API;

sub process_node {
  my $ret = "";
  my $t = shift;
  my $c = $t->getChildNodes;
  my $m = $c->getLength;
  for (my $j = 0; $j < $m; $j++) {
    my $e = $c->item($j);
    my $nt = $e->getNodeType;
    if ($nt == TEXT_NODE) {
      my $a = $e->getData;
      $a =~ s/^\s+/ /;
      $a =~ s/^\s+$//;
      $a =~ s/\n\s+/ /g;
      $ret .= $a;
    }
    elsif ($nt == ELEMENT_NODE) {
      my $tag = $e->getTagName;
    if ($tag eq "parameter") {
        $ret .= "\n" . process_node ($e);
      }
      elsif  ($tag eq "function") {
        $ret .= process_node ($e);
      }
      elsif  ($tag eq "link") {
        $ret .= process_node ($e) ;
      }
      elsif  ($tag eq "funcprototype") {
        $startproto = 1;
	my $p = process_node ($e);
	$p =~ s/ +$//;
        $ret .= $p . ")";
      }
      elsif  ($tag eq "paramdef") {
	my $p = process_node ($e);
	$p =~ s/ +$//;
        $ret .= ($startproto ? "(" : ", ") . $p;
        $startproto = 0;
      }
      elsif  ($tag eq "term") {
        $ret .= "\n" . process_node ($e) . ":";
      }
      elsif  ($tag eq "listitem") {
        $ret .= "\t" . process_node ($e) . "\n";
      }
      elsif  ($tag eq "xref") {
        $ret .= "the PLplot documentation";
      }
      elsif  ($tag eq "varlistentry") {
        $ret .= "\n" . process_node ($e);
      }
      else {
        $ret .= process_node ($e);
      }
    }
    else {
      $ret .= process_node ($e);
    }
  }
  $ret =~ s/^\s+//;
  return $ret;
}

$p = new XML::DOM::Parser;
$sects = $p->parse ($api)->getElementsByTagName ("sect1");
my $ns = $sects->getLength;
$titles = "";

for ($i = 0; $i < $ns; $i++) {
  $fun = $sects->item ($i);
  $name = $fun->getAttribute ("id");
  $c = $fun->getChildNodes;
  $nc = $c->getLength;
  $desc = "";
  $varlist = "";
  $got_synopsis = 0;
  for ($j = 0; $j < $nc; $j++) {
    $part = $c->item($j);
    if ($part->getNodeType == ELEMENT_NODE) {
      $contents = process_node ($part);
      $node = $part->getTagName;
      if ($node eq "title") {
        $title = $contents;
      }
      elsif ($node eq "para") {
        if ($got_synopsis) {
          $desc =  $contents;
        }
        else {
          $synopsis = $contents;
          $got_synopsis = 1;
        }
      }
      elsif ($node eq "variablelist") {
        $varlist = $contents;
      }
    }
  }
  $indent = '    ';
  $desc = wrap ($indent, $indent, split(/\n\s*\n/, $desc));
  $varlist = join ("\n", map {
                           s/\t/    /g;
                           /(^\s+)/;
                           $_ = wrap ($indent . $1,
                                      $indent . $1 . $indent, $_);
                           s/\t/    /g;
                           $_;
                         } split ("\n", $varlist));
  open (MAN, "> $name.txt");
  print MAN "\n$title\n";
  print MAN "\nDESCRIPTION:\n\n$desc\n";
  print MAN "\nSYNOPSIS:\n\n$synopsis\n";
  print MAN "\nARGUMENTS:\n\n$varlist\n"
    if not $varlist eq "";
  close MAN;
}
