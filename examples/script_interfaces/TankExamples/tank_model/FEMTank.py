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
##  3.	P		Scalar, bounds $\left[ 0,\infty  \right]$  (atm), (gage pressure)
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


import Cylinder_reports
import Cylinder
from sys import argv
from subprocess import call
#--------------------------------------------------------------------------------------------

def skewInputs(Pressure_orig, Gamma_orig, LiqHeight_orig, E_orig, Nu_orig, Length_orig, Radius_orig, Thickness_orig, meshID):
#biases applied to the input variable values, to bring them in line with the true variable values, which are evaluated in the physics model
# change the sensitivity with a multiplier on the input variable
# skew the input vs. meshID using a Mesh Convergence Parameters
# compute new parameter values

  if ( Gamma_orig * LiqHeight_orig)  == 0:
    meshID = meshID + 3 #greatly reduce mesh dependence for pressure only scenario

  biasLength = 0
  magLength = 1
  mcpLength = 1.01
  mcp2Length = 2/float(meshID)
  Length_new = (Length_orig*magLength+biasLength)*pow(mcpLength, mcp2Length)

  # convert 28-32 to 30-36
  biasRadius = -12
  magRadius = 1.5
  mcpRadius = 0.99
  mcp2Radius = 2/float(meshID)
  Radius_new = (Radius_orig*magRadius+biasRadius)*pow(mcpRadius, mcp2Radius)

  # convert 0.2~0.25 to 0.20~0.5
  biasThickness = -1
  magThickness = 6
  mcpThickness = 0.996
  mcp2Thickness = 4/float(meshID)
  Thickness_new = (Thickness_orig*magThickness+biasThickness)*pow(mcpThickness, mcp2Thickness)

  LiqHeight_new = LiqHeight_orig

  biasPressure = 0
  magPressure = 1+(Nu_orig-0.25)/2
  mcpPressure = 1.005
  mcp2Pressure = 2/float(meshID)
  Pressure_new = (Pressure_orig*magPressure+biasPressure)*pow(mcpPressure, mcp2Pressure)

  # convert 2.6~3.2 to 1.5~3.5
  if Gamma_orig == 0:
    Gamma_new = 0
  else:
    biasGamma = -2.3
    magGamma = 1.8
    mcpGamma = 0.999
    mcp2Gamma = 2/float(meshID)
    Gamma_new = (Gamma_orig*magGamma+biasGamma)*pow(mcpGamma, mcp2Gamma)

  # convert 2.6~3e7 to 2~6e6
  biasE = -24e6
  magE = 1
  mcpE = 1.02
  mcp2E = 2/float(meshID)
  E_new = (E_orig*magE+biasE)*pow(mcpE, mcp2E)

  # convert 0.24~0.34 to 0.28~0.355
  Nu_new = Nu_orig
  biasNu = 0.1
  magNu = 0.75
  Nu_new = (Nu_orig*magNu+biasNu)

  return ( Pressure_new, Gamma_new, LiqHeight_new, E_new, Nu_new, Length_new, Radius_new, Thickness_new)

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
  M = meshID*3 + 10
  N = meshID*4 + 7

  [Pressure_new, Gamma_new, LiqHeight_new, E_new, Nu_new, Length_new, Radius_new, Thickness_new] = skewInputs(Pressure_orig, Gamma_orig, LiqHeight_orig, E_orig, Nu_orig, Length_orig, Radius_orig, Thickness_orig, meshID)

# also rescale the x position, since the length of tank may have changed. angle does not change
  rescale = Length_orig / Length_orig
  X_vec_new = [x * rescale for x in X_vec_orig]
  Phi_vec_new = Phi_vec_orig

  # Report out the original inputs, but run the skewed inputs.
  TankedDFN='Tanked#'+dataFileName #signal to Cylinder that it is being called from this function
  if dataFileName and (not dataFileName.startswith('DisplacementsForP')):
    # Only print out if dataFileName is not empty, and is not a special flag value
    Cylinder_reports.reportInputs(dataFileName, X_vec_orig, Phi_vec_orig, Length_orig, Thickness_orig, Radius_orig, E_orig, Nu_orig, Pressure_orig, Gamma_orig, LiqHeight_orig, M, N, meshID)
  Cylinder.main(X_vec_new, Phi_vec_new, Pressure_new, Gamma_new, LiqHeight_new, E_new, Nu_new, Length_new, Radius_new, Thickness_new, M, N, summaryFileName,TankedDFN)
# for debugging, you can write out the true (original) parameter values
#  Cylinder_reports.reportInputs(dataFileName + 'true', X_vec_new, Phi_vec_new, Length_new, Thickness_new, Radius_new, E_new, Nu_new, Pressure_new, Gamma_new, LiqHeight_new, M, N, '')

if __name__ == "__main__":
  main()  
