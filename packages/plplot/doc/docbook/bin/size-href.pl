#!/usr/bin/perl
#
# Copyright (C) 2004 Rafael Laboissiere
#
# This file is part of PLplot
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

sub get_size {
  my $f = shift;
  my $s = 0;
  $s = `ls -sh $f`;
  $s =~ s/\s*([0-9.]+[MKk])\s+.*/$1/;
  chomp $s;
  return $s;
}

while (<>) {
  if (/(.*)href="(.*)"(.*)/) {
    $file = $2;
    print "$1href=\"$2\"";
    $_ = "$3\n";
    redo;
  }
  if (/(.*)\@SIZE@(.*)/) {
    if ( -f $file ) {
      $size = get_size ($file);
    }
    elsif ( -f "src/$file" ) {
      $size = get_size ("src/$file")
    }
    else {
      die "Neither $file nor src/$file exist";
    }
    print "$1($size)$2\n";
    next;
  }
  print;
}
