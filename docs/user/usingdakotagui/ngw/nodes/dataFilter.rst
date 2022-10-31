.. _ngw-node-dataFilter:

==========
dataFilter
==========

-----------
Description
-----------

Allows you to define a "filter data structure" that can be passed to other nodes that understand
how to read the data structure.  If provided to downstream nodes, those nodes will read the filter
data structure and use that information to filter on their own data.

Because this is still an experimental capability, most other NGW nodes do not know how to read the
filter data structure from this node.

----------
Properties
----------

- **Select variable/response source file**: A file selector that allows you to import a set of variable and response labels from an existing file.
- **Add New Filter**: Add a custom filter label.
- **Remove Filter**: Remove a filter label.
- **Filter table**: A table that shows the names and types of data elements that will subsequently be filtered.
- **Invert filter**: If checked, this node will tell subsequent workflow nodes to filter everything
  EXCEPT what is shown in the filter table.  If unchecked, this node will tell subsequent workflow
  nodes to only filter out what is shown in the filter table.

-----------
Input Ports
----------- 

No input ports for this node.

------------
Output Ports
------------       
      
- **dataFilter**: A data structure that contains all the information about what needs to be filtered.