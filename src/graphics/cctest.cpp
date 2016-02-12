/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//#include <unistd.h>
#include "Graphics.hpp"
#include <iostream>
using namespace std;

main()
{
  int i;
  Graphics2D  my_interface;
  my_interface.create_plots2d(9);
  //my_interface.set_yrange2d(3,0,1.0);
  //my_interface.set_xrange2d(3,0,0);
  //char *test="this"; 
  my_interface.set_title2d(0,"this");
  my_interface.change_color2d(0,0,200,100);
  my_interface.go();

  // while(2>1){sleep(1); printf("hi \n");}  

  for(i=0;i<9;i++){
    my_interface.add_datapoint2d(0,(double)i,(double)i);
    my_interface.add_datapoint2d(1,(double)i,(double)i);
    my_interface.add_datapoint2d(2,(double)i,(double)i);
    my_interface.add_datapoint2d(3,(double)i,(double)i*i);
    my_interface.add_datapoint2d(4,(double)i,(double)i*i);
    my_interface.add_datapoint2d(5,(double)i,(double)i*i);
    my_interface.add_datapoint2d(6,(double)i,(double)i*i);
    my_interface.add_datapoint2d(7,(double)i,(double)i*i);
    my_interface.add_datapoint2d(8,(double)i,(double)i);
  }

  while(2>1){}
}
