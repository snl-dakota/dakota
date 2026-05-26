.. _keyword-reference-area:

Keyword Reference
=================

This section provides reference information for all Dakota keywords. It is organized by the six types
of blocks that are permitted in Dakota input files: 

.. toctree::
   :maxdepth: 1
   
   reference/environment
   reference/method
   reference/model
   reference/variables
   reference/interface
   reference/responses

The purpose of these blocks and relationships between them are described on the :ref:`inputfile-main` page.
   
Keyword Pages
-------------

Every Dakota keyword has its own page in this manual. Each page contains the following
information about the keyword:

- A short blurb describing the keyword and its purpose in Dakota
- A `Specification` section that describes:
  
  - Whether the keyword accepts a literal argument, and, if so, its type
  - Any aliases for the keyword
  - The keyword's :ref:`JSON argument key <jsoninput:argumentkeys>`, if it has one

- A `Child Keywords` table which contains information about all of the keyword's children.
  For each child keyword, the table specifies:

   - Whether the child keyword is optional or required, or part of an exclusive (choose one)
     group
   - If it's part of a group, whether the group has a :ref:`JSON group key <jsoninput:groupkeys>`
   - The name of the keyword, and its blurb

- A longer `Description` of the keyword, which may include examples, HDF5 output, and other pertinent
  information.





