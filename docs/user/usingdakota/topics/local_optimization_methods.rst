.. _`topic-local_optimization_methods`:

==========================
Local Optimization Methods
==========================

Local optimization methods begin from an initial point, optionally
specified by the user, and, broadly speaking, move "downhill" to the
nearest local minimum. Dakota includes both gradient-based and
derivative-free local optimizers. 

The :ref:`opt` section contains more information.

Methods
-------

Gradient-based
^^^^^^^^^^^^^^

- :dakkw:`method-optpp_cg`
- :dakkw:`method-rol`
- :dakkw:`method-dot_bfgs`
- :dakkw:`method-dot_frcg`
- :dakkw:`method-conmin_frcg`
- :dakkw:`method-npsol_sqp`
- :dakkw:`method-nlpql_sqp`
- :dakkw:`method-dot_mmfd`
- :dakkw:`method-dot_slp`
- :dakkw:`method-dot_sqp`
- :dakkw:`method-conmin_mfd`
- :dakkw:`method-optpp_newton`
- :dakkw:`method-optpp_q_newton`
- :dakkw:`method-optpp_fd_newton`

Derivative-free
^^^^^^^^^^^^^^^

- :dakkw:`method-optpp_pds`
- :dakkw:`method-coliny_cobyla`
- :dakkw:`method-coliny_pattern_search`
- :dakkw:`method-coliny_solis_wets`
- :dakkw:`method-asynch_pattern_search`
- :dakkw:`method-surrogate_based_local`
- :dakkw:`method-mesh_adaptive_search`


