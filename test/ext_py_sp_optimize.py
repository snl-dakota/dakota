
import numpy as np
from scipy.optimize import minimize

import ext_method

class Standalone:

    # Define the objective function
    def objective_function(self, vars):
        x, y = vars
        print("Type vars: ",type(vars))
        return (x - 1)**4 + (y - 1)**4

    # Define the gradient of the objective function
    def gradient(self, vars):
        x, y = vars
        grad_x = 4 * (x - 1)**3
        grad_y = 4 * (y - 1)**3
        return np.array([grad_x, grad_y])

    # Define the Hessian of the objective function
    def hessian(self, vars):
        x, y = vars
        h_xx = 12 * (x - 1)**2
        h_yy = 12 * (y - 1)**2
        return np.array([[h_xx, 0   ], 
                         [0   , h_yy]])

    def solve(self):

        # Initial guess
        initial_guess = [0, 0]

        # Perform the optimization with the gradient and Hessian
        #result = minimize(self.objective_function, initial_guess)
        #result = minimize(self.objective_function, initial_guess, jac=self.gradient, hess=self.hessian)
        result = minimize(self.objective_function, initial_guess, method='trust-ncg', jac=self.gradient, hess=self.hessian)

        # Print the results
        print("Optimal value of x1:", result.x[0])
        print("Optimal value of x2:", result.x[1])
        print("Minimum value of the function:", result.fun)



###############################################################
#   Demo Python Method --> Scipy Optimizer   with Derivatives #
###############################################################

class ScipyOpt:

    def __init__(self, executor, params_file=None):

        print("python ScipyOpt class constructer...")
        self.executor = executor
        if params_file is not None:
            self.params_file = params_file

    def obj_fn(self, x):
        f = self.executor.function_value(x)
        return f[0] # Single objective only for now

    def grad(self, x):
        grad = np.array(self.executor.gradient_values(x)[0])
        print("grad:",grad)
        return grad

    def hess(self, x):
        hess = np.array(self.executor.hessian_values(x)[0])
        print("hess:",hess)
        h_xx = 12 * (x[0] - 1)**2
        h_yy = 12 * (x[1] - 1)**2
        h = np.array([[h_xx, 0   ],
                     [0   , h_yy]])
        print(h)
        return hess

    def core_run(self):

        # Only treat continuous vars for now
        n_fns  = self.executor.response_size()
        init_pts = np.array(self.executor.initial_values())

        if n_fns != 1:
            raise RuntimeError("ScipyOpt only supports a single response")


        # Use the appropriate scipy minimizer
        if self.executor.has_gradient() and self.executor.has_hessian():
            result = minimize(self.obj_fn, init_pts, method='trust-ncg', \
                              jac=self.grad, hess=self.hess)

        elif self.executor.has_gradient():
            result = minimize(self.obj_fn, init_pts, jac=self.grad)

        else:
            result = minimize(self.obj_fn, init_pts)

        # Print the results
        print("Optimal value of x:", result.x[0])
        print("Optimal value of y:", result.x[1])
        print("Minimum value of the function:", result.fun)

        retval = {}
        retval['fns_sp']    = [result.fun]
        retval['best_x_np'] = [result.x[0], result.x[1]]

        # Send output to Dakota in a format amenable to regression testing
        self.executor.dak_print("<<<<< Best parameters          =")
        self.executor.dak_print(f"\t\t{result.x[0]} x1")
        self.executor.dak_print(f"\t\t{result.x[1]} x2")
        self.executor.dak_print("<<<<< Best objective function  =")
        self.executor.dak_print(f"\t\t{result.fun}")

        return retval


if __name__ == "__main__":

    opt = Standalone()
    opt.solve()

