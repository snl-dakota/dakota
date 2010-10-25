@echo off
cls

set cl= /FPi87 /AL /Od /Zi /DPLAIN_DOS
set fl= /FPi87 /AL /Od /Zi /link /NOE /NOD:llibfor7 llibc7 llibf7rc

echo *
echo *

if %1.==.		goto help
if %1 == mainlinks	goto mainlinks
if %1 == stublinks	goto stublinks
if %1 == mainlib	goto mainlib
if %1 == drivers	goto drivers
if %1 == dosvga		goto dosvga
if %1 == plmeta		goto plmeta
if %1 == render		goto render
if %1 == stublib	goto stublib
if %1 == fstubs		goto fstubs
if %1 == cdemos		goto cdemos
if %1 == fdemos		goto fdemos

goto error
:*******************************************************************************
:error
	echo *** You did not issue an allowable command.	Run with no
	echo *** commands to see a description of usage.
:*******************************************************************************
:help

echo *
echo * Instructions:
echo * To build PLPLOT on DOS, you need to do several things.  First you
echo * need to copy all the files to the tmp directory.  Do this by issuing
echo * the command:	plmake mainlinks
echo * and also:	plmake stublinks
echo * Then you can build the main library with:	plmake mainlib
echo * To build the stub library, issue:		plmake stublib
echo * To rebuild the drivers, issue:			plmake drivers
echo * To rebuild only the dosvga driver, issue:	plmake dosvga
echo * Once you have build both the mainlib and the stublib, you are
echo * ready to build make the examples, or any of your own programs.
echo * Do this with:	plmake cdemos
echo * or		plmake fdemos
echo *

goto done
:*******************************************************************************
:mainlinks
	echo * Copy all the main files over ...
	copy ..\src\*.c .
	copy ..\include\*.h .
	copy ..\examples\*.c .
	copy ..\fortran\examples\*.f .
	copy ..\os2\src\* .
	copy ..\os2\drivers\* .
	copy ..\dos\*.rsp .
	ren *.f *.for

goto done
:*******************************************************************************
:stublinks
	copy ..\os2\stubf\* .
	copy ..\os2\stubc\* .
	ren *.f *.for

goto done
:*******************************************************************************
:mainlib
	echo * Compiling all main library files.
	
qcl /c /Fodefine.o define.c
qcl /c /Fofcnvrt.o fcnvrt.c
qcl /c /Fogenlin.o genlin.c
qcl /c /Foglobal.o global.c
qcl /c /Foicnvrt.o icnvrt.c
qcl /c /Fomovphy.o movphy.c
qcl /c /Fomovwor.o movwor.c
qcl /c /Fopl3cut.o pl3cut.c
qcl /c /Foplabv.o plabv.c
qcl /c /Fopladv.o pladv.c
qcl /c /Foplbeg.o plbeg.c
qcl /c /Foplbin.o plbin.c
qcl /c /Foplbox.o plbox.c
qcl /c /Foplbox3.o plbox3.c
qcl /c /Foplccal.o plccal.c
qcl /c /Foplclr.o plclr.c
qcl /c /Foplcntr.o plcntr.c
qcl /c /Foplcol.o plcol.c
qcl /c /Foplcont.o plcont.c
qcl /c /Foplconf.o plconf.c
qcl /c /Foplcvec.o plcvec.c
qcl /c /Fopldeco.o pldeco.c
qcl /c /Fopldtik.o pldtik.c
qcl /c /Foplend.o plend.c
qcl /c /Foplenv.o plenv.c
qcl /c /Foplerrx.o plerrx.c
qcl /c /Foplerry.o plerry.c
qcl /c /Foplerx1.o plerx1.c
qcl /c /Foplery1.o plery1.c
qcl /c /Foplexit.o plexit.c
qcl /c /Foplfill.o plfill.c
qcl /c /Foplfont.o plfont.c
qcl /c /Foplfontld.o plfontld.c
qcl /c /Foplform.o plform.c
qcl /c /Foplgra.o plgra.c
qcl /c /Foplgrid3.o plgrid3.c
qcl /c /Foplgspa.o plgspa.c
qcl /c /Foplhist.o plhist.c
qcl /c /Foplhrsh.o plhrsh.c
qcl /c /Fopljoin.o pljoin.c
qcl /c /Fopllab.o pllab.c
qcl /c /Fopllclp.o pllclp.c
qcl /c /Foplline.o plline.c
qcl /c /Fopllsty.o pllsty.c
qcl /c /Foplmesh.o plmesh.c
qcl /c /Foplmtex.o plmtex.c
qcl /c /Foplnxtv.o plnxtv.c
qcl /c /Foplot3d.o plot3d.c
qcl /c /Foplpage.o plpage.c
qcl /c /Foplpat.o plpat.c
qcl /c /Foplpoi1.o plpoi1.c
qcl /c /Foplpoin.o plpoin.c
qcl /c /Foplpsty.o plpsty.c
qcl /c /Foplptex.o plptex.c
qcl /c /Foplr135.o plr135.c
qcl /c /Foplr45.o plr45.c
qcl /c /Foplschr.o plschr.c
qcl /c /Foplside3.o plside3.c
qcl /c /Foplsmaj.o plsmaj.c
qcl /c /Foplsmin.o plsmin.c
qcl /c /Foplssym.o plssym.c
qcl /c /Foplstar.o plstar.c
qcl /c /Foplstik.o plstik.c
qcl /c /Foplstr.o plstr.c
qcl /c /Foplstrl.o plstrl.c
qcl /c /Foplstyl.o plstyl.c
qcl /c /Foplsvpa.o plsvpa.c
qcl /c /Foplsym.o plsym.c
qcl /c /Foplsym1.o plsym1.c
qcl /c /Foplt3zz.o plt3zz.c
qcl /c /Fopltext.o pltext.c
qcl /c /Foplvpor.o plvpor.c
qcl /c /Foplvsta.o plvsta.c
qcl /c /Foplw3d.o plw3d.c
qcl /c /Foplwid.o plwid.c
qcl /c /Foplwind.o plwind.c
qcl /c /Foplxtik.o plxtik.c
qcl /c /Foplxybx.o plxybx.c
qcl /c /Foplxytx.o plxytx.c
qcl /c /Foplytik.o plytik.c
qcl /c /Foplzbx.o plzbx.c
qcl /c /Foplztx.o plztx.c
qcl /c /Fosetphy.o setphy.c
qcl /c /Fosetpxl.o setpxl.c
qcl /c /Fosetsub.o setsub.c
qcl /c /Fostindex.o stindex.c
qcl /c /Fostrpos.o strpos.c
qcl /c /Fostsearch.o stsearch.c

	goto compile_drivers
:*******************************************************************************
:drivers
	echo * Copying over the driver files
	copy ..\os2\drivers\* .

:compile_drivers
	echo * Compiling the drivers now.

qcl /c /Fodispatch.o dispatch.c
qcl /c /Fohp7470.o hp7470.c
qcl /c /Fohp7580.o hp7580.c
qcl /c /Foimpress.o impress.c
qcl /c /Fohpljii.o hpljii.c
qcl /c /Fopscript.o pscript.c
qcl /c /Fotektronx.o tektronx.c
qcl /c /Foplmeta.o plmeta.c
qcl /c /Fodosvga.o dosvga.c

	goto build_main_lib
:*******************************************************************************
:dosvga
	echo * Recompiling the DOSVGA driver, and rebuilding main library.

	copy ..\os2\drivers\dosvga.c .
	qcl /c /Fodosvga.o dosvga.c
	goto build_main_lib
:*******************************************************************************
:plmeta
	echo * Recompiling the PLMETA driver, and rebuilding main library.

	copy ..\os2\drivers\plmeta.c .
	copy ..\os2\drivers\metadefs.h .
	qcl /c /Foplmeta.o plmeta.c
	pause
	goto build_main_lib
:*******************************************************************************
:render
	echo * Rebuilding the PLPLOT Metafile RENDER utility.

	copy ..\os2\drivers\plrender.c .
	copy ..\os2\drivers\metadefs.h .
	qcl plrender.c plplot.lib
	goto done
:*******************************************************************************
:build_main_lib

	echo * Ready to put the main .o files into a library.
	del plplot.lib
	lib @mainlib.rsp

goto done
:*******************************************************************************
:stublib

:cstubs

qcl /c /Foscadv.o scadv.c
qcl /c /Foscbin.o scbin.c
qcl /c /Foscbox3.o scbox3.c
qcl /c /Foscbox.o scbox.c
qcl /c /Foscclr.o scclr.c
qcl /c /Fosccol.o sccol.c
qcl /c /Fosccont.o sccont.c
qcl /c /Foscconf.o scconf.c
qcl /c /Foscend.o scend.c
qcl /c /Foscenv.o scenv.c
qcl /c /Foscerrx.o scerrx.c
qcl /c /Foscerry.o scerry.c
qcl /c /Foscfill.o scfill.c
qcl /c /Foscfontld.o scfontld.c
qcl /c /Foscfont.o scfont.c
qcl /c /Foscgra.o scgra.c
qcl /c /Foscgspa.o scgspa.c
qcl /c /Foschist.o schist.c
qcl /c /Foscjoin.o scjoin.c
qcl /c /Fosclab.o sclab.c
qcl /c /Foscline.o scline.c
qcl /c /Fosclsty.o sclsty.c
qcl /c /Foscmesh.o scmesh.c
qcl /c /Foscmtex.o scmtex.c
qcl /c /Foscot3d.o scot3d.c
qcl /c /Foscpat.o scpat.c
qcl /c /Foscpoin.o scpoin.c
qcl /c /Foscprec.o scprec.c
qcl /c /Foscpsty.o scpsty.c
qcl /c /Foscptex.o scptex.c
qcl /c /Foscschr.o scschr.c
qcl /c /Foscsmaj.o scsmaj.c
qcl /c /Foscsmin.o scsmin.c
qcl /c /Foscssym.o scssym.c
qcl /c /Foscstar.o scstar.c
qcl /c /Foscstyl.o scstyl.c
qcl /c /Foscsvpa.o scsvpa.c
qcl /c /Foscsym.o scsym.c
qcl /c /Fosctext.o sctext.c
qcl /c /Foscvpor.o scvpor.c
qcl /c /Foscvsta.o scvsta.c
qcl /c /Foscw3d.o scw3d.c
qcl /c /Foscwid.o scwid.c
qcl /c /Foscwind.o scwind.c

:fstubs

fl /c /Foplbox3sf.o plbox3sf.for
fl /c /Foplboxsf.o plboxsf.for
fl /c /Foplcontsf.o plcontsf.for
fl /c /Foplconfsf.o plconfsf.for
fl /c /Fopllabsf.o pllabsf.for
fl /c /Foplmtexsf.o plmtexsf.for
fl /c /Foplptexsf.o plptexsf.for
fl /c /Foplstarsf.o plstarsf.for

	echo * Ready to put the main .o files into a library.
	del plstub.lib
	lib @stublib.rsp

goto done
:*******************************************************************************
:cdemos

qcl /AL /Od /Zi x01c.c plplot.lib
qcl /AL /Od /Zi x02c.c plplot.lib
qcl /AL /Od /Zi x03c.c plplot.lib
qcl /AL /Od /Zi x04c.c plplot.lib
qcl /AL /Od /Zi x05c.c plplot.lib
qcl /AL /Od /Zi x06c.c plplot.lib
qcl /AL /Od /Zi x07c.c plplot.lib
qcl /AL /Od /Zi x08c.c plplot.lib
qcl /AL /Od /Zi x09c.c plplot.lib
qcl /AL /Od /Zi x10c.c plplot.lib
qcl /AL /Od /Zi x11c.c plplot.lib
qcl /AL /Od /Zi x12c.c plplot.lib
qcl /AL /Od /Zi x13c.c plplot.lib

goto done
:*******************************************************************************
:fdemos

fl /AL /Od /Zi /nologo x01f.for plplot.lib plstub.lib
fl /AL /Od /Zi /nologo x02f.for plplot.lib plstub.lib
fl /AL /Od /Zi /nologo x03f.for plplot.lib plstub.lib
fl /AL /Od /Zi /nologo x04f.for plplot.lib plstub.lib
fl /AL /Od /Zi /nologo x05f.for plplot.lib plstub.lib
fl /AL /Od /Zi /nologo x06f.for plplot.lib plstub.lib
fl /AL /Od /Zi /nologo x07f.for plplot.lib plstub.lib
fl /AL /Od /Zi /nologo x08f.for plplot.lib plstub.lib
fl /AL /Od /Zi /nologo x09f.for plplot.lib plstub.lib
fl /AL /Od /Zi /nologo x10f.for plplot.lib plstub.lib
fl /AL /Od /Zi /nologo x11f.for plplot.lib plstub.lib
fl /AL /Od /Zi /nologo x12f.for plplot.lib plstub.lib
fl /AL /Od /Zi /nologo x13f.for plplot.lib plstub.lib

goto done
:*******************************************************************************
:done
	echo * All Done!
:*******************************************************************************
