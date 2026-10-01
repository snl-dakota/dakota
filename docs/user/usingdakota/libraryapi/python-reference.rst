.. _library-api-python-reference:

Python API Reference
====================

This reference is generated from the ``dakota.study`` module in the
documentation build.  Factories controlled by optional Dakota capabilities
appear only when those capabilities are enabled.

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

