/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <cassert>
#include <iostream>
#include <string>

//#include "hdf5.h"
//#include "netcdf.h"
// This header not currently in the repo:
#include "NetcdfFiles.hh"

// Note: may need newer Cmake for NetCDF probe, particularly CMakePushCheckState

using radlib_seacasio::SimpleNetcdfFile;

int main()
{

  double val_out = 3.14159, val_in = 0.0;
  std::string fileName("binary_io_test.h5");

  // scope within which file write takes place
  {
    // open/create file
    const bool fileExist = false;
    const bool readOnly = false;
    SimpleNetcdfFile netcdfFile(fileName, fileExist, readOnly);
    
    // write data 
    netcdfFile.addVar("adouble", val_out);
    
    // close file
    // goes out of scope...
  }

  // scope within which file write takes place
  {
    // open/read file
    const bool fileExist = true;
    const bool readOnly = true;
    SimpleNetcdfFile netcdfFile(fileName, fileExist, readOnly);
  
    // read data 
    netcdfFile.getVar("adouble", val_in);
  }

  assert(val_in == val_out);

  return(0);
}

