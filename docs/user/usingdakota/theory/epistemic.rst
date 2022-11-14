.. _`uq:epist`:

Epistemic Methods
=================

This chapter covers theoretical aspects of methods for propagating
epistemic uncertainty.

.. _`sec:epist_uq:dste`:

Dempster-Shafer theory of evidence (DSTE)
-----------------------------------------

In Dempster-Shafer theory, the event space is defined by a triple
:math:`(\mathcal{S},\mathbb{S},m)` which defines :math:`\mathcal{S}` the
universal set, :math:`\mathbb{S}` a countable collection of subsets of
:math:`\mathcal{S}`, and a notional measure :math:`m`.
:math:`\mathcal{S}` and :math:`\mathbb{S}` have a similar meaning to
that in classical probability theory; the main difference is that
:math:`\mathbb{S}`, also known as the focal elements, does not have to
be a :math:`\sigma`-algebra over :math:`\mathcal{S}`. The operator
:math:`m` is defined to be

.. math::
   :label: epistemic-eq-1

   \begin{aligned}
   m(\mathcal{U}) 
   =  \left\{
   \begin{array}{rr}
   > 0 & \mathrm{if} \ \mathcal{U} \in \mathbb{S}\\
   0 & \mathrm{if} \ \mathcal{U} \subset \mathcal{S} \ \mathrm{and} \ \mathcal{U} \notin \mathbb{S} 
   \end{array} \right.\end{aligned}

.. math::
   :label: epistemic-eq-2

   \begin{aligned}
   \displaystyle\sum_{\mathcal{U} \in \mathbb{S}} m(\mathcal{U}) = 1\end{aligned}

where :math:`m(\mathcal{U})` is known as the basic probability
assignment (BPA) of the set :math:`\mathcal{U}`. In the DSTE framework,
belief and plausibility are defined as:

.. math::
   :label: bel

   \mathrm{Bel}(\mathcal{E}) = \displaystyle\sum_{\{ \mathcal{U} \ | \ \mathcal{U} \subset \mathcal{E}, \ \mathcal{U} \in \mathbb{S}\}} m(\mathcal{U})
   
.. math::
   :label: pl
   
   \mathrm{Pl}(\mathcal{E}) = \displaystyle\sum_{\{ \mathcal{U} \ | \ \mathcal{U} \cap \mathcal{E} \neq \emptyset, \ \mathcal{U} \in \mathbb{S}\}} m(\mathcal{U})

The belief Bel(:math:`\mathcal{E}`) is interpreted to be the minimum
likelihood that is associated with the event :math:`\mathcal{E}`.
Similarly, the plausibility Pl(:math:`\mathcal{E}`) is the maximum
amount of likelihood that could be associated with :math:`\mathcal{E}`.
This particular structure allows us to handle unconventional inputs,
such as conflicting pieces of evidence (e.g. dissenting expert
opinions), that would be otherwise discarded in an interval analysis or
probabilistic framework. The ability to make use of this information
results in a commensurately more informed output.

..
   TODO: cross ref this in users

   Appears already in Users Manual:
   Figure~\ref{fig:bel_plaus} shows example cumulative 
   belief and plausibility functions (CBF and CPF) and complementary 
   cumulative belief and plausibility functions (CCBF and CCPF, respectively). 
   This figure was taken from~\cite{helton_2004}.
   \begin{figure}[h!]% order of placement preference: here, top, bottom
   \begin{center}
   \includegraphics[width = 5in]{belief_plaus.eps}
   \caption{Example Cumulative and Complementary Cumulative Distributions for Belief and Plausibility}
   \label{fig:bel_plaus}
   \end{center} 
   \end{figure}


The procedure to compute belief structures involves four major steps:

#. Determine the set of :math:`d`-dimensional hypercubes that have a
   nonzero evidential measure

#. Compute the composite evidential measure (BPA) of each hypercube

#. Propagate each hypercube through the model and obtain the response
   bounds within each hypercube

#. Aggregate the minimum and maximum values of the response per
   hypercube with the BPAs to obtain cumulative belief and plausibility
   functions on the response (e.g. calculate a belief structure on the
   response).

The first step involves identifying combinations of focal elements
defined on the inputs that define a hypercube. The second step involves
defining an aggregate BPA for that hypercube, which is the product of
the BPAs of the individual focal elements defining the hypercube. The
third step involves finding the maximum and minimum values of the
response value in each hypercube, and this part can be very
computationally expensive. Finally, the results over all hypercubes are
aggregated to form belief structures on the response.
