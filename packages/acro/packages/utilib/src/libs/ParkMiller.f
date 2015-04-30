
C    Presented below is a FORTRAN 77 adaptation of one of the Pascal codes
C    that appears in [8].  This code is portable to any machine that has a
C    maximum integer greater than, or equal to, 2**31-1.  Thus, this code
C    should run on any 32-bit machine.  The code meets all of the
C    requirements of the "minimal standard" as described in [8].  <p>
C 
C    We ran it on all of the systems mentioned in this article.  In general, 
C    the execution times for running the portable code were 2 to 50 times
C    slower  than the random number generator supplied by the vendor.
C    For versions of  random number generators that implement the minimal
C    standard on machines  with a maximum integer less than 2**31-1, or
C    generators that are more  specific to machines with 64-bit arithmetic,
C    see [8].

      SUBROUTINE SRAND(ISEED)
C
C  This subroutine sets the integer seed to be used with the
C  companion RAND function to the value of ISEED.  A flag is
C  set to indicate that the sequence of pseudo-random numbers
C  for the specified seed should start from the beginning.
C
      COMMON /SEED/JSEED,IFRST
C
      JSEED = ISEED
      IFRST = 0
C
      RETURN
      END
      REAL FUNCTION RAND()
C
C  This function returns a pseudo-random number for each invocation.
C  It is a FORTRAN 77 adaptation of the "Integer Version 2" minimal
C  standard number generator whose Pascal code appears in the article:
C
C     Park, Steven K. and Miller, Keith W., "Random Number Generators:
C     Good Ones are Hard to Find", Communications of the ACM,
C     October, 1988.
C
      PARAMETER (MPLIER=16807,MODLUS=2147483647,MOBYMP=127773,
     +           MOMDMP=2836)
C
      COMMON  /SEED/JSEED,IFRST
      INTEGER HVLUE, LVLUE, TESTV, NEXTN
      SAVE    NEXTN
C
      IF (IFRST .EQ. 0) THEN
        NEXTN = JSEED
        IFRST = 1
      ENDIF
C
      HVLUE = NEXTN / MOBYMP
      LVLUE = MOD(NEXTN, MOBYMP)
      TESTV = MPLIER*LVLUE - MOMDMP*HVLUE
      IF (TESTV .GT. 0) THEN
        NEXTN = TESTV
      ELSE
        NEXTN = TESTV + MODLUS
      ENDIF
      RAND = REAL(NEXTN)/REAL(MODLUS)
C
      RETURN
      END
      BLOCKDATA RANDBD
      COMMON /SEED/JSEED,IFRST
C
      DATA JSEED,IFRST/123456789,0/
C
      END
