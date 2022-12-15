%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% indefinite misfit Hessian, uncorrelated prior
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

L = [ 0.2 0
      0 0.2 ];
H_m = [ 0.92 1.44
        1.44 0.08 ]

LHLI = L' * H_m * L + eye(2);
C1 = L*inv(LHLI)*L'

Prior_Cov = L * L';
H_p = H_m + inv(Prior_Cov);
C2 = inv(H_p)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% positive definite misfit Hessian, correlated prior
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

L = [ 0.2  0
      0.05 0.2 ];
H_m = [ 1.64 0.48
        0.48 1.36 ]

LHLI = L' * H_m * L + eye(2);
C1 = L*inv(LHLI)*L'

Prior_Cov = L * L';
H_p = H_m + inv(Prior_Cov);
C2 = inv(H_p)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% No contribution from misfit --> proposal covariance = prior covariance
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

H_m = [ 0. 0.
	0. 0. ]

LHLI = L' * H_m * L + eye(2);
C1 = L*inv(LHLI)*L'

H_p = H_m + inv(Prior_Cov);
C2 = inv(H_p)
