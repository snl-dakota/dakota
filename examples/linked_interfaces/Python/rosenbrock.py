def rosenbrock_list(**kwargs):

    num_fns = kwargs['functions']
 #   if num_fns > 1:
 #       least_sq_flag = true
 #   else:
 #       least_sq_flag = false
        
    x = kwargs['cv']
    ASV = kwargs['asv']

    # get analysis components
    #an_comps = kwargs['analysis_components']
    #print an_comps

    f0 = x[1]-x[0]*x[0]
    f1 = 1-x[0]

    retval = dict([])

    if (ASV[0] & 1): # **** f:
        f = [100*f0*f0+f1*f1]
        retval['fns'] = f

    if (ASV[0] & 2): # **** df/dx:
        g = [ [-400*f0*x[0] - 2*f1, 200*f0] ]
        retval['fnGrads'] = g

    if (ASV[0] & 4): # **** d^2f/dx^2:
        fx = x[1]-3*x[0]*x[0]
        
        h = [
              [ [-400*fx + 2, -400*x[0]],
                [-400*x[0],    200     ] ]
            ]
        retval['fnHessians'] = h


    return(retval)

def rosenbrock_numpy(**kwargs):

    from numpy import array

    num_fns = kwargs['functions']
 #   if num_fns > 1:
 #       least_sq_flag = true
 #   else:
 #       least_sq_flag = false
        
    x = kwargs['cv']
    ASV = kwargs['asv']

    f0 = x[1]-x[0]*x[0]
    f1 = 1-x[0]

    retval = dict([])

    if (ASV[0] & 1): # **** f:
        f = array([100*f0*f0+f1*f1])
        retval['fns'] = f

    if (ASV[0] & 2): # **** df/dx:
        g = array([[-400*f0*x[0] - 2*f1, 200*f0]])
        retval['fnGrads'] = g

    if (ASV[0] & 4): # **** d^2f/dx^2:
        fx = x[1]-3*x[0]*x[0]
        
        h = array([ [ [-400*fx + 2, -400*x[0]],
              [-400*x[0],    200     ] ] ]    )
        retval['fnHessians'] = h


    return(retval)

