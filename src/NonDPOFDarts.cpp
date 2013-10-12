/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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

NonDPOFDarts::NonDPOFDarts(Model& model): NonD(model),
seed(probDescDB.get_int("method.random_seed")),
samples(probDescDB.get_int("method.samples"))
{
  // any initialization is done here.   For now, you should just specify 
  // the number of samples, but eventually we will get that from the input spec
  Cout << "Hello World, POF Darts is coming! " << '\n';
  Cout << "Number of samples " << samples << '\n';
}

NonDPOFDarts::~NonDPOFDarts()
{}


void NonDPOFDarts::quantify_uncertainty()
{
    Cout << "I am now computing uncertainty! " << '\n';
    
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
        _global_optimization = false;
        
        _accepted_void_ratio = 1E-9;
        
        _n_dim = numContinuousVars;
        
        _ieval = 0; _num_sample_eval = 2 * _n_dim + 1;
        
        _dx = 1E-4; // spacing for FD
        
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
        _grad_vec = new double[_n_dim];
        
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
        double diag(0.0);
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            double dx = _xmax[idim] - _xmin[idim];
            diag += dx * dx;
        }
        diag = sqrt(diag);
        _max_radius = /*0.1*/ 1000.0 * diag;
        
        _fval = new double*[numFunctions];
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++) _fval[resp_fn_count] = new double[_total_budget * _num_sample_eval];
        
        
        
        // to be commmented later!!
        Cout << "pof:: Number of dimensions = " << _n_dim << '\n';
        Cout << "pof:: Sample Budget = " << samples << '\n';
        Cout << "pof:: Failure Threshold = " << _failure_threshold << '\n';
        Cout << "pof:: Number of Successive Misses = " << _max_num_successive_misses << '\n';
        Cout << "pof:: Maximum disk radius = " << _max_radius << '\n';
        Cout << "pof:: first lower bound = " << _xmin[0] << '\n';
        Cout << "pof:: second lower bound = " << _xmin[1] << '\n';
        Cout << "pof:: first lower bound = " << _xmax[0] << '\n';
        Cout << "pof:: second upper bound = " << _xmax[1] << '\n';
        // */
    }
    
    void NonDPOFDarts::exit_pof_darts()
    {
        delete[] _dart;
        delete[] _grad_vec;
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
    }
    
    void NonDPOFDarts::execute(size_t kd)
    {
       
        clock_t start_time, end_time; double cpu_time, total_time(0.0);
       
        _num_inserted_points = 0; _num_darts = 0;
        
        for (size_t resp_fn_count = 0; resp_fn_count<numFunctions; resp_fn_count++)
        {
            _active_response_function = resp_fn_count;
            
            size_t num_levels = requestedRespLevels[resp_fn_count].length();
            
            for (size_t level_count=0; level_count<num_levels; level_count++)
            {
                _failure_threshold = requestedRespLevels[resp_fn_count][level_count];
                
                for (size_t isample = 0; isample < _num_inserted_points; isample++) assign_sphere_radius_POF(_sample_points[isample], isample);
               
                for (size_t isample = 0; isample < _num_inserted_points; isample++) resolve_overlap_POF(isample);
                
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
        
        double lower(0.0), upper(0.0);
        double pof_val(0.0);
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            _active_response_function = resp_fn_count;
            
            size_t num_levels = requestedRespLevels[resp_fn_count].length();
            
            for (size_t level_count = 0; level_count < num_levels; level_count++)
            {
                Cout << "resp_fn_count = " << resp_fn_count << '\n';
                Cout << "level_count = " << level_count << '\n';
                
                _failure_threshold = requestedRespLevels[resp_fn_count][level_count];
                
                Cout << "pof::  Estimating pof for threshold = " << _failure_threshold << '\n';
                
                for (size_t isample = 0; isample < _num_inserted_points; isample++) assign_sphere_radius_POF(_sample_points[isample], isample);
                
                for (size_t isample = 0; isample < _num_inserted_points; isample++) resolve_overlap_POF(isample);
                
                /*
                if (_n_dim == 2)
                {
                    Cout<< "pof:: Plotting in mps_shphere.ps ... ";
                    plot_vertices_2d();
                }
                */
                
                retrieve_POF_bounds(lower, upper);
                pof_val = 0.5 * (lower + upper);
                
                // Laura, uncommenting this statement give me a segmentation fault can you please check the reason
                // do we need to resize computedProbLevels?!
                
                computedProbLevels[resp_fn_count][level_count] = pof_val;
            }
        }
        return;
    }
    
    void NonDPOFDarts::print_POF_results(double lower, double upper)
    {
        Cout<< "pof::    POF (lower bound) = " << std::setprecision(10) << lower << '\n';
        Cout<< "pof::    POF (upper bound) = " << std::setprecision(10) << upper << '\n';
        Cout<< "pof::    POF (average) = " << std::setprecision(10) << 0.5 * (lower + upper) << '\n';
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
                    std::cout<< " done!\npof:: Simulation Budget has been exhausted!" << std::endl;
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
                        std::cout<< " done!\npof:: Simulation Budget has been exhausted!" << std::endl;
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
        
        if (_global_optimization) assign_sphere_radius_OPT(x, _num_inserted_points);
        else                      assign_sphere_radius_POF(x, _num_inserted_points);
        
        resolve_overlap_POF(_num_inserted_points);
        
        _num_inserted_points++;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // OPT / POF methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void NonDPOFDarts::shrink_big_spheres()
    {
        double rr_max(0.0);
        for (size_t isample = 0; isample < _num_inserted_points; isample++)
        {
            if (fabs(_sample_points[isample][_n_dim]) > rr_max) rr_max = fabs(_sample_points[isample][_n_dim]);
        }
        
        Cout<< "rr_max = " << rr_max << std::endl;
        
        for (size_t isample = 0; isample < _num_inserted_points; isample++)
        {
            if (fabs(_sample_points[isample][_n_dim]) > 0.81 * rr_max) _sample_points[isample][_n_dim] *= 0.81;
        }
                
    }

    void NonDPOFDarts::assign_sphere_radius_POF(double* x, size_t isample)
    {
        if (isample == _num_inserted_points)
        {
            // A new disk: compute ALL responses
            compute_response(x);
            compute_response_for_FD_gradients(x);
        }
        
        size_t ieval = isample * _num_sample_eval;
        double fval = _fval[_active_response_function][ieval]; ieval++;
     
        double grad(0.0), curv(0.0);
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            double fp = _fval[_active_response_function][ieval];ieval++;
            double fm = _fval[_active_response_function][ieval];ieval++;
            double dgrad = (fp - fm) / (2 * _dx);
            grad += dgrad * dgrad;
            double dcurv =  (fp - 2 * fval + fm) / (_dx * _dx);
            curv+= dcurv * dcurv;
        }
        curv = sqrt(curv);
        grad = sqrt(grad);

        
        // form a quadratic function and solve it to retrieve the radius
        // f(r) = a r^2 + b r + c = failure_threshold
        
        if (fval > _failure_threshold)
        {
            // we are interested in -gradient direction only if we are in the failure region
            grad  = - grad; curv = -curv;
        }
        
        double r = get_dart_radius(fval, grad, curv);
        
        _sample_points[isample][_n_dim] = r * r;
        if (fval < _failure_threshold) _sample_points[isample][_n_dim] = - _sample_points[isample][_n_dim];
    }
    
    
    void NonDPOFDarts::assign_sphere_radius_OPT(double* x, size_t isample)
    {
        /*
        double f = f_true(x);
        double fcurv;
        double fgrad = f_grad_FD(x, f, fcurv); // this could be
        // form a quadratic function and solve it to retrieve the radius
        // f(r) = a r^2 + b r + c = failure_threshold
        
        fgrad = -fgrad; fcurv = -fcurv; // we are interested in the descending direction onlly
        
        double r(0.0);
        if (_num_inserted_points == 0)
        {
            _global_minima = f;
        }
        else if (f < _global_minima)
        {
            // adjust radius of all spheres
            _global_minima = f;
            for (size_t jsample = 0; jsample < _num_inserted_points; jsample++)
            {
                // adjust radius of old sphere
                double fj = _sample_points[jsample][_n_dim + 1];
                double gj = _sample_points[jsample][_n_dim + 2];
                double hj = _sample_points[jsample][_n_dim + 3];
                double rj = get_dart_radius(fj, gj, hj);
                _sample_points[jsample][_n_dim] = rj * rj;
            }
        }
        else
        {
            r = get_dart_radius(f, fgrad, fcurv);
        }
        
        _sample_points[isample][_n_dim] = r * r;
        */
    }
    
    void NonDPOFDarts::resolve_overlap_POF(size_t ksample)
    {
        if (_num_inserted_points < 2) return;
       
        for (size_t isample = 0; isample < _num_inserted_points; isample++)
        {
            if (isample == ksample) continue;
            
            double rr_n = _sample_points[ksample][_n_dim];
            double rr_i = _sample_points[isample][_n_dim];
            
            double bigger_rr(rr_n), smaller_rr(rr_n);
            if (rr_i > bigger_rr) bigger_rr = rr_i;
            if (rr_i < smaller_rr) smaller_rr = rr_i;
            
            double dd(0.0);
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double dx = _sample_points[ksample][idim] - _sample_points[isample][idim];
                dd += dx * dx;
            }
            
            if (rr_i * rr_n < 0.0)
            {
                // disks of different colors check for overlap:
                double rcomb(sqrt(fabs(rr_i)) + sqrt(fabs(rr_n)));
                if (dd < rcomb * rcomb)
                {
                    // Inaccurate Overlap
                    double d = sqrt(dd);
                    double fn = _fval[_active_response_function][ksample * _num_sample_eval];
                    double fi = _fval[_active_response_function][isample * _num_sample_eval];
                    double ri = (_failure_threshold - fi) / (fn - fi) * d;
                    double rn = d - ri;
                    
                    // overlaping disks of different colors, shrink both 
                    if (rn * rn < fabs(rr_n))
                    {
                        _sample_points[ksample][_n_dim] = rn * rn;
                        if (fn < _failure_threshold) _sample_points[ksample][_n_dim] = -_sample_points[ksample][_n_dim];
                    }
                    
                    if (ri * ri < fabs(rr_i))
                    {
                        _sample_points[isample][_n_dim] = ri * ri;
                        if (fi < _failure_threshold) _sample_points[isample][_n_dim] = -_sample_points[isample][_n_dim];
                    }
                }
            }
        }
    }
    
    void NonDPOFDarts::compute_response(double* x)
    {
        RealVector newX(_n_dim);
        for (size_t idim = 0; idim < _n_dim; idim++) newX[idim] = x[idim];
        
        iteratedModel.continuous_variables(newX);
        iteratedModel.compute_response();
        
        for (size_t resp_fn_count=0; resp_fn_count<numFunctions; resp_fn_count++)
        {
            double fval = iteratedModel.current_response().function_value(resp_fn_count);
            _fval[resp_fn_count][_ieval] = fval;
        }
        _ieval++;
    }
    
    void NonDPOFDarts::compute_response_for_FD_gradients(double* x)
    {
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            x[idim] += _dx; // moving forward
            compute_response(x);
            x[idim] -= (2 * _dx); // moving backward
            compute_response(x);
            x[idim] += _dx; // back to original location
        }
    }
    
    double NonDPOFDarts::get_dart_radius(double f, double fgrad, double fcurv)
    {
        double a, b, c;
        
        if (_global_optimization) 
            c = f - _global_minima; // f(0) - global_minima = c
        else
            c = f - _failure_threshold;
        
        b = fgrad; // f'(0) = b
        
        a = 0.5 * fcurv; // at r = 0, f''(0) = f_curv
        
        
        double r;
        if (fabs(a) < 1E-10)
        {
            r = fabs(c / b);
        }
        else
        {
            r =  -b / (2 * a);
            double z = b * b - 4 * a * c;
            if (z > 0.0)
            {
                if (r < 0) r += fabs(sqrt(z) / (2 * a));
                else r -= fabs(sqrt(z) / (2 * a));
            }
        }	
        if (r > _max_radius) r = _max_radius;
        return r;
    }
    
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // VOLUME OF SPHERE UNION METHODS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    
    void NonDPOFDarts::retrieve_POF_bounds(double &lower, double &upper)
    {
        Cout <<  "\n*** Estimating Volume of overlapping spheres ... ";
        clock_t start_time, end_time; double cpu_time, total_time(0.0);
        start_time = clock();
        
        double v_failure = 0.0; double v_nonfailure = 0.0;
        size_t num_failure(0), num_non_failure(0);
        for (size_t isphere = 0; isphere < _num_inserted_points; isphere++)
        {
            if (_sample_points[isphere][_n_dim] > 0.0) num_non_failure++; // non-failure sphere
            else                                       num_failure++;     // failure region
        }
        
        double VOL(1.0);
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            double dx = _xmax[idim] - _xmin[idim];
            VOL *= dx;
        }
        
        if (num_failure > 0)
        {
            double** failure_spheres = new double*[num_failure];
            for (size_t isphere = 0; isphere < num_failure; isphere++) failure_spheres[isphere] = new double[_n_dim + 1];
            size_t ifailure(0);
            for (size_t isphere = 0; isphere < _num_inserted_points; isphere++)
            {
                if (_sample_points[isphere][_n_dim] < 0.0)
                {
                    for (size_t idim = 0; idim < _n_dim; idim++) failure_spheres[ifailure][idim] = _sample_points[isphere][idim];
                    failure_spheres[ifailure][_n_dim] = sqrt(-_sample_points[isphere][_n_dim]);
                    ifailure++;
                }
            }
            
            v_failure = estimate_spheres_volume_0d(failure_spheres, num_failure, _n_dim, _xmin, _xmax);
            // v_failure = estimate_spheres_volume_1d(failure_spheres, num_failure, _n_dim, _xmin, _xmax);
            //v_failure = estimate_spheres_volume_2d(failure_spheres, num_failure, _n_dim, _xmin, _xmax);
            
            for (size_t isphere = 0; isphere < num_failure; isphere++) delete[] failure_spheres[isphere];
            delete[] failure_spheres;
        }
        else v_failure = 0.0;
        
        if (num_non_failure > 0 && num_failure > 0)
        {
            // only calculate upper bound if non-failure disks exists
            double** non_failure_spheres = new double*[num_non_failure];
            for (size_t isphere = 0; isphere < num_non_failure; isphere++) non_failure_spheres[isphere] = new double[_n_dim + 1];
            size_t inonfailure(0);
            for (size_t isphere = 0; isphere < _num_inserted_points; isphere++)
            {
                if (_sample_points[isphere][_n_dim] > 0.0)
                {
                    for (size_t idim = 0; idim < _n_dim; idim++) non_failure_spheres[inonfailure][idim] = _sample_points[isphere][idim];
                    non_failure_spheres[inonfailure][_n_dim] = sqrt(_sample_points[isphere][_n_dim]);
                    inonfailure++;
                }
            }
            v_nonfailure = estimate_spheres_volume_0d(non_failure_spheres, num_non_failure, _n_dim, _xmin, _xmax);
            // v_nonfailure = estimate_spheres_volume_1d(non_failure_spheres, num_non_failure, _n_dim, _xmin, _xmax);
            //v_nonfailure = estimate_spheres_volume_2d(non_failure_spheres, num_non_failure, _n_dim, _xmin, _xmax);
            
            for (size_t isphere = 0; isphere < num_non_failure; isphere++) delete[] non_failure_spheres[isphere];
            delete[] non_failure_spheres;
        }
        else v_nonfailure = VOL;
        
        lower = v_failure / VOL;
        upper = 1.0 - (v_nonfailure / VOL);
        end_time = clock();
        
        std::cout<< "done!" << std::endl;
        
        cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
        std::cout<<  "    Execution Time = " << std::fixed << cpu_time << " seconds." << std::endl;		
        
        if (upper < lower) upper = lower;
        
        print_POF_results(lower, upper);
    }
    
    double NonDPOFDarts::estimate_spheres_volume_0d(double** spheres, size_t num_spheres, size_t num_dim, double* xmin, double* xmax)
    {
        std::vector<size_t> sphere_neighbors;
        double total_volume(0.0);
        double* dart = new double[num_dim];
        for (size_t isphere = 0; isphere < num_spheres; isphere++)
        {
            double ri = spheres[isphere][num_dim]; // radius of sphere i
            
            // retrieve overlapping spheres
            sphere_neighbors.clear(); bool redundant_sphere(false);
            for (size_t jsphere = 0; jsphere < num_spheres; jsphere++)
            {
                if (isphere == jsphere) continue;
                double rj = spheres[jsphere][num_dim]; // radius of sphere j
                double d(0.0);
                for (size_t idim = 0; idim < num_dim; idim++)
                {
                    double dx = spheres[jsphere][idim] - spheres[isphere][idim];
                    d += dx * dx;
                }
                d = sqrt(d);
                if (d + ri < rj)
                {
                    // sphere i is entirely inside sphere j
                    redundant_sphere = true; break;
                }
                if (d + rj < ri)
                {
                    // sphere j is entirely inside sphere i
                    continue;
                }
                if (d < ri + rj)
                {
                    // sphere j overlaps with sphere i
                    sphere_neighbors.push_back(jsphere);
                }
            }
            if (redundant_sphere) continue;
            
            double vi = get_sphere_volume(ri, num_dim);
            
            size_t num_experiments(20);
            std::vector<double> sum(num_experiments);
            std::vector<double> num(num_experiments);
            while (true)
            {
                // thow groups of 1000 darts uniformly from a sphere
                for (size_t iexp = 0; iexp < num_experiments; iexp++)
                {
                    size_t idart(0);
                    while (idart < 100)
                    {
                        sample_uniformly_from_unit_sphere(dart, num_dim);
                        
                        for (size_t idim = 0; idim < num_dim; idim++) dart[idim] = spheres[isphere][idim] + dart[idim] * ri;
                        
                        double di_sq(0.0);
                        for (size_t idim = 0; idim < num_dim; idim++)
                        {
                            double dx = dart[idim] - spheres[isphere][idim];
                            di_sq += dx * dx;
                        }
                        
                        double pi = di_sq - ri * ri; bool miss(false);
                        size_t num_neighbors(sphere_neighbors.size());
                        for (size_t ineighbor = 0; ineighbor < num_neighbors; ineighbor++)
                        {
                            size_t jsphere = sphere_neighbors[ineighbor];
                            double rj = spheres[jsphere][num_dim]; // radius of sphere j
                            
                            double dj_sq(0.0);
                            for (size_t idim = 0; idim < num_dim; idim++)
                            {
                                double dx = dart[idim] - spheres[jsphere][idim];
                                dj_sq += dx * dx;
                            }
                            double pj = dj_sq - rj * rj;
                            if (pj < pi)
                            {
                                miss = true; // dart is inside another power cell
                                break;
                            }
                        }
                        
                        if (!miss)
                        {
                            for (size_t idim = 0; idim < num_dim; idim++)
                            {	
                                if (dart[idim] < xmin[idim] || dart[idim] > xmax[idim])
                                {								
                                    miss = true; break;  // dart is outside domain
                                }
                            }
                        }
                        
                        idart++;
                        num[iexp] += 1.0;
                        if (miss) continue;
                        
                        sum[iexp] += 1.0;  // dart is inside the power cell of isphere
                    }
                }
                
                // mean and variance
                double mean(0.0);
                for (size_t iexp = 0; iexp < num_experiments; iexp++) mean+= sum[iexp] / num[iexp];
                mean /= num_experiments;
                
                double var(0.0);
                for (size_t iexp = 0; iexp < num_experiments; iexp++)
                {
                    double dvar = sum[iexp] / num[iexp] - mean;
                    var+= dvar * dvar;
                }
                var = sqrt(var/num_experiments);
                
                // exact bounds = mean +- variance 
                
                if (var * vi < 1E-2)
                {
                    total_volume += mean * vi;
                    break; // we obtain estimate up to 8 digits accuray					
                }
            }
            
        }
        delete[] dart;
        return total_volume;
    }


    double NonDPOFDarts::get_sphere_volume(double r, size_t num_dim)
    {
        double PI = 3.1415926535897932384626433832795;
        // borrowed from Scott's code + some edits
        // "0-dim"
        double v = 1.; // volume
        double s = 2.; // surface area of one dim higher ball
        for (unsigned int i = 1; i <= num_dim; ++i)
        {
            double v_next = s / i;
            s = 2 * PI * v;
            v = v_next;
        }
        double volume = v * pow(r, int(num_dim));
        return volume;
    }
    
    void NonDPOFDarts::sample_uniformly_from_unit_sphere(double* dart, size_t num_dim)
    {
        double sf = 0.0;
        for (size_t idim = 0; idim < num_dim; idim++)
        {
            double sum(0.0);
            // select 12 random numbers from 0.0 to 1.0
            for (size_t i = 0; i < 12; i++) sum += generate_a_random_number();
            sum -= 6.0;
            dart[idim] = sum;
            sf += dart[idim] * dart[idim];
        }
        sf = 1.0 / sqrt(sf);
        for (size_t idim = 0; idim < num_dim; idim++) dart[idim] *= sf;
        
        // point is on sphere surface now
        double u = pow(generate_a_random_number(), 1.0 / num_dim);
        for (size_t idim = 0; idim < num_dim; idim++) dart[idim] *= u;
    }
    
    double NonDPOFDarts::area_triangle(double x1, double y1, double x2, double y2, double x3, double y3)
    {
        return 0.5 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));
    }
    
    ////////////////////////////////////////////////////////////////
    // OUTPUT METHODS
    ////////////////////////////////////////////////////////////////
    
    double NonDPOFDarts::f_true(double* x)
    {
        return 0.0;
    }
    
    void NonDPOFDarts::plot_vertices_2d()
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
            
            if (_global_optimization && fabs(r) < 1E-10)
            {
                // global minima
                file << _sample_points[index][0] * scale << "  " << _sample_points[index][1] * scale << "  " << 0.02 * scale << "  ";
                file << "redfcirc"     << std::endl; // non-failure disk
            }
            else
            {
                file << _sample_points[index][0] * scale << "  " << _sample_points[index][1] * scale << "  " << r * scale << "  ";
                
                if (_sample_points[index][2] > 0) file << "greenfcirc"     << std::endl; // non-failure disk
                else                              file << "redfcirc"     << std::endl; // non-failure disk
            }
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
        
        // plot failure isocontour
        if (!_global_optimization)
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
            print_POF_results(lower, upper);
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

