#----------------------------------------------------------------------------
# $Id: x10.tcl 3186 2006-02-15 18:17:33Z slbrow $
#----------------------------------------------------------------------------

proc x10 {{w loopback}} {
  $w cmd pladv 0
  $w cmd plvpor 0.0 1.0 0.0 1.0
  $w cmd plwind 0.0 1.0 0.0 1.0
  $w cmd plbox bc 0.0 0 bc 0.0 0
  $w cmd plsvpa 50.0 150.0 50.0 100.0
  $w cmd plwind 0.0 1.0 0.0 1.0
  $w cmd plbox bc 0.0 0 bc 0.0 0
  $w cmd plptex 0.5 0.5 1.0 0.0 0.5 "BOX at (50,150,50,100)"
# Restore defaults
  $w cmd plcol0 1
}
