*   _______________________________________________________________________
*
*   DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
*   Copyright 2014 Sandia Corporation.
*   This software is distributed under the GNU Lesser General Public License.
*   For more information, see the README file in the top Dakota directory.
*   _______________________________________________________________________
*
C  From http://www.netlib.org/lawson-hanson/all
C  
C  This package of  Fortran 77 and Fortran 90 code accompanies
C  the SIAM Publications printing of "Solving Least Squares
C  Problems," by C. Lawson and R. Hanson.  The routines in the
C  package are filed as shown in the list below.  The units 
C  listed 1.-14. may be compiled and placed in a library. Routine
C  G2 is no longer used but is included for completeness.  The
C  main program units 15.-18., 20.-21. require this library for
C  all external references.  The item 19.  is a data file read
C  by program unit PROG4.FOR.  The main program 23., PROG7.F90,
C  requires the subprogram 22. or BVLS.F90.
C  
C  1.   BNDACC.FOR 
C  2.   BNDSOL.FOR 
C  3.   DIFF.FOR
C  4.   G1.FOR
C  5.   G2.FOR 
C  6.   GEN.FOR        
C  7.   H12.FOR         
C  8.   HFTI.FOR 
C  9.   LDP.FOR        
C  10.  MFEOUT.FOR         
C  11.  NNLS.FOR        
C  12.  QRBD.FOR        
C  13.  SVA.FOR       
C  14.  SVDRS.FOR
C  15.  PROG1.FOR         
C  16.  PROG2.FOR        
C  17.  PROG3.FOR        
C  18.  PROG4.FOR 
C  19.  DATA4.DAT            
C  20.  PROG5.FOR        
C  21.  PROG6.FOR
C  
C  22.  BVLS.F90
C  23.  PROG7.F90        
C  -------------------------------------------------------------
C  -------------------------------------------------------------
      SUBROUTINE BNDACC (G,MDG,NB,IP,IR,MT,JT)  
c
C  SEQUENTIAL ALGORITHM FOR BANDED LEAST SQUARES PROBLEM..  
C  ACCUMULATION PHASE.      FOR SOLUTION PHASE USE BNDSOL.  
c
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1973 JUN 12, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
C   
C     THE CALLING PROGRAM MUST SET IR=1 AND IP=1 BEFORE THE FIRST CALL  
C     TO BNDACC FOR A NEW CASE.     
C   
C     THE SECOND SUBSCRIPT OF G( ) MUST BE DIMENSIONED AT LEAST 
C     NB+1 IN THE CALLING PROGRAM.  
c     ------------------------------------------------------------------
      integer I, J, IE, IG, IG1, IG2, IP, IR, JG, JT, K, KH, L, LP1
      integer MDG, MH, MT, MU, NB, NBP1
c     double precision G(MDG,NB+1)
      double precision G(MDG,*)
      double precision RHO, ZERO
      parameter(ZERO = 0.0d0)
c     ------------------------------------------------------------------
C   
C              ALG. STEPS 1-4 ARE PERFORMED EXTERNAL TO THIS SUBROUTINE.
C   
      NBP1=NB+1 
      IF (MT.LE.0) RETURN   
C                                             ALG. STEP 5   
      IF (JT.EQ.IP) GO TO 70
C                                             ALG. STEPS 6-7
      IF (JT.LE.IR) GO TO 30
C                                             ALG. STEPS 8-9
      DO 10 I=1,MT  
        IG1=JT+MT-I     
        IG2=IR+MT-I     
        DO 10 J=1,NBP1  
   10   G(IG1,J)=G(IG2,J)   
C                                             ALG. STEP 10  
      IE=JT-IR  
      DO 20 I=1,IE  
        IG=IR+I-1   
        DO 20 J=1,NBP1  
   20   G(IG,J)=ZERO    
C                                             ALG. STEP 11  
      IR=JT 
C                                             ALG. STEP 12  
   30 MU=min(NB-1,IR-IP-1) 
      IF (MU.EQ.0) GO TO 60 
C                                             ALG. STEP 13  
      DO 50 L=1,MU  
C                                             ALG. STEP 14  
        K=min(L,JT-IP) 
C                                             ALG. STEP 15  
        LP1=L+1 
        IG=IP+L 
        DO 40 I=LP1,NB  
          JG=I-K
   40     G(IG,JG)=G(IG,I)  
C                                             ALG. STEP 16  
        DO 50 I=1,K     
        JG=NBP1-I   
   50   G(IG,JG)=ZERO   
C                                             ALG. STEP 17  
   60 IP=JT 
C                                             ALG. STEPS 18-19  
   70 MH=IR+MT-IP   
      KH=min(NBP1,MH)  
C                                             ALG. STEP 20  
      DO 80 I=1,KH  
        CALL H12 (1,I,max(I+1,IR-IP+1),MH,G(IP,I),1,RHO,   
     *            G(IP,I+1),1,MDG,NBP1-I)   
   80 continue
C                                             ALG. STEP 21  
      IR=IP+KH  
C                                             ALG. STEP 22  
      IF (KH.LT.NBP1) GO TO 100     
C                                             ALG. STEP 23  
      DO 90 I=1,NB  
   90   G(IR-1,I)=ZERO  
C                                             ALG. STEP 24  
  100 CONTINUE  
C                                             ALG. STEP 25  
      RETURN
      END   


      SUBROUTINE BNDSOL (MODE,G,MDG,NB,IP,IR,X,N,RNORM)     
c
C  SEQUENTIAL SOLUTION OF A BANDED LEAST SQUARES PROBLEM..  
C  SOLUTION PHASE.   FOR THE ACCUMULATION PHASE USE BNDACC.     
c
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1973 JUN 12, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
C   
C     MODE = 1     SOLVE R*X=Y   WHERE R AND Y ARE IN THE G( ) ARRAY    
C                  AND X WILL BE STORED IN THE X( ) ARRAY.  
C            2     SOLVE (R**T)*X=Y   WHERE R IS IN G( ),   
C                  Y IS INITIALLY IN X( ), AND X REPLACES Y IN X( ),    
C            3     SOLVE R*X=Y   WHERE R IS IN G( ).
C                  Y IS INITIALLY IN X( ), AND X REPLACES Y IN X( ).    
C   
C     THE SECOND SUBSCRIPT OF G( ) MUST BE DIMENSIONED AT LEAST 
C     NB+1 IN THE CALLING PROGRAM.  
      integer I, I1, I2, IE, II, IP, IR, IX, J, JG, L
      integer MDG, MODE, N, NB, NP1, IRM1
      double precision G(MDG,*), RNORM, RSQ, S, X(N), ZERO
      parameter(ZERO = 0.0d0)
C   
      RNORM=ZERO
      GO TO (10,90,50), MODE
C                                   ********************* MODE = 1  
C                                   ALG. STEP 26
   10      DO 20 J=1,N  
   20      X(J)=G(J,NB+1)   
      RSQ=ZERO  
      NP1=N+1   
      IRM1=IR-1 
      IF (NP1.GT.IRM1) GO TO 40     
           DO 30 J=NP1,IRM1 
   30      RSQ=RSQ+G(J,NB+1)**2     
      RNORM=SQRT(RSQ)   
   40 CONTINUE  
C                                   ********************* MODE = 3  
C                                   ALG. STEP 27
   50      DO 80 II=1,N 
           I=N+1-II     
C                                   ALG. STEP 28
           S=ZERO   
           L=max(0,I-IP)   
C                                   ALG. STEP 29
           IF (I.EQ.N) GO TO 70     
C                                   ALG. STEP 30
           IE=min(N+1-I,NB)
                DO 60 J=2,IE
                JG=J+L  
                IX=I-1+J
   60           S=S+G(I,JG)*X(IX)   
C                                   ALG. STEP 31
   70      continue
           IF (G(I,L+1) .eq. ZERO) go to 130
   80      X(I)=(X(I)-S)/G(I,L+1)   
C                                   ALG. STEP 32
      RETURN
C                                   ********************* MODE = 2  
   90      DO 120 J=1,N 
           S=ZERO   
           IF (J.EQ.1) GO TO 110    
           I1=max(1,J-NB+1)
           I2=J-1   
                DO 100 I=I1,I2  
                L=J-I+1+max(0,I-IP)
  100           S=S+X(I)*G(I,L)     
  110      L=max(0,J-IP)   
           IF (G(J,L+1) .eq. ZERO) go to 130
  120      X(J)=(X(J)-S)/G(J,L+1)   
      RETURN
C   
  130 write (*,'(/a/a,4i6)')' ZERO DIAGONAL TERM IN BNDSOL.',
     *      ' MODE,I,J,L = ',MODE,I,J,L  
      STOP  
      END

       double precision FUNCTION DIFF(X,Y)
c
c  Function used in tests that depend on machine precision.
c
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1973 JUN 7, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
C
      double precision X, Y
      DIFF=X-Y  
      RETURN
      END   
      SUBROUTINE G1 (A,B,CTERM,STERM,SIG)   
c
C     COMPUTE ORTHOGONAL ROTATION MATRIX..  
c
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1973 JUN 12, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
C   
C     COMPUTE.. MATRIX   (C, S) SO THAT (C, S)(A) = (SQRT(A**2+B**2))   
C                        (-S,C)         (-S,C)(B)   (   0          )    
C     COMPUTE SIG = SQRT(A**2+B**2) 
C        SIG IS COMPUTED LAST TO ALLOW FOR THE POSSIBILITY THAT 
C        SIG MAY BE IN THE SAME LOCATION AS A OR B .
C     ------------------------------------------------------------------
      double precision A, B, CTERM, ONE, SIG, STERM, XR, YR, ZERO
      parameter(ONE = 1.0d0, ZERO = 0.0d0)
C     ------------------------------------------------------------------
      if (abs(A) .gt. abs(B)) then
         XR=B/A
         YR=sqrt(ONE+XR**2)
         CTERM=sign(ONE/YR,A)
         STERM=CTERM*XR
         SIG=abs(A)*YR     
         RETURN
      endif

      if (B .ne. ZERO) then
         XR=A/B
         YR=sqrt(ONE+XR**2)
         STERM=sign(ONE/YR,B)
         CTERM=STERM*XR
         SIG=abs(B)*YR     
         RETURN
      endif

      SIG=ZERO  
      CTERM=ZERO  
      STERM=ONE   
      RETURN
      END   


      SUBROUTINE G2    (CTERM,STERM,X,Y)
c
C  APPLY THE ROTATION COMPUTED BY G1 TO (X,Y).  
c
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1972 DEC 15, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
c     ------------------------------------------------------------------
      double precision CTERM, STERM, X, XR, Y
c     ------------------------------------------------------------------
      XR=CTERM*X+STERM*Y    
      Y=-STERM*X+CTERM*Y    
      X=XR  
      RETURN
      END 
  
            double precision FUNCTION   GEN(ANOISE)
c
C  GENERATE NUMBERS FOR CONSTRUCTION OF TEST CASES. 
c
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1972 DEC 15, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
c     ------------------------------------------------------------------
      integer I, J, MI, MJ
      double precision AI, AJ, ANOISE, ZERO
      parameter(ZERO = 0.0d0)
      SAVE
c     ------------------------------------------------------------------
      IF (ANOISE) 10,30,20  
   10 MI=891
      MJ=457
      I=5   
      J=7   
      AJ= ZERO
      GEN= ZERO
      RETURN
C   
C     THE SEQUENCE OF VALUES OF J  IS BOUNDED BETWEEN 1 AND 996 
C     IF INITIAL J = 1,2,3,4,5,6,7,8, OR 9, THE PERIOD IS 332   
   20 J=J*MJ
      J=J-997*(J/997)   
      AJ=J-498  
C     THE SEQUENCE OF VALUES OF I  IS BOUNDED BETWEEN 1 AND 999 
C     IF INITIAL I = 1,2,3,6,7, OR 9,  THE PERIOD WILL BE 50
C     IF INITIAL I = 4 OR 8   THE PERIOD WILL BE 25 
C     IF INITIAL I = 5        THE PERIOD WILL BE 10 
   30 I=I*MI
      I=I-1000*(I/1000) 
      AI=I-500  
      GEN=AI+AJ*ANOISE  
      RETURN
      END   


C     SUBROUTINE H12 (MODE,LPIVOT,L1,M,U,IUE,UP,C,ICE,ICV,NCV)  
C   
C  CONSTRUCTION AND/OR APPLICATION OF A SINGLE   
C  HOUSEHOLDER TRANSFORMATION..     Q = I + U*(U**T)/B   
C   
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1973 JUN 12, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
C     ------------------------------------------------------------------
c                     Subroutine Arguments
c
C     MODE   = 1 OR 2   Selects Algorithm H1 to construct and apply a
c            Householder transformation, or Algorithm H2 to apply a
c            previously constructed transformation.
C     LPIVOT IS THE INDEX OF THE PIVOT ELEMENT. 
C     L1,M   IF L1 .LE. M   THE TRANSFORMATION WILL BE CONSTRUCTED TO   
C            ZERO ELEMENTS INDEXED FROM L1 THROUGH M.   IF L1 GT. M     
C            THE SUBROUTINE DOES AN IDENTITY TRANSFORMATION.
C     U(),IUE,UP    On entry with MODE = 1, U() contains the pivot
c            vector.  IUE is the storage increment between elements.  
c            On exit when MODE = 1, U() and UP contain quantities
c            defining the vector U of the Householder transformation.
c            on entry with MODE = 2, U() and UP should contain
c            quantities previously computed with MODE = 1.  These will
c            not be modified during the entry with MODE = 2.   
C     C()    ON ENTRY with MODE = 1 or 2, C() CONTAINS A MATRIX WHICH
c            WILL BE REGARDED AS A SET OF VECTORS TO WHICH THE
c            HOUSEHOLDER TRANSFORMATION IS TO BE APPLIED.
c            ON EXIT C() CONTAINS THE SET OF TRANSFORMED VECTORS.
C     ICE    STORAGE INCREMENT BETWEEN ELEMENTS OF VECTORS IN C().  
C     ICV    STORAGE INCREMENT BETWEEN VECTORS IN C().  
C     NCV    NUMBER OF VECTORS IN C() TO BE TRANSFORMED. IF NCV .LE. 0  
C            NO OPERATIONS WILL BE DONE ON C(). 
C     ------------------------------------------------------------------
      SUBROUTINE H12 (MODE,LPIVOT,L1,M,U,IUE,UP,C,ICE,ICV,NCV)  
C     ------------------------------------------------------------------
      integer I, I2, I3, I4, ICE, ICV, INCR, IUE, J
      integer L1, LPIVOT, M, MODE, NCV
      double precision B, C(*), CL, CLINV, ONE, SM
c     double precision U(IUE,M)
      double precision U(IUE,*)
      double precision UP
      parameter(ONE = 1.0d0)
C     ------------------------------------------------------------------
      IF (0.GE.LPIVOT.OR.LPIVOT.GE.L1.OR.L1.GT.M) RETURN    
      CL=abs(U(1,LPIVOT))   
      IF (MODE.EQ.2) GO TO 60   
C                            ****** CONSTRUCT THE TRANSFORMATION. ******
          DO 10 J=L1,M  
   10     CL=MAX(abs(U(1,J)),CL)  
      IF (CL) 130,130,20
   20 CLINV=ONE/CL  
      SM=(U(1,LPIVOT)*CLINV)**2   
          DO 30 J=L1,M  
   30     SM=SM+(U(1,J)*CLINV)**2 
      CL=CL*SQRT(SM)   
      IF (U(1,LPIVOT)) 50,50,40     
   40 CL=-CL
   50 UP=U(1,LPIVOT)-CL 
      U(1,LPIVOT)=CL    
      GO TO 70  
C            ****** APPLY THE TRANSFORMATION  I+U*(U**T)/B  TO C. ******
C   
   60 IF (CL) 130,130,70
   70 IF (NCV.LE.0) RETURN  
      B= UP*U(1,LPIVOT)
C                       B  MUST BE NONPOSITIVE HERE.  IF B = 0., RETURN.
C   
      IF (B) 80,130,130 
   80 B=ONE/B   
      I2=1-ICV+ICE*(LPIVOT-1)   
      INCR=ICE*(L1-LPIVOT)  
          DO 120 J=1,NCV
          I2=I2+ICV     
          I3=I2+INCR    
          I4=I3 
          SM=C(I2)*UP
              DO 90 I=L1,M  
              SM=SM+C(I3)*U(1,I)
   90         I3=I3+ICE 
          IF (SM) 100,120,100   
  100     SM=SM*B   
          C(I2)=C(I2)+SM*UP
              DO 110 I=L1,M 
              C(I4)=C(I4)+SM*U(1,I)
  110         I4=I4+ICE 
  120     CONTINUE  
  130 RETURN
      END   

      SUBROUTINE HFTI (A,MDA,M,N,B,MDB,NB,TAU,KRANK,RNORM,H,G,IP)   
c
C  SOLVE LEAST SQUARES PROBLEM USING ALGORITHM, HFTI.   
c  Householder Forward Triangulation with column Interchanges.
c
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1973 JUN 12, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
C     ------------------------------------------------------------------
      integer I, II, IP1, J, JB, JJ, K, KP1, KRANK
      integer L, LDIAG, LMAX, M, MDA, MDB, N, NB
c     integer IP(N)     
c     double precision A(MDA,N),B(MDB,NB),H(N),G(N),RNORM(NB)  
      integer IP(*)     
      double precision A(MDA,*),B(MDB, *),H(*),G(*),RNORM( *)  
      double precision DIFF, FACTOR, HMAX, SM, TAU, TMP, ZERO     
      parameter(FACTOR = 0.001d0, ZERO = 0.0d0)
C     ------------------------------------------------------------------
C   
      K=0   
      LDIAG=min(M,N)   
      IF (LDIAG.LE.0) GO TO 270     
          DO 80 J=1,LDIAG   
          IF (J.EQ.1) GO TO 20  
C   
C     UPDATE SQUARED COLUMN LENGTHS AND FIND LMAX   
C    ..     
          LMAX=J
              DO 10 L=J,N   
              H(L)=H(L)-A(J-1,L)**2 
              IF (H(L).GT.H(LMAX)) LMAX=L   
   10         CONTINUE  
          IF(DIFF(HMAX+FACTOR*H(LMAX),HMAX)) 20,20,50   
C   
C     COMPUTE SQUARED COLUMN LENGTHS AND FIND LMAX  
C    ..     
   20     LMAX=J
              DO 40 L=J,N   
              H(L)=0.   
                  DO 30 I=J,M   
   30             H(L)=H(L)+A(I,L)**2   
              IF (H(L).GT.H(LMAX)) LMAX=L   
   40         CONTINUE  
          HMAX=H(LMAX)  
C    ..     
C     LMAX HAS BEEN DETERMINED  
C   
C     DO COLUMN INTERCHANGES IF NEEDED. 
C    ..     
   50     CONTINUE  
          IP(J)=LMAX    
          IF (IP(J).EQ.J) GO TO 70  
              DO 60 I=1,M   
              TMP=A(I,J)
              A(I,J)=A(I,LMAX)  
   60         A(I,LMAX)=TMP 
          H(LMAX)=H(J)  
C   
C     COMPUTE THE J-TH TRANSFORMATION AND APPLY IT TO A AND B.  
C    ..     
   70     CALL H12 (1,J,J+1,M,A(1,J),1,H(J),A(1,J+1),1,MDA,N-J) 
   80     CALL H12 (2,J,J+1,M,A(1,J),1,H(J),B,1,MDB,NB)     
C   
C     DETERMINE THE PSEUDORANK, K, USING THE TOLERANCE, TAU.
C    ..     
          DO 90 J=1,LDIAG   
          IF (ABS(A(J,J)).LE.TAU) GO TO 100     
   90     CONTINUE  
      K=LDIAG   
      GO TO 110 
  100 K=J-1 
  110 KP1=K+1   
C   
C     COMPUTE THE NORMS OF THE RESIDUAL VECTORS.
C   
      IF (NB.LE.0) GO TO 140
          DO 130 JB=1,NB
          TMP=ZERO     
          IF (KP1.GT.M) GO TO 130   
              DO 120 I=KP1,M
  120         TMP=TMP+B(I,JB)**2    
  130     RNORM(JB)=SQRT(TMP)   
  140 CONTINUE  
C                                           SPECIAL FOR PSEUDORANK = 0  
      IF (K.GT.0) GO TO 160 
      IF (NB.LE.0) GO TO 270
          DO 150 JB=1,NB
              DO 150 I=1,N  
  150         B(I,JB)=ZERO 
      GO TO 270 
C   
C     IF THE PSEUDORANK IS LESS THAN N COMPUTE HOUSEHOLDER  
C     DECOMPOSITION OF FIRST K ROWS.
C    ..     
  160 IF (K.EQ.N) GO TO 180 
          DO 170 II=1,K 
          I=KP1-II  
  170     CALL H12 (1,I,KP1,N,A(I,1),MDA,G(I),A,MDA,1,I-1)  
  180 CONTINUE  
C   
C   
      IF (NB.LE.0) GO TO 270
          DO 260 JB=1,NB
C   
C     SOLVE THE K BY K TRIANGULAR SYSTEM.   
C    ..     
              DO 210 L=1,K  
              SM=ZERO  
              I=KP1-L   
              IF (I.EQ.K) GO TO 200 
              IP1=I+1   
                  DO 190 J=IP1,K    
  190             SM=SM+A(I,J)*B(J,JB)
  200         continue
  210         B(I,JB)=(B(I,JB)-SM)/A(I,I)  
C   
C     COMPLETE COMPUTATION OF SOLUTION VECTOR.  
C    ..     
          IF (K.EQ.N) GO TO 240     
              DO 220 J=KP1,N
  220         B(J,JB)=ZERO 
              DO 230 I=1,K  
  230         CALL H12 (2,I,KP1,N,A(I,1),MDA,G(I),B(1,JB),1,MDB,1)  
C   
C      RE-ORDER THE SOLUTION VECTOR TO COMPENSATE FOR THE   
C      COLUMN INTERCHANGES. 
C    ..     
  240         DO 250 JJ=1,LDIAG     
              J=LDIAG+1-JJ  
              IF (IP(J).EQ.J) GO TO 250 
              L=IP(J)   
              TMP=B(L,JB)   
              B(L,JB)=B(J,JB)   
              B(J,JB)=TMP   
  250         CONTINUE  
  260     CONTINUE  
C    ..     
C     THE SOLUTION VECTORS, X, ARE NOW  
C     IN THE FIRST  N  ROWS OF THE ARRAY B(,).  
C   
  270 KRANK=K   
      RETURN
      END  

      SUBROUTINE LDP (G,MDG,M,N,H,X,XNORM,W,INDEX,MODE)     
c
C  Algorithm LDP: LEAST DISTANCE PROGRAMMING
c
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1974 MAR 1, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
C     ------------------------------------------------------------------
      integer I, IW, IWDUAL, IY, IZ, J, JF, M, MDG, MODE, N, NP1
c     integer INDEX(M)  
c     double precision G(MDG,N), H(M), X(N), W(*)  
      integer INDEX(*)  
      double precision G(MDG,*), H(*), X(*), W(*)
      double precision DIFF, FAC, ONE, RNORM, XNORM, ZERO
      parameter(ONE = 1.0d0, ZERO = 0.0d0)
C     ------------------------------------------------------------------
      IF (N.LE.0) GO TO 120 
          DO 10 J=1,N   
   10     X(J)=ZERO     
      XNORM=ZERO
      IF (M.LE.0) GO TO 110 
C   
C     THE DECLARED DIMENSION OF W() MUST BE AT LEAST (N+1)*(M+2)+2*M.   
C   
C      FIRST (N+1)*M LOCS OF W()   =  MATRIX E FOR PROBLEM NNLS.
C       NEXT     N+1 LOCS OF W()   =  VECTOR F FOR PROBLEM NNLS.
C       NEXT     N+1 LOCS OF W()   =  VECTOR Z FOR PROBLEM NNLS.
C       NEXT       M LOCS OF W()   =  VECTOR Y FOR PROBLEM NNLS.
C       NEXT       M LOCS OF W()   =  VECTOR WDUAL FOR PROBLEM NNLS.    
C     COPY G**T INTO FIRST N ROWS AND M COLUMNS OF E.   
C     COPY H**T INTO ROW N+1 OF E.  
C   
      IW=0  
          DO 30 J=1,M   
              DO 20 I=1,N   
              IW=IW+1   
   20         W(IW)=G(J,I)  
          IW=IW+1   
   30     W(IW)=H(J)    
      JF=IW+1   
C                                STORE N ZEROS FOLLOWED BY A ONE INTO F.
          DO 40 I=1,N   
          IW=IW+1   
   40     W(IW)=ZERO    
      W(IW+1)=ONE   
C   
      NP1=N+1   
      IZ=IW+2   
      IY=IZ+NP1 
      IWDUAL=IY+M   
C   
      CALL NNLS (W,NP1,NP1,M,W(JF),W(IY),RNORM,W(IWDUAL),W(IZ),INDEX,   
     *           MODE)  
C                      USE THE FOLLOWING RETURN IF UNSUCCESSFUL IN NNLS.
      IF (MODE.NE.1) RETURN 
      IF (RNORM) 130,130,50 
   50 FAC=ONE   
      IW=IY-1   
          DO 60 I=1,M   
          IW=IW+1   
C                               HERE WE ARE USING THE SOLUTION VECTOR Y.
   60     FAC=FAC-H(I)*W(IW)
C   
      IF (DIFF(ONE+FAC,ONE)) 130,130,70 
   70 FAC=ONE/FAC   
          DO 90 J=1,N   
          IW=IY-1   
              DO 80 I=1,M   
              IW=IW+1   
C                               HERE WE ARE USING THE SOLUTION VECTOR Y.
   80         X(J)=X(J)+G(I,J)*W(IW)
   90     X(J)=X(J)*FAC 
          DO 100 J=1,N  
  100     XNORM=XNORM+X(J)**2   
      XNORM=sqrt(XNORM) 
C                             SUCCESSFUL RETURN.
  110 MODE=1
      RETURN
C                             ERROR RETURN.       N .LE. 0. 
  120 MODE=2
      RETURN
C                             RETURNING WITH CONSTRAINTS NOT COMPATIBLE.
  130 MODE=4
      RETURN
      END   
 
      subroutine MFEOUT (A, MDA, M, N, NAMES, MODE, UNIT, WIDTH)
c
C  LABELED MATRIX OUTPUT FOR USE WITH SINGULAR VALUE ANALYSIS.
C   
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1973 JUN 12, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
C     ------------------------------------------------------------------   
c  This 1995 version has additional arguments, UNIT and WIDTH,
c  to support user options regarding the output unit and the width of
c  print lines.  Also allows user to choose length of names in NAMES().
c     ------------------------------------------------------------------
c                           Subroutine Arguments
C     All are input arguments.  None are modified by this subroutine.
c
C     A(,)     Array containing matrix to be output.
C     MDA      First dimension of the array, A(,).
C     M, N     No. of rows and columns, respectively in the matrix
c              contained in A(,).
C     NAMES()  [character array]  Array of names.
c              If NAMES(1) contains only blanks, the rest of the NAMES()
c              array will be ignored.
C     MODE     =1  Write header for V matrix and use an F format.
C              =2  Write header for  for candidate solutions and use
c                  P format.
c     UNIT  [integer]   Selects output unit.  If UNIT .ge. 0 then UNIT
c              is the output unit number.  If UNIT = -1, output to
c              the '*' unit.
c     WIDTH [integer]   Selects width of output lines.
c              Each output line from this subroutine will have at most
c              max(26,min(124,WIDTH)) characters plus one additional
c              leading character for Fortran "carriage control".  The
c              carriage control character will always be a blank.
c     ------------------------------------------------------------------
      integer I, J, J1, J2, KBLOCK, L, LENNAM
      integer M, MAXCOL, MDA, MODE, N, NAMSIZ, NBLOCK, UNIT, WIDTH
      double precision    A(MDA,N)
      character  NAMES(M)*(*)
      character*4  HEAD (2)
      character*26 FMT1(2)
      character*26 FMT2(2)
      logical      BLKNAM, STAR
C
      data HEAD(1)/' COL'/
      data HEAD(2)/'SOLN'/
      data FMT1 / '(/7x,00x,8(5x,a4,i4,1x)/)',
     *            '(/7x,00x,8(2x,a4,i4,4x)/)'/
      data FMT2 / '(1x,i4,1x,a00,1x,4p8f14.0)',
     *            '(1x,i4,1x,a00,1x,8g14.6  )'/
c     ------------------------------------------------------------------
      if (M .le. 0 .or. N .le. 0) return
      STAR = UNIT .lt. 0
C
c     The LEN function returns the char length of a single element of
c     the NAMES() array.
c
      LENNAM = len(NAMES(1))
      BLKNAM = NAMES(1) .eq. ' '
      NAMSIZ = 1
      if(.not. BLKNAM) then
         do 30 I = 1,M
            do 10 L = LENNAM, NAMSIZ+1, -1
               if(NAMES(I)(L:L) .ne. ' ') then
                  NAMSIZ = L
                  go to 20
               endif
   10       continue
   20       continue
   30    continue
      endif
c
      write(FMT1(MODE)(6:7),'(i2.2)') NAMSIZ
      write(FMT2(MODE)(12:13),'(i2.2)') NAMSIZ

   70 format(/' V-Matrix of the Singular Value Decomposition of A*D.'/
     *    ' (Elements of V scaled up by a factor of 10**4)')
   80 format(/' Sequence of candidate solutions, X')

      if(STAR) then
         if (MODE .eq. 1) then
            write (*,70)
         else
            write (*,80)
         endif
      else
         if (MODE .eq. 1) then
            write (UNIT,70)
         else
            write (UNIT,80)
         endif
      endif
c
c     With NAMSIZ characters allowed for the "name" and MAXCOL
c     columns of numbers, the total line width, exclusive of a
c     carriage control character, will be 6 + LENNAM + 14 * MAXCOL.
c
      MAXCOL = max(1,min(8,(WIDTH - 6 - NAMSIZ)/14))
C
      NBLOCK = (N + MAXCOL -1) / MAXCOL
      J2 = 0
C
      do 50 KBLOCK = 1, NBLOCK
         J1 = J2 + 1
         J2 = min(N, J2 + MAXCOL)
         if(STAR) then
            write (*,FMT1(MODE)) (HEAD(MODE),J,J=J1,J2)
         else
            write (UNIT,FMT1(MODE)) (HEAD(MODE),J,J=J1,J2)
         endif
C
         do 40 I=1,M
            if(STAR) then
               if(BLKNAM) then
                  write (*,FMT2(MODE)) I,' ',(A(I,J),J=J1,J2)
               else
                  write (*,FMT2(MODE)) I,NAMES(I),(A(I,J),J=J1,J2)
               endif
            else
               if(BLKNAM) then
                  write (UNIT,FMT2(MODE)) I,' ',(A(I,J),J=J1,J2)
               else
                  write (UNIT,FMT2(MODE)) I,NAMES(I),(A(I,J),J=J1,J2)
               endif
            endif
   40    continue
   50 continue
      end

C     SUBROUTINE NNLS  (A,MDA,M,N,B,X,RNORM,W,ZZ,INDEX,MODE)
C   
C  Algorithm NNLS: NONNEGATIVE LEAST SQUARES
C   
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1973 JUN 15, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
c
C     GIVEN AN M BY N MATRIX, A, AND AN M-VECTOR, B,  COMPUTE AN
C     N-VECTOR, X, THAT SOLVES THE LEAST SQUARES PROBLEM   
C   
C                      A * X = B  SUBJECT TO X .GE. 0   
C     ------------------------------------------------------------------
c                     Subroutine Arguments
c
C     A(),MDA,M,N     MDA IS THE FIRST DIMENSIONING PARAMETER FOR THE   
C                     ARRAY, A().   ON ENTRY A() CONTAINS THE M BY N    
C                     MATRIX, A.           ON EXIT A() CONTAINS 
C                     THE PRODUCT MATRIX, Q*A , WHERE Q IS AN   
C                     M BY M ORTHOGONAL MATRIX GENERATED IMPLICITLY BY  
C                     THIS SUBROUTINE.  
C     B()     ON ENTRY B() CONTAINS THE M-VECTOR, B.   ON EXIT B() CON- 
C             TAINS Q*B.
C     X()     ON ENTRY X() NEED NOT BE INITIALIZED.  ON EXIT X() WILL   
C             CONTAIN THE SOLUTION VECTOR.  
C     RNORM   ON EXIT RNORM CONTAINS THE EUCLIDEAN NORM OF THE  
C             RESIDUAL VECTOR.  
C     W()     AN N-ARRAY OF WORKING SPACE.  ON EXIT W() WILL CONTAIN    
C             THE DUAL SOLUTION VECTOR.   W WILL SATISFY W(I) = 0.  
C             FOR ALL I IN SET P  AND W(I) .LE. 0. FOR ALL I IN SET Z   
C     ZZ()     AN M-ARRAY OF WORKING SPACE.     
C     INDEX()     AN INTEGER WORKING ARRAY OF LENGTH AT LEAST N.
C                 ON EXIT THE CONTENTS OF THIS ARRAY DEFINE THE SETS    
C                 P AND Z AS FOLLOWS..  
C   
C                 INDEX(1)   THRU INDEX(NSETP) = SET P.     
C                 INDEX(IZ1) THRU INDEX(IZ2)   = SET Z.     
C                 IZ1 = NSETP + 1 = NPP1
C                 IZ2 = N   
C     MODE    THIS IS A SUCCESS-FAILURE FLAG WITH THE FOLLOWING 
C             MEANINGS. 
C             1     THE SOLUTION HAS BEEN COMPUTED SUCCESSFULLY.
C             2     THE DIMENSIONS OF THE PROBLEM ARE BAD.  
C                   EITHER M .LE. 0 OR N .LE. 0.
C             3    ITERATION COUNT EXCEEDED.  MORE THAN 3*N ITERATIONS. 
C   
C     ------------------------------------------------------------------
      SUBROUTINE NNLS (A,MDA,M,N,B,X,RNORM,W,ZZ,INDEX,MODE) 
C     ------------------------------------------------------------------
      integer I, II, IP, ITER, ITMAX, IZ, IZ1, IZ2, IZMAX, J, JJ, JZ, L
      integer M, MDA, MODE,N, NPP1, NSETP, RTNKEY
c     integer INDEX(N)  
c     double precision A(MDA,N), B(M), W(N), X(N), ZZ(M)   
      integer INDEX(*)  
      double precision A(MDA,*), B(*), W(*), X(*), ZZ(*)   
      double precision ALPHA, ASAVE, CC, DIFF, DUMMY, FACTOR, RNORM
      double precision SM, SS, T, TEMP, TWO, UNORM, UP, WMAX
      double precision ZERO, ZTEST
      parameter(FACTOR = 0.01d0)
      parameter(TWO = 2.0d0, ZERO = 0.0d0)
C     ------------------------------------------------------------------
      MODE=1
      IF (M .le. 0 .or. N .le. 0) then
         MODE=2
         RETURN
      endif
      ITER=0
      ITMAX=3*N 
C   
C                    INITIALIZE THE ARRAYS INDEX() AND X(). 
C   
          DO 20 I=1,N   
          X(I)=ZERO     
   20     INDEX(I)=I    
C   
      IZ2=N 
      IZ1=1 
      NSETP=0   
      NPP1=1
C                             ******  MAIN LOOP BEGINS HERE  ******     
   30 CONTINUE  
C                  QUIT IF ALL COEFFICIENTS ARE ALREADY IN THE SOLUTION.
C                        OR IF M COLS OF A HAVE BEEN TRIANGULARIZED.    
C   
      IF (IZ1 .GT.IZ2.OR.NSETP.GE.M) GO TO 350   
C   
C         COMPUTE COMPONENTS OF THE DUAL (NEGATIVE GRADIENT) VECTOR W().
C   
      DO 50 IZ=IZ1,IZ2  
         J=INDEX(IZ)   
         SM=ZERO   
         DO 40 L=NPP1,M
   40        SM=SM+A(L,J)*B(L)     
         W(J)=SM   
   50 continue
C                                   FIND LARGEST POSITIVE W(J). 
   60 continue
      WMAX=ZERO 
      DO 70 IZ=IZ1,IZ2  
         J=INDEX(IZ)   
         IF (W(J) .gt. WMAX) then
            WMAX=W(J)     
            IZMAX=IZ  
         endif
   70 CONTINUE  
C   
C             IF WMAX .LE. 0. GO TO TERMINATION.
C             THIS INDICATES SATISFACTION OF THE KUHN-TUCKER CONDITIONS.
C   
      IF (WMAX .le. ZERO) go to 350
      IZ=IZMAX  
      J=INDEX(IZ)   
C   
C     THE SIGN OF W(J) IS OK FOR J TO BE MOVED TO SET P.    
C     BEGIN THE TRANSFORMATION AND CHECK NEW DIAGONAL ELEMENT TO AVOID  
C     NEAR LINEAR DEPENDENCE.   
C   
      ASAVE=A(NPP1,J)   
      CALL H12 (1,NPP1,NPP1+1,M,A(1,J),1,UP,DUMMY,1,1,0)    
      UNORM=ZERO
      IF (NSETP .ne. 0) then
          DO 90 L=1,NSETP   
   90       UNORM=UNORM+A(L,J)**2     
      endif
      UNORM=sqrt(UNORM) 
      IF (DIFF(UNORM+ABS(A(NPP1,J))*FACTOR,UNORM) .gt. ZERO) then
C   
C        COL J IS SUFFICIENTLY INDEPENDENT.  COPY B INTO ZZ, UPDATE ZZ
C        AND SOLVE FOR ZTEST ( = PROPOSED NEW VALUE FOR X(J) ).    
C   
         DO 120 L=1,M  
  120        ZZ(L)=B(L)    
         CALL H12 (2,NPP1,NPP1+1,M,A(1,J),1,UP,ZZ,1,1,1)   
         ZTEST=ZZ(NPP1)/A(NPP1,J)  
C   
C                                     SEE IF ZTEST IS POSITIVE  
C   
         IF (ZTEST .gt. ZERO) go to 140
      endif
C   
C     REJECT J AS A CANDIDATE TO BE MOVED FROM SET Z TO SET P.  
C     RESTORE A(NPP1,J), SET W(J)=0., AND LOOP BACK TO TEST DUAL
C     COEFFS AGAIN.     
C   
      A(NPP1,J)=ASAVE   
      W(J)=ZERO 
      GO TO 60  
C   
C     THE INDEX  J=INDEX(IZ)  HAS BEEN SELECTED TO BE MOVED FROM
C     SET Z TO SET P.    UPDATE B,  UPDATE INDICES,  APPLY HOUSEHOLDER  
C     TRANSFORMATIONS TO COLS IN NEW SET Z,  ZERO SUBDIAGONAL ELTS IN   
C     COL J,  SET W(J)=0.   
C   
  140 continue
      DO 150 L=1,M  
  150    B(L)=ZZ(L)    
C   
      INDEX(IZ)=INDEX(IZ1)  
      INDEX(IZ1)=J  
      IZ1=IZ1+1 
      NSETP=NPP1
      NPP1=NPP1+1   
C   
      IF (IZ1 .le. IZ2) then
         DO 160 JZ=IZ1,IZ2 
            JJ=INDEX(JZ)  
            CALL H12 (2,NSETP,NPP1,M,A(1,J),1,UP,A(1,JJ),1,MDA,1)
  160    continue
      endif
C   
      IF (NSETP .ne. M) then
         DO 180 L=NPP1,M   
  180       A(L,J)=ZERO   
      endif
C   
      W(J)=ZERO 
C                                SOLVE THE TRIANGULAR SYSTEM.   
C                                STORE THE SOLUTION TEMPORARILY IN ZZ().
      RTNKEY = 1
      GO TO 400 
  200 CONTINUE  
C   
C                       ******  SECONDARY LOOP BEGINS HERE ******   
C   
C                          ITERATION COUNTER.   
C 
  210 continue  
      ITER=ITER+1   
      IF (ITER .gt. ITMAX) then
         MODE=3
         write (*,'(/a)') ' NNLS quitting on iteration count.'
         GO TO 350 
      endif
C   
C                    SEE IF ALL NEW CONSTRAINED COEFFS ARE FEASIBLE.    
C                                  IF NOT COMPUTE ALPHA.    
C   
      ALPHA=TWO 
      DO 240 IP=1,NSETP 
         L=INDEX(IP)   
         IF (ZZ(IP) .le. ZERO) then
            T=-X(L)/(ZZ(IP)-X(L))     
            IF (ALPHA .gt. T) then
               ALPHA=T   
               JJ=IP 
            endif
         endif
  240 CONTINUE  
C   
C          IF ALL NEW CONSTRAINED COEFFS ARE FEASIBLE THEN ALPHA WILL   
C          STILL = 2.    IF SO EXIT FROM SECONDARY LOOP TO MAIN LOOP.   
C   
      IF (ALPHA.EQ.TWO) GO TO 330   
C   
C          OTHERWISE USE ALPHA WHICH WILL BE BETWEEN 0. AND 1. TO   
C          INTERPOLATE BETWEEN THE OLD X AND THE NEW ZZ.    
C   
      DO 250 IP=1,NSETP 
         L=INDEX(IP)   
         X(L)=X(L)+ALPHA*(ZZ(IP)-X(L)) 
  250 continue
C   
C        MODIFY A AND B AND THE INDEX ARRAYS TO MOVE COEFFICIENT I  
C        FROM SET P TO SET Z.   
C   
      I=INDEX(JJ)   
  260 continue
      X(I)=ZERO 
C   
      IF (JJ .ne. NSETP) then
         JJ=JJ+1   
         DO 280 J=JJ,NSETP 
            II=INDEX(J)   
            INDEX(J-1)=II 
            CALL G1 (A(J-1,II),A(J,II),CC,SS,A(J-1,II))   
            A(J,II)=ZERO  
            DO 270 L=1,N  
               IF (L.NE.II) then
c
c                 Apply procedure G2 (CC,SS,A(J-1,L),A(J,L))  
c
                  TEMP = A(J-1,L)
                  A(J-1,L) = CC*TEMP + SS*A(J,L)
                  A(J,L)   =-SS*TEMP + CC*A(J,L)
               endif
  270       CONTINUE  
c
c                 Apply procedure G2 (CC,SS,B(J-1),B(J))   
c
            TEMP = B(J-1)
            B(J-1) = CC*TEMP + SS*B(J)    
            B(J)   =-SS*TEMP + CC*B(J)    
  280    continue
      endif
c
      NPP1=NSETP
      NSETP=NSETP-1     
      IZ1=IZ1-1 
      INDEX(IZ1)=I  
C   
C        SEE IF THE REMAINING COEFFS IN SET P ARE FEASIBLE.  THEY SHOULD
C        BE BECAUSE OF THE WAY ALPHA WAS DETERMINED.
C        IF ANY ARE INFEASIBLE IT IS DUE TO ROUND-OFF ERROR.  ANY   
C        THAT ARE NONPOSITIVE WILL BE SET TO ZERO   
C        AND MOVED FROM SET P TO SET Z. 
C   
      DO 300 JJ=1,NSETP 
         I=INDEX(JJ)   
         IF (X(I) .le. ZERO) go to 260
  300 CONTINUE  
C   
C         COPY B( ) INTO ZZ( ).  THEN SOLVE AGAIN AND LOOP BACK.
C   
      DO 310 I=1,M  
  310     ZZ(I)=B(I)    
      RTNKEY = 2
      GO TO 400 
  320 CONTINUE  
      GO TO 210 
C                      ******  END OF SECONDARY LOOP  ******
C   
  330 continue
      DO 340 IP=1,NSETP 
          I=INDEX(IP)   
  340     X(I)=ZZ(IP)   
C        ALL NEW COEFFS ARE POSITIVE.  LOOP BACK TO BEGINNING.  
      GO TO 30  
C   
C                        ******  END OF MAIN LOOP  ******   
C   
C                        COME TO HERE FOR TERMINATION.  
C                     COMPUTE THE NORM OF THE FINAL RESIDUAL VECTOR.    
C 
  350 continue  
      SM=ZERO   
      IF (NPP1 .le. M) then
         DO 360 I=NPP1,M   
  360       SM=SM+B(I)**2 
      else
         DO 380 J=1,N  
  380       W(J)=ZERO     
      endif
      RNORM=sqrt(SM)    
      RETURN
C   
C     THE FOLLOWING BLOCK OF CODE IS USED AS AN INTERNAL SUBROUTINE     
C     TO SOLVE THE TRIANGULAR SYSTEM, PUTTING THE SOLUTION IN ZZ().     
C   
  400 continue
      DO 430 L=1,NSETP  
         IP=NSETP+1-L  
         IF (L .ne. 1) then
            DO 410 II=1,IP
               ZZ(II)=ZZ(II)-A(II,JJ)*ZZ(IP+1)   
  410       continue
         endif
         JJ=INDEX(IP)  
         ZZ(IP)=ZZ(IP)/A(IP,JJ)    
  430 continue
      go to (200, 320), RTNKEY
      END   

C     SUBROUTINE QRBD (IPASS,Q,E,NN,V,MDV,NRV,C,MDC,NCC)    
c
C  QR ALGORITHM FOR SINGULAR VALUES OF A BIDIAGONAL MATRIX.
c
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1973 JUN 12, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
C     ------------------------------------------------------------------   
C     THE BIDIAGONAL MATRIX 
C   
C                       (Q1,E2,0...    )
C                       (   Q2,E3,0... )
C                D=     (       .      )
C                       (         .   0)
C                       (           .EN)
C                       (          0,QN)
C   
C                 IS PRE AND POST MULTIPLIED BY 
C                 ELEMENTARY ROTATION MATRICES  
C                 RI AND PI SO THAT 
C   
C                 RK...R1*D*P1**(T)...PK**(T) = DIAG(S1,...,SN) 
C   
C                 TO WITHIN WORKING ACCURACY.   
C   
C  1. EI AND QI OCCUPY E(I) AND Q(I) AS INPUT.  
C   
C  2. RM...R1*C REPLACES 'C' IN STORAGE AS OUTPUT.  
C   
C  3. V*P1**(T)...PM**(T) REPLACES 'V' IN STORAGE AS OUTPUT.
C   
C  4. SI OCCUPIES Q(I) AS OUTPUT.   
C   
C  5. THE SI'S ARE NONINCREASING AND NONNEGATIVE.   
C   
C     THIS CODE IS BASED ON THE PAPER AND 'ALGOL' CODE..    
C REF..     
C  1. REINSCH,C.H. AND GOLUB,G.H. 'SINGULAR VALUE DECOMPOSITION 
C     AND LEAST SQUARES SOLUTIONS' (NUMER. MATH.), VOL. 14,(1970).  
C   
C     ------------------------------------------------------------------   
      SUBROUTINE QRBD (IPASS,Q,E,NN,V,MDV,NRV,C,MDC,NCC)    
C     ------------------------------------------------------------------   
      integer MDC, MDV, NCC, NN, NRV
c     double precision C(MDC,NCC), E(NN), Q(NN),V(MDV,NN)
      double precision C(MDC,*  ), E(* ), Q(* ),V(MDV,* )
      integer I, II, IPASS, J, K, KK, L, LL, LP1, N, N10, NQRS
      double precision CS, DIFF, DNORM, F, G, H, SMALL
      double precision ONE, SN, T, TEMP, TWO, X, Y, Z, ZERO
      
      logical WNTV ,HAVERS,FAIL     
      parameter(ONE = 1.0d0, TWO = 2.0d0, ZERO = 0.0d0)
C     ------------------------------------------------------------------   
      N=NN  
      IPASS=1   
      IF (N.LE.0) RETURN
      N10=10*N  
      WNTV=NRV.GT.0     
      HAVERS=NCC.GT.0   
      FAIL=.FALSE.  
      NQRS=0
      E(1)=ZERO 
      DNORM=ZERO
           DO 10 J=1,N  
   10      DNORM=max(abs(Q(J))+abs(E(J)),DNORM)   
           DO 200 KK=1,N
           K=N+1-KK     
C   
C     TEST FOR SPLITTING OR RANK DEFICIENCIES.. 
C         FIRST MAKE TEST FOR LAST DIAGONAL TERM, Q(K), BEING SMALL.    
   20       IF(K.EQ.1) GO TO 50     
            IF(DIFF(DNORM+Q(K),DNORM) .ne. ZERO) go to 50
C   
C     SINCE Q(K) IS SMALL WE WILL MAKE A SPECIAL PASS TO    
C     TRANSFORM E(K) TO ZERO.   
C   
           CS=ZERO  
           SN=-ONE  
                DO 40 II=2,K
                I=K+1-II
                F=-SN*E(I+1)
                E(I+1)=CS*E(I+1)    
                CALL G1 (Q(I),F,CS,SN,Q(I))     
C         TRANSFORMATION CONSTRUCTED TO ZERO POSITION (I,K).
C   
                IF (.NOT.WNTV) GO TO 40 
                     DO 30 J=1,NRV  
c
c                          Apply procedure G2 (CS,SN,V(J,I),V(J,K))  
c
                        TEMP = V(J,I)
                        V(J,I) = CS*TEMP + SN*V(J,K)
                        V(J,K) =-SN*TEMP + CS*V(J,K)
   30                continue
C              ACCUMULATE RT. TRANSFORMATIONS IN V. 
C   
   40           CONTINUE
C   
C         THE MATRIX IS NOW BIDIAGONAL, AND OF LOWER ORDER  
C         SINCE E(K) .EQ. ZERO..    
C   
   50           DO 60 LL=1,K
                  L=K+1-LL
                  IF(DIFF(DNORM+E(L),DNORM) .eq. ZERO) go to 100
                  IF(DIFF(DNORM+Q(L-1),DNORM) .eq. ZERO) go to 70
   60           CONTINUE
C     THIS LOOP CAN'T COMPLETE SINCE E(1) = ZERO.   
C   
           GO TO 100    
C   
C         CANCELLATION OF E(L), L.GT.1. 
   70      CS=ZERO  
           SN=-ONE  
                DO 90 I=L,K 
                F=-SN*E(I)  
                E(I)=CS*E(I)
                IF(DIFF(DNORM+F,DNORM) .eq. ZERO) go to 100
                CALL G1 (Q(I),F,CS,SN,Q(I))     
                IF (HAVERS) then
                     DO 80 J=1,NCC  
c
c                          Apply procedure G2 ( CS, SN, C(I,J), C(L-1,J)
c
                        TEMP = C(I,J)
                        C(I,J)   = CS*TEMP + SN*C(L-1,J)
                        C(L-1,J) =-SN*TEMP + CS*C(L-1,J)
   80                continue
                endif
   90           CONTINUE
C   
C         TEST FOR CONVERGENCE..    
  100      Z=Q(K)   
           IF (L.EQ.K) GO TO 170    
C   
C         SHIFT FROM BOTTOM 2 BY 2 MINOR OF B**(T)*B.   
           X=Q(L)   
           Y=Q(K-1)     
           G=E(K-1)     
           H=E(K)   
           F=((Y-Z)*(Y+Z)+(G-H)*(G+H))/(TWO*H*Y)
           G=sqrt(ONE+F**2) 
           IF (F .ge. ZERO) then
              T=F+G
           else
              T=F-G
           endif
           F=((X-Z)*(X+Z)+H*(Y/T-H))/X  
C   
C         NEXT QR SWEEP..   
           CS=ONE   
           SN=ONE   
           LP1=L+1  
                DO 160 I=LP1,K  
                G=E(I)  
                Y=Q(I)  
                H=SN*G  
                G=CS*G  
                CALL G1 (F,H,CS,SN,E(I-1))  
                F=X*CS+G*SN 
                G=-X*SN+G*CS
                H=Y*SN  
                Y=Y*CS  
                IF (WNTV) then
C   
C              ACCUMULATE ROTATIONS (FROM THE RIGHT) IN 'V' 
c
                     DO 130 J=1,NRV 
c                    
c                          Apply procedure G2 (CS,SN,V(J,I-1),V(J,I))
c
                        TEMP = V(J,I-1)
                        V(J,I-1) = CS*TEMP + SN*V(J,I)
                        V(J,I)   =-SN*TEMP + CS*V(J,I)
  130                continue
                endif
                CALL G1 (F,H,CS,SN,Q(I-1))  
                F=CS*G+SN*Y 
                X=-SN*G+CS*Y
                IF (HAVERS) then
                     DO 150 J=1,NCC 
c
c                          Apply procedure G2 (CS,SN,C(I-1,J),C(I,J))
c
                        TEMP = C(I-1,J)
                        C(I-1,J) = CS*TEMP + SN*C(I,J)
                        C(I,J)   =-SN*TEMP + CS*C(I,J)
  150                continue
                endif
c
C              APPLY ROTATIONS FROM THE LEFT TO 
C              RIGHT HAND SIDES IN 'C'..
C   
  160           CONTINUE
           E(L)=ZERO    
           E(K)=F   
           Q(K)=X   
           NQRS=NQRS+1  
           IF (NQRS.LE.N10) GO TO 20
C          RETURN TO 'TEST FOR SPLITTING'.  
C 
           SMALL=ABS(E(K))
           I=K     
C          IF FAILURE TO CONVERGE SET SMALLEST MAGNITUDE
C          TERM IN OFF-DIAGONAL TO ZERO.  CONTINUE ON.
C      ..           
                DO 165 J=L,K 
                TEMP=ABS(E(J))
                IF(TEMP .EQ. ZERO) GO TO 165
                IF(TEMP .LT. SMALL) THEN
                     SMALL=TEMP
                     I=J
                end if  
  165           CONTINUE
           E(I)=ZERO
           NQRS=0
           FAIL=.TRUE.  
           GO TO 20
C     ..    
C     CUTOFF FOR CONVERGENCE FAILURE. 'NQRS' WILL BE 2*N USUALLY.   
  170      IF (Z.GE.ZERO) GO TO 190 
           Q(K)=-Z  
           IF (WNTV) then
                DO 180 J=1,NRV  
  180           V(J,K)=-V(J,K)  
           endif
  190      CONTINUE     
C         CONVERGENCE. Q(K) IS MADE NONNEGATIVE..   
C   
  200      CONTINUE     
      IF (N.EQ.1) RETURN
           DO 210 I=2,N 
           IF (Q(I).GT.Q(I-1)) GO TO 220
  210      CONTINUE     
      IF (FAIL) IPASS=2 
      RETURN
C     ..    
C     EVERY SINGULAR VALUE IS IN ORDER..
  220      DO 270 I=2,N 
           T=Q(I-1)     
           K=I-1
                DO 230 J=I,N
                IF (T.GE.Q(J)) GO TO 230
                T=Q(J)  
                K=J     
  230           CONTINUE
           IF (K.EQ.I-1) GO TO 270  
           Q(K)=Q(I-1)  
           Q(I-1)=T     
           IF (HAVERS) then
                DO 240 J=1,NCC  
                T=C(I-1,J)  
                C(I-1,J)=C(K,J)     
  240           C(K,J)=T
           endif

  250      IF (WNTV) then
                DO 260 J=1,NRV  
                T=V(J,I-1)  
                V(J,I-1)=V(J,K)     
  260           V(J,K)=T
           endif
  270      CONTINUE     
C         END OF ORDERING ALGORITHM.
C   
      IF (FAIL) IPASS=2 
      RETURN
      END   
      subroutine SVA(A,MDA,M,N,MDATA,B,SING,KPVEC,NAMES,ISCALE,D,WORK)
c
C  SINGULAR VALUE ANALYSIS.  COMPUTES THE SINGULAR VALUE
C  DECOMPOSITION OF THE MATRIX OF A LEAST SQUARES PROBLEM, AND
C  PRODUCES A PRINTED REPORT.
c
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1973 JUN 12, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
C     ------------------------------------------------------------------   
c  This 1995 version differs from the original 1973 version by the 
c  addition of the arguments KPVEC() and WORK(), and by allowing user to
c  choose the length of names in NAMES().
c  KPVEC() allows the user to exercise options regarding printing.
c  WORK() provides 2*N locations of work space.  Originally SING() was
c  required to have 3*N elements, of which the last 2*N were used for
c  work space.  Now SING() only needs N elements.
c     ------------------------------------------------------------------
c                         Subroutine Arguments
c     A(,)     [inout]  On entry, contains the M x N matrix of the least
c              squares problem to be analyzed.  This could be a matrix
c              obtained by preliminary orthogonal transformations
c              applied to the actual problem matrix which may have had
c              more rows (See MDATA below.)
c
c     MDA   [in]  First dimensioning parameter for A(,).  Require
c              MDA .ge. max(M, N).
c
c     M,N   [in]  No. of rows and columns, respectively, in the
c              matrix, A.  Either M > N or M .le. N is permitted.
c              Require M > 0 and N > 0.
c
c     MDATA [in]  No. of rows in actual least squares problem.
c              Generally MDATA .ge. M.  MDATA is used only in computing
c              statistics for the report and is not used as a loop
c              count or array dimension.
c
c     B()   [inout]  On entry, contains the right-side vector, b, of the
c              least squares problem.  This vector is of length, M.
c              On return, contains the vector, g = (U**t)*b, where U
c              comes from the singular value decomposition of A.  The
c              vector , g, is also of length M.
c
c     SING()   [out] On return, contains the singular values of A, in
c              descending order, in locations indexed 1 thru min(M,N).
c              If M < N, locations indexed from M+1 through N will be
c              set to zero.
c
c     KPVEC()  [integer array, in]  Array of integers to select print
c              options.  KPVEC(1) determines whether the rest of
c              the array is to be used or ignored.
c              If KPVEC(1) = 1, the contents of (KPVEC(I), I=2,4)
c              will be used to set internal variables as follows:
c                       PRBLK = KPVEC(2)
c                       UNIT  = KPVEC(3)
c                       WIDTH = KPVEC(4)
c              If KPVEC(1) = 0 default settings will be used.  The user
c              need not dimension KPVEC() greater than 1.  The subr will
c              set PRBLK = 111111, UNIT = -1, and WIDTH = 69.
c
c              The internal variables PRBLK, UNIT, and WIDTH are
c              interpreted as follows:
c
c              PRBLK    The decimal representation of PRBLK must be
c              representable as at most 6 digits, each being 0 or 1.
c              The decimal digits will be interpreted as independant
c              on/off flags for the 6 possible blocks of printed output.
c              Examples:  111111 selects all blocks, 0 suppresses all
c              printing,  101010 selects the 1st, 3rd, and 5th blocks,
c              etc.
c              The six blocks are:
c              1. Header, with size and scaling option parameters.
c              2. V-matrix.  Amount of output depends on M and N.
c              3. Singular values and related quantities.  Amount of
c                 output depends on N.
c              4. Listing of YNORM and RNORM and their logarithms.
c                 Amount of output depends on N.
c              5. Levenberg-Marquart analysis.
c              6. Candidate solutions.  Amount of output depends on
c                 M and N.
c
c              UNIT     Selects the output unit.  If UNIT .ge. 0,
c              UNIT will be used as the output unit number.
c              If UNIT = -1, output will be written to the "*" output
c              unit, i.e., the standard system output unit.
c              The calling program unit is responsible for opening
c              and/or closing the selected output unit if the host
c              system requires these actions.
c
c        WIDTH    Default value is 79.  Determines the width of
c        blocks 2, 3, and 6 of the output report.
c        Block 3 will use 95(+1) cols if WIDTH .ge. 95, and otherwise
c        69(+1) cols.
c        Blocks 2 and 6 are printed by subroutine MFEOUT.  These blocks
c        generally use at most WIDTH(+1) cols, but will use more if
c        the names are so long that more space is needed to print one
c        name and one numeric column.  The (+1)'s above are reminders
c        that in all cases there is one extra initial column for Fortran
c        "carriage control".  The carriage control character will always
c        be a blank.
c        Blocks 1, 4, and 5 have fixed widths of 63(+1), 66(+1) and
c        66(+1), respectively.
c
c     NAMES()  [in]  NAMES(j), for j = 1, ..., N, may contain a
c              name for the jth component of the solution
c              vector.  The declared length of the elements of the
c              NAMES() array is not specifically limited, but a
c              greater length reduces the space available for columns
c              of the matris to be printed.
c              If NAMES(1) contains only blank characters,
c              it will be assumed that no names have been provided,
c              and this subr will not access the NAMES() array beyond
c              the first element.
C
C     ISCALE   [in]  Set by the user to 1, 2, or 3 to select the column
c              scaling option.
C                  1   SUBR WILL USE IDENTITY SCALING AND IGNORE THE D()
C                      ARRAY.
C                  2   SUBR WILL SCALE NONZERO COLS TO HAVE UNIT EUCLID-
C                      EAN LENGTH AND WILL STORE RECIPROCAL LENGTHS OF
C                      ORIGINAL NONZERO COLS IN D().
C                  3   USER SUPPLIES COL SCALE FACTORS IN D(). SUBR
C                      WILL MULT COL J BY D(J) AND REMOVE THE SCALING
C                      FROM THE SOLN AT THE END.
c
c     D()   [ignored or out or in]  Usage of D() depends on ISCALE as
c              described above.  When used, its length must be
c              at least N.
c
c     WORK()   [scratch]   Work space of length at least 2*N.  Used
c              directly in this subr and also in _SVDRS.
c     ------------------------------------------------------------------
      integer I, IE, IPASS, ISCALE, J, K, KPVEC(4), M, MDA, MDATA
      integer MINMN, MINMN1, MPASS, N, NSOL
      integer PRBLK, UNIT, WIDTH
      double precision A(MDA,N), A1, A2, A3, A4, ALAMB, ALN10, B(M)
      double precision D(N), DEL, EL, EL2
      double precision ONE, PCOEF, RL, RNORM, RS
      double precision SB, SING(N), SL, TEN, THOU, TWENTY
      double precision WORK(2*N), YL, YNORM, YS, YSQ, ZERO
      character*(*) NAMES(N)
      logical BLK(6), NARROW, STAR
      parameter( ZERO = 0.0d0, ONE = 1.0d0)
      parameter( TEN = 10.0d0, TWENTY = 20.0d0, THOU = 1000.0d0)
c     -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  220 format (1X/' INDEX  SING. VAL.    P COEF   ',
     *        '   RECIPROCAL    G COEF         G**2   ',
     *        '   CUMULATIVE  SCALED SQRT'/
     *    31x,'   SING. VAL.',26x,
     *        '  SUM of SQRS  of CUM.S.S.')
  221 format (1X/' INDEX  SING. VAL.    P COEF   ',
     *        '   RECIPROCAL    G COEF     SCALED SQRT'/
     *    31x,'   SING. VAL.',13x,'  of CUM.S.S.')
  222 format (1X/' INDEX  SING. VAL.    G COEF         G**2   ',
     *        '   CUMULATIVE  SCALED SQRT'/
     *    44x,'  SUM of SQRS  of CUM.S.S.')

  230 format (' ',4X,'0',64X,2g13.4)
  231 format (' ',4X,'0',51X, g13.4)
  232 format (' ',4X,'0',38X,2g13.4)

  240 format (' ',i5,g12.4,6g13.4)

  260 format (1X,' M  = ',I6,',   N  =',I4,',   MDATA  =',I8)
  270 format (1X/' Singular Value Analysis of the least squares',
     *        ' problem,  A*X = B,'/
     *        ' scaled as (A*D)*Y = B.')
  280 format (1X/' Scaling option No.',I2,'.  D is a diagonal',
     * ' matrix with the following diagonal elements..'/(5X,10E12.4))
  290 format (1X/' Scaling option No. 1.   D is the identity matrix.'/
     * 1X)
  300 format (1X/' INDEX',12X,'YNORM      RNORM',11X,
     * '      LOG10      LOG10'/
     * 45X,'      YNORM      RNORM'/1X)
  310 format (' ',I5,6X,2E11.3,11X,2F11.3)
  320 format (1X/
     *' Norms of solution and residual vectors for a range of values'/
     *' of the Levenberg-Marquardt parameter, LAMBDA.'//
     *        '      LAMBDA      YNORM      RNORM',
     *         '      LOG10      LOG10      LOG10'/
     *     34X,'     LAMBDA      YNORM      RNORM')
  330 format (1X, 3E11.3, 3F11.3)
c     ------------------------------------------------------------------
      IF (M.LE.0 .OR. N.LE.0) RETURN
      MINMN = min(M,N)
      MINMN1 = MINMN + 1
      if(KPVEC(1) .eq. 0) then
         PRBLK = 111111
         UNIT = -1
         WIDTH = 79
      else
         PRBLK = KPVEC(2)
         UNIT = KPVEC(3)
         WIDTH = KPVEC(4)
      endif
      STAR = UNIT .lt. 0
c                           Build logical array BLK() by testing
c                           decimal digits of PRBLK.
      do 20 I=6, 1, -1
         J = PRBLK/10
         BLK(I) = (PRBLK - 10*J) .gt. 0
         PRBLK = J
   20 continue
c     -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
c                         Optionally print header and M, N, MDATA
      if(BLK(1)) then
         if(STAR) then
            write (*,270)
            write (*,260) M,N,MDATA
         else
            write (UNIT,270)
            write (UNIT,260) M,N,MDATA
         endif
      endif
c     -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
c                                  Handle scaling as selected by ISCALE.
      if( ISCALE .eq. 1) then
         if(BLK(1)) then
            if(STAR) then
               write (*,290)
            else
               write (UNIT,290)
            endif
         endif
      else
C
C                                  Apply column scaling to A.
C
         DO 52 J = 1,N
            A1 = D(J)
            if( ISCALE .le. 2) then
               SB = ZERO
               DO 30 I = 1,M
   30             SB = SB + A(I,J)**2
               A1 = sqrt(SB)
               IF (A1.EQ.ZERO) A1 = ONE
               A1 = ONE/A1
               D(J) = A1
            endif
            DO 50 I = 1,M
               A(I,J) = A(I,J)*A1
   50       continue
   52    continue
         if(BLK(1)) then
            if(STAR) then
               write (*,280) ISCALE,(D(J),J = 1,N)
            else
               write (UNIT,280) ISCALE,(D(J),J = 1,N)
            endif
         endif
      endif
c     -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C         Compute the Singular Value Decomposition of the scaled matrix.
C
      call SVDRS (A,MDA,M,N,B,M,1,SING,WORK)
c
c                                               Determine NSOL.
      NSOL = MINMN
      do 60 J = 1,MINMN
         if(SING(J) .eq. ZERO) then
            NSOL = J-1
            go to 65
         endif
   60 continue
   65 continue
C
c              The array B() contains the vector G.
C              Compute cumulative sums of squares of components of
C              G and store them in WORK(I), I = 1,...,MINMN+1
C
      SB = ZERO
      DO 70 I = MINMN1,M
         SB = SB + B(I)**2
   70 CONTINUE
      WORK(MINMN+1) = SB
      DO  75 J = MINMN, 1, -1
         SB = SB + B(J)**2
         WORK(J) = SB
   75 CONTINUE
c     -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C                                          PRINT THE V MATRIX.
C
      if(BLK(2)) CALL MFEOUT (A,MDA,N,N,NAMES,1, UNIT, WIDTH)
c     -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C                                REPLACE V BY D*V IN THE ARRAY A()
      if (ISCALE .gt.1) then
         do 82 I = 1,N
            do 80 J = 1,N
               A(I,J) = D(I)*A(I,J)
   80       continue
   82    continue
      endif
c     -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      if(BLK(3)) then
c                     Print singular values and other summary results.
c
c     Output will be done using one of two layouts.  The narrow
c     layout uses 69 cols + 1 for carriage control, and makes two passes
c     through the computation.
c     The wide layout uses 95 cols + 1 for carriage control, and makes
c     only one pass through the computation.
c
C                       G  NOW IN  B() ARRAY.  V NOW IN A(,) ARRAY.
C
      NARROW = WIDTH .lt. 95
      MPASS = 1
      if(NARROW) MPASS = 2
      do 170 IPASS = 1, MPASS
         if(STAR) then
            if(NARROW) then
               if(IPASS .eq. 1) then
                  write(*,221)
               else
                  write(*,222)
               endif
            else
               write (*,220)
            endif
         else
            if(NARROW) then
               if(IPASS .eq. 1) then
                  write(UNIT,221)
               else
                  write(UNIT,222)
               endif
            else
               write (UNIT,220)
            endif
         endif
c                               The following stmt converts from
c                               integer to floating-point.
      A3 = WORK(1)
      A4 = sqrt(A3/ max(1,MDATA))
      if(STAR) then
         if(NARROW) then
            if(IPASS .eq. 1) then
               write(*,231) A4
            else
               write(*,232) A3, A4
            endif
         else
            write (*,230) A3,A4
         endif
      else
         if(NARROW) then
            if(IPASS .eq. 1) then
               write(UNIT,231) A4
            else
               write(UNIT,232) A3, A4
            endif
         else
            write (UNIT,230) A3,A4
         endif
      endif
C
      DO 160 K = 1,MINMN
         if (SING(K).EQ.ZERO) then
            PCOEF = ZERO
            if(STAR) then
               write (*,240) K,SING(K)
            else
               write (UNIT,240) K,SING(K)
            endif
         else
            PCOEF = B(K) / SING(K)
            A1 = ONE / SING(K)
            A2 = B(K)**2
            A3 = WORK(K+1)
            A4 = sqrt(A3/max(1,MDATA-K))
            if(STAR) then
               if(NARROW) then
                  if(IPASS .eq. 1) then
                     write(*,240) K,SING(K),PCOEF,A1,B(K),      A4
                  else
                     write(*,240) K,SING(K),         B(K),A2,A3,A4
                  endif
               else
                  write (*,240) K,SING(K),PCOEF,A1,B(K),A2,A3,A4
               endif
            else
               if(NARROW) then
                  if(IPASS .eq. 1) then
                     write(UNIT,240) K,SING(K),PCOEF,A1,B(K),      A4
                  else
                     write(UNIT,240) K,SING(K),         B(K),A2,A3,A4
                  endif
               else
                  write (UNIT,240) K,SING(K),PCOEF,A1,B(K),A2,A3,A4
               endif

            endif
         endif
  160 continue
  170 continue
      endif
c     -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      if( BLK(4) ) then
C
C         Compute and print values of YNORM, RNORM and their logarithms.
C
      if(STAR) then
         write (*,300)
      else
         write (UNIT,300)
      endif
      YSQ = ZERO
      do 180 J = 0, NSOL
         if(J .ne. 0) YSQ = YSQ + (B(J) / SING(J))**2
         YNORM = sqrt(YSQ)
         RNORM = sqrt(WORK(J+1))
         YL = -THOU
         IF (YNORM .GT. ZERO) YL = log10(YNORM)
         RL = -THOU
         IF (RNORM .GT. ZERO) RL = log10(RNORM)
         if(STAR) then
            write (*,310) J,YNORM,RNORM,YL,RL
         else
            write (UNIT,310) J,YNORM,RNORM,YL,RL
         endif
  180 continue
      endif
c     -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      if( BLK(5) .and. SING(1) .ne. ZERO ) then
C
C     COMPUTE VALUES OF XNORM AND RNORM FOR A SEQUENCE OF VALUES OF
C     THE LEVENBERG-MARQUARDT PARAMETER.
C
         EL = log10(SING(1)) + ONE
         EL2 = log10(SING(NSOL)) - ONE
         DEL = (EL2-EL) / TWENTY
         ALN10 = log(TEN)
         if(STAR) then
            write (*,320)
         else
            write (UNIT,320)
         endif
         DO 200 IE = 1,21
C                                    COMPUTE        ALAMB = 10.0**EL
            ALAMB = EXP(ALN10*EL)
            YS = ZERO
            RS = WORK(NSOL+1)
            DO 190 I = 1,MINMN
               SL = SING(I)**2 + ALAMB**2
               YS = YS + (B(I)*SING(I)/SL)**2
               RS = RS + (B(I)*(ALAMB**2)/SL)**2
  190       CONTINUE
            YNORM = sqrt(YS)
            RNORM = sqrt(RS)
            RL = -THOU
            IF (RNORM.GT.ZERO) RL = log10(RNORM)
            YL = -THOU
            IF (YNORM.GT.ZERO) YL = log10(YNORM)
            if(STAR) then
               write (*,330) ALAMB,YNORM,RNORM,EL,YL,RL
            else
               write (UNIT,330) ALAMB,YNORM,RNORM,EL,YL,RL
            endif
            EL = EL + DEL
  200    CONTINUE
      endif
c     -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C                   Compute and optionally print candidate solutions.
C
      do 215 K = 1,NSOL
         PCOEF = B(K) / SING(K)
         DO 210 I = 1,N
                A(I,K) = A(I,K) * PCOEF
  210           IF (K.GT.1) A(I,K) = A(I,K) + A(I,K-1)
  215 continue
      if (BLK(6) .and. NSOL.GE.1)
     *       CALL MFEOUT (A,MDA,N,NSOL,NAMES,2,UNIT,WIDTH)
      return
      END
C     SUBROUTINE SVDRS (A, MDA, M1, N1, B, MDB, NB, S, WORK) 
c
C  SINGULAR VALUE DECOMPOSITION ALSO TREATING RIGHT SIDE VECTOR.
c
c  The original version of this code was developed by
c  Charles L. Lawson and Richard J. Hanson at Jet Propulsion Laboratory
c  1974 SEP 25, and published in the book
c  "SOLVING LEAST SQUARES PROBLEMS", Prentice-HalL, 1974.
c  Revised FEB 1995 to accompany reprinting of the book by SIAM.
C     ------------------------------------------------------------------   
c  This 1995 version differs from the original 1974 version by adding
c  the argument WORK().
c  WORK() provides 2*N1 locations of work space.  Originally S() was
c  required to have 3*N1 elements, of which the last 2*N1 were used for
c  work space.  Now S() only needs N1 elements.
C     ------------------------------------------------------------------
c  This subroutine computes the singular value decomposition of the
c  given M1 x N1 matrix, A, and optionally applys the transformations
c  from the left to the NB column vectors of the M1 x NB matrix B.
c  Either M1 .ge. N1  or  M1 .lt. N1 is permitted.
c
c       The singular value decomposition of A is of the form
c
c                  A  =  U * S * V**t
c
c  where U is M1 x M1 orthogonal, S is M1 x N1 diagonal with the
c  diagonal terms nonnegative and ordered from large to small, and
c  V is N1 x N1 orthogonal.  Note that these matrices also satisfy
c
c                  S = (U**t) * A * V
c
c       The matrix V is returned in the leading N1 rows and
c  columns of the array A(,).
c
c       The singular values, i.e. the diagonal terms of the matrix S,
c  are returned in the array S().  If M1 .lt. N1, positions M1+1
c  through N1 of S() will be set to zero.
c
c       The product matrix  G = U**t * B replaces the given matrix B
c  in the array B(,).
c
c       If the user wishes to obtain a minimum length least squares
c  solution of the linear system
c
c                        A * X ~=~ B
c
c  the solution X can be constructed, following use of this subroutine,
c  by computing the sum for i = 1, ..., R of the outer products
c
c          (Col i of V) * (1/S(i)) * (Row i of G)
c
c  Here R denotes the pseudorank of A which the user may choose
c  in the range 0 through Min(M1, N1) based on the sizes of the
c  singular values.
C     ------------------------------------------------------------------
C                    Subroutine Arguments
c
c  A(,)     (In/Out)  On input contains the M1 x N1 matrix A.
c           On output contains the N1 x N1 matrix V.
c
c  LDA      (In)  First dimensioning parameter for A(,).
c           Require LDA .ge. Max(M1, N1).
c
c  M1       (In)  No. of rows of matrices A, B, and G.
c           Require M1 > 0.
c
c  N1       (In)  No. of cols of matrix A, No. of rows and cols of
c           matrix V.  Permit M1 .ge. N1  or  M1 .lt. N1.
c           Require N1 > 0.
c
c  B(,)     (In/Out)  If NB .gt. 0 this array must contain an
c           M1 x NB matrix on input and will contain the
c           M1 x NB product matrix, G = (U**t) * B on output.
c
c  LDB      (In)  First dimensioning parameter for B(,).
c           Require LDB .ge. M1.
c
c  NB       (In)  No. of cols in the matrices B and G.
c           Require NB .ge. 0.
c
c  S()      (Out)  Must be dimensioned at least N1.  On return will
c           contain the singular values of A, with the ordering
c                S(1) .ge. S(2) .ge. ... .ge. S(N1) .ge. 0.
c           If M1 .lt. N1 the singular values indexed from M1+1
c           through N1 will be zero.
c           If the given integer arguments are not consistent, this
c           subroutine will return immediately, setting S(1) = -1.0.
c
c  WORK()  (Scratch)  Work space of total size at least 2*N1.
c           Locations 1 thru N1 will hold the off-diagonal terms of
c           the bidiagonal matrix for subroutine QRBD.  Locations N1+1
c           thru 2*N1 will save info from one call to the next of
c           H12.
c     ------------------------------------------------------------------
C  This code gives special treatment to rows and columns that are
c  entirely zero.  This causes certain zero sing. vals. to appear as
c  exact zeros rather than as about MACHEPS times the largest sing. val.
c  It similarly cleans up the associated columns of U and V.  
c
c  METHOD..  
c   1. EXCHANGE COLS OF A TO PACK NONZERO COLS TO THE LEFT.  
c      SET N = NO. OF NONZERO COLS.  
c      USE LOCATIONS A(1,N1),A(1,N1-1),...,A(1,N+1) TO RECORD THE    
c      COL PERMUTATIONS. 
c   2. EXCHANGE ROWS OF A TO PACK NONZERO ROWS TO THE TOP.   
c      QUIT PACKING IF FIND N NONZERO ROWS.  MAKE SAME ROW EXCHANGES 
c      IN B.  SET M SO THAT ALL NONZERO ROWS OF THE PERMUTED A   
c      ARE IN FIRST M ROWS.  IF M .LE. N THEN ALL M ROWS ARE 
c      NONZERO.  IF M .GT. N THEN      THE FIRST N ROWS ARE KNOWN    
c      TO BE NONZERO,AND ROWS N+1 THRU M MAY BE ZERO OR NONZERO.     
c   3. APPLY ORIGINAL ALGORITHM TO THE M BY N PROBLEM.   
c   4. MOVE PERMUTATION RECORD FROM A(,) TO S(I),I=N+1,...,N1.   
c   5. BUILD V UP FROM  N BY N  TO  N1 BY N1  BY PLACING ONES ON     
c      THE DIAGONAL AND ZEROS ELSEWHERE.  THIS IS ONLY PARTLY DONE   
c      EXPLICITLY.  IT IS COMPLETED DURING STEP 6.   
c   6. EXCHANGE ROWS OF V TO COMPENSATE FOR COL EXCHANGES OF STEP 2. 
c   7. PLACE ZEROS IN  S(I),I=N+1,N1  TO REPRESENT ZERO SING VALS.   
c     ------------------------------------------------------------------
      subroutine SVDRS (A, MDA, M1, N1, B, MDB, NB, S, WORK) 
      integer I, IPASS, J, K, L, M, MDA, MDB, M1
      integer N, NB, N1, NP1, NS, NSP1
c     double precision A(MDA,N1),B(MDB,NB), S(N1)
      double precision A(MDA, *),B(MDB, *), S( *)
      double precision ONE, T, WORK(N1,2), ZERO
      parameter(ONE = 1.0d0, ZERO = 0.0d0)
c     -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C                             BEGIN.. SPECIAL FOR ZERO ROWS AND COLS.   
C   
C                             PACK THE NONZERO COLS TO THE LEFT 
C   
      N=N1  
      IF (N.LE.0.OR.M1.LE.0) RETURN 
      J=N   
   10 CONTINUE  
         DO 20 I=1,M1  
            IF (A(I,J) .ne. ZERO) go to 50
   20    CONTINUE  
C   
C           COL J  IS ZERO. EXCHANGE IT WITH COL N.   
C   
         IF (J .ne. N) then
            DO 30 I=1,M1  
   30       A(I,J)=A(I,N) 
         endif
         A(1,N)=J  
         N=N-1 
   50    CONTINUE  
         J=J-1 
      IF (J.GE.1) GO TO 10  
C                             IF N=0 THEN A IS ENTIRELY ZERO AND SVD    
C                             COMPUTATION CAN BE SKIPPED    
      NS=0  
      IF (N.EQ.0) GO TO 240 
C                             PACK NONZERO ROWS TO THE TOP  
C                             QUIT PACKING IF FIND N NONZERO ROWS   
      I=1   
      M=M1  
   60 IF (I.GT.N.OR.I.GE.M) GO TO 150   
      IF (A(I,I)) 90,70,90  
   70     DO 80 J=1,N   
          IF (A(I,J)) 90,80,90  
   80     CONTINUE  
      GO TO 100 
   90 I=I+1 
      GO TO 60  
C                             ROW I IS ZERO     
C                             EXCHANGE ROWS I AND M 
  100 IF(NB.LE.0) GO TO 115 
          DO 110 J=1,NB 
          T=B(I,J)  
          B(I,J)=B(M,J) 
  110     B(M,J)=T  
  115     DO 120 J=1,N  
  120     A(I,J)=A(M,J) 
      IF (M.GT.N) GO TO 140 
          DO 130 J=1,N  
  130     A(M,J)=ZERO   
  140 CONTINUE  
C                             EXCHANGE IS FINISHED  
      M=M-1 
      GO TO 60  
C   
  150 CONTINUE  
C                             END.. SPECIAL FOR ZERO ROWS AND COLUMNS   
C                             BEGIN.. SVD ALGORITHM 
C     METHOD..  
C     (1)     REDUCE THE MATRIX TO UPPER BIDIAGONAL FORM WITH   
C     HOUSEHOLDER TRANSFORMATIONS.  
C          H(N)...H(1)AQ(1)...Q(N-2) = (D**T,0)**T  
C     WHERE D IS UPPER BIDIAGONAL.  
C   
C     (2)     APPLY H(N)...H(1) TO B.  HERE H(N)...H(1)*B REPLACES B    
C     IN STORAGE.   
C   
C     (3)     THE MATRIX PRODUCT W= Q(1)...Q(N-2) OVERWRITES THE FIRST  
C     N ROWS OF A IN STORAGE.   
C   
C     (4)     AN SVD FOR D IS COMPUTED.  HERE K ROTATIONS RI AND PI ARE 
C     COMPUTED SO THAT  
C          RK...R1*D*P1**(T)...PK**(T) = DIAG(S1,...,SM)    
C     TO WORKING ACCURACY.  THE SI ARE NONNEGATIVE AND NONINCREASING.   
C     HERE RK...R1*B OVERWRITES B IN STORAGE WHILE  
C     A*P1**(T)...PK**(T)  OVERWRITES A IN STORAGE. 
C   
C     (5)     IT FOLLOWS THAT,WITH THE PROPER DEFINITIONS,  
C     U**(T)*B OVERWRITES B, WHILE V OVERWRITES THE FIRST N ROW AND     
C     COLUMNS OF A.     
C   
      L=min(M,N)   
C             THE FOLLOWING LOOP REDUCES A TO UPPER BIDIAGONAL AND  
C             ALSO APPLIES THE PREMULTIPLYING TRANSFORMATIONS TO B.     
C   
          DO 170 J=1,L  
          IF (J.GE.M) GO TO 160     
          CALL H12 (1,J,J+1,M,A(1,J),1,T,A(1,J+1),1,MDA,N-J)
          CALL H12 (2,J,J+1,M,A(1,J),1,T,B,1,MDB,NB)
  160     IF (J.GE.N-1) GO TO 170   
          CALL H12 (1,J+1,J+2,N,A(J,1),MDA,work(J,2),A(J+1,1),MDA,1,M-J)   
  170     CONTINUE  
C   
C     COPY THE BIDIAGONAL MATRIX INTO S() and WORK() FOR QRBD.   
C 1986 Jan 8. C. L. Lawson. Changed N to L in following 2 statements.
      IF (L.EQ.1) GO TO 190 
          DO 180 J=2,L  
          S(J)=A(J,J) 
  180     WORK(J,1)=A(J-1,J)   
  190 S(1)=A(1,1)     
C   
      NS=N  
      IF (M.GE.N) GO TO 200 
      NS=M+1
      S(NS)=ZERO  
      WORK(NS,1)=A(M,M+1)  
  200 CONTINUE  
C   
C     CONSTRUCT THE EXPLICIT N BY N PRODUCT MATRIX, W=Q1*Q2*...*QL*I    
C     IN THE ARRAY A(). 
C   
          DO 230 K=1,N  
          I=N+1-K   
          IF (I .GT. min(M,N-2)) GO TO 210     
          CALL H12 (2,I+1,I+2,N,A(I,1),MDA,WORK(I,2),A(1,I+1),1,MDA,N-I)   
  210         DO 220 J=1,N  
  220         A(I,J)=ZERO   
  230     A(I,I)=ONE    
C   
C          COMPUTE THE SVD OF THE BIDIAGONAL MATRIX 
C   
      CALL QRBD (IPASS,S(1),WORK(1,1),NS,A,MDA,N,B,MDB,NB)   
C   
      if(IPASS .eq. 2) then
         write (*,'(/a)')
     *      ' FULL ACCURACY NOT ATTAINED IN BIDIAGONAL SVD'
      endif

  240 CONTINUE  
      IF (NS.GE.N) GO TO 260
      NSP1=NS+1 
          DO 250 J=NSP1,N   
  250     S(J)=ZERO   
  260 CONTINUE  
      IF (N.EQ.N1) RETURN   
      NP1=N+1   
C                                  MOVE RECORD OF PERMUTATIONS  
C                                  AND STORE ZEROS  
          DO 280 J=NP1,N1   
          S(J)=A(1,J) 
              DO 270 I=1,N  
  270         A(I,J)=ZERO   
  280     CONTINUE  
C                             PERMUTE ROWS AND SET ZERO SINGULAR VALUES.
          DO 300 K=NP1,N1   
          I=S(K)  
          S(K)=ZERO   
              DO 290 J=1,N1 
              A(K,J)=A(I,J) 
  290         A(I,J)=ZERO   
          A(I,K)=ONE    
  300     CONTINUE  
C                             END.. SPECIAL FOR ZERO ROWS AND COLUMNS   
      RETURN
      END   
