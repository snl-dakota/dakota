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
// Direct.cpp
//
/*********************************************************/ 
/*	Direct Optimizer Library - Constrained Version   */ 
/*	Sequential Implementation		         */ 
/*	Steven Cox 				         */ 
/*	March 2002                                       */ 
/*	Object Oriented Version 			 */
/*********************************************************/ 
 
/************************************************************
Questions:
  destructor for points?
  asynchronous calls and MPI?
  
************************************************************/

#include <acro_config.h>

#include <scolib/Direct.h>

#include <colin/SolverMngr.h>

#include <utilib/_math.h>
#include <utilib/sort.h>
#include <utilib/stl_auxiliary.h>

const double pi = 3.14159265358979323846;

using namespace std;

namespace scolib {

using std::max;

using namespace direct;

double epsilon = 1e-8;

double length(vector<double>& vec)
{
unsigned int len = vec.size();

double val=0.0;
for (unsigned int i=0; i<len; i++)
  val = vec[i]*vec[i];
return sqrt(val);
}

real UPDATE_POINT_VALUE(point& point ,int method,double  penalty)
{
real ans;
if (method == 2)
   ans = point.Val;
return ans;
}



DIRECT::DIRECT()
 : dimen(0)
{
division = 2;
constraint_method = 2;
minsize2 = 0.01;
nsort = 0;
directbp_converge=false;

//mindist = 0.05;
ibest = 0;

MPIVersion = false;

   //utilib::OptionParser &opt = this->option;

   properties.erase("function_value_tolerance");

   
   DIRECT_BP = false;
   this->properties.declare
      ( "bp_flag", 
        "When true, optimization calls the DIRECT-BP subroutines to "
        "perform a neighborhood search about the best point found "
        "at each iteration",
        utilib::Privileged_Property(DIRECT_BP) );
   //opt.add("bp_flag", DIRECT_BP,
   //        "When true, optimization calls the DIRECT-BP subroutines to "
   //        "perform a\n\tneighborhood search about the best point found "
   //        "at each iteration");

   Gamma = .01;
   this->properties.declare
      ( "gamma", 
        "Enforces a lower limit on the distance to neighboring boxes in "
        "DIRECT-BP",
        utilib::Privileged_Property(Gamma) );
   //opt.add("gamma", Gamma,
   //        "Enforces a lower limit on the distance to neighboring boxes in\n"
   //        "\tDIRECT-BP");

   agressive = false;
   this->properties.declare
      ( "aggressive", 
        "If false, then eliminate potentially optimal boxes based on the "
        "standard DIRECT Graham sort routine.",
        utilib::Privileged_Property(agressive) );
   //opt.add("aggressive", agressive,
   //        "If false, then eliminate potentially optimal boxes based on the\n"
   //        "\tstandard DIRECT Graham sort routine.");

   division_str = "single";
   this->properties.declare
      ( "division", 
        "Controls how many dimensions of a box are subdivided:\n"
        "   single - boxes are divided on one long side\n"
        "   multi  - boxes are divided on all long sides",
        utilib::Privileged_Property(division_str) );
   //opt.add("division", division_str,
   //        "Controls how many dimensions of a box are subdivided:\n"
   //        "\t  single - boxes are divided on one long side\n"
   //        "\t  multi  - boxes are divided on all long sides");

   min_improvement = 1e-8;
   this->properties.declare
      ( "min_improvement", 
        "Minimum potential function value improvement for a box on "
        "the convex hull to be considered potentially optimal.  "
        "Indirectly sets a minimum value of the Lipschitz constant "
        "at any iteration based with the size of the box containing "
        "the best point found.",
        utilib::Privileged_Property(min_improvement) );
   //opt.add("min_improvement", min_improvement,
   //        "Minimum potential function value improvement for a box on "
   //        "the convex\n\thull to be considered potentially optimal.  "
   //        "Indirectly sets a minimum\n\tvalue of the Lipschitz constant "
   //        "at any iteration based with the size of\n\the box containing "
   //        "the best point found.");
   //opt.alias("min_improvement","local_search_balance");

   method_str = "penalty";
   this->properties.declare
      ( "constraints", 
        "Method used to manage constraints:\n"
        "   explicit - use Jone's constrained DIRECT technique\n"
        "   penalty  - use a simple penalty function technique",
        utilib::Privileged_Property(method_str) );
   //opt.add("constraints", method_str,
   //        "Method used to manage constraints:\n"
   //        "\t  explicit - use Jone's constrained DIRECT technique\n"
   //        "\t  penalty  - use a simple penalty function technique");

#if 0
/// Not currently used
create_parameter("ls_distance", mindist,
	"<double>","0.05",
	"Minimum separation between points selected for origin of a local optimization");
#endif

   min_boxsize = 0.0001;
   this->properties.declare
      ( "min_boxsize_limit",  
        "Smallest size box divided by DIRECT.  DIRECT stops when smallest "
        "box is smaller than this size",
        utilib::Privileged_Property(min_boxsize) );
   //opt.add("min_boxsize_limit", min_boxsize,
   //        "Smallest size box divided by DIRECT.  DIRECT stops when smallest\n"
   //        "\tbox is smaller than this size");

   maxsize = 0.0;
   this->properties.declare
      ( "max_boxsize_limit",  
        "DIRECT stops when the largest box is smaller than this size",
        utilib::Privileged_Property(maxsize) );
   //opt.add("max_boxsize_limit", maxsize,
   //        "DIRECT stops when the largest box is smaller than this size");

   max_boxsize_ratio = 0.0;
   this->properties.declare
      ( "max_boxsize_ratio",  
        "Boxes whose size divided by the max box size is less than this "
        "ratio are not divided",
        utilib::Privileged_Property(max_boxsize_ratio) );
   //opt.add("max_boxsize_ratio", max_boxsize_ratio,
   //        "Boxes whose size divided by the max box size is less than this "
   //        "ratio\n\t are not divided");
   //opt.alias("max_boxsize_ratio","global_search_balance");

   ratio = 20; //*sqrt(10);
   this->properties.declare
      ( "ratio",  
        "Maximum ratio of the length of the longest vector needed to form "
        "a possitive spanning set, to the length of the shortest side "
        "of the best box in DIRECT-BP where the neighborhood is still "
        "considered balanced and the best box will still be divided",
        utilib::Privileged_Property(ratio) );
   //opt.add("ratio", ratio,
   //        "Maximum ratio of the length of the longest vector needed to form "
   //        "a\n\tpossitive spanning set, to the length of the shortest side "
   //        "of the best\n\tbox in DIRECT-BP where the neighborhood is still "
   //        "considered balanced\n\tand the best box will still be divided");

   reset_signal.connect(boost::bind(&DIRECT::reset_DIRECT, this));
}



void DIRECT::reset_DIRECT()
{
if ( problem.empty() ) return;

dimen = problem->num_real_vars;
x.resize(dimen);
x << 0.0;
division_counter.resize(dimen);
division_counter << (unsigned int)0;
e.resize(dimen);
e << 0.0;

ncon = 0; // problem->numConstraints(); -> by definition, UNLP is unconstrained
g.resize(ncon);
g << 0.0;
constraint_scale.resize(ncon);
constraint_scale << 0.0;
#ifndef ACRO_HAVE_TEMPLATES_AS_TEMPLATE_ARGUMENTS
constr_buffer.resize(ncon);
constr_buffer << 0.0;
#endif

center.clear();
order.clear();
best.clear();
local.clear();
    	
if (division_str == "single")
   division = 1;
else if (division_str == "multi")
   division = 2;
else
   EXCEPTION_MNGR(runtime_error,"DIRECT::reset - bad value \"" << division_str << "\" for option \"division\"");

if (method_str == "explicit")
   constraint_method = 1;
else if (method_str == "penalty")
   constraint_method = 2;
else
   EXCEPTION_MNGR(runtime_error,"DIRECT::reset - bad value \"" << method_str << "\" for option \"constraints\"");

nsort = 0;
ibest = 0;
    	
//min box size needed to check for neighbors.6
minsize3 = sqrt((double)dimen)/ratio/3.0;
   	
if ( problem->enforcing_domain_bounds ) {
   x_lower = problem->real_lower_bounds;
   x_upper = problem->real_upper_bounds;
   range.resize(dimen);
   for (unsigned int i=0; i<dimen; i++)
     range[i] = x_upper[i]-x_lower[i];
   }

//problem->get_nonlinear_constraint_bounds(clower,cupper);
eval_mngr().clear_evaluations();
#ifdef COLINDONE
batch_evaluator_t::reset();
#endif
}
  


//
//
//
bool DIRECT::check_convergence()
{
if ( colin::ColinSolver<vector<double>, colin::UNLP0_problem>
     ::check_convergence() )
   return true;

list<direct::simple_sort>::iterator iorder;
iorder = order.begin();
if ( ((min_boxsize > 0.0) && ((*iorder).size() < min_boxsize)) || directbp_converge) {
   stringstream tmp;
   tmp << "Box-Size-Limit - min boxsize (" << (*iorder).size() << " < " << min_boxsize << " )";
   solver_status.termination_info = tmp.str();
   return true;
   }

DEBUGPR(10,
        // Checking to ensure that all of the boxes 'fill' the space
        vector<point>::iterator curr = center.begin();
        vector<point>::iterator last = center.end();
        double total=0.0;
        while (curr != last) {
           vector<double>& tmp = (*curr).l;
           double val = 1.0;
           for (unsigned int i=0; i<tmp.size(); i++)
              val *= tmp[i];
           total += val;
           curr++;
        }
        if (fabs(total - 1.0) > 0.00001)
        EXCEPTION_MNGR(runtime_error, "Total volume of boxes is " << 
                       (total*100) << " percent != 100%");
        );

return false;
}


/*******************************************************************/ 
/* Main Program                                                    */ 
/*******************************************************************/ 
void DIRECT::optimize() 
{
//
// Misc initialization of the optimizer
//
//opt_init();
if (initial_point.size() == 0) {
   solver_status.termination_info = "No-Real-Params";
   return;
   }
if (!(problem->finite_bound_constraints())) {
   solver_status.termination_info = "Missing-Bound-Constraints";
   return;
   }
if ((max_boxsize_ratio < 0.0) || (max_boxsize_ratio > 1.0)) {
   solver_status.termination_info = "Error - bad value for max_boxsize_ratio";
   return;
   }
//
// Misc data structures
//
direct::point temppt(dimen, ncon);
vector<direct::point> tempcenter;
//
// find sizes and center point
//
for (unsigned int i = 0; i < dimen; i++) { 
  temppt.x[i] = (x_upper[i]+x_lower[i])/2.0;    //center point of box 
  temppt.l[i] = 1.0;		    //each box is 1 unit long in each dimension 
  e[i] = 0.2;
  } 
temppt.reset();
tempcenter.push_back(temppt);
Eval(tempcenter);
vector<unsigned int> optimal;
optimal.push_back(0);
//
// Setup the iteration counters and do debugging IO
//
unsigned int num_iters;
if (max_iters <= 0)
   num_iters = MAXINT;
else
   num_iters = curr_iter + max_iters;
debug_io(ucout);
//
// Iterate...
//
for (curr_iter++; curr_iter <= num_iters; curr_iter++) {
  //
  // locate potentially optimal points 
  //
  if (constraint_method == 1)
     Consort(optimal);
  else if (constraint_method == 2)
     Graham(optimal,min_improvement,agressive,true); 

  //
  // Break after (re)ordering optimal so that we actually return the
  // correct set of potentially optimal boxes.
  //
  if (check_convergence())
     break;

  //
  // Divide Potentially optimal boxes
  //
  Divider(optimal); 


  //
  // Sort 'order', to order boxes by box size and then value
  //
  order.sort();

  debug_io(ucout);
  }

//
// Return the final point(s)
//
// Return all potentially optimal boxes
vector<unsigned int>::iterator it = optimal.begin();
vector<unsigned int>::iterator itEnd = optimal.end();
for ( ; it != itEnd; ++it )
   final_points->insert(center[*it].response);
// Return the best point
this->final_points->insert(opt_response.response);
   


//
// Verify if this is a valid termination
//
debug_io(ucout,true);
check_convergence();
//opt_response.response->get(colin::f_info,opt_response.value());
eval_mngr().clear_evaluations();
#ifdef COLINDONE
this->clear_evaluations();
#endif
}
 
 
/************************************************************************/ 
/* Subroutine to evaluate the function value at a point                 */ 
/************************************************************************/ 
 
void DIRECT::Eval(vector<direct::point> & A) 
{
   DEBUGPR(10, ucout << "Eval - num points = " << A.size() << endl);

vector<direct::point>::iterator ccurr = A.begin();
vector<direct::point>::iterator cend  = A.end();
while (ccurr != cend) {
  colin::AppRequest request = problem->set_domain(ccurr->x);
  problem->Request_response(request, ccurr->response);
  problem->Request_F(request,ccurr->Val);
  eval_mngr().queue_evaluation(request);
  #ifdef COLINDONE
  queue_evaluation(ccurr->x, 1.0,ccurr->response);
  #endif
  ccurr++;
  }
eval_mngr().synchronize();
#ifdef COLINDONE
execute_evaluations();
#endif

#if COLINDONE
vector<direct::point>::iterator tmp = A.begin();
ccurr = A.begin();
while (ccurr != cend) {
  if (ncon > 0)
     ccurr->Val = ccurr->response.augmented_function_value();
  else
     ccurr->response->get(colin::f_info,ccurr->Val);
  ccurr->response->get(colin::f_info,ccurr->fVal);
  ccurr->cVal = ccurr->response.l2_constraint_violation();
  ccurr->feasible = (ccurr->cVal == 0.0);
  if ((ccurr != tmp) && (ccurr->Val < tmp->Val)) tmp = ccurr;
  ccurr++;
  }
#endif

ccurr = A.begin();
while (ccurr != cend) {
     
  if (ccurr->Val+epsilon < opt_response.value()) {
     opt_response << ccurr->response;
     opt_response.value() = ccurr->Val;
     opt_response.point = ccurr->x;
     ibest = center.size();
     }
  order.push_back(simple_sort(center.size(), &center ));
  center.push_back(*ccurr);

  DEBUGPR(1000,
          list<simple_sort>::const_iterator iter = order.end();
          iter--;
          ucout << "Order info: index=" << (*iter).order << " size=" 
                << (*iter).size() << " val=" << (*iter).Val() << endl;
          );
  DEBUGPR(100, 
          ucout << "Eval - new box info:  " << endl; ccurr->write(ucout); 
          ucout << endl);

  ccurr++;
  }

///
/// TEST FOR A BUG
///
DEBUGPR(100,
        for (unsigned int i=0; i<center.size(); i++) 
        {
           if (i == ibest) continue;
           if ((center[i].Val+epsilon < center[ibest].Val) && 
               ((center[i].feasible == center[ibest].feasible) ||
                center[i].feasible)) 
           {
              cout << "I info" << endl;
              center[i].write(cout);
              cout << "IBEST info" << endl;
              center[ibest].write(cout);
              EXCEPTION_MNGR(runtime_error, "There is a bug in DIRECT::Eval");
           }
        }
        );

DEBUGPR(1000000,
        ucout << "Eval - center points after evaluation\n";
        vector<point>::const_iterator icurr = center.begin();
        vector<point>::const_iterator ilast = center.end();
        for (int j=0; icurr != ilast; icurr++, j++) 
        {
           ucout << "Point " << j << endl;
           icurr->write(ucout);
        }
        );
} 
 

/************************************************************************/ 
/* Subroutine Graham's scan 				                */ 
/************************************************************************/ 
 
void DIRECT::Graham(vector<unsigned int>& optimal, double min_improvement, bool aggressive, bool post_process) 
{
   DEBUGPR(100, 
           if (order.size() != center.size())
               EXCEPTION_MNGR(runtime_error, "order.size= " << order.size() 
                              << " center.size= " << center.size());
           );

//
// Find the best boxes of each size.  These are stored in temp_optimal,
// since we may need to add and delete from this list.
//
double max_boxsize=0.0;
std::list<simple_sort> temp_optimal; 
std::list<simple_sort>::iterator iorder    = order.begin();
std::list<simple_sort>::iterator order_end = order.end();
//// Take the first element of 'order'
double prev = (*iorder).size();
temp_optimal.push_back(*iorder); 
DEBUGPR(10, 
        ucout << "* Num " << (*iorder).order << "\tBoxSize " 
        << (*iorder).size() << "\t Val " << (*iorder).Val() << endl);
iorder++;
while (iorder != order_end) { 
  //
  // if the box size changes then index1[j] is potentially optimal 
  //
  if (((*iorder).size()-0.001*min_boxsize) > prev) { 
     temp_optimal.push_back(*iorder); 
     prev = (*iorder).size();
     DEBUGPR(10, ucout <<  "* Num " << (*iorder).order << "\tBoxSize " << 
             (*iorder).size() << "\t Val " << (*iorder).Val() << endl);
     max_boxsize = (*iorder).size();
     } 
  else {
     DEBUGPR(100, ucout << "  Num " << (*iorder).order << "\tBoxSize " << 
             (*iorder).size() << "\t Val " << (*iorder).Val() << endl);
     }
  iorder++; 
  }
//
// Find the convex hull, if aggressive==false
//
if (!aggressive && (temp_optimal.size() > 1)) {
   //
   // Find the lower convex hull of the points
   //
   iorder = temp_optimal.begin();
   list<simple_sort>::iterator iorder2   = iorder;
   iorder2++;
   list<simple_sort>::iterator iorder3 = iorder2;
   iorder3++;
   list<simple_sort>::iterator iend = temp_optimal.end();

   while (iorder3 != iend) 
   {
      DEBUGPR(100, ucout << 
              "(" << (*iorder).size() << "," << (*iorder).Val() << ") " <<
              "(" << (*iorder2).size() << "," << (*iorder2).Val() << ") "
              "(" << (*iorder3).size() << "," << (*iorder3).Val() << ")\n" );
     //
     // The points iorder, iorder2 and iorder three have increasing
     // box sizes (size()).  Thus to ensure that we have a convex lower
     // hull, we need to have the values (size,Val) for iorder3
     // above the line through the first two points.  If the following
     // value is positive, then this is true.
     //
     double o = (*iorder).size() * ((*iorder2).Val() - (*iorder3).Val()) 
         	     - (*iorder).Val() * ((*iorder2).size() - (*iorder3).size()) 
         	     + ((*iorder2).size() * (*iorder3).Val() - (*iorder3).size()
					* (*iorder2).Val()); 
     //
     // Allow for some roundoff effects, so this can be slightly 
     // negative and still consider this to be a 'convex' set
     //
     if (o > -1e-8) { 
	//
	// Increment each of the poitners
	//
        iorder++;
        iorder2 = iorder;
      	iorder2++; 
      	iorder3 = iorder2;
      	iorder3++; 
        } 
     else { 
	//
	// Increment each of the pointers
	//
        iorder2 = temp_optimal.erase(iorder2);
        if (iorder != temp_optimal.begin()) {
           iorder3 = iorder;
      	   iorder3++; 
           iorder2 = iorder;
           iorder--;
           }
        else {
           iorder2 = iorder;
      	   iorder2++; 
           iorder3 = iorder2;
      	   iorder3++; 
           } 
        } 
     } 
   //
   // Remove left side of convex hull 
   //
   while (true) {
     iorder = temp_optimal.begin();
     list<simple_sort>::iterator iorder2 = iorder;
     iorder2++;
     if ((iorder2 != temp_optimal.end()) && 
	 (center[(*iorder2).order].Val < center[(*iorder).order].Val))
        temp_optimal.erase(iorder);
     else
        break;
     }
   //
   // Ensure that subsequent boxes ensure sufficient gain
   //
   double fmingain = center[(*iorder).order].Val - 
			min_improvement*fabs(center[(*iorder).order].Val);
   while (true) {
     iorder = temp_optimal.begin();
     list<simple_sort>::iterator iorder2 = iorder;
     iorder2++;
     if (iorder2 != temp_optimal.end()) {
	double Lmin = (center[(*iorder2).order].Val - 
			         center[(*iorder).order].Val)/
			        ((*iorder2).size() - (*iorder).size());
     	double Ldes = (center[(*iorder).order].Val-fmingain)/(*iorder).size();
      	if (Lmin <= Ldes) 
      	   temp_optimal.erase(iorder);
        else 
    	   break;
    	   }
     else
    	break;
     }
   }
if (post_process) {
   //
   // Check for small boxes with large neighbors
   //
   if (DIRECT_BP)
      Neighbor(temp_optimal);
   //
   // Record Potentially optimal points, eliminating boxes that are 
   //
   iorder = temp_optimal.begin();
   order_end = temp_optimal.end();
   optimal.clear();
   while (iorder != order_end) {
     //
     // Only divide boxes that are sufficiently large compared with the
     // biggest box.
     //
     if ((max_boxsize_ratio > 0.0) && 
         ((*iorder).size() < max_boxsize * max_boxsize_ratio)) 
     {
        DEBUGPR(100, ucout << "Graham - skip box " << (*iorder).order << 
                " which has a max-boxsize ratio " << 
                (((*iorder).size())/max_boxsize) << " less than " 
                << max_boxsize_ratio << "\n");
        }
     else
        optimal.push_back((*iorder).order);
     //
     // Put this box on the optimal list
     //
     iorder++;
     }
  }
else {
   iorder = temp_optimal.begin();
   order_end = temp_optimal.end();
   optimal.clear();
   while (iorder != order_end) {
     optimal.push_back((*iorder).order);
     iorder++;
     }
  }
}
 


/************************************************************************/ 
/* Subroutine Constrained Sort				                */ 
/************************************************************************/ 
 
void DIRECT::Consort(vector<unsigned int>& optimal) 
{ 
double z, c, zmin, fstar=0.0, fint, fintmin,h,h2; 
int nint; 
int m;
std::list<direct::boxsort> sorter;
std::list<direct::boxsort>::iterator tin, nin, bin ;
list<direct::simple_sort>::iterator oiter, oiter2;
double min_sort_value=0.0;

//        cout << "Start Consort " << endl;
        
#if 0
if (!MPIVersion)
 {
// remove then re-insert optimal points

   for (int i = 0; i < optimal.size(); i++) 
     { 
	yy = optimal[i];		//optimal contains the boxes that were divided last round 
      	z = center[yy].size;
      
        oiter = order.begin();
        for (unsigned k = 0; k < order.size(); k++) 
          { 
      	    if ((*oiter).order == yy)        			//find out where the box was in the list 
            	{ 
                 oiter = order.erase(oiter); 
            	 break; 
            	} 
            oiter++;
          }
        order.push_back(direct::simple_sort(yy,&center));
      } 
      
// add new points to order 
 
   for (int i = order.size(); i < center.size(); i++)//now do the rest of the new points 
     { 
     z = center[i].size(); 
     order.push_back(direct::simple_sort(i,&center));
     } 
 
   order.sort();

 }// end sequential addition
#endif


// sort points by distance 
   	zmin = dimen;  
   	tin = sorter.begin();
	oiter = order.begin();
	oiter2 = order.end();
	
   	while (oiter != oiter2)                		//sort the points by constraint, F & size
    	  { 
   	    z = (*oiter).size();  //0; 
	    if (z < zmin)
	      zmin = z;
	    
            c = 0; 
    	    for (int k = 0; k < (ncon); k++) 
      	      if(center[(*oiter).order].constraint(k) > 0)
      	         c += constraint_scale[0]/max(constraint_scale[k],.0000000000000001)*center[(*oiter).order].constraint(k); 
      	      	
            sorter.push_back(direct::boxsort((*oiter).order,(*oiter).Val(),c/z,z));
            if ((c == 0) || ((*oiter).Val() < min_sort_value))
              {
                min_sort_value = (*oiter).Val();
              }
            oiter++;
        
         } 

   	nsort = center.size(); 
   	sorter.sort();

//	cout << "finished sorting " << endl;
/*	tin = sorter.begin();
	for (; tin != sorter.end();tin++)
//	  cout << (*tin).numb << " " << (*tin).o1 << " " << (*tin).o2 << " " << (*tin).o3 << endl;
//	cout <<endl;
*/
/*************Start finding envelope**************/ 
 
   tin = sorter.begin();			//position of pt j
   nin = sorter.begin();			//position of pt i
   
//                   printf("adding #%d to location 1\n",(*tin).o1);
   m=0;
   
   fintmin = -9999999;
   nint = 0;
   
   optimal.clear();
      
/*********Select 1st pt based on min_improvement improvement*********/  
   int i = 0;
   optimal.push_back((*tin).numb);
//   fprintf(fp2,"\nindex 1 = %d optimal 1 = %d\n",(*tin).numb,optimal[1]);

   if ((*tin).o2 == 0)
    {
      fstar = (*tin).o1-max(fabs(.0001*(*tin).o1),min_improvement);
      h = max(fabs(.0001*min_sort_value),min_improvement);
//  cout << "Checking con =0  Fstar = " << fstar << endl;
  
      for (unsigned int j=1; j< center.size(); j++)
       {
         tin++;
         h2 = (max((*tin).o1-fstar,0.0) + (*tin).o2);
         if (h2 < h)
           {
//              cout << h << "  " << h2 << endl;
              nin = tin;
              i=j;
              optimal[0] = (*tin).numb;
              h = h2;
//              printf("changing #1 to #%d    (%d)\n",i,(*tin).numb);
           }
       }
    }
   else 					// No feasible point, take just the least infeasible one (value of h)
    {
      m=1;
      h = (*tin).o2;
      
      for (unsigned int j=1; j< center.size(); j++)
       {
         tin++;
         h2 = (*tin).o2;
         if (h2 < h)
           {
              i=j;
              for (int k=1 ; k<i;k++)
                nin++;
              optimal[0] = (*tin).numb;
              h = h2;
   	      fstar = min_sort_value-max(fabs(.0001*min_sort_value),min_improvement);
           }
       }
    }
    
//   cout << "optimal[0] = " << optimal[0] << endl;
   while (m==0){
     tin = sorter.begin();			//position of pt j
     fintmin = -9999999;
     for (int j=0; j< nsort; j++)
      {
	if ( tin == nin)
	  {
//	    printf("skipping all\n");
	    tin++;
	    continue;
	  }  
	if ((fabs((*nin).o1 - (*tin).o1) < 0.00000001) && 
	    (fabs((*nin).o2 - (*tin).o2) < 0.00000001) &&
	    (fabs((*nin).o3 - (*tin).o3) < 0.00000001))
          {
//	printf("MATCH!! box %d and %d\n", (*nin).numb,(*tin).numb);
           for (unsigned int k=0; k<optimal.size(); k++)				// Identical boxes, box j is added
            {
              if (optimal[k] == (*tin).numb)
                 break;
              if (k == optimal.size())
                 {
                   optimal.push_back((*tin).numb);
                 }
            }
	   tin++;
           continue;
         }
       else if (((*nin).o1 <= (*tin).o1) && 
	        ((*nin).o2 <= (*tin).o2) &&
		((*nin).o3 >= (*tin).o3))
         {
           if (((*nin).o2 == (*tin).o2) &&  ((*tin).o1 <= fstar))
             {
               for (unsigned int k=0; k<optimal.size(); k++)				// Identical boxes, box j is added
                 {
              	   if (optimal[k] == (*tin).numb)
                     break;
              	   if (k == optimal.size())
                     {
                  	optimal.push_back((*tin).numb);
                     }
                 }
//	       printf("identical plateaus\n");
	       tin++;
               continue;
             }
              
//           printf("point %d was masked by point %d  A\n",(*tin).numb,(*nin).numb);
		   tin = sorter.erase(tin); 
	   bin = tin;
	   bin--;
           j--;
//           tin++;
           nsort--;
           if (i> j)
             i--;
           if (nint> j)
             nint--;
//           printf("point tin-numb = %d and point (*nin).numb = %d  bin->numb = %d A\n",(*tin).numb,(*nin).numb,bin->numb);
//	   printf("j = %d  nsort = %d\n",j,nsort);
           continue;
          
          }
       else 
         {
           if ((*nin).o1 <= fstar)				// cases where i was intersected originally on the plateau
             {
                if (((*nin).o2 == (*tin).o2)&& ((*tin).o1 <= fstar))
         	   {
          	     for (unsigned int k=0; k<optimal.size(); k++)				// Identical boxes, box j is added
         	       {
         	         if (optimal[k] == (*tin).numb)
         	            break;
         	         if (k == optimal.size())
         	            {
                               optimal.push_back((*tin).numb);
              		    }
           	       }
        	     if ((*tin).o1 < (*nin).o1)		// j extends further out than i so you must switch to j
        	       {
        	         nint=j;
        	         fint = fstar-.00000001;
        	         break;
        	       }
	//	     printf("switching\n");
		     tin++;
       		     continue;
     		   }
     	     }
           if (fabs((*nin).o3 - (*tin).o3) > 0.000000001)   // diff slopes
             {
               fint = ((*tin).o1/(*tin).o3 - (*nin).o1/(*nin).o3 + (*tin).o2 - (*nin).o2)/(1/(*tin).o3-1/(*nin).o3);
//	printf("point %d/%d intersects point %d at %f on the slope of i and j\n",j,(*tin).numb,(*nin).numb,fint);
               if ((fint > fintmin) && (fint < (*tin).o1)&& (fint <= fstar))
                 {
//	printf("point %d, ie. box  %d is new nint and fintmin = %f \n",j,(*tin).numb,fint);
                   fintmin = fint;
                   nint = j;
                 }
	       if (((*tin).o3 < (*nin).o3) && (fint > fstar))
                 {
//	printf("point %d was masked by point %d  B\n",(*tin).numb,(*nin).numb);
             	   tin = sorter.erase(tin); 
                   j--;
//                   tin--;
                   nsort--;
                   if (i> j)
                     i--;
           	   if (nint> j)
             	     nint--;
                   continue;

                 }
             }
           fint = ((*nin).o1+(*nin).o2*(*nin).o3-(*tin).o2*(*nin).o3);
//	printf("point %d/%d intersects point %d at %f on the slope of i and plateau j\n",j,(*tin).numb,(*nin).numb,fint);
           if ((fint >= (*tin).o1) && (fint > fintmin) && (fint <= fstar))
             {
               fintmin = fint;
//	printf("point %d, ie. box  %d is new nint and fintmin = %f \n",j,(*tin).numb,fint);
               nint = j;
             }
	   if (((*tin).o3<(*nin).o3) && (fint > fstar))
             {
//	printf("point %d was masked by point %d  C\n",(*tin).numb,(*nin).numb);
               tin = sorter.erase(tin); 
               j--;
 //              tin--;
               nsort--;
               if (i> j)
                 i--;
               if (nint> j)
                 nint--;
               continue;
                   
             }
//        printf("incrementing tin \n");
	tin++;
         }
      }

//printf("   fintmin = %f and fstar = %f,  nint = %d, i = %d\n\n",fintmin, fstar,nint, i);     
     if (fintmin < fstar)
      {
        fstar = fintmin - min_improvement;

        tin = sorter.begin();
        for (int k=0;k<nint;k++)
          tin++;
        
           
        for (unsigned int k=0; k<optimal.size(); k++)				// Identical boxes, box j is added
          {
            if (optimal[k] == (*tin).numb)
               break;
            if (k == optimal.size()-1)
               {
                 optimal.push_back((*tin).numb);
//                 printf("added %d to optimal\n",(*tin).numb);
               }
          }
      }
     if (nint == i)
       m=1;
     i = nint;
     nin = sorter.begin();
     for (int j=0;j<nint;j++)
       nin++;
   
//	printf("ibox = %d jbox = %d\n",(*nin).numb,(*tin).numb);
//	printf("end of loop :: fintmin = %f  fstar = %f m = %d\n\n",fintmin, fstar,m);
   };

   
//added 2-28-01 to see if the sorting routine is correct 
#if 0
	printf("optimal points          iteration #%2d\n", curr_iter); 
            for (int j = 0; j < optimal.size(); j++) 
              printf(" %d %9.4f \n", optimal[j] , center[optimal[j]].Val); 
            printf("\n"); 
#endif
 
//end addition 
        
/************End locating envelope****************/
//        fprintf(fp2,"t[1] = %d  t[2] = %d \n\n ",t[1], t[2]);

} 
 

/************************************************************************/ 
/* Subroutine to divide the potentially optimal boxes into smaller boxes*/ 
/************************************************************************/ 
 
void DIRECT::Divider(vector<unsigned int>& optimal) 
{
DEBUGPR(10, ucout << "Divider - starting with " << optimal.size() << 
        " potentially optimal Box(es)" << endl);
//
// Main loop
//
vector<unsigned int>::iterator curr = optimal.begin();
vector<unsigned int>::iterator last = optimal.end();
for (curr = optimal.begin(); curr != last; curr++) {
  unsigned int index = *curr;
  DEBUGPR(100, ucout << "Divider - size=" << center[index].size << endl );
  //
  // Do not divide boxes that are smaller than the min_boxsize
  //
  if (center[index].size < min_boxsize)
     EXCEPTION_MNGR(runtime_error, "DIRECT::Divider - begin applied to a box " << index << " that is smaller than min_boxsize=" << min_boxsize);
  //
  // Debugging I/O
  //
  DEBUGPR(100, ucout << "Divider  - box " << 
          index << " " << center[index].l << "\n");
  double lng = center[index].max_dimension();
  DEBUGPR(100, ucout << "Divider  - longest: " << 
          lng << " counters: " << division_counter << "\n");
  //
  // Find the dimensions to divide
  //
  vector<unsigned int> indices;
  //
  // Divide all of the largest dimensions 
  //
  if (division == 2) {
     for (unsigned int i = 0; i < dimen; i++) {
       if (fabs(center[index].l[i]-lng) < (0.001*min_boxsize))
          indices.push_back(i);
       }
     }
  ////
  //// Divide the largest dimension that has been divided the least
  ////
  else {
     unsigned int tmp = 0;
     for (unsigned int i = 1; i < dimen; i++) {
       if ((fabs(center[index].l[i]-lng) < (0.001*min_boxsize)) &&
           (division_counter[i] < division_counter[tmp]))
          tmp = i;
       }
     indices.push_back(tmp);
     // Update the division counter
     division_counter[tmp]++;
     }
  ////
  //// Create the list of points that need to be evaluated
  ////
  vector<direct::point> newpoints;
  vector<unsigned int>::const_iterator icurr = indices.begin();
  vector<unsigned int>::const_iterator ilast = indices.end();
  for (int j=0; icurr != ilast; icurr++) {
    unsigned int i = *icurr;
    DEBUGPR(100, ucout << "Divider  - dividing dimension " << i << "\n");
    //
    // Generate the up and down neighbors within this dimension
    //
    newpoints.push_back(center[index]);
    newpoints[j].x[i] += range[i]*newpoints[j].l[i]/3.0; 
    j++;
    newpoints.push_back(center[index]);
    newpoints[j].x[i] -= range[i]*newpoints[j].l[i]/3.0; 
    j++;
    }
  //
  // Evaluate the new points and add them to the 'center' list
  //
  Eval(newpoints);
  //
  // Update the edges and value of the current point
  //
  icurr = indices.begin();
  {
  //double tmpsize = center[index].size;
  for (; icurr != ilast; icurr++)
    center[index].l[*icurr] /= 3.0; 
  center[index].reset();
  center[index].Val = 
       UPDATE_POINT_VALUE(center[index], constraint_method, constraint_penalty);
  }
  //
  // Divide the new boxes based on the values at the centers 
  //
  ////
  //// Reorder the list of dimensions divided, based on the best value of the 
  //// two points generated in each dimension
  //// Note: this is being sorted with an O(n^2) operation because we do not
  ////    expect to ever be sorting too many indices
  ////
  DEBUGPR(100, ucout << "Divider - initial indices: " << indices << endl);
  int offset = center.size() - newpoints.size();
  vector<int> rank(indices.size());
  for(unsigned int i=0; i<rank.size(); i++)
    rank[i] = i;
  if (indices.size() > 1) {
     for (unsigned int i=0; i<indices.size(); i++) {
       DEBUGPR(100, ucout << "Divider - values: " << center[2*i+offset].Val << 
               "\t" << center[2*i+1+offset].Val << endl);
       for (unsigned int j=i; j<indices.size(); j++) {
         if (min(center[2*i+offset].Val, center[2*i+1+offset].Val) > 
	        min(center[2*j+offset].Val, center[2*j+1+offset].Val)) {
            swap(indices[i], indices[j]);
            swap(rank[i], rank[j]);
            }
         }
       }
     DEBUGPR(100, ucout << "Divider - sorted indices: " << indices << endl);
     }
  ////
  //// Update the edges for the new boxes
  ////
  for (unsigned int i=0; i<indices.size(); i++) {
    for (unsigned int j=0; j<=i; j++) {
      center[2*rank[j]+offset].l[indices[i]] /= 3.0;
      center[2*rank[j]+1+offset].l[indices[i]] /= 3.0;
      }
    }
  for (unsigned int i=0; i<indices.size(); i++) {
      center[2*i+offset].reset();
      center[2*i+1+offset].reset();
      }

#if 0
  for (unsigned int i = (center.size()); i < (center.size()+2*net); i++) 
    for (int j = 0; j < (2*net); j++) { 
      if (divide[j] == 0) { 
         divide[j] = i; 
         break; 
         } 
      if (center[i].Val < center[divide[j]].Val) { 
         for (int k = (2*dimen-1); k >= (j+1); k--) 
           divide[k] = divide[k-1]; 
         divide[j] = i; 
         break; 
         } 
      } 
            
  for (int i = 0; i < net; i++)	{ //divide the boxes based on the sorted values 
    split[i*2] = 0;               //has the box been divided completely yet 
    split[i*2+1] = 0; 
    } 
          
  for (int i = 0; i < (2*net); i++) { 
    k = divide[i]-center.size(); 
    if (split[k] != 1) { 
       for (unsigned int j = (center.size()); j < (center.size()+2*net); j++) 
         if (split[j-center.size()] != 1) 
            center[j].l[dim[(k)/2]] /= 3;         

       split[k] = 1; 
       if ((k/2) == ((k+1)/2))	//integer division???? 
          split[k+1] = 1; 
       else 
          split[k-1] = 1; 
       } 
    }
#endif
  //
  // Update the scale vector used with Jone's constraint handling method
  // 
  if (constraint_method == 1) {
     double z=lng/3;			// z = distance between points 
     for (unsigned int i=center.size()-newpoints.size(); i<center.size(); i++){
       //s0 += fabs((center[i].Val-center[k].Val)/z);
       for (int j=0; j< ncon; j++)
 	 constraint_scale[j] += fabs((center[i].constraint(j)-center[index].constraint(j))/z);
       } 
     }
  } 
DEBUGPR(100,
        ucout << "Divider - the new set of points\n";
        vector<point>::const_iterator icurr = center.begin();
        vector<point>::const_iterator ilast = center.end();
        for (int j=0; icurr != ilast; icurr++, j++) {
           ucout << "Point " << j << endl;
           icurr->write(ucout);
        }
        );
}


void DIRECT::Neighbor(list<direct::simple_sort> &temporder)
{
//
// If the smallest box on the hull is smaller than limit,
// then check all boxes with size less than minsize3 to... TODO
// neighbors for size ratio
//
#if 0
list<direct::simple_sort>::iterator titer = temporder.begin();
if ((*titer).size() > minsize3) return;

   int k,m,q,overlap,neb[3];
   double dist;
   bool border=0;
   direct::bvector VM,VM2;
   double dmax,dmin,tmin,theta,mag,length,Bminside,Nminside;
   int nvect = 0, vmin=0,send,s1,cc;
   list<direct::NeighborHood> Nsort;
   vector<double> vec;
   list<direct::simple_sort> ::iterator titer,titer2,titer3;
   list<direct::NeighborHood> ::iterator niter, niter2, niter3;
   vector<unsigned int> dim(dimen);
   list<int> hull;
   dmax = 0;
   vec.resize(dimen);

   //
   //
   //
   Bminside = 1;
   vector<unsigned int> cover(2*dimen);
   for (unsigned int j=0; j<dimen; j++) {  
     if (center[(*titer).order].l[j] < Bminside)
        Bminside = center[(*titer).order].l[j];
	cover[j]=0;
	cover[j+dimen]=0;
	}
           
   for (unsigned int j=0; j< center.size(); j++)
     if ((*titer).order!=j) {
        m=0;
        q=0;
        overlap = 0;
       	Nminside = 1;

        for (unsigned int k=0; k< dimen; k++) { 
	  if (center[j].l[k] < Nminside)
 	     Nminside = center[j].l[k];
 	              
          if (center[(*titer).order].x[k] < center[j].x[k]) { 
             if (fabs(center[(*titer).order].x[k] + center[(*titer).order].l[k]/2*range[k]+center[j].l[k]/2*range[k] - center[j].x[k])< .00001) { 
                dim[q] = k;
                q++;
		//  printf("k = %d  j = %d  (*titer).order = %d   x1 = %f, x2 = %f, d1 = %f, d2 = %f \n",k,j,(*titer).order,center[(*titer).order].x[k],center[j].x[k],center[(*titer).order].l[k]/2*range[k],center[j].l[k]/2*range[k]);
                }
             else if ((center[(*titer).order].x[k] + center[(*titer).order].l[k]/2*range[k]+center[j].l[k]/2*range[k]) < center[j].x[k]) {
		m=1;
		// printf("j = %4d  m = 1   due to dimen %2d nvect = %3d \n",j,k,nvect);
		}	    
             else if ((center[(*titer).order].x[k] + center[(*titer).order].l[k]/2*range[k]+center[j].l[k]/2*range[k]) > center[j].x[k])
                overlap++;		
	     //   printf("   k = %d  j = %d  (*titer).order = %d   x1 = %f, x2 = %f, d1 = %f/%f, d2 = %f/%f  %f\n",k,j,(*titer).order,center[(*titer).order].x[k],center[j].x[k],center[(*titer).order].l[k]/2*range[k],center[(*titer).order].l[k],center[j].l[k]/2*range[k],center[j].l[k],range[k]);
             }
          else { 
             if (fabs(center[j].x[k] + center[(*titer).order].l[k]/2*range[k]+center[j].l[k]/2*range[k] - center[(*titer).order].x[k]) < .0000001) {
                dim[q] = k;
                q++;
		//  printf(" k = %d  j = %d  (*titer).order = %d   x1 = %f, x2 = %f, d1 = %f, d2 = %f \n",k,j,(*titer).order,center[(*titer).order].x[k],center[j].x[k],center[(*titer).order].l[k]/2*range[k],center[j].l[k]/2*range[k]);
                }
             else if ((center[j].x[k] + center[(*titer).order].l[k]/2*range[k]+center[j].l[k]/2*range[k]) < center[(*titer).order].x[k]) {
		m=1;
		//    printf("j = %4d  m = 1   due to dimen %2d nvect = %3d \n",j,k,nvec);
		}	    
             else if ((center[j].x[k] + center[(*titer).order].l[k]/2*range[k]+center[j].l[k]/2*range[k]) > center[(*titer).order].x[k])
                overlap++;		
	     //  printf("   k = %d  j = %d  (*titer).order = %d   x1 = %f, x2 = %f, d1 = %f/%f, d2 = %f/%f  %f\n",k,j,(*titer).order,center[(*titer).order].x[k],center[j].x[k],center[(*titer).order].l[k]/2*range[k],center[(*titer).order].l[k],center[j].l[k]/2*range[k],center[j].l[k],range[k]);
             }
          }
        int ok = 1;                  
        if ((m == 0) && (q == 1) && (Nminside <= Bminside*Gamma))
	   for (unsigned int h=0; h< dimen; h++)
	     if ((!(dim[0] == h)) && (!(fabs(center[j].x[h] - center[(*titer).order].x[h]) < 0.00001)))
	        ok = 0;
 
        if (((m == 0) && (q <= (dimen)) && (Nminside >= Bminside*Gamma)) || (ok))  		// j is neighbor to i & and must be any dimensional face
           {  
//         printf(" box %d is a neighbor ",j);
           dist = 0;
           for (unsigned int k=0;k< dimen; k++) {
             vec[k]= center[j].x[k] - center[(*titer).order].x[k];
             dist += pow(vec[k]/range[k],2);
             }
           dist = sqrt(dist);
           neb[0] = j;
	   // printf(" box %d is a neighbor  dist = %f \n",j,dist);
                     
           if (dist > dmax)
              dmax = dist;
                     
      	   tmin = pi;
    	   for (unsigned int i=0; i< dimen;i++)		// find face V passes through
	     {
   	     theta = acos(vec[i]);
   	     if (theta < tmin ) {
          	tmin = theta;
          	vmin = i;
          	}
   	     theta = acos(-vec[i]);
   	     if (theta < tmin ) {
          	tmin = theta;
          	vmin = -i;
          	}
	     }

             neb[1] = vmin;		//closest face in neighbor[2][k]

	     //neighbor[3][k] has dimension touched by n-1 dimensional face
     	     if (q == 1) { 
                neb[2] = dim[0];
                if (center[(*titer).order].x[dim[0]] < center[j].x[dim[0]])
                   cover[dim[0]] = j;
                else
                   cover[dim[0]+dimen] = j;		// face has a neighbor
                }     
             else
                neb[2] = 0;                   
             dim[0] =0;
             Nsort.push_back(direct::NeighborHood(vec, neb, dist));							//add vector to V
	     //	printf("- Added %d to Nsort, now have %d neighbors \n",neb[0],Nsort.size());
             }
	}     
      Nsort.sort();
           
      dmin =1;
      for (unsigned int i=0; i< dimen; i++)
        if (center[(*titer).order].l[i] < dmin)
           dmin = center[(*titer).order].l[i];
              
      for (unsigned int i = 0;i < 2*dimen;i++)
        //if no neighbor to face then add dummy vector on C.D.
        if (cover[i] == 0) {
           dist = dmin;
           neb[0] = -1;
           if (i < dimen)
              neb[1] = i;
           else
              neb[1] = -(i-dimen);
           neb[2] = neb[1];
           for (unsigned int l=0;l<dimen;l++)
             vec[l] = 0;
           if (i<dimen)
              vec[i] = 1;
           else
              vec[i-dimen] = -1;
           Nsort.push_front(direct::NeighborHood(vec, neb, dist));           
	   border =1;
           }
           
      /*****Find all of the short enough vectors to send for the first try*****/
      send = Nsort.size();  
      titer = temporder.begin();
      niter = Nsort.begin();
      nvect = send;
           
      if ((*titer).size() > min_boxsize) {
	 niter++;
         for (int j=1;j<send;j++) {
           if ((*niter).dist/dmin > ratio) {
              send = j;
              break;
              }
           niter++;
           }
         niter--;
	 // printf(" There are %d neighbors, sending %3d dist = %f  dmin = %f dmax = %f\n",nvect,send,(*niter).ist,dmin,dmax);
         }
      else {
	 niter++;
	 for (int j=1;j<send;j++) {
	   if ((*niter).dist > minsize2) {
	      send = j;
	      break;
	      }
	   // cout << (*niter).dist << endl;
           niter++;
	   }
	 // printf("**Small** There are %d neighbors, sending %3d dist = %f\n",nvect,send,(*niter).dist);
	 }

      if (send < dimen+1)
	 send = dimen+1;
	   
      s1 = send;
           
      // printf("sending %d short vectors   -   ",send);

      /****Send first batch****/
      VM = Basis(send,Nsort,hull);
           
      // cout << VM.is_p_basis << endl;
           
      /****Check all vectors before adding one at a time****/
      if (!VM.is_p_basis) {
         if (send !=nvect) {
            send = nvect;
            // printf("sending %d vectors to check all - ",send);
            VM2 = Basis(send,Nsort,hull);
            // cout << VM2.is_p_basis << endl;
               
            if (!VM2.is_p_basis)
	       //don't bother checking subsets of vectors, skip the next section
               goto skip_check;
            else {
	       // All of them form a pb so start checking from the s1 set
               send = s1;
	       //  Add all `face' neighbors to list if they are large

	       for (unsigned int i=0;i<2*dimen;i++) {  
	    	 if (cover[i] != 0) {
	            niter = Nsort.begin();
        	    for (int l=1;l<=nvect;l++) {
	              if (cover[i]==(*niter).n[0])
         		 break;
         	      niter++;
         	      }
	  	    dist = (*niter).dist;
	  	    niter3 = niter;
	  	    niter3++;
		    if (dist/dmin > ratio) {
       		       niter2 = Nsort.begin();
       		       for (int l=1; l<=send; l++)
       		      	 niter2++;
		         //niter2 points to the element just past the last one sent
       		       Nsort.insert(niter2,*niter);
		       //,niter3);		//inserts niter before niter2
       		       Nsort.erase(niter);
       		       if ((unsigned)send < (Nsort.size()))
			  send++;
		       else
			  break;
	               }
	            }
	         }
                   
               // printf("sending %d vectors to check all face neighbors   -   \n",send);
               VM = Basis(send,Nsort,hull);
               // cout << VM.is_p_basis << endl;
               }
            }
         }
          
      // printf("send = %d\n\n",send);
           
      while (!VM.is_p_basis) {
        if (send == nvect)
           break;
        dist = l2_norm(VM.V1);
	tmin = pi;
	niter=Nsort.begin();
	for (int i=1;i<send;i++)	//niter -> last sent box
	  niter++;
	niter2 = niter;
        // cout << (*niter).n[0] << endl;
	
        for (int i=send+1; i<= nvect;i++) {		//calculate thetas
          niter2++;
          length = l2_norm((*niter2).V);
          mag = 0;
          for (unsigned int j=0; j< dimen; j++) {
            mag += (-VM.V1[j]*(*niter2).V[j]);
            }
          theta = acos(mag/(length*dist));
	  // cout << (*niter2).n[0] << "  " << (*niter2).V[j] << "  " << dist << "  " << VM.V1[j] << "  " << mag << "  " << length << endl;
          if (theta < tmin ) {			//take closest vector to VM
             tmin = theta;
             niter3 = niter2;
	     // cout << (*niter3).n[0] << "  " << theta << endl;
             }
	  //or accept shortest vector within 20 degrees of VM (.35)
          if (theta < .35) {
             niter3 = niter2;
	     // cout << "adding " << (*niter3).n[0] << " because it is < 20 deg" << endl;
             break;
             }
          }
        // cout << (*niter3).n[0] << "  " << endl;
       	niter++;  //niter points to the element just past the last one sent
       	Nsort.insert(niter,*niter3); //,niter3);//inserts niter before niter2
       	Nsort.erase(niter3);
       	send++;
        VM = Basis(send,Nsort,hull);
        }  

      //printf("send = %d  ratio = %f\n\n",send,ratio);
      skip_check :	// go here if all of the vectors do not form a pb
		
      if (VM.is_p_basis) {
	 m=0;
	 niter = Nsort.begin();
	 for (int l=1;l<s1;l++)
	   niter++;			//niter = last small vector
	 niter2 = Nsort.begin();
	 for (int l=1;l<=send;l++)
	   niter2++;			//niter2 = 1 past last used vector
	             
         //cout << "stops at box " << (*niter2).n[0] << endl;
	 while (niter != niter2)   {
	   bool present = false;
	   titer = temporder.begin();
 	   titer2 = temporder.end();
           // printf("checking for box %d   ratio = %f  temporder1.size =%f\n",(*niter).n[0], (*niter).dist/dmin,(*titer).size);
	   if ((((*niter).dist/dmin > ratio)&&((*titer).size() > min_boxsize))||(((*niter).dist > minsize2)&&((*titer).size() <= min_boxsize))) {
              do { 
		 //	add box to temporder if not there
                 if ((*niter).n[0] == (*titer).order) {
		    //cout << (*niter).n[0] << "   " << (*titer).order<< endl;
                    present = true;
                    break;
                    }
                 titer++;
                 } while(titer != titer2);
              if ( !present) {
#if 0
BUG?
               	          z= center[(*niter).n[0]].size();
                          temporder.push_back(direct::simple_sort((*niter).n[0],z,center[(*niter).n[0]].Val));
                          titer3 = temporder.begin();
// 	          	  titer2 = temporder.end();
// 	          	  titer2--;
//                          printf("%f %d %d %d   %f %f   %d %f %f \n",(*niter).dist,(*niter).n[0],(*niter).n[1],(*niter).n[2],(*niter).V[1],(*niter).V[2],titer2->order,titer2->size,titer2->Val);
//                  	  printf("1adding box %d to optimal because it is too long of a search direction from box %d\n",(*niter).n[0],(*titer3).order);
//                    	  printf("dist = %f and size[%d] = %f\n",dist,i,(*titer).size);
                          m =1;
#endif
                 }
	      }
	   niter++;
           }
         // cout << "next box was " << (*niter).n[0] << endl;
		
	if (border) { //if best box is on a border
           // cout << "on border" << endl;
	   cc = 0;
	   for (unsigned int i=0; i<2*dimen;i++)
	     if (cover[i] == 0)		//if not border of this dimension
		cc++;
	     if (cc == 1)		//only at edge of one dimension
		for (unsigned int i=0; i<dimen;i++)
		  if ((cover[i] != 0) && (cover[i+dimen] != 0))	{
		     //if not border of this dimension
	             bool present = false;
	             titer = temporder.begin();
                     if (center[cover[i]].l[i] > center[(*titer).order].l[i]) {
			// add neighbor on border if it is larger
  			// add box to temporder if not there
 	          	titer2 = temporder.end();
                 	do { 
			   // add box to temporder if not there
    	                   if (cover[i] == (*titer).order) {
			      // cout << cover[i] << "   " << (*titer).order<< endl;
    	      		      present = true;
    	         	      break;
    	             	      }
     	                   titer++;
     	                   } while(titer != titer2);
              		if (!present) {
#if 0
BUG?
               		          z=center[cover[i]].size();
                	          temporder.push_back(direct::simple_sort(cover[i],z,center[cover[i]].Val));
                	          titer3 = temporder.begin();
//                          printf("%f %d %d %d   %f %f   %d %f %f \n",(*niter).dist,(*niter).n[0],(*niter).n[1],(*niter).n[2],(*niter).V[1],(*niter).V[2],titer2->order,titer2->size,titer2->Val);
//             		   	  printf("2adding box %d to optimal because it is too long of a search direction from box %d\n",cover[i],(*titer3).order);
//                    	  printf("dist = %f and size[%d] = %f\n",dist,i,(*titer).size);
                 	         m =1;
#endif
                	   }
                    	}
	             present = false;
	             titer = temporder.begin();
                     if (center[cover[i+dimen]].l[i] > center[(*titer).order].l[i]) {
			// add neighbor on border if it is larger
			// add box to temporder if not there
 	          	titer2 = temporder.end();
                 	do { 
			   // add box to temporder if not there
    	                   if (cover[i+dimen] == (*titer).order) {
			      // cout << cover[i+dimen] << "   " << (*titer).order<< endl;
    	      		      present = true;
    	         	      break;
    	             	      }
     	                   titer++;
     	                   } while(titer != titer2);
              		if ( !present) {
#if 0
BUG?
               		          z=center[cover[i+dimen]].size();
                	          temporder.push_back(direct::simple_sort(cover[i+dimen],z,center[cover[i+dimen]].Val));
                	          titer3 = temporder.begin();
//                          printf("%d %f %d %d %d   %f %f   %d %f %f \n",l,(*niter).dist,(*niter).n[0],(*niter).n[1],(*niter).n[2],(*niter).V[1],(*niter).V[2],(*titer3).order,(*titer3).size,(*titer3).Val);
//           		   	  printf("3adding box %d to optimal because it is too long of a search direction from box %d\n",cover[i+dimen],(*titer3).order);
//                    	  printf("dist = %f and size[%d] = %f\n",dist,i,(*titer).size);
                 	         m =1;
#endif
                	   }
                    	}
                     }
                  else
		     // borders 2 or more edges so all face neighbors on edge
		     for (unsigned int i=1; i<=2*dimen;i++)
		       if (cover[i] != 0) {
			  //check all real face neighbors
		          if (i >= dimen)
		             k = i-dimen;
		          else
		             k = i;
		            
	           	  bool present = false;
	            	  titer = temporder.begin();
                 	  if (center[cover[i]].l[k] > center[(*titer).order].l[k]) {
			     // add neighbor on border if it is larger
			     //	add box to temporder if not there
 	          	     titer2 = temporder.end();
                 	     do { 
    	                        if (cover[i] == (*titer).order) {
				   // cout << cover[i] << "   " << (*titer).order<< endl;
    	      			   present = true;
    	         	           break;
    	             		   }
     	                        titer++;
     	                        } while(titer != titer2);
              		     if ( !present) {
#if 0
BUG?
               		          z=center[cover[i]].size();
                	          temporder.push_back(direct::simple_sort(cover[i],z,center[cover[i]].Val));
                	          titer3 = temporder.begin();
//                          printf("%d %f %d %d %d   %f %f   %d %f %f \n",l,(*niter).dist,(*niter).n[0],(*niter).n[1],(*niter).n[2],(*niter).V[1],(*niter).V[2],(*titer3).order,(*titer3).size,(*titer3).Val);
//             		   	  printf("4adding box %d to optimal because it is too long of a search direction from box %d\n",cover[i],(*titer3).order);
//                    	    printf("dist = %f and size[%d] = %f\n",dist,i,(*titer).size);
                 	         m =1;
#endif
                	        }
                    	     }
                          }
           }

	if (m == 1) {	//remove order[0].order if it is square
           m=0;
	   titer = temporder.begin();
           for (unsigned int k=0;k<dimen-1;k++)
             if (center[(*titer).order].l[k] != center[(*titer).order].l[k+1])
                m=1;
 	   if (m == 0) {                           
	      temporder.erase(temporder.begin());
	      // printf("removed initial box from optimal.\n");
              }
           }
                

	/*************************/
	titer = temporder.begin();
        if ((min_boxsize > 0.0) && (m == 0)&&((*titer).size() <= min_boxsize)) {
	   directbp_converge = true;
	   // cout << "ending in neighbor, stopcond 4" << endl;
	   }
	/*************************/
        }
     else { // NO PB COULD BE FOUND MUST CHOOSE NEIGHBORS TO DIVIDE
        m=0;
	//  Add all `face' neighbors to list if they are large
	//  cout << "adding faces for no PSS" << endl;
	for (unsigned int i=0;i<2*dimen;i++) {  
          // cout << "dimen " << i << cover[i] << endl;
	  if (cover[i] != 0) {
#if 0
BUG?
               	           z = center[cover[i]].size();
			   if (z >(*titer).size)
			      {
        	             	temporder.push_back(direct::simple_sort(cover[i],z,center[cover[i]].Val));
				m=1;
			      }
#endif
	     }
	  }
        // cout << "checking if best should be removed" << endl;
        m=0;
	titer3 = temporder.begin();
	//  cout << (*titer3).order;
        for (unsigned int k=0;k<dimen-1;k++)
          if (center[(*titer3).order].l[k] != center[(*titer3).order].l[k+1])
             m=1;
 	if (m == 0) {                           
	   temporder.erase(temporder.begin());
	   // printf("removed initial box from optimal.\n");
           }
        }
   hull.clear();
   }
#endif
}
                  


/*******************************************************************/ 
/* Positive Spanning Set Calculator                                */ 
/*******************************************************************/ 

direct::bvector DIRECT::Basis(int n, list<direct::NeighborHood> &Nsort, list<int> &hull)
{
const double Estart = 0.2;
const double alphstart = 0.3;
const double interval = 10.0;
int j=0;
double tmax,tmin,alph = alphstart,E = Estart;   
double leps = 0.01,diff, denom;
direct::bvector VM;
    
vector<double> Vnew(dimen);
vector<double> Vold(dimen);
vector<double> theta(n+3);
vector<double> alpha(n+3);
vector<int>    lorder(n+3);
vector<int>    S(n+3);
list<direct::NeighborHood>::iterator niter1;
    
int t = 0;
niter1 = Nsort.begin();

Vnew << 0.0;
Vold << 0.0;
  
for (int i = 0; i < n; i++) {
  //
  // Compute the length of the next neighbor vector
  //
  double mag = length( (*niter1).V );
  //
  // Normalize the vector and add it to Vold
  //
  for (unsigned int k = 0; k < dimen; k++) {
    (*niter1).V[k] = (*niter1).V[k] /mag;
    Vold[k] += (*niter1).V[k];
    }
  niter1++;
  }
double mag = length(Vold);
  
//
// Create a normalized Vold vector in Vnew
//
if (mag >= 0.0001) {
   for (unsigned int k = 0; k < dimen; k++)
     Vnew[k] = Vold[k]/mag;
   }
else { 
   niter1 = Nsort.begin();					
   for (unsigned int j = 0; j < dimen; j++)  {
     for (unsigned int k = 0; k < dimen; k++)
       Vnew[k] += (*niter1).V[k];
     niter1++;
     }
   mag = length(Vnew);
   for (unsigned int k = 0; k < dimen; k++)
     Vnew[k] = Vnew[k]/mag;
   }

//
// A hack to find a good direction for subdivision
//
do {
 t++;
 if (t > interval) {
    E=Estart/(t/interval);
    alph = alphstart/(t/interval);
    }
        
 Vold << Vnew;
 tmax = 0;
 tmin = pi;
 niter1 = Nsort.begin();

 for (int i=0; i< n;i++) {
   mag = 0;
   for (unsigned int j=0; j< dimen; j++)
     mag += (Vnew[j]*(*niter1).V[j]) ; 
   theta[i] = acos(mag);
   if (theta[i] < tmin )
      tmin = theta[i];
   if (theta[i] > tmax)
      tmax = theta[i];
   niter1++;
   }

 //
 // Sort the thetas
 //
 for (int i = 0; i < n; i++)    
   lorder[i]=0;
	      
 int j;
 for (int i = 0; i < n; i++) { 
   for (j = 0; j < n; j++) { 
     //if at end of list put box here 
     if (lorder[j] == 0) { 
        lorder[j] = i; 
        break; 
        } 
     //if smaller than this entry insert here 
     if (theta[i] > theta[lorder[j]]) { 
        for (int k = i; k >= (j+1); k--) 
          lorder[k] = lorder[k-1]; 
        lorder[j] = i; 
        break; 
        } 
     } 
   } 

 for (j=0;j< (int)dimen;j++)
   S[j] = lorder[j];
 if (tmax-theta[lorder[dimen-1]] > E)
    denom = tmax-theta[lorder[dimen-1]];
 else
    denom = E;
             
 for (int i=j; i< n; i++)
   if ( tmax - theta[lorder[i]] < E) {
      S[j] = lorder[i];
      j++;
      }
   else
      break;
           
  for (int i=0; i< j; i++) //Calculate alphas
    alpha[i] = (1-(tmax-theta[S[i]])/(denom*.95))*alph;
      
  mag = 0;
  diff = 0;
  for (unsigned int i=0; i< dimen;i++) {
    for (int k=0;k<j; k++) {
      niter1 = Nsort.begin();
      for (int l =1; l <= S[k]; l++)
        niter1++;
      Vnew[i] = Vnew[i]+alpha[k]*(*niter1).V[i];
      }              
    mag += Vnew[i]*Vnew[i];
    }
  mag = sqrt(mag);
  for (unsigned int i=0; i< dimen;i++) {
    Vnew[i] = Vnew[i]/mag;
    diff += pow(Vnew[i]-Vold[i],2);
    }
  diff = sqrt(diff);
        
  } while ((diff > leps));   //(tmax > pi/2.) && (tmin < pi/2) && 
    
 if ((tmax > (pi*18./36.)) && (tmin < pi/2.))
   VM.is_p_basis = true;
 else
   VM.is_p_basis = false;
  
 VM.V1.resize(dimen);
 for (unsigned int i=0;i<dimen;i++)
   VM.V1[i]=Vnew[i];
   
 hull.clear();
 
 for (int k=0;k<j;k++)    
   hull.push_back(S[k]);
   
 return VM;
} 


void DIRECT::virt_debug_io(ostream& os, const bool finishing,
                                         const int output_level)
{
//
// Only print this info in verbose mode
//
if (output_level < 3) return;

list<direct::simple_sort>::iterator iorder;
iorder = order.begin();
os << "[Min-Box-Size: " << (*iorder).size() << " ]" << endl;

}


void DIRECT::get_final_points(std::vector<std::vector<double> >& pts)
{
std::vector<unsigned int> indeces;
Graham(indeces,0.0,false,false);

std::vector<int> pt_order;
std::vector<double> pt_vals(indeces.size());
pts.resize(indeces.size());
for (unsigned int i=0; i<pts.size(); i++)
  pt_vals[i] = center[indeces[i]].Val;
utilib::order(pt_order,pt_vals);

for (unsigned int i=0; i<pts.size(); i++) {
  pts[i] = center[indeces[pt_order[i]]].x;
  //ucout << "Final Point " << i << " " << center[indeces[pt_order[i]]].Val << std::endl;
  }
}


REGISTER_COLIN_SOLVER_WITH_ALIAS
   (DIRECT, "sco:DIRECT", "sco:direct", "The SCO DIRECT optimizer")

} // namespace scolib

