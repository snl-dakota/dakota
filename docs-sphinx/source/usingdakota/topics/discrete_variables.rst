Discrete Variables
==================

This page discusses discrete design, uncertain, and state variables
(which have ``discrete`` in their keyword name) as they have similar
specifications. These include:


-Integer ranges
-Sets of integers
-Sets of reals
-Sets of stringsand each is described below.

In addition, some aleatory uncertain variables, e.g.,
:ref:`variables-binomial_uncertain<variables-binomial_uncertain>`, are discrete integer-valued random
variables specified using parameters. These are described on their
individual keyword pages.

*Sets*

Sets of integers, reals, and strings have similar specifications,
though different value types.

The variables are specified using three keywords:

- Variable declaration keyword - specifies the number of variables being defined
- ``elements_per_variable`` - a list of positive integers specifying how many set members each variable admits  - Length = # of variables

- ``elements`` - a list of the permissible integer values in ALL sets, concatenated together.  - Length = sum of ``elements_per_variable``, or an integer multiple of number of variables
  - The order is very important here.
  - The list is partitioned according to the values of ``elements_per_variable``, and each partition is assigned to a variable.

- The ordering of ``elements_per_variable``, and the partitions of ``elements`` must match the strings from ``descriptors``

For string variables, each string element value must be quoted and may
contain alphanumeric, dash, underscore, and colon. White space, quote
characters, and backslash/metacharacters are not permitted.

Examples are given on the pages:

- discrete design set :ref:`variables-discrete_design_set-integer<variables-discrete_design_set-integer>`
- discrete design set :ref:`variables-discrete_design_set-real<variables-discrete_design_set-real>`
- discrete design set :ref:`variables-discrete_design_set-string<variables-discrete_design_set-string>`
- discrete uncertain set :ref:`variables-discrete_uncertain_set-integer<variables-discrete_uncertain_set-integer>`
- discrete uncertain set :ref:`variables-discrete_uncertain_set-real<variables-discrete_uncertain_set-real>`
- discrete uncertain set :ref:`variables-discrete_uncertain_set-string<variables-discrete_uncertain_set-string>`


*Range*

For discrete variables defined by range(s), the ``lower_bounds`` and ``upper_bounds`` restrict the permisible values.
For design variables, this constrains the feasible design space and is frequently used to prevent nonphysical designs.
This is a discrete interval variable that may take any integer value within bounds (e.g., [1, 4], allowing values of 1, 2, 3, or 4).
For some variable types, each variable is can be defined by multiple ranges.

Examples are given on the pages:

- :ref:`variables-discrete_interval_uncertain<variables-discrete_interval_uncertain>`