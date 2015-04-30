/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */
//
// knapMPS.cp
//
// Write a knapsack problem in MPS format.
//

#include <acro_config.h>
#include <utilib/ParameterSet.h>
#include <pebbl/serialKnapsack.h>

using namespace utilib;
using namespace pebbl;
using namespace std;

int debug=0;


int main(int argc, char** argv) 
{
  utilib::ParameterSet params;
  params.create_parameter("debug",debug,"<int>","0",
            "Debugging level",ParameterNonnegative<int>());

  if (!params.process_parameters(argc,argv)) {
     cerr << "Unknown option: " << argv[1] << endl;
     return -1;
     }
  if (params.get_parameter<bool>("help")) {
     params.write_parameters(cout);
     return -1;
     }

  binaryKnapsack k;
  k.setup(argc,argv); 
  k.setDebug(debug);
  k.preprocess();
  ucout << "NAME knapsack\nOBJSENSE\n MAX\nROWS\n N   value\n L   weight\n"
          "COLUMNS\n";
  int i;
  for(i=0; i<k.numItems; i++)
    ucout << "    " << k.item[i].name << " value "
      << k.item[i].value << " weight " << k.item[i].weight << '\n';
  ucout << "RHS\n rhs weight " << k.capacity << "\nBOUNDS\n";
  for(i=0; i<k.numItems; i++)
    ucout << " BV BOUND " << k.item[i].name << '\n';
  ucout << "ENDATA\n";
  ucout << Flush;
  return 0;
}

