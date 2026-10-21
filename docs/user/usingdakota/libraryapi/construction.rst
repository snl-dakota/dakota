.. _library-api-construction:

Constructing a Study
====================

Users familiar with Dakota input files can think of the library APIs as
constructing the same study graph (method➔model➔variables, interface, responses)
from the leaves inward.  A typical study is assembled in this order:

#. Create a :py:class:`~dakota.study.Study`, which owns the runtime services
   used by every component in the study.
#. Construct variables, responses, and an interface.
#. Construct a model from those components.
#. Construct a method that operates on the model.
#. Pass the top-level method to :py:meth:`dakota.study.Study.run`.

The ``Study`` must remain alive while its components are being constructed or
used.  Components from unrelated studies should not be combined because their
runtime services may be incompatible.

Configuration
-------------

Each construction call accepts the configuration of one Dakota block, not a 
complete input file.  For example, the Python call

.. code-block:: python

   sampling = study.method.sampling(model, samples=10, seed=1234)

constructs a sampling method from an existing model instance and two
pieces of configuration information: the number of samples and the seed.
The equivalent C++ factory accepts this JSON fragment:

.. code-block:: cpp

   const nlohmann::json method = {
     {"samples", 10},
     {"seed", 1234}
   };
   auto sampling = study.method().sampling(method, model);

The :ref:`Keyword Reference <keyword-reference-area>` remains the authoritative
description of available options, required children, and exclusive groups.
Build a configuration by starting at the keyword represented by the factory and
following its child-keyword hierarchy.

Differences from freeform input
-------------------------------

Configuration objects follow the structured representation described for
:ref:`JSON input <jsoninput>`.  In particular:

* mutually exclusive freeform keyword groups may have explicit JSON group
  keys;
* an inline freeform argument may appear under a JSON argument key;
* leaf keywords use Boolean ``true``; and
* aliases, abbreviations, ``N*Value`` repetition, and ``L:S:U`` sequences are
  not accepted.

The keyword reference identifies JSON group and argument keys.  Defaults and
validation rules are shared with Dakota's generated input specification.

Python configuration forms
--------------------------

Python factories accept exactly one of three configuration forms.  The
following calls are equivalent:

.. code-block:: python

   from dakota.spec.method import SamplingConfig

   study.method.sampling(model, samples=10, seed=1234)                 # keyword arguments
   study.method.sampling(model, {"samples": 10, "seed": 1234})         # a dict
   study.method.sampling(model, SamplingConfig(samples=10, seed=1234)) # Pydantic model

A design principle of the factory interfaces is for components such as 
``model`` to come before configuration information. When a factory has optional
components that are not provided as arguments, the ``config`` keyword
argument can be used to provide configuration information. Do not combine 
``config`` with configuration keyword arguments.


Object relationships and recursion
----------------------------------

Freeform input uses pointer strings to connect methods, models, variables,
interfaces, and responses.  In the library APIs, the corresponding objects are
passed directly to factory calls.  A simulation model, for example, receives
its ``Variables``, ``Interface``, and ``Response`` objects.

Construct recursive studies from the innermost component outward.  Create a
sub-method before a nested model or meta-iterator that consumes it, and create
truth and approximation models before a surrogate that combines them.  Some
configuration models retain pointer fields because the input grammar is
shared; those fields are configuration metadata and do not replace the
injected objects.

