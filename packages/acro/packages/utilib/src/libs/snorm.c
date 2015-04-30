/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */

/* snorm.c
 *
 * Routines which generate the normal distribution N(0,1).
 * These can be used by gennor to generate an arbitrary normal distribution.
 */

#include <utilib_config.h>
#include <stdio.h>
#include <math.h>
#include <utilib/Random.h>


double snorm1()
/*
**********************************************************************


     (STANDARD-)  N O R M A L  DISTRIBUTION


**********************************************************************
**********************************************************************

     FOR DETAILS SEE:

               AHRENS, J.H. AND DIETER, U.
               EXTENSIONS OF FORSYTHE'S METHOD FOR RANDOM
               SAMPLING FROM THE NORMAL DISTRIBUTION.
               MATH. COMPUT., 27,124 (OCT. 1973), 927 - 937.

     ALL STATEMENT NUMBERS CORRESPOND TO THE STEPS OF ALGORITHM 'FL'
     (M=5) IN THE ABOVE PAPER     (SLIGHTLY MODIFIED IMPLEMENTATION)

     Modified by Barry W. Brown, Feb 3, 1988 to use RANF instead of
     SUNIF.  The argument IR thus goes away.

**********************************************************************
     THE DEFINITIONS OF THE CONSTANTS A(K), D(K), T(K) AND
     H(K) ARE ACCORDING TO THE ABOVEMENTIONED ARTICLE
*/
{
static double a[32] = {
    0.0,3.917609E-2,7.841241E-2,0.11777,0.1573107,0.1970991,0.2372021,0.2776904,    0.3186394,0.36013,0.4022501,0.4450965,0.4887764,0.5334097,0.5791322,
    0.626099,0.6744898,0.7245144,0.7764218,0.8305109,0.8871466,0.9467818,
    1.00999,1.077516,1.150349,1.229859,1.318011,1.417797,1.534121,1.67594,
    1.862732,2.153875
};
static double d[31] = {
    0.0,0.0,0.0,0.0,0.0,0.2636843,0.2425085,0.2255674,0.2116342,0.1999243,
    0.1899108,0.1812252,0.1736014,0.1668419,0.1607967,0.1553497,0.1504094,
    0.1459026,0.14177,0.1379632,0.1344418,0.1311722,0.128126,0.1252791,
    0.1226109,0.1201036,0.1177417,0.1155119,0.1134023,0.1114027,0.1095039
};
static double t[31] = {
    7.673828E-4,2.30687E-3,3.860618E-3,5.438454E-3,7.0507E-3,8.708396E-3,
    1.042357E-2,1.220953E-2,1.408125E-2,1.605579E-2,1.81529E-2,2.039573E-2,
    2.281177E-2,2.543407E-2,2.830296E-2,3.146822E-2,3.499233E-2,3.895483E-2,
    4.345878E-2,4.864035E-2,5.468334E-2,6.184222E-2,7.047983E-2,8.113195E-2,
    9.462444E-2,0.1123001,0.136498,0.1716886,0.2276241,0.330498,0.5847031
};
static double h[31] = {
    3.920617E-2,3.932705E-2,3.951E-2,3.975703E-2,4.007093E-2,4.045533E-2,
    4.091481E-2,4.145507E-2,4.208311E-2,4.280748E-2,4.363863E-2,4.458932E-2,
    4.567523E-2,4.691571E-2,4.833487E-2,4.996298E-2,5.183859E-2,5.401138E-2,
    5.654656E-2,5.95313E-2,6.308489E-2,6.737503E-2,7.264544E-2,7.926471E-2,
    8.781922E-2,9.930398E-2,0.11556,0.1404344,0.1836142,0.2790016,0.7010474
};
static long i;
static double snorm,u,s,ustar,aa,w,y,tt;
    u = ranf();
    s = 0.0;
    if(u > 0.5) s = 1.0;
    u += (u-s);
    u = 32.0*u;
    i = (long) (u);
    if(i == 32) i = 31;
    if(i == 0) goto S100;
/*
                                START CENTER
*/
    ustar = u-(double)i;
    aa = *(a+i-1);
S40:
    if(ustar <= *(t+i-1)) goto S60;
    w = (ustar-*(t+i-1))**(h+i-1);
S50:
/*
                                EXIT   (BOTH CASES)
*/
    y = aa+w;
    snorm = y;
    if(s == 1.0) snorm = -y;
    return snorm;
S60:
/*
                                CENTER CONTINUED
*/
    u = ranf();
    w = u*(*(a+i)-aa);
    tt = (0.5*w+aa)*w;
    goto S80;
S70:
    tt = u;
    ustar = ranf();
S80:
    if(ustar > tt) goto S50;
    u = ranf();
    if(ustar >= u) goto S70;
    ustar = ranf();
    goto S40;
S100:
/*
                                START TAIL
*/
    i = 6;
    aa = *(a+31);
    goto S120;
S110:
    aa += d[i-1];  	/* *(d+i-1) */
    i += 1;
S120:
    u += u;
    if((u < 1.0) && (i <= 30)) goto S110;	/* Is this the right fix? */
    if ((u == 0.0) && (i == 31)) {
       fprintf(stdout,"Bad Uniform Var?\n");
       fflush(stdout);
       }
    u -= 1.0;
S140:
    w = u**(d+i-1);
    tt = (0.5*w+aa)*w;
    goto S160;
S150:
    tt = u;
S160:
    ustar = ranf();
    if(ustar > tt) goto S50;
    u = ranf();
    if(ustar >= u) goto S150;
    u = ranf();
    goto S140;
}





/*
 * Adapted from Numerical Recipies in C and from code by Alan Miller
 *	alan@mel.dms.csiro.au
 *
 * This is the Box-Muller method for generating independent normal deviates.
 *  	G.E.P.Box and M.E.Mueller
 *  	A Note on the Generation  of Random Normal Deviates
 *  	Ann.Math.Stat. (1958) 29, 610-611
 *
 * This is also called the polar method
 *      Marsaglia,G. & Bray,T.A. 'A convenient method for
 *      generating normal variables', Siam Rev., vol.6,
 *      260-264, 1964.
 *

Path: sdcc12!network.ucsd.edu!swrinde!zaphod.mps.ohio-state.edu!wupost!uunet!mcsun!uknet!warwick!cam-cl!cam-cl!nmm
From: nmm@cl.cam.ac.uk (Nick Maclaren)
Newsgroups: sci.math.stat
Subject: Re: random number generator
Keywords: pseudo-random, C , uniform, normal
Message-ID: <1992Jul27.124120.29511@cl.cam.ac.uk>
Date: 27 Jul 92 12:41:20 GMT
References: <1992Jul27.103659.11820@kth.se>
Sender: news@cl.cam.ac.uk (The news facility)
Reply-To: nmm@cl.cam.ac.uk (Nick Maclaren)
Organization: U of Cambridge Computer Lab, UK
Lines: 25

In article <1992Jul27.103659.11820@kth.se>, md87-mpe@hemul.nada.kth.se
(Magnus Pettersson) writes:

|> .... There is also a description on how to make two independent and
|> U(0,1)-distributed random numbers into two independent N(0,1)-distributed
|> randomnumbers, with a method called "the Box-Mueller Method". That method is
|> first described in
|>
|> 3. G.E.P.Box and M.E.Mueller
|>    A Note on the Generation  of Random Normal Deviates
|>    Ann.Math.Stat. (1958) 29, 610-611

Don't use this method in single precision - it has a very serious
flaw.  While it is OK in double precision, there are faster and simpler
methods available (e.g. the Polar Method as described in Knuth).  The
reference for the flaw is by H.R. Neave round about 1970.
 
 
Nick Maclaren
University of Cambridge Computer Laboratory,
New Museums Site, Pembroke Street,
Cambridge CB2 3QG, England.
Email:  nmm@cl.cam.ac.uk
Tel.:   +44 223 334761
Fax:    +44 223 334679

Path: sdcc12!network.ucsd.edu!usc!cs.utexas.edu!uunet!mcsun!uknet!warwick!strgh
From: strgh@warwick.ac.uk (J E H Shaw)
Newsgroups: sci.math.stat
Subject: Re: random number generator
Keywords: pseudo-random, C , uniform, normal
Message-ID: <ssanb14z@csv.warwick.ac.uk>
Date: 27 Jul 92 13:32:17 GMT
References: <1992Jul27.103659.11820@kth.se> <1992Jul27.124120.29511@cl.cam.ac.uk>
Sender: news@csv.warwick.ac.uk (Network news)
Organization: Computing Services, Warwick University, UK
Lines: 24
Nntp-Posting-Host: clover

In article <1992Jul27.124120.29511@cl.cam.ac.uk> nmm@cl.cam.ac.uk (Nick Maclaren) writes:
>|> ...
>|> 3. G.E.P.Box and M.E.Mueller
>|>    A Note on the Generation  of Random Normal Deviates
>|>    Ann.Math.Stat. (1958) 29, 610-611
>
>Don't use this method in single precision - it has a very serious
>flaw.  While it is OK in double precision, there are faster and simpler
>methods available (e.g. the Polar Method as described in Knuth).  The
>reference for the flaw is by H.R. Neave round about 1970.
>
See H.R. Neave (1973), "On using the Box-Muller transformation with
multiplicative congruential pseudo-random number generators",
Applied Statistics 22:92-97.
 
In general, if you're interested in the tail-behaviour of some distribution,
be particularly careful about the method you choose for simulation.
 
                -- Ewart Shaw

--
J.E.H.Shaw,  Department of Statistics,  |  JANET:  strgh@uk.ac.warwick.cu
             University of Warwick,     |  BITNET: strgh%uk.ac.warwick.cu@UKACRL             Coventry  CV4 7AL,  U.K.   |  PHONE:  +44 203 523069
yacc - the piece of code that understandeth all parsing

Path: sdcc12!network.ucsd.edu!usc!cs.utexas.edu!sun-barr!olivea!bu.edu!jade.tufts.edu!news.tufts.edu!sage.hnrc.tufts.edu!jerry
From: jerry@ginger.hnrc.tufts.edu (Jerry Dallal)
Newsgroups: sci.math.stat
Subject: Re: random number generator
Message-ID: <1992Jul27.132508.217@ginger.hnrc.tufts.edu>
Date: 27 Jul 92 18:25:07 GMT
References: <1992Jul27.103659.11820@kth.se> <1992Jul27.124120.29511@cl.cam.ac.uk>
Organization: USDA HNRC at Tufts University
Lines: 48

In article <1992Jul27.124120.29511@cl.cam.ac.uk>, nmm@cl.cam.ac.uk (Nick Maclaren) writes:
> In article <1992Jul27.103659.11820@kth.se>, md87-mpe@hemul.nada.kth.se
> (Magnus Pettersson) writes:
>
> |> 3. G.E.P.Box and M.E.Mueller
> |>    A Note on the Generation  of Random Normal Deviates
> |>    Ann.Math.Stat. (1958) 29, 610-611
>
> Don't use this method in single precision - it has a very serious
> flaw.  While it is OK in double precision, there are faster and simpler
> methods available (e.g. the Polar Method as described in Knuth).  The
> reference for the flaw is by H.R. Neave round about 1970.
>
>
> Nick Maclaren
> University of Cambridge Computer Laboratory,
> New Museums Site, Pembroke Street,
> Cambridge CB2 3QG, England.


Wait! . . .  Stop! . . . Hold on a moment! . . .

There is nothing wrong with the Box-Muller method.  It is theoretically sound
and changing from single precision to double precision isn't going to affect
anything.  (For that matter, it isn't going to effect anything, either!)

The Box-Muller transformation is based on the simple fact of distribution
functions that if U and V are iid U(0,1), then

                       sqrt(-2 log U) sin (2 pi V)
                       sqrt(-2 log U) cos (2 pi V)

are iid N(0,1).

I don't have the article in front of me but I recall that (as explained in a
followup letter) Neave's argument was flawed.  The problem was with a
uniform random number generator that used too small a modulus and didn't "kick
over" fast enough.

I agree with Mclaren that the Neave article was published in the 70's.  I
can add that it appeared in Applied Statistics.  The followup letter appear
within the next year or two.  I'll leave it to someone else to look up the
exact references.

(As a practical matter, I prefer to use the inverse of the standard normal
distribution to generate my random numbers.  I know it is computationally
intensive, but it relieves you from having to worry (as much)
about the joint distribution of values in your pseudo-random number stream.)

Path: sdcc12!network.ucsd.edu!sdd.hp.com!cs.utexas.edu!asuvax!ncar!noao!amethyst!organpipe.uug.arizona.edu!news
From: paul@music.sie.arizona.edu (Paul J. Sanchez)
Newsgroups: sci.math.stat
Subject: Re: random number generator
Message-ID: <1992Jul27.201230.984@organpipe.uug.arizona.edu>
Date: 27 Jul 92 20:12:30 GMT
References: <1992Jul27.132508.217@ginger.hnrc.tufts.edu>
Sender: news@organpipe.uug.arizona.edu
Organization: University of Arizona UNIX Users Group
Lines: 41

Jerry Dallal writes
> In article <1992Jul27.124120.29511@cl.cam.ac.uk>, nmm@cl.cam.ac.uk (Nick
Maclaren) writes:
> > In article <1992Jul27.103659.11820@kth.se>, md87-mpe@hemul.nada.kth.se
> > (Magnus Pettersson) writes:
> >
> > |> 3. G.E.P.Box and M.E.Mueller
> > |>    A Note on the Generation  of Random Normal Deviates
> > |>    Ann.Math.Stat. (1958) 29, 610-611
> >
> > Don't use this method in single precision [...]
> >
>
> Wait! . . .  Stop! . . . Hold on a moment! . . .
>
> There is nothing wrong with the Box-Muller method.  It is
> theoretically sound [...]

Theory works beautifully when all of the assumptions hold.  One of the
assumptions for the Box-Muller method is that you have i.i.d. uniform
variables on the input side.  This happens to be false in most computing
contexts -- most people use a sequence of pseudo-random numbers which are
uncorrelated but hardly independent.

The most common source of pseudo-random numbers is the Linear Congruential
Generator algorithm, or LCG for short.  A result by Marsaglia states that
k-tuples from a LCG all fall on a bounded number of hyperplanes.  As a
result, there is an interaction with the Box-Muller algorithm which causes
all of the values produced to fall along a spiral.  For some very pretty
pictures, check out the book "Stochastic Simulation" by Brian D. Ripley
('87, J. Wiley & Sons).  Pages 55-58 should do a good job of convincing
you that the results are definitely not independent normals.

There are a couple of morals to this story:
a) If you want to use Box-Muller, use something other than a LCG; and
b) Just because an algorithm is theoretically sound, don't assume you'll
get usable results.

--paul
paul@music.sie.arizona.edu
(NeXT mail welcome!)


Path: sdcc12!network.ucsd.edu!usc!howland.reston.ans.net!bogus.sura.net!jhunix.hcf.jhu.edu!jhunix.hcf.jhu.edu!not-for-mail
From: merkel@jhunix.hcf.jhu.edu (Frederick C Merkel)
Newsgroups: sci.math.num-analysis
Subject: Generating random Gaussian deviates
Keywords: random gaussian deviates
Message-ID: <1ktvb0INNb5p@jhunix.hcf.jhu.edu>
Date: 5 Feb 93 14:58:40 GMT
Sender: Merkel (merkel@jhunix.hcf.jhu.edu)
Distribution: inet
Organization: Department of Psychology, Johns Hopkins University
Lines: 17
NNTP-Posting-Host: jhunix.hcf.jhu.edu

We need to generate literally billions of gaussian deviates for some Monte
Carlo simulations.  We had been using the  Box-Muller method to generate
the deviates, but found some speed-up by using the ziggurat method described
in Marsaglia & Tang (1984, SIAM J Sci Stat Comput, 5, 2).  (The speed-up
over B-M seems to be highly system dependent).

I have two general questions:
1) Has anyone implemented the 256-layer algorithm?  Marsaglia describe the
   algorithm/heuristic? for generating the necessary coefficients, but I am
   a bit hesitant to calculate it myself.
2) Is there a more efficient algorithm for generating Gaussian deviates?

Thanks in advance,

Fred Merkel
Department of Psychology
Johns Hopkins U

Path: sdcc12!network.ucsd.edu!usc!howland.reston.ans.net!bogus.sura.net!darwin.sura.net!sgiblab!nec-gw!netkeeper!vivaldi!seas.smu.edu!convex!convex!dodson
From: dodson@convex.COM (Dave Dodson)
Newsgroups: sci.math.num-analysis
Subject: Re: Generating random Gaussian deviates
Keywords: random gaussian deviates
Message-ID: <1993Feb5.160142.1261@news.eng.convex.com>
Date: 5 Feb 93 16:01:42 GMT
References: <1ktvb0INNb5p@jhunix.hcf.jhu.edu>
Sender: usenet@news.eng.convex.com (news access account)
Reply-To: dodson@convex.COM (Dave Dodson)
Distribution: inet
Organization: Engineering, CONVEX Computer Corp., Richardson, Tx., USA
Lines: 21
Originator: dodson@wagner.convex.com
Nntp-Posting-Host: wagner.convex.com
X-Disclaimer: This message was written by a user at CONVEX Computer
              Corp. The opinions expressed are those of the user and
              not necessarily those of CONVEX.

In article <1ktvb0INNb5p@jhunix.hcf.jhu.edu> merkel@jhunix.hcf.jhu.edu (Frederick C Merkel) writes:
>We need to generate literally billions of gaussian deviates for some Monte
>Carlo simulations.  We had been using the  Box-Muller method to generate
>the deviates, but found some speed-up by using the ziggurat method described
>in Marsaglia & Tang (1984, SIAM J Sci Stat Comput, 5, 2).  (The speed-up
>over B-M seems to be highly system dependent).
>
>I have two general questions:
>1) Has anyone implemented the 256-layer algorithm?  Marsaglia describe the
>   algorithm/heuristic? for generating the necessary coefficients, but I am
>   a bit hesitant to calculate it myself.
>2) Is there a more efficient algorithm for generating Gaussian deviates?

Check out the paper "A fast normal random number generator" by J.L. Leva
in ACM Transactions on Mathematical Software, Vol 18, No. 4, December 1992.
Code is presented in an accompanying algorithm paper.

----------------------------------------------------------------------

Dave Dodson                                          dodson@convex.COM
Convex Computer Corporation      Richardson, Texas      (214) 497-4234

Path: sdcc12!network.ucsd.edu!sdd.hp.com!swrinde!elroy.jpl.nasa.gov!ames!purdue!mentor.cc.purdue.edu!pop.stat.purdue.edu!hrubin
From: hrubin@pop.stat.purdue.edu (Herman Rubin)
Newsgroups: sci.math.num-analysis
Subject: Re: Generating random Gaussian deviates
Keywords: random gaussian deviates
Message-ID: <C21C9n.Fop@mentor.cc.purdue.edu>
Date: 6 Feb 93 16:24:59 GMT
References: <1ktvb0INNb5p@jhunix.hcf.jhu.edu>
Sender: news@mentor.cc.purdue.edu (USENET News)
Distribution: inet
Organization: Purdue University Statistics Department
Lines: 74

In article <1ktvb0INNb5p@jhunix.hcf.jhu.edu> merkel@jhunix.hcf.jhu.edu (Frederick C Merkel) writes:
>We need to generate literally billions of gaussian deviates for some Monte
>Carlo simulations.  We had been using the  Box-Muller method to generate
>the deviates, but found some speed-up by using the ziggurat method described
>in Marsaglia & Tang (1984, SIAM J Sci Stat Comput, 5, 2).  (The speed-up
>over B-M seems to be highly system dependent).

>I have two general questions:
>1) Has anyone implemented the 256-layer algorithm?  Marsaglia describe the
>   algorithm/heuristic? for generating the necessary coefficients, but I am
>   a bit hesitant to calculate it myself.
>2) Is there a more efficient algorithm for generating Gaussian deviates?

There are far more efficient ways to do this, requiring little arithmetic.
They involve generating exponentials efficiently as well; the methods in
my 1976 technical report are quite good, although there may be a slight
error which would not be noticed in practice.  I have better ones, quite
suited for vector processing (although a little problematic on some) and
even parallel.  There are MANY tradeoffs, and the whole package has to
take into account startup and "catastrophes."

To generate exponentials, one at a time, which is NOT good.  But this
vectorizes well, at least on some machines, and even on non-vector machines,
can be done in a loop.

Input: k uniform integer [0, N-1], u uniform real (0,1), E standard exponential,and F and G standard exponential (if needed)

Output: X, E exponential.  Yes, this is the same E.

Real constants: ep[N], w[N], tl

        d = w[k]*u;
        E -= d;
        if(E >= 0) X=(ep[k] + d); end;
        else {E = F; X = tl + G; end;}

The normal is somewhat similar.  For the half-normal (similar things can be
done for the normal directly), one can do the following:

Input: k uniform integer [0, N-1], u uniform real (0,1), E standard exponential,F standard exponential and T tail normal if needed

Output: X half-normal, E exponential.  Yes, this is the same E.

Real constants: nep[N], nw[N]

        d = nw[k]*u; c=nep[k]; X=c+d;
        E -= .5*d*(c+X);
        if(E >= 0) end;
        else {E = F; X = G; end;}

Now this needs tail normals, on (a, \infty).  A good way of doing this again
uses exponentials, and this one recurses until done.  Using the constant
b = .5*(a + sqrt(a^2 + 4)) allows the following rather fast algorithm,
using only exponentials, with E being also in the output, and u, v, being
input:

do until done:
        d = (u-1)/b;
        E -= .5*d^2;
        if (E >= 0) {G = b+d; done;}
        else E=u; repeat;

It is the reuse of exponentials which makes these reasonably cheap.

I would be glad to cooperate in producing a "package" which includes these
algorithms, and also those for other distributions.  Interfacing is a problem
which I have considered.
--
Herman Rubin, Dept. of Statistics, Purdue Univ., West Lafayette IN47907-1399
Phone: (317)494-6054
hrubin@snap.stat.purdue.edu (Internet, bitnet)
{purdue,pur-ee}!snap.stat!hrubin(UUCP)


*/

double snorm2()
{
static int flag=0;
static double ans;
double fac,r,v1,v2;

if (flag == 0) {
   do {
      v1 = ranf();
      v1 = v1 + v1 - 1.0;
      v2 = ranf();
      v2 = v2 + v2 - 1.0;
      r=v1*v1+v2*v2;
      } while (r >= 1.0);
   fac=sqrt(-2.0*log(r)/r);
   ans=v1*fac;
   flag=1;
   return v2*fac;
   }
else {
   flag=0;
   return ans;
   }
}



/*
 * This is a quick and dirty version.
 */


double snorm3()
{
double result;
int i;

result = 0.0;
for (i=0; i<12; ++i)
   result += ranf();
result = result - 6.0;

return(result);
}
