.. _ngw-node-toPlotlyJS:

==========
toPlotlyJS
==========

-----------
Description
-----------

Exports a Chartreuse plot model to plotly.js format, stored in an offline HTML file.

----------
Properties
----------

- **Destination**: The file destination to write the plotly.js data to. Always exports to HTML format, even if another file extension is provided in this field.
- **Plot width**: The width in pixels of the plot. If left blank, the width will be 600 pixels.
- **Plot height**: The height in pixels of the plot. If left blank, the height will be 600 pixels.

-----------
Input Ports
-----------

- **plotFileDataIn**: The serialized plot data to render. (Should ideally come from a plotFile output port from a previous Chartreuse node.)

------------
Output Ports
------------

- **htmlFile**: A reference to the saved HTML file.

-----
Notes
-----

Note that a peer file called ``plotly-latest.min.js`` will be written out next to your HTML file. This is a necessary dependency for viewing Plotly plots in a browser.
