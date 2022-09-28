..
   TODO: 
   % needed by pyrepro manual section, which was converted by
   % pandoc from markdown.
   \providecommand{\tightlist}{%
   \setlength{\itemsep}{0pt}\setlength{\parskip}{0pt}}

.. _`interfaces:main`:

Interfaces
==========

.. _`interfaces:overview`:

Overview
--------

The :dakkw:`interface` specification in a Dakota input file controls details
of function evaluations. The mechanisms currently in place for function
evaluations involve interfacing with one or more computational
simulation codes, computing :ref:`algebraic mappings <advint:algebraic>`,
or a combination of the two.

.. note::
   
   It is highly recommended that, at minimum, you are familiar with the
   :ref:`general strategy by which Dakota interfaces with external simulation models <couplingtosimulations-main>`
   before delving into this section.

..
   TODO:
   %In the case of use of an approximation in place of an expensive
   %simulation code, an \texttt{approximation} interface can be selected
   %to make use of surrogate modeling capabilities available within
   %Dakota.  Surrogate models are discussed further in Chapter~\ref{models}.

This section will focus on mechanisms for simulation code invocation,
starting with :ref:`interface types <interfaces:sim>` and
followed by a guide to :ref:`constructing simulation-based interfaces <interfaces:building>`.
This section also provides an overview of simulation interface components,
covers issues relating to file management, and presents a number of example data mappings.

.. toctree::
   :maxdepth: 1
   
   interfaces/simulationinterfaces
   interfaces/buildingblackboxinterface
   interfaces/simulationinterfacecomponents
   interfaces/simulationfilemanagement
   interfaces/batchedevaluations
   interfaces/mappingexamples
   interfaces/dakotainterfacing
