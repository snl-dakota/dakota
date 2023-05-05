:orphan:

.. _faq-main:

""""""""""""""""""""""""""
Frequently Asked Questions
""""""""""""""""""""""""""

.. _faq-general:

=======
General
=======

- **What is Dakota?**
    Dakota is a general-purpose software toolkit for performing systems analysis and design on high performance computers. Dakota provides algorithms for design optimization, uncertainty quantification, parameter estimation, design of experiments, and sensitivity analysis, as well as a range of parallel computing and simulation interfacing services.
- **How is Dakota used?**
    To use Dakota for a particular application, an interface between Dakota and your simulation code must be developed. For an overview see Section 1.3 of the User's Manual. The Interfacing section of the FAQ may also prove helpful. Once the simulation interface has been developed, switching between Dakota's many iterative methods usually requires only minimal modifications to the Dakota input file. Refer to Chapter 2 in the User's Manual for discussion of example input files.
- **Is there a graphical user interface (GUI)?**
    Yes, Dakota 6.5 and newer include an update graphical user interface.  It's included in the binary package downloads with UI in their name.
    The formerly available JAGUAR GUI was compatible with Dakota 5.4, but not with more recent Dakota releases.
- **How should I cite DAKOTA if I use it in my work?**
    Please cite the Dakota User's Manual (or other appropriate manual) for the version you used (see SAND Reports). For example:

        Adams, B.M., Bohnhoff, W.J., Dalbey, K.R., Ebeida, M.S., Eddy, J.P., Eldred, M.S., Geraci, G., Hooper, R.W.,
        Hough, P.D., Hu, K.T., Jakeman, J.D., Khalil, M., Maupin, K.A., Monschke, J.A., Ridgway, E.M., Rushdi, A.A.,
        Stephens, J.A., Swiler, L.P., Vigil, D.M., Wildey, T.M., and Winokur, J.G., "Dakota, A Multilevel Parallel
        Object-Oriented Framework for Design Optimization, Parameter Estimation, Uncertainty Quantification, and
        Sensitivity Analysis: Version 6.11 User’s Manual," Sandia Technical Report SAND2014-4633, July 2014;
        updated November 2019.
	
- **How is it that Sandia can release government software as open source?**
    Sandia is a government-owned, contractor-operated (GOCO) national laboratory operated for the U.S. Department of Energy (DOE) by National Technology and Engineering Solutions of Sandia, LLC. The authority to release open source software resides with the DOE, and Dakota has gone through a series of copyright assertion and classification approvals to allow release to the general public. Important proponents for the open source release of Sandia software are the DOE's Accelerated Strategic Computing (ASC) Program Office and the DOE's Office of Science.
- **Why are you releasing Dakota as open source?**
    To foster collaborations and streamline the licensing process. Of particular note is the fact that an export control classification of "publicly available" allows us to work effectively with universities. For more on some of the motivations behind open source software in general, The Cathedral and the Bazaar is interesting reading.
- **Does the Dakota software have assigned manufacturer part numbers (MPNs) or stock-keeping units (SKUs)?**
    No. Dakota software releases are versioned, but are are not assigned MPNs nor SKUs. Dakota is open source software, distributed under the GNU LGPL license and associated compatible licenses.

.. _faq-downloading:

===========
Downloading 
===========

- **When I download one of the Dakota manuals in PDF, Acrobat reader fails.**
    The problem appears to be with embedded PDF viewers in some browsers, rather than with the PDF files themselves. In particular, problems have been reported when using Acrobat 5.0 from within Internet Explorer or Netscape, whereas other combinations work fine. In these cases, we recommend the following:
    
  - Try saving the file to disk and using Acrobat reader outside of the browser (bypassing the browser-embedded PDF viewer).
  - Try another computer/browser/Acrobat combination.
  - For the Reference and Developers manuals, you can use the HTML documentation if hardcopies are not needed.

.. _faq-running:

==============
Running Dakota
==============

- **When I try to run Dakota in parallel, the system hangs/aborts.**
    This problem can result from not being able to open a remote/secure shell (rsh/ssh) without a password challenge. MPI by default uses rsh, but can be configured to use ssh. Read the man pages for the shell in use and set up the necessary files (e.g., .rhosts, authorized_keys) so that you can open a shell on the target machine without a password challenge. Parallel Dakota runs should then work.
- **When I try to run Dakota with my simulator, I get a "command not found" error when Dakota attempts to execute the simulator.**
    This is most commonly a path issue. Some platforms do not provide "." (the current working directory) as a default search path within user environments. To add it, use ``export PATH=$PATH:.`` (for sh, bash, or zsh) or ``set path = ($path .)`` (for csh or tcsh), either at your shell command prompt (for temporary addition) or within your shell resource file (for permanent addition). Alternatively, modify your Dakota input file by putting "./" in front of the name of the simulator (if it is in the current directory), or by specifying a full pathname to the simulator.
- **When I try to run Dakota with my input file, I get an "Invalid iterator" error.**
    You selected an iterator in your method specification that comes from a package which was omitted from the configuration/build of your Dakota executable. For example, Dakota binaries must omit DOT and NPSOL since we cannot distribute optional commercial library extensions. The solution to the problem is to select a different iterator from one of the available packages (e.g., CONMIN and OPT++ may be used in place of DOT and NPSOL).
- **When I try to run Dakota with my input file, I get a parser error.**
    First, cross-reference your input syntax with the master input specification reference (dakota.input.nspec or the generated dakota.input.txt) that was used in building your executable. Also, refer to the "Common Specification Mistakes" section in the Reference Manual. If you still can't find the problem, check out some of our other help resources.
- **When I try to execute a Dakota binary, I get an error message that there are missing libraries.**
    When building Dakota executables, a static linking of all libraries is not always possible (since, in some cases, only shared object libraries are made available by
    the platform vendor). If differences in the required shared object libraries exist between the build and run platforms, then the run time shared object linker
    will abort with an error.
	
    To resolve this, there are a few courses of action:
	
    1. Locate the missing shared object libraries on your system (using whereis or find) and then add this path to your linker path (``$LD_LIBRARY_PATH`` on most \*nix platforms; ``$DYLD_LIBRARY_PATH`` on OS X),
    2. Contact your platform vendor for the missing libraries, or
    3. Build Dakota from source on your run platform. It may also prove useful to list your shared object library dependencies using ``ldd dakota``.
	
- **How can I run Dakota with QUESO Bayesian calibration?**
    Why when I run Dakota and specify method > bayes_calibration > queso, do I get an error that it's not available? Public Dakota binaries do not include QUESO due to its dependence on the GPL-licensed GSL library. To run Dakota with QUESO a couple options include:
    
  - Compile from source code, enabling GSL and QUESO.
  - Try out Kayla Coleman's Ubuntu Docker container that has Dakota + QUESO running in it.

.. _faq-interfacing:

==================================
Interfacing Dakota to a Simulation
==================================

- **How do I tightly couple Dakota to my software using Dakota's library mode?**
    Refer to the Dakota Developer's Manual section on Interfacing with Dakota as a Library.
- **How do I implement Dakota's black-box interface to my simulation?**
    Refer to Sections 1.3 and 17.1 of the User's Manual for additional information. Chapter 17 references example files included with the Dakota distribution which demonstrate how to construct a black-box interface. In addition the Users Manual sections on "Dakota Parameters File Data Format" and "Dakota Results File Data Format" may be helpful.
- **What are the options for interfacing Dakota to my computational model?**
    Dakota can be either loosely or tightly coupled to a simulation. Most users start by loosely coupling Dakota to an application using Dakota's black-box interface. In this mode, Dakota exchanges information with the application through the file system and executes the application with a system call. Some users wish to use Dakota's library mode to tightly couple Dakota algorithms with their applications. This more advanced use case can be very powerful, but requires programming to Dakota's C++ library API and typically involves compiling Dakota from source.

.. _faq-building:

===============
Building Dakota
===============

- **I get compile-time MPI errors: "SEEK_SET is #defined but must not be for the C++ binding of MPI" and similar for SEEK_CUR and SEEK_END.**
    When compiling Dakota against the MPI2-compliant OpenMPI, you will need to define `MPICH_IGNORE_CXX_SEEK` at compile time, e.g., add the following to `CPPFLAGS`: `-DMPICH_IGNORE_CXX_SEEK.`
- **My build fails because it can't find header files/libraries that Dakota needs.**
    The Dakota configuration files are set up for a typical build within the Sandia environment. Customizations for other environments may be needed and will typically involve supplying overrides or additional path information to CMake. Refer to the INSTALL file within the source distribution for additional information.

.. _faq-featureadditions:

=================
Feature Additions
=================

- **What are the terms of contribution?**
    Contributions to Dakota, including the Dakota GUI are subject to the terms of their respective licenses. Contributions which are derivative works of Dakota or the Dakota GUI will therefore be accepted under the same license terms as the product from which they are derived. Contributions which are not derivative works, such as additional Dakota examples, should be licensed as permissively as possible, preferrably BSD or similar.
    
    Along with or following your contribution, please include:
    
    - Complete list of authors and affiliations at time of authorship.
    - Consent from each author indicating the following or similar:
    - I contributed [NAME OF FEATURE], via patches submitted to the Dakota team. I agree to the following terms and conditions for my contributions: First, I agree my contributions are submitted under the terms of the LGPL [EPL for Dakota GUI] license. Second, I represent I am authorized to make the contributions and grant the license. If my employer has rights to intellectual property that includes my contributions, I represent that I have received permission to make contributions and grant the required license on behalf of my employer.
- **How can I contribute?**
    Our open source software benefits greatly from the contributions of its user community. Ways that you can contribute include:
    
    - Use the code and offer feedback. We welcome constructive suggestions.
    - Port Dakota to another platform or operating system and share the configuration extensions.
    - Add a capability such as a new iterative algorithm, surrogate model, or interface; this extension typically involves a class derivation along with the definition of a few virtual functions (refer to the Developer's Manual for information on class hierarchies and the structure provided by their base classes).
    - Resolve a bug you've observed or has been discussed on the dakota-users mailing list. 
  
    Submitting changes:
    
    - Dakota's issue tracking system is not publicly accessible at this time.
    - If you wish to submit a patch or larger changeset, please discuss on the dakota-users mailing list.
    - A Dakota development team member will ask you to submit the changes via email or (if large enough) our file transfer system.

.. _faq-support:

=======
Support
=======

- **Which DoD systems provide Dakota?**
    Dakota is included in the DoD High Performance Computing Modernization Program (HPCMP) Common Open Source Software Base and is supported on related DoD HPC systems. Other DoD sites have installed Dakota as well; inquire with your local system administrators for more information.
- **Do you support Macintosh?**
    A Mac OS X port has been made available starting with the DAKOTA v3.2 release.
- **Do you support Windows?**
    Dakota can be compiled using Microsoft Visual Studio and the Intel compilers. A Windows version of Dakota is available on our Downloads page. Dakota can be compiled in the Cygwin environment.
- **Is training available for Dakota?**
    The Dakota team performs regular training for DOE laboratories and industrial CRADA partners, but not normally for other users. Our introductory training sessions closely follow the User's Manual, especially Chapter 2, so careful study of this document should be enough to get you started.
- **What information should I include with my support request or bug report?**
    When contacting the dakota-users mailing list or other mechanism for help, please clearly specify (1) what you expected to happen, (2) what you tried, and (3) what resulted instead. In particular, be sure to include the following:

    - Brief problem description.
    - DAKOTA version: either major release version number (e.g., 5.4) or stable subversion revision number (e.g., 5.4+, r2012). Determine the version number based on which DAKOTA you downloaded, or if installed and running, by typing "dakota -version".
    - Operating system (Linux, Solaris, AIX, Windows, Mac OS X, etc.), including the distribution and/or version, and architecture (Intel, AMD, PowerPC, etc.). Indicate whether you're running in a 32- or 64-bit environment.
    - For problems running DAKOTA, include:

      - Relevant DAKOTA input deck, scripts (if possible), and commands executed.
      - Relevant output from code, for example, run ``dakota -i inputfile -o output.txt -e error.txt`` or perhaps more usefully, since both standard and error output will appear in the same file: ``dakota >output.txt 2>&1" (if you are using sh or bash or zsh) or "dakota >& output.txt`` (if you are using csh or tcsh).

    - For problems compiling DAKOTA include:
      
      - Your BuildDakotaTemplate.cmake file, along with any other files you modified.
      - The commands used to invoke cmake and make and their output to the screen. E.g. "cmake -C BuildDakotaTemplate.cmake $DAK_SRC > cmake_output.txt 2>&1"
      - Since make output can be voluminous, run make until the failure occurs, then type "make >make.out 2>&1" (or similar) to just capture the problem behavior.
      - The CMakeCache.txt file created by cmake (located in your build directory.)
- **Is support available for Dakota?**
    Not in the sense of commercial software. See Help Resources for getting help with Dakota. We track problem reports and enhancement suggestions in Trac, where they are are vetted, prioritized, and planned. Enhancements are then accessible through our Stable releases.

