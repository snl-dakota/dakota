/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

#include <fstream>
#include <ios>
#include <cstdio>

namespace utilib {

inline int copy_file(const string& source_file, const string& dest_file)
{
std::ifstream infile(source_file.c_str(), std::ios_base::binary);
std::ofstream outfile(dest_file.c_str(), std::ios_base::binary);

int status = 0;

if (infile.is_open() && outfile.is_open()) {
    outfile << infile.rdbuf();
    status = 1;
}

else if (!outfile.is_open()) {
    std::cout << "copy_file error: cannot open destination file '" << dest_file << "'" << std::endl;
}

else if (!infile.is_open()) {
    std::cout << "copy_file error: cannot open source file '" << source_file << "'" << std::endl;
    }
        
infile.close();
outfile.close();
return status;
}

inline void move_file(const string& source_file, const string& dest_file)
{
if (copy_file(source_file, dest_file)) {
   remove(source_file.c_str());   
}
}

}
