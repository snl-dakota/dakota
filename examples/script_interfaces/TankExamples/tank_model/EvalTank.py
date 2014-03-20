#!/usr/bin/env python
# The major purpose of EvalTank.py is to set up the $x$ and $\varphi $ locations for 
# four common usages of FEMTank.py and generally make life easier.
# It utilizes Dakota friendly input/output via text files

# Usage: this is called from the command line or script.
# python EvalTank.py input_file_name summary_file_name data_file_name


# Arguments: 
## inputFileName - gives the parameter values (in dprepro format).
## summaryFileName - results file to be written (in a format suitable for Dakota)
## dataFileName - writes out detailed simulation results, in plain text
##   this can be large. This is optional, if nothing is passed, no datafile is written.
##   if an empty string "" is passed, no datafile is written


# The inputFile must have the following inputs: P Gamma_Chi H E Nu L R T meshID resultStyle
##  the names MUST be consistent, but the ordering does not matter
##  The first 9 inputs are described in FEMTank.py
##  Note that if either Gamma_Chi OR H is ommitted, they will both be set to 0 (pressure only scenario)

# resultStyle is used to select from four predetermined sets of responses
##  It selects the location inputs X_vec and Phi_vec, and sets flags to produce speciallized summaryFiles
##
##  resultStyle 1
###  Set up a fine grid of $x$ and $\varphi $,
###  This is suitable for visualizing the model responses, but takes longer to run
###  Normal reporting in summaryFile and dataFile
##
##  resultStyle 2
###  Set up a nonuniform grid of $x$ and $\varphi $, finest near the centerline and the support.
###  This is suitable for searching for the max von Mises stress, with less computational expense
###  Normal reporting in summaryFile and dataFile
##
##  resultStyle 3
###  Set $x$ and $\varphi $ to the nominal locations corresponding to Dataset 5 - Pressure only loading tests. See the problem statement.
###  Print only the four displacements to the summaryFile. No dataFile is written.
##
##  resultStyle 4
###  Set $x$and $\varphi $ to the nominal locations corresponding to Dataset 6 - Pressure and Liquid loading tests. See the problem statement.
###  Print only the 20 displacements to the summaryFile. No dataFile is written.
##
##  If other locations are required, use FEMTank.py directly.

from sys import argv
from subprocess import call
from math import pi

def main():
  if len(argv) < 3: 
    print('Usage: EvalTank.py input_file_name summary_file_name [data_file_name]')
    return 1
  elif len(argv) < 4: 
    dataFileName = ''
  else:
    dataFileName = argv[3]
  inputFileName = argv[1]
  summaryFileName = argv[2]

# Read inputs
  inputDict = {}
  with open(inputFileName, "r") as f:
    for line in f:
      if line.strip().split('#')[0].strip('\n'): # only parse non-empty lines, and ignore comments (#)
        tempLine = line.split('#')[0]
        if not ( 'DVV_' in tempLine or 'ASV_' in tempLine or tempLine.startswith('#')): # Dakota adds some extra information to the I/O files
          (valueString, name) = tempLine.split()
          name = name.strip()
          value = float(valueString) #fails if valueString is not a string of numbers... currently have no validation
          inputDict[name] = value
    f.close()

  Length = inputDict.get('L') #inches
  Thickness = inputDict.get('T') #inches
  Radius = inputDict.get('R') #inches

  E = inputDict.get('E') #Young's modulus (lb_f / inches ^2)
  Nu = inputDict.get('Nu') #Poisson's ratio

  Pressure = inputDict.get('P') #Internal Pressure (lb_f/in^2 gage)
  if (inputDict.get('H') != None) and (inputDict.get('Gamma_Chi') != None):
    LiqHeight = inputDict.get('H') #height of liquid (inches)
    Gamma_Chi = inputDict.get('Gamma_Chi') #liquid specific weight (lb_f / inches^3)
  else:
    LiqHeight = 0
    Gamma_Chi = 0

  meshID = inputDict.get('meshID')
  if meshID == 24: #flag to run "untanked" cylinder, must also specify M,N
    if (inputDict.get('N') != None) and (inputDict.get('M') != None):
      N = inputDict.get('N')
      M = inputDict.get('M')
    else:
      print('When using special meshID flag, you must specify M and N values')
      return 24

  resultStyle = inputDict.get('resultStyle')

# Observations
  # x=0 is at the centerline
  if resultStyle == 1: #high fidelity, for plotting, etc.
    nX=101
    dx=Length/(nX-1)/2 # Half-plane symmetry
    X_vec = [idx*dx for idx in range(0,nX)]
    nPhi=91
    dPhi=pi/(nPhi-1)
    Phi_vec = [idx*dPhi for idx in range(0,nPhi)]
    if dataFileName == '':
      dataFileName = summaryFileName + '.data' 

  elif resultStyle == 2: # balance run time and accuracy of the max statistic
    nX=51 
    dx=Length/(nX-1)/2 # Half-plane symmetry
    X_vec = [idx*dx for idx in range(0,int(nX*1/8))]+[idx*dx for idx in range(int(nX*5/8),nX)] #all the action is at the end or near the middle.
    nX=len(X_vec)
    nPhi=61
    dPhi=pi/(nPhi-1)
    Phi_vec = [idx*dPhi for idx in range(0,int(nPhi*5/6))] #exclude the top 5 inches
    nX=len(X_vec)

  elif resultStyle == 3: # for Pressure Only Tests
    X_vec = [0, 27, 28, 29]
    nX=len(X_vec)
    Phi_vec = [pi/6, pi/2, pi/4, 3*pi/4]
    nPhi=len(Phi_vec)
    dataFileName = 'DisplacementsDataOnDiagonal'

  elif resultStyle == 4: # for Validation tests
    X_vec = [0, 15, 20, 25]
    nX=len(X_vec)
    Phi_vec = [pi/6, pi/3, pi/2, 2*pi/3, 5*pi/6]
    nPhi=len(Phi_vec)
    dataFileName = 'DisplacementsDataFullGrid'

  else:
    print("resultStyle must be 1, 2, 3, or 4")
    return 404

  import FEMTank
  if (meshID != 24):
    # Run the tank (skewed cylinder)
    FEMTank.main(X_vec, Phi_vec, Pressure, Gamma_Chi, LiqHeight, E, Nu, Length, Radius, Thickness, meshID, summaryFileName, dataFileName)
  else:
    # Run the cylinder
    FEMTank.cylinder(X_vec, Phi_vec, Pressure, Gamma_Chi, LiqHeight, E, Nu, Length, Radius, Thickness, M, N, summaryFileName, dataFileName)

if __name__ == "__main__":
  main()  
