.. _ngw-node-qoiExtractor:

============
qoiExtractor
============

-----------
Description
-----------

This node allows you to define "quantity of interest extractors" (QOI extractors, from this point on)
which are simple, human-readable regular expressions that allow you to extract values from
unknown text sources. For instance, suppose you want to take a specific value from a log file. A log
file can be of varying length, so we can't use absolute positioning to get the value. But a QOI extractor
will do the trick. 

Refer to the :ref:`main documentation on QOIs <qoi-main>` for more information.

----------
Properties
----------

No properties.

--------
Controls
--------

This node's settings editor contains a table displaying the QOI extractors you have defined so far.

- **Add QOI Extractor**: opens a dialog allowing you to define a new QOI extractor.
- **Edit QOI Extractor**: opens a dialog allowing you to edit a previously defined QOI extractor.
- **Remove QOI Extractor**: deletes a previously defined QOI extractor.

-----------
Input Ports
-----------

- **inputText**: the stream of text to extract QOIs from.

------------
Output Ports
------------

- **qoiMap**: A map data structure containing key-value pairs, where the key is the name of your QOI extractor,and the value is what was extracted from the text stream based on the expression you defined.

-----
Notes
-----

Note that individual output ports are created for you each time you create a new QOI extractor. This is providedso that you can pass out individual extracted QOIs instead of using the qoiMap, if you prefer.