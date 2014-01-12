#!/usr/bin/env python 
# This module has functions for reporting the results from Cylinder.py

from math import pi

# Notes:
# 1) compute phi in radians, but report in degrees
# 2) compute transverse displacement w/ positive inward, but report postive outward

def reportShort(fileName, results, X_vec, Phi_vec):
  f = open(fileName, 'w')
  for X_idx in range(0, len(X_vec)):
    for Phi_idx in range(0, len(Phi_vec)):
      f.write('%.6g\n' % (results.strainMap[X_idx][Phi_idx].eps_phi_out))
  f.close
            
def reportDisplPonly(fileName, results, X_vec, Phi_vec):
  f = open(fileName, 'w')
  for XPhi_idx in range(0, len(X_vec)):
    f.write('%.6g\n' % (-1*results.midDisplMap[XPhi_idx][XPhi_idx].w))
  f.close

def reportDisplPandL(fileName, results, X_vec, Phi_vec):
  f = open(fileName, 'w')
  for X_idx in range(0, len(X_vec)):
    for Phi_idx in range(0, len(Phi_vec)):
      f.write('%.6g\n' % (-1*results.midDisplMap[X_idx][Phi_idx].w))
  f.close

def reportInputs(fileName, X_vec, Phi_vec, Length, Thickness, Radius, E, nu, Pressure, gamma, LiqHeight, M, N, meshID):
  f = open(fileName, 'w')
  f.write('Tank Results\n')
  f.write('Pressure  = %s\n' % (Pressure))
  f.write('gamma     = %s\n' % (gamma))
  f.write('LiqHeight = %s\n' % (LiqHeight))
  f.write('E         = %s\n' % (E))
  f.write('nu        = %s\n' % (nu))
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
  f = open(fileName, 'a')
#  f.write('\n\n%s' % ('Axial Displacement, u')) 
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
 
def resultsSummary(results, outputFileName):
  f = open(outputFileName, "w")
  f.write('%.15e %s \n' % (results.maxStressEqu, 'max_stress_equ'))
  f.write('%i %s \n' % (results.X_ind_max_stress, 'X_ind_max_stress'))
  f.write('%i %s \n' % (results.Phi_ind_max_stress, 'Phi_ind_max_stress'))
  f.write('%i %s \n' % (results.surface_stress, 'surface_stress'))
  f.write('\n')
  f.close()
