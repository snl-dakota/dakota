.. _`pydantic-docs`:

Pydantic Model Reference
========================

`Pydantic`_ models have replaced dakota.xml as the authoratative statement
of Dakota grammar. Ordinary users likely will find the Pydantic models
themselves to be of limited use.

.. _Pydantic: https://pydantic.dev/docs/validation/latest/get-started/

The Pydantic model documentation is arranged by module, which
corresponds roughly to top-level Dakota blocks (environment, method,
etc). The `DakotaStudy` model in the `Study` section defines the overall
organization of the input. The `Shared` section documents models
that are shared via inheritance by other models.

.. toctree::
   :maxdepth: 1

   pydanticref/study
   pydanticref/methods
   pydanticref/environment
   pydanticref/interface
   pydanticref/model
   pydanticref/responses
   pydanticref/variables
   pydanticref/shared

How to Work with the Pydantic Documentation
-------------------------------------------

When using the generated Pydantic documentation, think of each model as
representing a portion of the Dakota input hierarchy. Field names within
a model correspond to input keywords. Nested models represent nested blocks
of the input structure. Required vs optional fields, default values, and
allowed types are all defined there.

Constructing a Dakota input using the documentation would involve
starting from the top-level :ref:`DakotaStudy <pydantic-study>` model and
working down through the nested models.

In the documentation, fields are followed by a type. Some of the more
common ones are:

* a primitive type such as `int` or `str` if the keyword takes an argument
* the hyperlinked name of a model, which indicates that a field has children
* pipe-delimited models, indicating a Union or "choose one" relationship
* `Literal[True]`. These are leaf keywords and must be set to true.

Some fields have defaults, which follow an equal sign. Fields with defaults
are not required to be set. Others are followed by `| None`, which marks a
field that is optional but has no specified default.
