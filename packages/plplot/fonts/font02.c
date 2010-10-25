/* $Id: font02.c 3186 2006-02-15 18:17:33Z slbrow $
   $Log$
   Revision 1.1  2006/02/15 18:15:45  slbrow
   This is the commit of the DAKOTA 'autoconfiscation' I've been working on, which
   enables builds using the GNU Autotools.  I have been merging in changes to the
   mainline the entire time I've been working on these changes to my working copy
   (in ~slbrow/projects/Dakota), so none of your changes to DAKOTA will be lost.
   In addition, the final pre-autotools version of DAKOTA has been tagged with:

   Version-3-3-Plus-pre-autotools

   The GNU autotools will be needed by all developers who check DAKOTA out from
   CVS with intentions to build; this is because the version of DAKOTA in the
   repository after this commit does not have any configure or Makefile.in files
   (since they are now machine-generated).  All the source tarballs rolled for
   external consumption (both release and votd) will automatically include all the
   configure and Makefile.in files our non-autotools savvy users will need, while
   still omitting those proprietary sources we may not distribute (DOT, NPSOL).

   To re-generate the missing configure and Makefile.in files in a checkout of
   DAKOTA, only one GNU Autotools script is necessary to run from the
   command-line:

   $ autoreconf --install

   This script (and all the scripts it runs in sequence on a copy of DAKOTA
   checked out of the repository) can be found on the SCICO LAN in the directory:

   /usr/netpub/autotools/bin

   This directory exists on all five of the DAKOTA nightly build platforms (AIX,
   IRIX, Linux, Solaris and Tru64), and should be added to the beginning of your
   $PATH if you are on the SCICO LAN.  If you are not, the versions of the tools
   you need can be easily downloaded from the GNU website.  The versions we use:

   autoconf 2.59  (http://ftp.gnu.org/gnu/autoconf/autoconf-2.59.tar.gz)
   automake 1.9.6 (http://ftp.gnu.org/gnu/automake/automake-1.9.6.tar.gz)
   libtool 1.5.22 (http://ftp.gnu.org/gnu/libtool/libtool-1.5.22.tar.gz)
   m4 1.4.3       (http://ftp.gnu.org/gnu/m4/m4-1.4.3.tar.gz)

   Here is a "short" summary of the major changes made in this )commit:

   1. All configure.in and Makefile.in files are no longer in the CVS repository.
      They are replaced by configure.ac and Makefile.am files, respectively.  When
      you run autoreconf, the configure and Makefile.in files are regenerated
      automatically and recursively.  No $DAKOTA environment variable needs to be
      set in order to build DAKOTA.

   2. Build directories as they were in DAKOTA using the Cygnus configure system
      no longer exist.  Executables built by the GNU Autotools (e.g., the dakota*
      binaries which were built in Dakota/src/i686-unknown-linux on Linux) are now
      built in Dakota/src; the autotools provide deployment targets for installing
      binaries and libraries in a deployment directory, as well as a simple means
      of maintaining more than one build of Dakota for multiple platforms (i.e.,
      using VPATH builds and source- and build-tree separation).

      Suppose you have in your home directory a checked-out and 'autoreconf'd copy
      of Dakota, and you want to simultaneously build versions of DAKOTA for Linux
      and Solaris.  You would create two uniquely named directories in your home
      directory, called 'my_linux_build' and 'my_solaris_build', respectively.
      Assuming two terminals are open on both machines, you would 'cd' into the
      appropriate build directory on each platform and run this command on both:

      $ ../Dakota/configure -C

      This will create a directory tree which is identical to the source tree, but
      containing only Makefiles.  Then, when you run 'make', it will traverse the
      build tree, referencing the sources by relative paths to create objects and
      binaries.  At your option, you can then delete these build directories
      without touching your source tree (the 'autoreconf'd checkout of Dakota).  I
      should note here that the GNU Autotools do not _force_ this separation of
      trees; you can do a 'make distclean' after building for each platform in
      your 'autoreconf'd copy of Dakota with no problems.

   3. All the configure flags which were supported in the old build system (i.e.,
      --without-acro, --enable-debugging, etc.), are supported in the new system
      with the following changes and additions:

      --with-mpi/--without-mpi           -> --enable-mpi/--disable-mpi
      --with-graphics/--without-graphics -> --enable-graphics/--disable-graphics
      --enable-docs (default is off)

      I should note that all the default settings have been preserved.  Further,
      since ACRO, OPT++ and DDACE haven't been integrated yet (see item 9), their
      flags are still commented out in 'Dakota/config/dak_vendoroptimizers.m4'.

   4. The epetra and plplot directories under VendorPackages have been completely
      replaced with the versions available online (versions 3.3 and 5.5.2,
      respectively), which were originally flattened to provide support for our
      build system.  Epetra has been altered from the version distributed by the
      Trilinos team to provide full libtool support as that is necessary for
      building mixed-language archives under Solaris, while plplot has had all
      language-bindings except C++ turned off by default.  Additionally, GSL has
      been updated from version 1.7 available online, but preserving the flattened
      structure it had prior to autoconfiscation.

   5. All other packages in the DAKOTA repository under VendorPackages and
      VendorOptimizers have been 'autoconfiscated' separately from DAKOTA.  To
      avoid duplicated configure-time probes, run configure from the top-level
      DAKOTA directory with the '-C' argument; this will cache the probe results
      from each subpackage, removing the need to do anything other than a lookup
      the next time the probe runs.

   6. No 'mpi' symbolic link needs to be made from VendorPackages/mpi to the MPI
      implementation you wish to build against; MPI wrapper scripts for the C++
      compiler are used instead; if you wish to change the default choices for the
      MPI implementation to build against, you can specify where the wrapper
      scripts you wish to use are via environment variables, which may be set
      on separate lines (using setenv in CSH), or with env on the configure line:

      $ env MPICC=mpicc MPICXX=mpiCC MPIF77=mpif77 ./configure -C

   7. All the clean-up make targets ('clean', 'distclean' and a new one for those
      unfamiliar with the GNU Autotools, 'maintainer-clean') are now machine-
      generated.  The targets remove increasing numbers of files as you would
      expect; the last one removes all the autotools-generated files, returning
      you to a pristine CVS checkout.

   8. Many changes have been made to the sources in Dakota/src:

      A) Fortran calls are now done using FC_FUNC and F77_FUNC macro wrappers,
         removing the need for #ifdef RS6K preprocessor checks for underscores.
      B) Platform testing preprocessor conditionals have been replaced* with the
         feature that was assumed to be supported on that platform 'a priori'
         * mostly: MPI implementation choice is still done by platform.
           See ParallelLibrary.C for details.
      C) TFLOPS and COUGAR #ifdef's have been removed, leaving their #else
         branches to execute unconditionally (as those machines have gone away).

   9. Since the GNU Autotools preferentially look for GCC compilers first; if you
      wish to build on Solaris with the Sun compilers you must set the compilers
      to use in environment variables in the same way the MPI wrappers must be set
      (see item 5 above):

       $ env CC=cc CXX=CC F77=f77 FC=f90 ./configure ...

   Additionally, there are some minor problems to fix with the new build system:

   10. This autoconfiscation covers Dakota, as integration with ACRO, OPT++ and
       DDACE has yet to be done.  I have integrated Surfpack and removed the
       separate ann and kriging packages from VendorPackages, based on positive
       results from some integration testing I've done on the Surfpack versions of
       ann and kriging against those in the separate versions in VendorPackages.

   11. Purely static-linked executables have proved difficult with libtool.  The
       way libtool sets things up, all the DAKOTA package libraries are static and
       linked as such, but system libraries are linked against dynamically (the
       way it is currently done on Solaris).  Strangely though, the MPI libraries
       aren't linked against dynamically, so this is an area for investigation
       (especially as I planned on having it working for the ASC White/Purple
       deployment of Dakota 4.0).

   12. Running 'make -j <n>' with positive integer n for parallel builds is not
       yet supported, as there are a couple packages with dependency requirements
       (LHS and IDR, namely) which preclude compiling in parallel until their
       requirements are met.  In LHS, those requirements arise from the need for
       all the *.mod module files to exist prior to anything which USE's them, and
       in IDR, they arise from the lex/yacc generated source dependencies.

   13. Disabling F90 support (via the --disable-f90 configure flag) doesn't work
       yet.  GNU Automake isn't quite smart enough yet to accept arbitrary
       suffixes for F90 files, so it expects that the suffixes will be .f90 or
       .f95, _not_ .F!  The Automake developers are working on this, so in the
       meantime, I'm using a hack.  I put that hack into Dakota/src/Makefile.am to
       address this situation (the same one I use in LHS, by the way, which sets
       F77 and FFLAGS to their F90-equivalents) doesn't work when F90 is disabled.

   14. The nightly build and test system (in Dakota/test/sqa) will need to be
       modified to use the new system, so the nightlies will be broken until I can
       get the scripts modified to use the new system.  That will be my next CVS
       checkin, and I don't expect it to take very long to make those mods.

   Revision 1.1  1992/05/20 21:33:20  furnish
   Initial checkin of the whole PLPLOT project.

*/

      short int subbuffer002[100] = {
      7621, 7877, 7868,   64, 7877, 8389, 8644, 8641, 8384, 7872,
         0,   -4, 7749, 7622, 8261, 8004, 7874, 7871, 7997, 8252,
      8380, 8637, 8767, 8770, 8644, 8389, 8261,   64, 8382, 8763,
         0,   -4, 7749, 7621, 7877, 7868,   64, 7877, 8389, 8644,
      8641, 8384, 7872,   64, 8256, 8636,    0,   -4, 7749, 7621,
      8644, 8389, 8133, 7876, 7875, 8002, 8512, 8639, 8637, 8380,
      8124, 7869,    0,   -4, 7749, 7621, 8261, 8252,   64, 7749,
      8773,    0,   -4, 7749, 7622, 7877, 7871, 7997, 8252, 8380,
      8637, 8767, 8773,    0,   -4, 7749, 7621, 7749, 8252,   64,
      8773, 8252,    0,   -4, 7749, 7494, 7749, 7996,   64, 8261
      };
      short int subbuffer003[100] = {
      7996,   64, 8261, 8508,   64, 8773, 8508,    0,   -4, 7749,
      7621, 7877, 8636,   64, 8645, 7868,    0,   -4, 7749, 7621,
      7749, 8257, 8252,   64, 8773, 8257,    0,   -4, 7749, 7621,
      8645, 7868,   64, 7877, 8645,   64, 7868, 8636,    0,   -4,
      7749, 7621, 8261, 7740,   64, 8261, 8764,   64, 7999, 8511,
         0,   -4, 7749, 7621, 7877, 7868,   64, 7877, 8389, 8644,
      8642, 8385,   64, 7873, 8385, 8640, 8637, 8380, 7868,    0,
        -4, 7749, 7620, 7877, 7868,   64, 7877, 8645,    0,   -4,
      7749, 7621, 8261, 7740,   64, 8261, 8764,   64, 7740, 8764,
         0,   -4, 7749, 7621, 7877, 7868,   64, 7877, 8645,   64
      };
      short int subbuffer004[100] = {
      7873, 8385,   64, 7868, 8636,    0,   -4, 7749, 7621, 8645,
      7868,   64, 7877, 8645,   64, 7868, 8636,    0,   -4, 7749,
      7621, 7877, 7868,   64, 8645, 8636,   64, 7873, 8641,    0,
        -4, 7749, 7622, 8261, 8004, 7874, 7871, 7997, 8252, 8380,
      8637, 8767, 8770, 8644, 8389, 8261,   64, 8129, 8512,   64,
      8513, 8128,    0,   -4, 7749, 8002, 8261, 8252,    0,   -4,
      7749, 7621, 7877, 7868,   64, 8645, 7871,   64, 8129, 8636,
         0,   -4, 7749, 7621, 8261, 7740,   64, 8261, 8764,    0,
        -4, 7749, 7494, 7749, 7740,   64, 7749, 8252,   64, 8773,
      8252,   64, 8773, 8764,    0,   -4, 7749, 7621, 7877, 7868
      };
      short int subbuffer005[100] = {
        64, 7877, 8636,   64, 8645, 8636,    0,   -4, 7749, 7621,
      7877, 8645,   64, 8001, 8512,   64, 8513, 8000,   64, 7868,
      8636,    0,   -4, 7749, 7622, 8261, 8004, 7874, 7871, 7997,
      8252, 8380, 8637, 8767, 8770, 8644, 8389, 8261,    0,   -4,
      7749, 7621, 7877, 7868,   64, 8645, 8636,   64, 7877, 8645,
         0,   -4, 7749, 7621, 7877, 7868,   64, 7877, 8389, 8644,
      8641, 8384, 7872,    0,   -4, 7749, 7621, 7877, 8257, 7868,
        64, 7877, 8645,   64, 7868, 8636,    0,   -4, 7749, 7621,
      8261, 8252,   64, 7749, 8773,    0,   -4, 7749, 7621, 7747,
      7748, 7877, 8005, 8132, 8258, 8252,   64, 8771, 8772, 8645
      };
      short int subbuffer006[100] = {
      8517, 8388, 8258,    0,   -4, 7749, 7494, 8261, 8252,   64,
      8003, 7746, 7743, 7998, 8510, 8767, 8770, 8515, 8003,    0,
        -4, 7749, 7621, 7877, 8636,   64, 8645, 7868,    0,   -4,
      7749, 7494, 8261, 8252,   64, 7747, 7874, 7871, 8126, 8382,
      8639, 8642, 8771,    0,   -4, 7749, 7622, 7868, 8124, 7872,
      7874, 8004, 8261, 8389, 8644, 8770, 8768, 8508, 8764,    0,
        -4, 7749, 8256,   64,    0,   -4, 7749, 8002,   64,    0,
        -4, 7749, 7748,   64,    0,   -4, 7749, 7621, 8261, 8004,
      7874, 7871, 7997, 8252, 8509, 8639, 8642, 8516, 8261,    0,
        -4, 7749, 7621, 8003, 8261, 8252,    0,   -4, 7749, 7621
      };
      short int subbuffer007[100] = {
      7876, 8133, 8389, 8644, 8642, 8512, 7868, 8636,    0,   -4,
      7749, 7621, 7876, 8133, 8389, 8644, 8642, 8385,   64, 8257,
      8385, 8640, 8637, 8380, 8124, 7869,    0,   -4, 7749, 7621,
      8389, 8380,   64, 8389, 7743, 8767,    0,   -4, 7749, 7621,
      8005, 7873, 8130, 8258, 8513, 8639, 8509, 8252, 8124, 7869,
        64, 8005, 8517,    0,   -4, 7749, 7621, 8517, 8261, 8004,
      7874, 7871, 7997, 8252, 8509, 8639, 8513, 8258, 8001, 7871,
         0,   -4, 7749, 7621, 8645, 8124,   64, 7877, 8645,    0,
        -4, 7749, 7621, 8133, 7876, 7874, 8129, 8385, 8642, 8644,
      8389, 8133,   64, 8129, 7872, 7869, 8124, 8380, 8637, 8640
      };
      short int subbuffer008[100] = {
      8385,    0,   -4, 7749, 7621, 8642, 8512, 8255, 8000, 7874,
      8004, 8261, 8516, 8642, 8639, 8509, 8252, 7996,    0,   -4,
      7749, 8003, 8253, 8252, 8380, 8381, 8253,    0,   -4, 7749,
      8003, 8380, 8252, 8253, 8381, 8379, 8249,    0,   -4, 7749,
      8003, 8258, 8257, 8385, 8386, 8258,   64, 8253, 8252, 8380,
      8381, 8253,    0,   -4, 7749, 8003, 8258, 8257, 8385, 8386,
      8258,   64, 8380, 8252, 8253, 8381, 8379, 8249,    0,   -4,
      7749, 8003, 8261, 8256,   64, 8389, 8384,   64, 8253, 8252,
      8380, 8381, 8253,    0,   -4, 7749, 7749, 8004, 8261, 8389,
      8644, 8642, 8257, 8256, 8384, 8385, 8642,   64, 8253, 8252
      };
      short int subbuffer009[100] = {
      8380, 8381, 8253,    0,   -4, 7749, 8002, 8261, 8257,    0,
        -4, 7749, 7748, 8005, 8001,   64, 8517, 8513,    0,   -4,
      7749, 7748, 8133, 8004, 8002, 8129, 8385, 8514, 8516, 8389,
      8133,    0,   -4, 7749, 7621, 8644, 8389, 8133, 7876, 7874,
      8129, 8512, 8639, 8637, 8380, 8124, 7869,   64, 8262, 8251,
         0,   -4, 7749, 7621, 8774, 7739,    0,   -4, 7749, 7876,
      8518, 8260, 8130, 8127, 8253, 8507,    0,   -4, 7749, 7747,
      8006, 8260, 8386, 8383, 8253, 7995,    0,   -4, 7749, 8002,
      8262, 8251,    0,   -4, 7749, 7494, 7744, 8768,    0,   -4,
      7749, 7494, 8260, 8252,   64, 7744, 8768,    0,   -4, 7749
      };
      short int subbuffer010[100] = {
      7494, 7746, 8770,   64, 7742, 8766,    0,   -4, 7749, 7621,
      7875, 8637,   64, 8643, 7869,    0,   -4, 7749, 7621, 8259,
      8253,   64, 7874, 8638,   64, 8642, 7870,    0,   -4, 7749,
      8003, 8257, 8256, 8384, 8385, 8257,    0,   -4, 7749, 8003,
      8389, 8259, 8257, 8385, 8386, 8258,    0,   -4, 7749, 8003,
      8388, 8260, 8261, 8389, 8387, 8257,    0,   -4, 7749, 7494,
      8387, 8768, 8381,   64, 7744, 8768,    0,   -4, 7749, 7622,
      8262, 7995,   64, 8646, 8379,   64, 7874, 8770,   64, 7871,
      8767,    0,   -4, 7749, 7494, 8768, 8640, 8511, 8381, 8252,
      7868, 7741, 7743, 7872, 8257, 8386, 8388, 8261, 8005, 7876
      };
      short int subbuffer011[100] = {
      7874, 8127, 8381, 8636, 8764,    0,   -4, 7749, 7494, 7748,
      7873, 7871, 7740,   64, 8772, 8641, 8639, 8764,   64, 7748,
      8131, 8387, 8772,   64, 7740, 8125, 8381, 8764,    0,   -4,
      7749, 7621, 8382, 8386, 8258, 8130, 8001, 7999, 8126, 8510,
      8768, 8769, 8643, 8388, 8132, 7875, 7745, 7743, 7869, 8124,
      8380, 8637,    0,  -16, 7116, 6986, 7744, 7617, 7488, 7615,
      7744,   64, 8768, 8897, 9024, 8895, 8768,    0,  -16, 7116,
      6986, 7618, 7489, 7488, 7615, 7743, 7872, 7873, 7746, 7618,
        64, 7617, 7616, 7744, 7745, 7617,   64, 8898, 8769, 8768,
      8895, 9023, 9152, 9153, 9026, 8898,   64, 8897, 8896, 9024
      };
      short int subbuffer012[100] = {
      9025, 8897,    0,  -16, 7116, 7113, 7360, 9152,    0,  -16,
      7116, 6986, 8253, 7234, 8254, 9282, 8253,    0,  -16, 7116,
      6732, 7103, 7233, 7490, 7746, 8001, 8510, 8765, 9021, 9278,
      9409,   64, 7103, 7234, 7491, 7747, 8002, 8511, 8766, 9022,
      9279, 9409,   64, 7097, 7227, 7484, 7740, 7995, 8504, 8759,
      9015, 9272, 9403,   64, 7097, 7228, 7485, 7741, 7996, 8505,
      8760, 9016, 9273, 9403,    0,  -16, 7116, 6732, 7103, 7233,
      7490, 7746, 8001, 8510, 8765, 9021, 9278, 9409,   64, 7103,
      7234, 7491, 7747, 8002, 8511, 8766, 9022, 9279, 9409,   64,
      7099, 9403,   64, 7095, 9399,    0,  -16, 7116, 6351, 6851
      };
      short int subbuffer013[100] = {
      9667, 9033,   64, 7479, 6845, 9661,    0,  -16, 7116, 6478,
      6708, 9780, 9804, 6732, 6708,    0,  -16, 7116, 7367, 8124,
      7869, 7743, 7745, 7875, 8132, 8388, 8643, 8769, 8767, 8637,
      8380, 8124,   64, 8380, 8767,   64, 8124, 8769,   64, 7869,
      8643,   64, 7743, 8388,   64, 7745, 8132,    0,  -16, 7116,
      6986, 7104, 9408,    0,  -16, 7116, 6986, 7104, 9408,   64,
      7105, 9409,    0,  -16, 7116, 7495, 8012, 8399, 8386,   64,
      8270, 8258,   64, 7874, 8770,   64, 7616, 9024,   64, 7868,
      7867, 7739, 7740, 7869, 8126, 8510, 8765, 8891, 8761, 8504,
      8119, 7862, 7732, 7729,   64, 8510, 8637, 8763, 8633, 8504
      };
      short int subbuffer014[100] = {
        64, 7730, 7859, 7987, 8370, 8754, 8883,   64, 7987, 8369,
      8753, 8883, 8884,    0,  -16, 7116, 7495, 8012, 8399, 8386,
        64, 8270, 8258,   64, 7874, 8770,   64, 7616, 9024,   64,
      7868, 7867, 7739, 7740, 7869, 8126, 8510, 8765, 8891, 8761,
      8504,   64, 8510, 8637, 8763, 8633, 8504,   64, 8248, 8504,
      8759, 8885, 8884, 8754, 8497, 8113, 7858, 7731, 7732, 7860,
      7859,   64, 8504, 8631, 8757, 8756, 8626, 8497,    0,  -16,
      7116, 7495, 8012, 8399, 8386,   64, 8270, 8258,   64, 7874,
      8770,   64, 7616, 9024,   64, 8764, 8763, 8891, 8892, 8765,
      8510, 8254, 7997, 7868, 7737, 7732, 7858, 8113, 8497, 8754
      };
      short int subbuffer015[100] = {
      8884, 8886, 8760, 8505, 8121, 7735,   64, 8254, 7996, 7865,
      7860, 7986, 8113,   64, 8497, 8626, 8756, 8758, 8632, 8505,
         0,  -16, 7116, 7495, 8012, 8399, 8386,   64, 8270, 8258,
        64, 7874, 8770,   64, 7616, 9024,   64, 8126, 7869, 7739,
      7865, 8120, 8504, 8761, 8891, 8765, 8510, 8126,   64, 8126,
      7997, 7867, 7993, 8120,   64, 8504, 8633, 8763, 8637, 8510,
        64, 8120, 7863, 7733, 7732, 7858, 8113, 8497, 8754, 8884,
      8885, 8759, 8504,   64, 8120, 7991, 7861, 7860, 7986, 8113,
        64, 8497, 8626, 8756, 8757, 8631, 8504,    0,  -16, 7116,
      7495, 7885, 7884, 7756, 7757, 7886, 8143, 8527, 8782, 8908
      };
      short int subbuffer016[100] = {
      8778, 8521, 8136, 7879, 7749, 7746,   64, 8527, 8654, 8780,
      8650, 8521,   64, 7747, 7876, 8004, 8387, 8771, 8900,   64,
      8004, 8386, 8770, 8900, 8901,   64, 7616, 9024,   64, 7868,
      7867, 7739, 7740, 7869, 8126, 8510, 8765, 8891, 8761, 8504,
        64, 8510, 8637, 8763, 8633, 8504,   64, 8248, 8504, 8759,
      8885, 8884, 8754, 8497, 8113, 7858, 7731, 7732, 7860, 7859,
        64, 8504, 8631, 8757, 8756, 8626, 8497,    0,  -16, 7116,
      7495, 7885, 7884, 7756, 7757, 7886, 8143, 8527, 8782, 8908,
      8778, 8521,   64, 8527, 8654, 8780, 8650, 8521,   64, 8265,
      8521, 8776, 8902, 8901, 8771, 8514, 8130, 7875, 7748, 7749
      };
      short int subbuffer017[100] = {
      7877, 7876,   64, 8521, 8648, 8774, 8773, 8643, 8514,   64,
      7616, 9024,   64, 8126, 7869, 7739, 7865, 8120, 8504, 8761,
      8891, 8765, 8510, 8126,   64, 8126, 7997, 7867, 7993, 8120,
        64, 8504, 8633, 8763, 8637, 8510,   64, 8120, 7863, 7733,
      7732, 7858, 8113, 8497, 8754, 8884, 8885, 8759, 8504,   64,
      8120, 7991, 7861, 7860, 7986, 8113,   64, 8497, 8626, 8756,
      8757, 8631, 8504,    0,  -16, 7116, 7495, 7887, 7753,   64,
      7887, 8783,   64, 7886, 8398, 8783,   64, 7753, 7882, 8139,
      8523, 8778, 8904, 8901, 8771, 8514, 8130, 7875, 7748, 7749,
      7877, 7876,   64, 8523, 8650, 8776, 8773, 8643, 8514,   64
      };
      short int subbuffer018[100] = {
      7616, 9024,   64, 8126, 7869, 7739, 7865, 8120, 8504, 8761,
      8891, 8765, 8510, 8126,   64, 8126, 7997, 7867, 7993, 8120,
        64, 8504, 8633, 8763, 8637, 8510,   64, 8120, 7863, 7733,
      7732, 7858, 8113, 8497, 8754, 8884, 8885, 8759, 8504,   64,
      8120, 7991, 7861, 7860, 7986, 8113,   64, 8497, 8626, 8756,
      8757, 8631, 8504,    0,  -16, 7116, 7495, 7759, 7755,   64,
      8781, 8262, 8002,   64, 8911, 8521, 8130,   64, 7757, 8015,
      8271, 8653,   64, 7757, 8014, 8270, 8653, 8781,   64, 7616,
      9024,   64, 8126, 7869, 7739, 7865, 8120, 8504, 8761, 8891,
      8765, 8510, 8126,   64, 8126, 7997, 7867, 7993, 8120,   64
      };
      short int subbuffer019[100] = {
      8504, 8633, 8763, 8637, 8510,   64, 8120, 7863, 7733, 7732,
      7858, 8113, 8497, 8754, 8884, 8885, 8759, 8504,   64, 8120,
      7991, 7861, 7860, 7986, 8113,   64, 8497, 8626, 8756, 8757,
      8631, 8504,    0,  -16, 7116, 7495, 7887, 7753,   64, 7887,
      8783,   64, 7886, 8398, 8783,   64, 7753, 7882, 8139, 8523,
      8778, 8904, 8901, 8771, 8514, 8130, 7875, 7748, 7749, 7877,
      7876,   64, 8523, 8650, 8776, 8773, 8643, 8514,   64, 7616,
      9024,   64, 8764, 8763, 8891, 8892, 8765, 8510, 8254, 7997,
      7868, 7737, 7732, 7858, 8113, 8497, 8754, 8884, 8886, 8760,
      8505, 8121, 7735,   64, 8254, 7996, 7865, 7860, 7986, 8113
      };
      short int subbuffer020[100] = {
        64, 8497, 8626, 8756, 8758, 8632, 8505,    0,  -16, 7116,
      7495, 8012, 8399, 8386,   64, 8270, 8258,   64, 7874, 8770,
        64, 7616, 9024,   64, 8380, 8369,   64, 8510, 8497,   64,
      8510, 7605, 9013,   64, 8113, 8753,    0,  -16, 7116, 7495,
      7885, 7884, 7756, 7757, 7886, 8143, 8527, 8782, 8908, 8778,
      8521,   64, 8527, 8654, 8780, 8650, 8521,   64, 8265, 8521,
      8776, 8902, 8901, 8771, 8514, 8130, 7875, 7748, 7749, 7877,
      7876,   64, 8521, 8648, 8774, 8773, 8643, 8514,   64, 7616,
      9024,   64, 8380, 8369,   64, 8510, 8497,   64, 8510, 7605,
      9013,   64, 8113, 8753,    0,  -16, 7116, 6732, 9146, 8888
      };
      short int subbuffer021[100] = {
      8631, 8247, 7992, 7610, 7355, 7099, 6970, 6968, 7095, 7351,
      7608, 7867, 7997, 8257, 8519, 8907, 9164, 9292, 9419, 9546,
      9417, 9290, 9419, 9420, 9292, 9164, 8907, 8778, 8518, 8258,
      7996, 7736, 7608,   64, 7874, 8642,    0,  -16, 7116, 6604,
      7991, 7608, 7226, 6973, 6848, 6851, 6982, 7241, 7627, 8012,
      8524, 8907, 9161, 9414, 9539, 9536, 9405, 9146, 8760, 8375,
      7991,   64, 7752, 7739,   64, 7880, 7867,   64, 7496, 8392,
      8775, 8901, 8900, 8770, 8385, 7873,   64, 8392, 8647, 8773,
      8772, 8642, 8385,   64, 7483, 8123,   64, 8385, 8512, 8636,
      8763, 8891, 9020,   64, 8385, 8640, 8764, 8891,    0,  -16
      };
      short int subbuffer022[100] = {
      7116, 6604, 7991, 7608, 7226, 6973, 6848, 6851, 6982, 7241,
      7627, 8012, 8524, 8907, 9161, 9414, 9539, 9536, 9405, 9146,
      8760, 8375, 7991,   64, 8647, 8776, 8772, 8647, 8392, 8008,
      7751, 7622, 7491, 7488, 7613, 7740, 7995, 8379, 8636, 8766,
        64, 8008, 7750, 7619, 7616, 7741, 7995,    0,  -16, 7116,
      6605, 7109, 9413,   64, 7104, 9408,   64, 7099, 9403,   64,
      7351, 9161,    0,  -16, 7116, 6351, 6977, 6848, 6975, 7104,
      6977,   64, 8257, 8128, 8255, 8384, 8257,   64, 9537, 9408,
      9535, 9664, 9537,    0,  -16, 7116, 6605, 7739, 7486, 7104,
      7490, 7749,   64, 7490, 7232, 7486,   64, 7232, 9280,   64
      };
      short int subbuffer023[100] = {
      8763, 9022, 9408, 9026, 8773,   64, 9026, 9280, 9022,    0,
       -16, 7116, 7240, 8900, 8518, 8265, 8006, 7620,   64, 8006,
      8264, 8518,   64, 8264, 8248,   64, 8892, 8506, 8247, 7994,
      7612,   64, 7994, 8248, 8506,    0,  -16, 7116, 6732, 9288,
      8392, 7879, 7622, 7364, 7233, 7231, 7356, 7610, 7865, 8376,
      9272,   64, 8396, 8372,    0,  -16, 7116, 6732, 7240, 8136,
      8647, 8902, 9156, 9281, 9279, 9148, 8890, 8633, 8120, 7224,
        64, 8140, 8116,    0,  -16, 7116, 6732, 7240, 8136, 8647,
      8902, 9156, 9281, 9279, 9148, 8890, 8633, 8120, 7224,   64,
      7872, 9280,    0,  -16, 7116, 6986, 7504, 7479,   64, 8272
      };
      short int subbuffer024[100] = {
      8247,   64, 9040, 9015,    0,  -16, 7116, 7113, 8268, 7223,
        64, 8268, 9271,   64, 7614, 8894,    0,  -16, 7116, 6858,
      7372, 7351,   64, 7372, 8524, 8907, 9034, 9160, 9158, 9028,
      8899, 8514,   64, 7362, 8514, 8897, 9024, 9150, 9147, 9017,
      8888, 8503, 7351,    0,  -16, 7116, 6987, 9287, 9161, 8907,
      8652, 8140, 7883, 7625, 7495, 7364, 7359, 7484, 7610, 7864,
      8119, 8631, 8888, 9146, 9276,    0,  -16, 7116, 6858, 7372,
      7351,   64, 7372, 8268, 8651, 8905, 9031, 9156, 9151, 9020,
      8890, 8632, 8247, 7351,    0,  -16, 7116, 6985, 7500, 7479,
        64, 7500, 9164,   64, 7490, 8514,   64, 7479, 9143,    0
      };
      short int subbuffer025[100] = {
       -16, 7116, 6984, 7500, 7479,   64, 7500, 9164,   64, 7490,
      8514,    0,  -16, 7116, 6987, 9287, 9161, 8907, 8652, 8140,
      7883, 7625, 7495, 7364, 7359, 7484, 7610, 7864, 8119, 8631,
      8888, 9146, 9276, 9279,   64, 8639, 9279,    0,  -16, 7116,
      6859, 7372, 7351,   64, 9164, 9143,   64, 7362, 9154,    0,
       -16, 7116, 7748, 8268, 8247,    0,  -16, 7116, 7240, 8780,
      8764, 8633, 8504, 8247, 7991, 7736, 7609, 7484, 7486,    0,
       -16, 7116, 6858, 7372, 7351,   64, 9164, 7358,   64, 8003,
      9143,    0,  -16, 7116, 6983, 7500, 7479,   64, 7479, 9015,
         0,  -16, 7116, 6732, 7244, 7223,   64, 7244, 8247,   64
      };
      short int subbuffer026[100] = {
      9292, 8247,   64, 9292, 9271,    0,  -16, 7116, 6859, 7372,
      7351,   64, 7372, 9143,   64, 9164, 9143,    0,  -16, 7116,
      6859, 8012, 7755, 7497, 7367, 7236, 7231, 7356, 7482, 7736,
      7991, 8503, 8760, 9018, 9148, 9279, 9284, 9159, 9033, 8779,
      8524, 8012,    0,  -16, 7116, 6858, 7372, 7351,   64, 7372,
      8524, 8907, 9034, 9160, 9157, 9027, 8898, 8513, 7361,    0,
       -16, 7116, 6859, 8012, 7755, 7497, 7367, 7236, 7231, 7356,
      7482, 7736, 7991, 8503, 8760, 9018, 9148, 9279, 9284, 9159,
      9033, 8779, 8524, 8012,   64, 8379, 9141,    0,  -16, 7116,
      6858, 7372, 7351,   64, 7372, 8524, 8907, 9034, 9160, 9158
      };
      short int subbuffer027[100] = {
      9028, 8899, 8514, 7362,   64, 8258, 9143,    0,  -16, 7116,
      6986, 9161, 8907, 8524, 8012, 7627, 7369, 7367, 7493, 7620,
      7875, 8641, 8896, 9023, 9149, 9146, 8888, 8503, 7991, 7608,
      7354,    0,  -16, 7116, 7240, 8268, 8247,   64, 7372, 9164,
         0,  -16, 7116, 6859, 7372, 7357, 7482, 7736, 8119, 8375,
      8760, 9018, 9149, 9164,    0,  -16, 7116, 7113, 7244, 8247,
        64, 9292, 8247,    0,  -16, 7116, 6732, 6988, 7607,   64,
      8268, 7607,   64, 8268, 8887,   64, 9548, 8887,    0,  -16,
      7116, 6986, 7372, 9143,   64, 9164, 7351,    0,  -16, 7116,
      7113, 7244, 8258, 8247,   64, 9292, 8258,    0,  -16, 7116
      };
      short int subbuffer028[100] = {
      6986, 9164, 7351,   64, 7372, 9164,   64, 7351, 9143,    0,
       -16, 7116, 7113, 8268, 7223,   64, 8268, 9271,   64, 7614,
      8894,    0,  -16, 7116, 6858, 7372, 7351,   64, 7372, 8524,
      8907, 9034, 9160, 9158, 9028, 8899, 8514,   64, 7362, 8514,
      8897, 9024, 9150, 9147, 9017, 8888, 8503, 7351,    0,  -16,
      7116, 6983, 7500, 7479,   64, 7500, 9036,    0,  -16, 7116,
      7113, 8268, 7223,   64, 8268, 9271,   64, 7223, 9271,    0,
       -16, 7116, 6985, 7500, 7479,   64, 7500, 9164,   64, 7490,
      8514,   64, 7479, 9143,    0,  -16, 7116, 6986, 9164, 7351,
        64, 7372, 9164,   64, 7351, 9143,    0,  -16, 7116, 6859
      };
      short int subbuffer029[100] = {
      7372, 7351,   64, 9164, 9143,   64, 7362, 9154,    0,  -16,
      7116, 6859, 8012, 7755, 7497, 7367, 7236, 7231, 7356, 7482,
      7736, 7991, 8503, 8760, 9018, 9148, 9279, 9284, 9159, 9033,
      8779, 8524, 8012,   64, 7874, 8642,    0,  -16, 7116, 7748,
      8268, 8247,    0,  -16, 7116, 6858, 7372, 7351,   64, 9164,
      7358,   64, 8003, 9143,    0,  -16, 7116, 7113, 8268, 7223,
        64, 8268, 9271,    0,  -16, 7116, 6732, 7244, 7223,   64,
      7244, 8247,   64, 9292, 8247,   64, 9292, 9271,    0,  -16,
      7116, 6859, 7372, 7351,   64, 7372, 9143,   64, 9164, 9143,
         0,  -16, 7116, 7113, 7372, 9164,   64, 7874, 8642,   64
      };
      short int subbuffer030[100] = {
      7351, 9143,    0,  -16, 7116, 6859, 8012, 7755, 7497, 7367,
      7236, 7231, 7356, 7482, 7736, 7991, 8503, 8760, 9018, 9148,
      9279, 9284, 9159, 9033, 8779, 8524, 8012,    0,  -16, 7116,
      6859, 7372, 7351,   64, 9164, 9143,   64, 7372, 9164,    0,
       -16, 7116, 6858, 7372, 7351,   64, 7372, 8524, 8907, 9034,
      9160, 9157, 9027, 8898, 8513, 7361,    0,  -16, 7116, 7113,
      7372, 8258, 7351,   64, 7372, 9164,   64, 7351, 9143,    0,
       -16, 7116, 7240, 8268, 8247,   64, 7372, 9164,    0,  -16,
      7116, 7113, 7367, 7369, 7499, 7628, 7884, 8011, 8137, 8261,
      8247,   64, 9159, 9161, 9035, 8908, 8652, 8523, 8393, 8261
      };
      short int subbuffer031[100] = {
         0,  -16, 7116, 6986, 8268, 8247,   64, 8007, 7622, 7493,
      7363, 7360, 7486, 7613, 7996, 8508, 8893, 9022, 9152, 9155,
      9029, 8902, 8519, 8007,    0,  -16, 7116, 6986, 7372, 9143,
        64, 7351, 9164,    0,  -16, 7116, 6859, 8268, 8247,   64,
      7110, 7238, 7365, 7489, 7615, 7742, 8125, 8381, 8766, 8895,
      9025, 9157, 9286, 9414,    0,  -16, 7116, 6986, 7351, 7863,
      7486, 7362, 7366, 7497, 7755, 8140, 8396, 8779, 9033, 9158,
      9154, 9022, 8631, 9143,    0,  -16, 7116, 6857, 6839, 7096,
      7483, 7871, 8390, 8780, 8759, 8634, 8381, 8127, 7745, 7489,
      7360, 7358, 7484, 7738, 8120, 8503, 9143,    0,  -16, 7116
      };
      short int subbuffer032[100] = {
      6731, 8394, 8521, 8518, 8386, 8255, 8125, 7866, 7608, 7351,
      7223, 7096, 7099, 7232, 7363, 7493, 7752, 8010, 8267, 8652,
      9036, 9291, 9417, 9415, 9285, 9156, 8899, 8514,   64, 8386,
      8514, 8897, 9024, 9150, 9147, 9017, 8888, 8631, 8247, 7992,
      7866,    0,  -16, 7116, 6986, 8518, 8517, 8644, 8900, 9157,
      9287, 9289, 9163, 8908, 8524, 8139, 7881, 7622, 7492, 7360,
      7356, 7481, 7608, 7863, 8119, 8504, 8762, 8892,    0,  -16,
      7116, 6860, 8524, 8267, 8137, 8005, 7871, 7740, 7610, 7352,
      7095, 6839, 6712, 6714, 6843, 7099, 7354, 7608, 7991, 8375,
      8760, 9018, 9278, 9411, 9415, 9290, 9163, 8908, 8524, 8266
      };
      short int subbuffer033[100] = {
      8264, 8389, 8642, 8896, 9278, 9533,    0,  -16, 7116, 6986,
      8776, 8775, 8902, 9158, 9287, 9289, 9163, 8780, 8268, 7883,
      7753, 7750, 7876, 8003, 8386, 8002, 7617, 7488, 7358, 7355,
      7481, 7608, 7991, 8375, 8760, 9018, 9148,    0,  -16, 7116,
      6986, 8262, 8006, 7751, 7625, 7755, 8140, 8524, 9035, 9419,
      9676,   64, 9035, 8772, 8510, 8250, 7992, 7735, 7479, 7224,
      7098, 7100, 7229, 7485, 7740,   64, 8130, 9282,    0,  -16,
      7116, 6860, 6839, 7096, 7612, 8001, 8132, 8264, 8267, 8140,
      8012, 7883, 7753, 7750, 7876, 8131, 8643, 9028, 9157, 9287,
      9281, 9148, 9018, 8760, 8375, 7863, 7480, 7226, 7100, 7102
      };
      short int subbuffer034[100] = {
         0,  -16, 7116, 6732, 7621, 7366, 7240, 7241, 7371, 7628,
      7756, 8011, 8137, 8135, 8003, 7741, 7481, 7223, 6967, 6840,
      6842,   64, 7616, 8771, 9028, 9414, 9672, 9802, 9803, 9676,
      9548, 9290, 9030, 8768, 8635, 8632, 8759, 8887, 9144, 9273,
      9532,    0,  -16, 7116, 7112, 8892, 8638, 8385, 8259, 8134,
      8137, 8267, 8396, 8652, 8779, 8905, 8902, 8769, 8508, 8378,
      8120, 7863, 7607, 7352, 7226, 7228, 7357, 7613, 7868,    0,
       -16, 7116, 7239, 8500, 8247, 7996, 7874, 7880, 8011, 8268,
      8524, 8651, 8776, 8773, 8640, 8247, 7985, 7854, 7724, 7467,
      7340, 7342, 7473, 7732, 7990, 8376, 8890,    0,  -16, 7116
      };
      short int subbuffer035[100] = {
      6732, 7621, 7366, 7240, 7241, 7371, 7628, 7756, 8011, 8137,
      8135, 8003, 7741, 7481, 7223, 6967, 6840, 6842,   64, 9801,
      9803, 9676, 9548, 9291, 9033, 8774, 8516, 8259, 8003,   64,
      8259, 8385, 8378, 8504, 8631, 8759, 9016, 9145, 9404,    0,
       -16, 7116, 7114, 7616, 7872, 8385, 8771, 9029, 9159, 9162,
      9036, 8780, 8651, 8521, 8388, 8255, 8124, 7994, 7736, 7479,
      7223, 7096, 7098, 7227, 7483, 7738, 8120, 8503, 8759, 9144,
      9402,    0,  -16, 7116, 5967, 6597, 6342, 6216, 6217, 6347,
      6604, 6732, 6987, 7113, 7111, 6978, 6846, 6583,   64, 6846,
      7238, 7498, 7627, 7884, 8012, 8267, 8393, 8391, 8258, 8126
      };
