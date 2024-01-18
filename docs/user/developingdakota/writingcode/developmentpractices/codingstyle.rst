"""""""""""""""""""""""""""""""""""""""
Coding Style Guidelines and Conventions
"""""""""""""""""""""""""""""""""""""""

============
Introduction
============

Common code development practices can be extremely useful in multiple developer environments. Particular styles for code components lead to improved readability of the code and can provide important visual cues to other developers. Much of this recommended practices document is borrowed from the CUBIT mesh generation project, which in turn borrows its recommended practices from other projects, yielding some consistency across Sandia projects. While not strict requirements, these guidelines suggest a best-practices starting point for coding in Dakota.

======================
C++/C Style Guidelines
======================

Style guidelines involve the ability to discern at a glance the type and scope of a variable or function.

Class and variable styles
-------------------------

Class names should be composed of two or more descriptive words, with the first character of each word capitalized, e.g.:

.. code-block:: cpp

   class ClassName;

Class member variables should be composed of two or more descriptive words, with the first character of the second and succeeding words capitalized, e.g.:

.. code-block:: cpp

   double classMemberVariable;

Temporary (i.e. local) variables are lower case, with underscores separating words in a multiple word temporary variable, e.g.:

.. code-block:: cpp

   int temporary_variable;

Constants (i.e. parameters) and enumeration values are upper case, with underscores separating words, e.g.:

.. code-block:: cpp

   const double CONSTANT_VALUE;

Function styles
---------------

Function names are lower case, with underscores separating words, e.g.:

.. code-block:: cpp

   int function_name();

There is no need to distinguish between member and non-member functions by style, as this distinction is usually clear by context. This style convention allows member function names which set and return the value of a similarly-named private member variable, e.g.:

.. code-block:: cpp

    int memberVariable;
    void member_variable(int a) { // set
      memberVariable = a;
    }
    int member_variable() const { // get
      return memberVariable;
    }

In cases where the data to be set or returned is more than a few bytes, it is highly desirable to employ const references to avoid unnecessary copying, e.g.:

.. code-block:: cpp

    void continuous_variables(const RealVector& c_vars) { // set
      continuousVariables = c_vars;
    }
    const RealVector& continuous_variables() const {      // get
      return continuousVariables;
    }

Note that it is not necessary to always accept the returned data as a const reference. If it is desired to be able change this data, then accepting the result as a new variable will generate a copy, e.g.:

.. code-block:: cpp

    // reference to continuousVariables cannot be changed
    const RealVector& c_vars = model.continuous_variables();
    // local copy of continuousVariables can be changed
    RealVector c_vars = model.continuous_variables();
    
Miscellaneous
-------------

Appearance of typedefs to redefine or alias basic types is isolated to a few header files (``data_types.h``, ``template_defs.h``), so that issues like program precision can be changed by changing a few lines of typedefs rather than many lines of code, e.g.:

.. code-block:: cpp

    typedef double Real;

``xemacs`` is the preferred source code editor, as it has C++ modes for enhancing readability through color (turn on "Syntax highlighting"). Other helpful features include "Paren highlighting" for matching parentheses and the "New Frame" utility to have more than one window operating on the same set of files (note that this is still the same edit session, so all windows are synchronized with each other). Window width should be set to 80 internal columns, which can be accomplished by manual resizing, or preferably, using the following alias in your shell resource file (e.g., .cshrc):

.. code-block::

    alias xemacs "xemacs -g 81x63"

where an external width of 81 gives 80 columns internal to the window and the desired height of the window will vary depending on monitor size. This window width imposes a coding standard since you should avoid line wrapping by continuing anything over 80 columns onto the next line.

Indenting increments are 2 spaces per indent and comments are aligned with the code they describe, e.g.:

.. code-block:: cpp

    void abort_handler(int code)
    {
      int initialized = 0;
      MPI_Initialized(&initialized);
      if (initialized) {
        // comment aligned to block it describes
        int size;
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        if (size>1)
          MPI_Abort(MPI_COMM_WORLD, code);
        else
          exit(code);
      }
      else
        exit(code);
    }

Also, the continuation of a long command is indented 2 spaces, e.g.:

.. code-block:: cpp

    const String& iterator_scheduling
      = problem_db.get_string("strategy.iterator_scheduling");

and similar lines are aligned for readability, e.g.:

.. code-block:: cpp

    cout << "Numerical gradients using " << finiteDiffStepSize*100. << "%"
         << finiteDiffType << " differences\nto be calculated by the "
         << methodSource << " finite difference routine." << endl;

Lastly, #ifdef's are not indented (to make use of syntax highlighting in xemacs).

=======================
File Naming Conventions
=======================

In addition to the style outlined above, the following file naming conventions have been established for the Dakota project.

File names for C++ classes should, in general, use the same name as the class defined by the file. Exceptions include:

 - with the introduction of the Dakota namespace, base classes which previously utilized prepended Dakota identifiers can now safely omit the identifiers. However, since file names do not have namespace protection from name collisions, they retain the prepended Dakota identifier. For example, a class previously named DakotaModel which resided in DakotaModel.cpp/hpp, is now Dakota::Model (class Model in namespace Dakota) residing in the same filenames. The retention of the previous filenames reduces the possibility of multiple instances of a Model.hpp causing problems. Derived classes (e.g., NestedModel) do not require a prepended Dakota identifier for either the class or file names.
 - in a few cases, it is convenient to maintain several closely related classes in a single file, in which case the file name may reflect the top level class or some generalization of the set of classes (e.g., DakotaResponse.[CH] files contain Dakota::Response and Dakota::ResponseRep classes, and DakotaBinStream.[CH] files contain the Dakota::BiStream and Dakota::BoStream classes).

The type of file is determined by one of the four file name extensions listed below:

 - **.hpp** A class header file ends in the suffix .hpp. The header file provides the class declaration. This file does not contain code for implementing the methods, except for the case of inline functions. Inline functions are to be placed at the bottom of the file with the keyword inline preceding the function name.
 - **.cpp** A class implementation file ends in the suffix .cpp. An implementation file contains the definitions of the members of the class.
 - **.h** A header file ends in the suffix .h. The header file contains information usually associated with procedures. Defined constants, data structures and function prototypes are typical elements of this file.
 - **.c** A procedure file ends in the suffix .c. The procedure file contains the actual procedures.

===============================
Class Documentation Conventions
===============================

Class documentation uses the doxygen tool available from http://www.doxygen.org and employs the JAVA-doc comment style. Brief comments appear in header files next to the attribute or function declaration. Detailed descriptions for functions should appear alongside their implementations (i.e., in the .cpp files for non-inlined, or in the headers next to the function definition for inlined). Detailed comments for a class or a class attribute must go in the header file as this is the only option.

NOTE: Previous class documentation utilities (class2frame and class2html) used the "//-" comment style and comment blocks such as this:

.. code-block::

    //- Class:       Model
    //- Description: The model to be iterated by the Iterator.  
    //-              Contains Variables, Interface, and Response objects.
    //- Owner:       Mike Eldred
    //- Version: $Id: Dev_Recomm_Pract.dox 4549 2007-09-20 18:25:03Z mseldre $

These tools are no longer used, so remaining comment blocks of this type are informational only and will not appear in the documentation generated by doxygen.

======================
CMake Style Guidelines
======================

Dakota conventions for CMake files, such as CMakeLists.txt, FooConfig.cmake, etc., follow. Our goal is ease of reading, maintenance, and support, similar to the C++ code itself. Current CMake versions and build hints are maintained at the Developer Portal http://dakota.sandia.gov/developer/.

CMake Code Formatting
---------------------

 - Indentation is 2 spaces, consistent with Dakota C++ style.
 - Lines should be kept to less than 80 chars per line where possible.
 - Wrapped lines may be indented two spaces or aligned with prior lines.
 - For ease of viewing and correctness checking in Emacs, a customization file is available: http://www.cmake.org/CMakeDocs/cmake-mode.el

CMake Variable Naming Conventions
---------------------------------

These variable naming conventions are especially important for those that ultimately become preprocessor defines and affect compilation of source files.

- Classic/core elements of the CMake language are set in lower_case, e.g., option, set, if, find_library.
- Static arguments to CMake functions and macros are set in UPPER_CASE, e.g. REQUIRED, NO_MODULE, QUIET.
- Minimize "global" variables, i.e., don't use 2 variables with the same meaning when one will do the job.
- Feature toggling: when possible, use the "HAVE_<pkg/feature>" convention already in use by many CMake-enabled TPLs, e.g.,
    
.. code-block::

    $ grep HAVE_SYSTEM Dakota/src/CMakeLists.txt
    check_function_exists(system HAVE_SYSTEM)
    if(HAVE_SYSTEM)
      add_definitions("-DHAVE_SYSTEM")
    endif(HAVE_SYSTEM)
    $ grep HAVE_CONMIN Dakota/src/CMakeLists.txt Dakota/packages/CMakeLists.txt
    Dakota/src/CMakeLists.txt:if(HAVE_CONMIN)
    Dakota/src/CMakeLists.txt:endif(HAVE_CONMIN)
    Dakota/packages/CMakeLists.txt:option(HAVE_CONMIN "Build the CONMIN package." ON)
    Dakota/packages/CMakeLists.txt:if(HAVE_CONMIN)
    Dakota/packages/CMakeLists.txt:endif(HAVE_CONMIN)

- When a variable/preprocessor macro could result in name clashes beyond Dakota scope, e.g., for library_mode users, consider prefixing the "HAVE_<pkg>" name with DAKOTA\_, e.g. DAKOTA_HAVE_MPI. Currently, MPI is the only use case for such a variable in Dakota, but many examples can be found in the CMake Modules source, e.g.
 
 .. code-block::

    grep _HAVE_ <cmake_prefix_dir>/share/cmake-2.8/Modules/*
