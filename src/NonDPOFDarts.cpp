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
  emulatorSamples(probDescDB.get_int("method.nond.samples_on_emulator")),
  lipschitzType(probDescDB.get_string("method.lipschitz")),
  samples(probDescDB.get_int("method.build_samples"))
{
    // any initialization is done here.   For now, you should just specify
    // the number of samples, but eventually we will get that from the input spec
    /*
    Cout << "Hello World, POF Darts is coming! " << '\n';
    Cout << "Number of samples " << samples << '\n';
    Cout << "Emulator Samples " << emulatorSamples << '\n';
    Cout << "lipschitzType " << lipschitzType << '\n';
    */

    if (lipschitzType == "local")
    {
        _use_local_L = true;
        std::cout<< "pof: using local Lipschitz" << std::endl;
    }
    else if (lipschitzType == "global")
    {
        _use_local_L = false;       // Global Lipschitz: less sampling time - less accuracy
        std::cout<< "pof: using global Lipschitz" << std::endl;
    }
  
    if (emulatorSamples==0)
        emulatorSamples = 1E6;         // number of samples to evaluate surrogate


    if (iteratedModel.model_type() != "surrogate") {
      Cerr << "Error: NonDPOFDarts::iteratedModel must be a "
	   << "surrogate model." << std::endl;
      abort_handler(-1);
    }
    
    //Cout << "in initialize loop" << '\n';

}
    

NonDPOFDarts::~NonDPOFDarts()
{ }

bool NonDPOFDarts::resize()
{
  bool parent_reinit_comms = NonD::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void NonDPOFDarts::core_run()
{
    _eval_error = false; _safety_factor = 2.0;
    
    Cout << "I am now computing uncertainty! " << '\n';
    
    if (fabs(seed - 1.0) < 1E-10) seed  = time(0); // Automated seed
    else if (fabs(seed - 1.0) < 2)
    {
        seed = 1234567890;
        
        _eval_error = true;
        
        std::cout << "Evaluate Error: (0: No, 1: SmoothHerbie, 2: Herbie, 3: PlanarCross, 4: CircularCone) ";
        std::cin >> _test_function;
        
        if (_test_function == 0) _eval_error = false;
        if (_test_function == 1) std::cout << "\nSmooth Herbie" << std::endl;
        if (_test_function == 2) std::cout << "\nHerbie" << std::endl;
        if (_test_function == 3) std::cout << "\nPlanar cross" << std::endl;
        if (_test_function == 4) std::cout << "\nCircular Cone" << std::endl;

    }
    
   

    
    
    
    std::cout<< "seed = " << seed << std::endl;
    
    initiate_random_number_generator(seed);
    
    /* testing in-house rng
    for (size_t i = 0; i < 20; i++)
    {
        std::cout<< "Random number " << i << " = " << generate_a_random_number()<<std::endl;
    }
    */
    
    
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
        
        c = 0.0; zc = 0.0;	/* current CSWB and SWB `borrow` */
        zx = 5212886298506819.0 / 9007199254740992.0;	/* SWB seed1 */
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
        
        initialize_level_mappings();
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
        
        _sample_points = new double*[_total_budget];
        _sample_neighbors = new size_t*[_total_budget];
        _sample_vsize = new double[_total_budget];
        _dart = new double[_n_dim];
        
        _line_flat = new size_t[_n_dim];
        _line_flat_start = new double[_total_budget];
        _line_flat_end = new double[_total_budget];
        _line_flat_length = new double[_total_budget];
        
        _xmin = new double[_n_dim];
        _xmax = new double[_n_dim];
        
        const RealVector&  lower_bounds = iteratedModel.continuous_lower_bounds();
        const RealVector&  upper_bounds = iteratedModel.continuous_upper_bounds();
        
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            _xmin[idim] = lower_bounds[idim];
            _xmax[idim] = upper_bounds[idim];
        }
        
        // Estmate max radius of spheres based on number of evaluation and unifrom distribution
        _diag = std::sqrt((double)_n_dim);
        
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
        for (size_t isample = 0; isample < _num_inserted_points; isample++)
        {
            delete[] _sample_points[isample];
            if (_sample_neighbors[isample] != 0) delete[] _sample_neighbors[isample];
        }
        delete[] _sample_points;
        delete[] _sample_neighbors;
        delete[] _sample_vsize;
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
                for (size_t isample = 0; isample < _num_inserted_points; isample++) assign_sphere_radius_POF(isample);
                
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
                
                /*
                for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
                {
                    std::cout << "Sample Point :" << ipoint << " - num neighbors = " << _sample_neighbors[ipoint][0] << std::endl;
                    
                    for (size_t i = 1; i < _sample_neighbors[ipoint][0]; i++)
                    {
                        std::cout << "   Neighbor: " << i << " : " << _sample_neighbors[ipoint][i] << std::endl;
                    }
                }
                */

                
            }
        }
        
        std::cout <<  "pof:: Building Surrogates ..." << std::endl;
        estimate_pof_surrogate();
        
        if (_n_dim == 2 && _eval_error)
        {
            std::cout <<  "pof::    Plotting 2d disks ...";
            plot_vertices_2d(true, true);
            plot_neighbors();
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
            // sampling from a unit bounding box
            for (size_t idim = 0; idim < _n_dim; idim++) _dart[idim] = generate_a_random_number();
            
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
                    std::cout<< "\npof:: Void-finding budget has been exhausted, Increaing Lipschitz Multiplier!" << std::endl;
                    _safety_factor *= 1.5;
                    for (size_t isample = 0; isample < _num_inserted_points; isample++)
                    {
                        assign_sphere_radius_POF(isample);
                    }
                    
                    //std::cout<< "\npof:: Void-finding budget has been exhausted, shrinking BIG disks!" << std::endl;
                    //shrink_big_spheres();
                }
            }
        }
    }
    
    void NonDPOFDarts::line_dart_throwing_games(size_t game_index)
    {
        _num_successive_misses_p = 0;
        _num_successive_misses_m = 0;
        size_t num_prior_disks = _num_inserted_points;
        size_t nVsampling(0);
        while (true)
        {
            _num_darts++;
            
            nVsampling++;
            if (_use_local_L && nVsampling == 25)
            {
                // Sample a point at Futhest Voronoi Corner of larger Voronoi cell
                _max_vsize = 0.0; size_t ilargest(_num_inserted_points);
                for (size_t isample = 0; isample < _num_inserted_points; isample++)
                {
                    if (_sample_vsize[isample] > _max_vsize)
                    {
                        _max_vsize = _sample_vsize[isample];
                        ilargest = isample;
                    }
                }
                sample_furthest_vertex(ilargest, _dart);
                add_point(_dart);
                if (_num_inserted_points - num_prior_disks == samples)
                {
                    std::cout<< "\npof:: Simulation Budget has been exhausted!" << std::endl;
                    return;
                }
                nVsampling = 0;
                continue;
            }
            
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
                for (size_t jdim = 0; jdim < _n_dim; jdim++) _dart[jdim] = generate_a_random_number();
                
                _flat_dim = _line_flat[idim];
                _num_flat_segments = 1;
                _line_flat_start[0] = 0.0; _line_flat_end[0] = 1.0;
                
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
                    std::cout<< "\npof:: Void-finding budget has been exhausted, Increaing Lipschitz Multiplier!" << std::endl;
                    _safety_factor *= 1.5;
                    for (size_t isample = 0; isample < _num_inserted_points; isample++)
                    {
                        assign_sphere_radius_POF(isample);
                    }
                    //std::cout<< "\npof:: Void-finding budget has been exhausted, shrinking all disks!" << std::endl;
                    //shrink_big_spheres();
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
                double v = std::sqrt(r_sq - hh);
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
        _sample_neighbors[_num_inserted_points] = new size_t[1];
        _sample_neighbors[_num_inserted_points][0] = 0;
        
        for (size_t idim = 0; idim < _n_dim; idim++) _sample_points[_num_inserted_points][idim] = x[idim];
        
        double* x_actual = new double[_n_dim];
        for (size_t idim = 0; idim < _n_dim; idim++) x_actual[idim] = _xmin[idim] + x[idim] * (_xmax[idim] - _xmin[idim]);
        compute_response(x_actual);
        _num_inserted_points++;
        
        // Adjusting Spheres radii due to new point
        if (_use_local_L)
        {
            retrieve_neighbors(_num_inserted_points - 1, true);
            assign_sphere_radius_POF(_num_inserted_points - 1);
            
            verify_neighbor_consistency();
            
            size_t num_neighbors = 0;
            if (_sample_neighbors[_num_inserted_points - 1] != 0) num_neighbors = _sample_neighbors[_num_inserted_points - 1][0];
            for (size_t i = 1; i <= num_neighbors; i++)
            {
                size_t neighbor = _sample_neighbors[_num_inserted_points - 1][i];
                assign_sphere_radius_POF(neighbor); // adjusting radius of old disks due to new L
            }
            
            
        }
        else
        {
            update_global_L();
            for (size_t isample = 0; isample < _num_inserted_points; isample++) assign_sphere_radius_POF(isample);
        }
        delete [] x_actual;
    }
    
    void NonDPOFDarts::compute_response(double* x)
    {
        RealVector newX(_n_dim);
        for (size_t idim = 0; idim < _n_dim; idim++) newX[idim] = x[idim];
        
        iteratedModel.continuous_variables(newX);
	// bypass the surrogate model to evaluate the underlying truth model
	iteratedModel.surrogate_response_mode(BYPASS_SURROGATE);
        iteratedModel.evaluate();

	// TODO: later, generalize DataFitSurrModel to automatically
	// cache the points when in bypass mode
        add_surrogate_data(iteratedModel.current_variables(),
			   iteratedModel.current_response());
        
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            double fval = iteratedModel.current_response().function_value(resp_fn_count);
            _fval[resp_fn_count][_num_inserted_points] = fval;
        }
    }
    
    void NonDPOFDarts::verify_neighbor_consistency()
    {
        for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
        {
            size_t num_neighbors(_sample_neighbors[ipoint][0]);
            for (size_t i = 1; i <= num_neighbors; i++)
            {
                size_t neighbor = _sample_neighbors[ipoint][i];
                if (add_neighbor(neighbor, ipoint)) assign_sphere_radius_POF(neighbor);
            }
        }
    }

    
    bool NonDPOFDarts::add_neighbor(size_t ipoint, size_t ineighbor)
    {
        bool found(false);
        size_t num_neighbors(_sample_neighbors[ipoint][0]);
        for (size_t i = 1; i <= num_neighbors; i++)
        {
            if (_sample_neighbors[ipoint][i] == ineighbor) {found = true; break;}
        }
        if (!found)
        {
            // adding jpoint to ipoint neighbors
            size_t* tmp  = new size_t[num_neighbors + 2];
            for (size_t i = 0; i <= num_neighbors; i++) tmp[i] = _sample_neighbors[ipoint][i];
            tmp[num_neighbors + 1] = ineighbor;
            tmp[0]++;
            delete[] _sample_neighbors[ipoint];
            _sample_neighbors[ipoint] = tmp;
            return true;
        }
        return false; // Neighbor is already there
    }
    
    
    
    void NonDPOFDarts::retrieve_neighbors(size_t ipoint, bool update_point_neighbors)
    {
        if (_num_inserted_points == 1)
        {
            _sample_vsize[0] = 0.5 * _diag;
            return;
        }
        
        size_t* old_neighbors = _sample_neighbors[ipoint];
        
        size_t* tmp_neighbors = new size_t[_total_budget];
        
        double* tmp_pnt = new double[_n_dim];    // end of spoke
        double* qH = new double[_n_dim];         // mid-ppint
        double* nH = new double[_n_dim];         // normal vector
        
        _sample_vsize[ipoint] = 0.0;
        size_t num_neighbors(0), num_misses(0), max_misses(10);
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
            sf = 1.0 / std::sqrt(sf);
            for (size_t idim = 0; idim < _n_dim; idim++) tmp_pnt[idim] *= sf;
            
            // scale line spoke to extend outside bounding box
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                tmp_pnt[idim] *= _diag;
                tmp_pnt[idim] += _sample_points[ipoint][idim];
            }
            
            // trim line spoke with domain boundaries
            double t_end(1.0);
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                if (tmp_pnt[idim] > 1.0)
                {
                    double t = (1.0 - _sample_points[ipoint][idim]) / (tmp_pnt[idim] - _sample_points[ipoint][idim]);
                    if (t < t_end) t_end = t;
                }
                if (tmp_pnt[idim] < 0.0)
                {
                    double t = (_sample_points[ipoint][idim]) / (_sample_points[ipoint][idim] - tmp_pnt[idim]);
                    if (t < t_end) t_end = t;
                }
            }
            for (size_t idim = 0; idim < _n_dim; idim++) tmp_pnt[idim] = _sample_points[ipoint][idim] + t_end * (tmp_pnt[idim] - _sample_points[ipoint][idim]);
            
            // trim spoke using Voronoi faces
            size_t ineighbor(ipoint);
            for (size_t jpoint = 0; jpoint < _num_inserted_points; jpoint++)
            {
                if (jpoint == ipoint) continue;
                
                // trim line spoke via hyperplane between
                double norm(0.0);
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    qH[idim] = 0.5 * (_sample_points[ipoint][idim] + _sample_points[jpoint][idim]);
                    nH[idim] =  _sample_points[jpoint][idim] -  _sample_points[ipoint][idim];
                    norm+= nH[idim] * nH[idim];
                }
                norm = 1.0 / std::sqrt(norm);
                for (size_t idim = 0; idim < _n_dim; idim++) nH[idim] *= norm;
                
                if (trim_line_using_Hyperplane(_n_dim, _sample_points[ipoint], tmp_pnt, qH, nH))
                {
                    ineighbor = jpoint;
                }
            }
            
            double dst = 0.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double dx = _sample_points[ipoint][idim] - tmp_pnt[idim];
                dst += dx * dx;
            }
            dst = std::sqrt(dst);
            if (dst > _sample_vsize[ipoint]) _sample_vsize[ipoint] = dst;
            
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
        
        _sample_neighbors[ipoint] = new size_t[num_neighbors + 1];
        _sample_neighbors[ipoint][0] = num_neighbors;
        for (size_t i = 0; i < num_neighbors; i++)  _sample_neighbors[ipoint][i + 1] = tmp_neighbors[i];
        
        delete[] tmp_pnt;
        delete[] qH;
        delete[] nH;
        
        if (update_point_neighbors)
        {
            for (size_t i = 0; i < num_neighbors; i++)
            {
                retrieve_neighbors(tmp_neighbors[i], false);
            }
        }
        delete[] tmp_neighbors;
    }

    void NonDPOFDarts::sample_furthest_vertex(size_t ipoint, double* fv)
    {
        double* tmp_pnt = new double[_n_dim];    // end of spoke
        double* qH = new double[_n_dim];         // mid-ppint
        double* nH = new double[_n_dim];         // normal vector
        
        double vsize = 0.0;
        for (size_t ispoke = 0; ispoke < 10000; ispoke++)
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
            sf = 1.0 / std::sqrt(sf);
            for (size_t idim = 0; idim < _n_dim; idim++) tmp_pnt[idim] *= sf;
            
            // scale line spoke to extend outside bounding box
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                tmp_pnt[idim] *= _diag;
                tmp_pnt[idim] += _sample_points[ipoint][idim];
            }
            
            // trim line spoke with domain boundaries
            double t_end(1.0);
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                if (tmp_pnt[idim] > 1.0)
                {
                    double t = (1.0 - _sample_points[ipoint][idim]) / (tmp_pnt[idim] - _sample_points[ipoint][idim]);
                    if (t < t_end) t_end = t;
                }
                if (tmp_pnt[idim] < 0.0)
                {
                    double t = (_sample_points[ipoint][idim]) / (_sample_points[ipoint][idim] - tmp_pnt[idim]);
                    if (t < t_end) t_end = t;
                }
            }
            for (size_t idim = 0; idim < _n_dim; idim++) tmp_pnt[idim] = _sample_points[ipoint][idim] + t_end * (tmp_pnt[idim] - _sample_points[ipoint][idim]);
            
            // trim spoke using Voronoi faces
            size_t ineighbor(ipoint);
            for (size_t jpoint = 0; jpoint < _num_inserted_points; jpoint++)
            {
                if (jpoint == ipoint) continue;
                
                // trim line spoke via hyperplane between
                double norm(0.0);
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    qH[idim] = 0.5 * (_sample_points[ipoint][idim] + _sample_points[jpoint][idim]);
                    nH[idim] =  _sample_points[jpoint][idim] -  _sample_points[ipoint][idim];
                    norm+= nH[idim] * nH[idim];
                }
                norm = 1.0 / std::sqrt(norm);
                for (size_t idim = 0; idim < _n_dim; idim++) nH[idim] *= norm;
                
                if (trim_line_using_Hyperplane(_n_dim, _sample_points[ipoint], tmp_pnt, qH, nH))
                {
                    ineighbor = jpoint;
                }
            }
            
            double dst = 0.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double dx = _sample_points[ipoint][idim] - tmp_pnt[idim];
                dst += dx * dx;
            }
            dst = std::sqrt(dst);
            if (dst > vsize)
            {
                vsize = dst;
                for (size_t idim = 0; idim < _n_dim; idim++) fv[idim] = tmp_pnt[idim];
            }
            
        } // end of spoke loop
        
        delete[] tmp_pnt; delete[] qH; delete[] nH;
    }

    
    
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // POF methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void NonDPOFDarts::update_global_L()
    {
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            // update Lipschitz constant
            size_t last_point = _num_inserted_points - 1;
            for (size_t isample = 0; isample < last_point; isample++)
            {
                double dst = 0.0;
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    double dx = _sample_points[isample][idim] - _sample_points[last_point][idim];
                    dst += dx * dx;
                }
                dst = std::sqrt(dst);
                double L = fabs(_fval[resp_fn_count][isample] - _fval[resp_fn_count][last_point]) / dst;
                if (L > _Lip[resp_fn_count]) _Lip[resp_fn_count] = L;
            }
        }
    }

    
    void NonDPOFDarts::assign_sphere_radius_POF(size_t isample)
    {
        double r = _diag;
        
        double L = 0.0;
        
        if (_use_local_L)
        {
            r = _sample_vsize[isample];
            
            size_t num_neighbors = 0;
            if (_sample_neighbors[isample]!= 0) num_neighbors = _sample_neighbors[isample][0];
            
            double local_L = 0.0;
            for (size_t i = 1; i <= num_neighbors; i++)
            {
                size_t neighbor = _sample_neighbors[isample][i];

                double dst = 0.0;
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    double dx = _sample_points[isample][idim] - _sample_points[neighbor][idim];
                    dst += dx * dx;
                }
                dst = std::sqrt(dst);
                double grad = fabs(_fval[_active_response_function][isample] - _fval[_active_response_function][neighbor]) / dst;
                if (grad > local_L) local_L = grad;
            }
            L = _safety_factor * local_L; // going conservative
            if (L < 0.1) L = 0.1;
        }
        else
        {
            L = _Lip[_active_response_function]; // Global Lipschitz constant
        }
        
        // std::cout << "point " << isample << " L = " << L << std::endl;
        
        if (L > 1E-10) r = fabs(_fval[_active_response_function][isample]  - _failure_threshold) / L; // radius based on Lipschitz
        
        _sample_points[isample][_n_dim] = r * r;
        if (_fval[_active_response_function][isample] < _failure_threshold) _sample_points[isample][_n_dim] = - _sample_points[isample][_n_dim];
        
        if (_use_local_L)
        {
            // make sure that last disk does not overlap with a disk of different color
            
            // A sphere shouldn't contain a sample point that is not its neighbor
            
            for (size_t jsample = 0; jsample < _num_inserted_points; jsample++)
            {
                //if (_sample_points[isample][_n_dim] * _sample_points[jsample][_n_dim] > 0.0) continue; // same color
                
                if (isample == jsample) continue;
                
                double dst_sq = 0.0;
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    double dx = _sample_points[isample][idim] - _sample_points[jsample][idim];
                    dst_sq += dx * dx;
                }
                
                double r_i = std::sqrt(fabs(_sample_points[isample][_n_dim]));
                double r_j = std::sqrt(fabs(_sample_points[jsample][_n_dim]));
                double dst = std::sqrt(dst_sq);
                
                if (r_i + r_j > dst)
                {
                    L = fabs(_fval[_active_response_function][isample] - _fval[_active_response_function][jsample]) / dst;
                    double r_i_new = fabs(_fval[_active_response_function][isample] - _failure_threshold) / L;
                    double r_j_new = fabs(_fval[_active_response_function][jsample] - _failure_threshold) / L;
                    if (r_i_new < r_i)
                    {
                        _sample_points[isample][_n_dim] = r_i_new * r_i_new;
                        if (_fval[_active_response_function][isample] < _failure_threshold) _sample_points[isample][_n_dim] = - _sample_points[isample][_n_dim];
                    }
                    if (r_j_new < r_j)
                    {
                        _sample_points[jsample][_n_dim] = r_j_new * r_j_new;
                        if (_fval[_active_response_function][jsample] < _failure_threshold) _sample_points[jsample][_n_dim] = - _sample_points[jsample][_n_dim];
                    }
                }
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
        
        std::cout << "\npof:: maximum radius = " << std::sqrt(rr_max) << std::endl;
        
        for (size_t isample = 0; isample < _num_inserted_points; isample++)
        {
            if (fabs(_sample_points[isample][_n_dim]) > 0.95 * 0.95 * rr_max) _sample_points[isample][_n_dim] *= (0.95 * 0.95);
        }
    }
    
 
    double NonDPOFDarts::area_triangle(double x1, double y1, double x2, double y2, double x3, double y3)
    {
        return 0.5 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));
    }
    
    ////////////////////////////////////////////////////////////////
    // SUUROGATE METHODS
    ////////////////////////////////////////////////////////////////
    
    void NonDPOFDarts::add_surrogate_data(const Variables& vars, const Response& resp)
    {
      // TODO: have the surrogate model instead cache the data when in
      // bypass mode
      IntResponsePair tmp_pair((int)0, resp);
      bool rebuild_flag = false;
      iteratedModel.append_approximation(vars, tmp_pair, rebuild_flag);
    }
    
    void NonDPOFDarts::build_surrogate()
    {
      // TODO: do we just send all points here? or append one at a time?
      iteratedModel.build_approximation();
      // change surrogate to evaluate the surrogate model
      iteratedModel.surrogate_response_mode(AUTO_CORRECTED_SURROGATE); 
    }
    
    Real NonDPOFDarts::eval_surrogate(size_t function_index, double* x)
    {
      // this copy could be moved outside the loop for memory efficiency
      for (size_t vi = 0; vi < numContinuousVars; ++vi)
	iteratedModel.continuous_variable(x[vi], vi);
      // TODO: use active_set_vector for efficiency if you truly only
      // need 1 response function?
      iteratedModel.evaluate();
      const RealVector& fn_vals = 
	iteratedModel.current_response().function_values();
      return fn_vals[function_index];
    }
   
    void NonDPOFDarts::estimate_pof_surrogate()
    {
        clock_t start_time, end_time; double cpu_time, total_time(0.0);
        
        start_time = clock();
        
        build_surrogate();
            
        end_time = clock();
        cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
            
        std::cout.precision(4);
        std::cout <<  "pof::    Surrogates built in " << std::fixed << cpu_time << " seconds." << std::endl;

         // evaluate the surrogate for the given function
        double** num_fMC_samples = new double*[numFunctions];
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            size_t num_levels = requestedRespLevels[resp_fn_count].length();
            num_fMC_samples[resp_fn_count] = new double[num_levels];
            for (size_t level_count = 0; level_count < num_levels; level_count++) num_fMC_samples[resp_fn_count][level_count] = 0.0;
        }
        
        start_time = clock();

        double pof_exact = 0.0;
        
        double isample = 0.0;
        double num_MC_samples(emulatorSamples);
        double* tmp_pnt = new double[_n_dim];
        if (pdfOutput) {
          extremeValues.resize(numFunctions);
          for (size_t i=0; i<numFunctions; ++i)
            { extremeValues[i].first = DBL_MAX; extremeValues[i].second = -DBL_MAX; }
        }

        while (isample < num_MC_samples)
        {
            // sample a random point from the domain
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                tmp_pnt[idim] = _xmin[idim] + generate_a_random_number() * (_xmax[idim] - _xmin[idim]);
            }
            
            double f_exact = 0.0;
            if (_eval_error) f_exact = f_true(tmp_pnt);
            
            for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
            {
                // evaluate sample point using surrogate
                double surrogate_value = eval_surrogate(resp_fn_count, tmp_pnt);
                if (pdfOutput) {
                  if (surrogate_value < extremeValues[resp_fn_count].first)
                    extremeValues[resp_fn_count].first = surrogate_value;
                  if (surrogate_value > extremeValues[resp_fn_count].second) 
                    extremeValues[resp_fn_count].second = surrogate_value;
                }
                
                size_t num_levels = requestedRespLevels[resp_fn_count].length();
                for (size_t level_count = 0; level_count < num_levels; level_count++)
                {
                    _failure_threshold = requestedRespLevels[resp_fn_count][level_count];
                    
                    if (surrogate_value <  _failure_threshold)
                    {
                        num_fMC_samples[resp_fn_count][level_count] += 1.0;
                    }
                    
                    if (_eval_error && f_exact <  _failure_threshold) pof_exact += 1.0;
                }
            }
            isample+=1.0;
        }

        end_time = clock();
        cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
        
        std::cout <<  "pof::    Surrogate evaluated in " << std::fixed << cpu_time << " seconds." << std::endl;
        
        if (_eval_error)
        {
            std::cout.precision(15);
            std::cout <<  "pof::    exact pof " << std::fixed << fabs(pof_exact)/ num_MC_samples << std::endl;
            std::cout <<  "pof::    Surrogate error " << std::fixed << fabs((num_fMC_samples[0][0] - pof_exact) / num_MC_samples) << std::endl;
        }
        
        
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
        
        if (pdfOutput) {
        // infer PDFs from computedProbLevels
          compute_densities(extremeValues);
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
    
    ////////////////////////////////////////////////////////////////
    // OUTPUT METHODS
    ////////////////////////////////////////////////////////////////
    
    double NonDPOFDarts::f_true(double* x)
    {
        
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
        
        if (_test_function == 1) // Smooth Herbie
        {
            double fval = 1.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double xm = x[idim] - 1.0;
                double xp = x[idim] + 1.0;
                double wherb = exp(- xm * xm) + exp(-0.8 * xp * xp);
                fval *= wherb;
            }
            fval = -fval;
            return fval;
        }
        if (_test_function == 2) // Herbie
        {
            double fval = 1.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double xm = x[idim] - 1.0;
                double xp = x[idim] + 1.0;
                double wherb = exp(- xm * xm) + exp(-0.8 * xp * xp) - 0.05 * sin(8 * (x[idim] + 0.1));
                fval *= wherb;
            }
            fval = -fval;
            return fval;
        }
        if (_test_function == 3) // Planar Cross
        {
            const double PI = 3.14159265359;
            double fval = 1.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double wcross = (1 + cos(2 * PI * x[idim])) / 2.0;
                fval *= wcross;
            }
            fval = pow(fval, 1.0/_n_dim);
            return fval;
        }
        if (_test_function == 4) // Circular Cone
        {
            double h = 0.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double dx = x[idim];
                h += dx * dx;
            }
            h = std::sqrt(h);
            return h;
        }
        
        return 0.0;
    }
    
    void NonDPOFDarts::plot_vertices_2d(bool plot_true_function, bool plot_suurogate)
    {
        std::stringstream ss;
        ss << "pof_" << _active_response_function << ".ps";
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
        file << " 0.0 setlinewidth" << std::endl;
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
        file << " 0.0 setlinewidth" << std::endl;
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
            double r = std::sqrt(fabs(_sample_points[index][2]));
            
            double x = _xmin[0] + _sample_points[index][0] * (_xmax[0] - _xmin[0]);
            double y = _xmin[1] + _sample_points[index][1] * (_xmax[1] - _xmin[1]);
            double rs = r * (_xmax[0] - _xmin[0]);
            
            
            file << x * scale << "  " << y * scale << "  " << rs * scale << "  ";
            
            //if (_sample_vsize[index] >  0.8 * _max_vsize) file << "bluefcirc"     << std::endl; // large Vcell
            if (_sample_points[index][2] > 0) file << "greenfcirc"     << std::endl; // non-failure disk
            else                              file << "redfcirc"     << std::endl; // non-failure disk
           
        }
        
        // plot discs boundaries
        for (size_t index = 0; index < _num_inserted_points; index++)
        {
            double r = std::sqrt(fabs(_sample_points[index][2]));
            double x = _xmin[0] + _sample_points[index][0] * (_xmax[0] - _xmin[0]);
            double y = _xmin[1] + _sample_points[index][1] * (_xmax[1] - _xmin[1]);
            double rs = r * (_xmax[0] - _xmin[0]);
            
            file << x * scale << "  " << y * scale << "  " << rs * scale << "  ";
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
            double r = std::sqrt(fabs(_sample_points[index][2]));
            
            double x = _xmin[0] + _sample_points[index][0] * (_xmax[0] - _xmin[0]);
            double y = _xmin[1] + _sample_points[index][1] * (_xmax[1] - _xmin[1]);
            double rs = r * (_xmax[0] - _xmin[0]);
            s = 0.002 * (_xmax[0] - _xmin[0]);
            if (r * 0.05 < s) s = r * 0.05;

            
            // plot vertex
            file << x * scale << "  " << y * scale << "  " << s * scale << " ";
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
    
    void NonDPOFDarts::plot_neighbors( )
    {
        std::stringstream ss;
        ss << "pof_neighbors" << _active_response_function << ".ps";
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
            double r = std::sqrt(fabs(_sample_points[index][2]));
            
            file << _sample_points[index][0] * scale << "  " << _sample_points[index][1] * scale << "  " << r * scale << "  ";
            
            if (_sample_points[index][2] > 0) file << "greenfcirc"     << std::endl; // non-failure disk
            else                              file << "redfcirc"     << std::endl; // non-failure disk
            
        }
        
        // plot discs boundaries
        for (size_t index = 0; index < _num_inserted_points; index++)
        {
            double r = std::sqrt(fabs(_sample_points[index][2]));
            file << _sample_points[index][0] * scale << "  " << _sample_points[index][1] * scale << "  " << r * scale << "  ";
            file << "circ"     << std::endl;
        }
        
        // plot line flat segments
        
        
        for (size_t isample = 0; isample < _num_inserted_points; isample++)
        {
            size_t num_neighbors = 0;
            if (_sample_neighbors[isample] != 0) num_neighbors = _sample_neighbors[isample][0];
            
            for (size_t i = 1; i <= num_neighbors; i++)
            {
                size_t neighbor = _sample_neighbors[isample][i];
                // draw a line between isample and neighbor
                file << _sample_points[isample][0] * scale << "  " << _sample_points[isample][1] * scale << "  ";
                file << _sample_points[neighbor][0] * scale << "  " << _sample_points[neighbor][1] * scale << "  ";
                file << "blueseg"     << std::endl;
            }
        }
        
        double s(0.01);
        for (size_t index = 0; index < _num_inserted_points; index++)
        {
            double r = std::sqrt(fabs(_sample_points[index][2]));
            s = r * 0.05;
            
            // plot vertex
            file << _sample_points[index][0] * scale << "  " << _sample_points[index][1] * scale << "  " << s * scale << " ";
            file << "blackfcirc"     << std::endl; // non-failure disk
        }
    }

    void NonDPOFDarts::print_results(std::ostream& s)
    {
        //s << "\nStatistics based on the importance sampling calculations:\n";
        s << "\nStatistics based on MC sampling calculations:\n";
        print_level_mappings(s);
    }

} // namespace Dakota

