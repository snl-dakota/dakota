.. _library-api-tutorial:

Sampling Study Tutorial
=======================

This example builds a Latin hypercube sampling study with two uniform
uncertain variables and the Dakota ``text_book`` analysis driver.  The Python
and C++ versions assemble the same study graph.

Python
------

The Python version uses configuration keyword arguments.  This is usually the
most readable form when translating a small freeform input.

.. literalinclude:: ../../../../python/dakota/study/di_construction_demo_kwargs.py
   :language: python
   :start-after: [docs-library-api-start]
   :end-before: [docs-library-api-end]
   :dedent: 4

The call to ``sampling`` passes its top-level configuration fields as keyword
arguments. Dictionaries are still used for nested keyword groups: for example,
``sample_type={"lhs": True}`` represents the freeform ``sample_type lhs``
choice. The keyword-argument and Pydantic forms for that call are:

.. code-block:: python

   from dakota.spec.method import SamplingConfig

   sampling = study.method.sampling(
       model,
       sample_type={"lhs": True},
       samples=10,
       seed=1234,
   )

   sampling = study.method.sampling(
       model,
       config=SamplingConfig(
           sample_type={"lhs": True},
           samples=10,
           seed=1234,
       ),
   )

C++
---

The C++ API accepts JSON configuration fragments and typed study-level output
configuration.  Dependencies are passed as C++ objects and smart pointers.

.. literalinclude:: ../../../../src/di_construction_demo.cpp
   :language: cpp
   :start-after: [docs-library-api-start]
   :end-before: [docs-library-api-end]

Both examples keep the ``Study`` alive through execution.  The study owns the
runtime services shared by the variables, interface, model, and iterator.

Running the examples
--------------------

The examples require a Dakota build containing the Study API and the
``text_book`` analysis driver.  Run them from a writable directory because the
configured interface and output settings create files there.  For Python, put
the build-tree or installed Dakota Python package on ``PYTHONPATH``.
