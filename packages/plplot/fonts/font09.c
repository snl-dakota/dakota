/* $Id: font09.c 3186 2006-02-15 18:17:33Z slbrow $
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

   Revision 1.1  1992/05/20 21:33:30  furnish
   Initial checkin of the whole PLPLOT project.

*/

      short int subbuffer200[100] = {
       8634, 8248, 7863,    0,  -16, 7116, 6732, 7494, 7367, 7369,
       7499, 7884, 8268, 7873, 7611, 7481, 7352, 7095, 6839, 6712,
       6714, 6843, 6970, 6841,   64, 8268, 7875, 7744, 7483, 7353,
       7095,   64, 7230, 7359, 7616, 8771, 9028, 9414, 9672, 9802,
       9803, 9676, 9548, 9291, 9032, 8902, 8640, 8508, 8505, 8759,
       8887, 9144, 9402,   64, 9548, 9290, 9030, 8768, 8636, 8633,
       8759,    0,  -16, 7116, 7111, 8906, 8647, 8386, 8125, 7995,
       7736, 7479,   64, 9158, 8900, 8515, 8131, 7876, 7750, 7752,
       7882, 8139, 8652, 9164, 8906, 8776, 8514, 8252, 8122, 7864,
       7479, 7223, 7096, 7098, 7227, 7354, 7225,    0,  -16, 7116
      };
      short int subbuffer201[100] = {
       7112, 9164, 8906, 8647, 8386, 7993, 7733,   64, 9157, 8899,
       8514, 8130, 7875, 7749, 7751, 7881, 8139, 8652, 9164, 8905,
       8775, 8382, 8122, 7992, 7733, 7604, 7347, 7220, 7222, 7352,
       7610, 7867, 8252, 8765,    0,  -16, 7116, 6732, 7494, 7367,
       7369, 7627, 8012, 8268, 7873, 7611, 7481, 7352, 7095, 6839,
       6712, 6714, 6843, 6970, 6841,   64, 8268, 7875, 7744, 7483,
       7353, 7095,   64, 9291, 8903, 8645, 8388, 8003,   64, 9675,
       9546, 9673, 9802, 9803, 9676, 9548, 9291, 8902, 8773, 8516,
       8003,   64, 8003, 8386, 8512, 8633, 8759,   64, 8003, 8258,
       8384, 8505, 8759, 8887, 9144, 9402,    0,  -16, 7116, 7113
      };
      short int subbuffer202[100] = {
       7625, 7495, 7493, 7619, 7874, 8258, 8643, 8900, 9287, 9418,
       9419, 9292, 9164, 8907, 8778, 8519, 7997, 7867, 7608, 7351,
         64, 8778, 8518, 8255, 8124, 7994, 7736, 7351, 7095, 6968,
       6970, 7099, 7355, 7610, 7992, 8247, 8631, 8888, 9146,    0,
        -16, 7116, 6478, 8268, 7747, 7357, 7098, 6840, 6583, 6327,
       6200, 6202, 6331, 6458, 6329,   64, 8268, 8005, 7873, 7740,
       7736, 7991,   64, 8268, 8136, 8003, 7868, 7864, 7991,   64,
       9420, 8899, 8250, 7991,   64, 9420, 9157, 9025, 8892, 8888,
       9143, 9271, 9528, 9786,   64, 9420, 9288, 9155, 9020, 9016,
       9143,    0,  -16, 7116, 6860, 8268, 8136, 7874, 7613, 7483
      };
      short int subbuffer203[100] = {
       7224, 6967, 6711, 6584, 6586, 6715, 6842, 6713,   64, 8268,
       8263, 8380, 8503,   64, 8268, 8391, 8508, 8503,   64,10059,
       9930,10057,10186,10187,10060, 9804, 9547, 9288, 9158, 8897,
       8635, 8503,    0,  -16, 7116, 6987, 8396, 8139, 7881, 7622,
       7492, 7360, 7356, 7481, 7608, 7863, 8119, 8504, 8762, 9021,
       9151, 9283, 9287, 9162, 9035, 8907, 8650, 8392, 8132, 7999,
       7996,   64, 8139, 7880, 7620, 7488, 7484, 7609, 7863,    0,
        -16, 7116, 6731, 8651, 8522, 8392, 8131, 7869, 7739, 7480,
       7223,   64, 8522, 8391, 8127, 7996, 7866, 7608, 7223, 6967,
       6840, 6842, 6971, 7098, 6969,   64, 7878, 7748, 7619, 7363
      };
      short int subbuffer204[100] = {
       7236, 7238, 7368, 7626, 7883, 8268, 8780, 9163, 9290, 9416,
       9413, 9283, 9154, 8769, 8513, 8258,   64, 8780, 9035, 9162,
       9288, 9285, 9155, 9026, 8769,    0,  -16, 7116, 6987, 8648,
       8646, 8516, 8387, 8130, 7874, 7748, 7750, 7881, 8139, 8524,
       8908, 9163, 9289, 9285, 9154, 8895, 8379, 7993, 7736, 7351,
       7095, 6968, 6970, 7099, 7355, 7610, 7992, 8375, 8759, 9016,
       9274,   64, 8908, 9035, 9161, 9157, 9026, 8767, 8380, 7865,
       7351,    0,  -16, 7116, 6732, 8651, 8522, 8392, 8131, 7869,
       7739, 7480, 7223,   64, 8522, 8391, 8127, 7996, 7866, 7608,
       7223, 6967, 6840, 6842, 6971, 7098, 6969,   64, 7878, 7748
      };
      short int subbuffer205[100] = {
       7619, 7363, 7236, 7238, 7368, 7626, 7883, 8268, 8908, 9291,
       9417, 9415, 9285, 9156, 8771, 8259,   64, 8908, 9163, 9289,
       9287, 9157, 9028, 8771,   64, 8259, 8642, 8768, 8889, 9015,
         64, 8259, 8514, 8640, 8761, 9015, 9143, 9400, 9658,    0,
        -16, 7116, 6986, 7753, 7623, 7621, 7747, 8002, 8386, 8771,
       9028, 9415, 9546, 9547, 9420, 9292, 9035, 8906, 8776, 8645,
       8382, 8251, 7992, 7735,   64, 8776, 8644, 8509, 8378, 8120,
       7735, 7351, 7096, 6970, 6971, 7100, 7227, 7098,    0,  -16,
       7116, 7113, 9162, 9032, 8771, 8509, 8379, 8120, 7863,   64,
       8390, 8260, 8003, 7747, 7621, 7623, 7753, 8011, 8396, 9548
      };
      short int subbuffer206[100] = {
       9291, 9162, 9031, 8767, 8636, 8506, 8248, 7863, 7607, 7352,
       7225, 7226, 7355, 7482, 7353,   64, 8652, 9163, 9291,    0,
        -16, 7116, 6859, 6984, 7243, 7500, 7628, 7882, 7879, 7748,
       7356, 7353, 7479,   64, 7628, 7754, 7751, 7359, 7228, 7225,
       7479, 7735, 7992, 8379, 8638, 8768,   64, 9292, 8768, 8636,
       8633, 8887, 9015, 9272, 9530,   64, 9420, 8896, 8764, 8761,
       8887,    0,  -16, 7116, 6858, 6984, 7243, 7500, 7628, 7882,
       7879, 7747, 7484, 7481, 7607,   64, 7628, 7754, 7751, 7488,
       7356, 7353, 7607, 7735, 8120, 8507, 8766, 9026, 9157, 9289,
       9291, 9164, 9036, 8907, 8777, 8774, 8900, 9154, 9409, 9665
      };
      short int subbuffer207[100] = {
          0,  -16, 7116, 6731, 7110, 6982, 6855, 6857, 6987, 7244,
       7756, 7626, 7494, 7357, 7223,   64, 7494, 7485, 7351,   64,
       8780, 8522, 8262, 7869, 7609, 7351,   64, 8780, 8650, 8518,
       8381, 8247,   64, 8518, 8509, 8375,   64,10060, 9803, 9545,
       9286, 8893, 8633, 8375,    0,  -16, 7116, 6986, 8007, 7878,
       7622, 7495, 7497, 7627, 7884, 8140, 8395, 8521, 8518, 8386,
       8125, 7866, 7608, 7223, 6967, 6840, 6842, 6971, 7098, 6969,
         64, 8140, 8267, 8393, 8390, 8258, 7997, 7738, 7480, 7223,
         64, 9675, 9546, 9673, 9802, 9803, 9676, 9420, 9163, 8905,
       8646, 8386, 8253, 8250, 8376, 8503, 8631, 8888, 9146,    0
      };
      short int subbuffer208[100] = {
        -16, 7116, 6859, 7240, 7499, 7756, 7884, 8139, 8137, 7875,
       7872, 7998,   64, 7884, 8011, 8009, 7747, 7744, 7998, 8254,
       8639, 8897, 9156, 9286,   64, 9548, 9286, 8894, 8634,   64,
       9676, 9414, 9153, 8893, 8634, 8376, 7991, 7479, 7224, 7098,
       7099, 7228, 7355, 7226,    0,  -16, 7116, 6858, 9290, 9160,
       8899, 8768, 8638, 8379, 8121, 7864, 7479,   64, 8390, 8260,
       8003, 7747, 7621, 7623, 7753, 8011, 8396, 9676, 9419, 9290,
       9159, 9027, 8765, 8506, 8120, 7479, 6967, 6840, 6842, 6971,
       7227, 7482, 7864, 8119, 8503, 8888, 9146,   64, 8780, 9291,
       9419,    0,  -16, 7116, 6986, 8901, 8887, 9015,   64, 8901
      };
      short int subbuffer209[100] = {
       9029, 9015,   64, 8898, 8644, 8389, 8005, 7748, 7490, 7359,
       7357, 7482, 7736, 7991, 8375, 8632, 8890,   64, 8898, 8388,
       8004, 7747, 7618, 7487, 7485, 7610, 7737, 7992, 8376, 8890,
          0,  -16, 7116, 6986, 7500, 7479, 7607,   64, 7500, 7628,
       7607,   64, 7618, 7876, 8133, 8517, 8772, 9026, 9151, 9149,
       9018, 8760, 8503, 8119, 7864, 7610,   64, 7618, 8132, 8516,
       8771, 8898, 9023, 9021, 8890, 8761, 8504, 8120, 7610,    0,
        -16, 7116, 7113, 9026, 8772, 8517, 8133, 7876, 7618, 7487,
       7485, 7610, 7864, 8119, 8503, 8760, 9018,   64, 9026, 8897,
       8771, 8516, 8132, 7875, 7746, 7615, 7613, 7738, 7865, 8120
      };
      short int subbuffer210[100] = {
       8504, 8761, 8891, 9018,    0,  -16, 7116, 6986, 8908, 8887,
       9015,   64, 8908, 9036, 9015,   64, 8898, 8644, 8389, 8005,
       7748, 7490, 7359, 7357, 7482, 7736, 7991, 8375, 8632, 8890,
         64, 8898, 8388, 8004, 7747, 7618, 7487, 7485, 7610, 7737,
       7992, 8376, 8890,    0,  -16, 7116, 7113, 7614, 9022, 9025,
       8899, 8772, 8517, 8133, 7876, 7618, 7487, 7485, 7610, 7864,
       8119, 8503, 8760, 9018,   64, 7615, 8895, 8897, 8771, 8516,
       8132, 7875, 7746, 7615, 7613, 7738, 7865, 8120, 8504, 8761,
       8891, 9018,    0,  -16, 7116, 7496, 8908, 8652, 8395, 8264,
       8247, 8375,   64, 8908, 8907, 8651, 8394,   64, 8523, 8392
      };
      short int subbuffer211[100] = {
       8375,   64, 7877, 8773, 8772,   64, 7877, 7876, 8772,    0,
        -16, 7116, 6986, 9029, 8901, 8886, 8755, 8626, 8369, 8113,
       7858, 7731, 7475,   64, 9029, 9014, 8883, 8625, 8368, 7984,
       7729, 7475,   64, 8898, 8644, 8389, 8005, 7748, 7490, 7359,
       7357, 7482, 7736, 7991, 8375, 8632, 8890,   64, 8898, 8388,
       8004, 7747, 7618, 7487, 7485, 7610, 7737, 7992, 8376, 8890,
          0,  -16, 7116, 6986, 7500, 7479, 7607,   64, 7500, 7628,
       7607,   64, 7617, 8004, 8261, 8645, 8900, 9025, 9015,   64,
       7617, 8003, 8260, 8516, 8771, 8897, 8887, 9015,    0,  -16,
       7116, 7749, 8268, 8139, 8138, 8265, 8393, 8522, 8523, 8396
      };
      short int subbuffer212[100] = {
       8268,   64, 8267, 8266, 8394, 8395, 8267,   64, 8261, 8247,
       8375,   64, 8261, 8389, 8375,    0,  -16, 7116, 7749, 8268,
       8139, 8138, 8265, 8393, 8522, 8523, 8396, 8268,   64, 8267,
       8266, 8394, 8395, 8267,   64, 8261, 8240, 8368,   64, 8261,
       8389, 8368,    0,  -16, 7116, 6985, 7500, 7479, 7607,   64,
       7500, 7628, 7607,   64, 9029, 8901, 7611,   64, 9029, 7610,
         64, 7998, 8759, 9015,   64, 8127, 9015,    0,  -16, 7116,
       7749, 8268, 8247, 8375,   64, 8268, 8396, 8375,    0,  -16,
       7116, 6352, 6853, 6839, 6967,   64, 6853, 6981, 6967,   64,
       6977, 7364, 7621, 8005, 8260, 8385, 8375,   64, 6977, 7363
      };
      short int subbuffer213[100] = {
       7620, 7876, 8131, 8257, 8247, 8375,   64, 8385, 8772, 9029,
       9413, 9668, 9793, 9783,   64, 8385, 8771, 9028, 9284, 9539,
       9665, 9655, 9783,    0,  -16, 7116, 6986, 7493, 7479, 7607,
         64, 7493, 7621, 7607,   64, 7617, 8004, 8261, 8645, 8900,
       9025, 9015,   64, 7617, 8003, 8260, 8516, 8771, 8897, 8887,
       9015,    0,  -16, 7116, 7114, 8133, 7876, 7618, 7487, 7485,
       7610, 7864, 8119, 8503, 8760, 9018, 9149, 9151, 9026, 8772,
       8517, 8133,   64, 8132, 7875, 7746, 7615, 7613, 7738, 7865,
       8120, 8504, 8761, 8890, 9021, 9023, 8898, 8771, 8516, 8132,
          0,  -16, 7116, 6986, 7493, 7472, 7600,   64, 7493, 7621
      };
      short int subbuffer214[100] = {
       7600,   64, 7618, 7876, 8133, 8517, 8772, 9026, 9151, 9149,
       9018, 8760, 8503, 8119, 7864, 7610,   64, 7618, 8132, 8516,
       8771, 8898, 9023, 9021, 8890, 8761, 8504, 8120, 7610,    0,
        -16, 7116, 6986, 8901, 8880, 9008,   64, 8901, 9029, 9008,
         64, 8898, 8644, 8389, 8005, 7748, 7490, 7359, 7357, 7482,
       7736, 7991, 8375, 8632, 8890,   64, 8898, 8388, 8004, 7747,
       7618, 7487, 7485, 7610, 7737, 7992, 8376, 8890,    0,  -16,
       7116, 7367, 7877, 7863, 7991,   64, 7877, 8005, 7991,   64,
       7999, 8130, 8388, 8645, 9029,   64, 7999, 8129, 8387, 8644,
       9028, 9029,    0,  -16, 7116, 7241, 9026, 8900, 8517, 8133
      };
      short int subbuffer215[100] = {
       7748, 7618, 7744, 7999, 8637, 8892,   64, 8765, 8891, 8890,
       8760,   64, 8889, 8504, 8120, 7737,   64, 7864, 7738, 7610,
         64, 9026, 8898, 8772,   64, 8899, 8516, 8132, 7747,   64,
       7876, 7746, 7872,   64, 7745, 8000, 8638, 8893, 9019, 9018,
       8888, 8503, 8119, 7736, 7610,    0,  -16, 7116, 7622, 8268,
       8247, 8375,   64, 8268, 8396, 8375,   64, 7877, 8773, 8772,
         64, 7877, 7876, 8772,    0,  -16, 7116, 6986, 7493, 7483,
       7608, 7863, 8247, 8504, 8891,   64, 7493, 7621, 7611, 7737,
       7992, 8248, 8505, 8891,   64, 8901, 8887, 9015,   64, 8901,
       9029, 9015,    0,  -16, 7116, 7240, 7493, 8247,   64, 7493
      };
      short int subbuffer216[100] = {
       7621, 8249,   64, 9029, 8901, 8249,   64, 9029, 8247,    0,
        -16, 7116, 6732, 7109, 7735,   64, 7109, 7237, 7738,   64,
       8261, 7738,   64, 8258, 7735,   64, 8258, 8759,   64, 8261,
       8762,   64, 9413, 9285, 8762,   64, 9413, 8759,    0,  -16,
       7116, 7113, 7493, 8887, 9015,   64, 7493, 7621, 9015,   64,
       9029, 8901, 7479,   64, 9029, 7607, 7479,    0,  -16, 7116,
       7240, 7493, 8247,   64, 7493, 7621, 8249,   64, 9029, 8901,
       8249, 7728,   64, 9029, 8247, 7856, 7728,    0,  -16, 7116,
       7113, 8772, 7479,   64, 9029, 7736,   64, 7493, 9029,   64,
       7493, 7492, 8772,   64, 7736, 9016, 9015,   64, 7479, 9015
      };
      short int subbuffer217[100] = {
          0,  -16, 7116, 7369, 8637, 8511, 8256, 8000, 7743, 7614,
       7484, 7482, 7608, 7863, 8119, 8376, 8506,   64, 8000, 7742,
       7612, 7609, 7863,   64, 8768, 8506, 8504, 8759, 9016, 9145,
       9404,   64, 8896, 8634, 8632, 8759,    0,  -16, 7116, 7496,
       7484, 7743, 8003,   64, 8396, 7610, 7608, 7863, 7991, 8248,
       8506, 8637, 8640, 8764, 8891, 9019, 9276,   64, 8524, 7738,
       7736, 7863,    0,  -16, 7116, 7494, 8511, 8382, 8510, 8511,
       8384, 8128, 7871, 7742, 7612, 7610, 7736, 7991, 8375, 8761,
       9020,   64, 8128, 7870, 7740, 7737, 7991,    0,  -16, 7116,
       7369, 8637, 8511, 8256, 8000, 7743, 7614, 7484, 7482, 7608
      };
      short int subbuffer218[100] = {
       7863, 8119, 8376, 8506,   64, 8000, 7742, 7612, 7609, 7863,
         64, 9292, 8506, 8504, 8759, 9016, 9145, 9404,   64, 9420,
       8634, 8632, 8759,    0,  -16, 7116, 7494, 7865, 8122, 8251,
       8381, 8383, 8256, 8128, 7871, 7742, 7612, 7610, 7736, 7991,
       8375, 8761, 9020,   64, 8128, 7870, 7740, 7737, 7991,    0,
        -16, 7116, 7878, 8256, 8643, 8902, 9033, 9035, 8908, 8651,
       8521, 7342, 7340, 7467, 7724, 7855, 7992, 8119, 8375, 8632,
       8761, 9020,   64, 8521, 8388, 8256, 7863, 7602, 7342,    0,
        -16, 7116, 7369, 8637, 8511, 8256, 8000, 7743, 7614, 7484,
       7482, 7608, 7863, 8119, 8376, 8506,   64, 8000, 7742, 7612
      };
      short int subbuffer219[100] = {
       7609, 7863,   64, 8768, 7982,   64, 8896, 8503, 8242, 7982,
       7852, 7595, 7468, 7470, 7601, 7859, 8245, 8759, 9145, 9404,
          0,  -16, 7116, 7497, 7484, 7743, 8003,   64, 8396, 7479,
         64, 8524, 7607,   64, 7869, 8127, 8384, 8512, 8767, 8765,
       8634, 8632, 8759,   64, 8512, 8639, 8637, 8506, 8504, 8759,
       9016, 9145, 9404,    0,  -16, 7116, 7748, 8390, 8261, 8388,
       8517, 8390,   64, 8128, 7866, 7864, 8119, 8376, 8505, 8764,
         64, 8256, 7994, 7992, 8119,    0,  -16, 7116, 7748, 8390,
       8261, 8388, 8517, 8390,   64, 8128, 7342,   64, 8256, 7863,
       7602, 7342, 7212, 6955, 6828, 6830, 6961, 7219, 7605, 8119
      };
      short int subbuffer220[100] = {
       8505, 8764,    0,  -16, 7116, 7496, 7484, 7743, 8003,   64,
       8396, 7479,   64, 8524, 7607,   64, 8640, 8639, 8767, 8640,
       8512, 8254, 7869,   64, 7869, 8252, 8376, 8503,   64, 7869,
       8124, 8248, 8503, 8631, 9017, 9276,    0,  -16, 7116, 7748,
       7740, 7999, 8259,   64, 8652, 7866, 7864, 8119, 8376, 8505,
       8764,   64, 8780, 7994, 7992, 8119,    0,  -16, 7116, 6604,
       6588, 6847, 7104, 7359, 7357, 7095,   64, 7104, 7231, 7229,
       6967,   64, 7357, 7615, 7872, 8000, 8255, 8253, 7991,   64,
       8000, 8127, 8125, 7863,   64, 8253, 8511, 8768, 8896, 9151,
       9149, 9018, 9016, 9143,   64, 8896, 9023, 9021, 8890, 8888
      };
      short int subbuffer221[100] = {
       9143, 9400, 9529, 9788,    0,  -16, 7116, 7113, 7100, 7359,
       7616, 7871, 7869, 7607,   64, 7616, 7743, 7741, 7479,   64,
       7869, 8127, 8384, 8512, 8767, 8765, 8634, 8632, 8759,   64,
       8512, 8639, 8637, 8506, 8504, 8759, 9016, 9145, 9404,    0,
        -16, 7116, 7367, 8256, 8000, 7743, 7614, 7484, 7482, 7608,
       7863, 8119, 8376, 8505, 8635, 8637, 8511, 8256, 8127, 8125,
       8251, 8506, 8762, 9019, 9148,   64, 8000, 7742, 7612, 7609,
       7863,    0,  -16, 7116, 7497, 7484, 7743, 8003,   64, 8134,
       6955,   64, 8262, 7083,   64, 7869, 8127, 8384, 8512, 8767,
       8765, 8634, 8632, 8759,   64, 8512, 8639, 8637, 8506, 8504
      };
      short int subbuffer222[100] = {
       8759, 9016, 9145, 9404,    0,  -16, 7116, 7369, 8637, 8511,
       8256, 8000, 7743, 7614, 7484, 7482, 7608, 7863, 8119, 8376,
         64, 8000, 7742, 7612, 7609, 7863,   64, 8768, 7982, 7980,
       8107, 8364, 8495, 8503, 8759, 9145, 9404,   64, 8896, 8503,
       8242, 7982,    0,  -16, 7116, 7496, 7484, 7743, 8000, 8255,
       8253, 7991,   64, 8000, 8127, 8125, 7863,   64, 8253, 8511,
       8768, 8896, 8765,   64, 8768, 8765, 8891, 9019, 9276,    0,
        -16, 7116, 7752, 7740, 7999, 8129, 8127, 8509, 8635, 8633,
       8504, 8247,   64, 8127, 8381, 8507, 8505, 8247,   64, 7736,
       7991, 8631, 9017, 9276,    0,  -16, 7116, 7748, 7740, 7999
      };
      short int subbuffer223[100] = {
       8259,   64, 8652, 7866, 7864, 8119, 8376, 8505, 8764,   64,
       8780, 7994, 7992, 8119,   64, 8004, 8772,    0,  -16, 7116,
       7369, 7744, 7482, 7480, 7735, 7863, 8120, 8378, 8637,   64,
       7872, 7610, 7608, 7735,   64, 8768, 8506, 8504, 8759, 9016,
       9145, 9404,   64, 8896, 8634, 8632, 8759,    0,  -16, 7116,
       7368, 7744, 7614, 7483, 7480, 7735, 7863, 8248, 8506, 8637,
       8640,   64, 7872, 7742, 7611, 7608, 7735,   64, 8640, 8764,
       8891, 9019, 9276,    0,  -16, 7116, 6987, 7488, 7230, 7099,
       7096, 7351, 7479, 7736, 7994,   64, 7616, 7358, 7227, 7224,
       7351,   64, 8256, 7994, 7992, 8247, 8375, 8632, 8890, 9021
      };
      short int subbuffer224[100] = {
       9024,   64, 8384, 8122, 8120, 8247,   64, 9024, 9148, 9275,
       9403, 9660,    0,  -16, 7116, 7240, 7228, 7487, 7744, 8000,
       8127, 8125, 7994, 7864, 7607, 7479, 7352, 7353, 7481, 7352,
         64, 8895, 8766, 8894, 8895, 8768, 8640, 8383, 8253, 8122,
       8120, 8247, 8631, 9017, 9276,   64, 8127, 8253,   64, 8383,
       8125,   64, 7994, 8120,   64, 8122, 7864,    0,  -16, 7116,
       7369, 7744, 7482, 7480, 7735, 7863, 8120, 8378, 8637,   64,
       7872, 7610, 7608, 7735,   64, 8768, 7982,   64, 8896, 8503,
       8242, 7982, 7852, 7595, 7468, 7470, 7601, 7859, 8245, 8759,
       9145, 9404,    0,  -16, 7116, 7495, 7484, 7743, 8000, 8256
      };
