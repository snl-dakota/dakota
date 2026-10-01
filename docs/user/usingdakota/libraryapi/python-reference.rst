.. _library-api-python-reference:

Python API Reference
====================

This reference is generated from the ``dakota.study`` module in the
documentation build.  Factories controlled by optional Dakota capabilities
appear only when those capabilities are enabled.

How to use this reference
-------------------------

A :class:`dakota.study.Study` owns the runtime services used by every object
created from it. Build a study from the leaves inward: create variables,
responses, and an interface; inject those objects into a model factory; inject
the model into a method factory; and finally pass the method to
:meth:`dakota.study.Study.run`. Keep the study alive as long as any of its
components are in use, and do not mix components from different studies.

Factory signatures distinguish two kinds of input. Parameters typed as Study
API objects are *dependencies*: pass the actual object rather than an
input-file pointer name. The remaining Dakota options form a configuration
fragment relative to the selected factory. They may be supplied as keyword
arguments, as one dictionary passed through ``config``, or as the corresponding
Pydantic configuration model. Do not combine ``config`` with configuration
keyword arguments.

The generated :ref:`keyword reference <keyword-reference-area>` remains the
authoritative description of individual configuration options. Each factory
below links to the relevant keyword subtree. Pointer fields required by the
input-file schema are automatically represented by internal sentinels in API
mode when the dependency has already been injected as an object.

API map
-------

.. list-table::
   :header-rows: 1
   :widths: 19 25 28 28

   * - Task
     - Entry point
     - Result
     - Notes
   * - Establish runtime ownership
     - :class:`dakota.study.Study`
     - A study with model and method factories
     - A Study object provides an execution context for a single study. Instantiate only
       one Study at a time.
   * - Define Model sub-components
     - ``Study.variables``, ``Study.responses``, ``Study.interface``
     - :class:`dakota.study.Variables`, :class:`dakota.study.Response`, and
       :class:`dakota.study.Interface`
     - Not every Model requires an interface
   * - Assemble a model
     - :class:`dakota.study.ModelFactory`
     - A :class:`dakota.study.Model` specialization
     - :class:`ModelFactory <dakota.study.ModelFactory>` should not be directly instantiated. The
       :attr:`model <dakota.study.Study.model>` member variable of :class:`Study <dakota.study.Study>`
       objects are a ModelFactory.
       Some Models have method/iterator or model sub-components
   * - Select an algorithm
     - :class:`dakota.study.MethodFactory`
     - An :class:`dakota.study.Iterator` specialization
     - :class:`MethodFactory <dakota.study.MethodFactory>` should not be directly instantiated. The
       :attr:`method <dakota.study.Study.method>` member variable of Study objects are a Methodactory.
   * - Execute the study
     - :meth:`dakota.study.Study.run`
     - Results retained by the returned iterator handle
     - None

Study and configuration
-----------------------

.. autoclass:: dakota.study.Study
   :members:
   :undoc-members:

.. autoclass:: dakota.study.StudyConfig
   :members:
   :undoc-members:

.. autoclass:: dakota.study.StudyOutputConfig
   :members:
   :undoc-members:

.. autoclass:: dakota.study.StudyRunConfig
   :members:
   :undoc-members:

Factories
---------

``ModelFactory`` and ``MethodFactory`` support the implementation of
:class:`dakota.study.Study`; users ordinarily should not instantiate them
directly. Access the factory instances owned by a study through
:attr:`dakota.study.Study.model` and :attr:`dakota.study.Study.method`. This
ensures that constructed models and methods use the correct runtime services
and ownership context.

.. autoclass:: dakota.study.ModelFactory
   :members:
   :undoc-members:

.. autoclass:: dakota.study.MethodFactory
   :members:
   :undoc-members:

Component handles
-----------------

These objects are returned by ``Study`` and its factories.  They cannot be
constructed independently through the public Python API.

.. autoclass:: dakota.study.Variables
   :members:

.. autoclass:: dakota.study.Response
   :members:

.. autoclass:: dakota.study.Interface
   :members:

.. autoclass:: dakota.study.Model
   :members:

.. autoclass:: dakota.study.Iterator
   :members:

.. autoclass:: dakota.study.NonDLHSSampling
   :members:
