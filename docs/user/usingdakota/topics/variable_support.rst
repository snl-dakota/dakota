.. _`topic-variable_support`:

Variable Support
================

Different nondeterministic methods have differing support for
uncertain variable distributions. Tables :numref:`table:variable_support_part_1`,
:numref:`table:variable_support_part_2`, and :numref:`table:variable_support_part_3`
summarize the uncertain variables that are available for use by the different methods, where a "-"
indicates that the distribution is not supported by the method, a "U"
means the uncertain input variables of this type must be uncorrelated,
a "C" denotes that correlations are supported involving uncertain
input variables of this type, and an "A" means the appropriate
variables must be specified as active in the variables
specification block.  For example, if one wants to support
sampling or a stochastic expansion method over both
continuous uncertain and continuous state variables, the
specification :dakkw:`active all <variables-active-all>` must be listed in the variables
specification block.
Additional notes include:

- we have four variants for stochastic expansions (SE), listed as 
  Wiener, Askey, Extended, and Piecewise which draw from different
  sets of basis polynomials.  The term stochastic expansion indicates
  polynomial chaos and stochastic collocation collectively, although
  the Piecewise option is only currently supported for stochastic
  collocation.  Refer to :dakkw:`method-polynomial_chaos` and
  :dakkw:`method-stoch_collocation` for additional information on these three  options.
- methods supporting the epistemic interval distributions have differing
  approaches:
  
  - :dakkw:`method-sampling` and :dakkw:`global_interval_est lhs <method-global_interval_est-lhs>` 
    model the interval basic probability  assignments (BPAs) as continuous histogram
    bin distributions for  purposes of generating samples;
  - :dakkw:`method-local_interval_est` and :dakkw:`global_interval_est ego <method-global_interval_est-ego>`
    ignore the BPA details and models these variables as simple bounded regions defined by the cell extremes;
  - and :dakkw:`method-local_evidence` and :dakkw:`method-global_evidence` model the interval
    specifications as true BPAs.

.. table:: Part I:  Continuous Aleatory Types
   :name: table:variable_support_part_1

   =========================== =========== ==================== ===================== ============ =========== ============== =============== ================= ================== ================= ================= 
    Distribution Type           Sampling    Local Reliability    Global Reliability    Wiener SE    Askey SE    Extended SE    Piecewise SE    Local Interval    Global Interval    Local Evidence    Global Evidence  
   =========================== =========== ==================== ===================== ============ =========== ============== =============== ================= ================== ================= ================= 
    Normal                      C           C                    C                     C            C           C              \-              \-                \-                 \-                \-               
    Bounded Normal              C           U                    U                     U            U           U              U               \-                \-                 \-                \-               
    Lognormal                   C           C                    C                     C            C           U              \-              \-                \-                 \-                \-               
    Bounded Lognormal           C           U                    U                     U            U           U              U               \-                \-                 \-                \-               
    Uniform                     C           C                    C                     C            U           U              U               \-                \-                 \-                \-               
    Loguniform                  C           U                    U                     U            U           U              U               \-                \-                 \-                \-               
    Triangular                  C           U                    U                     U            U           U              U               \-                \-                 \-                \-               
    Exponential                 C           C                    C                     C            U           U              \-              \-                \-                 \-                \-               
    Beta                        C           U                    U                     U            U           U              U               \-                \-                 \-                \-               
    Gamma                       C           C                    C                     C            U           U              \-              \-                \-                 \-                \-               
    Gumbel                      C           C                    C                     C            C           U              \-              \-                \-                 \-                \-               
    Frechet                     C           C                    C                     C            C           U              \-              \-                \-                 \-                \-               
    Weibull                     C           C                    C                     C            C           U              \-              \-                \-                 \-                \-               
    Continuous Histogram Bin    C           U                    U                     U            U           U              U               \-                \-                 \-                \-               
   =========================== =========== ==================== ===================== ============ =========== ============== =============== ================= ================== ================= ================= 

.. table:: Part II: Discrete Aleatory Types
   :name: table:variable_support_part_2

   =========================== =========== ==================== ===================== ============ =========== ============== =============== ================= ================== ================= ================= 
    Distribution Type           Sampling    Local Reliability    Global Reliability    Wiener SE    Askey SE    Extended SE    Piecewise SE    Local Interval    Global Interval    Local Evidence    Global Evidence  
   =========================== =========== ==================== ===================== ============ =========== ============== =============== ================= ================== ================= ================= 
    Poisson                     C           \-                   \-                    \-           \-          \-             \-              \-                \-                 \-                \-               
    Binomial                    C           \-                   \-                    \-           \-          \-             \-              \-                \-                 \-                \-               
    Negative Binomial           C           \-                   \-                    \-           \-          \-             \-              \-                \-                 \-                \-               
    Geometric                   C           \-                   \-                    \-           \-          \-             \-              \-                \-                 \-                \-               
    Hypergeometric              C           \-                   \-                    \-           \-          \-             \-              \-                \-                 \-                \-               
    Discrete Histogram Point    C           \-                   \-                    \-           \-          \-             \-              \-                \-                 \-                \-               
   =========================== =========== ==================== ===================== ============ =========== ============== =============== ================= ================== ================= ================= 

.. table:: Part III: Epistemic, Design, and State Types
   :name: table:variable_support_part_3

   =========================================== =========== ==================== ===================== ============ =========== ============== =============== ================= ================== ================= ================= 
    Distribution Type                           Sampling    Local Reliability    Global Reliability    Wiener SE    Askey SE    Extended SE    Piecewise SE    Local Interval    Global Interval    Local Evidence    Global Evidence  
   =========================================== =========== ==================== ===================== ============ =========== ============== =============== ================= ================== ================= ================= 
    Interval                                    U           \-                   U,A                   U,A          U,A         U,A            U,A             U                 U                  U                 U                
    Continuous Design                           U,A         \-                   U,A                   U,A          U,A         U,A            U,A             \-                \-                 \-                \-               
    Discrete Design Range, Int Set, Real Set    U,A         \-                   \-                    \-           \-          \-             \-              \-                \-                 \-                \-               
    Continuous State                            U,A         \-                   U,A                   U,A          U,A         U,A            U,A             \-                \-                 \-                \-               
    Discrete State Range, Int Set, Real Set     U,A         \-                   \-                    \-           \-          \-             \-              \-                \-                 \-                \-               
   =========================================== =========== ==================== ===================== ============ =========== ============== =============== ================= ================== ================= ================= 

