/*--------------------------------------------------------------------------*\
 * pl_setcontlabelformatCmd
 *
 * Processes pl_setcontlabelformat Tcl command.
\*--------------------------------------------------------------------------*/

static int
pl_setcontlabelformatCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT lexp;
    PLINT sigprec;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "pl_setcontlabelformat lexp sigprec", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 2 - 0))) ||
         (!0 && !0 && (argc != (2 + 1))) ||
         ( 0 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "pl_setcontlabelformat lexp sigprec", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    lexp = atoi(argv[1+0]);
    sigprec = atoi(argv[1+1]);

    pl_setcontlabelformat ( lexp, sigprec );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * pl_setcontlabelparamCmd
 *
 * Processes pl_setcontlabelparam Tcl command.
\*--------------------------------------------------------------------------*/

static int
pl_setcontlabelparamCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT offset;
    PLFLT size;
    PLFLT spacing;
    PLINT active;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "pl_setcontlabelparam offset size spacing active", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "pl_setcontlabelparam offset size spacing active", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    offset = atof(argv[1+0]);
    size = atof(argv[1+1]);
    spacing = atof(argv[1+2]);
    active = atoi(argv[1+3]);

    pl_setcontlabelparam ( offset, size, spacing, active );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * pladvCmd
 *
 * Processes pladv Tcl command.
\*--------------------------------------------------------------------------*/

static int
pladvCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT page = 0;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "pladv page", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 1 && (argc < (1 + 1 - 1))) ||
         (!0 && !1 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "pladv page", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if (argc > 0+1) {
        page = atoi(argv[1+0]);
    }

    pladv ( page );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plaxesCmd
 *
 * Processes plaxes Tcl command.
\*--------------------------------------------------------------------------*/

static int
plaxesCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT x0;
    PLFLT y0;
    char *xopt;
    PLFLT xtick;
    PLINT nxsub;
    char *yopt;
    PLFLT ytick;
    PLINT nysub;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plaxes x0 y0 xopt xtick nxsub yopt ytick nysub", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 8 - 0))) ||
         (!0 && !0 && (argc != (8 + 1))) ||
         ( 0 && (argc != 1) && (argc != (8 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plaxes x0 y0 xopt xtick nxsub yopt ytick nysub", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    x0 = atof(argv[1+0]);
    y0 = atof(argv[1+1]);
    xopt = argv[1+2];
    xtick = atof(argv[1+3]);
    nxsub = atoi(argv[1+4]);
    yopt = argv[1+5];
    ytick = atof(argv[1+6]);
    nysub = atoi(argv[1+7]);

    plaxes ( x0, y0, xopt, xtick, nxsub, yopt, ytick, nysub );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plbinCmd
 *
 * Processes plbin Tcl command.
\*--------------------------------------------------------------------------*/

static int
plbinCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT nbin;
    PLFLT *x;
    tclMatrix *matx;
    PLFLT *y;
    tclMatrix *maty;
    PLINT center;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plbin nbin x y center", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plbin nbin x y center", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    nbin = atoi(argv[1+0]);
    matx = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matx == NULL) return TCL_ERROR;
    x = matx->fdata;
    maty = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (maty == NULL) return TCL_ERROR;
    y = maty->fdata;
    center = atoi(argv[1+3]);

    plbin ( nbin, x, y, center );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plbopCmd
 *
 * Processes plbop Tcl command.
\*--------------------------------------------------------------------------*/

static int
plbopCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plbop ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 0 - 0))) ||
         (!0 && !0 && (argc != (0 + 1))) ||
         ( 0 && (argc != 1) && (argc != (0 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plbop ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }


    plbop (  );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plboxCmd
 *
 * Processes plbox Tcl command.
\*--------------------------------------------------------------------------*/

static int
plboxCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    char *xopt;
    PLFLT xtick;
    PLINT nxsub;
    char *yopt;
    PLFLT ytick;
    PLINT nysub;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plbox xopt xtick nxsub yopt ytick nysub", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 6 - 0))) ||
         (!0 && !0 && (argc != (6 + 1))) ||
         ( 0 && (argc != 1) && (argc != (6 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plbox xopt xtick nxsub yopt ytick nysub", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    xopt = argv[1+0];
    xtick = atof(argv[1+1]);
    nxsub = atoi(argv[1+2]);
    yopt = argv[1+3];
    ytick = atof(argv[1+4]);
    nysub = atoi(argv[1+5]);

    plbox ( xopt, xtick, nxsub, yopt, ytick, nysub );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plbox3Cmd
 *
 * Processes plbox3 Tcl command.
\*--------------------------------------------------------------------------*/

static int
plbox3Cmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    char *xopt;
    char *xlabel;
    PLFLT xtick;
    PLINT nsubx;
    char *yopt;
    char *ylabel;
    PLFLT ytick;
    PLINT nsuby;
    char *zopt;
    char *zlabel;
    PLFLT ztick;
    PLINT nsubz;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plbox3 xopt xlabel xtick nsubx yopt ylabel ytick nsuby zopt zlabel ztick nsubz", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 12 - 0))) ||
         (!0 && !0 && (argc != (12 + 1))) ||
         ( 0 && (argc != 1) && (argc != (12 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plbox3 xopt xlabel xtick nsubx yopt ylabel ytick nsuby zopt zlabel ztick nsubz", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    xopt = argv[1+0];
    xlabel = argv[1+1];
    xtick = atof(argv[1+2]);
    nsubx = atoi(argv[1+3]);
    yopt = argv[1+4];
    ylabel = argv[1+5];
    ytick = atof(argv[1+6]);
    nsuby = atoi(argv[1+7]);
    zopt = argv[1+8];
    zlabel = argv[1+9];
    ztick = atof(argv[1+10]);
    nsubz = atoi(argv[1+11]);

    plbox3 ( xopt, xlabel, xtick, nsubx, yopt, ylabel, ytick, nsuby, zopt, zlabel, ztick, nsubz );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plcol0Cmd
 *
 * Processes plcol0 Tcl command.
\*--------------------------------------------------------------------------*/

static int
plcol0Cmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT icol0;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plcol0 icol0", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plcol0 icol0", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    icol0 = atoi(argv[1+0]);

    plcol0 ( icol0 );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plcol1Cmd
 *
 * Processes plcol1 Tcl command.
\*--------------------------------------------------------------------------*/

static int
plcol1Cmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT col1;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plcol1 col1", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plcol1 col1", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    col1 = atof(argv[1+0]);

    plcol1 ( col1 );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * pldid2pcCmd
 *
 * Processes pldid2pc Tcl command.
\*--------------------------------------------------------------------------*/

static int
pldid2pcCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xmin;
    PLFLT ymin;
    PLFLT xmax;
    PLFLT ymax;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "pldid2pc ?xmin ymin xmax ymax?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 4 - 0))) ||
         (!1 && !0 && (argc != (4 + 1))) ||
         ( 1 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "pldid2pc ?xmin ymin xmax ymax?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* xmin is for output. */
/* ymin is for output. */
/* xmax is for output. */
/* ymax is for output. */

    pldid2pc ( &xmin, &ymin, &xmax, &ymax );

    sprintf( buf, "%f", xmin );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", ymin );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", xmax );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+2], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", ymax );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+3], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * pldip2dcCmd
 *
 * Processes pldip2dc Tcl command.
\*--------------------------------------------------------------------------*/

static int
pldip2dcCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xmin;
    PLFLT ymin;
    PLFLT xmax;
    PLFLT ymax;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "pldip2dc ?xmin ymin xmax ymax?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 4 - 0))) ||
         (!1 && !0 && (argc != (4 + 1))) ||
         ( 1 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "pldip2dc ?xmin ymin xmax ymax?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* xmin is for output. */
/* ymin is for output. */
/* xmax is for output. */
/* ymax is for output. */

    pldip2dc ( &xmin, &ymin, &xmax, &ymax );

    sprintf( buf, "%f", xmin );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", ymin );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", xmax );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+2], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", ymax );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+3], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plendCmd
 *
 * Processes plend Tcl command.
\*--------------------------------------------------------------------------*/

static int
plendCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plend ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 0 - 0))) ||
         (!0 && !0 && (argc != (0 + 1))) ||
         ( 0 && (argc != 1) && (argc != (0 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plend ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }


    plend (  );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plend1Cmd
 *
 * Processes plend1 Tcl command.
\*--------------------------------------------------------------------------*/

static int
plend1Cmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plend1 ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 0 - 0))) ||
         (!0 && !0 && (argc != (0 + 1))) ||
         ( 0 && (argc != 1) && (argc != (0 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plend1 ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }


    plend1 (  );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plenvCmd
 *
 * Processes plenv Tcl command.
\*--------------------------------------------------------------------------*/

static int
plenvCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xmin;
    PLFLT xmax;
    PLFLT ymin;
    PLFLT ymax;
    PLINT just;
    PLINT axis;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plenv xmin xmax ymin ymax just axis", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 6 - 0))) ||
         (!0 && !0 && (argc != (6 + 1))) ||
         ( 0 && (argc != 1) && (argc != (6 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plenv xmin xmax ymin ymax just axis", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    xmin = atof(argv[1+0]);
    xmax = atof(argv[1+1]);
    ymin = atof(argv[1+2]);
    ymax = atof(argv[1+3]);
    just = atoi(argv[1+4]);
    axis = atoi(argv[1+5]);

    plenv ( xmin, xmax, ymin, ymax, just, axis );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * pleopCmd
 *
 * Processes pleop Tcl command.
\*--------------------------------------------------------------------------*/

static int
pleopCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "pleop ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 0 - 0))) ||
         (!0 && !0 && (argc != (0 + 1))) ||
         ( 0 && (argc != 1) && (argc != (0 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "pleop ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }


    pleop (  );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plerrxCmd
 *
 * Processes plerrx Tcl command.
\*--------------------------------------------------------------------------*/

static int
plerrxCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT n;
    PLFLT *xmin;
    tclMatrix *matxmin;
    PLFLT *xmax;
    tclMatrix *matxmax;
    PLFLT *y;
    tclMatrix *maty;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plerrx n xmin xmax y", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plerrx n xmin xmax y", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    n = atoi(argv[1+0]);
    matxmin = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matxmin == NULL) return TCL_ERROR;
    xmin = matxmin->fdata;
    matxmax = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (matxmax == NULL) return TCL_ERROR;
    xmax = matxmax->fdata;
    maty = Tcl_GetMatrixPtr( interp, argv[1+3] );
    if (maty == NULL) return TCL_ERROR;
    y = maty->fdata;

    plerrx ( n, xmin, xmax, y );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plerryCmd
 *
 * Processes plerry Tcl command.
\*--------------------------------------------------------------------------*/

static int
plerryCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT n;
    PLFLT *x;
    tclMatrix *matx;
    PLFLT *ymin;
    tclMatrix *matymin;
    PLFLT *ymax;
    tclMatrix *matymax;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plerry n x ymin ymax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plerry n x ymin ymax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    n = atoi(argv[1+0]);
    matx = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matx == NULL) return TCL_ERROR;
    x = matx->fdata;
    matymin = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (matymin == NULL) return TCL_ERROR;
    ymin = matymin->fdata;
    matymax = Tcl_GetMatrixPtr( interp, argv[1+3] );
    if (matymax == NULL) return TCL_ERROR;
    ymax = matymax->fdata;

    plerry ( n, x, ymin, ymax );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plfamadvCmd
 *
 * Processes plfamadv Tcl command.
\*--------------------------------------------------------------------------*/

static int
plfamadvCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plfamadv ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 0 - 0))) ||
         (!0 && !0 && (argc != (0 + 1))) ||
         ( 0 && (argc != 1) && (argc != (0 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plfamadv ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }


    plfamadv (  );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plfillCmd
 *
 * Processes plfill Tcl command.
\*--------------------------------------------------------------------------*/

static int
plfillCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT n;
    PLFLT *x;
    tclMatrix *matx;
    PLFLT *y;
    tclMatrix *maty;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plfill n x y", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 3 - 0))) ||
         (!0 && !0 && (argc != (3 + 1))) ||
         ( 0 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plfill n x y", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    n = atoi(argv[1+0]);
    matx = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matx == NULL) return TCL_ERROR;
    x = matx->fdata;
    maty = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (maty == NULL) return TCL_ERROR;
    y = maty->fdata;

    plfill ( n, x, y );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plflushCmd
 *
 * Processes plflush Tcl command.
\*--------------------------------------------------------------------------*/

static int
plflushCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plflush ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 0 - 0))) ||
         (!0 && !0 && (argc != (0 + 1))) ||
         ( 0 && (argc != 1) && (argc != (0 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plflush ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }


    plflush (  );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plfontCmd
 *
 * Processes plfont Tcl command.
\*--------------------------------------------------------------------------*/

static int
plfontCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT ifont;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plfont ifont", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plfont ifont", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    ifont = atoi(argv[1+0]);

    plfont ( ifont );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plfontldCmd
 *
 * Processes plfontld Tcl command.
\*--------------------------------------------------------------------------*/

static int
plfontldCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT fnt;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plfontld fnt", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plfontld fnt", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    fnt = atoi(argv[1+0]);

    plfontld ( fnt );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgchrCmd
 *
 * Processes plgchr Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgchrCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT def;
    PLFLT ht;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgchr ?def ht?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 2 - 0))) ||
         (!1 && !0 && (argc != (2 + 1))) ||
         ( 1 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgchr ?def ht?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* def is for output. */
/* ht is for output. */

    plgchr ( &def, &ht );

    sprintf( buf, "%f", def );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", ht );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgcol0Cmd
 *
 * Processes plgcol0 Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgcol0Cmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT icol0;
    PLINT r;
    PLINT g;
    PLINT b;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgcol0 ?icol0 r g b?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 4 - 0))) ||
         (!1 && !0 && (argc != (4 + 1))) ||
         ( 1 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgcol0 ?icol0 r g b?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    icol0 = atoi(argv[1+0]);
/* r is for output. */
/* g is for output. */
/* b is for output. */

    plgcol0 ( icol0, &r, &g, &b );

    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", r );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", g );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+2], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", b );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+3], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgcolbgCmd
 *
 * Processes plgcolbg Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgcolbgCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT r;
    PLINT g;
    PLINT b;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgcolbg ?r g b?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 3 - 0))) ||
         (!1 && !0 && (argc != (3 + 1))) ||
         ( 1 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgcolbg ?r g b?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* r is for output. */
/* g is for output. */
/* b is for output. */

    plgcolbg ( &r, &g, &b );

    sprintf( buf, "%d", r );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", g );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", b );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+2], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgdevCmd
 *
 * Processes plgdev Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgdevCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    char devnam[200];

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgdev ?devnam?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 1 - 0))) ||
         (!1 && !0 && (argc != (1 + 1))) ||
         ( 1 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgdev ?devnam?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* devnam is for output. */

    plgdev ( devnam );

    if (argc > 1)
       Tcl_SetVar( interp, argv[1+0], devnam, 0 );
    else
        Tcl_AppendResult( interp, devnam, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgdidevCmd
 *
 * Processes plgdidev Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgdidevCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT mar;
    PLFLT aspect;
    PLFLT jx;
    PLFLT jy;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgdidev ?mar aspect jx jy?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 4 - 0))) ||
         (!1 && !0 && (argc != (4 + 1))) ||
         ( 1 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgdidev ?mar aspect jx jy?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* mar is for output. */
/* aspect is for output. */
/* jx is for output. */
/* jy is for output. */

    plgdidev ( &mar, &aspect, &jx, &jy );

    sprintf( buf, "%f", mar );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", aspect );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", jx );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+2], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", jy );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+3], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgdioriCmd
 *
 * Processes plgdiori Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgdioriCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT rot;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgdiori ?rot?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 1 - 0))) ||
         (!1 && !0 && (argc != (1 + 1))) ||
         ( 1 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgdiori ?rot?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* rot is for output. */

    plgdiori ( &rot );

    sprintf( buf, "%f", rot );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgdipltCmd
 *
 * Processes plgdiplt Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgdipltCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xmin;
    PLFLT ymin;
    PLFLT xmax;
    PLFLT ymax;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgdiplt ?xmin ymin xmax ymax?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 4 - 0))) ||
         (!1 && !0 && (argc != (4 + 1))) ||
         ( 1 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgdiplt ?xmin ymin xmax ymax?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* xmin is for output. */
/* ymin is for output. */
/* xmax is for output. */
/* ymax is for output. */

    plgdiplt ( &xmin, &ymin, &xmax, &ymax );

    sprintf( buf, "%f", xmin );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", ymin );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", xmax );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+2], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", ymax );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+3], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgescCmd
 *
 * Processes plgesc Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgescCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    char esc;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgesc ?esc?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 1 - 0))) ||
         (!1 && !0 && (argc != (1 + 1))) ||
         ( 1 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgesc ?esc?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* esc is for output. */

    plgesc ( &esc );

    sprintf( buf, "%c", esc );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgfamCmd
 *
 * Processes plgfam Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgfamCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT fam;
    PLINT num;
    PLINT bmax;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgfam ?fam num bmax?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 3 - 0))) ||
         (!1 && !0 && (argc != (3 + 1))) ||
         ( 1 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgfam ?fam num bmax?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* fam is for output. */
/* num is for output. */
/* bmax is for output. */

    plgfam ( &fam, &num, &bmax );

    sprintf( buf, "%d", fam );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", num );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", bmax );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+2], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgfnamCmd
 *
 * Processes plgfnam Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgfnamCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    char fnam[200];

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgfnam ?fnam?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 1 - 0))) ||
         (!1 && !0 && (argc != (1 + 1))) ||
         ( 1 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgfnam ?fnam?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* fnam is for output. */

    plgfnam ( fnam );

    if (argc > 1)
       Tcl_SetVar( interp, argv[1+0], fnam, 0 );
    else
        Tcl_AppendResult( interp, fnam, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plglevelCmd
 *
 * Processes plglevel Tcl command.
\*--------------------------------------------------------------------------*/

static int
plglevelCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT level;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plglevel ?level?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 1 - 0))) ||
         (!1 && !0 && (argc != (1 + 1))) ||
         ( 1 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plglevel ?level?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* level is for output. */

    plglevel ( &level );

    sprintf( buf, "%d", level );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgpageCmd
 *
 * Processes plgpage Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgpageCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xp;
    PLFLT yp;
    PLINT xleng;
    PLINT yleng;
    PLINT xoff;
    PLINT yoff;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgpage ?xp yp xleng yleng xoff yoff?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 6 - 0))) ||
         (!1 && !0 && (argc != (6 + 1))) ||
         ( 1 && (argc != 1) && (argc != (6 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgpage ?xp yp xleng yleng xoff yoff?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* xp is for output. */
/* yp is for output. */
/* xleng is for output. */
/* yleng is for output. */
/* xoff is for output. */
/* yoff is for output. */

    plgpage ( &xp, &yp, &xleng, &yleng, &xoff, &yoff );

    sprintf( buf, "%f", xp );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", yp );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", xleng );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+2], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", yleng );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+3], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", xoff );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+4], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", yoff );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+5], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgraCmd
 *
 * Processes plgra Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgraCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgra ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 0 - 0))) ||
         (!0 && !0 && (argc != (0 + 1))) ||
         ( 0 && (argc != 1) && (argc != (0 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgra ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }


    plgra (  );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgspaCmd
 *
 * Processes plgspa Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgspaCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xmin;
    PLFLT xmax;
    PLFLT ymin;
    PLFLT ymax;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgspa ?xmin xmax ymin ymax?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 4 - 0))) ||
         (!1 && !0 && (argc != (4 + 1))) ||
         ( 1 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgspa ?xmin xmax ymin ymax?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* xmin is for output. */
/* xmax is for output. */
/* ymin is for output. */
/* ymax is for output. */

    plgspa ( &xmin, &xmax, &ymin, &ymax );

    sprintf( buf, "%f", xmin );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", xmax );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", ymin );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+2], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%f", ymax );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+3], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgstrmCmd
 *
 * Processes plgstrm Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgstrmCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT strm;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgstrm ?strm?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 1 - 0))) ||
         (!1 && !0 && (argc != (1 + 1))) ||
         ( 1 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgstrm ?strm?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* strm is for output. */

    plgstrm ( &strm );

    sprintf( buf, "%d", strm );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgverCmd
 *
 * Processes plgver Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgverCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    char ver[200];

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgver ?ver?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 1 - 0))) ||
         (!1 && !0 && (argc != (1 + 1))) ||
         ( 1 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgver ?ver?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* ver is for output. */

    plgver ( ver );

    if (argc > 1)
       Tcl_SetVar( interp, argv[1+0], ver, 0 );
    else
        Tcl_AppendResult( interp, ver, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgxaxCmd
 *
 * Processes plgxax Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgxaxCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT digmax;
    PLINT digits;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgxax ?digmax digits?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 2 - 0))) ||
         (!1 && !0 && (argc != (2 + 1))) ||
         ( 1 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgxax ?digmax digits?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* digmax is for output. */
/* digits is for output. */

    plgxax ( &digmax, &digits );

    sprintf( buf, "%d", digmax );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", digits );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgyaxCmd
 *
 * Processes plgyax Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgyaxCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT digmax;
    PLINT digits;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgyax ?digmax digits?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 2 - 0))) ||
         (!1 && !0 && (argc != (2 + 1))) ||
         ( 1 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgyax ?digmax digits?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* digmax is for output. */
/* digits is for output. */

    plgyax ( &digmax, &digits );

    sprintf( buf, "%d", digmax );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", digits );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plgzaxCmd
 *
 * Processes plgzax Tcl command.
\*--------------------------------------------------------------------------*/

static int
plgzaxCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT digmax;
    PLINT digits;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plgzax ?digmax digits?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 2 - 0))) ||
         (!1 && !0 && (argc != (2 + 1))) ||
         ( 1 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plgzax ?digmax digits?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* digmax is for output. */
/* digits is for output. */

    plgzax ( &digmax, &digits );

    sprintf( buf, "%d", digmax );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );
    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", digits );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plhistCmd
 *
 * Processes plhist Tcl command.
\*--------------------------------------------------------------------------*/

static int
plhistCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT n;
    PLFLT *data;
    tclMatrix *matdata;
    PLFLT datmin;
    PLFLT datmax;
    PLINT nbin;
    PLINT oldwin;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plhist n data datmin datmax nbin oldwin", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 6 - 0))) ||
         (!0 && !0 && (argc != (6 + 1))) ||
         ( 0 && (argc != 1) && (argc != (6 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plhist n data datmin datmax nbin oldwin", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    n = atoi(argv[1+0]);
    matdata = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matdata == NULL) return TCL_ERROR;
    data = matdata->fdata;
    datmin = atof(argv[1+2]);
    datmax = atof(argv[1+3]);
    nbin = atoi(argv[1+4]);
    oldwin = atoi(argv[1+5]);

    plhist ( n, data, datmin, datmax, nbin, oldwin );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plhlsCmd
 *
 * Processes plhls Tcl command.
\*--------------------------------------------------------------------------*/

static int
plhlsCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT h;
    PLFLT l;
    PLFLT s;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plhls h l s", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 3 - 0))) ||
         (!0 && !0 && (argc != (3 + 1))) ||
         ( 0 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plhls h l s", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    h = atof(argv[1+0]);
    l = atof(argv[1+1]);
    s = atof(argv[1+2]);

    plhls ( h, l, s );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plinitCmd
 *
 * Processes plinit Tcl command.
\*--------------------------------------------------------------------------*/

static int
plinitCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plinit ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 0 - 0))) ||
         (!0 && !0 && (argc != (0 + 1))) ||
         ( 0 && (argc != 1) && (argc != (0 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plinit ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }


    plinit (  );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * pljoinCmd
 *
 * Processes pljoin Tcl command.
\*--------------------------------------------------------------------------*/

static int
pljoinCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT x1;
    PLFLT y1;
    PLFLT x2;
    PLFLT y2;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "pljoin x1 y1 x2 y2", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "pljoin x1 y1 x2 y2", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    x1 = atof(argv[1+0]);
    y1 = atof(argv[1+1]);
    x2 = atof(argv[1+2]);
    y2 = atof(argv[1+3]);

    pljoin ( x1, y1, x2, y2 );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * pllabCmd
 *
 * Processes pllab Tcl command.
\*--------------------------------------------------------------------------*/

static int
pllabCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    char *xlabel;
    char *ylabel;
    char *tlabel;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "pllab xlabel ylabel tlabel", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 3 - 0))) ||
         (!0 && !0 && (argc != (3 + 1))) ||
         ( 0 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "pllab xlabel ylabel tlabel", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    xlabel = argv[1+0];
    ylabel = argv[1+1];
    tlabel = argv[1+2];

    pllab ( xlabel, ylabel, tlabel );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * pllightsourceCmd
 *
 * Processes pllightsource Tcl command.
\*--------------------------------------------------------------------------*/

static int
pllightsourceCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT x;
    PLFLT y;
    PLFLT z;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "pllightsource x y z", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 3 - 0))) ||
         (!0 && !0 && (argc != (3 + 1))) ||
         ( 0 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "pllightsource x y z", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    x = atof(argv[1+0]);
    y = atof(argv[1+1]);
    z = atof(argv[1+2]);

    pllightsource ( x, y, z );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * pllineCmd
 *
 * Processes plline Tcl command.
\*--------------------------------------------------------------------------*/

static int
pllineCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT n;
    PLFLT *x;
    tclMatrix *matx;
    PLFLT *y;
    tclMatrix *maty;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plline n x y", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 3 - 0))) ||
         (!0 && !0 && (argc != (3 + 1))) ||
         ( 0 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plline n x y", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    n = atoi(argv[1+0]);
    matx = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matx == NULL) return TCL_ERROR;
    x = matx->fdata;
    maty = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (maty == NULL) return TCL_ERROR;
    y = maty->fdata;

    plline ( n, x, y );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plline3Cmd
 *
 * Processes plline3 Tcl command.
\*--------------------------------------------------------------------------*/

static int
plline3Cmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT n;
    PLFLT *x;
    tclMatrix *matx;
    PLFLT *y;
    tclMatrix *maty;
    PLFLT *z;
    tclMatrix *matz;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plline3 n x y z", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plline3 n x y z", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    n = atoi(argv[1+0]);
    matx = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matx == NULL) return TCL_ERROR;
    x = matx->fdata;
    maty = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (maty == NULL) return TCL_ERROR;
    y = maty->fdata;
    matz = Tcl_GetMatrixPtr( interp, argv[1+3] );
    if (matz == NULL) return TCL_ERROR;
    z = matz->fdata;

    plline3 ( n, x, y, z );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * pllstyCmd
 *
 * Processes pllsty Tcl command.
\*--------------------------------------------------------------------------*/

static int
pllstyCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT lin;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "pllsty lin", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "pllsty lin", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    lin = atoi(argv[1+0]);

    pllsty ( lin );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plmkstrmCmd
 *
 * Processes plmkstrm Tcl command.
\*--------------------------------------------------------------------------*/

static int
plmkstrmCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT strm;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plmkstrm ?strm?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 1 - 0))) ||
         (!1 && !0 && (argc != (1 + 1))) ||
         ( 1 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plmkstrm ?strm?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

/* strm is for output. */

    plmkstrm ( &strm );

    sprintf( buf, "%d", strm );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+0], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plmtexCmd
 *
 * Processes plmtex Tcl command.
\*--------------------------------------------------------------------------*/

static int
plmtexCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    char *side;
    PLFLT disp;
    PLFLT pos;
    PLFLT just;
    char *text;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plmtex side disp pos just text", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 5 - 0))) ||
         (!0 && !0 && (argc != (5 + 1))) ||
         ( 0 && (argc != 1) && (argc != (5 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plmtex side disp pos just text", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    side = argv[1+0];
    disp = atof(argv[1+1]);
    pos = atof(argv[1+2]);
    just = atof(argv[1+3]);
    text = argv[1+4];

    plmtex ( side, disp, pos, just, text );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plpatCmd
 *
 * Processes plpat Tcl command.
\*--------------------------------------------------------------------------*/

static int
plpatCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT nlin;
    PLINT *inc;
    tclMatrix *matinc;
    PLINT *del;
    tclMatrix *matdel;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plpat nlin inc del", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 3 - 0))) ||
         (!0 && !0 && (argc != (3 + 1))) ||
         ( 0 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plpat nlin inc del", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    nlin = atoi(argv[1+0]);
    matinc = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matinc == NULL) return TCL_ERROR;
    inc = matinc->idata;
    matdel = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (matdel == NULL) return TCL_ERROR;
    del = matdel->idata;

    plpat ( nlin, inc, del );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plpoinCmd
 *
 * Processes plpoin Tcl command.
\*--------------------------------------------------------------------------*/

static int
plpoinCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT n;
    PLFLT *x;
    tclMatrix *matx;
    PLFLT *y;
    tclMatrix *maty;
    PLINT code;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plpoin n x y code", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plpoin n x y code", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    n = atoi(argv[1+0]);
    matx = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matx == NULL) return TCL_ERROR;
    x = matx->fdata;
    maty = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (maty == NULL) return TCL_ERROR;
    y = maty->fdata;
    code = atoi(argv[1+3]);

    plpoin ( n, x, y, code );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plpoin3Cmd
 *
 * Processes plpoin3 Tcl command.
\*--------------------------------------------------------------------------*/

static int
plpoin3Cmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT n;
    PLFLT *x;
    tclMatrix *matx;
    PLFLT *y;
    tclMatrix *maty;
    PLFLT *z;
    tclMatrix *matz;
    PLINT code;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plpoin3 n x y z code", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 5 - 0))) ||
         (!0 && !0 && (argc != (5 + 1))) ||
         ( 0 && (argc != 1) && (argc != (5 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plpoin3 n x y z code", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    n = atoi(argv[1+0]);
    matx = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matx == NULL) return TCL_ERROR;
    x = matx->fdata;
    maty = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (maty == NULL) return TCL_ERROR;
    y = maty->fdata;
    matz = Tcl_GetMatrixPtr( interp, argv[1+3] );
    if (matz == NULL) return TCL_ERROR;
    z = matz->fdata;
    code = atoi(argv[1+4]);

    plpoin3 ( n, x, y, z, code );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plpoly3Cmd
 *
 * Processes plpoly3 Tcl command.
\*--------------------------------------------------------------------------*/

static int
plpoly3Cmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT n;
    PLFLT *x;
    tclMatrix *matx;
    PLFLT *y;
    tclMatrix *maty;
    PLFLT *z;
    tclMatrix *matz;
    PLINT *draw;
    tclMatrix *matdraw;
    PLINT ifcc;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plpoly3 n x y z draw ifcc", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 6 - 0))) ||
         (!0 && !0 && (argc != (6 + 1))) ||
         ( 0 && (argc != 1) && (argc != (6 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plpoly3 n x y z draw ifcc", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    n = atoi(argv[1+0]);
    matx = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matx == NULL) return TCL_ERROR;
    x = matx->fdata;
    maty = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (maty == NULL) return TCL_ERROR;
    y = maty->fdata;
    matz = Tcl_GetMatrixPtr( interp, argv[1+3] );
    if (matz == NULL) return TCL_ERROR;
    z = matz->fdata;
    matdraw = Tcl_GetMatrixPtr( interp, argv[1+4] );
    if (matdraw == NULL) return TCL_ERROR;
    draw = matdraw->idata;
    ifcc = atoi(argv[1+5]);

    plpoly3 ( n, x, y, z, draw, ifcc );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plprecCmd
 *
 * Processes plprec Tcl command.
\*--------------------------------------------------------------------------*/

static int
plprecCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT setp;
    PLINT prec;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plprec setp prec", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 2 - 0))) ||
         (!0 && !0 && (argc != (2 + 1))) ||
         ( 0 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plprec setp prec", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    setp = atoi(argv[1+0]);
    prec = atoi(argv[1+1]);

    plprec ( setp, prec );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plpstyCmd
 *
 * Processes plpsty Tcl command.
\*--------------------------------------------------------------------------*/

static int
plpstyCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT patt;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plpsty patt", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plpsty patt", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    patt = atoi(argv[1+0]);

    plpsty ( patt );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plptexCmd
 *
 * Processes plptex Tcl command.
\*--------------------------------------------------------------------------*/

static int
plptexCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT x;
    PLFLT y;
    PLFLT dx;
    PLFLT dy;
    PLFLT just;
    char *text;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plptex x y dx dy just text", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 6 - 0))) ||
         (!0 && !0 && (argc != (6 + 1))) ||
         ( 0 && (argc != 1) && (argc != (6 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plptex x y dx dy just text", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    x = atof(argv[1+0]);
    y = atof(argv[1+1]);
    dx = atof(argv[1+2]);
    dy = atof(argv[1+3]);
    just = atof(argv[1+4]);
    text = argv[1+5];

    plptex ( x, y, dx, dy, just, text );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plreplotCmd
 *
 * Processes plreplot Tcl command.
\*--------------------------------------------------------------------------*/

static int
plreplotCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plreplot ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 0 - 0))) ||
         (!0 && !0 && (argc != (0 + 1))) ||
         ( 0 && (argc != 1) && (argc != (0 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plreplot ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }


    plreplot (  );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plrgbCmd
 *
 * Processes plrgb Tcl command.
\*--------------------------------------------------------------------------*/

static int
plrgbCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT r;
    PLFLT g;
    PLFLT b;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plrgb r g b", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 3 - 0))) ||
         (!0 && !0 && (argc != (3 + 1))) ||
         ( 0 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plrgb r g b", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    r = atof(argv[1+0]);
    g = atof(argv[1+1]);
    b = atof(argv[1+2]);

    plrgb ( r, g, b );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plrgb1Cmd
 *
 * Processes plrgb1 Tcl command.
\*--------------------------------------------------------------------------*/

static int
plrgb1Cmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT r;
    PLINT g;
    PLINT b;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plrgb1 r g b", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 3 - 0))) ||
         (!0 && !0 && (argc != (3 + 1))) ||
         ( 0 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plrgb1 r g b", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    r = atoi(argv[1+0]);
    g = atoi(argv[1+1]);
    b = atoi(argv[1+2]);

    plrgb1 ( r, g, b );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plschrCmd
 *
 * Processes plschr Tcl command.
\*--------------------------------------------------------------------------*/

static int
plschrCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT def;
    PLFLT scale;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plschr def scale", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 2 - 0))) ||
         (!0 && !0 && (argc != (2 + 1))) ||
         ( 0 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plschr def scale", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    def = atof(argv[1+0]);
    scale = atof(argv[1+1]);

    plschr ( def, scale );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plscmap0Cmd
 *
 * Processes plscmap0 Tcl command.
\*--------------------------------------------------------------------------*/

static int
plscmap0Cmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT *r;
    tclMatrix *matr;
    PLINT *g;
    tclMatrix *matg;
    PLINT *b;
    tclMatrix *matb;
    PLINT ncol0;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plscmap0 r g b ncol0", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plscmap0 r g b ncol0", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    matr = Tcl_GetMatrixPtr( interp, argv[1+0] );
    if (matr == NULL) return TCL_ERROR;
    r = matr->idata;
    matg = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matg == NULL) return TCL_ERROR;
    g = matg->idata;
    matb = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (matb == NULL) return TCL_ERROR;
    b = matb->idata;
    ncol0 = atoi(argv[1+3]);

    plscmap0 ( r, g, b, ncol0 );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plscmap0nCmd
 *
 * Processes plscmap0n Tcl command.
\*--------------------------------------------------------------------------*/

static int
plscmap0nCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT ncol0;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plscmap0n ncol0", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plscmap0n ncol0", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    ncol0 = atoi(argv[1+0]);

    plscmap0n ( ncol0 );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plscmap1Cmd
 *
 * Processes plscmap1 Tcl command.
\*--------------------------------------------------------------------------*/

static int
plscmap1Cmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT *r;
    tclMatrix *matr;
    PLINT *g;
    tclMatrix *matg;
    PLINT *b;
    tclMatrix *matb;
    PLINT ncol1;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plscmap1 r g b ncol1", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plscmap1 r g b ncol1", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    matr = Tcl_GetMatrixPtr( interp, argv[1+0] );
    if (matr == NULL) return TCL_ERROR;
    r = matr->idata;
    matg = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matg == NULL) return TCL_ERROR;
    g = matg->idata;
    matb = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (matb == NULL) return TCL_ERROR;
    b = matb->idata;
    ncol1 = atoi(argv[1+3]);

    plscmap1 ( r, g, b, ncol1 );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plscmap1lCmd
 *
 * Processes plscmap1l Tcl command.
\*--------------------------------------------------------------------------*/

static int
plscmap1lCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT itype;
    PLINT npts;
    PLFLT *intensity;
    tclMatrix *matintensity;
    PLFLT *coord1;
    tclMatrix *matcoord1;
    PLFLT *coord2;
    tclMatrix *matcoord2;
    PLFLT *coord3;
    tclMatrix *matcoord3;
    PLINT *rev;
    tclMatrix *matrev;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plscmap1l itype npts intensity coord1 coord2 coord3 rev", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 7 - 0))) ||
         (!0 && !0 && (argc != (7 + 1))) ||
         ( 0 && (argc != 1) && (argc != (7 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plscmap1l itype npts intensity coord1 coord2 coord3 rev", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    itype = atoi(argv[1+0]);
    npts = atoi(argv[1+1]);
    matintensity = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (matintensity == NULL) return TCL_ERROR;
    intensity = matintensity->fdata;
    matcoord1 = Tcl_GetMatrixPtr( interp, argv[1+3] );
    if (matcoord1 == NULL) return TCL_ERROR;
    coord1 = matcoord1->fdata;
    matcoord2 = Tcl_GetMatrixPtr( interp, argv[1+4] );
    if (matcoord2 == NULL) return TCL_ERROR;
    coord2 = matcoord2->fdata;
    matcoord3 = Tcl_GetMatrixPtr( interp, argv[1+5] );
    if (matcoord3 == NULL) return TCL_ERROR;
    coord3 = matcoord3->fdata;
    matrev = Tcl_GetMatrixPtr( interp, argv[1+6] );
    if (matrev == NULL) return TCL_ERROR;
    rev = matrev->idata;

    plscmap1l ( itype, npts, intensity, coord1, coord2, coord3, rev );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plscmap1nCmd
 *
 * Processes plscmap1n Tcl command.
\*--------------------------------------------------------------------------*/

static int
plscmap1nCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT ncol1;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plscmap1n ncol1", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plscmap1n ncol1", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    ncol1 = atoi(argv[1+0]);

    plscmap1n ( ncol1 );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plscol0Cmd
 *
 * Processes plscol0 Tcl command.
\*--------------------------------------------------------------------------*/

static int
plscol0Cmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT icol0;
    PLINT r;
    PLINT g;
    PLINT b;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plscol0 icol0 r g b", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plscol0 icol0 r g b", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    icol0 = atoi(argv[1+0]);
    r = atoi(argv[1+1]);
    g = atoi(argv[1+2]);
    b = atoi(argv[1+3]);

    plscol0 ( icol0, r, g, b );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plscolbgCmd
 *
 * Processes plscolbg Tcl command.
\*--------------------------------------------------------------------------*/

static int
plscolbgCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT r;
    PLINT g;
    PLINT b;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plscolbg r g b", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 3 - 0))) ||
         (!0 && !0 && (argc != (3 + 1))) ||
         ( 0 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plscolbg r g b", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    r = atoi(argv[1+0]);
    g = atoi(argv[1+1]);
    b = atoi(argv[1+2]);

    plscolbg ( r, g, b );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plscolorCmd
 *
 * Processes plscolor Tcl command.
\*--------------------------------------------------------------------------*/

static int
plscolorCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT color;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plscolor color", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plscolor color", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    color = atoi(argv[1+0]);

    plscolor ( color );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsdevCmd
 *
 * Processes plsdev Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsdevCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    char *devnam;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsdev devnam", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsdev devnam", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    devnam = argv[1+0];

    plsdev ( devnam );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsdidevCmd
 *
 * Processes plsdidev Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsdidevCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT mar;
    PLFLT aspect;
    PLFLT jx;
    PLFLT jy;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsdidev mar aspect jx jy", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsdidev mar aspect jx jy", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    mar = atof(argv[1+0]);
    aspect = atof(argv[1+1]);
    jx = atof(argv[1+2]);
    jy = atof(argv[1+3]);

    plsdidev ( mar, aspect, jx, jy );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsdimapCmd
 *
 * Processes plsdimap Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsdimapCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT dimxmin;
    PLINT dimxmax;
    PLINT dimymin;
    PLINT dimymax;
    PLFLT dimxpmm;
    PLFLT dimypmm;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsdimap dimxmin dimxmax dimymin dimymax dimxpmm dimypmm", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 6 - 0))) ||
         (!0 && !0 && (argc != (6 + 1))) ||
         ( 0 && (argc != 1) && (argc != (6 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsdimap dimxmin dimxmax dimymin dimymax dimxpmm dimypmm", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    dimxmin = atoi(argv[1+0]);
    dimxmax = atoi(argv[1+1]);
    dimymin = atoi(argv[1+2]);
    dimymax = atoi(argv[1+3]);
    dimxpmm = atof(argv[1+4]);
    dimypmm = atof(argv[1+5]);

    plsdimap ( dimxmin, dimxmax, dimymin, dimymax, dimxpmm, dimypmm );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsdioriCmd
 *
 * Processes plsdiori Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsdioriCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT rot;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsdiori rot", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsdiori rot", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    rot = atof(argv[1+0]);

    plsdiori ( rot );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsdipltCmd
 *
 * Processes plsdiplt Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsdipltCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xmin;
    PLFLT ymin;
    PLFLT xmax;
    PLFLT ymax;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsdiplt xmin ymin xmax ymax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsdiplt xmin ymin xmax ymax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    xmin = atof(argv[1+0]);
    ymin = atof(argv[1+1]);
    xmax = atof(argv[1+2]);
    ymax = atof(argv[1+3]);

    plsdiplt ( xmin, ymin, xmax, ymax );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsdiplzCmd
 *
 * Processes plsdiplz Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsdiplzCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xmin;
    PLFLT ymin;
    PLFLT xmax;
    PLFLT ymax;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsdiplz xmin ymin xmax ymax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsdiplz xmin ymin xmax ymax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    xmin = atof(argv[1+0]);
    ymin = atof(argv[1+1]);
    xmax = atof(argv[1+2]);
    ymax = atof(argv[1+3]);

    plsdiplz ( xmin, ymin, xmax, ymax );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsescCmd
 *
 * Processes plsesc Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsescCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    char esc;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsesc esc", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsesc esc", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    esc = argv[1+0][0];

    plsesc ( esc );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsfamCmd
 *
 * Processes plsfam Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsfamCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT fam;
    PLINT num;
    PLINT bmax;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsfam fam num bmax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 3 - 0))) ||
         (!0 && !0 && (argc != (3 + 1))) ||
         ( 0 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsfam fam num bmax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    fam = atoi(argv[1+0]);
    num = atoi(argv[1+1]);
    bmax = atoi(argv[1+2]);

    plsfam ( fam, num, bmax );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsfnamCmd
 *
 * Processes plsfnam Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsfnamCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    char *fnam;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsfnam fnam", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsfnam fnam", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    fnam = argv[1+0];

    plsfnam ( fnam );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsmajCmd
 *
 * Processes plsmaj Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsmajCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT def;
    PLFLT scale;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsmaj def scale", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 2 - 0))) ||
         (!0 && !0 && (argc != (2 + 1))) ||
         ( 0 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsmaj def scale", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    def = atof(argv[1+0]);
    scale = atof(argv[1+1]);

    plsmaj ( def, scale );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsminCmd
 *
 * Processes plsmin Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsminCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT def;
    PLFLT scale;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsmin def scale", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 2 - 0))) ||
         (!0 && !0 && (argc != (2 + 1))) ||
         ( 0 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsmin def scale", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    def = atof(argv[1+0]);
    scale = atof(argv[1+1]);

    plsmin ( def, scale );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsoriCmd
 *
 * Processes plsori Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsoriCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT ori;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsori ori", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsori ori", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    ori = atoi(argv[1+0]);

    plsori ( ori );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plspageCmd
 *
 * Processes plspage Tcl command.
\*--------------------------------------------------------------------------*/

static int
plspageCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xp;
    PLFLT yp;
    PLINT xleng;
    PLINT yleng;
    PLINT xoff;
    PLINT yoff;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plspage xp yp xleng yleng xoff yoff", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 6 - 0))) ||
         (!0 && !0 && (argc != (6 + 1))) ||
         ( 0 && (argc != 1) && (argc != (6 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plspage xp yp xleng yleng xoff yoff", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    xp = atof(argv[1+0]);
    yp = atof(argv[1+1]);
    xleng = atoi(argv[1+2]);
    yleng = atoi(argv[1+3]);
    xoff = atoi(argv[1+4]);
    yoff = atoi(argv[1+5]);

    plspage ( xp, yp, xleng, yleng, xoff, yoff );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plspauseCmd
 *
 * Processes plspause Tcl command.
\*--------------------------------------------------------------------------*/

static int
plspauseCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT paus;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plspause paus", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plspause paus", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    paus = atoi(argv[1+0]);

    plspause ( paus );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsstrmCmd
 *
 * Processes plsstrm Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsstrmCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT strm;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsstrm strm", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsstrm strm", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    strm = atoi(argv[1+0]);

    plsstrm ( strm );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plssubCmd
 *
 * Processes plssub Tcl command.
\*--------------------------------------------------------------------------*/

static int
plssubCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT nx;
    PLINT ny;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plssub nx ny", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 2 - 0))) ||
         (!0 && !0 && (argc != (2 + 1))) ||
         ( 0 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plssub nx ny", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    nx = atoi(argv[1+0]);
    ny = atoi(argv[1+1]);

    plssub ( nx, ny );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plssymCmd
 *
 * Processes plssym Tcl command.
\*--------------------------------------------------------------------------*/

static int
plssymCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT def;
    PLFLT scale;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plssym def scale", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 2 - 0))) ||
         (!0 && !0 && (argc != (2 + 1))) ||
         ( 0 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plssym def scale", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    def = atof(argv[1+0]);
    scale = atof(argv[1+1]);

    plssym ( def, scale );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plstylCmd
 *
 * Processes plstyl Tcl command.
\*--------------------------------------------------------------------------*/

static int
plstylCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT nms;
    PLINT *mark;
    tclMatrix *matmark;
    PLINT *space;
    tclMatrix *matspace;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plstyl nms mark space", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 3 - 0))) ||
         (!0 && !0 && (argc != (3 + 1))) ||
         ( 0 && (argc != 1) && (argc != (3 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plstyl nms mark space", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    nms = atoi(argv[1+0]);
    matmark = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matmark == NULL) return TCL_ERROR;
    mark = matmark->idata;
    matspace = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (matspace == NULL) return TCL_ERROR;
    space = matspace->idata;

    plstyl ( nms, mark, space );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsvpaCmd
 *
 * Processes plsvpa Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsvpaCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xmin;
    PLFLT xmax;
    PLFLT ymin;
    PLFLT ymax;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsvpa xmin xmax ymin ymax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsvpa xmin xmax ymin ymax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    xmin = atof(argv[1+0]);
    xmax = atof(argv[1+1]);
    ymin = atof(argv[1+2]);
    ymax = atof(argv[1+3]);

    plsvpa ( xmin, xmax, ymin, ymax );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsxaxCmd
 *
 * Processes plsxax Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsxaxCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT digmax;
    PLINT digits;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsxax digmax digits", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 2 - 0))) ||
         (!0 && !0 && (argc != (2 + 1))) ||
         ( 0 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsxax digmax digits", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    digmax = atoi(argv[1+0]);
    digits = atoi(argv[1+1]);

    plsxax ( digmax, digits );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsyaxCmd
 *
 * Processes plsyax Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsyaxCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT digmax = 0;
    PLINT digits = 0;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsyax digmax digits", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 2 && (argc < (1 + 2 - 2))) ||
         (!0 && !2 && (argc != (2 + 1))) ||
         ( 0 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsyax digmax digits", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if (argc > 0+1) {
        digmax = atoi(argv[1+0]);
    }
    if (argc > 1+1) {
        digits = atoi(argv[1+1]);
    }

    plsyax ( digmax, digits );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsymCmd
 *
 * Processes plsym Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsymCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT n;
    PLFLT *x;
    tclMatrix *matx;
    PLFLT *y;
    tclMatrix *maty;
    PLINT code;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plsym n x y code", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plsym n x y code", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    n = atoi(argv[1+0]);
    matx = Tcl_GetMatrixPtr( interp, argv[1+1] );
    if (matx == NULL) return TCL_ERROR;
    x = matx->fdata;
    maty = Tcl_GetMatrixPtr( interp, argv[1+2] );
    if (maty == NULL) return TCL_ERROR;
    y = maty->fdata;
    code = atoi(argv[1+3]);

    plsym ( n, x, y, code );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plszaxCmd
 *
 * Processes plszax Tcl command.
\*--------------------------------------------------------------------------*/

static int
plszaxCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT digmax;
    PLINT digits;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plszax digmax digits", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 2 - 0))) ||
         (!0 && !0 && (argc != (2 + 1))) ||
         ( 0 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plszax digmax digits", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    digmax = atoi(argv[1+0]);
    digits = atoi(argv[1+1]);

    plszax ( digmax, digits );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * pltextCmd
 *
 * Processes pltext Tcl command.
\*--------------------------------------------------------------------------*/

static int
pltextCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "pltext ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 0 - 0))) ||
         (!0 && !0 && (argc != (0 + 1))) ||
         ( 0 && (argc != 1) && (argc != (0 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "pltext ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }


    pltext (  );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plvaspCmd
 *
 * Processes plvasp Tcl command.
\*--------------------------------------------------------------------------*/

static int
plvaspCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT aspect;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plvasp aspect", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plvasp aspect", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    aspect = atof(argv[1+0]);

    plvasp ( aspect );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plvpasCmd
 *
 * Processes plvpas Tcl command.
\*--------------------------------------------------------------------------*/

static int
plvpasCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xmin;
    PLFLT xmax;
    PLFLT ymin;
    PLFLT ymax;
    PLFLT aspect;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plvpas xmin xmax ymin ymax aspect", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 5 - 0))) ||
         (!0 && !0 && (argc != (5 + 1))) ||
         ( 0 && (argc != 1) && (argc != (5 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plvpas xmin xmax ymin ymax aspect", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    xmin = atof(argv[1+0]);
    xmax = atof(argv[1+1]);
    ymin = atof(argv[1+2]);
    ymax = atof(argv[1+3]);
    aspect = atof(argv[1+4]);

    plvpas ( xmin, xmax, ymin, ymax, aspect );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plvporCmd
 *
 * Processes plvpor Tcl command.
\*--------------------------------------------------------------------------*/

static int
plvporCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xmin;
    PLFLT xmax;
    PLFLT ymin;
    PLFLT ymax;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plvpor xmin xmax ymin ymax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plvpor xmin xmax ymin ymax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    xmin = atof(argv[1+0]);
    xmax = atof(argv[1+1]);
    ymin = atof(argv[1+2]);
    ymax = atof(argv[1+3]);

    plvpor ( xmin, xmax, ymin, ymax );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plvstaCmd
 *
 * Processes plvsta Tcl command.
\*--------------------------------------------------------------------------*/

static int
plvstaCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plvsta ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 0 - 0))) ||
         (!0 && !0 && (argc != (0 + 1))) ||
         ( 0 && (argc != 1) && (argc != (0 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plvsta ", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }


    plvsta (  );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plw3dCmd
 *
 * Processes plw3d Tcl command.
\*--------------------------------------------------------------------------*/

static int
plw3dCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT basex;
    PLFLT basey;
    PLFLT height;
    PLFLT xmin0;
    PLFLT xmax0;
    PLFLT ymin0;
    PLFLT ymax0;
    PLFLT zmin0;
    PLFLT zmax0;
    PLFLT alt;
    PLFLT az;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plw3d basex basey height xmin0 xmax0 ymin0 ymax0 zmin0 zmax0 alt az", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 11 - 0))) ||
         (!0 && !0 && (argc != (11 + 1))) ||
         ( 0 && (argc != 1) && (argc != (11 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plw3d basex basey height xmin0 xmax0 ymin0 ymax0 zmin0 zmax0 alt az", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    basex = atof(argv[1+0]);
    basey = atof(argv[1+1]);
    height = atof(argv[1+2]);
    xmin0 = atof(argv[1+3]);
    xmax0 = atof(argv[1+4]);
    ymin0 = atof(argv[1+5]);
    ymax0 = atof(argv[1+6]);
    zmin0 = atof(argv[1+7]);
    zmax0 = atof(argv[1+8]);
    alt = atof(argv[1+9]);
    az = atof(argv[1+10]);

    plw3d ( basex, basey, height, xmin0, xmax0, ymin0, ymax0, zmin0, zmax0, alt, az );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plwidCmd
 *
 * Processes plwid Tcl command.
\*--------------------------------------------------------------------------*/

static int
plwidCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT width;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plwid width", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 1 - 0))) ||
         (!0 && !0 && (argc != (1 + 1))) ||
         ( 0 && (argc != 1) && (argc != (1 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plwid width", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    width = atoi(argv[1+0]);

    plwid ( width );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plwindCmd
 *
 * Processes plwind Tcl command.
\*--------------------------------------------------------------------------*/

static int
plwindCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLFLT xmin;
    PLFLT xmax;
    PLFLT ymin;
    PLFLT ymax;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plwind xmin xmax ymin ymax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!0 && 0 && (argc < (1 + 4 - 0))) ||
         (!0 && !0 && (argc != (4 + 1))) ||
         ( 0 && (argc != 1) && (argc != (4 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plwind xmin xmax ymin ymax", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    xmin = atof(argv[1+0]);
    xmax = atof(argv[1+1]);
    ymin = atof(argv[1+2]);
    ymax = atof(argv[1+3]);

    plwind ( xmin, xmax, ymin, ymax );


    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plxormodCmd
 *
 * Processes plxormod Tcl command.
\*--------------------------------------------------------------------------*/

static int
plxormodCmd( ClientData clientData, Tcl_Interp *interp, int argc, char *argv[] )
{
    PLINT mod;
    PLINT st;

    errcode = 0; errmsg[0] = '\0';

    if ( (argc == 2) && (strncmp(argv[1],"-help",strlen(argv[1])) == 0) ) {
	Tcl_AppendResult( interp, "command syntax: \"",
			  "plxormod ?mod st?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    if ( (!1 && 0 && (argc < (1 + 2 - 0))) ||
         (!1 && !0 && (argc != (2 + 1))) ||
         ( 1 && (argc != 1) && (argc != (2 + 1))) ) {
	Tcl_AppendResult( interp, "wrong # args: should be \"",
			  "plxormod ?mod st?", "\"",
			  (char *) NULL);
	return TCL_ERROR;
    }

    mod = atoi(argv[1+0]);
/* st is for output. */

    plxormod ( mod, &st );

    if (argc == 1)
        Tcl_AppendResult( interp, " ", (char *) NULL );
    sprintf( buf, "%d", st );
    if (argc > 1)
        Tcl_SetVar( interp, argv[1+1], buf, 0 );
    else
        Tcl_AppendResult( interp, buf, (char *) NULL );

    if (errcode != 0) {
	Tcl_AppendResult(interp, errmsg, (char *) NULL);
	return TCL_ERROR;
    }

    plflush();
    return TCL_OK;
}

