#!/usr/bin/env python 
# FEMTank
# This is an intermediate function that modifies the cylinder model into a "tank" model
# it skews the parameters, in order to act as a proxy for a Finite Element model

# usage from python script:
## import FEMTank
## FEMTank.main(X_vec, Phi_vec, P, (Gamma, -Chi), H, E, Nu, L, R, T, m, summaryFile, dataFile)

# usage example from commandline:
## python -c"import FEMTank; FEMTank.main(3, [0.9,1.1], 10, 3.2, 12, 3e7, .3, 59, 30, 0.25, 1, 'summary', 'data')"

# Arguments are:
##  1.	X_vec 		$1\times {{N}_{x}}$ vector, passed as comma delimited string, bounds $\left[ 0,\frac{L}{2} \right]$  (in)
##  2.	Phi_vec 	$1\times {{N}_{\varphi }}$ vector, passed as comma delimited string, bounds $\left[ 0,180 \right]$  (o)
##			The code computes responses at all combinations of $x,\varphi$ to produce ${N_x}\times {N_\varphi}$ locations
##  3.	P		Scalar, bounds $\left[ 0,\infty  \right]$  (psig)
##  4.	Gamma, -Chi	Scalar - Liquid specific weight OR composition
##  			Positive numbers interpreted as specific weight, bounds $\left[ 0,\infty  \right]$ (lbs/in3)
##  			Negative numbers $\left[ -1,0 \right]$ interpreted as negative composition
##  5.	H		Scalar (zero in the pressure only scenario) , bounds $\left[ 0,2R \right]$ (in)
##  6.	E		Scalar, bounds $\left[ 2.6e6,3.0e6 \right]$ (psi)
##  7.	Nu 		Scalar, bounds $\left[ 0.2,0.5 \right]$
##  8.	L 		Scalar, bounds $\left[ 0,\infty  \right]$ (in)
##  9.	R 		Scalar, bounds $\left[ 0,\infty  \right]$ (in)
##  10.	T		Scalar, bounds $\left[ 0,\infty  \right]$ (in)
##  11.	m		Mesh ID (choose from 1,2,3,4)
##  12.	summaryFile 	name of summary file, This writes out: 
##  	a.	Maximum von Mises Stress
##  	b.	$x$  index (note that python starts indexing from 0)
##  	c.	$\varphi $ index (note that python starts indexing from 0)
##  	d.	Surface - inner (-1) or outer (1)
##  13.	dataFile	Name of data file - if empty string, no dataFile is written. This writes out:
##  	a.	All inputs
##  	b.	Locations ($x$ and $\varphi $ )
##  	c.	Comma separated matrices for normal displacement (in), and stresses (psi) on the outside and inside surface of the tank $\left[ {{N}_{x}}\times {{N}_{\varphi }} \right]$ 


from sys import argv
from subprocess import call
from math import pi, sin, cos, acos, sinh, cosh, sqrt

#--------------------------------------------------------------------------------------------
#  Cylinder Report Functions
#--------------------------------------------------------------------------------------------
# Various reporting options
# Notes:
# 1) compute phi in radians, but report in degrees
# 2) compute transverse displacement w/ positive inward, but report postive outward

def reportShort(fileName, results, X_vec, Phi_vec):
  # Dakota friendly output for strains. not currently used. need to be careful with ordering of resposnes
  f = open(fileName, 'w')
  for X_idx in range(0, len(X_vec)):
    for Phi_idx in range(0, len(Phi_vec)):
      f.write('%.6g\n' % (results.strainMap[X_idx][Phi_idx].eps_phi_out))
  f.close
            
def reportDisplPonly(fileName, results, X_vec, Phi_vec):
  # Dakota friendly output
  f = open(fileName, 'w')
  for XPhi_idx in range(0, len(X_vec)):
    f.write('%.6g\n' % (-1*results.midDisplMap[XPhi_idx][XPhi_idx].w))
  f.close

def reportDisplPandL(fileName, results, X_vec, Phi_vec):
  # Dakota friendly output
  f = open(fileName, 'w')
  for X_idx in range(0, len(X_vec)):
    for Phi_idx in range(0, len(Phi_vec)):
      f.write('%.6g\n' % (-1*results.midDisplMap[X_idx][Phi_idx].w))
  f.close

def reportInputs(fileName, X_vec, Phi_vec, Length, Thickness, Radius, E, nu, Pressure, gamma, LiqHeight, M, N, meshID):
  # text file with all the inputs, can handle either the Tank or Cylinder (meshID==0)
  f = open(fileName, 'w')
  f.write('Tank Results\n')
  f.write('Pressure  = %s\n' % (Pressure))
  f.write('Gamma     = %s\n' % (gamma))
  f.write('LiqHeight = %s\n' % (LiqHeight))
  f.write('E         = %s\n' % (E))
  f.write('Nu        = %s\n' % (nu))
  f.write('Length    = %s\n' % (Length))
  f.write('Radius    = %s\n' % (Radius))
  f.write('Thickness = %s\n' % (Thickness))
  f.write('nX        = %s\n' % (len(X_vec)))
  f.write('nPhi      = %s\n' % (len(Phi_vec)))
  if meshID == '':
    f.write('M         = %s\n' % (M))
    f.write('N         = %s\n' % (N))
  else:
    f.write('meshID    = %s\n' % (meshID))

  f.write('\n%s\n' % ('Distance from Centerline toward Supports (Inches)'))
  for x in X_vec: 
    f.write('%.6f%s' % (x, ','))
  f.write('\n%s\n' % ('Phi values (degrees)'))
  for phi in Phi_vec:
    f.write('%.6f%s' % (phi*180/pi, ','))

def reportResults(fileName, results, X_vec, Phi_vec):
  # text file with all the results
  # note that this APPENDS to the file created by reportInputs, for historical reasons.
  f = open(fileName, 'a')
#  f.write('\n%s' % ('Axial Displacement, u')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.midDisplMap[X_idx][Phi_idx].u, ','))
#  f.write('\n%s' % ('Circumferential Displacement, v')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.midDisplMap[X_idx][Phi_idx].v, ','))
  f.write('\n%s' % ('Normal Displacement, w')) 
  for X_idx in range(0, len(X_vec)):
    f.write('\n')
    for Phi_idx in range(0, len(Phi_vec)):
      f.write('%.6g%s' % (-1*results.midDisplMap[X_idx][Phi_idx].w, ','))

#  f.write('\n%s' % ('eps_x (Outboard)')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.strainMap[X_idx][Phi_idx].eps_x_out, ','))
#  f.write('\n%s' % ('eps_phi (Outboard)')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.strainMap[X_idx][Phi_idx].eps_phi_out, ','))
#  f.write('\n%s' % ('gamma_x_phi (Outboard)')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.strainMap[X_idx][Phi_idx].gamma_x_phi_out, ','))
    
#  f.write('\n%s' % ('eps_x (Inboard)')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.strainMap[X_idx][Phi_idx].eps_x_in, ','))
#  f.write('\n%s' % ('eps_phi (Inboard)')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.strainMap[X_idx][Phi_idx].eps_phi_in, ','))
#  f.write('\n%s' % ('gamma_x_phi (Inboard)')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.strainMap[X_idx][Phi_idx].gamma_x_phi_in, ','))
    
#  f.write('\n%s' % ('sig_x (Outboard)')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.stressMap[X_idx][Phi_idx].sig_x_out, ','))
#  f.write('\n%s' % ('sig_phi (Outboard)')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.stressMap[X_idx][Phi_idx].sig_phi_out, ','))
#  f.write('\n%s' % ('sig_x_phi (Outboard)')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.stressMap[X_idx][Phi_idx].sig_x_phi_out, ','))

  f.write('\n%s' % ('EFFECTIVE STRESSES (Outboard)')) 
  for X_idx in range(0, len(X_vec)):
    f.write('\n')
    for Phi_idx in range(0, len(Phi_vec)):
      f.write('%.6g%s' % (results.stressMap[X_idx][Phi_idx].sig_eq_out, ','))
#  f.write('\n%s' % ('sig_x (Inboard)')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.stressMap[X_idx][Phi_idx].sig_x_in, ','))
#  f.write('\n%s' % ('sig_phi (Inboard)')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.stressMap[X_idx][Phi_idx].sig_phi_in, ','))
#  f.write('\n%s' % ('sig_x_phi (Inboard)')) 
#  for X_idx in range(0, len(X_vec)):
#    f.write('\n')
#    for Phi_idx in range(0, len(Phi_vec)):
#      f.write('%.6g%s' % (results.stressMap[X_idx][Phi_idx].sig_x_phi_in, ','))

  f.write('\n%s' % ('EFFECTIVE STRESSES (Inboard)')) 
  for X_idx in range(0, len(X_vec)):
    f.write('\n')
    for Phi_idx in range(0, len(Phi_vec)):
      f.write('%.6g%s' % (results.stressMap[X_idx][Phi_idx].sig_eq_in, ','))
    
  f.close
 
def resultsSummary(results, X_vec, Phi_vec, outputFileName):
  # Dakota friendly result file, with value and location of max stress.
  f = open(outputFileName, "w")
  f.write('%.10e %s \n' % (results.maxStressEqu, 'max_stress_equ'))
  f.write('%.10e %s \n' % (X_vec[results.X_ind_max_stress], 'X_max_stress'))
  f.write('%.10e %s \n' % (Phi_vec[results.Phi_ind_max_stress], 'Phi_max_stress'))
  f.write('%i %s \n' % (results.surface_stress, 'surface_max_stress')) #report as -1 1 b/c Dakota only likes numerical responses.
  f.write('\n')
  f.close()



#--------------------------------------------------------------------------------------------
#  Cylinder calculations
#--------------------------------------------------------------------------------------------
# classes and functions for computing the cylinder solution

# usage from python script:
## import FEMTank
## FEMTank.cylinder(X_vec, Phi_vec, P, Gamma, H, E, Nu, L, R, T, M, N, summaryFile, dataFile)

# usage example from commandline:
## python -c"import FEMTank; FEMTank.cylinder(13, [1,1.1], 10, 3.2, 12, 3e7, .3, 59, 30, 0.25, 19, 13, 'summary', 'data')"

# This computes the stresses, strains, and displacements on a cylinder with
# simple supports on the ends, while it has liquid and pressure loading

# REFERENCE
## The series solution is adapted from: S. Timoshenko, S. Woinowsky-Krieger: Theory of Plates and Shells.  McGraw-Hill (1987). 
## https://ia700807.us.archive.org/34/items/TheoryOfPlatesAndShells/TheoryOfPlatesAndShellsS.timoshenko2ndEdition.pdf

# Identifiers for the tank surfaces and directions
U = 1 #Axial
V = 2 #Radial
W = 3 #Transverse
OUT = 1 #outer surface of the cylinder wall
IN = -1 #inner suface

class cylHydrostaticCoefficients_mn:
  def __init__(self):
    self.A_fluid_mn = float('NaN')
    self.B_fluid_mn = float('NaN')
    self.C_fluid_mn = float('NaN')
                
  def __str__(self):
    return 'A_fluid_mn='+str(self.A_fluid_mn)+'\n'+'B_fluid_mn='+str(self.B_fluid_mn)+'\n'+'C_fluid_mn='+str(self.C_fluid_mn)+'\n'

class cylHydrostaticCoefficients:
  def __init__(self, Mmax, Nmax):
    self.coefficients=[]
    self.columnIndex=[]
     
    for m in range (0, Mmax):
      self.columnIndex.append(-1)
      row=[]
      for n in range(0, Nmax):
        row.append(cylHydrostaticCoefficients_mn())
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

class cylMidDisplComponents:
  def __init__(self):
    self.u = 0
    self.v = 0
    self.w = 0
    
class cylStrainsAndCurvatures:
  def __init__(self):
    self.ep_x_0 = float('NaN')
    self.ep_phi_0 = float('NaN')
    self.gam_x_phi_0 = float('NaN')
    self.kappa_x = float('NaN')
    self.kappa_phi = float('NaN')
    self.kappa_x_phi = float('NaN')
   
class cylStressComponents:
  def __init__(self):
    self.sig_x_out = 0.0
    self.sig_phi_out = 0.0
    self.sig_x_phi_out = 0.0
    self.sig_eq_out = 0.0
    self.sig_x_in = 0.0
    self.sig_phi_in = 0.0
    self.sig_x_phi_in = 0.0
    self.sig_eq_in = 0.0    

class cylStrainComponents:
  def __init__(self):
    self.eps_x_out = 0.0
    self.eps_phi_out = 0.0
    self.gamma_x_phi_out = 0.0
    self.eps_x_in = 0.0
    self.eps_phi_in = 0.0
    self.gamma_x_phi_in = 0.0

    
class cylResults:
  def __init__(self):
    midDisplMap = []
    stressMap = []
    strainMap = []
    maxStressEqu = 0
    X_ind_max_stress = 0
    Phi_ind_max_stress = 0
    surface_stress = 0

def cylEvalCoefficents_fluid(m, n, D_fluid_mn, Length, Thickness, Radius, E, nu):
  coeffs = cylHydrostaticCoefficients_mn()
  
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
  
def cylEvalMidDispls_fluid_mn(x, phi, Length, m, n, coeffs):
  disp = cylMidDisplComponents()
  
  A_fluid_mn = coeffs.A_fluid_mn
  B_fluid_mn = coeffs.B_fluid_mn
  C_fluid_mn = coeffs.C_fluid_mn
  
  angularArg = n*phi
  axialArg = m*pi*x/Length
  disp.u = A_fluid_mn*cos(angularArg)*cos(axialArg)
  disp.v = B_fluid_mn*sin(angularArg)*sin(axialArg)
  disp.w = C_fluid_mn*cos(angularArg)*sin(axialArg)
  
  return disp

def cylEvalMidDispls_pres(x, beta, Length, Thickness, Radius, E, Pressure, PP, QQ, RR):
  disp = cylMidDisplComponents()

  disp.u = 0 
  disp.v = 0
  disp.w = -PP*(1-QQ*sin(beta*x)*sinh(beta*x)-RR*cos(beta*x)*cosh(beta*x))
  return disp

def cylEvalStrainsAndCurvatures_pres(x, phi, beta, Radius, nu, w, PP, QQ, RR):
  sc = cylStrainsAndCurvatures()
  
  sc.ep_phi_0 = PP*(-cos(beta*x)*cosh(beta*x)*RR-sin(beta*x)*sinh(beta*x)*QQ+1)/Radius
  sc.gam_x_phi_0 = 0
  sc.ep_x_0 = nu*w/Radius
  sc.kappa_x = -PP*(2*beta**2*sin(beta*x)*sinh(beta*x)*RR-2*beta**2*cos(beta*x)*cosh(beta*x)*QQ)
  sc.kappa_phi = 0
  sc.kappa_x_phi = 0
    
  return sc

def cylEvalStrainsAndCurvatures_fluid_mn(x, phi, Length, Radius, coeffs, m, n, q_fluid_mn):
  sc = cylStrainsAndCurvatures()
  
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

def cylEvalStresses_pres(x, phi, Thickness, Radius, E, nu, Pressure, w, beta, PP, QQ, RR):
  sig = cylStressComponents()
  
  sc = cylEvalStrainsAndCurvatures_pres(x, phi, beta, Radius, nu, w, PP, QQ, RR)
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

def cylEvalStresses_fluid_mn(x, phi, Length, Thickness, Radius, E, nu, coeffs, m, n, q_fluid_mn):
  sig_fluid_mn = cylStressComponents()
  
  sc = cylEvalStrainsAndCurvatures_fluid_mn(x, phi, Length, Radius, coeffs, m, n, q_fluid_mn)
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

def cylEvalLoadCoeff_fluid_mn(Radius, gamma, LiqHeight, m, n):
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

def cylEvalResults(M, N, X_vec, Phi_vec, Length, Thickness, Radius, E, nu, Pressure, gamma, LiqHeight):
  
  results = cylResults()

  midDisplMap = []
  stressMap = []
  strainMap = []

  for X_idx in X_vec:
    row1 = []
    row2 = []
    row3 = []
    for Phi_idx in Phi_vec:
      row1.append(cylStressComponents())
      row2.append(cylStrainComponents())
      row3.append(cylMidDisplComponents())
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
        midDisplMap[X_idx][Phi_idx] = cylEvalMidDispls_pres(x, beta, Length, Thickness, Radius, E, Pressure, PP, QQ, RR)
        stressMap[X_idx][Phi_idx] = cylEvalStresses_pres(x, phi, Thickness, Radius, E, nu, Pressure, midDisplMap[X_idx][Phi_idx].w, beta, PP, QQ, RR) 
      
  if (gamma != 0) & (LiqHeight != 0):
    parm1 = pi/Length
    mStart = gCoefficients.getRowIndex()+2    
    for m in range(1, M+1, 2):
      parm2 = m*parm1
      nStart = gCoefficients.getColumnIndex(m)+1
      for n in range(nStart, N+1):
        D_fluid_mn = cylEvalLoadCoeff_fluid_mn(Radius, gamma, LiqHeight, m, n)
        coeffs = cylEvalCoefficents_fluid(m, n, D_fluid_mn, Length, Thickness, Radius, E, nu)
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
            disp_fluid_mn = cylEvalMidDispls_fluid_mn(x, phi, Length, m, n, coeffs)
            midDisplMap[X_idx][Phi_idx].u += disp_fluid_mn.u 
            midDisplMap[X_idx][Phi_idx].v += disp_fluid_mn.v 
            midDisplMap[X_idx][Phi_idx].w += disp_fluid_mn.w 
            stresses_fluid_mn = cylEvalStresses_fluid_mn(x, phi, Length, Thickness, Radius, E, nu, coeffs, m, n, D_fluid_mn)

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

def cylinder(X_vec_in, Phi_vec_in, Pressure, Gamma, LiqHeight, E, Nu, Length, Radius, Thickness, M, N, summaryFileName, dataFileName):

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

  results = cylEvalResults(M, N, X_vec, Phi_vec, Length, Thickness, Radius, E, Nu, Pressure, Gamma, LiqHeight)

  # Write results files
  # default is to write four quantities of interest from the simulation
  # to the summary file, and an optional datafile with detailed information
  # about stress, strain, and displacement
  # In addition, two special cases exist that will skip the datafile,
  # and write out the displacements at specific locations to the summary file
  if (dataFileName == 'DisplacementsDataOnDiagonal'): #special case 3
    reportDisplPonly(summaryFileName, results, X_vec, Phi_vec)
  elif (dataFileName == 'DisplacementsDataFullGrid'): #special case 4
    reportDisplPandL(summaryFileName, results, X_vec, Phi_vec)
  elif dataFileName == '': # no datafile --> only write out the summary file, standard
    if summaryFileName:
      resultsSummary(results, X_vec, Phi_vec, summaryFileName) # write summary file in a Dakota friendly format
  else: #if not a special case and not empty, complete the datafile and summaryFile
    reportInputs(dataFileName, X_vec, Phi_vec, Length, Thickness, Radius, E, Nu, Pressure, Gamma, LiqHeight, M, N, '') 
    reportResults(dataFileName, results, X_vec, Phi_vec) #complete the datafile
    if summaryFileName:
      resultsSummary(results, X_vec, Phi_vec, summaryFileName) # write summary file in a Dakota friendly format


  return results

#-------------------------------------------------------------------------------
# Global vars for Cylinder 
# limit the maximum length of the expansion 
M_MAX = 45 # axial
N_MAX = 45 # circumferencial
           
gCoefficients = cylHydrostaticCoefficients(M_MAX+1, N_MAX+1)







#--------------------------------------------------------------------------------------------
# FEM Tank Functions
#--------------------------------------------------------------------------------------------

def skewInputs(Pressure_orig, Gamma_orig, LiqHeight_orig, E_orig, Nu_orig, Length_orig, Radius_orig, Thickness_orig, meshID):
#biases applied to the input variable values, to bring them in line with the true variable values, which are evaluated in the physics model
# change the sensitivity with a multiplier on the input variable
# skew the input vs. meshID using a Mesh Convergence Parameters
# compute new parameter values

# handle biases and skew differently for Ponly and PandL scenarios
  if ( Gamma_orig * LiqHeight_orig)  == 0: # pressure only scenario
    meshID = meshID + 2 

    biasLength = 0
    magLength = 1
    mcpLength = 1.005
    mcp2Length = 6/float(meshID)
    Length_new = (Length_orig*magLength+biasLength)*pow(mcpLength, mcp2Length)

    # convert 28-32 to 25-31
    biasRadius = -17
    magRadius = 1.5
    mcpRadius = 0.992
    mcp2Radius = 6/float(meshID)
    Radius_new = (Radius_orig*magRadius+biasRadius)*pow(mcpRadius, mcp2Radius)

    # convert 0.2~0.25 to 0.15~0.3
    biasThickness = -0.45
    magThickness = 3
    mcpThickness = 0.996
    mcp2Thickness = 6/float(meshID)
    Thickness_new = (Thickness_orig*magThickness+biasThickness)*pow(mcpThickness, mcp2Thickness)

    LiqHeight_new = LiqHeight_orig #don't mess with height, will be too obvious

    biasPressure = 0
    magPressure = 0.9-(Nu_orig-0.25)*2
    mcpPressure = 1.005
    mcp2Pressure = 7/float(meshID)
    Pressure_new = (Pressure_orig*magPressure+biasPressure)*pow(mcpPressure, mcp2Pressure)

    Gamma_new = 0

    # convert 26~30e6 to 15~23e6
    biasE = -37e6
    magE = 2
    mcpE = 1.012
    mcp2E = 6/float(meshID)
    E_new = (E_orig*magE+biasE)*pow(mcpE, mcp2E)

    # convert 0.24~0.34 to 0.28~0.355
    Nu_new = Nu_orig
    biasNu = 0.1
    magNu = 0.75
    Nu_new = (Nu_orig*magNu+biasNu)

  else: # pressure and liquid loading
    biasLength = 0
    magLength = 1
    mcpLength = 1.01
    mcp2Length = 4/float(meshID)
    Length_new = (Length_orig*magLength+biasLength)*pow(mcpLength, mcp2Length)

    # convert 28-32 to 25-31
    biasRadius = -17
    magRadius = 1.5
    mcpRadius = 0.99
    mcp2Radius = 4/float(meshID)
    Radius_new = (Radius_orig*magRadius+biasRadius)*pow(mcpRadius, mcp2Radius)

    # convert 0.2~0.25 to 0.22~0.32
    biasThickness = -0.18
    magThickness = 2
    mcpThickness = 0.993
    mcp2Thickness = 5/float(meshID)
    Thickness_new = (Thickness_orig*magThickness+biasThickness)*pow(mcpThickness, mcp2Thickness)

    LiqHeight_new = LiqHeight_orig #don't mess with height, will be too obvious

    biasPressure = 0
    magPressure = 0.9
    mcpPressure = 1.05
    mcp2Pressure = 5/float(meshID)
    Pressure_new = (Pressure_orig*magPressure+biasPressure)*pow(mcpPressure, mcp2Pressure)

    # convert 2.6~3.2 to 1.7~2.9
    biasGamma = -3.5
    magGamma = 2
    mcpGamma = 0.99
    mcp2Gamma = 5/float(meshID)
    Gamma_new = (Gamma_orig*magGamma+biasGamma)*pow(mcpGamma, mcp2Gamma)

    # convert 26~30e6 to 10~22e6
    biasE = -68e6
    magE = 3
    mcpE = 1.02
    mcp2E = 4/float(meshID)
    E_new = (E_orig*magE+biasE)*pow(mcpE, mcp2E)

    # convert 0.24~0.34 to 0.28~0.355
    Nu_new = Nu_orig
    biasNu = 0.1
    magNu = 0.75
    Nu_new = (Nu_orig*magNu+biasNu)


  M = meshID*4 + 8
  N = meshID*4 + 7

  return ( Pressure_new, Gamma_new, LiqHeight_new, E_new, Nu_new, Length_new, Radius_new, Thickness_new, M, N)

def main(X_vec_in, Phi_vec_in, Pressure, Gamma_Chi, LiqHeight, E, Nu, Length, Radius, Thickness, meshID, summaryFileName, dataFileName):

# Process variables - if this function is called from python, the types should 
# already be consisitent. If called from command line, the types may be ambiguous
  if type(X_vec_in) is list:
    X_vec_orig = X_vec_in
  elif type(X_vec_in) in [int, float]:
    X_vec_orig = [X_vec_in]
  elif type(X_vec_in) is str:
    X_vec_orig = [float(x) for x in X_vec_in.replace(","," ").replace(";"," ").split() ]
  else:
    print('Input failure: X_vec')
    return 6

  if type(Phi_vec_in) is list:
    Phi_vec_orig = Phi_vec_in
  elif type(Phi_vec_in) in [int, float]:
    Phi_vec_orig = [Phi_vec_in]
  elif type(Phi_vec_in) is str:
    Phi_vec_orig = [float(x) for x in Phi_vec_in.replace(","," ").replace(";"," ").split() ]
  else:
    print('Input failure: Phi_vec')
    return 5

  Pressure_orig = float(Pressure)

  if Gamma_Chi < 0:
    Chi = - Gamma_Chi
    Dfun_Chi_0 = 0.3
    Dfun_gamma_0 = 7
    Dfun_a = -8
    Dfun_b = 0.25
    Dfun_c = 5
    
    Gamma_orig = Dfun_gamma_0 * Chi / (1 + Dfun_b * pow(Chi - Dfun_Chi_0, 2) ) + Dfun_a * pow(Chi, 0.5) + Dfun_c
  else:
    Gamma_orig = Gamma_Chi

  LiqHeight_orig = float(LiqHeight)

  E_orig = float(E)
  Nu_orig = float(Nu)

  Length_orig = float(Length)
  Radius_orig = float(Radius)
  Thickness_orig = float(Thickness)

  meshID = int(float(meshID)/1)

# meshID has allowable values
  if 1 > meshID or meshID > 4:
    print("meshID must be 1, 2, 3 or 4")
    return 404

# convert meshID into expansion lengths

  [Pressure_new, Gamma_new, LiqHeight_new, E_new, Nu_new, Length_new, Radius_new, Thickness_new, M, N] = skewInputs(Pressure_orig, Gamma_orig, LiqHeight_orig, E_orig, Nu_orig, Length_orig, Radius_orig, Thickness_orig, meshID)

# also rescale the x position, since the length of tank may have changed. angle does not change
  rescale = Length_orig / Length_orig
  X_vec_new = [x * rescale for x in X_vec_orig]
  Phi_vec_new = Phi_vec_orig

  # Run the skewed inputs.
  results = cylinder(X_vec_new, Phi_vec_new, Pressure_new, Gamma_new, LiqHeight_new, E_new, Nu_new, Length_new, Radius_new, Thickness_new, M, N, '','') # don't have cylinder write any files

  # Write results files
  # default is to write four quantities of interest from the simulation
  # to the summary file, and an optional datafile with detailed information
  # about stress, strain, and displacement
  # In addition, two special cases exist that will skip the datafile,
  # and write out the displacements at specific locations to the summary file
  if (dataFileName == 'DisplacementsDataOnDiagonal'): #special case 3
    reportDisplPonly(summaryFileName, results, X_vec_orig, Phi_vec_orig)
  elif (dataFileName == 'DisplacementsDataFullGrid'): #special case 4
    reportDisplPandL(summaryFileName, results, X_vec_orig, Phi_vec_orig)
  elif dataFileName == '': # no datafile --> only write out the summary file, standard
    resultsSummary(results, X_vec_orig, Phi_vec_orig, summaryFileName) # write summary file in a Dakota friendly format
  else: #if not a special case and not empty, complete the datafile and summaryFile
    reportInputs(dataFileName, X_vec_orig, Phi_vec_orig, Length_orig, Thickness_orig, Radius_orig, E_orig, Nu_orig, Pressure_orig, Gamma_orig, LiqHeight_orig, M, N, meshID)
    reportResults(dataFileName, results, X_vec_orig, Phi_vec_orig) 
    resultsSummary(results, X_vec_orig, Phi_vec_orig, summaryFileName) # write summary file in a Dakota friendly format
    # for debugging, you can write out the true (skewed) parameter values
#    reportInputs(dataFileName + '.true', X_vec_new, Phi_vec_new, Length_new, Thickness_new, Radius_new, E_new, Nu_new, Pressure_new, Gamma_new, LiqHeight_new, M, N, '')

if __name__ == "__main__":
  main()  
