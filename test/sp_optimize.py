
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

    def __init__(self, params=None):

        print("python ScipyOpt class constructer...")
        if params is not None:
            self.params = params

    def obj_fn(self, x):
        f = self.executor.function_value(x)
        return f

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

    def pre_run(self, executor):
        print("Here... in pre_run.")
        self.executor = executor

    def core_run(self, executor):

        # Only treat continuous vars for now
        n_fns  = executor.response_size()
        init_pts = np.array(executor.initial_values())

        if n_fns != 1:
            raise RuntimeError("ScipyOpt only supports a single response")


        # Use the appropriate scipy minimizer
        if executor.has_gradient() and executor.has_hessian():
            result = minimize(self.obj_fn, init_pts, method='trust-ncg', \
                              jac=self.grad, hess=self.hess)

        elif executor.has_gradient():
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

        if False:
            print("Found best_f = ", best_f)
            print("Using x = ", best_x)

        return retval


if __name__ == "__main__":

    opt = Standalone()
    opt.solve()

