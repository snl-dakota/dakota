import numpy as np

def function_value(dakota_dict):
  cv = dakota_dict["cv"]
  return sum([(val - 1.)**4 for val in cv])

def function_gradient(dakota_dict):
  cv = dakota_dict["cv"]
  g = [4. * (val - 1.)**3 for val in cv]
  return np.array(g)

def function_hessian(dakota_dict):
  cv = dakota_dict["cv"]
  h = np.diag([12. * (val - 1.)**2 for val in cv])
  return h

def constraint_0_value(dakota_dict):
  cv = dakota_dict["cv"]
  print("in constraint 0 value")
  return cv[0]**2 - cv[1] / 2.

def constraint_0_gradient(dakota_dict):
  print("in constraint 0 grad")
  cv = dakota_dict["cv"]
  return np.array([2. * cv[0], -0.5])

def constraint_0_hessian(dakota_dict):
  print("in constraint 0 hessian")
  cv = dakota_dict["cv"]
  h = np.zeros((2, 2))
  h[0, 0] = 2.
  return h

def constraint_1_value(dakota_dict):
  print("in constraint 1 value")
  cv = dakota_dict["cv"]
  return cv[1]**2 - cv[0] / 2.

def constraint_1_gradient(dakota_dict):
  print("in constraint 1 grad")
  cv = dakota_dict["cv"]
  return np.array([-0.5, 2. * cv[1]])

def constraint_1_hessian(dakota_dict):
  print("in constraint 1 hessian")
  cv = dakota_dict["cv"]
  h = np.zeros((2, 2))
  h[1, 1] = 2.
  return h

def text_book_dict(dakota_dict):

    x = dakota_dict["cv"]
    ASV = dakota_dict["asv"]
    num_fns = len(ASV)

    retval = {}

    if (ASV[0] & 1):  # **** f:
        fn = sum([(val - 1.)**4 for val in x])
        retval["fns"] = np.array([fn])

    if (ASV[0] & 2):  # **** df/dx:
        g = [4. * (val - 1.)**3 for val in x]
        retval["fnGrads"] = np.array([np.array(g)])

    if (ASV[0] & 4):  # **** d^2f/dx^2:
        h = np.diag([12. * (val - 1.)**2 for val in x])
        retval["fnHessians"] = np.array([h])

    if num_fns == 1:
        return retval

    # We have nonlinear constraints to provide
    if num_fns != 3:
        raise("text_book problem supports exactly 2 nonlinear inequality constraints.")

    if (ASV[1] & 1):  # **** first nonlinear constraint objective:
        retval['fns'] = np.append(retval['fns'], (x[0] * x[0] - x[1] / 2.0))

    if (ASV[1] & 2):  # **** d/dx of first nonlinear constraint objective:
        g = [2.0 * x[0], -0.5]
        for i in range(len(x) - 2):
            g.append(0.0)
        retval['fnGrads'] = np.append(
            retval['fnGrads'], np.array([np.array(g)]), axis=0)

    if (ASV[1] & 4):  # **** d2/dx2 of first nonlinear constraint objective:
        h = np.zeros_like(retval['fnHessians'][0])
        h[0, 0] = 2.0
        retval['fnHessians'] = np.append(retval['fnHessians'], [h], axis=0)

    if (ASV[2] & 1):  # **** second nonlinear constraint objective:
        retval['fns'] = np.append(retval['fns'], (x[1] * x[1] - x[0] / 2.0))

    if (ASV[2] & 2):  # **** d/dx of second nonlinear constraint objective:
        g = [-0.5, 2.0 * x[1]]
        for i in range(len(x) - 2):
            g.append(0.0)
        retval['fnGrads'] = np.append(
            retval['fnGrads'], np.array([np.array(g)]), axis=0)

    if (ASV[2] & 4):  # **** d2/dx2 of second nonlinear constraint objective:
        h = np.zeros_like(retval['fnHessians'][0])
        h[1, 1] = 2.0
        retval['fnHessians'] = np.append(retval['fnHessians'], [h], axis=0)

    return retval

def text_book_batch(dakota_list):
    retvals = []
    for dak_dict in dakota_list:
        retvals.append(text_book_dict(dak_dict))
    return retvals
