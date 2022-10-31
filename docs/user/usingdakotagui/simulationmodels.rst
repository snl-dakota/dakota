Working with Simulation Models
==============================

Dakota's primary mechanism for iterating over a model is to contact an external simulation model through an exchange of parameters and responses.
Refer to the main article on :ref:`coupling Dakota to a simulation <couplingtosimulations-main>` for a detailed, general explanation.

In the context of Dakota GUI, there are two approaches for defining an external simulation model.

- :ref:`BMF <bmf-main>` - A BMF (Basic Model Format) file is a lightweight definition of parameters and responses. It
  is useful to create a BMF file in order to inform various parts of the GUI about your simulation model before you
  actually hook up to the external simulation model itself.
- :ref:`Next-Gen Workflow <ngw-main>` - Next-Gen Workflow is a powerful, node-based workflow tool that allows you to create an arbitrarily complex
  workflow that passes information from beginning to end. Dakota can use Next-Gen Workflow as the means by which it communicates with an
  external simulation model.