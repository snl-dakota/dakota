"""""""""""""""""""""""""""""""""""""""""""""""""""""""
Instructions for Modifying Dakota's Input Specification
"""""""""""""""""""""""""""""""""""""""""""""""""""""""

To modify Dakota's input specification (for maintenance or addition of new input syntax), specification maintenance mode must be enabled at Dakota configure time with the ``-DENABLE_SPEC_MAINT`` option, e.g.,

.. code-block::

   ./cmake -DENABLE_SPEC_MAINT:BOOL=ON ..

This will enable regeneration of NIDR and Dakota components which must be updated following a spec change.

=======================
XML Input Specification
=======================

The authoritative source for valid Dakota input grammar is ``dakota/src/dakota.xml``. The schema defining valid content for this XML file is in dakota/src/dakota.xsd. NIDR remains Dakota's user input file parser, so dakota.xml is translated to dakota/src/dakota.input.nspec during the Dakota build process. To update the XML input definition:

- Make sure ENABLE_SPEC_MAINT is enabled in your build and necessary Java development tools are installed (see below).
- Edit the XML spec in dakota.xml.
- Perform a make in dakota.build/src which will regenerate dakota.source/src/dakota.input.nspec and related file.
- Review that any changes induced in the dakota.input.nspec file are as expected.
- Proceed with verifying code changes and making downstream parse handler changes as normal (described below).
- Commit the modified dakota.xml, dakota.input.nspec, and other files generated to dakota.source/src along with your other code changes.

XML Build Requirements
----------------------

Editing the XML and then compiling Dakota requires

- Java Development Kit (JDK) providing the Java compiler javac. Java 6 (version 1.6) or newer should work, with Java 8 recommended. Can satisfy on RHEL6 with RPM packages java-1.8.0-openjdk-devel and java-1.8.0-openjdk. This is needed to build the Java-based XML to NIDR translator. If this becomes too burdensome, we can check in the generated xml2nidr.jar file.

XML Editing Tools
-----------------

The following tools will make editing dakota.input.xml easier.

- **Recommended: Eclipse Web Tools Platform.** Includes both graphical and text editors.

  1. Download Eclipse Standard (Classic)
  2. Configure proxy if needed, setting to manual: Window > Preferences > General > Network Connection > Proxy
  3. Install Web Tools Platform

    - Help > Install New Software
    - Work With: Kepler - http://download.eclipse.org/releases/kepler
    - Search "Eclipse X" and install two packages under Web, XML, Java

      - Eclipse XML Editors and Tools
      - Eclipse XSL Developer Tools
	  
    - Optionally install C/C++ Development Tools
  
  4. Optional: add Subclipse for subversion (Subversive is the other major competing tool and I don't think requires JavaHL) Help > Install New Software Work With: http://subclipse.tigris.org/update_1.6.x Install Subclipse On Linux: yum install subversion-javahl.x86_64
  5. Alternately install Eclipse for Java or Eclipse Java EE development which includes webtools, then optionally add subclipse and C/C++ dev
  
- **Alternate: Emacs or your usual editor.** For example, Emacs supports an Nxml mode. You can tell it where to find the schema, edit XML, and have it perform validation against the schema. See help at http://www.gnu.org/software/emacs/manual/html_mono/nxml-mode.html
- **Other Suggested Alternates:** XMLSpy, DreamWeaver, XML Copy Editor

XML Features (with map to NIDR)
-------------------------------

Out of necessity, Dakota XML dakota.xml closely mirrors dakota.input.nspec. Valid Dakota input grammar is constrained by dakota.xml, an XML document which must validate against dakota.xsd. The top-level element of interest is <input>, which is comprised of a sequence of content elements (keywords, alternates, etc.), which may themselves contain additional child content elements. The key content types are:

**Keyword ( <keyword>):**, specified with the <keyword> element whose definition is given by keywordType in dakota.xsd.

- The required attributes are:

  - **name:** The keyword name (lower case with underscores) as it will be given in user input; must follow same uniqueness rules are historical NIDR. User input is allowed in mixed case, but the XML must use lower case names.
    Since the NIDR parser allows keyword abbreviation, you must not add a keyword that could be misinterpreted as an abbreviation for a different keyword within the same top-level keyword, such as "environment" and "method". For example, adding the keyword "expansion" within the method specification would be a mistake if the keyword "expansion_factor" already was being used in this block.
    The NIDR input is somewhat order-dependent, allowing the same keyword to be reused multiple times in the specification. This often happens with aliases, such as lower_bounds, upper_bounds and initial_point. Ambiguities are resolved by attaching a keyword to the most recently seen context in which it could appear, if such exists, or to the first relevant context that subsequently comes along in the input file.
  - **code**: The verbatim NIDR handler to be invoked when this keyword parsed. In NIDR this was specified with {N_macro(...)}.

- Optional/useful parser-related elements/attributes in order of importance are:

  - **param sub-element:** Parameters and data types: A keyword may have an associated parameter element with a specified data type: <param type="PARAMTYPE" />. NIDR data types remain the same (INTEGER, REAL, STRING and LISTs thereof, but new data types INPUT_FILE and OUTPUT_FILE add convenience for the GUI, mapping to STRING for NIDR purposes. Parameters can also include attributes constraint, in_taglist, or taglist, which are used to help validate the user-specified parameter value. For example constraint >= 0 LEN normal_uncertain
  - **alias sub-element:** Historical aliases for this keyword (can appear multiple times). Alias has a single attribute name which must be lower case with underscores.
  - **id:** Unique ID for the keyword, usually name with an integer appended, but not currently used/enforced.
  - **minOccurs:** Minimum occurrences of the keyword in current context (set to 1 for required, 0 for optional)
  - **maxOccurs:** Maximum occurrences of the keyword in current context (for example environment may appear at most once)

- And optional/useful GUI-related attributes are:
  - **help:** (Don't add this attribute the new keywords!) A pointer to the corresponding reference manual section (deprecated as not needed with new reference manual format which mirrors keyword hierarchy).
  - **label:** a short, friendly label string for the keyword in the GUI. Format these like titles, e.g., "Initial Point for Search".
  - **group:** Category or group for this keyword, e.g., optimization vs. parameter study if they are to be groups for GUI purposes

**Alternation ( <oneOf>):** Alternation of groups of content is done with the element <oneOf> which indicates that its immediate children are alternates. In NIDR this was done with the pipe symbol: OptionA | OptionB. oneOf allows the label attribute and its use is recommended.

**Required Group ( <required>):** A required group can be specified by enclosing the contents in the <required> element. In NIDR this was done by enclosing the content in parentheses: ( required group... )

**Optional Group ( <optional>):** An optional group can be specified by enclosing the contents in the <optional> element. In NIDR this was done by enclosing the content in brackets: [ optional group... ]

=======================
Rebuild Generated Files
=======================

When configured with -DENABLE_SPEC_MAINT, performing a make in dakota.build/src will regenerate all files which derive from dakota.xml, include dakota.input.nspec, NIDR_keywds.hpp, and dakota.input.summary. If you commit changes to a source repository, be sure to commit any automatically generated files in addition to any modified in the following steps. It is not strictly necessary to run make at this point in the sequence, and in fact may generate errors if necessary handlers aren't yet available.

**Warning:** Please do not manually modify generated files!

==========================================
Update Parser Source NIDRProblemDescDB.cpp
==========================================

Many keywords have data associated with them: an integer, a floating-point number, a string, or arrays of such entities. Data requirements are specified in dakota.input.nspec by the tokens INTEGER, REAL, STRING, INTEGERLIST, REALLIST, STRINGLIST. (Some keywords have no associated data and hence no such token.) After each keyword and data token, the dakota.input.nspec file specifies functions that the NIDR parser should call to record the appearance of the keyword and deal with any associated data. The general form of this specification is

{ startfcn, startdata, stopfcn, stopdata }

i.e., a brace-enclosed list of one to four functions and data pointers, with trailing entities taken to be zero if not present; zero for a function means no function will be called. The startfcn must deal with any associated data. Otherwise, the distinction between startfcn and stopfcn is relevant only to keywords that begin a group of keywords (enclosed in parentheses or square brackets). The startfcn is called before other entities in the group are processed, and the stop function is called after they are processed. Top-level keywords often have both startfcn and stopfcn; stopfcn is uncommon but possible for lower-level keywords. The startdata and (if needed) stopdata values are usually pointers to little structures that provide keyword-specific details to generic functions for startfcn and stopfcn. Some keywords that begin groups (such as "approx_problem" within the top-level "environment" keyword) have no need of either a startfcn or a stopfcn; this is indicated by "{0}".

Most of the things within braces in dakota.input.nspec are invocations of macros defined in dakota.source/src/NIDRProblemDescDB.cpp. The macros simplify writing dakota.input.nspec and make it more readable. Most macro invocations refer to little structures defined in NIDRProblemDescDB.cpp, usually with the help of other macros, some of which have different definitions in different parts of NIDRProblemDescDB.cpp. When adding a keyword to dakota.input.nspec, you may need to add a structure definition or even introduce a new data type. NIDRProblemDescDB.cpp has sections corresponding to each top-level keyword. The top-level keywords are in alphabetical order, and most entities in the section for a top-level keyword are also in alphabetical order. While not required, it is probably good practice to maintain this structure, as it makes things easier to find.

Any integer, real, or string data associated with a keyword are provided to the keyword's startfcn, whose second argument is a pointer to a Values structure, defined in header file nidr.h.

**Example 1:** if you added the specification:

.. code-block::

    [method_setting REAL {method_setting_start, &method_setting_details} ]

you would provide a function

.. code-block:: cpp

    void NIDRProblemDescDB::
    method_setting_start(const char *keyname, Values *val, void **g, void *v)
    { ... }

in NIDRProblemDescDB.cpp. In this example, argument &method_setting_details would be passed as v, val->n (the number of values) would be 1 and \*val->r would be the REAL value given for the method_setting keyword. The method_setting_start function would suitably store this value with the help of method_setting_details.

For some top-level keywords, g (the third argument to the startfcn and stopfcn) provides access to a relevant context. For example, method_start (the startfcn for the top-level method keyword) executes

.. code-block:: cpp

   DataMethod *dm = new DataMethod;
    g = (void*)dm;

(and supplies a couple of default values to dm). The start functions for lower-level keywords within the method keyword get access to dm through their g arguments. Here is an example:


.. code-block:: cpp

   void NIDRProblemDescDB::
    method_str(const char *keyname, Values *val, void **g, void *v)
    {
            (*(DataMethod**)g)->**(String DataMethod::**)v = *val->s;
            }

In this example, v points to a pointer-to-member, and an assignment is made to one of the components of the DataMethod object pointed to by \*g. The corresponding stopfcn for the top-level method keyword is
 
.. code-block:: cpp

     void NIDRProblemDescDB::
    method_stop(const char *keyname, Values *val, void **g, void *v)
    {
            DataMethod *p = *(DataMethod**)g;
            pDDBInstance->dataMethodList.insert(*p);
            delete p;
            }

which copies the now populated DataMethod object to the right place and cleans up.

**Example 2:** if you added the specification

.. code-block::

    [method_setting REALLIST {{N_mdm(RealL,methodCoeffs)}

...then method_RealL (defined in NIDRProblemDescDB.cpp) would be called as the startfcn, and methodCoeffs would be the name of a (currently nonexistent) component of DataMethod. The N_mdm macro is defined in NIDRProblemDescDB.cpp; among other things, it turns RealL into NIDRProblemDescDB::method_RealL. This function is used to process lists of REAL values for several keywords. By looking at the source, you can see that the list values are val->r[i] for 0 <= i < val->n.

=================================
Update Corresponding Data Classes
=================================

The Data classes (DataEnvironment, DataMethod, DataModel, DataVariables, DataInterface, and DataResponses) store the parsed user input data. In this step, we extend the Data class definitions to include any new attributes referred to in dakota.xml or NIDRProblemDescDB

Update the Data Class Header File
---------------------------------

Add a new attribute to the public data for each of the new specifications. Follow the style guide for class attribute naming conventions (or mimic the existing code).

Update the .cpp File
--------------------

Define defaults for the new attributes in the constructor initialization list (if not a container with a sensible default constructor) in same order as they appear in the header. Add the new attributes to the write(MPIPackBuffer&), read(MPIUnpackBuffer&), and write(ostream&) functions, paying careful attention to the use of a consistent ordering.

========================================
Update Database Source ProblemDescDB.cpp
========================================

Augment/update get_<data_type>() Functions
------------------------------------------

The next update step involves extending the database retrieval functions in dakota.source/src/ProblemDescDB.cpp. These retrieval functions accept an identifier string and return a database attribute of a particular type, e.g., a RealVector:

.. code-block:: cpp

   const RealVector& get_rv(const String& entry_name);

The implementation of each of these functions contains tables of possible entry_name values and associated pointer-to-member values. There is one table for each relevant top-level keyword, with the top-level keyword omitted from the names in the table. Since binary search is used to look for names in these tables, each table must be kept in alphabetical order of its entry names. For example,

.. code-block:: cpp

    ...
    else if ((L = Begins(entry_name, "model."))) {
      if (dbRep->methodDBLocked)
          Locked_db();
      #define P &DataModelRep::
      static KW<RealVector, DataModelRep> RVdmo[] = {     // must be sorted
          {"nested.primary_response_mapping", P primaryRespCoeffs},
          {"nested.secondary_response_mapping", P secondaryRespCoeffs},
          {"surrogate.kriging_conmin_seed", P krigingConminSeed},
          {"surrogate.kriging_correlations", P krigingCorrelations},
          {"surrogate.kriging_max_correlations", P krigingMaxCorrelations},
          {"surrogate.kriging_min_correlations", P krigingMinCorrelations}};
      #undef P
      KW<RealVector, DataModelRep> *kw;
      if ((kw = (KW<RealVector, DataModelRep>*)Binsearch(RVdmo, L)))
          return dbRep->dataModelIter->dataModelRep->*kw->p;
    }

is the "model" portion of ProblemDescDB::get_rv(). Based on entry_name, it returns the relevant attribute from a DataModel object. Since there may be multiple model specifications, the dataModelIter list iterator identifies which node in the list of DataModel objects is used. In particular, dataModelList contains a list of all of the data_model objects, one for each time a top-level model keyword was seen by the parser. The particular model object used for the data retrieval is managed by dataModelIter, which is set in a set_db_list_nodes() operation that will not be described here.

There may be multiple DataMethod, DataModel, DataVariables, DataInterface, and/or DataResponses objects. However, only one specification is currently allowed so a list of DataEnvironment objects is not needed. Rather, ProblemDescDB::environmentSpec is the lone DataEnvironment object.

To augment the get_<data_type>() functions, add table entries with new identifier strings and pointer-to-member values that address the appropriate data attributes from the Data class object. The style for the identifier strings is a top-down hierarchical description, with specification levels separated by periods and words separated with underscores, e.g., "keyword.group_specification.individual_specification". Use the dbRep->listIter->attribute syntax for variables, interface, responses, and method specifications. For example, the method_setting example attribute would be added to get_drv() as:

.. code-block::

   {"method_name.method_setting", P methodSetting},

inserted at the beginning of the RVdmo array shown above (since the name in the existing first entry, i.e., "nested.primary_response_mapping", comes alphabetically after "method_name.method_setting").

===============================
Use get_<data_type>() Functions
===============================

At this point, the new specifications have been mapped through all of the database classes. The only remaining step is to retrieve the new data within the constructors of the classes that need it. This is done by invoking the get_<data_type>() function on the ProblemDescDB object using the identifier string you selected in Augment/update get_<data_type>() Functions. For example:

.. code-block:: cpp

    const String& interface_type = problem_db.get_string("interface.type");

passes the "interface.type" identifier string to the ProblemDescDB::get_string() retrieval function, which returns the desired attribute from the active DataInterface object.

**Warning:** Use of the get_<data_type>() functions is restricted to class constructors, since only in class constructors are the data list iterators (i.e., dataMethodIter, dataModelIter, dataVariablesIter, dataInterfaceIter, and dataResponsesIter) guaranteed to be set correctly. Outside of the constructors, the database list nodes will correspond to the last set operation, and may not return data from the desired list node.

========================
Update the Documentation
========================

Doxygen comments should be added to the Data class headers for the new attributes, and the reference manual sections describing the portions of dakota.xml that have been modified should be updated by updating files in dakota.source/docs/KeywordMetaData/. ddakota.xml, together with these metadata files generates the reference manual and GUI context-aware help documentation. 
