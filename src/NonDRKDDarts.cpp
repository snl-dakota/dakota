/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDRKDDarts
//- Description: Class for the Recursive k-d DARTS approach
//- Owner:       Mohamed Ebeida and Ahmad Rushdi
//- Checked by:
//- Version:


#include "NonDRKDDarts.hpp"
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

static const char rcsId[] = "@(#) $Id: NonDRKDDart.cpp 6080 2009-09-08 19:03:20Z gtang $";

namespace Dakota {

NonDRKDDarts::NonDRKDDarts(ProblemDescDB& problem_db, Model& model):
  NonD(problem_db, model), seed(probDescDB.get_int("method.random_seed")),
  emulatorSamples(probDescDB.get_int("method.nond.samples_on_emulator")),
  samples(probDescDB.get_int("method.build_samples"))
{
    // Constructor
    std::cout << "------------------" << std::endl;
    std::cout << "*** LAUNCH RKD ***" << std::endl;
    std::cout << "------------------" << std::endl;
    
    if (emulatorSamples==0) emulatorSamples = 1E6;
}
    // --------------------------
    // Destructor
    NonDRKDDarts::~NonDRKDDarts()
    { }

bool NonDRKDDarts::resize()
{
  bool parent_reinit_comms = NonD::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}
    
    // --------------------------
    /** Generate Parameter Sets. */
    void NonDRKDDarts::pre_run()
    {
        Analyzer::pre_run();

        Cout << "*** Generating " << samples << " points *** " << '\n';
        
        initiate_random_number_generator(seed);

        init_rkd_darts(); // prepare global containers
        
    }
    // --------------------------
    /** Loop over the set of samples and compute responses. */
    void NonDRKDDarts::core_run()
    {
        Cout << "*** Quantifying uncertainty *** " << '\n';
        
        _eval_error = false;
        
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
        execute();
    }
    // --------------------------
    /** Print function evaluation summary, and integration results. */
    void NonDRKDDarts::post_run(std::ostream& s)
    {
        // generate outputs
        
        iteratedModel.print_evaluation_summary(s);
        
        s << "-----------------------------------------------------------------\n";
        Cout << "*** Printing integration results *** " << '\n';
        print_integration_results(s);
        s << "-----------------------------------------------------------------\n";
        
        exit_rkd_darts(); // clean up memory

	Analyzer::post_run(s);
    }
    /////////////////////////////////////////////////////////////
    //
    // RKD METHODS
    //
    /////////////////////////////////////////////////////////////
    // Init random
    void NonDRKDDarts::initiate_random_number_generator(unsigned long x)
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
    // ----------------------------------------------------------------
    // generate random
    double NonDRKDDarts::generate_a_random_number()
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
    // ----------------------------------------------------------------
    // Initialization of the RKD method
    void NonDRKDDarts::init_rkd_darts()
    {
        _num_dim = numContinuousVars;
        _num_samples = 0;
        _num_evaluations = 0;
        _evaluation_budget = samples;
        _max_num_neighbors = 4;
        _discont_jump_threshold = 0.8;
        
        initialize_level_mappings();
        
        _xmin = new double[_num_dim];
        _xmax = new double[_num_dim];
        
        const RealVector&  lower_bounds = iteratedModel.continuous_lower_bounds();
        const RealVector&  upper_bounds = iteratedModel.continuous_upper_bounds();
        
        for (size_t idim = 0; idim < _num_dim; idim++)
        {
            _xmin[idim] = lower_bounds[idim];
            _xmax[idim] = upper_bounds[idim];
        }
        
        initiate_random_number_generator(seed);
        
        create_rkd_containers(100 * samples); // To be reviewed?
        
        _num_inserted_points = 0;
        
        _fval = new double*[numFunctions];
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++) _fval[resp_fn_count] = new double[100 * samples];
    }
    // ----------------------------------------------------------------
    // Containers and memory allocations
    void NonDRKDDarts::create_rkd_containers(size_t expected_num_samples)
    {
        _max_num_samples = expected_num_samples;
        _sample_dim = new size_t[_max_num_samples];
        _sample_parent = new size_t[_max_num_samples];
        _sample_first_child = new size_t[_max_num_samples];
        _sample_num_children = new size_t[_max_num_samples];
        _sample_left = new size_t[_max_num_samples];
        _sample_right = new size_t[_max_num_samples];
        _sample_coord = new double[_max_num_samples];
        _sample_value = new double[_max_num_samples];
        _sample_left_interp_err = new double[_max_num_samples];
        _sample_right_interp_err = new double[_max_num_samples];
        _sample_left_ev_err = new double[_max_num_samples];
        _sample_right_ev_err = new double[_max_num_samples];
        
        // root sample index = 0
        _sample_dim[0] = 0; // has no meaning
        _sample_parent[0] = UINT_MAX; // has no meaning
        _sample_first_child[0] = 0; // means has no child yet
        _sample_num_children[0] = 0;
        _sample_left[0] = 0; // means no sample exists to its left
        _sample_right[0] = 0; // means no sample exists to its ight
        _sample_coord[0] = DBL_MAX; // has no meaning
        _sample_value[0] = 0.0; // initial guess
        _sample_left_interp_err[0] = 0.0; // has no meaning
        _sample_right_interp_err[0] = 0.0; // has no meaning
        _sample_left_ev_err[0] = 0.0; // has no meaning
        _sample_right_ev_err[0] = 0.0; // has no meaning
        _num_samples++;
        
        _I_RKD = new double[numFunctions];
    }
    // ----------------------------------------------------------------
    void NonDRKDDarts::exit_rkd_darts()
    {
        destroy_rkd_containers();
    }
    // ----------------------------------------------------------------
    // Destroy memory containers
    void NonDRKDDarts::destroy_rkd_containers()
    {
        delete[] _xmin;
        delete[] _xmax;
        
        delete[] _sample_dim;
        delete[] _sample_parent;
        delete[] _sample_first_child;
        delete[] _sample_num_children;
        delete[] _sample_left;
        delete[] _sample_right;
        
        delete[] _sample_coord;
        delete[] _sample_value;
        delete[] _sample_left_interp_err;
        delete[] _sample_right_interp_err;
        delete[] _sample_left_ev_err;
        delete[] _sample_right_ev_err;
        
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++) delete[] _fval[resp_fn_count];
        delete[] _fval;
    }
    // ----------------------------------------------------------------
    // RKD Method Execution
    void NonDRKDDarts::execute()
    {
        clock_t start_time, end_time;
        double cpu_time, total_time(0.0);
        start_time = clock();
        
        create_initial_children(0);
        
        while (_num_evaluations < _evaluation_budget) improve_parent_evaluation(0);
        
        end_time = clock();
        cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
        
        estimate_rkd_surrogate();
    }
    // ----------------------------------------------------------------
    // Initial RKD tree creation
    void NonDRKDDarts::create_initial_children(size_t parent)
    {
        double u, dx, xx;
        
        size_t child_dim = _sample_dim[parent] + 1;
        if (parent == 0) child_dim = 0;
        
        size_t left_child = _num_samples;
        u = generate_a_random_number();
        dx = _xmax[child_dim] - _xmin[child_dim];
        xx = _xmin[child_dim] + 0.125 * dx + 0.25 * u * dx;
        create_new_sample(parent, 0, 0, xx); // hyperplane
        
        size_t right_child = _num_samples;
        u = generate_a_random_number();
        dx = _xmax[child_dim] - _xmin[child_dim];
        xx =  _xmin[child_dim] + 0.625 * dx + 0.25 * u * dx;
        create_new_sample(parent, left_child, 0, xx);
        evaluate_1d_surrogate(parent);
    }
    // ----------------------------------------------------------------
    // Add a new
    void NonDRKDDarts::create_new_sample(size_t parent, size_t left, size_t right, double position)
    {
        if (_num_evaluations == _evaluation_budget) return;
        
        size_t sample = _num_samples;
        size_t idim = _sample_dim[parent] + 1;
        if (parent == 0) idim = 0;
        
        _sample_dim[sample] = idim;
        _sample_parent[sample] = parent;
        _sample_first_child[sample] = 0; // means has no child yet
        _sample_left[sample] = left;
        _sample_right[sample] = right;
        if (left == 0) _sample_first_child[parent] = sample;
        if (left > 0) _sample_right[left] = sample;
        if (right > 0) _sample_left[right] = sample;
        _sample_coord[sample] = position;
        _sample_value[sample] = 0.0; // initial guess
        _sample_left_interp_err[sample] = 0.0; // initial guess
        _sample_right_interp_err[sample] = 0.0; // initial guess
        _sample_left_ev_err[sample] = 0.0; // initial guess
        _sample_right_ev_err[sample] = 0.0; // initial guess
        _num_samples++;  _sample_num_children[parent]++; _sample_num_children[sample] = 0;
        //std::cout << "New sample created, index = " << sample  << ", left = " << _sample_left[sample] << " , x = " << _sample_coord[sample] << ", parent = "<< parent << std::endl;
        
        if (idim < _num_dim - 1)
        {
            create_initial_children(sample);
            double err = get_surrogate_interp_error(sample);
            
            double min_err_neighbor = DBL_MAX;
            if (left > 0) min_err_neighbor = get_surrogate_interp_error(left);
            if (right > 0)
            {
                double right_err = get_surrogate_interp_error(right);
                if (right_err < min_err_neighbor) min_err_neighbor = right_err;
            }
            while (err > min_err_neighbor && _num_evaluations < _evaluation_budget)
            {
                improve_parent_evaluation(sample);
                err = get_surrogate_interp_error(sample);
            }
            
            min_err_neighbor = err;
            if (left > 0)
            {
                err = get_surrogate_interp_error(left);
                while (err > min_err_neighbor && _num_evaluations < _evaluation_budget)
                {
                    improve_parent_evaluation(left);
                    err = get_surrogate_interp_error(left);
                }
            }
            
            if (right > 0)
            {
                err = get_surrogate_interp_error(right);
                while (err > min_err_neighbor && _num_evaluations < _evaluation_budget)
                {
                    improve_parent_evaluation(right);
                    err = get_surrogate_interp_error(right);
                }
            }
        }
        else
        {
            // end of the tree, retrieve coordinate from parents
            size_t cc(sample), pp;
            double* x = new double [_num_dim];
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                x[_num_dim - idim - 1] = _sample_coord[cc];
                pp = _sample_parent[cc];
                cc = pp;
            }
            compute_response(x);
            
            
            for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
            {
                double fval = iteratedModel.current_response().function_value(resp_fn_count);
                _fval[resp_fn_count][_num_evaluations] = fval;
            }
            _sample_value[sample] = _fval[0][_num_evaluations];
            
            _num_evaluations++;
            
            delete[] x;
        }
        
        evaluate_1d_surrogate(parent);
    }
    // ----------------------------------------------------------------
    // Evaluate Interpolation Error of a 1-d surrogate
    double NonDRKDDarts::get_surrogate_interp_error(size_t parent)
    {
        size_t num_samples = _sample_num_children[parent];
        size_t* samples = new size_t[num_samples];
        get_children(parent, samples);
        
        double err(0.0);
        for (size_t i = 0; i < num_samples; i++)
        {
            size_t sample_index = samples[i];
            err+= _sample_left_interp_err[sample_index];
            err+= _sample_right_interp_err[sample_index];
        }
        delete[] samples;
        return err;
    }
    // ----------------------------------------------------------------
    // Estimate Evaluation Error of a function value
    double NonDRKDDarts::estimate_surrogate_evaluation_err(size_t parent)
    {
        size_t child_dim = _sample_dim[parent] + 1;
        if (parent == 0) child_dim = 0;
        if (child_dim == _num_dim - 1) return 0.0;
        
        size_t num_samples = _sample_num_children[parent];
        size_t* samples = new size_t[num_samples];
        get_children(parent, samples);
        
        double surrogate_ev_err(0.0);
        for (size_t i = 0; i < num_samples; i++)
        {
            size_t sample_index = samples[i];
            
            size_t idim = _sample_dim[sample_index];
            double xseed = _sample_coord[sample_index];
            double xmin = _xmin[idim]; double xmax = _xmax[idim];
            if (_sample_left[sample_index] > 0)
            {
                size_t left = _sample_left[sample_index];
                xmin = 0.5 * (_sample_coord[sample_index] + _sample_coord[left]);
            }
            if (_sample_right[sample_index] > 0)
            {
                size_t right = _sample_right[sample_index];
                xmax = 0.5 * (_sample_coord[sample_index] + _sample_coord[right]);
            }
            
            double eval_err = get_surrogate_interp_error(sample_index);
            
            _sample_left_ev_err[sample_index] = eval_err * (xseed - xmin);
            _sample_right_ev_err[sample_index] = eval_err * (xmax - xseed);
            
            surrogate_ev_err+= _sample_left_ev_err[sample_index];
            surrogate_ev_err+= _sample_right_ev_err[sample_index];
        }
        delete[] samples;
        return surrogate_ev_err;
    }
    // ----------------------------------------------------------------
    void NonDRKDDarts::get_children(size_t parent, size_t* children)
    {
        size_t num_children(0);
        size_t child = _sample_first_child[parent];
        while (child > 0)
        {
            children[num_children] = child;
            num_children++;
            child = _sample_right[child];
        }
    }
    // ----------------------------------------------------------------
    void NonDRKDDarts::get_neighbors(size_t sample, size_t &num_neighbors, size_t* neighbors)
    {
        num_neighbors = 0; double dF;
        size_t current_left(sample);
        size_t current_right(sample);
        size_t left = _sample_left[sample];
        size_t right = _sample_right[sample];
        while (num_neighbors < _max_num_neighbors)
        {
            if (left > 0)
            {
                dF = fabs(_sample_value[current_left] - _sample_value[left]);
                if (dF > _discont_jump_threshold) left = 0;
            }
            if (right > 0)
            {
                dF = fabs(_sample_value[current_right] - _sample_value[right]);
                if (dF > _discont_jump_threshold) right = 0;
            }
            if (left > 0)  {neighbors[num_neighbors] = left; num_neighbors++; current_left = left; left = _sample_left[current_left];}
            if (num_neighbors == _max_num_neighbors) break;
            if (right > 0) {neighbors[num_neighbors] = right; num_neighbors++; current_right = right; right = _sample_right[current_right];}
            if (left == 0 && right == 0) break;
        }
    }
    // ----------------------------------------------------------------
    // Return the value of 1-d Lagrange interpolation (one level in the tree)
    double NonDRKDDarts::interpolate_lagrange(size_t num_data_points, double* data_x, double* data_f, double x)
    {
        double sum(0.0);
        for (size_t i = 0; i < num_data_points; i++)
        {
            double prod(1.0);
            for (size_t j = 0; j < num_data_points; j++)
            {
                if (j == i) continue;
                double term = (x - data_x[j]) / (data_x[i] - data_x[j]);
                prod *= term;
            }
            sum+= data_f[i] * prod;
        }
        return sum;
    }
    // ----------------------------------------------------------------
    double NonDRKDDarts::integrate_legendre_gauss(double xmin, double xmax, size_t num_data_points, double* data_x, double* data_f, double &err_est)
    {
        double* x_quad;
        double* w_quad;
        
        size_t poly_order = num_data_points - 1;
        size_t num_quadrature_points = (poly_order + 1) / 2 + 1;
        x_quad = new double[num_quadrature_points];
        w_quad = new double[num_quadrature_points];
        
        if (num_quadrature_points == 1)
        {
            x_quad[0] = 0.0000000000000000; w_quad[0] = 2.0000000000000000;
        }
        else if (num_quadrature_points == 2)
        {
            x_quad[0] = -0.5773502691896257; w_quad[0] = 1.0000000000000000;
            x_quad[1] =  0.5773502691896257; w_quad[1] = 1.0000000000000000;
        }
        else if (num_quadrature_points == 3)
        {
            x_quad[0] =  0.0000000000000000; w_quad[0] = 0.8888888888888888;
            x_quad[1] = -0.7745966692414834; w_quad[1] = 0.5555555555555556;
            x_quad[2] =  0.7745966692414834; w_quad[2] = 0.5555555555555556;
        }
        else if (num_quadrature_points == 4)
        {
            x_quad[0] = -0.3399810435848563; w_quad[0] = 0.6521451548625461;
            x_quad[1] =  0.3399810435848563; w_quad[1] = 0.6521451548625461;
            x_quad[2] = -0.8611363115940526; w_quad[2] = 0.3478548451374538;
            x_quad[3] =  0.8611363115940526; w_quad[3] = 0.3478548451374538;
        }
        else if (num_quadrature_points == 5)
        {
            x_quad[0] =  0.0000000000000000; w_quad[0] = 0.5688888888888889;
            x_quad[1] = -0.5384693101056831; w_quad[1] = 0.4786286704993665;
            x_quad[2] =  0.5384693101056831; w_quad[2] = 0.4786286704993665;
            x_quad[3] = -0.9061798459386640; w_quad[3] = 0.2369268850561891;
            x_quad[4] =  0.9061798459386640; w_quad[4] = 0.2369268850561891;
        }
        else if (num_quadrature_points == 6)
        {
            x_quad[0] = -0.6612093864662645; w_quad[0] = 0.3607615730481386;
            x_quad[1] =  0.6612093864662645; w_quad[1] = 0.3607615730481386;
            x_quad[2] = -0.2386191860831969; w_quad[2] = 0.4679139345726910;
            x_quad[3] =  0.2386191860831969; w_quad[3] = 0.4679139345726910;
            x_quad[4] = -0.9324695142031521; w_quad[4] = 0.1713244923791704;
            x_quad[5] =  0.9324695142031521; w_quad[5] = 0.1713244923791704;
        }
        else if (num_quadrature_points == 7)
        {
            x_quad[0] =  0.0000000000000000; w_quad[0] = 0.4179591836734694;
            x_quad[1] = -0.4058451513773972; w_quad[1] = 0.3818300505051189;
            x_quad[2] =  0.4058451513773972; w_quad[2] = 0.3818300505051189;
            x_quad[3] = -0.7415311855993945; w_quad[3] = 0.2797053914892766;
            x_quad[4] =  0.7415311855993945; w_quad[4] = 0.2797053914892766;
            x_quad[5] = -0.9491079123427585; w_quad[5] = 0.1294849661688697;
            x_quad[6] =  0.9491079123427585; w_quad[6] = 0.1294849661688697;
        }
        else if (num_quadrature_points == 8)
        {
            x_quad[0] = -0.1834346424956498; w_quad[0] = 0.3626837833783620;
            x_quad[1] =  0.1834346424956498; w_quad[1] = 0.3626837833783620;
            x_quad[2] = -0.5255324099163290; w_quad[2] = 0.3137066458778873;
            x_quad[3] =  0.5255324099163290; w_quad[3] = 0.3137066458778873;
            x_quad[4] = -0.7966664774136267; w_quad[4] = 0.2223810344533745;
            x_quad[5] =  0.7966664774136267; w_quad[5] = 0.2223810344533745;
            x_quad[6] = -0.9602898564975363; w_quad[6] = 0.1012285362903763;
            x_quad[7] =  0.9602898564975363; w_quad[7] = 0.1012285362903763;
        }
        else if (num_quadrature_points == 9)
        {
            x_quad[0] =  0.0000000000000000; w_quad[0] = 0.3302393550012598;
            x_quad[1] = -0.8360311073266358; w_quad[1] = 0.1806481606948574;
            x_quad[2] =  0.8360311073266358; w_quad[2] = 0.1806481606948574;
            x_quad[3] = -0.9681602395076261; w_quad[3] = 0.0812743883615744;
            x_quad[4] =  0.9681602395076261; w_quad[4] = 0.0812743883615744;
            x_quad[5] = -0.3242534234038089; w_quad[5] = 0.3123470770400029;
            x_quad[6] =  0.3242534234038089; w_quad[6] = 0.3123470770400029;
            x_quad[7] = -0.6133714327005904; w_quad[7] = 0.2606106964029354;
            x_quad[8] =  0.6133714327005904; w_quad[8] = 0.2606106964029354;
        }
        else if (num_quadrature_points == 10)
        {
            x_quad[0] = -0.1488743389816312; w_quad[0] = 0.2955242247147529;
            x_quad[1] =  0.1488743389816312; w_quad[1] = 0.2955242247147529;
            x_quad[2] = -0.4333953941292472; w_quad[2] = 0.2692667193099963;
            x_quad[3] =  0.4333953941292472; w_quad[3] = 0.2692667193099963;
            x_quad[4] = -0.6794095682990244; w_quad[4] = 0.2190863625159820;
            x_quad[5] =  0.6794095682990244; w_quad[5] = 0.2190863625159820;
            x_quad[6] = -0.8650633666889845; w_quad[6] = 0.1494513491505806;
            x_quad[7] =  0.8650633666889845; w_quad[7] = 0.1494513491505806;
            x_quad[8] = -0.9739065285171717; w_quad[8] = 0.0666713443086881;
            x_quad[9] =  0.9739065285171717; w_quad[9] = 0.0666713443086881;
        }
        else std::cout<< "*** Error:: Too many quadrature points are needed!!" << std::endl;
        
        double sum(0.0), err(0.0);
        for (size_t i = 0; i < num_quadrature_points; i++)
        {
            double xx = 0.5* (xmax - xmin) * x_quad[i] + 0.5 * (xmax + xmin);
            double f_low = interpolate_lagrange(num_data_points - 1, data_x, data_f, xx);
            double f_high = interpolate_lagrange(num_data_points, data_x, data_f, xx);
            sum += w_quad[i] * f_high;
            err += w_quad[i] * fabs(f_high - f_low);
        }
        delete[] x_quad; delete[] w_quad;
        err_est = 0.5 * (xmax - xmin) * err;
        return 0.5 * (xmax - xmin) * sum;
    }
    // ----------------------------------------------------------------
    void NonDRKDDarts::improve_parent_evaluation(size_t parent)
    {
        // first segment left to first child
        size_t num_samples = _sample_num_children[parent];
        size_t* samples = new size_t[num_samples];
        get_children(parent, samples);
        
        double tot_interp_err(0.0), tot_ev_err(0.0);
        
        for (size_t i = 0; i < num_samples; i++)
        {
            size_t sample_index = samples[i];
            tot_interp_err+= _sample_left_interp_err[sample_index];
            tot_interp_err+= _sample_right_interp_err[sample_index];
            tot_ev_err+= _sample_left_ev_err[sample_index];
            tot_ev_err+= _sample_right_ev_err[sample_index];
        }
        
        //if (parent == 0)
        //std::cout<< "*** surrogate = " << parent << " , interp_err = " << tot_interp_err << " , ev_err = " << tot_ev_err << std::endl;
        
        double tot_err = tot_interp_err + tot_ev_err;
        
        double u = generate_a_random_number();
        
        if (u < tot_interp_err / tot_err)
        {
            //::cout<< "   -> improving interpolation ... " << std::endl;
            
            // improve interpolation error
            u = generate_a_random_number();
            double sum = 0.0;
            for (size_t i = 0; i < num_samples; i++)
            {
                size_t sample_index = samples[i];
                size_t idim = _sample_dim[sample_index];
                sum += _sample_left_interp_err[sample_index]/tot_interp_err;
                sum +=  _sample_right_interp_err[sample_index]/tot_interp_err;
                
                if (u < sum + 1E-10)
                {
                    // refine cell to improve interpolation error
                    double left_err = _sample_left_interp_err[sample_index];
                    double right_err =  _sample_right_interp_err[sample_index];
                    u = generate_a_random_number();
                    double total_cell_err = left_err + right_err;
                    if (u < left_err / total_cell_err)
                    {
                        // refine left
                        size_t left = _sample_left[sample_index];
                        double xmin = (5.0/6.0) * _xmin[idim] + (1.0/6.0) * _sample_coord[sample_index];
                        double xmax = 0.5 * _xmin[idim] + 0.5 *  _sample_coord[sample_index];
                        if (left > 0)
                        {
                            xmin = (2.0/3.0) * _sample_coord[left] + (1.0/3.0) * _sample_coord[sample_index];
                            xmax = (1.0/3.0) * _sample_coord[left] + (2.0/3.0) * _sample_coord[sample_index];
                        }
                        u = generate_a_random_number();
                        double position = xmin + u * (xmax - xmin);
                        create_new_sample(parent, left, sample_index, position);
                    }
                    else
                    {
                        // refine right
                        size_t right = _sample_right[sample_index];
                        double xmin = 0.5 *  _sample_coord[sample_index] +  0.5 * _xmax[idim];
                        double xmax = (1.0/6.0) *_sample_coord[sample_index] + (5.0/6.0) * _xmax[idim];
                        
                        if (right > 0)
                        {
                            xmin = (2.0/3.0) * _sample_coord[sample_index] + (1.0/3.0) * _sample_coord[right];
                            xmax = (1.0/3.0) * _sample_coord[sample_index] + (2.0/3.0) * _sample_coord[right];
                        }
                        u = generate_a_random_number();
                        double position = xmin + u * (xmax - xmin);
                        create_new_sample(parent, sample_index, right, position);
                    }
                    break;
                }
            }
        }
        else
        {
            // improve evaluation error
            u = generate_a_random_number();
            double sum = 0.0;
            for (size_t i = 0; i < num_samples; i++)
            {
                size_t sample_index = samples[i];
                size_t idim = _sample_dim[sample_index];
                sum += _sample_left_ev_err[sample_index]/tot_ev_err;
                sum += _sample_right_ev_err[sample_index]/tot_ev_err;
                
                if (u < sum + 1E-10)
                {
                    // improve evaluation of this sample
                    improve_parent_evaluation(sample_index);
                    break;
                }
            }
        }
        evaluate_1d_surrogate(parent);
        
        delete[] samples;
    }
    // ----------------------------------------------------------------
    void NonDRKDDarts::evaluate_1d_surrogate(size_t parent)
    {
        size_t num_samples = _sample_num_children[parent];
        size_t* samples = new size_t[num_samples];
        get_children(parent, samples);
        
        size_t num_neighbors;
        size_t* neighbors = new size_t[_max_num_neighbors];
        double* data_x    = new double[_max_num_neighbors + 1];
        double* data_f    = new double[_max_num_neighbors + 1];
        
        bool rc_left(true);
        size_t rc_seed;
        
        double integ(0.0), total_interp_err(0.0), discont_err(0.0), rc(0.0);
        for (size_t i = 0; i < num_samples; i++)
        {
            size_t sample_index = samples[i];
            get_neighbors(samples[i], num_neighbors, neighbors);
            for (size_t j = 0; j < num_neighbors; j++)
            {
                size_t neighbor_index = neighbors[j];
                data_x[j] = _sample_coord[neighbor_index];
                data_f[j] = _sample_value[neighbor_index];
            }
            data_x[num_neighbors] = _sample_coord[sample_index];
            data_f[num_neighbors] = _sample_value[sample_index];
            
            size_t idim = _sample_dim[sample_index];
            double xseed = _sample_coord[sample_index];
            double xmin = _xmin[idim]; double xmax = _xmax[idim];
            if (_sample_left[sample_index] > 0)
            {
                size_t left = _sample_left[sample_index];
                xmin = 0.5 * (_sample_coord[sample_index] + _sample_coord[left]);
            }
            if (_sample_right[sample_index] > 0)
            {
                size_t right = _sample_right[sample_index];
                xmax = 0.5 * (_sample_coord[sample_index] + _sample_coord[right]);
            }
            
            if (num_neighbors == 0)
            {
                _sample_left_interp_err[sample_index] = 0.0;
                _sample_right_interp_err[sample_index] = 0.0;
                integ+= _sample_value[sample_index] * (xmax - xmin);
            }
            else
            {
                integ += integrate_legendre_gauss(xmin, xseed, num_neighbors + 1, data_x, data_f, _sample_left_interp_err[sample_index]);
                integ += integrate_legendre_gauss(xseed, xmax, num_neighbors + 1, data_x, data_f, _sample_right_interp_err[sample_index]);
            }
            
            total_interp_err+= _sample_left_interp_err[sample_index];
            total_interp_err+= _sample_right_interp_err[sample_index];
            
            // adding err due to discontinuity
            size_t left = _sample_left[sample_index];
            if (left > 0)
            {
                double dF = fabs(_sample_value[sample_index] - _sample_value[left]);
                if (dF > _discont_jump_threshold)
                {
                    _sample_left_interp_err[sample_index] += dF * (xseed - xmin);
                    total_interp_err += dF * (xseed - xmin);
                    discont_err += dF * (xseed - xmin);
                }
            }
            
            size_t right = _sample_right[sample_index];
            if (right > 0)
            {
                double dF = fabs(_sample_value[sample_index] - _sample_value[right]);
                if (dF > _discont_jump_threshold)
                {
                    _sample_right_interp_err[sample_index] += dF * (xmax - xseed);
                    total_interp_err += dF * (xmax - xseed);
                    discont_err += dF * (xmax - xseed);
                }
            }
            
            if (xseed - xmin > rc)
            {
                rc = xseed - xmin; rc_left = true;
                rc_seed = sample_index;
            }
            if (xmax - xseed > rc)
            {
                rc = xmax - xseed; rc_left = false;
                rc_seed = sample_index;
            }
        }
        if (total_interp_err < 1E-10) // zero error estimate
        {
            if (rc_left) _sample_left_interp_err[rc_seed] = rc;
            else         _sample_right_interp_err[rc_seed] = rc;
        }
        else if (fabs(discont_err - total_interp_err) < 1E-10 && rc > 1000 * discont_err)  // All error is due to discontinuity
        {
            if (rc_left) _sample_left_interp_err[rc_seed] = rc;
            else         _sample_right_interp_err[rc_seed] = rc;
        }
        
        _sample_value[parent] = integ;
        estimate_surrogate_evaluation_err(parent);
        
        delete[] samples;
        delete[] neighbors;
        delete[] data_x;
        delete[] data_f;
    }
    // ----------------------------------------------------------------
    void NonDRKDDarts::compute_response(double* x)
    {
        RealVector newX(_num_dim);
        for (size_t idim = 0; idim < _num_dim; idim++) newX[idim] = x[idim];
        
        iteratedModel.continuous_variables(newX);
        // bypass the surrogate model to evaluate the underlying truth model
        iteratedModel.surrogate_response_mode(BYPASS_SURROGATE);
        iteratedModel.evaluate();
        
        // TODO: later, generalize DataFitSurrModel to automatically
        // cache the points when in bypass mode
        add_surrogate_data(iteratedModel.current_variables(),
                           iteratedModel.current_response());
    }
    // ----------------------------------------------------------------
    void NonDRKDDarts::add_surrogate_data(const Variables& vars, const Response& resp)
    {
        // TODO: have the surrogate model instead cache the data when in
        // bypass mode
        IntResponsePair tmp_pair((int)0, resp);
        bool rebuild_flag = false;
        iteratedModel.append_approximation(vars, tmp_pair, rebuild_flag);
    }
    // ----------------------------------------------------------------
    void NonDRKDDarts::build_surrogate()
    {
        // TODO: do we just send all points here? or append one at a time?
        iteratedModel.build_approximation();
        // change surrogate to evaluate the surrogate model
        iteratedModel.surrogate_response_mode(AUTO_CORRECTED_SURROGATE);
    }
    // ----------------------------------------------------------------
    Real NonDRKDDarts::eval_surrogate(size_t function_index, double* x)
    {
        // this copy could be moved outside the loop for memory efficiency
        for (size_t vi = 0; vi < numContinuousVars; ++vi)
            iteratedModel.continuous_variable(x[vi], vi);
        // TODO: use active_set_vector for efficiency if you truly only
        // need 1 response function?
        
        iteratedModel.evaluate();
        
        const RealVector& fn_vals = iteratedModel.current_response().function_values();
        
        return fn_vals[function_index];
    }
    // ----------------------------------------------------------------
    void NonDRKDDarts::estimate_rkd_surrogate()
    {
        clock_t start_time, end_time; double cpu_time, total_time(0.0);
        
        start_time = clock();
        
        // Build a surrogate over the final sample set. Type: as specified in the input spec
        build_surrogate();
        
        end_time = clock();
        cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
        
        std::cout.precision(4);
        std::cout <<  "<<<<< RKD :: Surrogate model built in " << std::fixed << cpu_time << " seconds." << std::endl<< std::endl;
        
        // evaluate the surrogate for the given function
        
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
            _I_RKD[resp_fn_count]= 0.0;
        
        start_time = clock();
        
        double rkd_exact = 0.0;
        
        double* tmp_pnt = new double[_num_dim];
        for (size_t isample = 0; isample < emulatorSamples; isample++)
        {
            // sample a random point from the domain
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                tmp_pnt[idim] = _xmin[idim] + generate_a_random_number() * (_xmax[idim] - _xmin[idim]);
            }
            
            double f_exact = 0.0;
            if (_eval_error)
            {
                f_exact = f_true(tmp_pnt);
                rkd_exact+= f_exact;
            }
            
            for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
            {
                // evaluate sample point using surrogate
                double surrogate_value = eval_surrogate(resp_fn_count, tmp_pnt);
                _I_RKD[resp_fn_count]+= surrogate_value;
            }
        }
        end_time = clock();
        cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
        
        std::cout <<  "<<<<< RKD :: Surrogate model evaluated in " << std::fixed << cpu_time << " seconds." << std::endl << std::endl;
        
        double vol(1.0);
        for (size_t idim = 0; idim < _num_dim; idim++) vol *= _xmax[idim] - _xmin[idim];
        
        
        // Passing results to gloabl containers
        if (_eval_error)
        {
            rkd_exact*= vol/emulatorSamples;
        }
        
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            _I_RKD[resp_fn_count]*= vol/emulatorSamples;
        }
        
        if (_eval_error)
        {
            std::cout.precision(15);
            std::cout <<  "<<<<< RKD :: exact integration " << std::fixed << rkd_exact << std::endl;
            std::cout <<  "<<<<< RKD :: Surrogate error " << std::fixed << fabs((_I_RKD[0] - rkd_exact)) << std::endl;
        }
        delete[] tmp_pnt;
    }
    // ----------------------------------------------------------------
    double NonDRKDDarts::f_true(double* x)
    {
        if (_test_function == 1) // Smooth Herbie
        {
            double fval = 1.0;
            for (size_t idim = 0; idim < _num_dim; idim++)
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
            for (size_t idim = 0; idim < _num_dim; idim++)
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
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                double wcross = (1 + cos(2 * PI * x[idim])) / 2.0;
                fval *= wcross;
            }
            fval = pow(fval, 1.0/_num_dim);
            return fval;
        }
        if (_test_function == 4) // Circular Cone
        {
            double h = 0.0;
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                double dx = x[idim];
                h += dx * dx;
            }
            h = std::sqrt(h);
            return h;
        }
        
        return 0.0;
    }
    // ----------------------------------------------------------------
    void NonDRKDDarts::print_integration_results(std::ostream& s)
    {
        size_t width = write_precision+7, w2p2 = 2*width+2, w3p4 = 3*width+4;
        
        StringMultiArrayConstView uv_labels
        = iteratedModel.continuous_variable_labels();
        const StringArray& fn_labels = iteratedModel.response_labels();
        
        std::cout << std::endl;
        
        std::cout << "RKD Numerical Integration results using " << samples << " samples:" << std::endl << std::endl;

        for (size_t i = 0; i < numFunctions; i++) {
            s << "Estimated integral of " << fn_labels[i] << " = " << _I_RKD[i] << std::endl;
        }
    }
    ///////////////////////////////////////////////////////////////////
    
// --------------------------------------------------------------------
} // namespace Dakota

