Constraint Interface Design
===========================


The base class is `rol_interface::Constraint` which inherits from ROL::Constraint
and overrides the following member functions:

    Constraint::update()
    Constraint::value()
    Constraint::applyJacobian()
    Constraint::applyAdjointJacobian()
    Constraint::applyAdjointHessian

Where the argument vector types are `ROL::Vector`. 

Each overridden member function downcasts the `ROL::Vector` objects to `rol_interface::Vector` and then accesses the underlying `Dakota::RealVector` using the `rol_interface::get_vector` function, for example:


    void Constraint::value(       ROL::Vector<Dakota::Real>& c,
                            const ROL::Vector<Dakota::Real>& x,
                                  Dakota::Real&              tol ) {
      value(get_vector(c), get_vector(x), tol);
    }

For each method taking `ROL::Vector` arguments there is an analogous overloaded method that takes `Dakota::RealVector` arguments. The correspondingh `value` function in this case has the signature

    void Constraint::value(       Dakota::RealVector& c,
                            const Dakota::RealVector& x,
                                  Dakota::Real&       tol );




 
