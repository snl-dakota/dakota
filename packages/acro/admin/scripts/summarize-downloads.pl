#!/usr/bin/env perl
#
# A convenient little script to summarize the downloads.csv scraped from
# the various project Trac sites.
use Date::Parse;

if ( $#ARGV > 3 || $#ARGV < 1 )
  { die "Usage: $0 username password [start_date [end_date]]
       (date format = mm/dd/yy [hh:mm:ss])\n"; }

($user, $pass, $start, $end) = @ARGV;
$STime = $start ? str2time($start) : 0;
$ETime = $end ? str2time($end) : time+time;

$FILE="downloads.csv";
@Packages = qw( Coopr FAST PyUtilib TicketModerator page-markup 
		acro-pico acro-pebbl acro-coliny acro utilib 
		CANARY_Webinar canary teva-spot );

unlink $FILE if ( -f $FILE );
system("./trac-download-stats.sh $user $pass");
open DATA, "<$FILE";
while ( $_ = <DATA> )
{
  ($x, $file, $ip, $d) = split /,/;
  $d =~ s/\"//g;
  $file =~ s/\"//g;
  $ip =~ s/\"//g;
  $Date = str2time($d);
  next if ( $Date < $STime || $Date >= $ETime );
  next if ( &snl_ip($ip) );
  ++$Count;
  $data_by_file{$file}{$ip}++;
  $data_by_ip{$ip}++;
  for $pkg ( @Packages )
    {
    if ( $file =~ /$pkg/i )
      { 
      $data_by_pkg{$pkg}{$ip}++; 
      last;
      }
    }
}
close DATA;

print "Non-SNL downloads:   $Count\n";
print "Unique IP addresses: ".keys(%data_by_ip)."\n";
print "Unique IP addresses by raw file:\n";
for ( sort keys %data_by_file )
{
  print sprintf "%8i %s\n", 0+keys(%{$data_by_file{$_}}), $_;
}
print "Unique IP addresses by project:\n";
for ( sort keys %data_by_pkg )
{
  print sprintf "%8i %s\n", 0+keys(%{$data_by_pkg{$_}}), $_;
}


exit 0;
###########################################################
sub snl_ip($)
{
  my($ip) = @_;
  foreach ( '^132\.175\.',
	    '^134\.25[23]\.',
	    '^134\.218\.',
	    '^146\.246\.',
	    '^192\.73\.207\.',
	    '^196\.208\.22[0123]\.',
	    '^198\.206\.219\.', # New: SNL/CA proxy
	    '^205\.137\.8?\.',
	    '^205\.137\.9[012345]\.',
	    )
    { return 1 if ( $ip =~ /$_/ ); }
      
  return 0;
}
