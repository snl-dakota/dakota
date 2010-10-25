*   _______________________________________________________________________
*
*   DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
*   Copyright (c) 2006, Sandia National Laboratories.
*   This software is distributed under the GNU Lesser General Public License.
*   For more information, see the README file in the top Dakota directory.
*   _______________________________________________________________________
*
      PROGRAM RNKCYC
C
      COMMON/A/ P(30),POWEL,OPTIM
      DIMENSION N(30)
      CHARACTER*6 BOTLIN
C!
      OPEN (UNIT=5, FILE='rsmas.in')
      OPEN (UNIT=6, FILE='rsmas.out')
C!!
      PRINT *,' THIS PROGRAM MODELS A RANKINE CYCLE SPACE POWER'
      PRINT *,' SYSTEM.  THE SYSTEM CONSISTS OF A LIQUID METAL'
      PRINT *,' COOLED REACTOR, A TURBINE, A GENERATOR'
      PRINT *,' A RADIATOR, AND POWER CONDITIONING.'
      PRINT *,' THREE TYPES OF CYCLES ARE COVERED:'
      PRINT *,'    1. A DIRECT CYCLE WITH NO SUPERHEAT, A VAPOR'
      PRINT *,'       SEPARATOR, AND LITHIUM AS A WORKING FLUID.'
      PRINT *,'    2. AND AN INDIRECT CYCLE WITH SUPERHEAT, A HEAT-'
      PRINT *,'       EXCHANGER, POTASSIUM AS A WORKING FLUID AND'
      PRINT *,'       LITHIUM AS A REACTOR COOLANT.'
      PRINT *,' CONDENSER TEMPERATURE IS OPTIMIZED'
      PRINT *,' TO GET EITHER MINIMUM SYSTEM WEIGHT OR COST.'
C
      PRINT 1
    1 FORMAT(/)
      PRINT 1
C
      PRINT *,' ENTER VALUES FOR ELECTRICAL POWER IN MW AND'
      PRINT *,' OPERATING TIME IN HOURS.   '
C!      READ *,POWEL,OPTIM
      POWEL = 5
      OPTIM = 20000
C!!
      PRINT *,' ENTER "WEIGHT" OR "COST" AS PARAMETER TO BE'
      PRINT *,'   MINIMIZED (IN CAPITAL LETTERS).   '
C!      READ 2,BOTLIN
      BOTLIN = 'WEIGHT'
C!!
    2 FORMAT(A6)
C
      PRINT *,' THE FOLLOWING ARE DEFAULT PARAMETER VALUES.'
      PRINT *,' THEY MAY BE CHANGED IF YOU WISH.'
      PRINT 1
C
      P(1)=1200.
      P(2)=1
      P(3)=1.
      P(4)=.95
      P(5)=.95
      P(6)=.93
      P(7)=18.
      P(8)=12170.
      P(9)=1.0
      P(10)=.28
      P(11)=.15E6
      P(12)=.07
      P(13)=0.
C! MODERATOR MOLECULRA WT = 39 FOR K
      P(14)=39.
C!!
      P(15)=1.
      P(16)=1.43
      P(17)=8300.
      P(18)=0.84
      P(19)=.233
      P(20)=5.E16
      P(21)=25.
      P(22)=15.
      P(23)=2
      P(24)=1.E07
      P(25)=4
      P(26)=1
      P(27)=10000.
      P(28)=1.5
      P(29)=0
      P(30)=.88
C
   50 PRINT *,' CYCLE PARAMETERS:'
      PRINT *,'      1. ',P(1),' TURBINE INLET TEMP, K'
      PRINT *,'      2. ',P(2),' CYCLE TYPE  1-DIRECT, 2-INDIRECT'
      PRINT *,'      3. ',P(3),' TURBINE INLET/SATURATION PRESSURE'
      PRINT *,'      4. ',P(4),' GENERATOR EFFICIENCY'
      PRINT *,'      5. ',P(5),' POWER CONDITIONING EFFICIENCY'
      PRINT 1
      PRINT *,' REACTOR PARAMETERS:'
      PRINT *,'      6. ',P(6),' FRACTIONAL FUEL ENRICHMENT'
      PRINT *,'      7. ',P(7),' CRITICAL COMPACT MASS, Kg'
      PRINT *,'      8. ',P(8),' FUEL DENSITY, kg/m3'
      PRINT *,'      9. ',P(9),' CRITICAL MASS CORRECTION FACT'
      PRINT *,'     10. ',P(10),' FUEL + MODERATOR VOL FRACT'
      PRINT *,'     11. ',P(11),' FUEL POWER DENSITY, W/kg'
      PRINT *,'     12. ',P(12),' FUEL BURNUP FRACTION LIMIT'
      PRINT *,'     13. ',P(13),' MODERATOR-TO-FUEL RATIO'
      PRINT *,'     14. ',P(14),' MODERATOR MOLECULAR WEIGHT'
      PRINT *,'     15. ',P(15),' MODERATOR DENSITY, Kg/m3'
      PRINT *,'     16. ',P(16),' STRUCTURE TO FUEL & MOD RATIO'
      PRINT *,'     17. ',P(17),' STRUCTURE DENSITY, kg/m3'
      PRINT *,'     18. ',P(18),' CORE REMOVAL X-SECTION, cm-1'
      PRINT *,'     19. ',P(19),' CORE GAMMA ATTEN X-SECT, cm-1'
      PRINT *,'     20. ',P(20),' ALLOWED PAYLOAD NEUTRON DOSE, nvt'
      PRINT *,'     21. ',P(21),' PAYLOAD SEPARATION DISTANCE, m'
      PRINT *,'     22. ',P(22),' PROTECTION CONE HALF ANGLE, DEG'
      PRINT *,'     23. ',P(23),' NEUTRON SHIELD MATL-B4C=1, LIH=2'
      PRINT *,'     24. ',P(24),' ALLOWED PAYLOAD GAMMA DOSE, R'
      PRINT 1
      PRINT *,' TURBINE PARAMETERS'
      PRINT *,'     25. ',P(25),' NUMBER OF TURBINES'
      PRINT *,'     26. ',P(26),' TURBINE MATERIAL  1-Ni, 2-Ta'
      PRINT *,'     27. ',P(27),' TURBINE SPEED,  RPM'
      PRINT *,'     28. ',P(28),' TURBINE WORK COEFFICIENT'
      PRINT *,'     29. ',P(29),' DISK COOL PARAM,  0-NO, 1-YES'
      PRINT 1
      PRINT *,' RADIATOR PARAMETERS:'
      PRINT *,'     30. ',P(30),' RADIATOR EMITTANCE'
      PRINT 1
      PRINT 1
C
      PRINT *,' ENTER THE NUMBER OF PARAMETERS YOU WISH TO CHANGE.   '
C!      READ *,NCHANGE
      NCHANGE = 0
C      IF(NCHANGE.EQ.0) GO TO 60
C      PRINT *,' ENTER THE ',NCHANGE,' PARAMETER NUMBERS.   '
C      READ *,(N(I),I=1,NCHANGE)
C      PRINT *,' ENTER THE ',NCHANGE,' PARAMETER VALUES.   '
C      READ *,(P(N(I)),I=1,NCHANGE)
C      PRINT 1
C!!      GO TO 50
C!
      READ(5,111) x1
      READ(5,111) x2
      READ(5,111) x3
      READ(5,112) i4
      P(6)=x1
      P(13)=x2
      P(22)=x3
      P(25)=i4
  111 FORMAT(10X,E22.16)
  112 FORMAT(10X,I22)
      PRINT *,'      6. ',P(6),' FRACTIONAL FUEL ENRICHMENT'
      PRINT *,'     13. ',P(13),' MODERATOR-TO-FUEL RATIO'
      PRINT *,'     22. ',P(22),' PROTECTION CONE HALF ANGLE, DEG'
      PRINT *,'     25. ',P(25),' NUMBER OF TURBINES'
C!!
   60 CONTINUE
C
C THIS SECTION DIRECTS THE ITERATION ON CONDENSER TEMPERATURE
C TO FIND A MINIMUN WEIGHT OR COST SYSTEM.
C
      PRINT *,'   TURBINE CONDENSER     CYCLE  RCT+SHLD  POW CONV  RADIA
     1TOR     TOTAL     TOTAL'
      PRINT *,'  TEMP (K)  TEMP (K)     EFFIC   WT (Kg)   WT (Kg)   WT (
     1Kg)   WT (Kg) COST (M$)'
      PRINT 1
C
      TC=P(1)-50.
      FN=1.E12
      TCOPT=TC
      FMIN=FN
  160 TC=TC-25
      FN=XMEAS(TC,BOTLIN,0)
      IF(FN.GE.FMIN) GO TO 190
      FMIN=FN
      TCOPT=TC
  190 IF(TC.GE.325) GO TO 160
      TC=TCOPT
      FN=XMEAS(TC,BOTLIN,1)
C
      STOP
      END
C
C
C*****************************************************************
C
      FUNCTION XMEAS(TC,BOTLIN,IPR)
C
      COMMON/A/ P(30),POWEL,OPTIM
      CHARACTER*6 BOTLIN
C
C
      TTI=P(1)
      CYCTYP=P(2)
      FP=P(3)
      EFFGEN=P(4)
      EFFPC=P(5)
      ENRCH=P(6)
      XMCRTC=P(7)
      DNSFUL=P(8)
      CM=P(9)
      VOLFRC=P(10)
      PS=P(11)
      BRNLIM=P(12)
      RMODFUL=P(13)
      XMMWT=P(14)
      DNSMOD=P(15)
      RV=P(16)
      DNSSTR=P(17)
      CORREMX=P(18)
      CORGAMX=P(19)
      DOSN=P(20)
      RPAYLD=P(21)
      HANG=P(22)
      MTLNSH=P(23)
      DOSGAM=P(24)
      NT=P(25)
      M=P(26)
      OMEG=P(27)
      PHI=P(28)
      NDCOOL=P(29)
      EMITT=P(30)
C
C
C THE FOLLOWING MODELS RANKINE CYCLE PERFORMANCE WITH AN ALKALINE
C METAL AS THE WORKING FLUID.
C
      CPL=830.
      NCT=CYCTYP
      GO TO (10,50),NCT
C
C
C  THIS SECTION MODELS A DIRECT CONDENSING CYCLE WITH POTASSIUM
C  AS THE WORKING FLUID.
C
   10 TB=TTI
      TRO=TTI
C
      GO TO (20,30),M
   20    F=.03
         IF (PHI.GT.1.5) F=.14
         EFFTRB=.842+.21*(TC/900.-1.)+.078*(1.-FP)
     1          +.12*(1.-TTI/1200.)+F*(1.-PHI/1.5)
         GO TO 40
   30    F=.017
         IF (PHI.GT.1.5) F=.12
         EFFTRB=.857+.155*(TC/1050.-1.)+.11*(1.-FP)
     1          +.137*(1.-TTI/1400.)+F*(1.-PHI/1.5)
C
   40 EFFP=1.-2.*TC/(TRO+TC)
      EFFB=(TRO-TC)/TRO
      HP=(TRO-TC)*CPL
      HB=2.57E6-640*TRO
      EFFCYC=EFFTRB*(HP*EFFP+HB*EFFB)/(HP+HB)
      POWTH=POWEL/(EFFCYC*EFFGEN*EFFPC)
      QRAD=POWTH-POWEL/(EFFGEN*EFFPC)
      QTUR=POWTH*EFFCYC
      FLRTT=POWTH*1.E6/(HP+HB)
      FLRTR=FLRTT
      PTI=1320.*EXP(-9760./TRO)
      PRCTR=PTI
      PC=1320.*EXP(-9760./TC)
      TRI=TC
      GO TO 100
C
C
C  THIS SECTION MODELS AN INDIRECT SUPERHEAT CYCLE WITH LITHIUM
C  AS A REACTOR COOLANT AND POTASSIUM AS A CYCLE WORKING FLUID.
C
   50 TB=9760.*TTI/(9760.-TTI*ALOG(FP))
         S0=2.6/(ALOG(TB)-5.687)
         S=S0+ALOG(TTI/TB)/1.06
      TCMAX=295.*EXP(2.6/S)
         IF (TC.LE.TCMAX) GO TO 60
         PRINT *,' Turbine outlet conditions are superheated'
         XMEAS=1.E12
         RETURN
C
   60 GO TO (70,80),M
   70    F=.03
         IF (PHI.GT.1.5) F=.14
         EFFTRB=.842+.21*(TC/900.-1.)+.078*(1.-FP)
     1          +.12*(1.-TTI/1200.)+F*(1.-PHI/1.5)
         GO TO 90
   80    F=.017
         IF (PHI.GT.1.5) F=.12
         EFFTRB=.857+.155*(TC/1050.-1.)+.11*(1.-FP)
     1          +.137*(1.-TTI/1400.)+F*(1.-PHI/1.5)
C
   90 EFFP=1.-2.*TC/(TB+TC)
      EFFB=(TB-TC)/TB
      EFFS=1.-2.*TC/(TTI+TB)
      HP=(TB-TC)*CPL
      HB=2.57E6-640*TB
      CPS=1.7*TCMAX-.05*TTI-.0005*TCMAX*TTI
      HS=(TTI-TB)*CPS
      EFFCYC=EFFTRB*(EFFP*HP+EFFB*HB+EFFS*HS)/(HP+HB+HS)
      POWTH=POWEL/(EFFCYC*EFFGEN*EFFPC)
      QRAD=POWTH-POWEL/(EFFGEN*EFFPC)
      QTUR=POWTH*EFFCYC
      FLRTT=POWTH*1.E6/(HP+HB+HS)
      PTI=1320.*EXP(-9760./TB)
      PC=1320.*EXP(-9760./TC)
      SEFF=.8
      PEFF=.97
      TPP=TC+(TB-TC)/PEFF
      TRO=TB+(TTI-TB)/SEFF
         IF (TRO.LT.TPP+1.) TRO=TPP+1.
      FLRTR=FLRTT*(HB+HS)/(4184.*(TRO-TPP))
      FLRTRMX=5.*POWTH
      IF (FLRTR.GT.FLRTRMX) THEN
         FLRTR=FLRTRMX
         TRO=TPP+FLRTT*(HB+HS)/(FLRTR*4184.)
         ENDIF
      TRI=TPP-FLRTT*HP/(FLRTR*4184.)
      PRCTR=7200.*EXP(-18100./TRO)
C
C
C MASS OF AN ALKALINE METAL COOLED REACTOR
C
  100 PF=1.3
      DNSREF=3000.
      FR=.5
      TMIN=.12
      TMAX=.12
      NPVOP=1
      US=280.
      DNSPV=8000.
      FMIS=1.
      GSREMX=.2
      GSGAMX=.787
      AA1=2.7
      A1=-.086
      A2=.134
      DNSGSH=19350.
      FS=3.
      FN=.5
      FG=.5
      GG=1.
      GN=1.
      IF(MTLNSH.EQ.1) THEN
      DNSNSH=2520.
      XNGAMX=.1
      SHLREMX=.12
      ELSE
      DNSNSH=820.
      XNGAMX=.0268
      SHLREMX=.124
      ENDIF
C
C CRITICAL AND BURNUP MASS
      ENRGY=POWTH*OPTIM/8760.
      XMBURN=0.38*ENRGY/ENRCH
      XMICRT=CM*XMCRTC/ENRCH**2.*(13600./(VOLFRC*DNSFUL))**1.5
      XMECRT=(XMICRT+SQRT(XMICRT**2+4.*XMICRT*XMBURN*(1.-ENRCH)))/2.
      XMEOL=XMECRT+XMBURN
C
C SPECIFIC POWER LIMIT MASS
      XMSPOW=POWTH*1.E06*PF/PS
C
C BURNUP LIMIT MASS
      XMBRNL=0.38*PF*ENRGY/BRNLIM
C
C FUEL MASS
      XMFUEL=XMEOL
      IF(XMSPOW.GT.XMFUEL) XMFUEL=XMSPOW
      IF(XMBRNL.GT.XMFUEL) XMFUEL=XMBRNL
C
C MODERATOR MASS
      XMMOD=ENRCH*XMFUEL*RMODFUL*XMMWT/235.
C
C STRUCTURE MASS
      XMSTRCT=DNSSTR*RV*(XMFUEL/DNSFUL+XMMOD/DNSMOD)
C
C REFLECTOR MASS
      VOLCOR=(XMFUEL/DNSFUL+XMMOD/DNSMOD)/VOLFRC
      RADCOR=(.75*VOLCOR/3.1416)**.333
      TRF=FR*RADCOR
      IF(TRF.LT.TMIN) TRF=TMIN
      IF(TRF.GT.TMAX) TRF=TMAX
      XMREF=4.*3.1416*RADCOR**2*TRF*DNSREF
C
C PRESSURE VESSEL MASS
      RADPV=RADCOR+NPVOP*TRF
      TPV=4.*PRCTR*RADPV/US
      XMPV=6.*3.1416*RADPV**2.*TPV*DNSPV
C
C MISCELLANEOUS MASS
      XMMIS=FMIS*(XMFUEL+XMMOD)
C
C TOTAL REACTOR MASS
      XMREACT=XMFUEL+XMMOD+XMSTRCT+XMREF+XMPV+XMMIS
C
C
C SHIELD MASS
C
C NEUTRON SHIELD
      TNS=-ALOG(1.5E-17*DOSN*RADCOR*RPAYLD**2.*CORREMX/ENRGY)
     1     /(100.*SHLREMX)
      IF(TNS.LT.0.) TNS=0.
C
C GAMMA SHIELD
      BLDUP=1.
      TGSO=1.
  210 TGS=-1./GSGAMX*(ALOG(DOSGAM*CORGAMX*RADCOR*RPAYLD**2.
     1     *1.0E-08/(ENRGY*BLDUP))/100.+XNGAMX*TNS)
      IF(TGS.LE.0.) THEN
      TGS=0.
      ELSE
      TOPT=(XNGAMX*TNS+GSGAMX*TGS)*100.
      BLDUP=AA1*EXP(-A1*TOPT)+(1.-AA1)*EXP(-A2*TOPT)
      IF(ABS(TGS/TGSO-1.).LT.0.05) GO TO 220
      TGSO=TGS
      GO TO 210
      ENDIF
C
  220 IF(TGS*TNS.LE.0.) THEN
      TGSF=TGS
      TNSF=TNS
C
      ELSE
      TNSI=TNS-TGS*GSREMX/SHLREMX
      IF(TNSI.LT.0.) TNSI=0.
      TGSF=TGS+(TNS-TNSI)*XNGAMX/GSGAMX
      IF(TGSF.LT.0.) TGSF=0.
      TNSF=TNS-TGSF*GSREMX/SHLREMX
      IF(TNSF.LT.0.) TNSF=0.
      ENDIF
C
      XLS=FS*RADCOR
      T1=FG*TGSF
      T2=FN*TNSF
      XL2=T1+T2
      T3=TGSF-T1
      T4=TNSF-T2
      RR=RADCOR+TRF
      TT=TAN(HANG/57.3)
      R1=RR+XLS*TT
      R2=R1+T1*TT
      R3=R2+T2*TT
      R4=R3+T3*TT
      R5=R4+T4*TT
      XMGS=3.14*DNSGSH*(T1*(R1**2+R1*R2+R2**2)
     1                 +T3*(R3**2+R3*R4+R4**2))/3.
      XMNS=3.14*DNSNSH*(T2*(R2**2+R2*R3+R3**2)
     1                 +T4*(R4**2+R4*R5+R5**2))/3.
C
      S0=RR
      S1=S0+T1
      S2=S1+T2
      S3=S2+T3
      S4=S3+T4
      W1=3.14*DNSGSH*(T1*S0**2+.5*(XLS+T1)*(S1**2-S0**2))
      W2=3.14*DNSNSH*(T2*S1**2+.5*(XLS+T1+T2)*(S2**2-S1**2))
      W3=3.14*DNSGSH*(T3*S2**2+.5*(XLS+T1+T2+T3)*(S3**2-S2**2))
      W4=3.14*DNSNSH*(T4*S3**2+.5*(XLS+T1+T2+T3+T4)*(S4**2-S3**2))
      WGS=W1+W3
      WNS=W2+W4
C
      IF(WGS+WNS.LT.XMGS+XMNS) THEN
      XMNSH=WNS*GN
      XMGAMSH=WGS*GG
      ELSE
      XMNSH=XMNS*GN
      XMGAMSH=XMGS*GG
      ENDIF
C
C TOTAL SHIELD MASS
      XMSHLD=XMNSH+XMGAMSH
C
C
C RADIATOR WEIGHT
C
      TR=TC-30
      TSPACE=250.
      SIG=5.7E-08
      ARAD=QRAD*1.E6/(SIG*EMITT*(TR**4-TSPACE**4))
      XMRAD=12.*ARAD
      IF(TR.LT.1000) XMRAD=8.*ARAD
      IF(TR.LT.650) XMRAD=5.0*ARAD
      XMRAD=XMRAD*1.25
C
C
C POWER CONVERSION WEIGHTS
C
C TURBINE WEIGHT
C
      PTURB=QTUR*1000./NT
      TD=TTI
      IF (NDCOOL.EQ.1) TD=TC
      IF (M.EQ.1) THEN
C
C  Nickel superalloy turbine
      OMEGMX=(.0157*TTI-13)*1.E05*PTURB**(-.5)
         IF (OMEG.GT.OMEGMX) OMEG=OMEGMX
         ZS=-2.95
           IF (OMEG.GT.10000.) ZS=-1.31
         ZT=-3.77
           IF (TC.GT.900.) ZT=-2.27
         ZP=0.
           IF (PTURB.GT.110.) ZP=.652
         ZTI=.102
           IF (TTI.GT.1200.) ZTI=.323
      XMTURB=NT*59.8*(OMEG/10000.)**ZS*(TC/900.)**ZT
     1       *(PTURB/110.)**ZP*(PHI/1.5)**(-1.34)*FP**(-.088)
     2       *(TD/1200.)**(-1.95)*EXP(.002*ZTI*(TTI-1200.))
C
      ELSE
C  Tantalum alloy turbine
      OMEGMX=(.0123*TTI-11.7)*1.E05*PTURB**(-.49)
         IF (OMEG.GT.OMEGMX) OMEG=OMEGMX
         ZS=-2.95
           IF (OMEG.GT.11500.) ZS=-1.22
         ZOM=10000.
           IF (OMEG.GT.11500.) ZOM=8202.
         ZP=0.
           IF (PTURB.GT.125.) ZP=.642
         ZTI=.88
           IF (TTI.GT.1400.) ZTI=1.9
      XMTURB=NT*80.2*(OMEG/ZOM)**ZS*(PTURB/125.)**ZP
     1       *(TTI/1400.)**ZTI*(TC/1050.)**(-4.0)*(PHI/1.5)**(-1.22)
     2       *(TD/TTI)**(-.53)
      ENDIF
C
C HEAT EXCHANGER OR SEPARATOR AND CONDENSER WEIGHT
      IF(QRAD.LT.0.5) THEN
      ZM=-.5828
      B=4.789
      ELSE IF(QRAD.LT.0.7) THEN
      ZM=-.5419
      B=4.8174
      ELSE IF(QRAD.LT.1.0) THEN
      ZM=-.4012
      B=4.8675
      ELSE IF(QRAD.LT.3.0) THEN
      ZM=-.3802
      B=4.9175
      ELSE IF(QRAD.LT.10.0) THEN
      ZM=-.2087
      B=4.7291
      ELSEIF(QRAD.LT.30.0) THEN
      ZM=-.3063
      B=4.9537
      ENDIF
      XMCND=EXP(ZM*ALOG(QRAD)+B)*QRAD
      IF(QRAD.GT.30.) XMCND=50.*QRAD
C
      IF(CYCTYP.EQ.1) THEN
      XMHX=0.
      XMSEP=XMCND
      ELSE
      XMHX=.53*((HP+HB)*FLRTT/1000.)**.74+14.*(HS*FLRTT/1000.)**.7
      XMSEP=0.
      ENDIF
C
C GENERATOR WEIGHT
      XMGEN=100.*POWEL/EFFPC
C
C POWER CONDITIONING WEIGHT
      XMPC=200.*POWEL
C
C GENERATOR AND POWER CONDITIONING RADIATOR WEIGHT
      TRADGPC=500.
      QGNPC=POWEL*1.E06*(1./(EFFGEN*EFFPC)-1.)
      ARADGPC=QGNPC/(SIG*EMITT*(TRADGPC**4-TSPACE**4))
      XMRADPC=5.0*ARADGPC*1.25
C
C TOTAL POWER CONVERSION WEIGHT
      XMPCNV=XMTURB+XMHX+XMSEP+XMGEN+XMPC+XMRADPC+XMCND
C
C
C MISCELLANEOUS WEIGHT
C
      XMMISC=.1*(XMREACT+XMSHLD+XMPCNV+XMRAD)
C
C
C TOTAL WEIGHT
C
      XMTOT=XMREACT+XMSHLD+XMPCNV+XMRAD+XMMISC
C
C
C SYSTEM COST
C
C REACTOR
      CSTRNS=(XMREACT+XMSHLD)*.0044
C
C POWER CONVERSION
      CSTPCNV=(XMTURB+XMHX+XMSEP+XMCND)*.002+XMGEN*.003
     1   +XMPC*.01+XMRADPC*.0002
C
C RADIATOR
      CSTRAD=XMRAD*.0002
C
C LAUNCH COST
      CSTLNCH=.00128*XMTOT
C
C TOTAL COST
      CSTTOT=CSTRNS+CSTPCNV+CSTRAD+CSTLNCH
C
C
      XMEAS=XMTOT
      IF(BOTLIN.EQ.'COST  ') XMEAS=CSTTOT
C
      PRINT 15,TTI,TC,EFFCYC,XMREACT+XMSHLD,XMPCNV,XMRAD,XMTOT,CSTTOT
   15 FORMAT(1X,3F10.3,5F10.0)
C
      IF(IPR.EQ.0) GO TO 500
C
      PRINT 1
      PRINT 1
    1 FORMAT(/)
      PRINT *,' CYCLE PARAMETERS'
      PRINT *,'    CYCLE EFFICIENCY                   = ',EFFCYC
      PRINT *,'    THERMAL POWER-MW                   = ',POWTH
      PRINT *,'    MASS FLOW RATE-Kg/s                = ',FLRTT
      PRINT *,'    TURBINE INLET TEMP-K               = ',TTI
      PRINT *,'    CONDENSER TEMP-K                   = ',TC
      PRINT *,'    BOILER TEMP-K                      = ',TB
      PRINT *,'    BOILER PRESSURE-MPa                = ',PTI
      PRINT *,'    CONDENSER PRESSURE-MPa             = ',PC
      PRINT *,'    REACTOR OUTLET TEMP-K              = ',TRO
      PRINT *,'    REACTOR INLET TEMP-K               = ',TRI
      PRINT *,'    REACTOR PRESSURE-MPa               = ',PRCTR
      PRINT *,'    REACTOR FLOW RATE-kg/s             = ',FLRTR
C
      PRINT 1
      PRINT *,' REACTOR PARAMETERS'
      PRINT *,'    BURNUP MASS-Kg                     = ',XMBURN
      PRINT *,'    INITIAL CRITICAL MASS-Kg           = ',XMICRT
      PRINT *,'    END CRITICAL MASS-Kg               = ',XMECRT
      PRINT *,'    TOTAL BRNUP+CRIT MASS-Kg           = ',XMEOL
      PRINT *,'    SPECIFIC POWER-W/Kg                = ',PS
      PRINT *,'    MASS FOR SPECIFIC POWER LIM-Kg     = ',XMSPOW
      PRINT *,'    MASS FOR ALLOWED BURNUP-Kg         = ',XMBRNL
      PRINT *,'    FUEL MASS-Kg                       = ',XMFUEL
      PRINT *,'    MODERATOR MASS-Kg                  = ',XMMOD
      PRINT *,'    STRUCTURE MASS-Kg                  = ',XMSTRCT
      PRINT *,'    REFLECTOR MASS-Kg                  = ',XMREF
      PRINT *,'    PRESSURE VESSEL MASS-Kg            = ',XMPV
      PRINT *,'    MISCELLANEOUS MASS-Kg              = ',XMMIS
      PRINT *,'    TOTAL REACTOR MASS-Kg              = ',XMREACT
      PRINT *,'    NEUTRON SHIELD THICKNESS-m         = ',TNSF
      PRINT *,'    NEUTRON SHIELD MASS-Kg             = ',XMNSH
      PRINT *,'    GAMMA SHIELD THICKNESS-m           = ',TGSF
      PRINT *,'    GAMMA SHIELD MASS-Kg               = ',XMGAMSH
      PRINT *,'    TOTAL SHIELD MASS-Kg               = ',XMSHLD
C
      PRINT 1
      PRINT *,' RADIATOR PARAMETERS'
      PRINT *,'    TOTAL AREA-m2                      = ',ARAD
      PRINT *,'    TOTAL WEIGHT-Kg                    = ',XMRAD
C
      PRINT 1
      PRINT *,' POWER CONVERSION PARAMETERS'
      PRINT *,'    TURBINE WEIGHT-Kg                  = ',XMTURB
      PRINT *,'    TURBINE SPEED-RPM                  = ',OMEG
      PRINT *,'    TURBINE EFFICIENCY                 = ',EFFTRB
      PRINT *,'    HEAT EXCHANGER WEIGHT-kg           = ',XMHX
      PRINT *,'    VAPOR SEPARATOR WEIGHT-kg          = ',XMSEP
      PRINT *,'    CONDENSER WEIGHT-kg                = ',XMCND
      PRINT *,'    GENERATOR WEIGHT-Kg                = ',XMGEN
      PRINT *,'    POWER CONDITIONING WEIGHT-Kg       = ',XMPC
      PRINT *,'    GEN & PC RADIATOR WEIGHT-Kg        = ',XMRADPC
      PRINT *,'    GEN & PC RADIATOR AREA-m2          = ',ARADGPC
      PRINT *,'    TOTAL WEIGHT-Kg                    = ',XMPCNV
C
      PRINT 1
      PRINT 1
      PRINT *,' WEIGHT AND COST SUMMARY'
      PRINT *,'    REACTOR WEIGHT-Kg                  = ',XMREACT
      PRINT *,'    SHIELD WEIGHT-Kg                   = ',XMSHLD
      PRINT *,'    POWER CONVERSION WEIGHT-Kg         = ',XMPCNV
      PRINT *,'    RADIATOR WEIGHT-Kg                 = ',XMRAD
      PRINT *,'    MISCELLANEOUS WEIGHT-Kg            = ',XMMISC
      PRINT *,'    TTOTAL WEIGHT-Kg                   = ',XMTOT
      PRINT 1
      PRINT *,'    REACTOR+SHIELD COST-M$             = ',CSTRNS
      PRINT *,'    POWER CONVERSION COST-M$           = ',CSTPCNV
      PRINT *,'    RADIATOR COST-M$                   = ',CSTRAD
      PRINT *,'    LAUNCH COST-M$                     = ',CSTLNCH
      PRINT *,'    TOTAL COST-M$                      = ',CSTTOT
C
C
  500 RETURN
      END
