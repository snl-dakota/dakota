.. _`Chap:ActSub`:

Active Subspace Models
======================

The idea behind active subspaces is to find directions in the input
variable space in which the quantity of interest is nearly constant.
After rotation of the input variables, this method can allow significant
dimension reduction. Below is a brief summary of the process.

#. Compute the gradient of the quantity of interest,
   :math:`q = f(\mathbf{x})`, at several locations sampled from the full
   input space,

   .. math:: \nabla_{\mathbf{x}} f_i = \nabla f(\mathbf{x}_i).

#. Compute the eigendecomposition of the matrix
   :math:`\hat{\mathbf{C}}`,

   .. math:: \hat{\mathbf{C}} = \frac{1}{M}\sum_{i=1}^{M}\nabla_{\mathbf{x}} f_i\nabla_{\mathbf{x}} f_i^T = \hat{\mathbf{W}}\hat{\mathbf{\Lambda}}\hat{\mathbf{W}}^T,

   where :math:`\hat{\mathbf{W}}` has eigenvectors as columns,
   :math:`\hat{\mathbf{\Lambda}} = \text{diag}(\hat{\lambda}_1,\:\ldots\:,\hat{\lambda}_N)`
   contains eigenvalues, and :math:`N` is the total number of
   parameters.

#. Using a truncation method or specifying a dimension to estimate the
   active subspace size, split the eigenvectors into active and inactive
   directions,

   .. math:: \hat{\mathbf{W}} = \left[\hat{\mathbf{W}}_1\quad\hat{\mathbf{W}}_2\right].

   These eigenvectors are used to rotate the input variables.

#. Next the input variables, :math:`\mathbf{x}`, are expanded in terms
   of active and inactive variables,

   .. math:: \mathbf{x} = \hat{\mathbf{W}}_1\mathbf{y} + \hat{\mathbf{W}}_2\mathbf{z}.

#. A surrogate is then built as a function of the active variables,

   .. math:: g(\mathbf{y}) \approx f(\mathbf{x})

As a concrete example, consider the
function: :raw-latex:`\cite{constantine2015active}`

.. math:: f(x) = \exp\left(0.7x_1 + 0.3x_2\right).

Figure `[fig:activesubspace] <#fig:activesubspace>`__\ (a) is a contour
plot of :math:`f(x)`. The black arrows indicate the eigenvectors of the
matrix :math:`\hat{\mathbf{C}}`. Figure
`[fig:activesubspace] <#fig:activesubspace>`__\ (b) is the same function
but rotated so that the axes are aligned with the eigenvectors. We
arbitrarily give these rotated axes the labels :math:`y_1` and
:math:`y_2`. From
fig. `[fig:activesubspace] <#fig:activesubspace>`__\ (b) it is clear
that all of the variation is along :math:`y_1` and the dimension of the
rotated input space can be reduced to 1.

.. container:: subfigmatrix

   2

For additional information, see
references :raw-latex:`\cite{Constantine-preprint-active,constantine2014active,constantine2015active}`.

.. _`Sec:trunc`:

Truncation Methods
------------------

Once the eigenvectors of :math:`\hat{\mathbf{C}}` are obtained we must
decide how many directions to keep. If the exact subspace size is known
*a priori* it can be specified. Otherwise there are three automatic
active subspace detection and truncation methods implemented:

-  Constantine metric (default),

-  Bing Li metric,

-  and Energy metric.

.. _`SubSec:constantine`:

Constantine metric
~~~~~~~~~~~~~~~~~~

The Constantine metric uses a criterion based on the variability of the
subspace estimate. Eigenvectors are computed for bootstrap samples of
the gradient matrix. The subspace size associated with the minimum
distance between bootstrap eigenvectors and the nominal eigenvectors is
the estimated active subspace size.

Below is a brief outline of the Constantine method of active subspace
identification. The first two steps are common to all active subspace
truncation methods.

#. Compute the gradient of the quantity of interest,
   :math:`q = f(\mathbf{x})`, at several locations sampled from the
   input space,

   .. math:: \nabla_{\mathbf{x}} f_i = \nabla f(\mathbf{x}_i).

#. Compute the eigendecomposition of the matrix
   :math:`\hat{\mathbf{C}}`,

   .. math:: \hat{\mathbf{C}} = \frac{1}{M}\sum_{i=1}^{M}\nabla_{\mathbf{x}} f_i\nabla_{\mathbf{x}} f_i^T = \hat{\mathbf{W}}\hat{\mathbf{\Lambda}}\hat{\mathbf{W}}^T,

   where :math:`\hat{\mathbf{W}}` has eigenvectors as columns,
   :math:`\hat{\mathbf{\Lambda}} = \text{diag}(\hat{\lambda}_1,\:\ldots\:,\hat{\lambda}_N)`
   contains eigenvalues, and :math:`N` is the total number of
   parameters.

#. Use bootstrap sampling of the gradients found in step 1 to compute
   replicate eigendecompositions,

   .. math:: \hat{\mathbf{C}}_j^* = \hat{\mathbf{W}}_j^*\hat{\mathbf{\Lambda}}_j^*\left(\hat{\mathbf{W}}_j^*\right)^T.

#. Compute the average distance between nominal and bootstrap subspaces,

   .. math:: e^*_n = \frac{1}{M_{boot}}\sum_j^{M_{boot}} \text{dist}(\text{ran}(\hat{\mathbf{W}}_n), \text{ran}(\hat{\mathbf{W}}_{j,n}^*)) = \frac{1}{M_{boot}}\sum_j^{M_{boot}} \left\| \hat{\mathbf{W}}_n\hat{\mathbf{W}}_n^T - \hat{\mathbf{W}}_{j,n}^*\left(\hat{\mathbf{W}}_{j,n}^*\right)^T\right\|,

   where :math:`M_{boot}` is the number of bootstrap samples,
   :math:`\hat{\mathbf{W}}_n` and :math:`\hat{\mathbf{W}}_{j,n}^*` both
   contain only the first :math:`n` eigenvectors, and :math:`n < N`.

#. The estimated subspace rank, :math:`r`, is then,

   .. math:: r = \operatorname*{arg\,min}_n \, e^*_n.

For additional information, see
Ref. :raw-latex:`\cite{constantine2015active}`.

.. _`SubSec:bingli`:

Bing Li metric
~~~~~~~~~~~~~~

The Bing Li metric uses a trade-off criterion to determine where to
truncate the active subspace. The criterion is a function of the
eigenvalues and eigenvectors of the active subspace gradient matrix.
This function compares the decrease in eigenvalue amplitude with the
increase in eigenvector variability under bootstrap sampling of the
gradient matrix. The active subspace size is taken to be the index of
the first minimum of this quantity.

Below is a brief outline of the Bing Li method of active subspace
identification. The first two steps are common to all active subspace
truncation methods.

#. Compute the gradient of the quantity of interest,
   :math:`q = f(\mathbf{x})`, at several locations sampled from the
   input space,

   .. math:: \nabla_{\mathbf{x}} f_i = \nabla f(\mathbf{x}_i).

#. Compute the eigendecomposition of the matrix
   :math:`\hat{\mathbf{C}}`,

   .. math:: \hat{\mathbf{C}} = \frac{1}{M}\sum_{i=1}^{M}\nabla_{\mathbf{x}} f_i\nabla_{\mathbf{x}} f_i^T = \hat{\mathbf{W}}\hat{\mathbf{\Lambda}}\hat{\mathbf{W}}^T,

   where :math:`\hat{\mathbf{W}}` has eigenvectors as columns,
   :math:`\hat{\mathbf{\Lambda}} = \text{diag}(\hat{\lambda}_1,\:\ldots\:,\hat{\lambda}_N)`
   contains eigenvalues, and :math:`N` is the total number of
   parameters.

#. Normalize the eigenvalues,

   .. math:: \lambda_i = \frac{\hat{\lambda}_i}{\sum_j^N \hat{\lambda}_j}.

#. Use bootstrap sampling of the gradients found in step 1 to compute
   replicate eigendecompositions,

   .. math:: \hat{\mathbf{C}}_j^* = \hat{\mathbf{W}}_j^*\hat{\mathbf{\Lambda}}_j^*\left(\hat{\mathbf{W}}_j^*\right)^T.

#. Compute variability of eigenvectors,

   .. math:: f_i^0 = \frac{1}{M_{boot}}\sum_j^{M_{boot}}\left\lbrace 1 - \left\vert\text{det}\left(\hat{\mathbf{W}}_i^T\hat{\mathbf{W}}_{j,i}^*\right)\right\vert\right\rbrace ,

   where :math:`\hat{\mathbf{W}}_i` and :math:`\hat{\mathbf{W}}_{j,i}^*`
   both contain only the first :math:`i` eigenvectors and
   :math:`M_{boot}` is the number of bootstrap samples. The value of the
   variability at the first index, :math:`f_1^0`, is defined as zero.

#. Normalize the eigenvector variability,

   .. math:: f_i = \frac{f_i^0}{\sum_j^N f_j^0}.

#. The criterion, :math:`g_i`, is defined as,

   .. math:: g_i = \lambda_i + f_i.

#. The index of first minimum of :math:`g_i` is then the estimated
   active subspace rank.

For additional information, see Ref. :raw-latex:`\cite{bing-li}`.

.. _`SubSec:energy`:

Energy metric
~~~~~~~~~~~~~

The energy metric truncation method uses a criterion based on the
derivative matrix eigenvalue energy. The user can specify the maximum
percentage (as a decimal) of the eigenvalue energy that is not captured
by the active subspace represenation.

Using the eigenvalue energy truncation metric, the subspace size is
determined using the following equation:

.. math:: n = \inf \left\lbrace d \in \mathbb{Z} \quad\middle|\quad 1 \le d \le N \quad \wedge\quad 1 - \frac{\sum_{i = 1}^{d} \lambda_i}{\sum_{i = 1}^{N} \lambda_i} \,<\, \epsilon \right\rbrace

where :math:`\epsilon` is the ``truncation_tolerance``, :math:`n` is the
estimated subspace size, :math:`N` is the size of the full space, and
:math:`\lambda_i` are the eigenvalues of the derivative matrix.
