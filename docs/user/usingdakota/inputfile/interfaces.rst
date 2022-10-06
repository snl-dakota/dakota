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

The :dakkw:`interface` specification in a Dakota input file controls all the details
of **function evaluations**. The mechanisms currently in place for function
evaluations involve interfacing with :ref:`one or more computational simulation codes <interfaces:sim>`,
computing :ref:`algebraic mappings <advint:algebraic>`, or a combination of the two.

It is highly recommended that, at minimum, you are familiar with the
:ref:`general strategy by which Dakota interfaces with external simulation models <couplingtosimulations-main>`
before delving into this section.

..
   TODO:
   %In the case of use of an approximation in place of an expensive
   %simulation code, an \texttt{approximation} interface can be selected
   %to make use of surrogate modeling capabilities available within
   %Dakota.  Surrogate models are discussed further in Chapter~\ref{models}.

.. toctree::
   :maxdepth: 1
   
   interfaces/simulationinterfaces
   interfaces/buildingblackboxinterface
   interfaces/simulationinterfacecomponents
   interfaces/simulationfilemanagement
   interfaces/batchedevaluations
   interfaces/mappingexamples
   interfaces/dakotainterfacing
