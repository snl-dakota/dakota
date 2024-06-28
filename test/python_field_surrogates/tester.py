import numpy as np


class Surrogate:

    def __init__(self, params=None):

        print("python Surrogate class constructer...")
        self.coeffs = None
        if params is not None:
            self.params = params


    def construct(self, var, resp):

        var2 = np.hstack((np.ones((var.shape[0], 1)), var))
        self.coeffs = np.zeros(var.shape[1])
        z = np.linalg.inv(np.dot(var2.T, var2))
        self.coeffs = np.dot(z, np.dot(var2.T, resp))

        return


    def predict(self, pts):

        return self.coeffs[0]+pts.dot(self.coeffs[1:])


    def gradient(self, pts):

        grad = self.coeffs[1:]
        return grad.T


def text_book_numpy(params):

    num_fns = params["functions"]
    x = params["cv"]
    ASV = params["asv"]

    retval = {}

    if (ASV[0] & 1):  # **** f:
        fn = sum([(val - 1.)**4 for val in x])
        retval["fns"] = np.array([fn, 1+x[0], 2+x[1], 3+x[0]*x[1], 4+x[0]*x[0], 5+x[1]*x[1]])

    #if (ASV[0] & 2):  # **** df/dx:
    #    g = [4. * (val - 1.)**3 for val in x]
    #    retval["fnGrads"] = np.array([np.array(g)])

    #if (ASV[0] & 4):  # **** d^2f/dx^2:
    #    h = np.diag([12. * (val - 1.)**2 for val in x])
    #    retval["fnHessians"] = np.array([h])

    return retval


def funcs(params):

    #print(params)
    return text_book_numpy(params)
