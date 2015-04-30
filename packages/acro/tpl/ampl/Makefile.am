## Process this file with automake to produce 'Makefile.in'.

## Set the AMPL solver interface library name.
lib_LTLIBRARIES = libamplsolver.la
#ampl_LDADD = amplsolver.a

#if WITH_Darwin
#CFLAGS += -D_NONSTD_SOURCE
#endif

if WITH_SunOS_i386
SunOS_i386 = fpsetprec.s
endif

if WITH_SunOS_i86pc
SunOS_i86pc = fpsetprec64.s
endif

## AMPL library sources.
libamplsolver_la_SOURCES = \
	asldate.c \
	atof.c \
	auxinfo.c \
	avldelete.c \
	avltree.c \
	b_search.c \
	basename.c \
	bscanf.c \
	com2eval.c \
	comeval.c \
	con1ival.c \
	con2ival.c \
	con2val.c \
	conadj.c \
	conpval.c \
	conscale.c \
	conval.c \
	derprop.c \
	dtoa1.c \
	duthes.c \
	dynlink.c \
	f_read.c \
	fg_read.c \
	fg_write.c \
	fgh_read.c \
	fpecatch.c \
	fpinit.c \
	fullhes.c \
	func_add.c \
	funcadd1.c \
	g_fmt.c \
	genrowno.c \
	getenv.c \
	getstub.c \
	htcl.c \
	jac0dim.c \
	jac2dim.c \
	jacdim.c \
	jacinc.c \
	jacinc1.c \
	mach.c \
	mainexit.c \
	mip_pri.c \
	misc.c \
	mypow.c \
	names.c \
	nl_obj.c \
	nqpcheck.c \
	obj2val.c \
	obj_prec.c \
	objconst.c \
	objval.c \
	objval_.c \
	op_type.c \
	pfg_read.c \
	pfghread.c \
	printf.c \
	pshvprod.c \
	punknown.c \
	qp_read.c \
	qpcheck.c \
	qsortv.c \
	readsol.c \
	repwhere.c \
	rops.c \
	rops2.c \
	sigcatch.c \
	sos_add.c \
	sphes.c \
	sscanf.c \
	stderr.c \
	studchk0.c \
	suf_sos.c \
	value.c \
	writesol.c \
	wrtsol_.c \
	ws_desc.c \
	wsu_desc.c \
	x2check.c \
	xectim.c \
	xp1known.c \
	xp2known.c \
	$(SunOS_i386) $(SunOS_i86pc)

## AMPL headers which need to be distributed.
pkginclude_HEADERS = \
	arith.h \
	asl.h \
	asl_pfg.h \
	asl_pfgh.h \
	avltree.h \
	dvalue.hd \
	errchk.h \
	funcadd.h \
	getstub.h \
	jacpdim.h \
	jac2dim.h \
	nlp.h \
	nlp2.h \
	opcode.hd \
	opnos.hd \
	op_typeb.hd \
	op_type.hd \
	psinfo.h \
	r_op.hd \
	r_opn.hd \
	r_opn0.hd \
	r_qp.hd \
	stdio1.h


## Extra files for the distribution.
EXTRA_DIST = \
	README \
	README.f77 \
	amplsolv.lbc \
	amplsolv.sy \
	arith.ibm \
	arith.h0 \
	arithchk.c \
	comptry.bat \
	configure0 \
	configurehere \
	details.c0 \
	dtoa.c \
	float.h0 \
	fpinitmt.c \
	fpsetprec.s \
	fpsetprec64.s \
	funcadd.c \
	funcadd0.c \
	funcaddk.c \
	funcaddr.c \
	makefile.lc \
	makefile.sy \
	makefile.u \
	makefile.vc \
	makefile.wat \
	rnd_prod.s \
	sjac0dim.c \
	sprintf.c \
	stdio1.h0 \
	strerror.c \
	xsum0.out

## Files to remove when doing 'make maintainer-clean'.
MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	ampl_config.h.in install-sh ltmain.sh missing Makefile.in Makefile2 \
	arith.h stdio1.h details.c amplsolver.a
