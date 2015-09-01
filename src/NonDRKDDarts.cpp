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
  emulatorSamples(probDescDB.get_int("method.nond.emulator_samples")),
  samples(probDescDB.get_int("method.samples"))
{
    // Constructor
    std::cout << "------------------" << std::endl;
    std::cout << "*** LAUNCH RKD ***" << std::endl;
    std::cout << "------------------" << std::endl;
    
    if (emulatorSamples==0) emulatorSamples = 1E6;
}
    // --------------------------
    NonDRKDDarts::~NonDRKDDarts()
    { }
    
    // --------------------------
    void NonDRKDDarts::pre_run()
    {
        // generate parameter sets
        Cout << "*** IN PROGRESS: generating " << samples << " points *** " << '\n';
        
        initiate_random_number_generator(seed);

        init_rkd_darts(); // prepare global containers
        
    }
    // --------------------------
    /** Loop over the set of samples and compute responses. */
    void NonDRKDDarts::quantify_uncertainty()
    {
        Cout << "*** IN PROGRESS: quantifying uncertainty *** " << '\n';
        
        _I_RKD = execute(samples);
    }
    // --------------------------
    void NonDRKDDarts::post_run(std::ostream& s)
    {
        // generate outputs
        Cout << "*** IN PROGRESS: printing integration results *** " << '\n';
        
        print_integration_results();
        
        exit_rkd_darts(); // clean up memory
    }
    /////////////////////////////////////////////////////////////
    //
    // RKD METHODS
    //
    /////////////////////////////////////////////////////////////
    
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
    void NonDRKDDarts::init_rkd_darts()
    {
        _num_dim = numContinuousVars;
        
        _xmin = new double[_num_dim];
        _xmax = new double[_num_dim];
        
        const RealVector&  lower_bounds = iteratedModel.continuous_lower_bounds();
        const RealVector&  upper_bounds = iteratedModel.continuous_upper_bounds();
        
        for (size_t idim = 0; idim < _num_dim; idim++)
        {
            _xmin[idim] = lower_bounds[idim];
            _xmax[idim] = upper_bounds[idim];
        }
    }
    // ----------------------------------------------------------------
    void NonDRKDDarts::exit_rkd_darts()
    {
    }
    // ----------------------------------------------------------------
    double NonDRKDDarts::execute(size_t num_darts)
    {
        clock_t start_time, end_time;
        double cpu_time, total_time(0.0);
        start_time = clock();
        
        double sum(0.0), idart(0.0);
        
        double* dart = new double[_num_dim];
        
        while (idart < num_darts)
        {
            for (size_t idim = 0; idim < _num_dim; idim++)
            {
                double u = generate_a_random_number();
                dart[idim] = _xmin[idim] + u * (_xmax[idim] - _xmin[idim]);
            }
            double f = evaluate_function(dart);
            sum +=f;
            idart++;
        }
        
        end_time = clock();
        cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
        
        delete[] dart;
        
        //return _bounding_box_volume * sum / num_darts;
        return sum / num_darts;
    }
    // ----------------------------------------------------------------
    double NonDRKDDarts::evaluate_function(double* x)
    {
        //Smooth Herbie
        double E = 2.7182818284590452353602874713526;
        double fval = 1.0;
        for (size_t idim = 0; idim < _num_dim; idim++)
        {
            double xm = x[idim] - 1.0;
            double xp = x[idim] + 1.0;
            double wherb = pow(E, - xm * xm) + pow(E, -0.8 * xp * xp) ;// - 0.05 * sin(8 * (x[idim] + 0.1));
            fval *= wherb;
        }
        fval = -fval;
        return fval;
    }
    // ----------------------------------------------------------------
    void NonDRKDDarts::print_integration_results()
    {
        size_t width = write_precision+7, w2p2 = 2*width+2, w3p4 = 3*width+4;
        
        std::cout << std::endl;
        
        std::cout << "Numerical integration results using Recursive k-d (RKD) Darts:" << std::endl << std::endl;
        std::cout << "     Sample Budget  Num. Integration " << std::endl;
        std::cout << "     -------------  ---------------- " << std::endl;
        std::cout << std::setw(width) << samples << std::setw(width) << _I_RKD << std::endl;
    }
    ///////////////////////////////////////////////////////////////////
    
// --------------------------------------------------------------------
} // namespace Dakota

