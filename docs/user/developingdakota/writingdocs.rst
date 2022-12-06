.. _writing_dakota_docs:

""""""""""""""""""""""""""""
Writing Dakota Documentation
""""""""""""""""""""""""""""

===========================
Install Sphinx and Packages
===========================

.. code-block::

	pip install --user -U Sphinx
	pip install --user -U myst-parser
	pip install --user -U sphinx-rtd-theme
	pip install --user -U sphinxcontrib-bibtex
	

.. note::	

   You may alternately use ``pip3`` if targeting Python 3 when both 2 and 3 are installed.

===============================
Interactive Documentation Build
===============================

-----------------
Bare Sphinx Build
-----------------

A bare ``sphinx-build`` command will build almost all of the documentation, except for keyword reference pages and sample Dakota input files.

.. code-block::

   cd <dakota checkout>/docs/user
   sphinx-build -b html <source dir> <build dir>
   # to speed up build using all available cores (may suppress some cross reference and other warnings/errors):
   sphinx-build -b html -j auto <source dir> <build dir>

------------------------------
CMake-based Build on Mac/Linux
------------------------------

This build process is more similar to historical documentation build using the routine CMake-based build process.

**Prerequisites**

- Python and the Sphinx Python tools specified above, with the sphinx-build executable on your PATH.
- Java JDK 11 or newer. For example on RHEL7 install java-11-openjdk-devel from EPEL and specify JAVA_HOME:PATH=/usr/lib/jvm/java-11 to CMake
- Doxygen 1.8.20 or newer (for C++ docs/dev manual)
- LaTeX (for PDF output), probably 2015 or newer will do
- Perl for generating user manual input samples

**Building / Editing**

First, configure Dakota with CMake, enabling documentation as usual, e.g., specifying the Boolean ENABLE_DAKOTA_DOCS or the typical developer
convenience macros such as DevDistro. You may need to help CMake find Java JDK 11 per above. While in theory reference manual building should work without
enabling tests, enabling them via DAKOTA_ENABLE_TESTS may be a safer choice.

Then build the docs-sphinx target (or optionally the docs-keywords, then docs-sphinx target, but the dependency is tracked) via IDE or command line, e.g.,

.. code-block::

   cd <dakota build>/docs
   # Generate reference content (will be built automatically by the dependent targets)
   make docs-keywords
 
   # Build the whole sphinx manual in serial
   make docs-user
   # Build the whole sphinx manual in parallel much faster but may omit some warnings/errors
   make docs-user-fast
   # Remove output dir user-html
   make docs-user-clean
 
   # Generate C++ Doxygen developer manual (requires LaTeX)
   cd <dakota build>/docs/dev
   make docs-dev-html
   make docs-dev-pdf

*Notes:*

- The build is setup to use all available cores to build the docs, so should only take a couple minutes.
- The keyword files will get generated directly into the source tree at <dakota source>/docs/user/usingdakota/reference (for now) while the generated manual will appear in <dakota build>/docs/user-html/
- Note that historical targets for building User's, and Theory manuals remain in inactive and are activate with DAKOTA_DOCS_LEGACY, but not Reference, since they are (1) broken and (2) subsumed by the new process.

*Development Process Notes:*

- When iteratively editing, if you edit a file in the keywords/ directory it should make the docs-keywords (and therefore docs-sphinx) target out of date.
- If you add a new keywords/ file you likely need to touch <dakota source>/docs/CMakeLists.txt to force a rescan of files
- You may need to force remove the sphinx output tree <dakota build>/docs/user-html/ to force a rebuild

----------------------
CMake-based on Windows
----------------------

You can build the user manual in its entirety on Windows using CMake, though currently Visual Studio is required.

Requirements:

- CMake
- Perl
- Python + Sphinx
- Java 11 JDK
- Visual Studio (community edition will work)

Command prompt config example, since I didn't have sphinx-build on my global PATH. If you do, or you set a variable like https://cmake.org/cmake/help/latest/variable/CMAKE_PROGRAM_PATH.html#variable:CMAKE_PROGRAM_PATH to find it, you can likely do this from the CMake GUI

.. code-block::

   set PATH=C:\Users\briadam\AppData\Roaming\Python\Python310\Scripts;%PATH%
   cmake -D DAKOTA_DOCS_DEV:BOOL=FALSE -D JAVA_HOME:PATH="C:\Program Files\Eclipse Adoptium\jdk-11.0.16.8-hotspot" -D Python_EXECUTABLE=C:\Python310\python.exe ..\source\docs

After which you can open DakotaDocs.sln in Visual Studio and build some or all of the projects.

Alternately, from command line in the build dir: ``cmake --build . --target docs-user-samples``

-----------------------
Keyword Reference Pages
-----------------------

*The following section only applies if you are building all the documentation without CMake. The existing CMake build process will handle keyword reference page generation automatically.*

Fully-fleshed-out keyword reference pages are not checked in under with the rest of the documentation files in the ``docs/user`` folder. The keyword documentation
contents are stored in the ``docs/keywords`` folder. In this folder, each file contains RST snippets, with custom Dakota markdown that demarcates the sections in
each file. These files need to be built into full RST pages before being added to the larger Sphinx manual. To add these keyword reference pages to the Sphinx build,
you must perform the following additional steps:

1. Acquire a Java IDE (such as Eclipse or IntelliJ). The next step will be much easier for you if you get an IDE to manage your Java classpath for you.
2. Run the RefManGenerator application in the java_utils project, using "<dakota checkout>/docs/keywords" as the first argument and an empty output folder as the second argument.
3. After RefManGenerator finishes running, copy all the output from your output folder to "<dakota checkout>/docs/user/usingdakota/reference" folder
4. Run the build commands in the "Bare Sphinx Build" section. Make sure that build detects all the keyword pages that you copied (i.e. it should take longer to build)

**Important:** Note that if you are hunting down documentation errors by line number, the line numbers displayed in Sphinx build warnings/errors allude to the fully-built
keyword RST pages, NOT the RST snippet files in docs/keywords. However, the original snippet files will be where you want to ultimately make the correction.

-------------------------------------------
Legacy: Convert LaTeX to RST through Pandoc
-------------------------------------------

.. code-block::

   pandoc -f latex -t rst Users_Preface.tex -o Users_Preface.rst