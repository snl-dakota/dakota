/* -*-C-*- */
/* $Id: plDevs.h 3186 2006-02-15 18:17:33Z slbrow $

    Maurice LeBrun
    IFS, University of Texas at Austin
    18-Jul-1994

    Contains macro definitions that determine what device drivers are
    compiled into the PLplot library.  On a Unix system, typically the
    configure script builds plDevs.h from plDevs.h.in.  Elsewhere, it's
    best to hand-configure a plDevs.h file and keep it with the
    system-specific files.
*/

#ifndef __PLDEVS_H__
#define __PLDEVS_H__

#define PLD_plmeta
#define PLD_tkwin
#define PLD_null
#undef PLD_xterm
#undef PLD_tek4010
#undef PLD_tek4010f
#undef PLD_tek4107
#undef PLD_tek4107f
#undef PLD_mskermit
#undef PLD_vlt
#undef PLD_versaterm
#undef PLD_conex
#undef PLD_linuxvga
#undef PLD_dg300
#undef PLD_png
#undef PLD_jpeg
#undef PLD_cgm
#define PLD_ps
#undef PLD_xfig
#undef PLD_ljiip
#undef PLD_ljii
#undef PLD_lj_hpgl
#undef PLD_hp7470
#undef PLD_hp7580
#undef PLD_imp
#undef PLD_xwin
#undef PLD_tk
#undef PLD_pbm
#undef PLD_gnome
#undef PLD_pstex
#undef PLD_ntk

#endif	/* __PLDEVS_H__ */
