C***********************************************************************
C
C  sfstubs.f
C
C  This file contains all the fortran stub routines.
C
C***********************************************************************

	subroutine plsfnam(fnam)

	character*(*) fnam

	parameter (maxlen = 300)
	character*1 string1(maxlen), string2(maxlen), string3(maxlen)
	character*1 string4(maxlen), string5(maxlen), string6(maxlen)
	integer s1(75), s2(75), s3(75), s4(75), s5(75), s6(75)
	equivalence ( s1, string1 ), ( s2, string2 )
	equivalence ( s3, string3 ), ( s4, string4 )
	equivalence ( s5, string5 ), ( s6, string6 )
	common /string/ string1, string2, string3, string4, string5, string6

	limit = min(len_trim(fnam),maxlen-1)
        do i = 1,limit
	   string1(i) = fnam(i:i) 
	enddo
	string1(i) = 0
	
	call plsfnam_(s1)

	end

C***********************************************************************

	subroutine plgfnam(fnam)

	character*(*) fnam

	parameter (maxlen = 300)
	character*1 string1(maxlen), string2(maxlen), string3(maxlen)
	character*1 string4(maxlen), string5(maxlen), string6(maxlen)
	character*300 stringbuf
	integer s1(75), s2(75), s3(75), s4(75), s5(75), s6(75)
	equivalence ( s1, string1 ), ( s2, string2 )
	equivalence ( s3, string3 ), ( s4, string4 )
	equivalence ( s5, string5 ), ( s6, string6 )
	common /string/ string1, string2, string3, string4, string5, string6

	call plgfnam_(s1)
	limit = 1
10	if (ichar(string1(limit)) .eq. 0) goto 20
	stringbuf(limit:limit) = string1(limit)
	limit = limit + 1
	goto 10
	
20	fnam = stringbuf(1:limit-1)

	end

C***********************************************************************

	subroutine plaxes(x0,y0,xopt,xtick,nxsub,yopt,ytick,nysub)

	real	x0, y0, xtick, ytick
	integer nxsub, nysub
	character*(*) xopt,yopt

	parameter (maxlen = 300)
	character*1 string1(maxlen), string2(maxlen), string3(maxlen)
	character*1 string4(maxlen), string5(maxlen), string6(maxlen)
	integer s1(75), s2(75), s3(75), s4(75), s5(75), s6(75)
	equivalence ( s1, string1 ), ( s2, string2 )
	equivalence ( s3, string3 ), ( s4, string4 )
	equivalence ( s5, string5 ), ( s6, string6 )
	common /string/ string1, string2, string3, string4, string5, string6

	limit = min(len_trim(xopt),maxlen-1)
        do i = 1,limit
	   string1(i) = xopt(i:i) 
        enddo
        string1(i) = 0

	limit = min(len_trim(yopt),maxlen-1)
        do i = 1,limit
	   string2(i) = yopt(i:i) 
        enddo
	string2(i) = 0

	call plaxes_(x0,y0,s1,xtick,nxsub,s2,ytick,nysub)

	end

C***********************************************************************

	subroutine plbox(xopt,xtick,nxsub,yopt,ytick,nysub)

	real	xtick, ytick
	integer nxsub, nysub
	character*(*) xopt,yopt

	parameter (maxlen = 300)
	character*1 string1(maxlen), string2(maxlen), string3(maxlen)
	character*1 string4(maxlen), string5(maxlen), string6(maxlen)
	integer s1(75), s2(75), s3(75), s4(75), s5(75), s6(75)
	equivalence ( s1, string1 ), ( s2, string2 )
	equivalence ( s3, string3 ), ( s4, string4 )
	equivalence ( s5, string5 ), ( s6, string6 )
	common /string/ string1, string2, string3, string4, string5, string6

	limit = min(len_trim(xopt),maxlen-1)
        do i = 1,limit
	   string1(i) = xopt(i:i) 
        enddo
        string1(i) = 0

	limit = min(len_trim(yopt),maxlen-1)
        do i = 1,limit
	   string2(i) = yopt(i:i) 
	enddo
        string2(i) = 0

	call plbox_(s1,xtick,nxsub,s2,ytick,nysub)

	end

C***********************************************************************

	subroutine plbox3(xopt,xlabel,xtick,nxsub,yopt,ylabel,ytick,nysub,
     *		    zopt,zlabel,ztick,nzsub)

	real	xtick, ytick, ztick
	character*(*) xopt,xlabel,yopt,ylabel,zopt,zlabel
	integer nxsub, nysub, nzsub

	parameter (maxlen = 300)
	character*1 string1(maxlen), string2(maxlen), string3(maxlen)
	character*1 string4(maxlen), string5(maxlen), string6(maxlen)
	integer s1(75), s2(75), s3(75), s4(75), s5(75), s6(75)
	equivalence ( s1, string1 ), ( s2, string2 )
	equivalence ( s3, string3 ), ( s4, string4 )
	equivalence ( s5, string5 ), ( s6, string6 )
	common /string/ string1, string2, string3, string4, string5, string6
	
c       Convert all those Fortran strings to their C equivalents

	limit = min(len_trim(xopt),maxlen-1)
	do i = 1,limit
	   string1(i) = xopt(i:i)
        enddo
        string1(i) = 0

	limit = min(len_trim(xlabel),maxlen-1)
        do i = 1,limit
	   string2(i) = xlabel(i:i)
	enddo
        string2(i) = 0

	limit = min(len_trim(yopt),maxlen-1)
        do i = 1,limit
	   string3(i) = yopt(i:i)
        enddo
        string3(i) = 0

	limit = min(len_trim(ylabel),maxlen-1)
        do i = 1,limit
	   string4(i) = ylabel(i:i)
	enddo
        string4(i) = 0

	limit = min(len_trim(zopt),maxlen-1)
        do i = 1,limit
	   string5(i) = zopt(i:i)
        enddo
        string5(i) = 0

	limit = min(len_trim(zlabel),maxlen-1)
        do i = 1,limit
	   string6(i) = zlabel(i:i)
	enddo
        string6(i) = 0

	call plbox3_( s1, s2, xtick, nxsub, s3, s4, ytick, nysub,
     *	      s5, s6, ztick, nzsub )

	end

C***********************************************************************

	subroutine plcon0(z,nx,ny,kx,lx,ky,ly,clevel,nlevel)
	call plcon0_(z,nx,ny,kx,lx,ky,ly,clevel,nlevel)
	end

C***********************************************************************

	subroutine plcon1(z,nx,ny,kx,lx,ky,ly,clevel,nlevel,xg,yg)
	call plcon1_(z,nx,ny,kx,lx,ky,ly,clevel,nlevel,xg,yg)
	end

C***********************************************************************

	subroutine plcon2(z,nx,ny,kx,lx,ky,ly,clevel,nlevel,xg,yg)
	call plcon2_(z,nx,ny,kx,lx,ky,ly,clevel,nlevel,xg,yg)
	end

C***********************************************************************

	subroutine plcont(z,nx,ny,kx,lx,ky,ly,clevel,nlevel)

	real	z, clevel
	integer nx, ny, kx, lx, ky, ly, nlevel
	real tr
	common /plplot/ tr(6)

	call plcont_(z,nx,ny,kx,lx,ky,ly,clevel,nlevel,tr)

	end

C***********************************************************************

	subroutine pllab(xlab,ylab,title)

	character*(*) xlab,ylab,title

	parameter (maxlen = 300)
	character*1 string1(maxlen), string2(maxlen), string3(maxlen)
	character*1 string4(maxlen), string5(maxlen), string6(maxlen)
	integer s1(75), s2(75), s3(75), s4(75), s5(75), s6(75)
	equivalence ( s1, string1 ), ( s2, string2 )
	equivalence ( s3, string3 ), ( s4, string4 )
	equivalence ( s5, string5 ), ( s6, string6 )
	common /string/ string1, string2, string3, string4, string5, string6

	limit = min(len_trim(xlab),maxlen-1)
        do i = 1,limit
	   string1(i) = xlab(i:i) 
        enddo
        string1(i) = 0

	limit = min(len_trim(ylab),maxlen-1)
        do i = 1,limit
	   string2(i) = ylab(i:i) 
	enddo
        string2(i) = 0

	limit = min(len_trim(title),maxlen-1)
        do i = 1,limit
	   string3(i) = title(i:i) 
	enddo
        string3(i) = 0
	
	call pllab_(s1,s2,s3)

	end

C***********************************************************************

	subroutine plancol(icolor, name)

	integer icolor
	character*(*) name

	parameter (maxlen = 300)
	character*1 string1(maxlen), string2(maxlen), string3(maxlen)
	character*1 string4(maxlen), string5(maxlen), string6(maxlen)
	integer s1(75), s2(75), s3(75), s4(75), s5(75), s6(75)
	equivalence ( s1, string1 ), ( s2, string2 )
	equivalence ( s3, string3 ), ( s4, string4 )
	equivalence ( s5, string5 ), ( s6, string6 )
	common /string/ string1, string2, string3, string4, string5, string6

	limit = min(len_trim(name),maxlen-1)
        do i = 1,limit
	   string1(i) = name(i:i) 
        enddo
        string1(i) = 0

	call plancol_(icolor, s1)

	end

C***********************************************************************

	subroutine plmtex(side,disp,pos,xjust,text)

	real	disp, pos, xjust
	character*(*) side, text

	parameter (maxlen = 300)
	character*1 string1(maxlen), string2(maxlen), string3(maxlen)
	character*1 string4(maxlen), string5(maxlen), string6(maxlen)
	integer s1(75), s2(75), s3(75), s4(75), s5(75), s6(75)
	equivalence ( s1, string1 ), ( s2, string2 )
	equivalence ( s3, string3 ), ( s4, string4 )
	equivalence ( s5, string5 ), ( s6, string6 )
	common /string/ string1, string2, string3, string4, string5, string6

	limit = min(len_trim(side),maxlen-1)
        do i = 1,limit
	   string1(i) = side(i:i)
        enddo
        string1(i) = 0

	limit = min(len_trim(text),maxlen-1)
        do i = 1,limit
	   string2(i) = text(i:i)
	enddo
        string2(i) = 0

	call plmtex_(s1,disp,pos,xjust,s2)

	end

C***********************************************************************

	subroutine plptex(x,y,dx,dy,xjust,text)

	real	x, y, dx, dy, xjust
	character*(*) text

	parameter (maxlen = 300)
	character*1 string1(maxlen), string2(maxlen), string3(maxlen)
	character*1 string4(maxlen), string5(maxlen), string6(maxlen)
	integer s1(75), s2(75), s3(75), s4(75), s5(75), s6(75)
	equivalence ( s1, string1 ), ( s2, string2 )
	equivalence ( s3, string3 ), ( s4, string4 )
	equivalence ( s5, string5 ), ( s6, string6 )
	common /string/ string1, string2, string3, string4, string5, string6

	limit = min(len_trim(text),maxlen-1)
        do i = 1,limit
	   string1(i) = text(i:i)
        enddo
        string1(i) = 0

	call plptex_(x,y,dx,dy,xjust,s1)

	end

C***********************************************************************

	subroutine plstart(devname, nx, ny)

	character*(*) devname
	integer nx, ny

	parameter (maxlen = 300)
	character*1 string1(maxlen), string2(maxlen), string3(maxlen)
	character*1 string4(maxlen), string5(maxlen), string6(maxlen)
	integer s1(75), s2(75), s3(75), s4(75), s5(75), s6(75)
	equivalence ( s1, string1 ), ( s2, string2 )
	equivalence ( s3, string3 ), ( s4, string4 )
	equivalence ( s5, string5 ), ( s6, string6 )
	common /string/ string1, string2, string3, string4, string5, string6

	limit = min(len_trim(devname),maxlen-1)
        do i = 1,limit
	   string1(i) = devname(i:i)
        enddo
        string1(i) = 0

	call plstart_(s1, nx, ny)

	end

C***********************************************************************
