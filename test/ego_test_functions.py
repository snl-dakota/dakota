import numpy as np
import os
import time
import datetime

def unpack_inputs(params):
    x = params['cv']
    ASV = params['asv']
    return x, ASV

### define utils function

## workflow:
# update_counter()
# start_diary(counter)
# random_wait()
# stop_diary(counter)

def update_counter():
    if os.path.isfile('counter.log'):
        counter = np.loadtxt('counter.log')
        counter += 1 
        f = open('counter.log', 'w') # overwrite
        f.write('%d\n' % int(counter))
        f.close()
    else:
        f = open('counter.log', 'w') # overwrite
        counter = 1
        f.write('%d\n' % counter)
        f.close()
    return counter

def random_wait(lower_bound, upper_bound):
    wait_time = np.random.uniform(low=lower_bound, high=upper_bound)
    time.sleep(wait_time)
    return None

def start_diary(counter):
    f = open('diary.log', 'a') # append
    currentTime = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    f.write('Start querying counter %d at %s\n' % (counter, currentTime))
    f.close()

def stop_diary(counter):
    f = open('diary.log', 'a') # append
    currentTime = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    f.write('Stop querying counter %d at %s\n' % (counter, currentTime))
    f.close()


def branin(**kwargs):
    """
    input space: [-5, 10] x [0, 15]
    min at (-pi, 12.275), (pi, 2.275), and (9.42478, 2.475)
    min value = 0.397887
    """

    x, ASV = unpack_inputs(kwargs)
    retval = {}

    a = 1.
    b = 5.1/(4.*np.pi**2)
    c = 5/np.pi
    r = 6.
    s = 10.
    t = 1./(8.*np.pi)


    if (ASV[0] & 1):
        f = a*(x[1] - b*x[0]*x[0] + c*x[0] - r)**2 \
          + s*(1. - t)*np.cos(x[0]) + s
        retval['fns'] = np.array([f])

    counter = update_counter() - 1; start_diary(); random_wait(10,300); stop_diary();
    return retval

def hartmann_3d(**kwargs):
    """
    input space: [0, 1]**3
    min at (0.114614, 0.555649, 0.852547)
    min value = -3.86278
    """

    x, ASV = unpack_inputs(kwargs)
    retval = {}

    x = np.array([x[0], x[1], x[2]])

    alpha = np.array([1.0, 1.2, 3.0, 3.2])

    A = np.array([[3.0, 10., 30.],
                  [0.1, 10., 35.],
                  [3.0, 10., 30.],
                  [0.1, 10., 35.]])

    P = np.array([[3689., 1170., 2673.],
                  [4699., 4387., 7470.],
                  [1091., 8732., 5547.],
                  [381. , 5743., 8828.]])
    P *= 1e-4

    v = np.zeros(4)
    for i in range(4):
      for j in range(len(x)):
        v[i] += -A[i,j] * (x[j] - P[i,j])**2


    if (ASV[0] & 1):
        f = -alpha.dot(np.exp(v))
        retval['fns'] = np.array([f])

    counter = update_counter() - 1; start_diary(); random_wait(10,300); stop_diary();
    return retval

def hartmann_6d(**kwargs):
    """
    input space: [0, 1]**6
    min at (0.20169, 0.150011, 0.476874, 0.275332, 0.311652, 0.6573)
    min value = -3.32237
    """

    x, ASV = unpack_inputs(kwargs)
    retval = {}

    x = np.array([x[0], x[1], x[2],
                  x[3], x[4], x[5]])

    alpha = np.array([1.0, 1.2, 3.0, 3.2])

    A = np.array([[10., 3., 17., 3.5, 1.7, 8.],
                  [0.05, 10., 17., 0.1, 8., 14.],
                  [3., 3.5, 1.7, 10., 17., 8.],
                  [17., 8., 0.05, 10., 0.1, 14.]])

    P = np.array([[1312., 1696., 5569., 124., 8283., 5886.],
                  [2329., 4135., 8307., 3736., 1004., 9991.],
                  [2348., 1451., 3522., 2883., 3047., 6650.],
                  [4047., 8828., 8732., 5743., 1091., 381.]])
    P *= 1e-4

    v = np.zeros(4)
    for i in range(4):
      for j in range(len(x)):
        v[i] += -A[i,j] * (x[j] - P[i,j])**2


    if (ASV[0] & 1):
        f = -alpha.dot(np.exp(v))
        retval['fns'] = np.array([f])

    counter = update_counter() - 1; start_diary(); random_wait(10,300); stop_diary();
    return retval

def six_hump_camel(**kwargs):
    """
    input space: [-3, 3] x [-2, 2]
    min at (0.0898, -0.7126) and (-0.0898, 0.7126)
    min value = -1.0316
    """

    x, ASV = unpack_inputs(kwargs)
    retval = {}

    if (ASV[0] & 1):
        f = (4. - 2.1*x[0]**2 + x[0]**4/3.)*x[0]**2 + x[0]*x[1] \
          + (-4. + 4.*x[1]**2)*x[1]**2
        retval['fns'] = np.array([f])

    counter = update_counter() - 1; start_diary(); random_wait(10,300); stop_diary();
    return retval

def three_hump_camel(**kwargs):
    """
    input space: [-5, 5]**2
    min at (0, 0)
    min value = 0
    """

    x, ASV = unpack_inputs(kwargs)
    retval = {}

    if (ASV[0] & 1):
        f = 2.*x[0]**2 - 1.05*x[0]**4 + x[0]**6/6. + x[0]*x[1] + x[1]**2
        retval['fns'] = np.array([f])

    counter = update_counter() - 1; start_diary(); random_wait(10,300); stop_diary();
    return retval

def rosenbrock(**kwargs):
    """
    arbitrary dimensionality
    input space: [-5, 10]**d or [-2.048, 2.048]**d
    min at (1)**d
    min value = 0
    """

    x, ASV = unpack_inputs(kwargs)
    retval = {}

    if (ASV[0] & 1):
        d = len(x) # d: dimensionality
        tmp_sum = 0
        for ii in range(d-2):
            xi = xx[ii]
            xnext = xx[ii+1]
            new = 100*(xnext-xi**2)**2 + (xi-1)**2
            tmp_sum += new;
        f = tmp_sum
        retval['fns'] = np.array([f])

    counter = update_counter() - 1; start_diary(); random_wait(10,300); stop_diary();
    return retval

def rotated_hyper_ellipsoid(**kwargs):
    """
    arbitrary dimensionality
    input space: [-65.536, 65.536]**d
    min at (0)**d
    min value = 0
    """

    x, ASV = unpack_inputs(kwargs)
    retval = {}

    if (ASV[0] & 1):
        d = len(x) # d: dimensionality
        outer = 0
        for ii in range(d):
            inner = 0
            for jj in range(ii):
                xj = xx[jj]
                inner = inner + xj**2;
            end
            outer += inner;
        end
        f = outer
        retval['fns'] = np.array([f])

    counter = update_counter() - 1; start_diary(); random_wait(10,300); stop_diary();
    return retval

def sphere(**kwargs):
    """
    arbitrary dimensionality
    input space: [-5.12, 5.12]**d
    min at (0)**d
    min value = 0
    """

    x, ASV = unpack_inputs(kwargs)
    retval = {}

    if (ASV[0] & 1):
        d = len(x) # d: dimensionality
        tmp_sum = 0
        for ii in range(d):
            xi = xx[ii]
            tmp_sum += xi**2
        f = tmp_sum
        retval['fns'] = np.array([f])

    counter = update_counter() - 1; start_diary(); random_wait(10,300); stop_diary();
    return retval
