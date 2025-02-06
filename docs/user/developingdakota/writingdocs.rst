.. _writing_dakota_docs:

""""""""""""""""""""""""""""""
Dakota Documentation Mechanics
""""""""""""""""""""""""""""""

Authoring Dakota documentation requires tools described in
:ref:`devenv`, notably Python Sphinx, Python/Perl (to extract input
file examples), Java Development Kit (to parse ``dakota.xml`` and
keyword reference files), and optionally Doxygen and LaTeX (for the
C++ developer manual).

===========================
Install Sphinx and Packages
===========================

A representative way to install the tools:

.. code-block::

	pip install --user -U Sphinx
	pip install --user -U myst-parser
	pip install --user -U sphinx-rtd-theme
	pip install --user -U sphinxcontrib-bibtex
	

.. note::	

   You may alternately use ``pip3`` if targeting Python 3 when both 2
   and 3 are installed.

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

- Python and the Sphinx Python tools specified above, with the
  sphinx-build executable on your PATH.

- Java JDK 11 or newer. For example on RHEL7 install
  java-11-openjdk-devel from EPEL and specify
  JAVA_HOME:PATH=/usr/lib/jvm/java-11 to CMake

- Doxygen 1.8.20 or newer (for C++ docs/dev manual)

- LaTeX (for PDF output), probably 2015 or newer will do

- Perl for generating user manual input samples

**Building / Editing**

First, configure Dakota with CMake, enabling documentation as usual,
e.g., specifying ``ENABLE_DAKOTA_DOCS:BOOL=TRUE`` or the typical
developer convenience macros such as ``DevDistro``. You may need to
help CMake find Java JDK 11 per above. While in theory reference
manual building should work without enabling tests, enabling them via
``DAKOTA_ENABLE_TESTS:BOOL=TRUE`` may be a safer choice.

Then build the docs-sphinx target (or optionally the docs-keywords,
then docs-sphinx target, but the dependency is tracked) via IDE or
command line, e.g.,

.. code-block::

   cd <dakota build>/docs
   # Generate reference or example input file content
   # (will be built automatically by the dependent targets)
   make docs-keywords
   make docs-user-samples

   # To see the most current make targets:
   make help
 
   # Build the whole sphinx manual in serial
   make docs-user
   # Build the whole sphinx manual in parallel much faster but may omit some warnings/errors
   make docs-user-fast
   # Only rebuild updated content (may miss some dependencies, but is fast):
   make docs-user-increm

   # Remove output dir user-html; useful to force generation of warnings/errors
   make docs-user-clean
 
   # Generate C++ Doxygen developer manual (requires LaTeX)
   cd <dakota build>/docs/dev
   make docs-dev-html
   make docs-dev-pdf

*Notes:*

- The build is set up to use all available cores to build the docs, so
  should only take a couple minutes.

- The keyword files will get generated directly into the source tree
  at ``<dakota source>/docs/user/usingdakota/reference`` (for now) while
  the generated manual will appear in ``<dakota build>/docs/user-html/``

- When iteratively editing, if you edit a metadata file in the
  ``keywords/`` directory it should make the ``docs-keywords`` (and
  therefore docs-user*) target out of date.

- If you add a new ``keywords/`` metadata file you likely need to
  touch ``<dakota source>/docs/CMakeLists.txt`` to force a rescan of
  files

- Historical targets for building User's, and Theory manuals remain in
  ``docs/inactive`` and are enabled with ``DAKOTA_DOCS_LEGACY``. The
  Reference manual is not, since it's (1) broken and (2) subsumed by
  the new Sphinx process.


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

   set PATH=C:\Users\username\AppData\Roaming\Python\Python310\Scripts;%PATH%
   cmake -D DAKOTA_DOCS_DEV:BOOL=FALSE -D JAVA_HOME:PATH="C:\Program Files\Eclipse Adoptium\jdk-11.0.16.8-hotspot" -D Python_EXECUTABLE=C:\Python310\python.exe ..\source\docs

After which you can open DakotaDocs.sln in Visual Studio and build some or all of the projects.

Alternately, from command line in the build dir: ``cmake --build . --target docs-user-samples``

-----------------------
Keyword Reference Pages
-----------------------

.. note::

   The following section only applies if you are building all the
   documentation without CMake. The existing CMake build process will
   handle keyword reference page generation automatically.

Fully-fleshed-out keyword reference pages are not checked in under with the rest of the documentation files in the ``docs/user`` folder. The keyword documentation
contents are stored in the ``docs/keywords`` folder. In this folder, each file contains RST snippets, with custom Dakota markdown that demarcates the sections in
each file. These files need to be built into full RST pages before being added to the larger Sphinx manual. To add these keyword reference pages to the Sphinx build,
you must perform the following additional steps:

1. Acquire a Java IDE (such as Eclipse or IntelliJ). The next step will be much easier for you if you get an IDE to manage your Java classpath for you.

2. Run the RefManGenerator application in the java_utils project, using "<dakota checkout>/docs/keywords" as the first argument and an empty output folder as the second argument.

3. After RefManGenerator finishes running, copy all the output from your output folder to "<dakota checkout>/docs/user/usingdakota/reference" folder

4. Run the build commands in the "Bare Sphinx Build" section. Make sure that build detects all the keyword pages that you copied (i.e. it should take longer to build)


-------------------------------------------
Legacy: Convert LaTeX to RST through Pandoc
-------------------------------------------

.. code-block::

   pandoc -f latex -t rst Users_Preface.tex -o Users_Preface.rst


========================
Authoring Sphinx Content
========================

See guide:
https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html;
some key things are highlighted here for convenience.

--------------------------------
Converting Old User/Theory LaTeX
--------------------------------

* Code (input, output fragments):

  - Remove needless indents to the minimum needed for Sphinx

  - The long-form ``code-block::`` directive (contrast with anonymous
    ``::`` ) admits a label and caption for cross-referencing when needed:
    https://www.sphinx-doc.org/en/master/usage/restructuredtext/directives.html#directive-code-block

* Dakota Keywords (also see discussion below)

  - :literal:`:dakkw:`variables-normal_uncertain`` to link and set in literal
    (warning: the role doesn't validate the link)

  - :literal:```normal_uncertain``` to just set in literal

  - :literal:`:ref:`Normal Uncertain <variables-normal_uncertain>`` to link
    keyword with alternate text in normal font

* Displayed environments (Listing, Figure, Table, Equation; see details below):

  - Use name or label, caption

  - Name after section/topic and figure content, e.g.,
    ``fig:pstudy:vector`` instead of ``pstudy02``, so it's clear what's
    referenced.

  - Refer to mainly with :literal:`:numref:`fig-name``, though can also do
    :literal:`:ref:`Link Text <fig-name>``

* Quotes: Replace fancy quotes with raw single or double quotes in
  source .rst files

* Referencing chapters, sections, etc.:

  - Using the section title as the link text :literal:`:ref:`dace:sa``

  - With custom link text :literal:`:ref:`DACE-based sensitivity
    analysis <dace:sa>``

* TODOs/Comments: Use bare ``..`` directive with indented comment starting
  on immediate next line.

Helpful Sphinx Roles:
https://www.sphinx-doc.org/en/master/usage/restructuredtext/roles.html

* ``:command:``, ``:program:`` (OS command or program, produces
  literal + strong)

* ``:file:`` (file or path), ``:samp:`` (code sample with
  substitution); both produce literal + emphasis

* ``:math:``

* ``:ref:``, ``:numref:``

Helpful Sphinx Directives:
https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html#directives:

* ``note::``
* ``warning::``

--------------------
Fixing Common Errors
--------------------

* **"Block quote ends without a blank line; unexpected unindent."** or **"Unexpected indentation."**

  - Sections in reStructuredText are indentation-based. This error
    means there is an unexpected indentation in the flow of the
    document. Usually this actually means that a line in the middle of
    a paragraph begins with a space, so RST is not sure whether to
    start a new subsection or not.

* **"Bullet list ends without a blank line; unexpected unindent."**

  - A bulleted list was started (typically using a hyphen), but the
    next line is not indented to indicate a continuation of the first
    bullet element, neither does the next line having another hyphen
    indicating the next bullet element.

* **"Explicit markup ends without a blank line; unexpected
  unindent."**

  - When a markup section is completed, it must be followed by a space
    before the next, new section begins.

* **"Inline interpreted text or phrase reference start-string without
  end-string."**

  - As it says, an inline expression (such as a hyperlink, a piece of
    code, or a math expression) was started but not completed. Check
    to see if there is a ` quote character that is used when ' or "
    would be more appropriate, since ` has special meaning in RST.

* **"undefined label"**

  - A global page reference was linked to, but that page reference was
    not previously defined. Usually this alludes to a spelling or
    punctuation mistake.

-----------------------------
Converting LaTeX Environments
-----------------------------

Converting and referecing code, figures, tables, equations...

**Code Listings**

To create a code block / listing that you can cross-reference, use the
`code-block directive
<https://www.sphinx-doc.org/en/master/usage/restructuredtext/directives.html#directive-code-block>`_,
with a name and caption. For example::

  .. code-block:: python
     :caption: this.py
     :name: this-py
  
     print 'Explicit is better than implicit.'

Alternately, for file-based code inclusion, use the `literalinclude
directive
<https://www.sphinx-doc.org/en/master/usage/restructuredtext/directives.html#directive-literalinclude>`_
with similar options.

Refer to these using numref, which will auto-generate text like see
Listing 4, e.g.::

  See :numref:`this-py` for an example.

**Figures**

If a figure
https://docutils.sourceforge.io/docs/ref/rst/directives.html#figure is
given a :name: you can then refer to it using :numref: and it'll
automatically be called Figure 6 or
whatever... https://www.sphinx-doc.org/en/master/usage/restructuredtext/roles.html#cross-referencing-figures-by-figure-number. The
caption goes as the first line of the directive body::

  .. figure:: img/sbo_mh.png
     :alt: SBO iteration progression for model hierarchies.
     :name: fig:sbo_mh
  
     This is the caption all about how this figure highlights SBO's
     amazing properties!

**Tables**

Tables
https://docutils.sourceforge.io/docs/ref/rst/directives.html#table
also support the :name: property for use in references. This is
untested, but hopefully works with :literal:`:numref:`table:people30s` ` to
auto-generate Table 74 in the text::

  .. table:: Personnel
     :name: table:people30s
  
     +----------+---------+
     |Name      |Age      |
     +==========+=========+
     |Bill      |39       |
     +----------+---------+
     |Jane      |38       |
     +----------+---------+

There are also list-formatted and CSV table options which are quite
handy and alleviate the need for fixed-width "painted" tables.

**Math Equations**

(This is likely good advice, though not deployed yet...) To label and
reference (displayed / non-inline) equations use the :label: property
and numref, e.g.,
https://www.sphinx-doc.org/en/master/usage/restructuredtext/domains.html#role-math-numref.

Sphinx already puts equations in an equation or align environment
automatically as needed, so it's usually not necessary to explicitly
retain those, just update them to have a single alignment mark (&) and
leave a blank line in between multiple equations::

  .. math:: e^{i\pi} + 1 = 0
     :label: euler

     Euler's identity, equation :math:numref:`euler`, was elected one of
     the most beautiful mathematical formulas.

    
    .. math:: 
       :label: euler2
    
       e^{i\pi} + 1 = 0
  
     Euler's identity, equation :math:numref:`euler2`, was elected one of
     the most beautiful mathematical formulas.

---------------------
Formatting Principles
---------------------

Suppress Developer Speak:

* Only refer to C++ Class concepts when discussing the code itself.

* Notably, user docs should refer to "method" not "iterator"

**Keyword References:** When should I use a inline code (literal role)
(``foo``), and when should I hyperlink to the keyword reference?

* What's the purpose of the explanation (it may be okay to replace
  keywords with "plain english" descriptions: "uniform uncertain
  variable" instead of "uniform_uncertain")

* When using a keyword, use the literal role.

* Optionally, link to the reference manual page. Things to think
  about:

  - Avoid gratuitous hyperlinks, which are distracting to the reader.

  - Is there an obvious way for the user to easily drill down to that
    information themselves nearby?

  - Is the specific instance of the keyword you are referring to
    ambiguous? E.g. initial_point could, in some contexts, refer to
    that keyword for a specific type of design variable, or it could
    refer to that property of all the design variables. In the former
    case, it's fine to link. In the latter, make sure your intent is
    clear.

  - Avoid using a mixture of hyperlinks and inline code for a keyword
    in a single section. That is, don't hyperlink 'uniform_uncertain'
    once and then use ``uniform_uncertain``` a bunch of other times,
    leaving the user to guess which ones are links. (Use plain english
    descriptions of the keyword after the initial hyperlink instead.)


================================
Authoring Keyword Reference Docs
================================

Keyword reference pages:

- Are documented in ``docs/keywords`` following the keyword hierarchy
  expressed in ``src/dakota.xml`` (or equivalently
  ``src/dakota.input.nspec``)

- Written in Sphinx-compatible ReStructuredText format

- Templated in ``docs/kw_abbrev_template.rst`` which offers a quick
  skeleton for new keywords. (files ``docs/kw_*.txt`` summarize more
  detailed guidance, but haven't been updated to .rst format)

.. attention::

   If you are hunting down documentation errors by line number, the
   line numbers displayed in Sphinx build warnings/errors allude to
   the fully-built keyword RST pages, *not* the RST snippet files in
   ``docs/keywords``. However, the original snippet files will be
   where you want to ultimately make the correction.

--------------------
Legacy Keyword Tools
--------------------

.. note::

   The historical
   ``dakota/local/scripts/generate_keyword_template.sh`` (accessible
   only to SNL developers) likely still has value when making large
   input specification / keyword changes. It has not been updated for
   the new Sphinx documentation process, yet the following notes may
   be useful in the future.

Prerequisite: Make sure you ran cmake with
``ENABLE_DAKOTA_DOCS:BOOL=TRUE``.

In ``path/to/dakotaBuild/docs``, do::

    make clean
    make docs-keywords >& make.log

While still in that directory, do the following to generate lists of
missing and obsolete keyword metadata files::

    path/to/dakotaSource/local/scripts/generate_keyword_template.sh -g make.log

You should find that three files have been created:
``missingKeywords.txt``, ``missingDuplicates.txt``, and
``obsoleteKeywords.txt``.

You can either use the lists in those files to create/remove keyword
metadata files by hand. If you would like to use this script to
facilitate the creation/removal of keyword metadata files, review and
edit those files to remove keywords that are not relevant to your
work, i.e., that you are not responsible documenting.  More
specifically,

* In missingKeywords.txt, there is a STOP entry.  Do not remove this entry.

  - In the list before the STOP entry, remove any keywords that you
    are not documenting.

  - Below the STOP entry is a list of keywords that may be duplicates,
    They are listed under their respective candidate DUPLICATE file.
    Remove any keywords that are not yours or that should not use the
    listed DUPLICATE file.  If you remove all of the keywords under a
    DUPLICATE file, then also remove the DUPLICATE file from the list.

   - If you have added new keywords that warrant the creation of a new
     DUPLICATE file, you may manually add it to the duplicate list in
     the file.  Manual entries should take the following form::

       <blank line>
       
       DUPLICATE-smallestRelevantKeywordFIlename
       
       fullKeywordFileName1
       
       fullKeywordFileName2
       
       ...

   - Note that the keywords can exist in both the list before STOP and
     the one after.  The script should handle it correctly.

* In the missingDuplicates.txt file, remove any that you are not
  responsible for.

* In obsoleteKeywords.txt, there is a STOP entry.  Do not remove this
  entry.

  - In the list before the STOP entry, remove any keywords that are
    not yours.

  - Below the STOP entry is a list of keywords that may have changed
    levels in the keyword hierarchy and the proposed move of an
    obsolete keyword to a new keyword.  Remove (or edit) any that are
    not yours or that are incorrect.  You may also manually add to the
    list.  You should also be able to use this manual approach to
    identify keywords that have changed names.  Manual entries should
    take the following form::

      MV:fullOldKeywordFileName:TO:fullNewKeywordFileName

  - Note that the keywords can exist in both the list before STOP and
    the one after.  The script should handle it correctly.

Then, while still in ``path/to/dakotaBuild/docs``, run the script
again with one or more of the following options:

* ``-k fileName`` (to create templates for missing keywords)

  - For new keywords that are duplicates, this will create keyword
    metadata files with the DUPLICATE file already inserted.  If the
    DUPLICATE file is also new (i.e., if you added it manually to
    fileName), a DUPLICATE file will be created with template content.
    The file duplicatesAdded.txt will be created if any duplicate
    keywords were added and will contain a list of them.  The file
    dupFilesAdded.txt will be created if any new DUPLICATE files were
    added and will contain a list of them.

  - For new keywords that are not duplicates, this will create new
    keyword metadata files with template content.  The file
    keywordsAdded.txt will be created if any new (non-duplicate)
    keywords were added and will contain a list of them.

* ``-d fileName`` (to create templates for missing duplicate files)

  - This will create DUPLICATE files with template content.  This
    covers the case where there are keyword metadata files that
    reference DUPLICATE files that do not exist.  The file
    dupFilesAdded.txt will be created if any new DUPLICATE files were
    added and will contain a list of them.

* ``-r fileName`` (to remove obsolete keywords)

  - For keywords that have changed levels in the hierarchy (or
    manually added ones that have name changes), this will move the
    old file to the new file.  The file levelChanged.txt will be
    created if any keywords have been moved and will contain a list of
    them.

  - For all other keywords in this file, the metadata files will be
    removed.  The file keywordsRemoved.txt will be created and will
    contain a list of them.

The files created by running this script are intended to provide a
quick check that the right thing happened.  Ultimately, you should
check in ``path/to/dakotaSource/docs/keywords`` to make sure the
expected files were created/removed. You can now populate the content
of the created keyword metadata files. After you have entered all
content, remove "TEMPLATE" from the top of the file.


==================
Authoring Examples
==================

Principles for authoring and testing examples in the
``dakota/dakota-examples`` repository.

-----------------
Example Structure
-----------------

An example consists of an explanatory README in markdown format, plus
other files provided for purposes of illustration. The README has two
required sections:

* Summary - (required) one or two sentence summary aimed at helping
  users to make a quick determination whether the example will be
  useful to them

* Description - (required) Longer description (up to a few paragraphs)
  describing the example. Executive Summary/Abstract.

In addition, authors are asked to include sections using these names
where applicable.

* How to run the example - (optional) Describe how to run the example,
  if applicable

* Requirements - (optional) Requirements for running the
  example. E.g. Python version, OS

* Contents - (optional) list the contents and give a brief (few word)
  description of what they are

* Further Reading - (optional) Dakota documentation sections,
  published papers, SAND reports, screencasts etc.

Authors are free to add other sections or to subdivide the sections above.

**Guidance for Writing a Good Example**

* The purpose of the example, or what the user should learn from it,
  should be clearly stated.

* Keep the purpose of the example in mind as you write, and avoid
  including detail that does not serve the purpose.

* Avoid duplication of content from the manuals. Refer to them
  instead. Although you can create hyperlinks to specific versions of
  the documentation, it may be better in many cases to refer to the
  most recent versions. Some handy URLs that we update at each
  release:

  - https://dakota.sandia.gov/content/manuals: Landing page for all the documentation, including older releases

  - https://dakota.sandia.gov/sites/default/files/docs/latest_release/user-html/: Latest User Guide; for any user, reference, theory content

  - https://dakota.sandia.gov/content/latest-developers-manual: Landing page for most recent version of the Developer's Manual

  - https://dakota.sandia.gov/sites/default/files/docs/latest_release/user-html/usingdakota/reference/environment.html:
    Example of linking to a specific keyword in the latest release of
    the Reference Manual

  - https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1Environment.html: Example of linking to specific class documentation in the latest release of the Developer's Manual

  - https://dakota.sandia.gov/sites/default/files/docs/latest_release/user-html/usingdakotagui/usingdakotagui.html: Latest GUI Manual

  - https://dakota.sandia.gov/sites/default/files/docs/latest_release/user-html/usingdakota/theory.html: Latest Theory Manual

  - https://dakota.sandia.gov/content/latest-release-notes: Latest Release Notes

* As much as possible, create examples that can run on all of our
  supported platforms (Windows, Mac, and Linux).

* Set up the example to be automatically tested. Testing ensures that
  runnable parts of examples continue to work as Dakota evolves.

----------------
Testing Examples
----------------

This brief guide is intended to help Dakota developers set up
automatic testing for an example in the Dakota Examples Library.

**Why add tests?**

We want examples to work as intended with the version of Dakota (and
auxiliary tools) for which they were designed. The Examples Library
will be automatically tested periodically.

**How do I run the tests?**

The Examples Library will be checked out as a submodule when you clone
the main Dakota repository, and the tests can be run by ctest if the
CMake variable ``DAKOTA_TEST_EXAMPLES_REPO:BOOL=TRUE`` was set at
configure time.

**What kinds of tests are supported?**

Four kinds of tests are supported:

#. Regression. These work just like regression tests in Dakota's test
   folder. They have a baseline that Dakota's console output is
   compared to, and have a PASS/DIFF/FAIL result. (The baseline
   platform is RHEL8 and all examples should reliably PASS on RHEL8).

#. Dakota Check. Run 'dakota -check' on a Dakota input to see whether
   it passes basic parsing/construction. The result is PASS or FAIL.

#. Dakota Run. Run Dakota on the input and just examine the
   returncode. This is a way to make sure that an input + driver runs
   to completion when you don't care much about the numerical
   results. PASS/FAIL.

#. Arbitrary Command. Run an arbitrary command and examine the
   returncode. The command could be an Python script that contains
   unit tests, for example. PASS/FAIL.

In addition to Dakota executables built by the Dakota project, tests
may use Python 3 (with the ``numpy`` and ``h5py`` modules), the
``dakota.interfacing`` Python module, and the new surrogate
capabilities.

**How do I add tests?**

Tests for all official examples are added by declaring them using the
``dakota_example_test`` CMake function in
``DakotaExamplesOfficial.cmake``. Multiple tests can be added in each
call. The signature of this function looks like::

  dakota_example_test(
      PATH path
      [CHECK input_name1 [input_name2 ...]]
      [RUN input_name1 [input_name2 ...]]
      [REGRESS input_name1 [input_name2 ...]]   
      [COMMAND command1 ]
      [DEPENDS test_name1 [test_name2 ...]]
  )


* ``PATH`` specifies the path of the folder that contains the tests,
  relative to the top level folder of the Examples Library.

* ``CHECK`` specifies the names of Dakota input files at PATH that
  Dakota should run with the '-check' option.

* ``RUN`` specifies the names of Dakota input files at PATH that
  should be run. Their console output will not be checked, only
  Dakota's returncode.

* ``REGRESS`` specifies the names of Dakota input files for which
  regression tests should be performed. The files, and corresponding
  .base files, must also be present at PATH.

* ``COMMAND`` specifies an arbitrary executable command to run. The
  returncode of the command will be examined to determine whether the
  test passes of fails.

* ``DEPENDS`` specifies a dependency of the test; the test only runs
  if the test it depends on passed. The argument is a test name, but
  it's often more convenient to use the ${_last_test_added} variable
  as an argument. Tests are added in the order that the keywords
  appear in, here. It may be necessary to make several separate calls
  to dakota_example_test (the calls will all have the same PATH) to
  properly express dependencies.

**Can I conditionally run tests?**

Yes. Here's an example from DakotaExamplesOfficial.cmake that
conditionally runs when Python is available and the system is UNIX::

  if(Python_EXECUTABLE AND UNIX)
    dakota_example_test(
      PATH contributed/sampling/lhs_basic_incremental
      COMMAND ${Python_EXECUTABLE} -V
      DEPENDS ${_last_test_added}
      )
  endif()

If your tests use optional Dakota features, you should protect them
using conditionals, e.g., ``HAVE_DOT``, ``HAVE_MUQ``.
