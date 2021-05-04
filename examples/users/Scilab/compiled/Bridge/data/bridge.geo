x11=1.0; x12=1.0; x13=1.0; x14=1.0; y11=2.0; y12=2.0; y13=1.0; y14=1.0; z11=0.0; z12=1.0; z13=1.0; z14=0.0;
x21=2.0; x22=2.0; x23=2.0; x24=2.0; y21=2.0; y22=2.0; y23=1.0; y24=1.0; z21=0.0; z22=1.0; z23=1.0; z24=0.0;
x31=3.0; x32=3.0; x33=3.0; x34=3.0; y31=2.0; y32=2.0; y33=1.0; y34=1.0; z31=0.0; z32=1.0; z33=1.0; z34=0.0;
x41=4.0; x42=4.0; x43=4.0; x44=4.0; y41=2.0; y42=2.0; y43=1.0; y44=1.0; z41=0.0; z42=1.0; z43=1.0; z44=0.0;
x51=5.0; x52=5.0; x53=5.0; x54=5.0; y51=2.0; y52=2.0; y53=1.0; y54=1.0; z51=0.0; z52=1.0; z53=1.0; z54=0.0;
x61=6.0; x62=6.0; x63=6.0; x64=6.0; y61=2.0; y62=2.0; y63=1.0; y64=1.0; z61=0.0; z62=1.0; z63=1.0; z64=0.0;
x71=7.0; x72=7.0; x73=7.0; x74=7.0; y71=2.0; y72=2.0; y73=1.0; y74=1.0; z71=0.0; z72=1.0; z73=1.0; z74=0.0;

xb1=0.0; xb2=0.0; xb3=8.0; xb4=8.0; yb1=1.0; yb2=1.0; yb3=1.0; yb4=1.0; zb1=0.0; zb2=1.0; zb3=0.0; zb4=1.0;

xp1=1.0; xp2=1.0; xp3=7.0; xp4=7.0; yp1=0.0; yp2=0.0; yp3=0.0; yp4=0.0; zp1=0.0; zp2=1.0; zp3=0.0; zp4=1.0;

I11=1;  I12=2;  I13=3;  I14=4;
I21=5;  I22=6;  I23=7;  I24=8;
I31=9;  I32=10; I33=11; I34=12;
I41=13; I42=14; I43=15; I44=16;
I51=17; I52=18; I53=19; I54=20;
I61=21; I62=22; I63=23; I64=24;
I71=25; I72=26; I73=27; I74=28;

IB1=29; IB2=30; IB3=31; IB4=32;
IP1=33; IP2=34; IP3=35; IP4=36;

lc = 0.1;

Point(I11) = {x11,y11,z11,lc};
Point(I12) = {x12,y12,z12,lc};
Point(I13) = {x13,y13,z13,lc};
Point(I14) = {x14,y14,z14,lc};

Point(I21) = {x21,y21,z21,lc};
Point(I22) = {x22,y22,z22,lc};
Point(I23) = {x23,y23,z23,lc};
Point(I24) = {x24,y24,z24,lc};

Point(I31) = {x31,y31,z31,lc};
Point(I32) = {x32,y32,z32,lc};
Point(I33) = {x33,y33,z33,lc};
Point(I34) = {x34,y34,z34,lc};

Point(I41) = {x41,y41,z41,lc};
Point(I42) = {x42,y42,z42,lc};
Point(I43) = {x43,y43,z43,lc};
Point(I44) = {x44,y44,z44,lc};

Point(I51) = {x51,y51,z51,lc};
Point(I52) = {x52,y52,z52,lc};
Point(I53) = {x53,y53,z53,lc};
Point(I54) = {x54,y54,z54,lc};

Point(I61) = {x61,y61,z61,lc};
Point(I62) = {x62,y62,z62,lc};
Point(I63) = {x63,y63,z63,lc};
Point(I64) = {x64,y64,z64,lc};

Point(I71) = {x71,y71,z71,lc};
Point(I72) = {x72,y72,z72,lc};
Point(I73) = {x73,y73,z73,lc};
Point(I74) = {x74,y74,z74,lc};

Point(IB1) = {xb1,yb1,zb1,lc};
Point(IB2) = {xb2,yb2,zb2,lc};
Point(IB3) = {xb3,yb3,zb3,lc};
Point(IB4) = {xb4,yb4,zb4,lc};

Point(IP1) = {xp1,yp1,zp1,lc};
Point(IP2) = {xp2,yp2,zp2,lc};
Point(IP3) = {xp3,yp3,zp3,lc};
Point(IP4) = {xp4,yp4,zp4,lc};

// Sections
Line(201) = {I14,I13};
Line(202) = {I13,I12,I11,I14};

Line(203) = {I24,I23};
Line(204) = {I23,I22,I21,I24};

Line(205) = {I34,I33}; 
Line(206) = {I33,I32,I31,I34};

Line(207) = {I44,I43}; 
Line(208) = {I43,I42,I41,I44};

Line(209) = {I54,I53}; 
Line(210) = {I53,I52,I51,I54};

Line(211) = {I64,I63}; 
Line(212) = {I63,I62,I61,I64};

Line(213) = {I74,I73}; 
Line(214) = {I73,I72,I71,I74};


// Crosses between 2 sections
// Top
Line(215) = {I12,I21};
Line(216) = {I11,I22};
// Bottom
Line(217) = {I13,I24};
Line(218) = {I14,I23};
// Side 1
Line(219) = {I11,I24};
Line(220) = {I14,I21};
// Side 2
Line(221) = {I12,I23};
Line(222) = {I13,I22};

// Top
Line(223) = {I22,I31};
Line(224) = {I21,I32};
// Bottomm
Line(225) = {I23,I34};
Line(226) = {I24,I33};
// Side 1
Line(227) = {I21,I34};
Line(228) = {I24,I31};
// Side 2
Line(229) = {I22,I33};
Line(230) = {I23,I32};

// Top
Line(231) = {I32,I41};
Line(232) = {I31,I42};
// Bottom
Line(233) = {I33,I44};
Line(234) = {I34,I43};
// Side 1
Line(235) = {I31,I44};
Line(236) = {I34,I41};
// Side 2
Line(237) = {I32,I43};
Line(238) = {I33,I42};

// Top
Line(239) = {I42,I51};
Line(240) = {I41,I52};
// Bottom
Line(241) = {I43,I54};
Line(242) = {I44,I53};
// Side 1
Line(243) = {I41,I54};
Line(244) = {I44,I51};
// Side 2
Line(245) = {I42,I53};
Line(246) = {I43,I52};

// Top
Line(247) = {I52,I61};
Line(248) = {I51,I62};
// Bottom
Line(249) = {I53,I64};
Line(250) = {I54,I63};
// Side 1
Line(251) = {I51,I64};
Line(252) = {I54,I61};
// Side 2
Line(253) = {I52,I63};
Line(254) = {I53,I62};

// Top
Line(255) = {I62,I71};
Line(256) = {I61,I72};
// Bottom
Line(257) = {I63,I74};
Line(258) = {I64,I73};
// Side 1
Line(259) = {I61,I74};
Line(260) = {I64,I71};
// Side 2
Line(261) = {I62,I73};
Line(262) = {I63,I72};


// Bars connecting 2 sections
Line(263) = {I11,I21};
Line(264) = {I12,I22};
Line(265) = {I13,I23};
Line(266) = {I14,I24};

Line(267) = {I21,I31};
Line(268) = {I22,I32};
Line(269) = {I23,I33};
Line(270) = {I24,I34};

Line(271) = {I31,I41};
Line(272) = {I32,I42};
Line(273) = {I33,I43};
Line(274) = {I34,I44};

Line(275) = {I41,I51};
Line(276) = {I42,I52};
Line(277) = {I43,I53};
Line(278) = {I44,I54};

Line(279) = {I51,I61};
Line(280) = {I52,I62};
Line(281) = {I53,I63};
Line(282) = {I54,I64};

Line(283) = {I61,I71};
Line(284) = {I62,I72};
Line(285) = {I63,I73};
Line(286) = {I64,I74};

// First extremity of the bridge
Line(287) = {IB1,I14};
Line(288) = {IB2,I13};
Line(289) = {IB1,I11};
Line(290) = {IB2,I12};
Line(291) = {IB1,IB2};

// Second extremity of the bridge
Line(292) = {IB3,I74};
Line(293) = {IB4,I73};
Line(294) = {IB3,I71};
Line(295) = {IB4,I72};
Line(296) = {IB3,IB4};

// First pylon
Line(297) = {I14,IP2};
Line(298) = {I13,IP1};
Line(299) = {I14,IP1};
Line(300) = {I13,IP2};
Line(301) = {I24,IP1};
Line(302) = {I23,IP2};
Line(303) = {IP1,IP2};
Line(304) = {IP1,I23};
Line(305) = {IP2,I24};

// Second pylon
Line(306) = {I74,IP4};
Line(307) = {I73,IP3};
Line(308) = {I74,IP3};
Line(309) = {I73,IP4};
Line(310) = {I64,IP3};
Line(311) = {I63,IP4};
Line(312) = {IP3,IP4};
Line(313) = {IP3,I63};
Line(314) = {IP4,I64};

// Definition of physical points and lines
Physical Point(401) = {IP1};
Physical Point(402) = {IP2};
Physical Point(403) = {IP3};
Physical Point(404) = {IP4};

Physical Point(405) = {IB1};
Physical Point(406) = {IB2};
Physical Point(407) = {IB3};
Physical Point(408) = {IB4};

Physical Point(409) = {I13};
Physical Point(410) = {I14};
Physical Point(411) = {I23};
Physical Point(412) = {I24};
Physical Point(413) = {I33};
Physical Point(414) = {I34};
Physical Point(415) = {I43};
Physical Point(416) = {I44};
Physical Point(417) = {I53};
Physical Point(418) = {I54};
Physical Point(419) = {I63};
Physical Point(420) = {I64};
Physical Point(421) = {I73};
Physical Point(422) = {I74};

Physical Line(501) = {201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,
	              220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,
		      239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,256,257,
		      258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,
		      277,278,279,280,281,282,283,284,285,286,287,288,289,290,291,292,293,294,295,
		      296,297,298,299,300,301,302,303,304,305,306,307,308,309,310,311,312,313,314};
