.. _keyword-reference-area:

Keyword Reference
=======================================

This page summarizes the overall input file structure, syntax, and the six types of blocks that may appear in Dakota input. Some are optional and some may appear multiple times:

.. toctree::
   :maxdepth: 1
   
   reference/environment
   reference/method
   reference/model
   reference/variables
   reference/interface
   reference/responses
   
**Introduction to Dakota Keywords**

In Dakota, the environment manages execution modes and I/O streams and defines the top-level iterator. Generally speaking, an iterator contains a model and a model contains a set of variables, an interface, and a set of responses. An iterator repeatedly operates on the model to map the variables into responses using the interface. Each of these six components (environment, method, model, variables, interface, and responses) are separate specifications in the user's input file, and as a whole, determine the study to be performed during an execution of the Dakota software.

A Dakota execution is limited to a single environment, but may involve multiple methods and multiple models. In particular, advanced iterators (i.e., meta- and component-based iterators) and advanced models (i.e., nested and surrogate models) may specialize to include recursions with additional sub-iterators and sub-models. Since each model may contain its own variables, interface, and responses, there may be multiple specifications of the method, model, variables, interface, and responses sections.

**Keyword Pages**

Every Dakota keyword has its own page in this manual. The page describes:

- Whether the keyword takes ARGUMENTS, and the data type Additional notes about ARGUMENTS can be found here: Specifying Arguments.
- Whether it has an ALIAS
- Which additional keywords can be specified to change its behavior
- Which of these additional keywords are required or optional
- Additional information about how to use the keyword in an input file
