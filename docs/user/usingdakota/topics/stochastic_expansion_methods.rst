.. _`topic-stochastic_expansion_methods`:

Stochastic Expansion Methods
============================

The development of these techniques mirrors that of deterministic
finite element analysis utilizing the notions of projection,
orthogonality, and weak convergence :cite:p:`Gha99`, :cite:p:`Gha91`. Rather
than estimating point probabilities, they form an approximation to the
functional relationship between response functions and their random
inputs, which provides a more complete uncertainty representation for
use in multi-code simulations. Expansion methods include polynomial
chaos expansions (PCE), which employ multivariate orthogonal
polynomials that are tailored to representing particular input
probability distributions, and stochastic collocation (SC), which
employs multivariate interpolation polynomials.  For PCE, expansion
coefficients may be evaluated using a spectral projection approach
(based on sampling, tensor-product quadrature, Smolyak sparse grid, or
cubature methods for numerical integration) or a regression approach
(least squares or compressive sensing). For SC, interpolants are
formed over tensor-product or sparse grids and may be local or global,
value-based or gradient-enhanced, and nodal or hierarchical. In global
value-based cases (Lagrange polynomials), the barycentric formulation
is used :cite:p:`BerTref04,Klimke05,Higham04` to improve numerical
efficiency and stability.  Both sets of methods provide analytic
response moments and variance-based metrics; however, CDF/CCDF
probabilities are evaluated numerically by sampling on the expansion.
