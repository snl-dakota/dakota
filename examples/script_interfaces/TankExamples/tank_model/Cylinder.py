#!/usr/bin/env python 
# Cylinder

# usage from python script:
## import Cylinder
## Cylinder.main(X_vec, Phi_vec, P, Gamma, H, E, Nu, L, R, T, M, N, summaryFile, dataFile)

# usage example from commandline:
## python -c"import Cylinder; Cylinder.main(13, [1,1.1], 10, 3.2, 12, 3e7, .3, 59, 30, 0.25, 19, 13, 'summary', 'data')"

# This computes the stresses, strains, and displacements on a cylinder with
# simple supports on the ends, while it has liquid and pressure loading
# REFERENCE
## The series solution is adapted from: S. Timoshenko, S. Woinowsky-Krieger: Theory of Plates and Shells.  McGraw-Hill (1987). 
## https://ia700807.us.archive.org/34/items/TheoryOfPlatesAndShells/TheoryOfPlatesAndShellsS.timoshenko2ndEdition.pdf


import Cylinder_reports
from sys import argv
from math import pi, sin, cos, acos, sinh, cosh, sqrt

# Identifiers for the tank surfaces and directions
U = 1 #Axial
V = 2 #Radial
W = 3 #Transverse
OUT = 1 #outer surface of the cylinder wall
IN = -1

class HydrostaticCoefficients_mn:
  def __init__(self):
    self.A_fluid_mn = float('NaN')
    self.B_fluid_mn = float('NaN')
    self.C_fluid_mn = float('NaN')
                
  def __str__(self):
    return 'A_fluid_mn='+str(self.A_fluid_mn)+'\n'+'B_fluid_mn='+str(self.B_fluid_mn)+'\n'+'C_fluid_mn='+str(self.C_fluid_mn)+'\n'

class HydrostaticCoefficients:
  def __init__(self, Mmax, Nmax):
    self.coefficients=[]
    self.columnIndex=[]
     
    for m in range (0, Mmax):
      self.columnIndex.append(-1)
      row=[]
      for n in range(0, Nmax):
        row.append(HydrostaticCoefficients_mn())
      self.coefficients.append(row)
    self.rowIndex=-1
     
  def set(self, m, n, coeff):
    self.rowIndex=max(self.rowIndex, m)
    self.columnIndex[m]=max(self.columnIndex[m], n)
    self.coefficients[m][n]=coeff
    
  def get(self, m, n):
    return self.coefficients[m][n]

  def getRowIndex(self):
    return self.rowIndex
  
  def getColumnIndex(self, m):
    return self.columnIndex[m]

class MidDisplComponents:
  def __init__(self):
    self.u = 0
    self.v = 0
    self.w = 0
    
class StrainsAndCurvatures:
  def __init__(self):
    self.ep_x_0 = float('NaN')
    self.ep_phi_0 = float('NaN')
    self.gam_x_phi_0 = float('NaN')
    self.kappa_x = float('NaN')
    self.kappa_phi = float('NaN')
    self.kappa_x_phi = float('NaN')
   
class StressComponents:
  def __init__(self):
    self.sig_x_out = 0.0
    self.sig_phi_out = 0.0
    self.sig_x_phi_out = 0.0
    self.sig_eq_out = 0.0
    self.sig_x_in = 0.0
    self.sig_phi_in = 0.0
    self.sig_x_phi_in = 0.0
    self.sig_eq_in = 0.0    

class StrainComponents:
  def __init__(self):
    self.eps_x_out = 0.0
    self.eps_phi_out = 0.0
    self.gamma_x_phi_out = 0.0
    self.eps_x_in = 0.0
    self.eps_phi_in = 0.0
    self.gamma_x_phi_in = 0.0

    
class Results:
  def __init__(self):
    midDisplMap = []
    stressMap = []
    strainMap = []
    maxStressEqu = 0
    X_ind_max_stress = 0
    Phi_ind_max_stress = 0
    surface_stress = 0

def evalCoefficents_fluid(m, n, D_fluid_mn, Length, Thickness, Radius, E, nu):
  coeffs = HydrostaticCoefficients_mn()
  
  PI = pi
  PI2 = PI*PI ; PI3 = PI2*PI ; PI4 = PI3*PI ; PI6 = PI4*PI2 ; PI8 = PI6*PI2
  T3 = Thickness*Thickness*Thickness
  R2 = Radius*Radius ; R4 = R2*R2 ; R5 = R4*Radius ; R6 = R5*Radius ; R7 = R6*Radius ; R8 = R7*Radius
  L2 = Length*Length ; L4 = L2*L2 ; L5 = L4*Length ; L6 = L5*Length ; L7 = L6*Length ; L8 = L7*Length
  nu2 = nu*nu ; nu3 = nu2*nu
  n2 = n*n ; n3 = n2*n ; n4 = n3*n ; n6 = n4*n2 ; n8 = n6*n2
  m2 = m*m ; m3 = m2*m ; m4 = m3*m ; m6 = m4*m2 ; m8 = m6*m2
  
  coeffs.A_fluid_mn = ((12*R7*L5*m3*nu3-12*R7*L5*m3*nu)*D_fluid_mn*PI3+(12*R5*L7*m*n2-12*R5*L7*m*n2*nu2)*D_fluid_mn*PI)/(R8*T3*m8*E*PI8+4*R6*T3*L2*m6*n2*E*PI6+(-12*R6*Thickness*L4*m4*nu2+6*R4*T3*L4*m4*n4+12*R6*Thickness*L4*m4)*E*PI4+4*R2*T3*L6*m2*n6*E*PI2+T3*L8*n8*E)
  coeffs.B_fluid_mn = -((12*R6*L6*m2*n*nu3+24*R6*L6*m2*n*nu2-12*R6*L6*m2*n*nu-24*R6*L6*m2*n)*D_fluid_mn*PI2+(12*R4*L8*n3*nu2-12*R4*L8*n3)*D_fluid_mn)/(R8*T3*m8*E*PI8+4*R6*T3*L2*m6*n2*E*PI6+(-12*R6*Thickness*L4*m4*nu2+6*R4*T3*L4*m4*n4+12*R6*Thickness*L4*m4)*E*PI4+4*R2*T3*L6*m2*n6*E*PI2+T3*L8*n8*E)
  coeffs.C_fluid_mn = -((12*R8*L4*m4*nu2-12*R8*L4*m4)*D_fluid_mn*PI4+(24*R6*L6*m2*n2*nu2-24*R6*L6*m2*n2)*D_fluid_mn*PI2+(12*R4*L8*n4*nu2-12*R4*L8*n4)*D_fluid_mn)/(R8*T3*m8*E*PI8+4*R6*T3*L2*m6*n2*E*PI6+(-12*R6*Thickness*L4*m4*nu2+6*R4*T3*L4*m4*n4+12*R6*Thickness*L4*m4)*E*PI4+4*R2*T3*L6*m2*n6*E*PI2+T3*L8*n8*E)   
  
  return coeffs
  
def evalMidDispls_fluid_mn(x, phi, Length, m, n, coeffs):
  disp = MidDisplComponents()
  
  A_fluid_mn = coeffs.A_fluid_mn
  B_fluid_mn = coeffs.B_fluid_mn
  C_fluid_mn = coeffs.C_fluid_mn
  
  angularArg = n*phi
  axialArg = m*pi*x/Length
  disp.u = A_fluid_mn*cos(angularArg)*cos(axialArg)
  disp.v = B_fluid_mn*sin(angularArg)*sin(axialArg)
  disp.w = C_fluid_mn*cos(angularArg)*sin(axialArg)
  
  return disp

def evalMidDispls_pres(x, beta, Length, Thickness, Radius, E, Pressure, PP, QQ, RR):
  disp = MidDisplComponents()

  disp.u = 0 
  disp.v = 0
  disp.w = -PP*(1-QQ*sin(beta*x)*sinh(beta*x)-RR*cos(beta*x)*cosh(beta*x))
  return disp

def evalStrainsAndCurvatures_pres(x, phi, beta, Radius, nu, w, PP, QQ, RR):
  sc = StrainsAndCurvatures()
  
  sc.ep_phi_0 = PP*(-cos(beta*x)*cosh(beta*x)*RR-sin(beta*x)*sinh(beta*x)*QQ+1)/Radius
  sc.gam_x_phi_0 = 0
  sc.ep_x_0 = nu*w/Radius
  sc.kappa_x = -PP*(2*beta**2*sin(beta*x)*sinh(beta*x)*RR-2*beta**2*cos(beta*x)*cosh(beta*x)*QQ)
  sc.kappa_phi = 0
  sc.kappa_x_phi = 0
    
  return sc

def evalStrainsAndCurvatures_fluid_mn(x, phi, Length, Radius, coeffs, m, n, q_fluid_mn):
  sc = StrainsAndCurvatures()
  
  A_fluid_mn = coeffs.A_fluid_mn
  B_fluid_mn = coeffs.B_fluid_mn
  C_fluid_mn = coeffs.C_fluid_mn
  PI = pi

  sc.ep_x_0 = -A_fluid_mn*m*cos(n*phi)*PI*sin(m*x*PI/Length)/Length
  sc.ep_phi_0 = B_fluid_mn*n*cos(n*phi)*sin(m*x*PI/Length)/Radius-C_fluid_mn*cos(n*phi)*sin(m*x*PI/Length)/Radius
  sc.gam_x_phi_0 = B_fluid_mn*m*sin(n*phi)*PI*cos(m*x*PI/Length)/Length-A_fluid_mn*n*sin(n*phi)*cos(m*x*PI/Length)/Radius
  sc.kappa_x = -C_fluid_mn*(m*m)*cos(n*phi)*PI*PI*sin(m*x*PI/Length)/(Length*Length)
  sc.kappa_phi = (B_fluid_mn*n*cos(n*phi)*sin(m*x*PI/Length)-C_fluid_mn*n*n*cos(n*phi)*sin(m*x*PI/Length))/(Radius*Radius)
  sc.kappa_x_phi = (B_fluid_mn*m*sin(n*phi)*PI*cos(m*x*PI/Length)/Length-C_fluid_mn*m*n*sin(n*phi)*PI*cos(m*x*PI/Length)/Length)/Radius
  
  return sc

def evalStresses_pres(x, phi, Thickness, Radius, E, nu, Pressure, w, beta, PP, QQ, RR):
  sig = StressComponents()
  
  sc = evalStrainsAndCurvatures_pres(x, phi, beta, Radius, nu, w, PP, QQ, RR)
  ep_x_0 = sc.ep_x_0
  ep_phi_0 = sc.ep_phi_0
  gam_x_phi_0 = sc.gam_x_phi_0
  kappa_x = sc.kappa_x
  kappa_phi = sc.kappa_x_phi
  kappa_x_phi = sc.kappa_x_phi
  
  G = E/(2*(1+nu))
  
  z = -Thickness/2
  sig.sig_x_out = E/(1-nu*nu)*(ep_x_0+nu*ep_phi_0-z*(kappa_x+nu*kappa_phi))
  sig.sig_phi_out = E/(1-nu*nu)*(ep_phi_0+nu*ep_x_0-z*(kappa_phi+nu*kappa_x))
  sig.sig_x_phi_in = (gam_x_phi_0-2*z*kappa_x_phi)*G
    
  z = Thickness/2
  sig.sig_x_in = E/(1-nu*nu)*(ep_x_0+nu*ep_phi_0-z*(kappa_x+nu*kappa_phi))
  sig.sig_phi_in = E/(1-nu*nu)*(ep_phi_0+nu*ep_x_0-z*(kappa_phi+nu*kappa_x))
  sig.sig_x_phi_in = (gam_x_phi_0-2*z*kappa_x_phi)*G
  
  return sig

def evalStresses_fluid_mn(x, phi, Length, Thickness, Radius, E, nu, coeffs, m, n, q_fluid_mn):
  sig_fluid_mn = StressComponents()
  
  sc = evalStrainsAndCurvatures_fluid_mn(x, phi, Length, Radius, coeffs, m, n, q_fluid_mn)
  ep_x_0 = sc.ep_x_0
  ep_phi_0 = sc.ep_phi_0
  gam_x_phi_0 = sc.gam_x_phi_0
  kappa_x = sc.kappa_x
  kappa_phi = sc.kappa_x_phi
  kappa_x_phi = sc.kappa_x_phi
  
  G = E/(2*(1+nu))
  
  z = -Thickness/2
  sig_fluid_mn.sig_x_out = E/(1-nu*nu)*(ep_x_0+nu*ep_phi_0-z*(kappa_x+nu*kappa_phi))
  sig_fluid_mn.sig_phi_out = E/(1-nu*nu)*(ep_phi_0+nu*ep_x_0-z*(kappa_phi+nu*kappa_x))
  sig_fluid_mn.sig_x_phi_in = (gam_x_phi_0-2*z*kappa_x_phi)*G
    
  z = Thickness/2
  sig_fluid_mn.sig_x_in = E/(1-nu*nu)*(ep_x_0+nu*ep_phi_0-z*(kappa_x+nu*kappa_phi))
  sig_fluid_mn.sig_phi_in = E/(1-nu*nu)*(ep_phi_0+nu*ep_x_0-z*(kappa_phi+nu*kappa_x))
  sig_fluid_mn.sig_x_phi_in = (gam_x_phi_0-2*z*kappa_x_phi)*G
  
  return sig_fluid_mn

def evalLoadCoeff_fluid_mn(Radius, gamma, LiqHeight, m, n):
  if LiqHeight > Radius:
    alpha = pi-acos((LiqHeight-Radius)/Radius)
  else:
    alpha = acos((Radius-LiqHeight)/Radius)
    
  if n == 0:
    D_fluid_mn = -4*gamma*Radius/(m*pi*pi)*(sin(alpha)-alpha*cos(alpha))
  elif n == 1:
    D_fluid_mn = -2*gamma*Radius/(m*pi*pi)*(2*alpha-sin(2*alpha))
  else:
    D_fluid_mn = -8*(gamma*Radius/(m*n*pi*pi*(n*n-1))*(cos(alpha)*sin(n*alpha)-n*cos(n*alpha)*(sin(alpha))))
    
  return D_fluid_mn

def evalResults(M, N, X_vec, Phi_vec, Length, Thickness, Radius, E, nu, Pressure, gamma, LiqHeight):
  
  results = Results()

  midDisplMap = []
  stressMap = []
  strainMap = []

  for X_idx in X_vec:
    row1 = []
    row2 = []
    row3 = []
    for Phi_idx in Phi_vec:
      row1.append(StressComponents())
      row2.append(StrainComponents())
      row3.append(MidDisplComponents())
    stressMap.append(row1)
    strainMap.append(row2)
    midDisplMap.append(row3)
 
  PI = pi
  
  beta = pow(3*(1-nu*nu)/(Radius*Radius*Thickness*Thickness), 0.25)
  DD = E*Thickness*Thickness*Thickness/(12*(1-nu*nu))

  if (Pressure != 0):
    alp = beta*Length/2
    PP = Pressure*Length*Length*Length*Length/(64*DD*alp*alp*alp*alp)
    QQ = 2*sin(alp)*sinh(alp)/(cos(2*alp)+cosh(2*alp))
    RR = 2*cos(alp)*cosh(alp)/(cos(2*alp)+cosh(2*alp))
    for X_idx in range(0, len(X_vec)):
      x = X_vec[X_idx] #  For hydrostatic solution x is (0, Length)  for Pressure x is (-Length/2,Length/2)
      for Phi_idx in range(0, len(Phi_vec)):
        phi = Phi_vec[Phi_idx]
        midDisplMap[X_idx][Phi_idx] = evalMidDispls_pres(x, beta, Length, Thickness, Radius, E, Pressure, PP, QQ, RR)
        stressMap[X_idx][Phi_idx] = evalStresses_pres(x, phi, Thickness, Radius, E, nu, Pressure, midDisplMap[X_idx][Phi_idx].w, beta, PP, QQ, RR) 
      
  if (gamma != 0) & (LiqHeight != 0):
    parm1 = pi/Length
    mStart = gCoefficients.getRowIndex()+2    
    for m in range(1, M+1, 2):
      parm2 = m*parm1
      nStart = gCoefficients.getColumnIndex(m)+1
      for n in range(nStart, N+1):
        D_fluid_mn = evalLoadCoeff_fluid_mn(Radius, gamma, LiqHeight, m, n)
        coeffs = evalCoefficents_fluid(m, n, D_fluid_mn, Length, Thickness, Radius, E, nu)
        gCoefficients.set(m, n, coeffs)
    
    if LiqHeight > Radius:
      alpha = pi-acos((LiqHeight-Radius)/Radius)
    else:
      alpha = acos((Radius-LiqHeight)/Radius)
       
    for X_idx in range(0, len(X_vec)):
      x = X_vec[X_idx] + Length/2 #  For hydrostatic solution x is (0, Length)  for Pressure x is (-Length/2,Length/2)
      for Phi_idx in range(0, len(Phi_vec)):
        phi = Phi_vec[Phi_idx] 
        for m in range(1, M+1, 2):
          for n in range(0, N+1):
            coeffs = gCoefficients.get(m,n)    
            disp_fluid_mn = evalMidDispls_fluid_mn(x, phi, Length, m, n, coeffs)
            midDisplMap[X_idx][Phi_idx].u += disp_fluid_mn.u 
            midDisplMap[X_idx][Phi_idx].v += disp_fluid_mn.v 
            midDisplMap[X_idx][Phi_idx].w += disp_fluid_mn.w 
            stresses_fluid_mn = evalStresses_fluid_mn(x, phi, Length, Thickness, Radius, E, nu, coeffs, m, n, D_fluid_mn)

            stressMap[X_idx][Phi_idx].sig_x_out     += stresses_fluid_mn.sig_x_out
            stressMap[X_idx][Phi_idx].sig_phi_out   += stresses_fluid_mn.sig_phi_out
            stressMap[X_idx][Phi_idx].sig_x_phi_out += stresses_fluid_mn.sig_x_phi_out
            stressMap[X_idx][Phi_idx].sig_x_in      += stresses_fluid_mn.sig_x_in
            stressMap[X_idx][Phi_idx].sig_phi_in    += stresses_fluid_mn.sig_phi_in
            stressMap[X_idx][Phi_idx].sig_x_phi_in  += stresses_fluid_mn.sig_x_phi_in

  for X_idx in range(0, len(X_vec)):
    for Phi_idx in range(0, len(Phi_vec)):
      sig_x     = stressMap[X_idx][Phi_idx].sig_x_out
      sig_phi   = stressMap[X_idx][Phi_idx].sig_phi_out
      sig_x_phi = stressMap[X_idx][Phi_idx].sig_x_phi_out
      stressMap[X_idx][Phi_idx].sig_eq_out = sqrt(sig_x*sig_x+sig_phi*sig_phi-sig_x*sig_phi+3*sig_x_phi*sig_x_phi)

      sig_x     = stressMap[X_idx][Phi_idx].sig_x_in
      sig_phi   = stressMap[X_idx][Phi_idx].sig_phi_in
      sig_x_phi = stressMap[X_idx][Phi_idx].sig_x_phi_in
      stressMap[X_idx][Phi_idx].sig_eq_in = sqrt(sig_x*sig_x+sig_phi*sig_phi-sig_x*sig_phi+3*sig_x_phi*sig_x_phi)

      strainMap[X_idx][Phi_idx].eps_x_out       = (stressMap[X_idx][Phi_idx].sig_x_out+nu*stressMap[X_idx][Phi_idx].sig_phi_out)/E
      strainMap[X_idx][Phi_idx].eps_phi_out     = (stressMap[X_idx][Phi_idx].sig_phi_out+nu*stressMap[X_idx][Phi_idx].sig_x_out)/E
      strainMap[X_idx][Phi_idx].gamma_x_phi_out = stressMap[X_idx][Phi_idx].sig_x_phi_out*2*(1+nu)/E
      strainMap[X_idx][Phi_idx].eps_x_in        = (stressMap[X_idx][Phi_idx].sig_x_in+nu*stressMap[X_idx][Phi_idx].sig_phi_in)/E
      strainMap[X_idx][Phi_idx].eps_phi_in      = (stressMap[X_idx][Phi_idx].sig_phi_in+nu*stressMap[X_idx][Phi_idx].sig_x_in)/E
      strainMap[X_idx][Phi_idx].gamma_x_phi_in  = stressMap[X_idx][Phi_idx].sig_x_phi_in*2*(1+nu)/E
 
  maxStressEqu = 0
  for X_idx in range(0, len(X_vec)):
    for Phi_idx in range(0, len(Phi_vec)):
        
      val = stressMap[X_idx][Phi_idx].sig_eq_out
      if (val > maxStressEqu):
        maxStressEqu = val
        X_ind_max_stress = X_idx
        Phi_ind_max_stress = Phi_idx
        surface_stress = OUT
        
      val = stressMap[X_idx][Phi_idx].sig_eq_in
      if (val > maxStressEqu):
        maxStressEqu = val
        X_ind_max_stress = X_idx
        Phi_ind_max_stress = Phi_idx
        surface_stress = IN
                
  results.midDisplMap = midDisplMap
  results.stressMap = stressMap
  results.strainMap = strainMap
  results.maxStressEqu = maxStressEqu
  results.X_ind_max_stress = X_ind_max_stress
  results.Phi_ind_max_stress = Phi_ind_max_stress
  results.surface_stress = surface_stress
 
  return results

def main(X_vec_in, Phi_vec_in, Pressure, Gamma, LiqHeight, E, Nu, Length, Radius, Thickness, M, N, summaryFileName,dataFileName):

# Process variables - if this function is called from python, the types should 
# already be consisitent. If called from command line, the types may be ambiguous
  if type(X_vec_in) is str:
    temp = [float(x) for x in X_vec_in.replace(","," ").replace(";"," ").split() ]
  elif type(X_vec_in) in [int, float]:
    temp = [X_vec_in]
  elif type(X_vec_in) is list:
    temp = X_vec_in
  else:
    print('Input failure: X_vec')
  X_vec = temp

  if type(Phi_vec_in) is str:
    temp = [float(x) for x in Phi_vec_in.replace(","," ").replace(";"," ").split() ]
  elif type(Phi_vec_in) in [int, float]:
    temp = [Phi_vec_in]
  elif type(Phi_vec_in) is list:
    temp = Phi_vec_in
  else:
    print('Input failure: Phi_vec')
  Phi_vec = temp

  Pressure = float(Pressure)
  Gamma = float(Gamma)
  LiqHeight = float(LiqHeight)

  E = float(E)
  Nu = float(Nu)

  Length = float(Length)
  Radius = float(Radius)
  Thickness = float(Thickness)

  M = int(float(M)/1)
  N = int(float(N)/1)


# some validation - not comprehensive
  if M > M_MAX or N > N_MAX:
    print('M must be less than ' +str(M_MAX)+ ', and N must be less than '+str(N_MAX))
    return 2

  results = evalResults(M, N, X_vec, Phi_vec, Length, Thickness, Radius, E, Nu, Pressure, Gamma, LiqHeight)

  # write out the results
  # default is to write four quantities of interest from the simulation
  # to the summary file, and an optional datafile with detailed information
  # about stress, strain, and displacement
  # In addition, two special cases exist that will skip the datafile,
  # and write out the displacements at specific locations to the summary file
  # if the dataFileName starts with Tanked, then this function is being called from FEMTank, and will not write out the true inputs
  if dataFileName: #if not empty
    if (dataFileName.startswith('Tanked#')):
      dataFileName = dataFileName.split('#')[1]
      if (dataFileName == 'DisplacementsForPressureOnly'):
        Cylinder_reports.reportDisplPonly(summaryFileName, results, X_vec, Phi_vec)
      elif (dataFileName == 'DisplacementsForPressureAndLiquid'):
        Cylinder_reports.reportDisplPandL(summaryFileName, results, X_vec, Phi_vec)
      elif dataFileName == '': 
        Cylinder_reports.resultsSummary(results, summaryFileName) # write summary file in a Dakota friendly format
      else: #if not a special case and not empty, complete the datafile and summaryFile
        Cylinder_reports.reportResults(dataFileName, results, X_vec, Phi_vec) 
        Cylinder_reports.resultsSummary(results, summaryFileName) # write summary file in a Dakota friendly format
    else: # normal reporting for datafile and summaryfile
      Cylinder_reports.reportInputs(dataFileName, X_vec, Phi_vec, Length, Thickness, Radius, E, Nu, Pressure, Gamma, LiqHeight, M, N, '') 
      Cylinder_reports.reportResults(dataFileName, results, X_vec, Phi_vec) #complete the datafile
  else: #summaryFile Only
    Cylinder_reports.resultsSummary(results, summaryFileName) # write summary file in a Dakota friendly format


#-------------------------------------------------------------------------------
 
# limit the maximum length of the expansion 
M_MAX = 25 # axial
N_MAX = 25 # circumferencial
           
gCoefficients = HydrostaticCoefficients(M_MAX, N_MAX)

if __name__ == "__main__":
  main()  

