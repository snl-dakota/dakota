Dakota/ROL Interface Overview
=============================

Since the orginal Dakota/ROL interface was written, ROL has had a number of significant 
updates, particularly the refactoring of the OptimizationProblem/OptimizationSolver 
interface to a simpler Problem/Solver interface. This new interface allows for 
distinction between linear and nonlinear (in)equality constraints as ROL offers 
algorithms that can treat them differently. 

The old ROLOptimzier used std::vector as the underlying container type which required quite a bit of copying and casting to/from Dakota's native RealMatrix/RealVector containers (Teuchos::SerialDenseMatrix and ..Vector) with utility functions defined in DakotaOptimizer.hpp. ROL is designed to work with user-defined containers and already provides
adapters for Teuchos::SerialDense types.


ROL's representation of optimization problem components
-------------------------------------------------------

Let us denote a generic optimization problem as having the form

         min_x f(x)           Objective     
    subject to l <= x <= u    Bounds on optimzation variables
               A1*x = b1      Linear Equality Constraint
               ce(x) = 0      Nonlinear Equality Constraint
               A2*x >= b2     Linear Inequality Constraint
               ci(x) >= 0     Nonlinear Inequality Constraint

Optimization and constraint vectors (and their duals) are represented by ROL::Vector

ROL represents the objective function through the ROL::Objective interface
and the bounds through ROL::BoundConstraint, which stores the upper and lower bound
vectors. ROL no longer has separate classes for equality and inequality constraints 
and only has a single ROL::Constraint base class whose meaning is inferred by 
ROL::Problem depending on how it is added to the problem. 

ROL::Problem::addConstraint is overloaded and will interpret the ROL::Constraint 
argument as an equality constraint when no accompanying ROL::BoundConstraint is 
provided and an inequality when it is. Note that inequalities may be expressed with
only a lower bound as above or with both upper and lower bounds.

Linear (in)equality constraints are implemented using ROL::LinearConstraint
and ROL::Problem::addLinearConstraint is overloaded similarly to 
ROL::Problem::addConstraint. Note that the right-hand-sides of the above linear 
(in)equalities are stored in the ROL::LinearConstraint class. 


Dakota's representation of optimization problem components
----------------------------------------------------------

The above quantities are access through Dakota::Model and Dakota::Response interfaces.
Dakota::Model will evaluate all of the possible objective and constraint values and 
their derivatives (if computed) simultaneously. These quantities are retrieved through 
Dakota::Response, itself being accessable from Dakota::Model

The Model is informed of the new optimization vector by calling 
Model::continuous\_variables(x) where x is a RealVector (Teuchos::SerialDenseVector).

If there is an active set, Model::evaluate() is called after Model::continuous\_variables()

Dakota distinguishes between equality and inequality constraints as well as linear and nonlinear constraints.  

Response::function\_values() returns a RealVector containing the objective and constraint values

  - the first element [0] is the objective function value at x
  - If there are m nonlinear inequality constraints then elements [1] through [m] contain their values at x 
  - If there are n nonlinear equality constraints, then elements [m+1] through [m+n+1] contain their values at x 

The bounds on the optimization vector are obtained from Model::continuous\_lower\_bounds() and
Model::continuous\_upper\_bounds()

If Dakota evalutes a gradient, it is accessed by Respose::gradient\_values(). If they are computed, then the partitioning above holds for the objective gradient and nonlinear constraint Jacobians
 


  
