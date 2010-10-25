/* $Id: tcpip.c 3186 2006-02-15 18:17:33Z slbrow $
 *
 * Maurice LeBrun
 * 6-Jan-94
 *
 * Functions to handle a variety of TPC-IP related chores, in particular
 * socket i/o for data transfer to the Tcl-DP driver.  For the latter, the
 * Tcl-DP routines were modified to use binary records; copyright follows:
 *
 * Copyright 1992 Telecom Finland
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that this copyright
 * notice appears in all copies.  Telecom Finland
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * Copyright (c) 1993 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *
 * Copyright (C) 2004  Joao Cardoso
 *
 * This file is part of PLplot.
 *
 * PLplot is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Library Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * PLplot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with PLplot; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
#define DEBUG
*/

#include "plDevs.h"

#if defined (PLD_tk)

/* This file is meant to be compiled with non-ANSI compilers ("cc").
 * The reason for doing it this way is to ensure that the full C
 * environment of the target machine is visible (at the time of writing
 * this, parts of this code are not covered by any international
 * standard).  ANSI compilers are required to omit these extra symbols,
 * and at the moment there is no way to get them back except for by
 * vendor-specific defines, e.g. _HPUX_SOURCE (HP), _ALL_SOURCE (AIX),
 * _DGUX_SOURCE (DGUX).  This is an omission in the POSIX standard more
 * than anything else, and will probably be rectified at some point.  So
 * for now, instead of relying on a hodgepodge of vendor specific symbols
 * I forego the ANSI compiler here and go with good (bad) old "cc".
 */

#include "plConfig.h"
#ifdef _POSIX_SOURCE
#undef _POSIX_SOURCE
#endif
#ifdef caddr_t
#undef caddr_t
#endif
#define PLARGS(a)	()

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#if defined(__sgi) && !defined(SVR3)
#include <sys/select.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "tcpip.h"
#include <tcl.h>
#include <tk.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/uio.h>
#include <errno.h>

extern int errno;

#ifndef MIN
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif

/*
 * This is a "magic number" prepended to the beginning of the packet
 * Used to help resync the packet machanism in the event of errors.
 */
#define PACKET_MAGIC	0x6feeddcc

/*
 * For TCP, it's possible to get a line in pieces.  In case everything we
 * want isn't there, we need a place to store partial results when we're
 * in non-blocking mode.  The partial buffers below are created
 * dynamically to store incomplete data in these cases.
*/

typedef struct PartialRead {
    char *buffer;		/* Buffer of characters */
    int bufSize;		/* Size of buffer */
    int offset;			/* Offset of current character within buffer */
    struct PartialRead *next;	/* Next buffer in chain */
} PartialRead;

#define MAX_OPEN_FILES	128

static PartialRead *partial[MAX_OPEN_FILES];

static void pl_FreeReadBuffer	PLARGS((int fd));
static void pl_Unread		PLARGS((int fd, char *buffer,
					int numBytes, int copy));
static int  pl_Read		PLARGS((int fd, char *buffer, int numReq));

/*
 *--------------------------------------------------------------
 *
 * pl_FreeReadBuffer --
 *
 *	This function is called to free up all the memory associated
 *	with a file once the file is closed.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Any data buffered locally will be lost.
 *
 *--------------------------------------------------------------
 */

static void
pl_FreeReadBuffer(fd)
    int fd;
{
    PartialRead *readList;

    while (partial[fd] != NULL) {
	readList = partial[fd];
	partial[fd] = readList->next;
	free (readList->buffer);
	free (readList);
    }
}

/*
 *--------------------------------------------------------------
 *
 * pl_Unread --
 *
 *	This function puts data back into the read chain on a
 *	file descriptor.  It's basically an extended "ungetc".
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Subsequent calls to pl_Read on the fd will get this data.
 *
 *--------------------------------------------------------------
 */

static void
pl_Unread (fd, buffer, numBytes, copy)
    int fd;                     /* File descriptor */
    char *buffer;               /* Data to unget */
    int numBytes;               /* Number of bytes to unget */
    int copy;			/* Should we copy the data, or use this */
				/* buffer? */
{
    PartialRead *new;

    new = (PartialRead *) malloc (sizeof(PartialRead));
    if (copy) {
	new->buffer = (char *) malloc (numBytes);
	memcpy (new->buffer, buffer, numBytes);
    } else {
	new->buffer = buffer;
    }
    new->bufSize = numBytes;
    new->offset = 0;
    new->next = partial[fd];
    partial[fd] = new;
}

/*
 *--------------------------------------------------------------
 *
 * pl_Read --
 *
 *	This function implements a "read"-like command, but
 *	buffers partial reads.  The semantics are the same as
 *	with read.
 *
 * Results:
 *	Number of bytes read, or -1 on error (with errno set).
 *
 * Side effects:
 *	All available data is read from the file descriptor.
 *
 *--------------------------------------------------------------
 */

static int
pl_Read (fd, buffer, numReq)
    int fd;			/* File descriptor to read from */
    char *buffer;		/* Place to put the data */
    int numReq;			/* Number of bytes to get */
{
    PartialRead *readList;
    PartialRead *tmp;
    int numRead;
    int numToCopy;

    readList = partial[fd];

    /*
     * If there's no data left over from a previous read, then just do a read
     * This is the common case.
     */
    if (readList == NULL) {
	numRead = read(fd, buffer, numReq);
#ifdef DEBUG
	{
	    int j;
	    fprintf(stderr, "received %d bytes starting with:", numRead);
	    for (j = 0; j < MIN(8,numRead); j++)
		fprintf(stderr, " %x", 0x000000FF & (unsigned long) buffer[j]);
	    fprintf(stderr, "\n");
	}
#endif
	return numRead;
    }

    /*
     * There's data left over from a previous read.  Yank it in and
     * only call read() if we didn't get enough data (this keeps the fd
     * readable if they only request as much data as is in the buffers).
     */
    numRead = 0;
    while ((readList != NULL) && (numRead < numReq)) {
	numToCopy = readList->bufSize - readList->offset;
	if (numToCopy + numRead > numReq) {
	    numToCopy = numReq - numRead;
	}
	memcpy (buffer+numRead, readList->buffer+readList->offset, numToCopy);

	/*
	 * Consume the data
	 */
	tmp = readList;
	readList = readList->next;
	tmp->offset += numToCopy;
	if (tmp->offset == tmp->bufSize) {
	    free (tmp->buffer);
	    free (tmp);
	    partial[fd] = readList;
	}
	numRead += numToCopy;
    }

    /*
     * Only call read if at the end of a previously incomplete packet.
     */
    if ((numRead < numReq)) {
	numToCopy = numReq - numRead;
	numRead += read(fd, buffer+numRead, numToCopy);
    }

    return numRead;
}

/*----------------------------------------------------------------------*\
 *  This part for Tcl-DP only
\*----------------------------------------------------------------------*/

#ifdef PLD_dp

#include <dp.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

/*----------------------------------------------------------------------*\
 * plHost_ID
 *
 * Tcl command -- return the IP address for the current host.
 *
 * Derived from source code in "UNIX Network Programming" by W. Richard
 * Stevens, Prentice Hall, 1990.
\*----------------------------------------------------------------------*/

static char *
get_inet(listptr, length)
    char **listptr;
    int length;
{
    struct in_addr	*ptr;

    while ( (ptr = (struct in_addr *) *listptr++) == NULL)
	continue;

    return inet_ntoa(*ptr);
}

int
plHost_ID(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    register struct hostent	*hostptr;
    char			hostname[100];

    if (gethostname(hostname, 100)) {
	Tcl_AppendResult(interp, "Error -- cannot get host name",
			 (char *) NULL);
	return TCL_ERROR;
    }

    if ( (hostptr = gethostbyname(hostname)) == NULL) {
	Tcl_AppendResult(interp, "Error -- cannot get host info for node ",
			 hostname, (char *) NULL);
	return TCL_ERROR;
    }

    Tcl_SetResult(interp,
		  get_inet(hostptr->h_addr_list, hostptr->h_length),
		  TCL_VOLATILE);

    return TCL_OK;
}

#endif	/* PLD_dp */

/*
 *--------------------------------------------------------------
 *
 * pl_PacketReceive --
 *
 *      This procedure is a modified version of Tdp_PacketReceive,
 *	from the Tcl-DP distribution.  It reads the socket,
 *	returning a complete packet.  If the entire packet cannot
 *	be read, the partial packet is buffered until the rest is
 *	available.  Some capabilities have been removed from the
 *	original, such as the check for a non-server TCP socket,
 *	since there's no access to the optFlags array from here,
 *	and the peek capability, since I don't need it.
 *
 * Results:
 *	Packet contents stored in pdfs->buffer and pdfs->bp set
 *	to the number of bytes read (zero if incomplete).
 *
 * Side effects:
 *	The file descriptor passed in is read.
 *
 *--------------------------------------------------------------
 */
int
pl_PacketReceive(interp, iodev, pdfs)
    Tcl_Interp *interp;
    PLiodev *iodev;
    PDFstrm *pdfs;
{
    int j, numRead;
    unsigned int packetLen, header[2];
    int headerSize;
    unsigned char hbuf[8];
    char *errMsg;

    pdfs->bp = 0;

    /*
     * Read in the header (8 bytes)
     */
    headerSize = 8;
    numRead = pl_Read (iodev->fd, (char *) hbuf, headerSize);

    if (numRead <= 0) {
#ifdef DEBUG
	fprintf(stderr, "Incorrect header read, numRead = %d\n", numRead);
#endif
	goto readError;
    }

    /*
     * Check for incomplete read.  If so, put it back and return.
     */
    if (numRead < headerSize) {
#ifdef DEBUG
	fprintf(stderr, "Incomplete header read, numRead = %d\n", numRead);
#endif
	pl_Unread (iodev->fd, (char *) hbuf, numRead, 1);
	Tcl_ResetResult(interp);
	return TCL_OK;
    }

    /*
     * Convert header character stream into ints.  This works when the
     * connecting machine has a different size int and takes care of the
     * endian problem to boot.  It is also mostly backward compatible since
     * network byte ordering (big endian) is used.
     */

    j = 0;

    header[0] = 0;
    header[0] |= hbuf[j++] << 24;
    header[0] |= hbuf[j++] << 16;
    header[0] |= hbuf[j++] << 8;
    header[0] |= hbuf[j++];

    header[1] = 0;
    header[1] |= hbuf[j++] << 24;
    header[1] |= hbuf[j++] << 16;
    header[1] |= hbuf[j++] << 8;
    header[1] |= hbuf[j++];

    /*
     * Format of each packet:
     *
     *		First 4 bytes are PACKET_MAGIC.
     *		Next 4 bytes are packetLen.
     *		Next packetLen-headerSize is zero terminated string
     */
    if (header[0] != PACKET_MAGIC) {
	fprintf(stderr, "Badly formatted packet, numRead = %d\n", numRead);
        Tcl_AppendResult(interp, "Error reading from ", iodev->typeName,
			 ": badly formatted packet", (char *) NULL);
	return TCL_ERROR;
    }
    packetLen = header[1] - headerSize;

    /*
     * Expand the size of the buffer, as needed.
     */

    if (header[1] > (unsigned) pdfs->bufmax) {
	free((void *) pdfs->buffer);
	pdfs->bufmax = header[1] + 32;
	pdfs->buffer = (unsigned char *) malloc(pdfs->bufmax);
    }

    /*
     * Read in the packet, and if it's not all there, put it back.
     *
     * We have to be careful here, because we could block if just the
     * header came in (making the file readable at the beginning of this
     * function) but the rest of the packet is still out on the network.
     */

    if (iodev->type == 0) {
	numRead = pl_Read (iodev->fd, (char *) pdfs->buffer, packetLen);
    } else {
#ifdef PLD_dp
	if (Tdp_FDIsReady(iodev->fd) & TCL_FILE_READABLE) {
	    numRead = pl_Read (iodev->fd, (char *) pdfs->buffer, packetLen);
	} else {
#ifdef DEBUG
	    fprintf(stderr, "Packet not ready, putting back header\n");
#endif
	    pl_Unread (iodev->fd, (char *) hbuf, headerSize, 1);
	    Tcl_ResetResult(interp);
	    return TCL_OK;
	}
#endif
    }

    if (numRead <= 0) {
	goto readError;
    }

    if ((unsigned) numRead != packetLen) {
#ifdef DEBUG
	fprintf(stderr, "Incomplete packet read, numRead = %d\n", numRead);
#endif
	pl_Unread (iodev->fd, (char *) pdfs->buffer, numRead, 1);
	pl_Unread (iodev->fd, (char *) hbuf, headerSize, 1);
	return TCL_OK;
    }

    pdfs->bp = numRead;
#ifdef DEBUG
    fprintf(stderr, "received %d byte packet starting with:", numRead);
    for (j = 0; j < 4; j++) {
	fprintf(stderr, " %x", 0x000000FF & (unsigned long) pdfs->buffer[j]);
    }
    fprintf(stderr, "\n");
#endif

    return TCL_OK;

readError:
    /*
     *
     * If we're in non-blocking mode, and this would block, return.
     * If the connection is closed (numRead == 0), don't return an
     * error message.  Otherwise, return one.
     *
     * In either case, we close the file, delete the file handler, and
     * return a null string.
     */

    if (errno == EWOULDBLOCK || errno == EAGAIN) {
	Tcl_ResetResult(interp);
	return TCL_OK;
    }

    /* Record the error before closing the file */
    if (numRead != 0) {
	errMsg = (char *) Tcl_PosixError (interp);
    } else {
	errMsg = NULL;	/* Suppresses spurious compiler warning */
    }

    /*
     * Remove the file handler and close the file.
     */
    if (iodev->type == 0) {
	Tk_DeleteFileHandler(iodev->fd);
	close(iodev->fd);
    }
    pl_FreeReadBuffer(iodev->fd);

    Tcl_ResetResult(interp);
    if (numRead == 0) {
	return TCL_OK;
    } else {
	Tcl_AppendResult (interp, "pl_PacketReceive -- error reading from ",
			  iodev->typeName, ": ", errMsg, (char *) NULL);
	return TCL_ERROR;
    }
}

/*
 *--------------------------------------------------------------
 *
 * pl_PacketSend --
 *
 *      This procedure is a modified version of Tdp_PacketSend,
 *	from the Tcl-DP distribution.  It writes a complete packet
 *	to a socket or file-oriented device.
 *
 * Results:
 *	A standard tcl result.
 *
 * Side effects:
 *	The specified buffer is written to the file descriptor passed
 *	in.
 *
 *--------------------------------------------------------------
 */

int
pl_PacketSend(interp, iodev, pdfs)
    Tcl_Interp *interp;
    PLiodev *iodev;
    PDFstrm *pdfs;
{
    int j, numSent;
    unsigned char hbuf[8];
    unsigned int packetLen, header[2];
    int len;
    char *buffer, tmp[256];

    /*
     * Format up the packet:
     *	  First 4 bytes are PACKET_MAGIC.
     *	  Next 4 bytes are packetLen.
     *	  Next packetLen-8 bytes are buffer contents.
     */

    packetLen = pdfs->bp + 8;

    header[0] = PACKET_MAGIC;
    header[1] = packetLen;

    /*
     * Convert header ints to character stream.
     * Network byte ordering (big endian) is used.
     */

    j = 0;

    hbuf[j++] = (header[0] & (unsigned long) 0xFF000000) >> 24;
    hbuf[j++] = (header[0] & (unsigned long) 0x00FF0000) >> 16;
    hbuf[j++] = (header[0] & (unsigned long) 0x0000FF00) >> 8;
    hbuf[j++] = (header[0] & (unsigned long) 0x000000FF);

    hbuf[j++] = (header[1] & (unsigned long) 0xFF000000) >> 24;
    hbuf[j++] = (header[1] & (unsigned long) 0x00FF0000) >> 16;
    hbuf[j++] = (header[1] & (unsigned long) 0x0000FF00) >> 8;
    hbuf[j++] = (header[1] & (unsigned long) 0x000000FF);

    /*
     * Send it off, with error checking.
     * Simulate writev using memcpy to put together
     * the msg so it can go out in a single write() call.
     */

    len = pdfs->bp + 8;
    buffer = (char *) malloc(len);

    memcpy(buffer, (char *) hbuf, 8);
    memcpy(buffer + 8, (char *) pdfs->buffer, pdfs->bp);

#ifdef DEBUG
    fprintf(stderr, "sending  %d byte packet starting with:", len);
    for (j = 0; j < 12; j++) {
	fprintf(stderr, " %x", 0x000000FF & (unsigned long) buffer[j]);
    }
    fprintf(stderr, "\n");
#endif
    numSent = write(iodev->fd, buffer, len);

    free(buffer);

    if ((unsigned) numSent != packetLen) {

	if ((errno == 0) || (errno == EWOULDBLOCK || errno == EAGAIN)) {
	    /*
	     * Non-blocking I/O: return number of bytes actually sent.
	     */
	    Tcl_ResetResult(interp);
	    sprintf (tmp, "%d", numSent - 8);
	    Tcl_SetResult(interp, tmp, TCL_VOLATILE);
	    return TCL_OK;
	} else if (errno == EPIPE) {
	    /*
	     * Got a broken pipe signal, which means the far end closed
	     * the connection.  Close the file and return 0 bytes sent.
	     */
	    if (iodev->type == 0) {
		close(iodev->fd);
	    }
	    sprintf (tmp, "0");
	    Tcl_SetResult(interp, tmp, TCL_VOLATILE);
	    return TCL_OK;
	} else {
	    Tcl_AppendResult (interp, "pl_PacketSend -- error writing to ",
			      iodev->typeName, ": ",
			      Tcl_PosixError (interp), (char *) NULL);
	}

	return TCL_ERROR;
    }

    /*
     * Return the number of bytes sent (minus the header).
     */
    sprintf (tmp, "%d", numSent - 8);
    Tcl_SetResult(interp, tmp, TCL_VOLATILE);
    return TCL_OK;
}

#else
int
pldummy_tcpip()
{
    return 0;
}

#endif	/* defined(PLD_tk) */
