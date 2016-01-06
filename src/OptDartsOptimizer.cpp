/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       OptDartsOptimizer
//- Description: Implementation of the OptDarts class
//- Owner:       Mohamed Ebeida 
//- Checked by:
//- Version: $Id$

#include "ProblemDescDB.hpp"

#include <algorithm>
#include <sstream>

#include "OptDartsOptimizer.hpp"

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

#ifndef E
#define E 2.7182818284590452353602874713526
#endif


namespace Dakota {

// Main Class: OptDartsOptimizer

OptDartsOptimizer::OptDartsOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model)
{     
     // load_parameters
     this->load_parameters(model);

     // Set Rnd Seed
     randomSeed = probDescDB.get_int("method.random_seed");
          
     // Set Max # of BB Evaluations
     maxBlackBoxEvals = probDescDB.get_int("method.max_function_evaluations");
          
     maxIterations = probDescDB.get_int("method.max_iterations");
     if (methodName == GENIE_OPT_DARTS) 
       use_DIRECT = false;
     else 
       use_DIRECT = true;
}

OptDartsOptimizer::OptDartsOptimizer(Model& model): Optimizer(GENIE_OPT_DARTS, model)
{
     // load_parameters
     this->load_parameters(model);
}


void OptDartsOptimizer::core_run()
{
    
    Cout << "I am now computing best values! " << '\n';
    
    size_t num_dim = numTotalVars;
    
    size_t budget = maxBlackBoxEvals;
    
    double* xmin = new double[num_dim];
    double* xmax = new double[num_dim];
    
    const RealVector&  lower_bounds = iteratedModel.continuous_lower_bounds();
    const RealVector&  upper_bounds = iteratedModel.continuous_upper_bounds();
    
    // default domain is for the Herbie Function
    for (size_t idim = 0; idim < num_dim; idim++)
    {
        xmin[idim] = lower_bounds[idim];
        xmax[idim] = upper_bounds[idim];
    }
    
    // to be deleted
    double TOL = 1E-6;
    size_t problem_index = 2;
    //bool use_DIRECT = false;
    Cout << "use_DIRECT " << use_DIRECT << '\n'; 
    double fw_MC = 1E6;
    double fb_MC = -1E6;
    
    opt_darts_execute(num_dim, budget, xmin, xmax, TOL, problem_index, fw_MC, fb_MC);
    
    delete[] xmin;
    delete[] xmax;
    
    
    //at the end, you need to return the best variables and corresponding responses
    //see NOMADOptimizer as an example
    // for example,
    Cout << " value of global minima =  " << _fb << '\n';
    Cout << " location of global minima: " << '\n';
    
    RealVector contVars(numContinuousVars);
    for(size_t idim = 0; idim < _num_dim; idim++)
    {
        contVars[idim] = _x[_ib][idim]; // contVars was not declared in this scope
        Cout << _x[_ib][idim] << "," << '\n';
    }
    bestVariablesArray.front().continuous_variables(contVars);
    
    // at the end, need to return the bestResponseArray with the best
    // function values; unless DB lookup is needed
    if (!localObjectiveRecast) {
      RealVector best_fns(numFunctions);
      best_fns[0] = _fb;
      bestResponseArray.front().function_values(best_fns);
    }
    Cout << "In body of core_run\n";
    
    opt_darts_terminate();
}


void OptDartsOptimizer::load_parameters(Model &model)
{
     numTotalVars = numContinuousVars +
                    numDiscreteIntVars + numDiscreteRealVars;
    
     // Define Input Types and Bounds
     
     //NOMAD::Point _initial_point (numTotalVars);
     //NOMAD::Point _upper_bound (numTotalVars);
     //NOMAD::Point _lower_bound (numTotalVars);
     
     const RealVector& initial_point_cont = model.continuous_variables();
     const RealVector& lower_bound_cont = model.continuous_lower_bounds();
     const RealVector& upper_bound_cont = model.continuous_upper_bounds();

     const IntVector& initial_point_int = model.discrete_int_variables();
     const IntVector& lower_bound_int = model.discrete_int_lower_bounds();
     const IntVector& upper_bound_int = model.discrete_int_upper_bounds();

     const RealVector& initial_point_real = model.discrete_real_variables();
     const RealVector& lower_bound_real = model.discrete_real_lower_bounds();
     const RealVector& upper_bound_real = model.discrete_real_upper_bounds();

     const BitArray& int_set_bits = iteratedModel.discrete_int_sets();
     const IntSetArray& initial_point_set_int = iteratedModel.discrete_set_int_values();
     const RealSetArray& initial_point_set_real = iteratedModel.discrete_set_real_values();

     // Define Output Types
     // responses.
     //		objective_functions
     //		nonlinear_inequality_constraints
     //		nonlinear_equality_constraints

     const RealVector& nln_ineq_lwr_bnds
       = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
     const RealVector& nln_ineq_upr_bnds
       = iteratedModel.nonlinear_ineq_constraint_upper_bounds();
     const RealVector& nln_eq_targets
       = iteratedModel.nonlinear_eq_constraint_targets();

}
    
    double OptDartsOptimizer::opt_darts_f()
    {

        RealVector newX(_num_dim);
        for (size_t idim = 0; idim < _num_dim; idim++) newX[idim] = _dart[idim];
        
        iteratedModel.continuous_variables(newX);
        iteratedModel.evaluate();
        
        
        double fval = 0.0;
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            double f = iteratedModel.current_response().function_value(resp_fn_count);
            fval += f;
        }
        
        return fval;


        // test functions
        if (_problem_index == 0)
        {
            double s = 0.0;
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                double dx = _dart[idim];
                s += dx * dx;
            }
            //fval = sqrt(fval);
            return -pow(E, -0.5 * s);
        }
        if (_problem_index == 1)
        {
            double fval = 1.0;
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                double t = 0.5 * (1.0 + cos(2 * PI * _dart[idim]));
                fval *= t;
            }
            fval = pow(fval, 1.0 / double(_num_dim));
            return fval;
        }
        if (_problem_index == 2)
        {
            double fval = 1.0;
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                double xm = _dart[idim] - 1.0;
                double xp = _dart[idim] + 1.0;
                double wherb = pow(E, - xm * xm) + pow(E, -0.8 * xp * xp) - 0.05 * sin(8 * (_dart[idim] + 0.1));
                fval *= wherb;
            }
            fval = -fval;
            return fval;
       }
        if (_problem_index == 3)
        {
            double fval = 1.0;
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                double xm = _dart[idim] - 1.0;
                double xp = _dart[idim] + 1.0;
                double wherb = pow(E, - xm * xm) + pow(E, -0.8 * xp * xp);// - 0.05 * sin(8 * (x[idim] + 0.1));
                fval *= wherb;
            }
            fval = -fval;
            return fval;
        }
        if (_problem_index == 4)
        {
            double sum(0.0);
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                double gi;
                double xi = _dart[idim];
                double ai = _alpha_Deceptive[idim];
                
                if      (xi < 0.8 * ai)           gi = -xi / ai + 0.8;
                else if (xi < ai)                 gi = 5 * xi / ai - 4.0;
                else if (xi < (1 + 4 * ai) / 5)   gi = 5 * (xi - ai) / (ai - 1.0);
                else                              gi = (xi - 1) / (1 - ai);
                
                sum+= gi;
            }
            sum /= _num_dim;
            return -sum * sum;
        }
        if (_problem_index == 5)
        {
            double a = 20.0; double b = 0.2; double c = 2 * PI;
            double s1(0.0), s2(0.0);
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                s1 += _dart[idim] * _dart[idim];
                s2 += cos(c * _dart[idim]);
            }
            return a - a* pow(E, -b * sqrt(s1/_num_dim)) + E - pow(E, s2 / _num_dim);
        }
        if (_problem_index == 6)
        {
            double s(0.0);
            for (size_t idim = 0; idim < _num_dim - 1; idim++)
            {
                s += _dart[idim] * _dart[idim];
                s += 2 * _dart[idim + 1] * _dart[ idim + 1];
                s -= 0.3 * cos(3 * PI * _dart[idim]);
                s -= 0.4 * cos(4 * PI * _dart[idim + 1]);
                s += 0.7;
            }
            return s;
        }
        if (_problem_index == 7)
        {
            double s1(0.0), s2(0.0);
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                s1 += cos(5 * PI * _dart[idim]);
                s2 += _dart[idim] * _dart[idim];
            }
            return -0.1 * s1 - s2;
        }
        return 0.0;
    }

    
    
    void OptDartsOptimizer::opt_darts_execute(size_t num_dim, size_t budget, double* xmin, double* xmax, double TOL, size_t problem_index, double fw_MC, double fb_MC)
    {
        initiate_random_generator(randomSeed);
        
        _num_dim = num_dim;
        _budget = budget;
        _problem_index = problem_index;
        _use_opt_darts = !use_DIRECT;
        
        _estimate_K = false;
        
        opt_darts_initiate(xmin, xmax);
        
        if (problem_index == 0 || problem_index == 1 || problem_index == 6 || problem_index == 7)
        {
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                _dart[idim] = xmax[idim];
            }
            fw_MC = opt_darts_f();
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                _dart[idim] = xmin[idim];
            }
            fb_MC = opt_darts_f();
        }
        
        _fb_MC = fb_MC; _fw_MC = fw_MC;
        
        double err;
        size_t count(0);
        while (_num_samples < _budget)
        {
            //err = fabs(_fb[0] - _fb_MC) / fabs(_fw_MC - _fb_MC);
            //if ( err < TOL) break;
            
            // pick candidate
            size_t icandidate = opt_darts_pick_candidate(0);
            
            count++;
            if (count == 100)
            {
                /*
                 std::cout<< "Number of samples so far = " << _num_samples << std::endl;
                 std::cout<< "Number of corners = " << _num_corners << std::endl;
                 std::cout<< "first corner = " << _corners[0] << std::endl;
                 std::cout<< "size of first corner = " << _h[_corners[0]] << std::endl;
                 std::cout<< "Value of first corner = " << _f[_corners[0]][0] << std::endl;
                 std::cout<< "best point = " << _ib << std::endl;
                 std::cout<< "size of best point = " << _h[_ib] << std::endl;
                 std::cout<< "Value of best point = " << _f[_ib][0] << std::endl;
                 */
                count  = 0;
            }
            
            if (icandidate == _budget)
            {
                std::cout<< "No candidate is found .. quitting!" << std::endl;
                break;
            }
            
            
            if (use_DIRECT)
            {
                DIRECT_sample_from_candidate_neighborhood(icandidate);
                continue;
            }
            
            // sample from candidate neighborhood (Opt-dart)
            opt_darts_sample_from_candidate_neighborhood(icandidate, 0);
            
        }
        
        std::cout<< "Method has converged after " << _num_samples << " function evaluations" << std::endl;
        //std::cout<< "worst value (MC) = " << std::setprecision(10) << _fw_MC << std::endl;
        //std::cout<< "best value (MC) = " << std::setprecision(10) << _fb_MC << std::endl;
        std::cout<< "best value = " << _fb << std::endl;
        //std::cout<< "error = " << err << std::endl;
        
        if (_problem_index == 4)
        {
            std::cout << "Deceptive alpha parameters: \n" << _alpha_Deceptive[0];
            for (size_t idim = 1; idim < _num_dim; idim++) std::cout << " , \n" << _alpha_Deceptive[idim];
        }
        
        if (_num_dim == 2) 
        {
            std::cout << "... plotting samples" << std::endl;
            opt_darts_plot_discs_2d(_num_samples);
        }
        
        

    }
    
    void OptDartsOptimizer::opt_darts_initiate(double* xmin, double* xmax)
    {
        
        _alpha_Deceptive = new double [_num_dim];
        for (size_t idim = 0; idim < _num_dim; idim++) _alpha_Deceptive[idim] = 0.699719487; //0.5 + 0.2 * generate_a_random_number();
        
        _st = new double[_num_dim];
        _end = new double[_num_dim];
        _qH = new double[_num_dim];
        _nH = new double[_num_dim];
        _tmp_neighbors = new size_t[_budget];
        _ext_neighbors = new size_t[_budget];
        _tmp_point = new double[_num_dim];
        
        
        _epsilon = 0.001;
        _corner_index = 0;
        _num_corners = 0;
        _corners = new size_t[_budget];
        
        _num_samples = 0;
        _xmin = new double[_num_dim];
        _xmax = new double[_num_dim];
        _dart =new double[_num_dim];
        _x = new double*[_budget];
        _xc = new double*[_budget];
        _f = new double*[_budget];
        _K = new double*[_budget];
        _h = new double[_budget];
        _r = new double[_budget];
        
        _xm = new double*[_budget];
        _xp = new double*[_budget];
        _neighbors = new size_t*[_budget];
        for (size_t i = 0; i < _budget; i++)
        {
            _x[i] = 0;
            _xc[i] = 0;
            _f[i] = 0;
            _K[i] = 0;
            _h[i] = 0.0;
            _r[i] = 0.0;
            
            _xm[i] = new double[_num_dim];
            _xp[i] = new double[_num_dim];
            _neighbors[i] = 0;
        }
        
        _diag = 0.0;
        for (size_t idim = 0; idim < _num_dim; idim++)
        {
            double dx = xmax[idim] - xmin[idim];
            _diag += dx * dx;
        }
        _diag = sqrt(_diag);
        
        // initiate sampling with a point in the domain
        for (size_t idim = 0; idim < _num_dim; idim++)
        {
            _xmin[idim] = xmin[idim];
            _xmax[idim] = xmax[idim];
            double u(0.5);
            if (_use_opt_darts) u = generate_a_random_number();
            else
            {
                // 1st DIRECT Cell
                _xm[0][idim] = _xmin[idim];
                _xp[0][idim] = _xmax[idim];
            }
            _dart[idim] = xmin[idim] + u * (xmax[idim] - xmin[idim]);
        }
        
        _num_ext_neighbors = 0;
        
        // Add first point
        opt_darts_add_dart();
        // update K and h based on Vornoi cells
        opt_darts_update_K_h_approximate_Voronoi(0);
    }
    
    size_t OptDartsOptimizer::opt_darts_pick_candidate(size_t ifunc)
    {
        
        if (_corner_index < _num_corners)
        {
            size_t cand = _corners[_corner_index];
            _corner_index++;
            return cand;
        }
        
        // retrieve corners of convex hull
        _num_corners = 0;
        double ho = 0.0;
        double fo = _fb - _epsilon * fabs(_fb);
        
        double dh(1E-10);
        
        while (true)
        {
            double min_slope(DBL_MAX); size_t next_corner(_budget);
            double hnext, fnext; bool found(false);
            for (size_t isample = 0; isample < _num_samples; isample++)
            {
                double h = _h[isample];
                if (h < ho + dh) continue;
                double f = _f[isample][ifunc];
                
                size_t num_neighbors(_neighbors[isample][0]);
                
                if (_estimate_K && num_neighbors > 2 * _num_dim)
                {
                    double hK = (f - _fb);
                    if (hK > h *  _K[isample][ifunc])
                    {
                        // Current cell should be discarded according to estimate of K from neighbors
                        // which could be inaccurate
                        double u = generate_a_random_number();
                        if (u > 0.001) continue;
                    }
                }
                
                double slope = (f - fo) / (h - ho);
                if (slope < min_slope)
                {
                    min_slope = slope;
                    next_corner = isample;
                    hnext = h; fnext = f;
                    found = true;
                }
            }
            if (!found) break;
            
            _corners[_num_corners] = next_corner;
            _num_corners++;
            ho = hnext;
            fo = fnext;
        }
        if (_num_corners == 0) return _budget;
        
        
        
        /*
         int bug(0);
         opt_darts_plot_hull_2d(_corners[0], 0);
         opt_darts_plot_discs_2d(_corners[0]);
         bug++;
         */
        
        _corner_index = 1;
        return _corners[0];
    }
    
    void OptDartsOptimizer::retrieve_extended_neighbors(size_t icandidate)
    {
        _num_ext_neighbors = 2;
        _ext_neighbors[0] = icandidate;
        _ext_neighbors[1] = _num_samples - 1;
        size_t num_neighbors = _neighbors[icandidate][0];
        for (size_t i = 1; i <= num_neighbors; i++)
        {
            size_t isample = _neighbors[icandidate][i];
            
            size_t n = _neighbors[isample][0];
            for (size_t j = 0; j <= n; j++)
            {
                size_t jsample =  _neighbors[isample][j];
                if (j == 0) jsample = isample;
                
                bool found(false);
                for (size_t k = 0; k < _num_ext_neighbors; k++)
                {
                    if (_ext_neighbors[k] == jsample)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    _ext_neighbors[_num_ext_neighbors] = jsample;
                    _num_ext_neighbors++;
                }
            }
        }
    }
    
    void OptDartsOptimizer::opt_darts_sample_from_candidate_neighborhood(size_t icandidate, size_t ifunc)
    {
        
        // simply add furthest witness
        for (size_t idim = 0; idim < _num_dim; idim++) _dart[idim] =  _xc[icandidate][idim];
		
        opt_darts_add_dart();
        
        retrieve_extended_neighbors(icandidate);
        
        opt_darts_update_K_h_approximate_Voronoi(_num_samples - 1);
		
        opt_darts_update_K_h_approximate_Voronoi(icandidate);
        
        size_t num_neighbors = _neighbors[_num_samples - 1][0];
        for (size_t i = 1; i <= num_neighbors; i++)
        {
            size_t isample = _neighbors[_num_samples - 1][i];
            retrieve_extended_neighbors(isample);
            opt_darts_update_K_h_approximate_Voronoi(isample);
        }
    }
    
    void OptDartsOptimizer::DIRECT_sample_from_candidate_neighborhood(size_t icandidate)
    {
        double dx_max(0.0); size_t dir(0);
        for (size_t idim = 0; idim < _num_dim; idim++)
        {
            double dx = _xp[icandidate][idim] - _xm[icandidate][idim];
            if (dx > dx_max)
            {
                dx_max = dx;
                dir = idim;
            }
        }
        
        // add two samples in the direction of dir
        for (size_t idim = 0; idim < _num_dim; idim++)
        {
            _dart[idim] = _x[icandidate][idim];
            _xm[_num_samples][idim] = _xm[icandidate][idim];
            _xp[_num_samples][idim] = _xp[icandidate][idim];
        }
        _dart[dir] -= dx_max / 3.0;
        _xp[_num_samples][dir] = _xm[_num_samples][dir] + dx_max / 3.0;
        _xm[icandidate][dir] = _xp[_num_samples][dir];
        
        // update h and r for icandidate
        double diag(0.0), min_dx(DBL_MAX);
        for (size_t idim = 0; idim < _num_dim; idim++)
        {
            double dx = _xp[icandidate][idim] - _xm[icandidate][idim];
            if (dx < min_dx) min_dx = dx;
            diag += dx * dx;
        }
        _r[icandidate] = 0.5 * min_dx;
        _h[icandidate] = 0.5 * sqrt(diag);
        
        diag = 0.0; min_dx = DBL_MAX;
        for (size_t idim = 0; idim < _num_dim; idim++)
        {
            double dx = _xp[_num_samples][idim] - _xm[_num_samples][idim];
            if (dx < min_dx) min_dx = dx;
            diag += dx * dx;
        }
        _r[_num_samples] = 0.5 * min_dx;
        _h[_num_samples] = 0.5 * sqrt(diag);
        
        opt_darts_add_dart();
        
        if (_num_samples == _budget) return;
        
        // add two samples in the direction of dir
        for (size_t idim = 0; idim < _num_dim; idim++)
        {
            _dart[idim] = _x[icandidate][idim];
            _xm[_num_samples][idim] = _xm[icandidate][idim];
            _xp[_num_samples][idim] = _xp[icandidate][idim];
        }
        _dart[dir] += dx_max / 3.0;
        _xm[_num_samples][dir] = _xp[_num_samples][dir] - dx_max / 3.0;
        _xp[icandidate][dir] = _xm[_num_samples][dir];
        
        // update h and r for icandidate
        diag = 0.0; min_dx = DBL_MAX;
        for (size_t idim = 0; idim < _num_dim; idim++)
        {
            double dx = _xp[icandidate][idim] - _xm[icandidate][idim];
            if (dx < min_dx) min_dx = dx;
            diag += dx * dx;
        }
        _r[icandidate] = 0.5 * min_dx;
        _h[icandidate] = 0.5 * sqrt(diag);
        
        diag = 0.0; min_dx = DBL_MAX;
        for (size_t idim = 0; idim < _num_dim; idim++)
        {
            double dx = _xp[_num_samples][idim] - _xm[_num_samples][idim];
            if (dx < min_dx) min_dx = dx;
            diag += dx * dx;
        }
        _r[_num_samples] = 0.5 * min_dx;
        _h[_num_samples] = 0.5 * sqrt(diag);
        
        opt_darts_add_dart();
    }
    
    void OptDartsOptimizer::opt_darts_add_dart()
    {
        // add new sample
        _x[_num_samples] = new double[_num_dim];
        _xc[_num_samples] = new double[_num_dim];
        for (size_t idim = 0; idim < _num_dim; idim++) _x[_num_samples][idim] = _dart[idim];
        
        // evaluate function at sample
        
        _fval = opt_darts_f();
        
        
        if (_num_samples == 0 || _fval < _fb)
        {
            _fb = _fval;
            _ib = _num_samples;
                
            
        }
        if (_num_samples == 0 || _fval > _fw) _fw = _fval;
                
        _f[_num_samples] = new double[numFunctions];
        _K[_num_samples] = new double[numFunctions];
        _neighbors[_num_samples] = new size_t[1];
        _neighbors[_num_samples][0] = 0;
        _K[_num_samples][0] = 0.0001;
        
        for (size_t resp_fn_count = 0; resp_fn_count < numFunctions; resp_fn_count++)
        {
            _f[_num_samples][resp_fn_count] = iteratedModel.current_response().function_value(resp_fn_count);
        }

        _num_samples++;
        
    }
    
    void OptDartsOptimizer::opt_darts_update_K_h_approximate_Voronoi(size_t isample)
    {
        
        for (size_t ifunc = 0; ifunc < numFunctions; ifunc++) _K[isample][ifunc] = 0.0;
        
        _h[isample] = 0.0; _r[isample] = _diag;
        
        size_t num_neighbors = 0;
        for (size_t j = 0; j < _num_ext_neighbors; j++)
        {
            size_t jsample = _ext_neighbors[j];
            if (jsample == isample) continue;
            
            // retrieve mid-point
            for (size_t idim = 0; idim < _num_dim; idim++) _end[idim] = 0.5 * (_x[isample][idim] + _x[jsample][idim]);
            
            // retrieve closest sample point to mid point
            double dmin(_diag * _diag); double dst_j(0.0);
            for (size_t k = 0; k < _num_ext_neighbors; k++)
            {
                size_t ksample(_ext_neighbors[k]);
                
                double dst(0.0);
                for (size_t idim = 0; idim < _num_dim; idim++)
                {
                    double dx = _end[idim] - _x[ksample][idim];
                    dst += dx * dx;
                }
                
                if (dst < dmin) dmin = dst;
                
                if (ksample == jsample) dst_j = dst;
            }
            
            if (fabs(dmin - dst_j) < 1E-10)
            {
                dmin = sqrt(dmin);
                if (_r[isample] > dmin) _r[isample] = dmin; // update r
                
                // Add jsample to neighbors of isample
                _tmp_neighbors[num_neighbors] = jsample;
                num_neighbors++;
                
                if (_estimate_K)
                {
                    double dst(0.0);
                    for (size_t idim = 0; idim < _num_dim; idim++)
                    {
                        double dx = _x[isample][idim] - _x[jsample][idim];
                        dst += dx * dx;
                    }
                    dst = sqrt(dst);
                    for (size_t ifunc = 0; ifunc < numFunctions; ifunc++)
                    {
                        double fo = _f[isample][ifunc];
                        double fn = _f[jsample][ifunc];
                        double K = fabs(fo - fn) / dst;
                        if (_K[isample][ifunc] < K) _K[isample][ifunc] = K;
                        if (_K[jsample][ifunc] < K) _K[jsample][ifunc] = K;
                    }
                }
            }
        }
        
        for (size_t idim = 0; idim < _num_dim; idim++) _st[idim] = _x[isample][idim];
        
        // throw a line dart from isample
        size_t num_successive_misses(0), max_sucessive_misses(200);
        double min_spoke_length_sq(_diag);
        while(true)
        {
            sample_uniformly_from_unit_sphere_surface(_end, _num_dim);
            
            // scale line spoke to extend outside bounding box
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                _end[idim] *= _diag;
                _end[idim] += _st[idim];
            }
            
            // trim line spoke with domain boundaries
            double t_end(1.0);
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                if (_end[idim] > _xmax[idim])
                {
                    double t = (_xmax[idim] - _st[idim]) / (_end[idim] - _st[idim]);
                    if (t < t_end) t_end = t;
                }
                if (_end[idim] < _xmin[idim])
                {
                    double t = (_st[idim] - _xmin[idim]) / (_st[idim] - _end[idim]);
                    if (t < t_end) t_end = t;
                }
            }
            for (size_t idim = 0; idim < _num_dim; idim++) _end[idim] = _st[idim] + t_end * (_end[idim] - _st[idim]);
            
            bool boundary(true);
            size_t ineighbor(isample);
            for (size_t j = 0; j < _num_ext_neighbors; j++)
            {
                size_t jsample= _ext_neighbors[j];
                if (jsample == isample) continue;
                
                // trim line spoke via hyperplane between
                double norm(0.0);
                for (size_t idim = 0; idim < _num_dim; idim++)
                {
                    _qH[idim] = 0.5 * (_x[isample][idim] + _x[jsample][idim]);
                    _nH[idim] =  _x[jsample][idim] -  _x[isample][idim];
                    norm+= _nH[idim] * _nH[idim];
                }
                norm = 1.0 / sqrt(norm);
                for (size_t idim = 0; idim < _num_dim; idim++) _nH[idim] *= norm;
                
                if (trim_line_using_Hyperplane(_num_dim, _st, _end, _qH, _nH))
                {
                    ineighbor = jsample;
                    boundary = false;
                }
            }
            
            // isample and ineighbor are neighbors
            bool new_neighbor(true);
            for (size_t i = 0; i < num_neighbors; i++)
            {
                if (_tmp_neighbors[i] == ineighbor)
                {
                    new_neighbor = false;
                    break;
                }
            }
            
            // lenght of spoke
            double h(0.0);
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                double dx = _end[idim] - _st[idim];
                h += dx * dx;
            }
            if (!boundary && h < min_spoke_length_sq) min_spoke_length_sq = h;
            h = sqrt(h);
            if (h > _h[isample])
            {
                _h[isample] = h;
                for (size_t idim = 0; idim < _num_dim; idim++) _xc[isample][idim] = _end[idim];
            }
            
            if (new_neighbor && isample != ineighbor)
            {
                _tmp_neighbors[num_neighbors] = ineighbor;
                num_neighbors++;
                num_successive_misses = 0;
                
                if (_estimate_K)
                {
                    double dst(0.0);
                    for (size_t idim = 0; idim < _num_dim; idim++)
                    {
                        double dx = _x[isample][idim] - _x[ineighbor][idim];
                        dst += dx * dx;
                    }
                    dst = sqrt(dst);
                    
                    for (size_t ifunc = 0; ifunc < numFunctions; ifunc++)
                    {
                        double fo = _f[isample][ifunc];
                        double fn = _f[ineighbor][ifunc];
                        double K = fabs(fo - fn) / dst;
                        if (_K[isample][ifunc] < K) _K[isample][ifunc] = K;
                        if (_K[ineighbor][ifunc] < K) _K[ineighbor][ifunc] = K;
                    }
                }
            }
            else num_successive_misses++;
            
            if (num_successive_misses >= max_sucessive_misses) break;
        }
        
        if (_neighbors[isample] != 0) delete[] _neighbors[isample];
        _neighbors[isample] = new size_t[num_neighbors + 1];
        _neighbors[isample][0] = num_neighbors;
        for (size_t i = 0; i < num_neighbors; i++)
        {
            size_t jsample = _tmp_neighbors[i];
            _neighbors[isample][i + 1] = jsample;
            // make sure that isample is also connected to jsample
            size_t num = _neighbors[jsample][0];
            bool found = false;
            for (size_t j = 1; j <= num; j++)
            {
                size_t ksample = _neighbors[jsample][j];
                if (ksample == isample)
                {
                    found = true; break;
                }
            }
            if (!found)
            {
                size_t* tmp = new size_t[num + 2];
                tmp[0] = num + 1;
                for (size_t j = 1; j <= num; j++) tmp[j] = _neighbors[jsample][j];
                tmp[num+1] = isample;
                delete[] _neighbors[jsample];
                _neighbors[jsample] = tmp;
            }
        }
    }
    
    
    void OptDartsOptimizer::opt_darts_terminate()
    {
        
        delete[] _alpha_Deceptive;
        
        delete[] _st;
        delete[] _end;
        delete[] _tmp_point;
        delete[] _qH;
        delete[] _nH;
        delete[] _tmp_neighbors;
        delete[] _ext_neighbors;
        
        _num_corners = 0;
        
        
        delete[] _xmin;
        delete[] _xmax;
        delete[] _dart;
        for (size_t isample = 0; isample < _num_samples; isample++)
        {
            delete[] _x[isample];
            delete[] _f[isample];
            delete[] _K[isample];
            
            delete[] _xm[isample];
            delete[] _xp[isample];
            
            if (_neighbors[isample] != 0) delete[] _neighbors[isample];
        }
        delete[] _xm;
        delete[] _xp;
        
        delete[] _K;
        delete[] _h;
        delete[] _r;
        delete[] _neighbors;
    }
    
    
    void OptDartsOptimizer::opt_darts_plot_discs_2d(size_t icandidate)
    {
        size_t iflat = _num_samples;
        //iflat = 0;
        std::stringstream ss;
        ss << "opt_darts_" << iflat <<".ps";
        std::fstream file(ss.str().c_str(), std::ios::out);
        file << "%!PS-Adobe-3.0" << std::endl;
        file << "72 72 scale     % one unit = one inch" << std::endl;
        
        double Lx(_xmax[0] - _xmin[0]);
        double Ly(_xmax[1] - _xmin[0]);
        
        double scale_x, scale_y, scale;
        double shift_x, shift_y;
        
        scale_x = 6.5 / Lx;
        scale_y = 9.0 / Ly;
        
        if (scale_x < scale_y)
        {
            scale = scale_x;
            shift_x = 1.0 - _xmin[0] * scale;
            shift_y = 0.5 * (11.0 - Ly * scale) - _xmin[1] * scale;
        }
        else
        {
            scale = scale_y;
            shift_x = 0.5 * (8.5 - Lx * scale) - _xmin[0] * scale;
            shift_y = 1.0 - _xmin[1] * scale;
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
        
        file << "/blackseg      % stack: x1 y1 x2 y2" << std::endl;
        file << "{newpath" << std::endl;
        file << " moveto" << std::endl;
        file << " lineto" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " grestore" << std::endl;
        file << " 0 0 0 setrgbcolor" << std::endl;
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
        
        file << "/bgreenfcirc    % stack: x y r" << std::endl;
        file << "{0 360 arc" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " 0 1 1 setrgbcolor" << std::endl;
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
        
        file << "/quad      % stack: x1 y1 x2 y2 x3 y3 x4 y4" << std::endl;
        file << "{newpath" << std::endl;
        file << " moveto" << std::endl;
        file << " lineto" << std::endl;
        file << " lineto" << std::endl;
        file << " lineto" << std::endl;
        file << " closepath" << std::endl;
        file << " 0.0 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        // plot filled circles
        for (size_t index = 0; index < _num_samples; index++)
        {
            // filled discs
            file << _x[index][0] * scale << "  " << _x[index][1] * scale << "  " << _r[index] * scale << "  ";
            file << "greenfcirc"     << std::endl; // non-active disk
        }
        
        if (icandidate < _num_samples)
        {
            file << _x[icandidate][0] * scale << "  " << _x[icandidate][1] * scale << "  " << _r[icandidate] * scale << "  ";
            file << "redfcirc"     << std::endl; // active disk
        }
        
        // plot discs boundaries
        for (size_t index = 0; index < _num_samples; index++)
        {
            // filled discs
            file << _x[index][0] * scale << "  " << _x[index][1] * scale << "  " << _r[index] * scale << "  ";
            file << "circ"     << std::endl;
            
            /*
             // K circles
             double h = (_f[index][0] - _fb[0]) / _K[index][0];
             if (h > 1E-10)
             {
             file << _x[index][0] * scale << "  " << _x[index][1] * scale << "  " << h * scale << "  ";
             file << "circ"     << std::endl;
             }
             */
        }
        
        for (size_t index = 0; index < _num_samples; index++)
        {
            // plot vertex
            file << _x[index][0] * scale << "  " << _x[index][1] * scale << "  " << _r[index] * 0.05 * scale << "  ";
            file << "blackfcirc"     << std::endl;
        }
        
        file << _x[_ib][0] * scale << "  " << _x[_ib][1] * scale << "  " << 0.02 * scale << "  ";
        file << "bluefcirc"     << std::endl; // best answer
        
        
        double* st = new double[2];
        double* end = new double[2];
        double* nH = new double[2];
        double* qH = new double[2];
        for (size_t index = 0; index < _num_samples; index++)
        {
            
            if (_neighbors[index] == 0)
            {
                // plot boxes
                file << _xm[index][0] * scale << "  "  <<  _xm[index][1] * scale << "  ";
                file << _xp[index][0] * scale << "  "  <<  _xm[index][1] * scale << "  ";
                file << _xp[index][0] * scale << "  "  <<  _xp[index][1] * scale << "  ";
                file << _xm[index][0] * scale << "  "  <<  _xp[index][1] * scale << "  ";
                file << "quad"      << std::endl;
                continue;
            }
            
            // plot Vornoi Diagram
            size_t num_neighbors(_neighbors[index][0]);
            for (size_t i = 1; i <= num_neighbors; i++)
            {
                if (_neighbors[index][i] == index) continue;
                size_t i_index = _neighbors[index][i];
                
                double xm = 0.5 * (_x[index][0] + _x[i_index][0]);
                double ym = 0.5 * (_x[index][1] + _x[i_index][1]);
                
                double dy = _x[i_index][0] - _x[index][0];
                double dx = -(_x[i_index][1] - _x[index][1]);
                double dl = sqrt(dx * dx + dy * dy);
                dx /= dl; dy /= dl;
                
                st[0] = xm + _diag * dx; st[1] = ym + _diag * dy;
                end[0] = xm - _diag * dx; end[1] = ym - _diag * dy;
                
                //file << st[0] * scale << "  " << st[1] * scale << "  " << end[0] * scale << "  " << end[1] * scale << " blackseg" << std::endl;
                
                for (size_t j = 1; j <= num_neighbors; j++)
                {
                    if (_neighbors[index][j] == index) continue;
                    size_t j_index = _neighbors[index][j];
                    qH[0] = 0.5 * (_x[index][0] + _x[j_index][0]);
                    qH[1] = 0.5 * (_x[index][1] + _x[j_index][1]);
                    nH[0] = _x[j_index][0] - _x[index][0];
                    nH[1] = _x[j_index][1] - _x[index][1];
                    
                    trim_line_using_Hyperplane(2, st, end, qH, nH);
                }
                file << st[0] * scale << "  " << st[1] * scale << "  " << end[0] * scale << "  " << end[1] * scale << " blackseg" << std::endl;
            }
        }
        delete[] st;
        delete[] end;
        delete[] qH;
        delete[] nH;
        
        // dart
        if (false)
        {
            file << _dart[0] * scale << "  " << _dart[1] * scale << "  " << 0.02 * scale << "  ";
            file << "blackfcirc"     << std::endl;
        }
        
        double DX = _xmax[0] - _xmin[0];
        double DY = _xmax[1] - _xmin[1];
        
        if (true)
        {
            // plot domain boundaries
            file << (_xmin[0] - DX) * scale << "  "  <<  _xmin[1]        * scale << "  ";
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
        }
        
        // plot domain boundaries
        file << _xmin[0] * scale << "  " << _xmin[1] * scale << "  ";
        file << _xmax[0] * scale << "  " << _xmin[1] * scale << "  ";
        file << _xmax[0] * scale << "  " << _xmax[1] * scale << "  ";
        file << _xmin[0] * scale << "  " << _xmax[1] * scale << "  ";
        file << "quad_bold"      << std::endl;
        
        file << "showpage" << std::endl;
        
    }
    
    void OptDartsOptimizer::opt_darts_plot_hull_2d(size_t icandidate, size_t ifunc)
    {
        std::stringstream ss;
        ss << "opt_darts_convex_hull_" << _num_samples <<".ps";
        std::fstream file(ss.str().c_str(), std::ios::out);
        file << "%!PS-Adobe-3.0" << std::endl;
        file << "72 72 scale     % one unit = one inch" << std::endl;
        
        
        double xmin(0.0), xmax(_h[0]);
        double ymin(_f[0][ifunc]), ymax(_f[0][ifunc]);
        for (size_t index = 0; index < _num_samples; index++)
        {
            if (_h[index]> xmax) xmax = _h[index];
            if (_f[index][ifunc] > ymax) ymax = _f[index][ifunc];
            if (_f[index][ifunc] < ymin) ymin = _f[index][ifunc];
        }
        
        double Lx(xmax - xmin);
        double Ly(ymax - ymin);
        
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
        shift_x = 1.0 - xmin * scale_x;
        shift_y = 1.0 - ymin * scale_y;
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
        
        file << "/blackseg_bold      % stack: x1 y1 x2 y2" << std::endl;
        file << "{newpath" << std::endl; 
        file << " moveto" << std::endl;
        file << " lineto" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " grestore" << std::endl;
        file << " 0 0 0 setrgbcolor" << std::endl;
        file << " 0.02 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        file << "/blackseg      % stack: x1 y1 x2 y2" << std::endl;
        file << "{newpath" << std::endl; 
        file << " moveto" << std::endl;
        file << " lineto" << std::endl;
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " grestore" << std::endl;
        file << " 0 0 0 setrgbcolor" << std::endl;
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
        
        file << "/bgreenfcirc    % stack: x y r" << std::endl;
        file << "{0 360 arc" << std::endl; 
        file << " closepath" << std::endl;
        file << " gsave" << std::endl;
        file << " 0 1 1 setrgbcolor" << std::endl;
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
        
        file << "/quad      % stack: x1 y1 x2 y2 x3 y3 x4 y4" << std::endl;
        file << "{newpath" << std::endl; 
        file << " moveto" << std::endl;
        file << " lineto" << std::endl;
        file << " lineto" << std::endl;
        file << " lineto" << std::endl;
        file << " closepath" << std::endl;
        file << " 0.0 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        if (false)
        {
            // plot axis
            file << xmin * scale << "  " << ymin * scale << "  " << xmax * scale << "  " << ymin * scale << " blackseg_bold" << std::endl;
            file << xmin * scale << "  " << ymax * scale << "  " << xmax * scale << "  " << ymax * scale << " blackseg_bold" << std::endl;
            file << xmin * scale << "  " << ymin * scale << "  " << xmin * scale << "  " << ymax * scale << " blackseg_bold" << std::endl;
            file << xmax * scale << "  " << ymin * scale << "  " << xmax * scale << "  " << ymax * scale << " blackseg_bold" << std::endl;
        }
        
        // plot small filled circles
        for (size_t index = 0; index < _num_samples; index++)
        {		
            // filled discs
            file << _h[index] * scale_x << "  " << _f[index][ifunc] * scale_y << "  " << 0.2 * scale << "  ";
            file << "greenfcirc"     << std::endl; // non-active disk		
        }
        
        double ho = 0.0; 
        double fo = _fb - _epsilon * fabs(_fb);
        for (size_t icorner = 0; icorner < _num_corners; icorner++)
        {
            size_t corner_index = _corners[icorner];
            double hn = _h[corner_index];
            double fn = _f[corner_index][ifunc];
            file << hn * scale_x << "  " << fn * scale_y << "  " << 0.2 * scale << "  ";
            file << "bluefcirc"     << std::endl; // non-active disk
            
            file << ho * scale_x << "  " << fo * scale_y << "  " << hn * scale_x << "  " << fn * scale_y << " blackseg" << std::endl;
            
            ho = hn; fo = fn;
        }
        
        // candidate disc
        file << _h[icandidate] * scale_x << "  " << _f[icandidate][ifunc] * scale_y << "  " << 0.2 * scale << "  ";
        file << "redfcirc"     << std::endl; // non-active disk
        
        file << "showpage" << std::endl;
        
    }

    
    /////////////////////////////////////////////////////////////////////
    // Random Methods
    /////////////////////////////////////////////////////////////////////
    
    void OptDartsOptimizer::initiate_random_generator(unsigned long x)
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
    
    double OptDartsOptimizer::generate_a_random_number()
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

    void OptDartsOptimizer::sample_uniformly_from_unit_sphere_surface(double* dart, size_t num_dim)
    {
        // unbiased method
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
    }

    bool OptDartsOptimizer::trim_line_using_Hyperplane(size_t num_dim,                               // number of dimensions
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



OptDartsOptimizer::~OptDartsOptimizer() {};
    
// Opt-dart/DIRECT methods start here
    
    

}


