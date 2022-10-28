.. _ngw-node-pyprepro:

========
pyprepro
========

-----------
Description
-----------

PyPrePro is a Python script distributed with Dakota that allows you to perform pre-processing
by adding template syntax to a text-based file. PyPrePro performs many of the same functions
as APREPRO, and is optimized for use with Dakota parameters files that are presented in either
format (regular Dakota format, or APREPRO format).

Generally, PyPrepro template syntax is of the form ``{a=1.0}`` where everything between and
including the curly braces represents the templatized value. At runtime, PyPrePro replaces these
template expressions with live parameter values.

For further information, :ref:`refer to the main PyPrePro page <interfaces:dprepro-and-pyprepro>`.

----------------
Node Usage Notes
----------------

Pre-processor nodes may receive their input from one of three sources:

- Explicit, custom input ports. You can add custom input ports by dragging a connector line from
  a previous node to the blank space on the left side of your pre-processor node.
- Global workflow parameter nodes are also made available as input to pre-processor nodes, without
  the need to draw explicit connector lines. You may wish to make the relationship between
  global workflow parameters and your pre-processor node easier to visually understand by drawing
  explicit connector lines, but this is not necessary.
- Finally, if you have a node that generates a map of name/value pairs as its output,
  you may connect this to the inputParametersMap input port of your pre-processor node.

----------
Properties
----------

- **templateFile** - The templatized file to be provided to PyPrePro.
- **outputFile** - The destination file for PyPrePro to send its processed output to.
- **inline** - Use this field to change the "brace" characters used for PyPrePro expressions. The text in this
  field should take the form "OPENBRACE CLOSEBRACE", where OPENBRACE is the opening character, and
  CLOSEBRACE is the closing character, separated by a space. For example, you may put "[ ]" in thisfield to use square
  braces instead of the default curly braces.
- **pypreproPath** - The path to the PyPrePro Python script. In most cases, it is not necessary to manually locate the script.
  After you open the Settings Editor for the pyprepro node once, Next-Gen Workflow will try to
  automatically locate PyPrePro at a handful of default locations.

-----------
Input Ports
-----------

- **templateFile** - The templatized file to be provided to PyPrePro.
- **inputParametersMap** - A Map of text parameter labels and values. If this input port is connected, it overrides other sources of input parameters that would otherwise be sent to PyPrePro. In general, this input port canbe left blank.

------------
Output Ports
------------

- **outputFile** - A file reference to your processed input file.
- **exitStatus** - The number value exit code produced by the PyPrePro script. An exit code of 0 indicates success.
- **stdout** - pyprepro's stdout stream.
- **stderr** - pyprepro's stderr stream.
