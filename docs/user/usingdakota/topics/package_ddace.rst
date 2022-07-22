.. _`topic-package_ddace`:

Package: DDACE
==============

The Distributed Design and Analysis of Computer Experiments (DDACE)
library provides the following DACE techniques: grid sampling (
``grid``), pure random sampling ( ``,random``), orthogonal array sampling (
``oas``), latin hypercube sampling ( ``,lhs``), orthogonal array latin
hypercube sampling ( ``oa_lhs``), Box-Behnken ( ``,box_behnken``), and
central composite design ( ``central_composite``).

It is worth noting
that there is some overlap in sampling techniques with those available
from the nondeterministic branch. The current distinction is that the
nondeterministic branch methods are designed to sample within a
variety of probability distributions for uncertain variables, whereas
the design of experiments methods treat all variables as having
uniform distributions. As such, the design of experiments methods are
well-suited for performing parametric studies and for generating data
sets used in building global approximations,
but are not currently suited for assessing the effect of
uncertainties characterized with probability distribution.
If a design of experiments over both design/state
variables (treated as uniform) and uncertain variables (with
probability distributions) is desired, then ``sampling`` can
support this with ``active`` ``all`` specified in the Variables
specification block.