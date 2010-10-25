## [xx, yy, z] = peaks()
##
## return the "well" known MatLab (R) peaks function
##   evaluated in the [-3,3] x,y range.

function [xx, yy, z] = peaks()

  xx = yy = -3:0.1:3;
  [x,y] = meshgrid (xx,yy);

  z = 3 * (1-x).^2 .* exp(-(x.^2) - (y+1).^2) - ...
      10 * (x/5 - x.^3 - y.^5) .* exp(-x.^2 - y.^2)- ...
      1/3 * exp(-(x+1).^2 - y.^2);

  ## z(z<-1) = -1; ## Jungfraujoch/Interlaken

endfunction
