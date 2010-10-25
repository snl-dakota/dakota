#----------------------------------------------------------------------------
# $Id: x04.tcl 3186 2006-02-15 18:17:33Z slbrow $
#----------------------------------------------------------------------------

proc x04 {{w loopback}} {

    $w cmd plfont 2
    plot41 $w 0
    plot41 $w 1

    # Restore defaults
    $w cmd plfont 1
    $w cmd plcol0 1

}

proc plot41 {w type} {

    set pi 3.14159265358979323846
    matrix freql f 101
    matrix ampl f 101
    matrix phase f 101

    $w cmd pladv 0

    set f0 1.0
    for {set i 0} {$i <= 100} {incr i} {
	freql $i = [expr -2.0 + $i / 20.0]
	set freq [expr pow(10.0, [freql $i])]
	ampl $i = [expr 20.0 * log10(1.0 / sqrt(1.0 + pow(($freq/$f0), 2)))]
	phase $i = [expr -(180.0 / $pi) * atan($freq/$f0)]
    }

    $w cmd plvpor 0.15 0.85 0.1 0.9
    $w cmd plwind -2.0 3.0 -80.0 0.0

    $w cmd plcol0 1
    if {$type == 0} {
       $w cmd plbox "bclnst" 0.0 0 "bnstv" 0.0 0
    } elseif {$type ==1} {
       $w cmd plbox "bcfghlnst" 0.0 0 "bcghnstv" 0.0 0
    } else {
       puts "error: type must be either 0 or 1"
    }

    $w cmd plcol0 2
    $w cmd plline 101 freql ampl

    $w cmd plcol0 1
    $w cmd plptex 1.6 -30.0 1.0 -20.0 0.5 "-20 dB/decade"

    # Put labels on
    $w cmd plcol0 1
    $w cmd plmtex "b" 3.2 0.5 0.5 "Frequency"
    $w cmd plmtex "t" 2.0 0.5 0.5 "Single Pole Low-Pass Filter"
    $w cmd plcol0 2
    $w cmd plmtex "l" 5.0 0.5 0.5 "Amplitude (dB)"

    # For the gridless case, put phase vs freq on same plot
    if {$type == 0} {
       $w cmd plcol0 1
       $w cmd plwind -2.0 3.0 -100.0 0.0
       $w cmd plbox "" 0.0 0 "cmstv" 30.0 3
       $w cmd plcol0 3
       $w cmd plline 101 freql phase
       $w cmd plcol0 3
       $w cmd plmtex "r" 5.0 0.5 0.5 "Phase shift (degrees)"
    }
}
