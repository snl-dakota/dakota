/* $Id: font05.c 3186 2006-02-15 18:17:33Z slbrow $
   $Log$
   Revision 1.1  2006/02/15 18:15:44  slbrow
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

   Revision 1.1  1992/05/20 21:33:24  furnish
   Initial checkin of the whole PLPLOT project.

*/

      short int subbuffer100[100] = {
       7741, 7996,   64, 8636, 8893, 9022, 9152, 9155, 9029, 8902,
       8647,   64, 7884, 8780,   64, 7863, 8759,    0,  -16, 7116,
       6986, 7372, 9015,   64, 7500, 9143,   64, 9164, 7351,   64,
       7116, 7884,   64, 8652, 9420,   64, 7095, 7863,   64, 8631,
       9399,    0,  -16, 7116, 6860, 8268, 8247,   64, 8396, 8375,
         64, 7109, 7238, 7493, 7617, 7743, 7870, 8125,   64, 7238,
       7365, 7489, 7615, 7742, 8125, 8509, 8894, 9023, 9153, 9285,
       9414,   64, 8509, 8766, 8895, 9025, 9157, 9414, 9541,   64,
       7884, 8780,   64, 7863, 8759,    0,  -16, 7116, 6859, 7226,
       7351, 7863, 7611, 7359, 7234, 7238, 7369, 7627, 8012, 8524
      };
      short int subbuffer101[100] = {
       8907, 9161, 9286, 9282, 9151, 8891, 8631, 9143, 9274,   64,
       7611, 7486, 7362, 7366, 7497, 7755, 8012,   64, 8524, 8779,
       9033, 9158, 9154, 9022, 8891,   64, 7352, 7736,   64, 8760,
       9144,    0,  -16, 7116, 6986, 8652, 6967,   64, 8652, 8759,
         64, 8522, 8631,   64, 7485, 8637,   64, 6711, 7479,   64,
       8247, 9015,    0,  -16, 7116, 6732, 7884, 7095,   64, 8012,
       7223,   64, 7500, 8908, 9291, 9417, 9415, 9284, 9155, 8770,
         64, 8908, 9163, 9289, 9287, 9156, 9027, 8770,   64, 7618,
       8770, 9025, 9151, 9149, 9018, 8760, 8247, 6711,   64, 8770,
       8897, 9023, 9021, 8890, 8632, 8247,    0,  -16, 7116, 6987
      };
      short int subbuffer102[100] = {
       9290, 9418, 9548, 9414, 9416, 9290, 9163, 8908, 8524, 8139,
       7881, 7622, 7491, 7359, 7356, 7481, 7608, 7991, 8375, 8632,
       8890, 9020,   64, 8524, 8267, 8009, 7750, 7619, 7487, 7484,
       7609, 7736, 7991,    0,  -16, 7116, 6731, 7884, 7095,   64,
       8012, 7223,   64, 7500, 8652, 9035, 9162, 9287, 9283, 9151,
       8891, 8633, 8376, 7863, 6711,   64, 8652, 8907, 9034, 9159,
       9155, 9023, 8763, 8505, 8248, 7863,    0,  -16, 7116, 6731,
       7884, 7095,   64, 8012, 7223,   64, 8518, 8254,   64, 7500,
       9420, 9286, 9292,   64, 7618, 8386,   64, 6711, 8631, 8892,
       8503,    0,  -16, 7116, 6730, 7884, 7095,   64, 8012, 7223
      };
      short int subbuffer103[100] = {
         64, 8518, 8254,   64, 7500, 9420, 9286, 9292,   64, 7618,
       8386,   64, 6711, 7607,    0,  -16, 7116, 6988, 9290, 9418,
       9548, 9414, 9416, 9290, 9163, 8908, 8524, 8139, 7881, 7622,
       7491, 7359, 7356, 7481, 7608, 7991, 8247, 8632, 8890, 9150,
         64, 8524, 8267, 8009, 7750, 7619, 7487, 7484, 7609, 7736,
       7991,   64, 8247, 8504, 8762, 9022,   64, 8638, 9534,    0,
        -16, 7116, 6605, 7756, 6967,   64, 7884, 7095,   64, 9420,
       8631,   64, 9548, 8759,   64, 7372, 8268,   64, 9036, 9932,
         64, 7490, 9026,   64, 6583, 7479,   64, 8247, 9143,    0,
        -16, 7116, 7495, 8652, 7863,   64, 8780, 7991,   64, 8268
      };
      short int subbuffer104[100] = {
       9164,   64, 7479, 8375,    0,  -16, 7116, 7113, 9036, 8379,
       8249, 8120, 7863, 7607, 7352, 7226, 7228, 7357, 7484, 7355,
         64, 8908, 8251, 8121, 7863,   64, 8524, 9420,    0,  -16,
       7116, 6731, 7884, 7095,   64, 8012, 7223,   64, 9676, 7487,
         64, 8387, 8887,   64, 8259, 8759,   64, 7500, 8396,   64,
       9164, 9932,   64, 6711, 7607,   64, 8375, 9143,    0,  -16,
       7116, 6986, 8140, 7351,   64, 8268, 7479,   64, 7756, 8652,
         64, 6967, 8887, 9149, 8759,    0,  -16, 7116, 6606, 7756,
       6967,   64, 7756, 7863,   64, 7884, 7993,   64, 9548, 7863,
         64, 9548, 8759,   64, 9676, 8887,   64, 7372, 7884,   64
      };
      short int subbuffer105[100] = {
       9548,10060,   64, 6583, 7351,   64, 8375, 9271,    0,  -16,
       7116, 6733, 7884, 7095,   64, 7884, 8762,   64, 7881, 8759,
         64, 9548, 8759,   64, 7500, 7884,   64, 9164, 9932,   64,
       6711, 7479,    0,  -16, 7116, 6859, 8396, 8011, 7753, 7494,
       7363, 7231, 7228, 7353, 7480, 7735, 8119, 8504, 8762, 9021,
       9152, 9284, 9287, 9162, 9035, 8780, 8396,   64, 8396, 8139,
       7881, 7622, 7491, 7359, 7356, 7481, 7735,   64, 8119, 8376,
       8634, 8893, 9024, 9156, 9159, 9034, 8780,    0,  -16, 7116,
       6731, 7884, 7095,   64, 8012, 7223,   64, 7500, 9036, 9419,
       9545, 9543, 9412, 9154, 8641, 7617,   64, 9036, 9291, 9417
      };
      short int subbuffer106[100] = {
       9415, 9284, 9026, 8641,   64, 6711, 7607,    0,  -16, 7116,
       6859, 8396, 8011, 7753, 7494, 7363, 7231, 7228, 7353, 7480,
       7735, 8119, 8504, 8762, 9021, 9152, 9284, 9287, 9162, 9035,
       8780, 8396,   64, 8396, 8139, 7881, 7622, 7491, 7359, 7356,
       7481, 7735,   64, 8119, 8376, 8634, 8893, 9024, 9156, 9159,
       9034, 8780,   64, 7481, 7482, 7612, 7869, 7997, 8252, 8378,
       8371, 8498, 8754, 8884, 8885,   64, 8378, 8500, 8627, 8755,
       8884,    0,  -16, 7116, 6732, 7884, 7095,   64, 8012, 7223,
         64, 7500, 8908, 9291, 9417, 9415, 9284, 9155, 8770, 7618,
         64, 8908, 9163, 9289, 9287, 9156, 9027, 8770,   64, 8258
      };
      short int subbuffer107[100] = {
       8513, 8640, 8760, 8887, 9143, 9273, 9274,   64, 8640, 8889,
       9016, 9144, 9273,   64, 6711, 7607,    0,  -16, 7116, 6860,
       9290, 9418, 9548, 9414, 9416, 9290, 9163, 8780, 8268, 7883,
       7625, 7623, 7749, 7876, 8768, 9022,   64, 7623, 7877, 8769,
       8896, 9022, 9019, 8889, 8760, 8375, 7863, 7480, 7353, 7227,
       7229, 7095, 7225, 7353,    0,  -16, 7116, 6987, 8652, 7863,
         64, 8780, 7991,   64, 7884, 7494, 7756, 9676, 9542, 9548,
         64, 7479, 8375,    0,  -16, 7116, 6733, 7756, 7361, 7229,
       7226, 7352, 7735, 8247, 8632, 8890, 9021, 9548,   64, 7884,
       7489, 7357, 7354, 7480, 7735,   64, 7372, 8268,   64, 9164
      };
      short int subbuffer108[100] = {
       9932,    0,  -16, 7116, 6986, 7756, 7863,   64, 7884, 7993,
         64, 9548, 7863,   64, 7500, 8268,   64, 9036, 9804,    0,
        -16, 7116, 6605, 7628, 7351,   64, 7756, 7481,   64, 8652,
       7351,   64, 8652, 8375,   64, 8780, 8505,   64, 9676, 8375,
         64, 7244, 8140,   64, 9292,10060,    0,  -16, 7116, 6859,
       7756, 8631,   64, 7884, 8759,   64, 9548, 6967,   64, 7500,
       8268,   64, 9036, 9804,   64, 6711, 7479,   64, 8247, 9015,
          0,  -16, 7116, 6987, 7756, 8258, 7863,   64, 7884, 8386,
       7991,   64, 9676, 8386,   64, 7500, 8268,   64, 9164, 9932,
         64, 7479, 8375,    0,  -16, 7116, 6859, 9420, 6967,   64
      };
      short int subbuffer109[100] = {
       9548, 7095,   64, 7884, 7494, 7756, 9548,   64, 6967, 8759,
       9021, 8631,    0,  -16, 7116, 6986, 7369, 7495, 9019, 9145,
       9143,   64, 7494, 9018,   64, 7369, 7367, 7493, 9017, 9143,
         64, 8002, 7486, 7356, 7354, 7480, 7351,   64, 7356, 7608,
         64, 7486, 7484, 7610, 7608, 7351,   64, 8383, 9028,   64,
       8777, 8774, 8900, 9156, 9158, 8903, 8777,   64, 8777, 8902,
       9156,    0,  -16, 7116, 7115, 7747, 7746, 7618, 7619, 7748,
       8005, 8517, 8772, 8899, 9025, 9018, 9144, 9271,   64, 8899,
       8890, 9016, 9271, 9399,   64, 8897, 8768, 7999, 7614, 7484,
       7482, 7608, 7991, 8375, 8632, 8890,   64, 7999, 7742, 7612
      };
      short int subbuffer110[100] = {
       7610, 7736, 7991,    0,  -16, 7116, 6858, 7500, 7479,   64,
       7628, 7607,   64, 7618, 7876, 8133, 8389, 8772, 9026, 9151,
       9149, 9018, 8760, 8375, 8119, 7864, 7610,   64, 8389, 8644,
       8898, 9023, 9021, 8890, 8632, 8375,   64, 7116, 7628,    0,
        -16, 7116, 6985, 8898, 8769, 8896, 9025, 9026, 8772, 8517,
       8133, 7748, 7490, 7359, 7357, 7482, 7736, 8119, 8375, 8760,
       9018,   64, 8133, 7876, 7618, 7487, 7485, 7610, 7864, 8119,
          0,  -16, 7116, 6987, 8908, 8887,   64, 9036, 9015,   64,
       8898, 8644, 8389, 8133, 7748, 7490, 7359, 7357, 7482, 7736,
       8119, 8375, 8632, 8890,   64, 8133, 7876, 7618, 7487, 7485
      };
      short int subbuffer111[100] = {
       7610, 7864, 8119,   64, 8524, 9036,   64, 8887, 9399,    0,
        -16, 7116, 6985, 7487, 9023, 9025, 8899, 8772, 8517, 8133,
       7748, 7490, 7359, 7357, 7482, 7736, 8119, 8375, 8760, 9018,
         64, 8895, 8898, 8772,   64, 8133, 7876, 7618, 7487, 7485,
       7610, 7864, 8119,    0,  -16, 7116, 7366, 8651, 8522, 8649,
       8778, 8779, 8652, 8396, 8139, 8009, 7991,   64, 8396, 8267,
       8137, 8119,   64, 7621, 8645,   64, 7607, 8503,    0,  -16,
       7116, 7114, 8133, 7876, 7747, 7617, 7615, 7741, 7868, 8123,
       8379, 8636, 8765, 8895, 8897, 8771, 8644, 8389, 8133,   64,
       7876, 7746, 7742, 7868,   64, 8636, 8766, 8770, 8644,   64
      };
      short int subbuffer112[100] = {
       8771, 8900, 9157, 9156, 8900,   64, 7741, 7612, 7482, 7481,
       7607, 7990, 8630, 9013, 9140,   64, 7481, 7608, 7991, 8631,
       9014, 9140, 9139, 9009, 8624, 7856, 7473, 7347, 7348, 7478,
       7863,    0,  -16, 7116, 6859, 7500, 7479,   64, 7628, 7607,
         64, 7618, 7876, 8261, 8517, 8900, 9026, 9015,   64, 8517,
       8772, 8898, 8887,   64, 7116, 7628,   64, 7095, 7991,   64,
       8503, 9399,    0,  -16, 7116, 7622, 8268, 8139, 8266, 8395,
       8268,   64, 8261, 8247,   64, 8389, 8375,   64, 7877, 8389,
         64, 7863, 8759,    0,  -16, 7116, 7622, 8396, 8267, 8394,
       8523, 8396,   64, 8517, 8499, 8369, 8112, 7856, 7729, 7730
      };
      short int subbuffer113[100] = {
       7859, 7986, 7857,   64, 8389, 8371, 8241, 8112,   64, 8005,
       8517,    0,  -16, 7116, 6858, 7500, 7479,   64, 7628, 7607,
         64, 8901, 7611,   64, 8255, 9015,   64, 8127, 8887,   64,
       7116, 7628,   64, 8517, 9285,   64, 7095, 7991,   64, 8503,
       9271,    0,  -16, 7116, 7622, 8268, 8247,   64, 8396, 8375,
         64, 7884, 8396,   64, 7863, 8759,    0,  -16, 7116, 6225,
       6853, 6839,   64, 6981, 6967,   64, 6978, 7236, 7621, 7877,
       8260, 8386, 8375,   64, 7877, 8132, 8258, 8247,   64, 8386,
       8644, 9029, 9285, 9668, 9794, 9783,   64, 9285, 9540, 9666,
       9655,   64, 6469, 6981,   64, 6455, 7351,   64, 7863, 8759
      };
      short int subbuffer114[100] = {
         64, 9271,10167,    0,  -16, 7116, 6859, 7493, 7479,   64,
       7621, 7607,   64, 7618, 7876, 8261, 8517, 8900, 9026, 9015,
         64, 8517, 8772, 8898, 8887,   64, 7109, 7621,   64, 7095,
       7991,   64, 8503, 9399,    0,  -16, 7116, 6986, 8133, 7748,
       7490, 7359, 7357, 7482, 7736, 8119, 8375, 8760, 9018, 9149,
       9151, 9026, 8772, 8389, 8133,   64, 8133, 7876, 7618, 7487,
       7485, 7610, 7864, 8119,   64, 8375, 8632, 8890, 9021, 9023,
       8898, 8644, 8389,    0,  -16, 7116, 6858, 7493, 7472,   64,
       7621, 7600,   64, 7618, 7876, 8133, 8389, 8772, 9026, 9151,
       9149, 9018, 8760, 8375, 8119, 7864, 7610,   64, 8389, 8644
      };
      short int subbuffer115[100] = {
       8898, 9023, 9021, 8890, 8632, 8375,   64, 7109, 7621,   64,
       7088, 7984,    0,  -16, 7116, 6986, 8901, 8880,   64, 9029,
       9008,   64, 8898, 8644, 8389, 8133, 7748, 7490, 7359, 7357,
       7482, 7736, 8119, 8375, 8632, 8890,   64, 8133, 7876, 7618,
       7487, 7485, 7610, 7864, 8119,   64, 8496, 9392,    0,  -16,
       7116, 7112, 7749, 7735,   64, 7877, 7863,   64, 7871, 8002,
       8260, 8517, 8901, 9028, 9027, 8898, 8771, 8900,   64, 7365,
       7877,   64, 7351, 8247,    0,  -16, 7116, 7241, 8899, 9029,
       9025, 8899, 8772, 8517, 8005, 7748, 7619, 7617, 7744, 7999,
       8637, 8892, 9019,   64, 7618, 7745, 8000, 8638, 8893, 9020
      };
      short int subbuffer116[100] = {
       9017, 8888, 8631, 8119, 7864, 7737, 7611, 7607, 7737,    0,
        -16, 7116, 7368, 8012, 7995, 8120, 8375, 8631, 8888, 9018,
         64, 8140, 8123, 8248, 8375,   64, 7621, 8645,    0,  -16,
       7116, 6859, 7493, 7482, 7608, 7991, 8247, 8632, 8890,   64,
       7621, 7610, 7736, 7991,   64, 8901, 8887,   64, 9029, 9015,
         64, 7109, 7621,   64, 8517, 9029,   64, 8887, 9399,    0,
        -16, 7116, 7113, 7493, 8247,   64, 7621, 8249,   64, 9029,
       8247,   64, 7237, 8005,   64, 8517, 9285,    0,  -16, 7116,
       6732, 7237, 7735,   64, 7365, 7738,   64, 8261, 7735,   64,
       8261, 8759,   64, 8389, 8762,   64, 9285, 8759,   64, 6853
      };
      short int subbuffer117[100] = {
       7749,   64, 8901, 9669,    0,  -16, 7116, 6986, 7493, 8887,
         64, 7621, 9015,   64, 9029, 7479,   64, 7237, 8005,   64,
       8517, 9285,   64, 7223, 7991,   64, 8503, 9271,    0,  -16,
       7116, 6985, 7493, 8247,   64, 7621, 8249,   64, 9029, 8247,
       7987, 7729, 7472, 7344, 7217, 7346, 7473,   64, 7237, 8005,
         64, 8517, 9285,    0,  -16, 7116, 7113, 8901, 7479,   64,
       9029, 7607,   64, 7621, 7489, 7493, 9029,   64, 7479, 9015,
       9019, 8887,    0,  -16, 7116, 6860, 8133, 7748, 7490, 7360,
       7229, 7226, 7352, 7735, 7991, 8248, 8635, 8894, 9154, 9285,
         64, 8133, 7876, 7618, 7488, 7357, 7354, 7480, 7735,   64
      };
      short int subbuffer118[100] = {
       8133, 8389, 8644, 8770, 9018, 9144, 9271,   64, 8389, 8516,
       8642, 8890, 9016, 9271, 9399,    0,  -16, 7116, 6858, 8524,
       8139, 7881, 7621, 7490, 7358, 7224, 7088,   64, 8524, 8267,
       8009, 7749, 7618, 7486, 7352, 7216,   64, 8524, 8780, 9035,
       9162, 9159, 9029, 8900, 8515, 8003,   64, 8780, 9034, 9031,
       8901, 8772, 8515,   64, 8003, 8514, 8768, 8894, 8891, 8761,
       8632, 8247, 7991, 7736, 7609, 7484,   64, 8003, 8386, 8640,
       8766, 8763, 8633, 8504, 8247,    0,  -16, 7116, 6986, 7106,
       7364, 7621, 7877, 8132, 8259, 8384, 8380, 8248, 7856,   64,
       7235, 7492, 8004, 8259,   64, 9285, 9154, 9024, 8377, 7988
      };
      short int subbuffer119[100] = {
       7728,   64, 9157, 9026, 8896, 8377,    0,  -16, 7116, 7114,
       8772, 8517, 8261, 7876, 7617, 7486, 7483, 7609, 7736, 7991,
       8247, 8632, 8891, 9022, 9025, 8899, 8392, 8266, 8268, 8397,
       8653, 8908, 9162,   64, 8261, 8004, 7745, 7614, 7610, 7736,
         64, 8247, 8504, 8763, 8894, 8898, 8772, 8519, 8393, 8395,
       8524, 8780, 9162,    0,  -16, 7116, 7113, 9026, 8772, 8517,
       8005, 7748, 7746, 8000, 8383,   64, 8005, 7876, 7874, 8128,
       8383,   64, 8383, 7742, 7484, 7482, 7608, 7991, 8375, 8632,
       8890,   64, 8383, 7870, 7612, 7610, 7736, 7991,    0,  -16,
       7116, 7113, 8524, 8267, 8138, 8137, 8264, 8647, 9287, 9288
      };
      short int subbuffer120[100] = {
       8903, 8389, 8003, 7616, 7485, 7483, 7609, 7991, 8373, 8499,
       8497, 8368, 8112, 7985,   64, 8646, 8131, 7744, 7613, 7611,
       7737, 7991,    0,  -16, 7116, 6859, 6977, 7107, 7365, 7749,
       7876, 7874, 7742, 7479,   64, 7621, 7748, 7746, 7614, 7351,
         64, 7742, 8002, 8260, 8517, 8773, 9028, 9155, 9152, 9019,
       8624,   64, 8773, 9027, 9024, 8891, 8496,    0,  -16, 7116,
       6731, 6849, 6979, 7237, 7621, 7748, 7746, 7613, 7610, 7736,
       7863,   64, 7493, 7620, 7618, 7485, 7482, 7608, 7863, 8119,
       8376, 8634, 8893, 9024, 9157, 9161, 9035, 8780, 8524, 8266,
       8264, 8389, 8642, 8896, 9278,   64, 8376, 8635, 8765, 8896
      };
      short int subbuffer121[100] = {
       9029, 9033, 8907, 8780,    0,  -16, 7116, 7494, 8261, 7998,
       7866, 7864, 7991, 8375, 8633, 8763,   64, 8389, 8126, 7994,
       7992, 8119,    0,  -16, 7116, 6986, 7749, 7223,   64, 7877,
       7351,   64, 9029, 9156, 9284, 9157, 8901, 8644, 8128, 7871,
       7615,   64, 7871, 8126, 8376, 8503,   64, 7871, 7998, 8248,
       8375, 8631, 8888, 9147,    0,  -16, 7116, 6986, 7372, 7628,
       7883, 8010, 8136, 8890, 9016, 9143,   64, 7628, 7882, 8008,
       8762, 8888, 9143, 9271,   64, 8261, 7223,   64, 8261, 7351,
          0,  -16, 7116, 6731, 7621, 6832,   64, 7749, 6960,   64,
       7618, 7484, 7481, 7735, 7991, 8248, 8506, 8765,   64, 9029
      };
      short int subbuffer122[100] = {
       8634, 8632, 8759, 9143, 9401, 9531,   64, 9157, 8762, 8760,
       8887,    0,  -16, 7116, 6986, 7749, 7479,   64, 7877, 7743,
       7610, 7479,   64, 9157, 9025, 8765,   64, 9285, 9154, 9024,
       8765, 8507, 8121, 7864, 7479,   64, 7365, 7877,    0,  -16,
       7116, 7112, 8524, 8267, 8138, 8137, 8264, 8647, 9031,   64,
       8647, 8134, 7877, 7747, 7745, 7999, 8382, 8766,   64, 8647,
       8262, 8005, 7875, 7873, 8127, 8382,   64, 8382, 7869, 7612,
       7482, 7480, 7734, 8372, 8499, 8497, 8240, 7984,   64, 8382,
       7997, 7740, 7610, 7608, 7862, 8372,    0,  -16, 7116, 7113,
       8261, 7876, 7617, 7486, 7483, 7609, 7736, 7991, 8247, 8632
      };
