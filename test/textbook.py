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
