/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDPOFDarts
//- Description: Class for the Probability of Failure DARTS approach
//- Owner:       Mohamed Ebeida and Laura Swiler
//- Checked by:
//- Version:

#include "NonDPOFDarts.hpp"
#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"
#include "ProblemDescDB.hpp"
#include "pecos_stat_util.hpp"

#include <cstdio>
#include <cassert>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string.h>

#include <time.h>

#include <cmath>
#include <vector>
#include <list>
#include <utility>
#include <stack>
#include <map>
#include <set>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <cstddef>
#include <cstdlib>
#include <limits>

//#define DEBUG

static const char rcsId[] = "@(#) $Id: NonDPOFDart.cpp 6080 2009-09-08 19:03:20Z gtang $";

namespace Dakota {

NonDPOFDarts::NonDPOFDarts(ProblemDescDB& problem_db, Model& model):
  NonD(problem_db, model), seed(probDescDB.get_int("method.random_seed")),
  emulatorOrder(probDescDB.get_int("method.nond.surrogate_order")),
  emulatorType(probDescDB.get_short("method.nond.emulator")),
  samples(probDescDB.get_int("method.samples"))
{
  // any initialization is done here.   For now, you should just specify 
  // the number of samples, but eventually we will get that from the input spec
  Cout << "Hello World, POF Darts is coming! " << '\n';
  Cout << "Number of samples " << samples << '\n';
  Cout << "Surrogate order " << emulatorOrder << '\n';
  Cout << "Emulator Type " << emulatorType << '\n';
 
  if ((emulatorType == GP_EMULATOR) || ( emulatorType == NO_EMULATOR)) { 
    initialize_surrogates(); // initialize one GP surrogate per function
    _use_vor_surrogate = false; // true = use VPS , false = use GP
  }
  else {
    _use_vor_surrogate = true; // true = use VPS , false = use GP
    _vps_order = emulatorOrder;             // order of vps surrogate if used
  } 
   /// /*if (!_use_vor_surrogate)*/ initialize_surrogates(); // initialize one GP surrogate per function
}


NonDPOFDarts::~NonDPOFDarts()
{ }


void NonDPOFDarts::quantify_uncertainty()
{
    
    Cout << "I am now computing uncertainty! " << '\n';
    
    if (fabs(seed - 1.0) < 1E-10) seed  = time(0);
    
    initiate_random_number_generator(seed);
    
    init_pof_darts(); // prepare global variables and containers
    
    execute(1);
    
    exit_pof_darts(); // delete dynamic arrays from memory
    
}
    

//////////////////////////////////////////////////////////////////////////////////////////
/// POF Darts Methods
//////////////////////////////////////////////////////////////////////////////////////////
    
    void NonDPOFDarts::initiate_random_number_generator(unsigned long x)
    {
        //assert(sizeof (double) >= 54) ;
        
        cc = 1.0 / 9007199254740992.0; // inverse of 2^53rd power
        size_t i;
        size_t qlen = indx = sizeof Q / sizeof Q[0];
        for (i = 0; i < qlen; i++) Q[i] = 0;
        
        double c = 0.0, zc = 0.0,	/* current CSWB and SWB `borrow` */
        zx = 5212886298506819.0 / 9007199254740992.0,	/* SWB seed1 */
        zy = 2020898595989513.0 / 9007199254740992.0;	/* SWB seed2 */
        
        size_t j;
        double s, t;	 /* Choose 32 bits for x, 32 for y */
        if (x == 0) x = 123456789; /* default seeds */
        unsigned long y = 362436069; /* default seeds */
        
        /* Next, seed each Q[i], one bit at a time, */
        for (i = 0; i < qlen; i++)
        { /* using 9th bit from Cong+Xorshift */
            s = 0.0;
            t = 1.0;
            for (j = 0; j < 52; j++)
            {
                t = 0.5 * t; /* make t=.5/2^j */
                x = 69069 * x + 123;
                y ^= (y << 13);
                y ^= (y >> 17);
                y ^= (y << 5);
                if (((x + y) >> 23) & 1) s = s + t; /* change bit of s, maybe */
            }	 /* end j loop */
            Q[i] = s;
        } /* end i seed loop, Now generate 10^9 dUNI's: */
    }
    
    double NonDPOFDarts::generate_a_random_number()
    {
        /* Takes 14 nanosecs, Intel Q6600,2.40GHz */
        int i, j;
        double t; /* t: first temp, then next CSWB value */
        /* First get zy as next lag-2 SWB */
        t = zx - zy - zc;
        zx = zy;
        if (t < 0)
        {
            zy = t + 1.0;
            zc = cc;
        }
        else
        {
            zy = t;
            zc = 0.0;
        }
        
        /* Then get t as the next lag-1220 CSWB value */
        if (indx < 1220)
            t = Q[indx++];
        else
        { /* refill Q[n] via Q[n-1220]-Q[n-1190]-c, */
            for (i = 0; i < 1220; i++)
            {
                j = (i < 30) ? i + 1190 : i - 30;
                t = Q[j] - Q[i] + c; /* Get next CSWB element */
                if (t > 0)
                {
                    t = t - cc;
                    c = cc;
                }
                else
                {
                    t = t - cc + 1.0;
                    c = 0.0;
                }
                Q[i] = t;
            }	 /* end i loop */
            indx = 1;
            t = Q[0]; /* set indx, exit 'else' with t=Q[0] */
        } /* end else segment; return t-zy mod 1 */
        
        return ((t < zy) ? 1.0 + (t - zy) : t - zy);	
    }
    
    void NonDPOFDarts::init_pof_darts()
    {
        _accepted_void_ratio = 1E-9;
        
        _n_dim = numContinuousVars;
        
        _total_budget = 0;
        
        initialize_distribution_mappings();
        for (size_t resp_fn_count=0; resp_fn_count<numFunctions; resp_fn_count++)
        {
            size_t num_levels = requestedRespLevels[resp_fn_count].length();
            
            for (size_t level_count=0; level_count<num_levels; level_count++) _total_budget += samples;
        }
        
        size_t kd(1); // use line darts for finding voids between disks
        double p = _accepted_void_ratio;
        if (kd == 1)
        {
            p = pow(_accepted_void_ratio, double(_n_dim - kd) / double(_n_dim)); // probability of a hit of one flat
            p = 1 - pow(1.0 - p, double(_n_dim)); // probability of a hit of any flat
        }
        _max_num_successive_misses = ceil(1.0 / p); // successive missed for the line darts algorithm
        
        size_t max_num_points(_total_budget);
        
        _sample_points = new double*[max_num_points];
        _dart = new double[_n_dim];
        
        _line_flat = new size_t[_n_dim];
        _line_flat_start = new double[max_num_points];
        _line_flat_end = new double[max_num_points];
        _line_flat_length = new double[max_num_points];
        
        _xmin = new double[_n_dim];
        _xmax = new double[_n_dim];
        
        const RealVector&  lower_bounds = iteratedModel.continuous_lower_bounds();
        const RealVector&  upper_bounds = iteratedModel.continuous_upper_bounds();
        
        // default domain is for the Herbie Function
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            _xmin[idim] = lower_bounds[idim];
            _xmax[idim] = upper_bounds[idim];
        }
        
        // Estmate max radius of spheres based on number of evaluation and unifrom distribution
        _diag = 0.0;
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            double dx = _xmax[idim] - _xmin[idim];
            _diag += dx * dx;
        }
        _diag = sqrt(_diag);
        _max_radius = 0.25 * _diag;
        
        
        _fval = new double*[numFunctions];
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++) _fval[resp_fn_count] = new double[_total_budget];
        
        _Lip = new double[numFunctions];
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++) _Lip[resp_fn_count] = 0.0;
        
    }
    
    void NonDPOFDarts::exit_pof_darts()
    {
        delete[] _dart;
        delete[] _line_flat;
        delete[] _line_flat_start;
        delete[] _line_flat_end;
        delete[] _line_flat_length;
        delete[] _xmin;
        delete[] _xmax;
        for (size_t isample = 0; isample < _num_inserted_points; isample++) delete[] _sample_points[isample];
        delete[] _sample_points;
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++) delete[] _fval[resp_fn_count];
        
        delete[] _fval;
        delete[] _Lip;
    }
    
    void NonDPOFDarts::execute(size_t kd)
    {
       
        clock_t start_time, end_time; double cpu_time, total_time(0.0);
       
        _num_inserted_points = 0; _num_darts = 0;
        
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            _active_response_function = resp_fn_count;
            
            size_t num_levels = requestedRespLevels[resp_fn_count].length();
            
            for (size_t level_count = 0; level_count<num_levels; level_count++)
            {
                _failure_threshold = requestedRespLevels[resp_fn_count][level_count];
                
                // adjust prior sphere radii to reflect current response function and threshold
                for (size_t isample = 0; isample < _num_inserted_points; isample++) assign_sphere_radius_POF(_sample_points[isample], isample);
                
                /*
                if (resp_fn_count == 1)
                {
                    Cout << "pof:: Plotting in mps_shphere.ps ... ";
                    plot_vertices_2d();
                    return;
                }
                */
                
                start_time = clock();
                if (kd == 0)
                {
                    Cout <<  "pof:: Classical Point-Dart Throwing Games ... ";
                    classical_dart_throwing_games(0);
                }
                else if (kd == 1)
                {
                    Cout <<  "pof:: Classical Line-Dart Throwing Games ... ";
                    line_dart_throwing_games(0);
                }
                end_time = clock();
                cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
            
                std::cout.precision(0);
                std::cout<<  "pof::    Number of inserted points = " << std::fixed << _num_inserted_points << std::endl;
                std::cout<<  "pof::    Number of thrown darts = " << std::fixed << _num_darts << std::endl;
                std::cout.precision(4);
                std::cout<<  "pof::    Execution Time = " << std::fixed << cpu_time << " seconds." << std::endl;
                
            }
        }
        
        std::cout <<  "pof:: Building Surrogates ..." << std::endl;
        if (_use_vor_surrogate)
        {
            VPS_execute();
            estimate_pof_VPS();
        }
        else estimate_pof_surrogate();
        
        if (_n_dim == 2 && false)
        {
            std::cout <<  "pof::    Plotting 2d disks ...";
            plot_vertices_2d(true, true);
        }
        
        return;
    }

    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MPS methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    
    void NonDPOFDarts::classical_dart_throwing_games(size_t game_index)
    {
        _num_successive_misses_p = 0;
        _num_successive_misses_m = 0;
        size_t num_prior_disks = _num_inserted_points;
        while(true)
        {
            // sampling from a bounding box
            for (size_t idim = 0; idim < _n_dim; idim++) _dart[idim] = _xmin[idim] + generate_a_random_number() * (_xmax[idim] - _xmin[idim]);
            
            if (valid_dart(_dart))
            {
                add_point(_dart);
                _num_successive_misses_m = _num_successive_misses_p;
                _num_successive_misses_p = 0;
                if (_num_inserted_points - num_prior_disks == samples)
                {
                    std::cout<< "\npof:: Simulation Budget has been exhausted!" << std::endl;
                    return;
                }
            }
            else
            {
                _num_successive_misses_p++;
                if (_num_successive_misses_p + _num_successive_misses_m > _max_num_successive_misses)
                {
                    std::cout<< "\npof:: Void-finding budget has been exhausted, shrinking all disks!" << std::endl;
                    shrink_big_spheres();
                    
                }
            }
        }
    }
    
    void NonDPOFDarts::line_dart_throwing_games(size_t game_index)
    {
        
        _num_successive_misses_p = 0;
        _num_successive_misses_m = 0;
        size_t num_prior_disks = _num_inserted_points;
        while (true)
        {
            _num_darts++;
            
            // randomize the d flats
            for (size_t idim = 0; idim < _n_dim; idim++) _line_flat[idim] = idim;
            
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double u = generate_a_random_number();
                size_t h = _n_dim - idim;
                size_t jdim = idim + size_t(floor(u * h));
                if (jdim == _n_dim) jdim--;
                size_t tmp = _line_flat[idim];
                _line_flat[idim] = _line_flat[jdim];
                _line_flat[jdim] = tmp;
            }
            
            
            bool covered(true);
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                // check if this flat has a point
                for (size_t jdim = 0; jdim < _n_dim; jdim++) _dart[jdim] = _xmin[jdim] + generate_a_random_number() * (_xmax[jdim] - _xmin[jdim]);
                
                _flat_dim = _line_flat[idim];
                _num_flat_segments = 1;
                _line_flat_start[0] = _xmin[_flat_dim]; _line_flat_end[0] = _xmax[_flat_dim];
                
                if (_num_inserted_points == 0 || valid_line_flat(_flat_dim, _dart))
                {
                    // sample from the line segements
                    double L_seg(0.0);
                    for (size_t j = 0; j < _num_flat_segments; j++)
                    {
                        _line_flat_length[j] = _line_flat_end[j] - _line_flat_start[j];
                        L_seg += _line_flat_length[j];
                        _line_flat_length[j] = L_seg;
                    }
                    
                    double t = generate_a_random_number() * L_seg;
                    
                    size_t ist = 0;
                    size_t iend = _num_flat_segments - 1;
                    while (ist != iend) // binary search
                    {
                        size_t imid = (ist + iend) / 2;
                        
                        if (_line_flat_length[imid] < t) ist = imid + 1;
                        else iend = imid;
                    }
                    
                    // Length of line segement
                    double tmpLseg = _line_flat_length[ist];
                    if (ist > 0) tmpLseg -= _line_flat_length[ist - 1];
                    
                    // sample from this line segment ist
                    if (ist > 0) t -= _line_flat_length[ist - 1];
                    _dart[_flat_dim] = _line_flat_start[ist] + t;
                    
                    add_point(_dart); covered = false;
                    
                    if (_num_inserted_points - num_prior_disks == samples)
                    {
                        std::cout<< "\npof:: Simulation Budget has been exhausted!" << std::endl;
                        return; 
                    }
                    
                    break;					
                }
            }
            
            if (covered) 
            {
                _num_successive_misses_p++;
                if (_num_successive_misses_p + _num_successive_misses_m > _max_num_successive_misses) 
                {
                    std::cout<< "\npof:: Void-finding budget has been exhausted, shrinking all disks!" << std::endl;
                    shrink_big_spheres();
                }
            }
            else 
            {
                _num_successive_misses_m = _num_successive_misses_p;
                _num_successive_misses_p = 0;
            }
        }
    }

    bool NonDPOFDarts::valid_dart(double* x)
    {
        for (size_t index = 0; index < _num_inserted_points; index++)
        {
            double dd(0.0);
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double dx = x[idim] - _sample_points[index][idim];
                dd += dx * dx;
            }
            
            if (dd < fabs(_sample_points[index][_n_dim])) return false; // prior disk approach
        }
        return true;
    }
    
    bool NonDPOFDarts::valid_line_flat(size_t flat_dim, double* flat_dart)
    {
        for (size_t index = 0; index < _num_inserted_points; index++)
        {
            double hh(0.0);
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                if (idim == flat_dim) continue;
                
                double dx = flat_dart[idim] - _sample_points[index][idim];
                hh += dx * dx;
            }
            
            if (hh < fabs(_sample_points[index][_n_dim]))
            {
                double r_sq = fabs(_sample_points[index][_n_dim]);
                double v = sqrt(r_sq - hh);
                double proj = _sample_points[index][flat_dim];
                double segmin = proj - v;
                double segmax = proj + v;
                
                // binary search to find segmin in flat segments
                size_t ist(0);
                size_t iend(_num_flat_segments - 1);
                while (ist != iend)
                {
                    size_t imid = (ist + iend) / 2;
                    
                    if (imid == ist)
                    {
                        iend = ist;
                        break;
                    }
                    
                    if (_line_flat_start[imid] < segmin) ist = imid;
                    else iend = imid;
                }
                
                // remove the line segment from _sample_points[index][flat_dim] - v to _sample_points[index][flat_dim] + v
                while (iend < _num_flat_segments)
                {
                    double xmin = _line_flat_start[iend]; double xmax = _line_flat_end[iend];
                    
                    if (segmax < xmin) break;
                    
                    if (segmax > xmin && segmax < xmax && segmin < xmin)
                    {
                        // Adjust line segment start
                        _line_flat_start[iend] = segmax; break;
                    }
                    else if (segmin > xmin && segmax < xmax)
                    {
                        // split line segment
                        for (size_t jseg = _num_flat_segments; jseg > iend; jseg--)  // iend >=0
                        {
                            _line_flat_start[jseg] = _line_flat_start[jseg - 1];
                            _line_flat_end[jseg]   = _line_flat_end[jseg - 1];
                        }
                        _line_flat_end[iend] = segmin;
                        _line_flat_start[iend + 1] = segmax;
                        _num_flat_segments++;			
                        break;
                    }
                    else if (segmin > xmin && segmin < xmax && segmax > xmax) 
                    {
                        _line_flat_end[iend] = segmin; iend++;
                    }
                    else if (xmin > segmin && xmax < segmax)
                    {
                        // delete line segment
                        _num_flat_segments--;
                        for (size_t jseg = iend; jseg < _num_flat_segments; jseg++)
                        {
                            _line_flat_start[jseg] = _line_flat_start[jseg + 1];
                            _line_flat_end[jseg]   = _line_flat_end[jseg + 1];
                        }				
                    }			
                    else iend++;
                }
            }
            
            if (_num_flat_segments == 0) return false;
            
        }
        return true;
    }
    
    void NonDPOFDarts::add_point(double* x)
    {
        _sample_points[_num_inserted_points] = new double[_n_dim + 1];
        
        for (size_t idim = 0; idim < _n_dim; idim++) _sample_points[_num_inserted_points][idim] = x[idim];
        
        assign_sphere_radius_POF(x, _num_inserted_points);
        
        // adjust prior sphere radii to reflect new Lipschitz constant
        for (size_t isample = 0; isample < _num_inserted_points; isample++) assign_sphere_radius_POF(_sample_points[isample], isample);
        
        _num_inserted_points++;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // POF methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void NonDPOFDarts::assign_sphere_radius_POF(double* x, size_t isample)
    {
        if (isample == _num_inserted_points)  compute_response_update_Lip(x);
        
        double fval = _fval[_active_response_function][isample];
        
        double r = _max_radius;
        
        if (_Lip[_active_response_function] > 1E-10)
        {
            r = fabs(fval - _failure_threshold) / _Lip[_active_response_function];
            if (r > _max_radius)
            {
                r = _max_radius;
                _Lip[_active_response_function] = fabs(fval - _failure_threshold) / r;
            }
        }
        
        _sample_points[isample][_n_dim] = r * r;
        
        if (fval < _failure_threshold) _sample_points[isample][_n_dim] = - _sample_points[isample][_n_dim];
    }
    
    void NonDPOFDarts::compute_response_update_Lip(double* x)
    {
        RealVector newX(_n_dim);
        for (size_t idim = 0; idim < _n_dim; idim++) newX[idim] = x[idim];
        
        iteratedModel.continuous_variables(newX);
        iteratedModel.compute_response();
        
        add_surrogate_data(iteratedModel.current_variables(), iteratedModel.current_response());
        
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            double fval = iteratedModel.current_response().function_value(resp_fn_count);
            _fval[resp_fn_count][_num_inserted_points] = fval;
            
            // update Lipschitz constant
            for (size_t isample = 0; isample < _num_inserted_points; isample++)
            {
                double dst = 0.0;
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    double dx = x[idim] - _sample_points[isample][idim];
                    dst += dx * dx;
                }
                dst = sqrt(dst);
                double L = fabs(fval - _fval[resp_fn_count][isample]) / dst;
                if (L > _Lip[resp_fn_count]) _Lip[resp_fn_count] = L;
            }
        }
    }
    
    void NonDPOFDarts::shrink_big_spheres()
    {
        double rr_max(0.0);
        for (size_t isample = 0; isample < _num_inserted_points; isample++)
        {
            if (fabs(_sample_points[isample][_n_dim]) > rr_max) rr_max = fabs(_sample_points[isample][_n_dim]);
        }
        
        Cout << "rr_max = " << rr_max << std::endl;
        
        for (size_t isample = 0; isample < _num_inserted_points; isample++)
        {
            if (fabs(_sample_points[isample][_n_dim]) > 0.81 * rr_max) _sample_points[isample][_n_dim] *= 0.81;
        }
        
    }
    
 
    double NonDPOFDarts::area_triangle(double x1, double y1, double x2, double y2, double x3, double y3)
    {
        return 0.5 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));
    }
    
    ////////////////////////////////////////////////////////////////
    // SUUROGATE METHODS
    ////////////////////////////////////////////////////////////////
    
    void NonDPOFDarts::initialize_surrogates()
    {
        // create the data to configure the surrogate
        // String approx_type("global_gaussian");  // Dakota GP
        String approx_type("global_kriging");  // Surfpack GP
        UShortArray approx_order;
        short data_order = 1;  // assume only function values
        short output_level = QUIET_OUTPUT;
        sharedData = SharedApproxData(approx_type, approx_order, numContinuousVars, data_order, output_level);
        //gpApproximations.resize(numFunctions, Approximation(sharedData));
        for (size_t i=0; i < numFunctions; ++i)
        {
            gpApproximations.push_back(Approximation(sharedData));
        }
        // setup a variables object for evaluating the model
        gpEvalVars = iteratedModel.current_variables().copy();
    }
    
    void NonDPOFDarts::add_surrogate_data(const Variables& vars, const Response& resp)
    {
        for (size_t i=0; i < numFunctions; ++i)
        {
            bool anchor_flag = false;
            bool deep_copy = true;
            gpApproximations[i].add(vars, anchor_flag, deep_copy);
            gpApproximations[i].add(resp, i, anchor_flag, deep_copy);
        }
    }
    
    void NonDPOFDarts::build_surrogate(size_t fn_index)
    {
        gpApproximations[fn_index].build();
    }
    
    // evaluate at vector of x values in vin
    Real NonDPOFDarts::eval_surrogate(size_t fn_index, double *vin)
    {
        if (_use_vor_surrogate)
        {
            // retrieve closest sample point
            double min_dst_sq(_diag * _diag);
            size_t iclosest(_num_inserted_points);
            for (size_t jsample = 0; jsample < _num_inserted_points; jsample++)
            {
                double dst_sq(0.0);
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    double dx = _sample_points[jsample][idim] - vin[idim];
                    dst_sq += dx * dx;
                }
                if (dst_sq < min_dst_sq)
                {
                    iclosest = jsample;
                    min_dst_sq = dst_sq;
                }
            }
            
            // peicewise surrogate using Vornoi Cells
            double f_surrogate(_fval[fn_index][iclosest]);
            if (_vor_order > 0)
            {
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    double dx = vin[idim] - _sample_points[iclosest][idim];
                    f_surrogate+= _node_coeff[iclosest][fn_index][idim] * dx;
                }
            }
            return f_surrogate;
        }
        
        // this copy could be moved outside the loop for memory efficiency
        for (size_t vi = 0; vi < numContinuousVars; ++vi)
            gpEvalVars.continuous_variable(vin[vi], vi);
        
        return gpApproximations[fn_index].value(gpEvalVars);
    }
    
    void NonDPOFDarts::estimate_pof_surrogate()
    {
        clock_t start_time, end_time; double cpu_time, total_time(0.0);
        
        // build the surrogate for the given function
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            start_time = clock();
            
            build_surrogate(resp_fn_count);
            
            end_time = clock();
            cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
            
            std::cout.precision(4);
            std::cout <<  "pof::    Surrogate " << resp_fn_count << " built in " << std::fixed << cpu_time << " seconds." << std::endl;
        }

         // evaluate the surrogate for the given function
        double** num_fMC_samples = new double*[numFunctions];
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            size_t num_levels = requestedRespLevels[resp_fn_count].length();
            num_fMC_samples[resp_fn_count] = new double[num_levels];
            for (size_t level_count = 0; level_count < num_levels; level_count++) num_fMC_samples[resp_fn_count][level_count] = 0.0;
        }
        
        start_time = clock();

        //double pof_exact = 0.0;
        
        double isample = 0.0;
        double num_MC_samples(1E6);
        double* tmp_pnt = new double[_n_dim];
        while (isample < num_MC_samples)
        {
            // sample a random point from the domain
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                tmp_pnt[idim] = _xmin[idim] + generate_a_random_number() * (_xmax[idim] - _xmin[idim]);
            }
            
            //double f_exact = f_true(tmp_pnt);
            
            for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
            {
                // evaluate sample point using surrogate
                double surrogate_value = eval_surrogate(resp_fn_count, tmp_pnt);

                size_t num_levels = requestedRespLevels[resp_fn_count].length();
                for (size_t level_count = 0; level_count < num_levels; level_count++)
                {
                    _failure_threshold = requestedRespLevels[resp_fn_count][level_count];
                    
                    if (surrogate_value <  _failure_threshold)
                    {
                        num_fMC_samples[resp_fn_count][level_count] += 1.0;
                    }
                    
                    //if (f_exact <  _failure_threshold) pof_exact+=1.0;
                }
            }
            isample+=1.0;
        }
        end_time = clock();
        cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
        
        std::cout <<  "pof::    Surrogate evaluated in " << std::fixed << cpu_time << " seconds." << std::endl;
        
        //std::cout.precision(15);
        //std::cout <<  "pof::    Surrogate error " << std::fixed << fabs((num_fMC_samples[0][0] - pof_exact) / num_MC_samples) << " seconds." << std::endl;
        
        
        // Passing results to gloabl containers
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            size_t num_levels = requestedRespLevels[resp_fn_count].length();
            for (size_t level_count = 0; level_count < num_levels; level_count++)
            {
                double pof_val = num_fMC_samples[resp_fn_count][level_count] / num_MC_samples;
                computedProbLevels[resp_fn_count][level_count] = pof_val;
            }
        }
        
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++) delete[] num_fMC_samples[resp_fn_count];
        delete[] num_fMC_samples;
        delete[] tmp_pnt;
    }

    
    bool NonDPOFDarts::trim_line_using_Hyperplane(size_t num_dim,                               // number of dimensions
                                                       double* st, double *end,                      // line segmenet end points
                                                       double* qH, double* nH)                // a point on the hyperplane and it normal
    {
        double dotv(0.0), dote(0.0);
        for (size_t idim = 0; idim < num_dim; idim++)
        {
            double dxv = qH[idim] - st[idim];
            double dxe = end[idim] - st[idim];
            dotv += dxv * nH[idim];
            dote += dxe * nH[idim];
        }
        
        if (fabs(dote) < 1E-10) // cos theta_n < 1E-10
        {
            // line is either degenerate or parallel to Hyperplane
            return false;
        }
        
        if (fabs(dote) < fabs(dotv))
        {
            // line lie on one side of the Hyperplane
            if (dotv < 0.0)
            {
                for (size_t idim = 0; idim < num_dim; idim++) end[idim] = st[idim];
            }
            return false;
        }
        
        double u = dotv / dote;
        if (u > 0.0 && u < 1.0)
        {
            for (size_t idim = 0; idim < num_dim; idim++)
            {
                if (dote > 0.0) end[idim] = st[idim] + u * (end[idim] - st[idim]);
                else st[idim] = st[idim] + u * (end[idim] - st[idim]);
            }
            return true;
        }
        
        if (u < 0.0 && dote > 0.0)
        {
            for (size_t idim = 0; idim < num_dim; idim++) end[idim] = st[idim];
            return false;
        }
        
        if (u > 1.0 && dote < 0.0)
        {
            for (size_t idim = 0; idim < num_dim; idim++) end[idim] = st[idim];
            return false;
        }
        return false;
    }
    
    //////////////////////////////////////////////////////////////
    // VPS METHODS
    //////////////////////////////////////////////////////////////
    
    bool NonDPOFDarts::VPS_execute()
    {
        clock_t start_time, end_time; double cpu_time, total_time(0.0);
        
        start_time = clock();
        
        _num_GMRES = 0;
        
        _vps_num_points = _num_inserted_points;
        _vps_num_functions = numFunctions;
        _vps_x = new double*[_vps_num_points];
		for (size_t i = 0; i < _vps_num_points; i++)
        {
            _vps_x[i] = new double[_n_dim];
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
				_vps_x[i][idim] = _sample_points[i][idim];
            }
        }
        
        _vps_f = new double*[_vps_num_points];
        for (size_t ipoint = 0; ipoint < _vps_num_points; ipoint++)
        {
            _vps_f[ipoint] = new double[_vps_num_functions];
            
            for (size_t ifunc = 0; ifunc < _vps_num_functions; ifunc++) _vps_f[ipoint][ifunc] = _fval[ifunc][ipoint];
        }
        
        _vps_dfar = new double[_vps_num_points];
        
        VPS_retrieve_neighbors_of_all_points_from_scratch();
        
        // retrive powers of the polynomial expansion
        retrieve_permutations(_vps_num_poly_terms, _vps_t, _n_dim, _vps_order, false, true, _vps_order);
        
        
        // extend neighbors for all points to match the desired max power per cell
        VPS_adjust_extend_neighbors_of_all_points();
        
        VPS_retrieve_poly_coefficients_for_all_points();
        
        end_time = clock();
        cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
        
        std::cout << "pof::    Number of polynomial coeffcients = " << std::fixed << _vps_num_poly_terms << std::endl;
        std::cout << "pof::    Number of GMRES solves = " << std::fixed << _num_GMRES << std::endl;
        std::cout << "pof::    VPS Surrogate built in " << std::fixed << cpu_time << " seconds." << std::endl;
        
        return true;
    }
    
    void NonDPOFDarts::VPS_retrieve_neighbors_of_all_points_from_scratch()
    {
        _vps_neighbors = new size_t*[_vps_num_points];
        _vps_ext_neighbors = new size_t*[_vps_num_points];
        for (size_t ipoint = 0; ipoint < _vps_num_points; ipoint++)
        {
            _vps_neighbors[ipoint] = 0;
            _vps_ext_neighbors[ipoint] = 0;
            VPS_retrieve_neighbors(ipoint);
        }
    }
    
    void NonDPOFDarts::VPS_retrieve_neighbors(size_t ipoint)
    {
        size_t* old_neighbors = _vps_neighbors[ipoint];
        
        size_t* tmp_neighbors = new size_t[_vps_num_points];
        
        double* tmp_pnt = new double[_n_dim];    // end of spoke
        double* qH = new double[_n_dim];         // mid-ppint
        double* nH = new double[_n_dim];         // normal vector
        
        size_t num_neighbors(0), num_misses(0), max_misses(20);
        while (num_misses < max_misses)
        {
            // sample point uniformly from a unit sphere
            double sf = 0.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double sum(0.0);
                // select 12 random numbers from 0.0 to 1.0
                for (size_t i = 0; i < 12; i++) sum += generate_a_random_number();
                sum -= 6.0;
                tmp_pnt[idim] = sum;
                sf += tmp_pnt[idim] * tmp_pnt[idim];
            }
            sf = 1.0 / sqrt(sf);
            for (size_t idim = 0; idim < _n_dim; idim++) tmp_pnt[idim] *= sf;
            
            // scale line spoke to extend outside bounding box
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                tmp_pnt[idim] *= _diag;
                tmp_pnt[idim] += _vps_x[ipoint][idim];
            }
            
            // trim line spoke with domain boundaries
            double t_end(1.0);
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                if (tmp_pnt[idim] > _xmax[idim])
                {
                    double t = (_xmax[idim] - _vps_x[ipoint][idim]) / (tmp_pnt[idim] - _vps_x[ipoint][idim]);
                    if (t < t_end) t_end = t;
                }
                if (tmp_pnt[idim] < _xmin[idim])
                {
                    double t = (_vps_x[ipoint][idim] - _xmin[idim]) / (_vps_x[ipoint][idim] - tmp_pnt[idim]);
                    if (t < t_end) t_end = t;
                }
            }
            for (size_t idim = 0; idim < _n_dim; idim++) tmp_pnt[idim] = _vps_x[ipoint][idim] + t_end * (tmp_pnt[idim] - _vps_x[ipoint][idim]);
            
            // trim spoke using Voronoi faces
            size_t ineighbor(ipoint);
            for (size_t jpoint = 0; jpoint < _vps_num_points; jpoint++)
            {
                if (jpoint == ipoint) continue;
                
                // trim line spoke via hyperplane between
                double norm(0.0);
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    qH[idim] = 0.5 * (_vps_x[ipoint][idim] + _vps_x[jpoint][idim]);
                    nH[idim] =  _vps_x[jpoint][idim] -  _vps_x[ipoint][idim];
                    norm+= nH[idim] * nH[idim];
                }
                norm = 1.0 / sqrt(norm);
                for (size_t idim = 0; idim < _n_dim; idim++) nH[idim] *= norm;
                
                if (trim_line_using_Hyperplane(_n_dim, _vps_x[ipoint], tmp_pnt, qH, nH))
                {
                    ineighbor = jpoint;
                }
            }
            
            if (ineighbor == ipoint) continue; // boundary neighbor
            
            // check if new neighbor is found
            bool new_neighbor(true);
            for (size_t i = 0; i < num_neighbors; i++)
            {
                if (tmp_neighbors[i] == ineighbor)
                {
                    new_neighbor = false;
                    break;
                }
            }
            
            if (!new_neighbor)
            {
                // old neighbor = a miss
                num_misses++;
                continue;
            }
            
            // a hit
            num_misses = 0;
            tmp_neighbors[num_neighbors] = ineighbor;
            num_neighbors++;
        } // end of spoke loop
        
        if (old_neighbors != 0)
        {
            delete[] old_neighbors;
        }
        
        _vps_neighbors[ipoint] = new size_t[num_neighbors + 1];
        _vps_neighbors[ipoint][0] = num_neighbors;
        _vps_ext_neighbors[ipoint] = new size_t[num_neighbors + 1];
        _vps_ext_neighbors[ipoint][0] = num_neighbors;
        for (size_t i = 0; i < num_neighbors; i++)
        {
            _vps_neighbors[ipoint][i + 1] = tmp_neighbors[i];
            _vps_ext_neighbors[ipoint][i + 1] = tmp_neighbors[i];
        }
        
        delete[] tmp_neighbors;
        delete[] tmp_pnt;
        delete[] qH;
        delete[] nH;
    }

    void NonDPOFDarts::VPS_adjust_extend_neighbors_of_all_points()
    {
        for (size_t ipoint = 0; ipoint < _vps_num_points; ipoint++)
        {
            while (_vps_ext_neighbors[ipoint][0] < 2 * _vps_num_poly_terms && _vps_ext_neighbors[ipoint][0] < _vps_num_points - 1)
            {
                VPS_extend_neighbors(ipoint);
            }
        }
        
        for (size_t ipoint = 0; ipoint < _vps_num_points; ipoint++)
        {
            _vps_dfar[ipoint] = 0.0;
            for (size_t i = 1; i <= _vps_ext_neighbors[ipoint][0]; i++)
            {
                size_t neighbor = _vps_ext_neighbors[ipoint][i];
                double dst = 0.0;
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    double dx = _vps_x[ipoint][idim] - _vps_x[neighbor][idim];
                    dst += dx * dx;				
                }	
                dst = sqrt(dst);
                if (dst > _vps_dfar[ipoint]) _vps_dfar[ipoint] = dst;
            }
        }
    }
    
    void NonDPOFDarts::VPS_extend_neighbors(size_t ipoint)
    {
        // initiate tmp_neighbors with old extended neighbors
        size_t num_ext_neigbors = _vps_ext_neighbors[ipoint][0];
        size_t* tmp_neighbors = new size_t[_vps_num_points];
        for (size_t i = 1; i <= _vps_ext_neighbors[ipoint][0]; i++) tmp_neighbors[i - 1] = _vps_ext_neighbors[ipoint][i];
        
        for (size_t i = 1; i <= _vps_ext_neighbors[ipoint][0]; i++)
        {
            size_t neighbor = _vps_ext_neighbors[ipoint][i];
            for (size_t j = 1; j <= _vps_neighbors[neighbor][0]; j++) // extend neighbors of ipoint using direct neighbors of neighbor
            {
                size_t ext_neighbor = _vps_neighbors[neighbor][j];
                if (ext_neighbor == ipoint) continue;
                bool new_neighbor = true;
                for (size_t k= 0; k < num_ext_neigbors; k++)
                {
                    if (ext_neighbor == tmp_neighbors[k])
                    {
                        new_neighbor = false;
                        break;
                    }
                }
                if (new_neighbor)
                {
                    tmp_neighbors[num_ext_neigbors] = ext_neighbor;
                    num_ext_neigbors++;
                }
            }
        }
        
        _vps_ext_neighbors[ipoint] = new size_t[num_ext_neigbors + 1];
        _vps_ext_neighbors[ipoint][0] = num_ext_neigbors;
        for (size_t i = 0; i < num_ext_neigbors; i++)
        {
            _vps_ext_neighbors[ipoint][i + 1] = tmp_neighbors[i];
        }	
        delete[] tmp_neighbors;
    }

    void NonDPOFDarts::VPS_retrieve_poly_coefficients_for_all_points()
    {
        _vps_c = new double**[_vps_num_points];
        for (size_t ipoint = 0; ipoint < _vps_num_points; ipoint++)
        {
            _vps_c[ipoint] = new double*[_vps_num_functions];
            for (size_t ifunction = 0; ifunction < _vps_num_functions; ifunction++)
            {
                _vps_c[ipoint][ifunction] = new double[_vps_num_poly_terms];
                VPS_retrieve_poly_coefficients(ipoint, ifunction);
            }
        }
    }
    
    void NonDPOFDarts::VPS_retrieve_poly_coefficients(size_t ipoint, size_t function_index)
    {
        double** A = new double*[_vps_num_poly_terms];
        double** LD = new double*[_vps_num_poly_terms];
        double* b = new double[_vps_num_poly_terms];
        
        double* x_neighbor = new double[_n_dim];
        
        for (size_t q = 0; q < _vps_num_poly_terms; q++)
        {
            A[q] = new double[_vps_num_poly_terms];
            LD[q] = new double[_vps_num_poly_terms];
        }
        
        for (size_t q = 0; q < _vps_num_poly_terms; q++)
        {
            b[q] = 0.0;
            for (size_t i = 0; i < _vps_num_poly_terms; i++)
            {
                A[q][i] = 0.0;
                LD[q][i] = 0.0;
            }
        }
        
        for (size_t j = 1; j <= _vps_ext_neighbors[ipoint][0]; j++) // loop over neighbors
        {
            size_t neighbor = _vps_ext_neighbors[ipoint][j];
            // shift origin
            for (size_t idim = 0; idim < _n_dim; idim++) x_neighbor[idim] = (_vps_x[neighbor][idim] - _vps_x[ipoint][idim]) / _vps_dfar[ipoint];
            
            for (size_t q = 0; q < _vps_num_poly_terms; q++)
            {
                double yq = vec_pow_vec(_n_dim, x_neighbor, _vps_t[q]);
                b[q]+= (_vps_f[neighbor][function_index] - _vps_f[ipoint][function_index]) * yq;
                for (size_t i = 0; i < _vps_num_poly_terms; i++)
                {
                    double yi = vec_pow_vec(_n_dim, x_neighbor, _vps_t[i]);
                    A[q][i] += yi * yq;
                }
            }
        }
        
        if (Cholesky(_vps_num_poly_terms, A, LD))
            Cholesky_solver(_vps_num_poly_terms, LD, b, _vps_c[ipoint][function_index]);
        else
        {
            GMRES(_vps_num_poly_terms, A, b, _vps_c[ipoint][function_index], 1E-6);
            _num_GMRES++;
        }
        
        for (size_t q = 0; q < _vps_num_poly_terms; q++) 
        {
            delete[] A[q];
            delete[] LD[q];
        }
        
        delete[] A;
        delete[] LD;
        delete[] b;
        delete[] x_neighbor;
    }
    
    double NonDPOFDarts::VPS_evaluate_VPS_surrogate(size_t function_index, double* x)
    {
        size_t iclosest = _vps_num_points;
        double dmin = DBL_MAX;
        for (size_t ipoint = 0; ipoint < _vps_num_points; ipoint++)
        {
            double dd = 0.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double dx = x[idim] - _vps_x[ipoint][idim];
                dd += dx * dx;
            }
            if (dd < dmin)
            {
                dmin = dd;
                iclosest = ipoint;
            }
        }
        
        // shift origin and scale
        for (size_t idim = 0; idim < _n_dim; idim++) x[idim] = (x[idim] - _vps_x[iclosest][idim]) / _vps_dfar[iclosest];
        
        double f_VPS = _vps_f[iclosest][function_index];
        for (size_t i = 0; i < _vps_num_poly_terms; i++)
        {
            double ci = _vps_c[iclosest][function_index][i];
            double yi = vec_pow_vec(_n_dim, x, _vps_t[i]);
            f_VPS += ci * yi;
        }
        
        // shift origin and scale back to origin
        for (size_t idim = 0; idim < _n_dim; idim++) x[idim] = (x[idim] * _vps_dfar[iclosest]) + _vps_x[iclosest][idim];
        
        return f_VPS;
    }
    
    void NonDPOFDarts::estimate_pof_VPS()
    {
        clock_t start_time, end_time; double cpu_time, total_time(0.0);
        
        start_time = clock();
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            size_t num_levels = requestedRespLevels[resp_fn_count].length();
            for (size_t level_count = 0; level_count < num_levels; level_count++)
            {
                computedProbLevels[resp_fn_count][level_count] = 0.0;
            }
        }
        
        // error report
        //double pof_exact = 0.0;
    
        double i_dart = 0.0;
        double num_darts = 1.0E7;
        double* dart = new double[_n_dim];
        double err = 0.0;
        while (i_dart < num_darts)
        {
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                dart[idim] = _xmin[idim] + generate_a_random_number() * (_xmax[idim] - _xmin[idim]);
            }
            for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
            {
                double f_VPS = VPS_evaluate_VPS_surrogate(resp_fn_count, dart);
                
                //double f_exact = f_true(dart);
                
                size_t num_levels = requestedRespLevels[resp_fn_count].length();
                for (size_t level_count = 0; level_count < num_levels; level_count++)
                {
                    _failure_threshold = requestedRespLevels[resp_fn_count][level_count];
                    
                    if (f_VPS < _failure_threshold)
                    {
                        computedProbLevels[resp_fn_count][level_count] += 1.0;
                    }
                    
                    // error report
                    //if (f_exact < _failure_threshold)
                    //{
                    //    pof_exact += 1.0;
                    //}
                }
            }
            i_dart++;
        }
    
        double sf = 1.0 / num_darts;
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            size_t num_levels = requestedRespLevels[resp_fn_count].length();
            for (size_t level_count = 0; level_count < num_levels; level_count++)
            {
                computedProbLevels[resp_fn_count][level_count] *= sf;
            }
        }
        
        //pof_exact *= sf;
        
        
        end_time = clock();
        cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
        
        std::cout <<  "pof::    VPS Surrogate evaluated in " << std::fixed << cpu_time << " seconds." << std::endl;
        
        // error report
        //std::cout.precision(15);
        //std::cout <<  "pof::    VPS error " << std::fixed << fabs(computedProbLevels[0][0] - pof_exact) << std::endl;
        
        delete[] dart;
    }

    void NonDPOFDarts::VPS_destroy_global_containers()
    {        
        for (size_t ipoint = 0; ipoint < _vps_num_points; ipoint++)
        {
            delete[] _vps_x[ipoint];
            delete[] _vps_f[ipoint];
            delete[] _vps_neighbors[ipoint];
            delete[] _vps_ext_neighbors[ipoint];
            for (size_t ifunc = 0; ifunc < _vps_num_functions; ifunc++)
            {
                delete[] _vps_c[ipoint][ifunc];
            }
            delete[] _vps_c[ipoint];
        }
        delete[] _vps_x;
        delete[] _vps_f;
        delete[] _vps_dfar;
        delete[] _vps_c;
        delete[] _vps_neighbors;
        delete[] _vps_ext_neighbors;
        
        delete[] _xmin;
        delete[] _xmax;
        
        for (size_t i = 0; i < _vps_num_poly_terms; i++)
        {
            delete[] _vps_t[i];
        }
        delete[] _vps_t;
        
    }
    
    void NonDPOFDarts::retrieve_permutations(size_t &m, size_t** &perm, size_t num_dim, size_t upper_bound, bool include_origin, bool force_sum_constraint, size_t sum_constraint)
    {        
        size_t* t = new size_t[num_dim];
        for (size_t idim = 0; idim < num_dim; idim++) t[idim] = 0;
        
        // count output
        m = 0;
        bool origin(true);
        size_t k_dim(num_dim - 1);
        while (true)
        {
            while (t[k_dim] <= upper_bound)
            {
                bool valid(true);
                
                if (!include_origin && origin)
                {
                    valid = false; origin = false;
                }
                if (force_sum_constraint)
                {
                    size_t s_const(0);
                    for (size_t idim = 0; idim < num_dim; idim++) s_const += t[idim];
                    
                    if (s_const > upper_bound)
                    {
                        valid = false; origin = false;
                    }
                }
                if (valid) m++;
                
                t[k_dim]++; // move to the next enumeration
            }
            
            
            size_t kk_dim(k_dim - 1);
            
            bool done(false);
            while (true)
            {
                t[kk_dim]++;
                if (t[kk_dim] > upper_bound)
                {
                    t[kk_dim] = 0;
                    if (kk_dim == 0)
                    {
                        done = true;
                        break;
                    }
                    kk_dim--;
                }
                else break;
            }
            if (done) break;
            t[k_dim] = 0;
        }
        
        // std::cout<< "Numer of permutations = " << m << std::endl;
        
        perm = new size_t*[m];
        for (size_t i = 0; i < m; i++)
        {
            perm[i] = new size_t[num_dim];
            for (size_t idim = 0; idim < num_dim; idim++) perm[i][idim] = 0;
        }
        
        // Taxi Counter to evaluate the surrogate
        origin = true;
        for (size_t idim = 0; idim < num_dim; idim++) t[idim] = 0;
        k_dim = num_dim - 1;
        m = 0; // index of alpha
        
        while (true)
        {
            while (t[k_dim] <= upper_bound)
            {
                bool valid(true);
                
                if (!include_origin && origin)
                {
                    valid = false; origin = false;
                }
                if (force_sum_constraint)
                {
                    size_t s_const(0);
                    for (size_t idim = 0; idim < num_dim; idim++) s_const += t[idim];
                    
                    if (s_const > upper_bound)
                    {
                        valid = false; origin = false;				
                    }
                }
                if (valid) 
                {
                    // store t in t_alpha of counter
                    for (size_t idim = 0; idim < num_dim; idim++) perm[m][idim] = t[idim];
                    
                    //for (size_t idim = 0; idim < num_dim; idim++) std::cout << perm[m][idim] << " ";
                    //std::cout << std::endl;
                    
                    m++;
                }
                
                t[k_dim]++; // move to the next enumeration
            }
            
            
            size_t kk_dim(k_dim - 1);
            
            bool done(false);
            while (true)
            {
                t[kk_dim]++;
                if (t[kk_dim] > upper_bound)
                {
                    t[kk_dim] = 0;
                    if (kk_dim == 0)
                    {
                        done = true;
                        break;
                    }
                    kk_dim--;
                }
                else break;
            }
            if (done) break;
            t[k_dim] = 0;
        }
        
        delete[] t;
    }

    double NonDPOFDarts::vec_pow_vec(size_t num_dim, double* vec_a, size_t* vec_b)
    {
        double ans = 1.0;
        for (size_t idim = 0; idim < num_dim; idim++)
        {
            ans *= pow(vec_a[idim], (int) vec_b[idim]);
        }
        return ans;
    }

    bool NonDPOFDarts::Cholesky(int n, double** A, double** LD)
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j <= i; j++)
            {
                double sum = 0.0;
                for (int k = 0; k < j; k++) sum += LD[i][k] * LD[j][k];
                
                if (i == j)
                {
                    if (A[i][i] < sum + 1E-8)
                    {
                        return false;
                    }
                    LD[i][j] = sqrt(A[i][i] - sum);
                }
                else        LD[i][j] = 1.0 / LD[j][j] * (A[i][j] - sum);
                
                LD[j][i] = LD[i][j];
            }		
        }
        return true;
    }
    
    void NonDPOFDarts::Cholesky_solver(int n, double** LD, double* b, double* x)
    {
        double* y = new double[n];
        for(int i = 0; i < n; i++)
        {
            double sum = 0.;
            for(int k = 0; k < i; k++) sum+= LD[i][k] * y[k];
            y[i] = (b[i] - sum) / LD[i][i];
        }
        for(int i = n - 1; i >= 0; i--)
        {
            double sum = 0.;
            for(int k = i + 1; k < n; ++k) sum += LD[i][k] * x[k];
            x[i] = (y[i] - sum) / LD[i][i];
        }
        delete[] y;
    }
    
    void NonDPOFDarts::GMRES(size_t n, double** A, double* b, double* x, double eps)
    {        
        size_t im = 0;
        
        for (size_t i = 0; i < n; i++) x[i] = 0.0;
        
        double* r = new double[n];
        
        double* rr = new double[n + 1];
        double* bh = new double[n + 1];
        double* c = new double[n + 1];
        double* s = new double[n + 1];
        
        double* w = new double[n];
        
        double** v = new double*[n + 1];
        for (size_t i = 0; i < n + 1; i++) v[i] = new double[n];
        double** HT = new double*[n];
        double** RT = new double*[n];
        double** QT = new double*[n];
        for (im = 0; im < n; im++)
        {
            HT[im] = new double[im + 2];
            RT[im] = new double[im + 1];
            QT[im] = new double[im + 2];
        }
        
        double ro = 0.0;
        double rNorm = 0.0;
        
        // Calculating residual:
        double beta = 0.0;
        for (size_t i = 0; i < n; i++)
        {
            r[i] = b[i];
            beta += r[i] * r[i];
        }
        beta = sqrt(beta);
        
        rNorm = beta;
        ro = beta;
        
        for (size_t i = 0; i < n + 1; i++)
        {
            rr[i] = 0.0;
            bh[i] = 0.0;
            c[i] = 0.0;
            s[i] = 0.0;
        }
        rr[0] = 1.0;
        bh[0] = beta;
        
        // Calculating vo:
        v[0] = new double[n];
        for (size_t i = 0; i < n; i++)
        {
            v[0][i] = r[i] / beta;
        }
        
        for (im = 0; im < n; im++)
        {
            // Calculating w = A v_i :
            
            double wNorm = 0.0;
            for (size_t i = 0; i < n; i++)
            {
                w[i] = 0.0;
                for (size_t j = 0; j < n; j++)
                {
                    w[i] += A[i][j] * v[im][j];
                }
            }
            
            // Updating HT:
            for (size_t ik = 0; ik <= im; ik++)
            {
                HT[im][ik] = 0.0;
                for (size_t j = 0; j < n; j++)
                {
                    HT[im][ik] += v[ik][j] * w[j];
                }
                for (size_t j = 0; j < n; j++)
                {
                    w[j] -= HT[im][ik] * v[ik][j];
                }
            }
            
            wNorm = 0.0;
            for (size_t i = 0; i < n; i++)
            {
                wNorm += w[i] * w[i];
            }
            wNorm = sqrt(wNorm);
            HT[im][im + 1] = wNorm;
            
            for (size_t i = 0; i < n; i++)
            {
                v[im + 1][i] = w[i] / wNorm;
            }
            
            // Updating RT:
            RT[im][0] = HT[im][0];
            for (size_t ik = 1; ik <= im; ik++)
            {
                double gamma = c[ik - 1] * RT[im][ik - 1] + s[ik - 1] * HT[im][ik];
                RT[im][ik] = -s[ik - 1] * RT[im][ik - 1] + c[ik - 1] * HT[im][ik];
                RT[im][ik - 1] = gamma;
            }
            
            double delta = sqrt(pow(RT[im][im], 2) + pow(HT[im][im + 1], 2));
            c[im] = RT[im][im] / delta;
            s[im] = HT[im][im + 1] / delta;
            RT[im][im] = c[im] * RT[im][im] + s[im] * HT[im][im + 1];
            
            
            // Updating QT:
            QT[im][im + 1] = HT[im][im + 1] / RT[im][im];
            size_t ik = im;
            while (true)
            {
                double sum = 0.0;
                for (size_t j = 0; j < im; j++)
                {
                    if (ik >= j + 2)
                        continue;
                    
                    sum += QT[j][ik] * RT[im][j];
                }
                QT[im][ik] = (HT[im][ik] - sum) / RT[im][im];
                if (ik == 0) break;
                ik--;
            }
            
            // Calculating new Norm:
            rNorm = 0.0;
            for (size_t i = 0; i < im + 2; i++)
            {
                rr[i] -= QT[im][0] * QT[im][i];
                rNorm += rr[i] * rr[i];
            }
            rNorm = sqrt(rNorm);
            
            if (rNorm < eps * ro && rNorm < eps)
                break;
        }
        
        // Calculating y and x :
        if (im == n) im--;
        
        double* yy = new double[im + 1];
        for (size_t j = 0; j < im + 1; j++) yy[j] = beta * QT[j][0];
        
        
        size_t nn = im + 1;
        double* y = new double[nn];                
        y[nn - 1] = yy[nn - 1] / RT[nn - 1][nn - 1];
        
        size_t i = nn - 2;
        while (true)       
        {
            double ss = yy[i];
            for (size_t j = i + 1; j < nn; j++)
            {
                ss -= RT[j][i] * y[j];
            }
            y[i] = ss / RT[i][i];                
            if (i == 0) break;
            i--;
        }
        for (size_t i = 0; i < n; i++)
        {
            for (size_t j = 0; j < nn; j++)
            {
                x[i] += v[j][i] * y[j];
            }
        } 
        
        delete[] yy;
        delete[] y;
        
        delete[] r;	
        delete[] rr;
        delete[] bh;
        delete[] c;
        delete[] s;
        
        delete[] w;
        
        for (size_t im = 0; im < n + 1; im++) delete[] v[im];
        delete[] v;
        
        for (size_t im = 0; im < n; im++)
        {
            delete[] HT[im];
            delete[] RT[im];
            delete[] QT[im];
        }
        delete[] HT;
        delete[] RT;
        delete[] QT;
    }
    
    
    
    ////////////////////////////////////////////////////////////////
    // OUTPUT METHODS
    ////////////////////////////////////////////////////////////////
    
    double NonDPOFDarts::f_true(double* x)
    {
        // return text book
        /*
        if (_active_response_function == 0)
        {
            return pow(x[0] - 1, 4) + pow(x[1] - 1, 4);
        }
        else if (_active_response_function == 1)
        {
            return pow(x[0], 2) - 0.5 * x[1];
        }
        */
        
        double E = 2.7182818284590452353602874713526;
        double fval = 1.0;
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            double xm = x[idim] - 1.0;
            double xp = x[idim] + 1.0;
            double wherb = pow(E, - xm * xm) + pow(E, -0.8 * xp * xp) ;// - 0.05 * sin(8 * (x[idim] + 0.1));
            fval *= wherb;
        }
        fval = -fval;
        return fval;
        return 0.0;
    }
    
    void NonDPOFDarts::plot_vertices_2d(bool plot_true_function, bool plot_suurogate)
    {
        std::stringstream ss;
        ss << "mps_spheres_" << _active_response_function << ".ps";
        std::fstream file(ss.str().c_str(), std::ios::out);
        file << "%!PS-Adobe-3.0" << std::endl;
        file << "72 72 scale     % one unit = one inch" << std::endl;
        
        double xmin(_xmin[0]);
        double ymin(_xmin[1]);
        double Lx(_xmax[0] - _xmin[0]);
        double Ly(_xmax[1] - _xmin[0]);
        
        double scale_x, scale_y, scale;
        double shift_x, shift_y;
        
        scale_x = 6.5 / Lx;
        scale_y = 9.0 / Ly;
        
        if (scale_x < scale_y)
        {
            scale = scale_x;
            shift_x = 1.0 - xmin * scale;
            shift_y = 0.5 * (11.0 - Ly * scale) - ymin * scale;
        }
        else
        {
            scale = scale_y;
            shift_x = 0.5 * (8.5 - Lx * scale) - xmin * scale;
            shift_y = 1.0 - ymin * scale;
        }
        file << shift_x << " " << shift_y << " translate" << std::endl;
        
        
        file << "/redseg      % stack: x1 y1 x2 y2" << std::endl;
        file << "{newpath" << std::endl;
        file << " moveto" << std::endl;
        file << " lineto" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " grestore" << std::endl;
        file << " 1 0 0 setrgbcolor" << std::endl;
        file << " 0.01 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        file << "/greenseg      % stack: x1 y1 x2 y2" << std::endl;
        file << "{newpath" << std::endl;
        file << " moveto" << std::endl;
        file << " lineto" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " grestore" << std::endl;
        file << " 0 1 0 setrgbcolor" << std::endl;
        file << " 0.01 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        
        file << "/blueseg      % stack: x1 y1 x2 y2" << std::endl;
        file << "{newpath" << std::endl;
        file << " moveto" << std::endl;
        file << " lineto" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " grestore" << std::endl;
        file << " 0 0 1 setrgbcolor" << std::endl;
        file << " 0.02 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        file << "/blackquad      % stack: x1 y1 x2 y2 x3 y3 x4 y4" << std::endl;
        file << "{newpath" << std::endl;
        file << " moveto" << std::endl;
        file << " lineto" << std::endl;
        file << " lineto" << std::endl;
        file << " lineto" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " grestore" << std::endl;
        file << " 0 0 0 setrgbcolor" << std::endl;
        file << " 0.02 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        file << "/circ    % stack: x y r" << std::endl;
        file << "{0 360 arc" << std::endl;
        file << " closepath" << std::endl;
        file << " 0.002 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        file << "/blackfcirc    % stack: x y r" << std::endl;
        file << "{0 360 arc" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " 0 0 0 setrgbcolor" << std::endl;
        file << " fill" << std::endl;
        file << " grestore" << std::endl;
        file << " 0 0 0 setrgbcolor" << std::endl;
        file << " 0.0 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        file << "/redfcirc    % stack: x y r" << std::endl;
        file << "{0 360 arc" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " 1 0 0 setrgbcolor" << std::endl;
        file << " fill" << std::endl;
        file << " grestore" << std::endl;
        file << " 0 0 0 setrgbcolor" << std::endl;
        file << " 0.0 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        file << "/bluefcirc    % stack: x y r" << std::endl;
        file << "{0 360 arc" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " 0 0 1 setrgbcolor" << std::endl;
        file << " fill" << std::endl;
        file << " grestore" << std::endl;
        file << " 0 0 0 setrgbcolor" << std::endl;
        file << " 0.0 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        file << "/greenfcirc    % stack: x y r" << std::endl;
        file << "{0 360 arc" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " 0 1 0 setrgbcolor" << std::endl;
        file << " fill" << std::endl;
        file << " grestore" << std::endl;
        file << " 0 0 0 setrgbcolor" << std::endl;
        file << " 0.0 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        
        file << "/quad_white      % stack: x1 y1 x2 y2 x3 y3 x4 y4" << std::endl;
        file << "{newpath" << std::endl;
        file << " moveto" << std::endl;
        file << " lineto" << std::endl;
        file << " lineto" << std::endl;
        file << " lineto" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " 1.0 setgray fill" << std::endl;
        file << " grestore" << std::endl;
        file << "} def" << std::endl;
        
        file << "/quad_bold      % stack: x1 y1 x2 y2 x3 y3 x4 y4" << std::endl;
        file << "{newpath" << std::endl;
        file << " moveto" << std::endl;
        file << " lineto" << std::endl;
        file << " lineto" << std::endl;
        file << " lineto" << std::endl;
        file << " closepath" << std::endl;
        file << " 0.01 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        
        // plot filled circles
        for (size_t index = 0; index < _num_inserted_points; index++)
        {
            double r = sqrt(fabs(_sample_points[index][2]));
            
            file << _sample_points[index][0] * scale << "  " << _sample_points[index][1] * scale << "  " << r * scale << "  ";
                
            if (_sample_points[index][2] > 0) file << "greenfcirc"     << std::endl; // non-failure disk
            else                              file << "redfcirc"     << std::endl; // non-failure disk
           
        }
        
        // plot discs boundaries
        for (size_t index = 0; index < _num_inserted_points; index++)
        {
            double r = sqrt(fabs(_sample_points[index][2]));
            file << _sample_points[index][0] * scale << "  " << _sample_points[index][1] * scale << "  " << r * scale << "  ";
            file << "circ"     << std::endl;
        }
        
        // plot line flat segments
        if (false)
        {
            for (size_t i = 0; i < _num_flat_segments; i++)
            {
                // plot vertex
                if (_flat_dim == 0)
                {
                    // draw a horizontal line passing through _dart
                    file << _line_flat_start[i] * scale << "  " << _dart[1] * scale << "  ";
                    file << _line_flat_end[i] * scale << "  " << _dart[1] * scale << "  ";
                    file << "blueseg"     << std::endl;
                }
                else
                {
                    // draw a vertical line passing through _dart
                    file << _dart[0] * scale << " " << _line_flat_start[i] * scale << "  ";
                    file << _dart[0] * scale << " " << _line_flat_end[i] * scale << "  ";
                    file << "blueseg"     << std::endl;
                }
            }
        }
        
        double s(0.01);
        for (size_t index = 0; index < _num_inserted_points; index++)
        {
            double r = sqrt(fabs(_sample_points[index][2]));
            s = r * 0.05;
            
            // plot vertex
            file << _sample_points[index][0] * scale << "  " << _sample_points[index][1] * scale << "  " << s * scale << " ";
            file << "blackfcirc"     << std::endl; // non-failure disk
        }
        
        // plot true failure isocontour
        if (plot_true_function)
        {
            double A((_xmax[0] - _xmin[0])*(_xmax[1] - _xmin[1]));
            
            double failure(0.0);
            double non_failure(0.0);
            double failure_spheres(0.0);
            double non_failure_spheres(0.0);
            double* xx = new double[2];
            double sx = 0.001 * (_xmax[0] - _xmin[0]);
            double sy = 0.001 * (_xmax[1] - _xmin[1]);
            for (size_t i = 0; i < 1000; i++)
            {
                double xo = _xmin[0] + i * sx;
                for (size_t j = 0; j < 1000; j++)
                {
                    double yo = _xmin[1] + j * sy;
                    
                    xx[0] = xo; xx[1] = yo;
                    double fo = f_true(xx);
                    xx[0] = xo+sx; xx[1] = yo;
                    double f1 = f_true(xx);
                    xx[0] = xo + sx; xx[1] = yo + sy;
                    double f2 = f_true(xx);
                    xx[0] = xo; xx[1] = yo + sy;
                    double f3 = f_true(xx);
                    
                    size_t num_points(0);
                    double x1, y1, x2, y2;
                    if ((fo > _failure_threshold && f1 < _failure_threshold) || (fo < _failure_threshold && f1 > _failure_threshold))
                    {
                        double h = sx * (_failure_threshold - fo) / (f1 - fo);
                        x1 = xo + h;
                        y1 = yo; num_points++;
                    }
                    if ((f1 > _failure_threshold && f2 < _failure_threshold) || (f1 < _failure_threshold && f2 > _failure_threshold))
                    {
                        double h = sy * (_failure_threshold - f1) / (f2 - f1);
                        if (num_points == 0)
                        {
                            x1 = xo + sx;
                            y1 = yo + h;
                        }
                        else
                        {
                            x2 = xo + sx;
                            y2 = yo + h;
                        }
                        num_points++;
                    }
                    if ((f2 > _failure_threshold && f3 < _failure_threshold) || (f2 < _failure_threshold && f3 > _failure_threshold))
                    {
                        double h = sx * (_failure_threshold - f2) / (f3 - f2);
                        if (num_points == 0)
                        {
                            x1 = xo + sx - h;
                            y1 = yo + sy;
                        }
                        else
                        {
                            x2 = xo + sx - h;
                            y2 = yo + sy;
                        }
                        num_points++;
                    }
                    if ((f3 > _failure_threshold && fo < _failure_threshold) || (f3 < _failure_threshold && fo > _failure_threshold))
                    {
                        double h = sy * (_failure_threshold - f3) / (fo - f3);
                        if (num_points == 0)
                        {
                            x1 = xo;
                            y1 = yo + sy - h;
                        }
                        else
                        {
                            x2 = xo;
                            y2 = yo + sy - h;
                        }
                        num_points++;
                    }
                    
                    if (num_points == 2)
                    {
                        file << x1 * scale << " " << y1 * scale << "  ";
                        file << x2 * scale << " " << y2 * scale << "  ";
                        file << "blueseg"     << std::endl;
                        
                        size_t num_failure_points(0);
                        if (fo < _failure_threshold) num_failure_points++;
                        if (f1 < _failure_threshold) num_failure_points++;
                        if (f2 < _failure_threshold) num_failure_points++;
                        if (f3 < _failure_threshold) num_failure_points++;
                        
                        if (num_failure_points == 1)
                        {
                            double a;
                            if (fo < _failure_threshold) a = fabs(area_triangle(xo, yo, x1, y1, x2, y2));
                            if (f1 < _failure_threshold) a = fabs(area_triangle(xo + sx, yo, x1, y1, x2, y2));
                            if (f2 < _failure_threshold) a = fabs(area_triangle(xo + sx, yo + sy, x1, y1, x2, y2));
                            if (f3 < _failure_threshold) a = fabs(area_triangle(xo, yo + sy, x1, y1, x2, y2));
                            failure += a;
                            non_failure += (sx * sy - a);
                        }
                        else if (num_failure_points == 2)
                        {
                            double a1, a2;
                            if (fo < _failure_threshold && f1 < _failure_threshold)
                            {
                                a1 = area_triangle(xo, yo, xo + sx, yo, x1, y1);
                                a2 = area_triangle(xo, yo, x1, y1, x2, y2);
                            }
                            if (f1 < _failure_threshold && f2 < _failure_threshold)
                            {
                                a1 = area_triangle(xo + sx, yo, xo + sx, yo + sy, x1, y1);
                                a2 = area_triangle(xo + sx, yo + sy, x2, y2, x1, y1);
                            }
                            if (f2 < _failure_threshold && f3 < _failure_threshold)
                            {
                                a1 = area_triangle(xo + sx, yo + sy, xo, yo + sy, x1, y1);
                                a2 = area_triangle(xo, yo + sy, x2, y2, x1, y1);
                            }
                            if (f3 < _failure_threshold && fo < _failure_threshold)
                            {
                                a1 = area_triangle(xo, yo, x1, y1, xo, yo + sy);
                                a2 = area_triangle(xo, yo + sy, x1, y1, x2, y2);
                            }
                            failure += a1 + a2;
                            non_failure += (sx * sy - a1 - a2);
                        }
                        else if (num_failure_points == 3)
                        {
                            double a;
                            if (fo > _failure_threshold) a = fabs(area_triangle(xo, yo, x1, y1, x2, y2));
                            if (f1 > _failure_threshold) a = fabs(area_triangle(xo + sx, yo, x1, y1, x2, y2));
                            if (f2 > _failure_threshold) a = fabs(area_triangle(xo + sx, yo + sy, x1, y1, x2, y2));
                            if (f3 > _failure_threshold) a = fabs(area_triangle(xo, yo + sy, x1, y1, x2, y2));
                            failure += (sx * sy - a);
                            non_failure += a;
                        }
                    }
                    else
                    {
                        if (fo < _failure_threshold) failure += sx * sy;
                        else non_failure += sx * sy;
                    }
                    
                    for (size_t index = 0; index < _num_inserted_points; index++)
                    {
                        double xs = _sample_points[index][0];
                        double ys = _sample_points[index][1];
                        double rrs = fabs(_sample_points[index][2]);
                        
                        double xf(xo), yf(yo);
                        if (fabs(xo + sx - xs) > fabs(xo - xs)) xf += sx;
                        if (fabs(yo + sy - ys) > fabs(yo - ys)) yf += sy;
                        
                        double dx(xs - xf), dy(ys-yf);
                        if (dx * dx + dy * dy < rrs)
                        {
                            // cell is entirely in sphere
                            if (_sample_points[index][2] < 0.0) failure_spheres += sx * sy;
                            else non_failure_spheres += sx * sy;
                            break; // move to the next cell
                        }
                    }
                    
                }
            }
            Cout<< "pof::       POF (isocontouring) = " << std::setprecision(10) << failure / A << '\n';
            double lower = failure_spheres / A;
            double upper = 1 - (non_failure_spheres / A);
            
            if (upper < lower) upper = lower;
            delete[] xx;
        }
        
        
        
        // plot surrogate failure isocontour
        if (plot_suurogate)
        {
            std::cout << "plotting _active_response_function = " << _active_response_function << std::endl;
            
            double A((_xmax[0] - _xmin[0])*(_xmax[1] - _xmin[1]));
            
            double failure(0.0);
            double non_failure(0.0);
            double failure_spheres(0.0);
            double non_failure_spheres(0.0);
            double* xx = new double[2];
            double sx = 0.001 * (_xmax[0] - _xmin[0]);
            double sy = 0.001 * (_xmax[1] - _xmin[1]);
            for (size_t i = 0; i < 1000; i++)
            {
                double xo = _xmin[0] + i * sx;
                for (size_t j = 0; j < 1000; j++)
                {
                    double yo = _xmin[1] + j * sy;
                    
                    xx[0] = xo; xx[1] = yo;
                    double fo = eval_surrogate(_active_response_function, xx);
                    xx[0] = xo+sx; xx[1] = yo;
                    double f1 = eval_surrogate(_active_response_function, xx);
                    xx[0] = xo + sx; xx[1] = yo + sy;
                    double f2 = eval_surrogate(_active_response_function, xx);
                    xx[0] = xo; xx[1] = yo + sy;
                    double f3 = eval_surrogate(_active_response_function, xx);
                    
                    size_t num_points(0);
                    double x1, y1, x2, y2;
                    if ((fo > _failure_threshold && f1 < _failure_threshold) || (fo < _failure_threshold && f1 > _failure_threshold))
                    {
                        double h = sx * (_failure_threshold - fo) / (f1 - fo);
                        x1 = xo + h;
                        y1 = yo; num_points++;
                    }
                    if ((f1 > _failure_threshold && f2 < _failure_threshold) || (f1 < _failure_threshold && f2 > _failure_threshold))
                    {
                        double h = sy * (_failure_threshold - f1) / (f2 - f1);
                        if (num_points == 0)
                        {
                            x1 = xo + sx;
                            y1 = yo + h;
                        }
                        else
                        {
                            x2 = xo + sx;
                            y2 = yo + h;
                        }
                        num_points++;
                    }
                    if ((f2 > _failure_threshold && f3 < _failure_threshold) || (f2 < _failure_threshold && f3 > _failure_threshold))
                    {
                        double h = sx * (_failure_threshold - f2) / (f3 - f2);
                        if (num_points == 0)
                        {
                            x1 = xo + sx - h;
                            y1 = yo + sy;
                        }
                        else
                        {
                            x2 = xo + sx - h;
                            y2 = yo + sy;
                        }
                        num_points++;
                    }
                    if ((f3 > _failure_threshold && fo < _failure_threshold) || (f3 < _failure_threshold && fo > _failure_threshold))
                    {
                        double h = sy * (_failure_threshold - f3) / (fo - f3);
                        if (num_points == 0)
                        {
                            x1 = xo;
                            y1 = yo + sy - h;
                        }
                        else
                        {
                            x2 = xo;
                            y2 = yo + sy - h;
                        }
                        num_points++;
                    }
                    
                    if (num_points == 2)
                    {
                        file << x1 * scale << " " << y1 * scale << "  ";
                        file << x2 * scale << " " << y2 * scale << "  ";
                        file << "redseg"     << std::endl;
                        
                        size_t num_failure_points(0);
                        if (fo < _failure_threshold) num_failure_points++;
                        if (f1 < _failure_threshold) num_failure_points++;
                        if (f2 < _failure_threshold) num_failure_points++;
                        if (f3 < _failure_threshold) num_failure_points++;
                        
                        if (num_failure_points == 1)
                        {
                            double a;
                            if (fo < _failure_threshold) a = fabs(area_triangle(xo, yo, x1, y1, x2, y2));
                            if (f1 < _failure_threshold) a = fabs(area_triangle(xo + sx, yo, x1, y1, x2, y2));
                            if (f2 < _failure_threshold) a = fabs(area_triangle(xo + sx, yo + sy, x1, y1, x2, y2));
                            if (f3 < _failure_threshold) a = fabs(area_triangle(xo, yo + sy, x1, y1, x2, y2));
                            failure += a;
                            non_failure += (sx * sy - a);
                        }
                        else if (num_failure_points == 2)
                        {
                            double a1, a2;
                            if (fo < _failure_threshold && f1 < _failure_threshold)
                            {
                                a1 = area_triangle(xo, yo, xo + sx, yo, x1, y1);
                                a2 = area_triangle(xo, yo, x1, y1, x2, y2);
                            }
                            if (f1 < _failure_threshold && f2 < _failure_threshold)
                            {
                                a1 = area_triangle(xo + sx, yo, xo + sx, yo + sy, x1, y1);
                                a2 = area_triangle(xo + sx, yo + sy, x2, y2, x1, y1);
                            }
                            if (f2 < _failure_threshold && f3 < _failure_threshold)
                            {
                                a1 = area_triangle(xo + sx, yo + sy, xo, yo + sy, x1, y1);
                                a2 = area_triangle(xo, yo + sy, x2, y2, x1, y1);
                            }
                            if (f3 < _failure_threshold && fo < _failure_threshold)
                            {
                                a1 = area_triangle(xo, yo, x1, y1, xo, yo + sy);
                                a2 = area_triangle(xo, yo + sy, x1, y1, x2, y2);
                            }
                            failure += a1 + a2;
                            non_failure += (sx * sy - a1 - a2);
                        }
                        else if (num_failure_points == 3)
                        {
                            double a;
                            if (fo > _failure_threshold) a = fabs(area_triangle(xo, yo, x1, y1, x2, y2));
                            if (f1 > _failure_threshold) a = fabs(area_triangle(xo + sx, yo, x1, y1, x2, y2));
                            if (f2 > _failure_threshold) a = fabs(area_triangle(xo + sx, yo + sy, x1, y1, x2, y2));
                            if (f3 > _failure_threshold) a = fabs(area_triangle(xo, yo + sy, x1, y1, x2, y2));
                            failure += (sx * sy - a);
                            non_failure += a;
                        }
                    }
                    else
                    {
                        if (fo < _failure_threshold) failure += sx * sy;
                        else non_failure += sx * sy;
                    }
                    
                    for (size_t index = 0; index < _num_inserted_points; index++)
                    {
                        double xs = _sample_points[index][0];
                        double ys = _sample_points[index][1];
                        double rrs = fabs(_sample_points[index][2]);
                        
                        double xf(xo), yf(yo);
                        if (fabs(xo + sx - xs) > fabs(xo - xs)) xf += sx;
                        if (fabs(yo + sy - ys) > fabs(yo - ys)) yf += sy;
                        
                        double dx(xs - xf), dy(ys-yf);
                        if (dx * dx + dy * dy < rrs)
                        {
                            // cell is entirely in sphere
                            if (_sample_points[index][2] < 0.0) failure_spheres += sx * sy;
                            else non_failure_spheres += sx * sy;
                            break; // move to the next cell
                        }
                    }
                    
                }
            }
            Cout<< "pof::       POF (isocontouring) = " << std::setprecision(10) << failure / A << '\n';
            double lower = failure_spheres / A;
            double upper = 1 - (non_failure_spheres / A);
            
            if (upper < lower) upper = lower;
            delete[] xx;
        }
        
        
        
        
        // plot current dart
        //file << (_dart[0] - s) * scale << "  " << (_dart[1] - s) * scale << "  ";
        //file << (_dart[0] + s) * scale << "  " << (_dart[1] + s) * scale << "  ";
        //file << "blueseg"     << std::endl;		
        
        //file << (_dart[0] + s) * scale << "  " << (_dart[1] - s) * scale << "  ";
        //file << (_dart[0] - s) * scale << "  " << (_dart[1] + s) * scale << "  ";
        //file << "blueseg"     << std::endl;   
        
        //file << _dart[0] * scale << "  " << _dart[1] * scale << "  " << 1.4142 * s * scale << "  ";
        //file << "circ"     << std::endl;	
    
        
        double DX = _xmax[0] - _xmin[0];
        double DY = _xmax[1] - _xmin[1];
        
        // plot domain boundaries
        file << (_xmin[0] - DX) * scale << "  "  << _xmin[1]        * scale << "  ";  
        file << (_xmax[0] + DX) * scale << "  "  <<  _xmin[1]       * scale << "  ";          
        file << (_xmax[0] + DX) * scale << "  "  << (_xmin[1] - DY) * scale << "  ";          
        file << (_xmin[0] - DX) * scale << "  "  << (_xmin[1] - DY) * scale << "  ";          
        file << "quad_white"      << std::endl;
        
        file << (_xmin[0] - DX) * scale << "  "  <<  _xmax[1]        * scale << "  ";  
        file << (_xmax[0] + DX) * scale << "  "  <<  _xmax[1]        * scale << "  ";          
        file << (_xmax[0] + DX) * scale << "  "  <<  (_xmax[1] + DY) * scale << "  ";          
        file << (_xmin[0] - DX) * scale << "  "  <<  (_xmax[1] + DY) * scale << "  ";          
        file << "quad_white"      << std::endl;
        
        file <<  _xmax[0]       * scale << "  "  <<  (_xmin[1] - DY) * scale << "  ";  
        file << (_xmax[0] + DX) * scale << "  "  <<  (_xmin[1] - DY) * scale << "  ";          
        file << (_xmax[0] + DX) * scale << "  "  <<  (_xmax[1] + DY) * scale << "  ";          
        file <<  _xmax[0]       * scale << "  "  <<  (_xmax[1] + DY) * scale << "  ";          
        file << "quad_white"      << std::endl;
        
        file << (_xmin[0] - DX) * scale << "  "  <<  (_xmin[1] - DY) * scale << "  ";  
        file <<  _xmin[0]       * scale << "  "  <<  (_xmin[1] - DY) * scale << "  ";          
        file <<  _xmin[0]       * scale << "  "  <<  (_xmax[1] + DY) * scale << "  ";          
        file << (_xmin[0] - DX) * scale << "  "  <<  (_xmax[1] + DY) * scale << "  ";          
        file << "quad_white"      << std::endl;
        
        
        // plot domain boundaries
        file << _xmin[0] * scale << "  " << _xmin[1] * scale << "  ";  
        file << _xmax[0] * scale << "  " << _xmin[1] * scale << "  ";          
        file << _xmax[0] * scale << "  " << _xmax[1] * scale << "  ";          
        file << _xmin[0] * scale << "  " << _xmax[1] * scale << "  ";          
        file << "quad_bold"      << std::endl;
        
        file << "showpage" << std::endl;
        
    }

    void NonDPOFDarts::print_results(std::ostream& s)
    {
      s << "\nStatistics based on the importance sampling calculations:\n";
      print_distribution_mappings(s);
    }

} // namespace Dakota

