/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedC3ApproxData
//- Description:  Implementation code for SharedC3ApproxData class
//-               
//- Owner:        Mike Eldred

#include "SharedC3ApproxData.hpp"
#include "ProblemDescDB.hpp"
#include "NonDIntegration.hpp"

#include "pecos_stat_util.hpp"
#include "pecos_global_defs.hpp"

#include <assert.h>
//#define DEBUG

namespace Dakota {

    SharedC3ApproxData::SharedC3ApproxData()
    {
        this->constructed = 0;
        printf("In the default constructor\n");
    }
    
    SharedC3ApproxData::
    SharedC3ApproxData(ProblemDescDB& problem_db, size_t num_vars):
        maxNum(problem_db.get_sizet("model.c3function_train.max_num")),
        maxRank(problem_db.get_sizet("model.c3function_train.max_rank")),
        startOrder(problem_db.get_sizet("model.c3function_train.start_order")),
        startRank(problem_db.get_sizet("model.c3function_train.start_rank")),
        rankAdapt(problem_db.get_sizet("model.c3function_train.rank_adapt")),
        crossMaxIter(problem_db.get_sizet("model.c3function_train.cross_maxiter")),
        convergenceTol(problem_db.get_real("model.c3function_train.solver_tolerance")),
        roundingTol(problem_db.get_real("model.c3function_train.rounding_tolerance")),
        maxIterations(problem_db.get_int("model.max_iterations")),
        kickRank(problem_db.get_sizet("model.c3function_train.kick_rank")),
        verbose(problem_db.get_sizet("model.c3function_train.verbosity")),
        SharedApproxData(BaseConstructor(), problem_db, num_vars)
    {
        // printf("CONSTRUCTING SHAREDC3APPROX\n");

        // std::cout << "Convergence Tolerance is " << convergenceTol << std::endl;
        this->approxOpts = multi_approx_opts_alloc(num_vars);
        this->oneApproxOpts = (struct OneApproxOpts **)
            malloc(num_vars * sizeof(struct OneApproxOpts *));
        for (size_t ii = 0; ii < num_vars; ii++){
            struct OpeOpts * opts = ope_opts_alloc(LEGENDRE);
            ope_opts_set_lb(opts,-2);
            ope_opts_set_ub(opts,2);
            ope_opts_set_nparams(opts,maxNum);
            this->oneApproxOpts[ii] = one_approx_opts_alloc(POLYNOMIAL,opts);
            multi_approx_opts_set_dim(this->approxOpts,ii,this->oneApproxOpts[ii]);
            // this->oneApproxOpts[ii] = NULL;
            // multi_approx_opts_set_dim(this->approxOpts,ii,this->oneApproxOpts[ii]);
        }
        this->constructed = 1;
    }

    SharedC3ApproxData::
    SharedC3ApproxData(const String& approx_type,
                       const UShortArray& approx_order, size_t num_vars,
                       short data_order, short output_level):
        SharedApproxData(NoDBBaseConstructor(), approx_type, num_vars, data_order,
                         output_level)
    {
        // short basis_type; approx_type_to_basis_type(approxType, basis_type);

        // printf("IN THIS CONSTRUCTOR\n");
        this->approxOpts = multi_approx_opts_alloc(num_vars);
        this->oneApproxOpts = (struct OneApproxOpts **)
            malloc(num_vars * sizeof(struct OneApproxOpts *));
        for (size_t ii = 0; ii < num_vars; ii++){
            this->oneApproxOpts[ii] = NULL;
        }
        this->constructed = 1;

        this->maxNum = 4;
        this->startOrder = 2;
        this->startRank = 5;
        this->rankAdapt = 0;
        this->maxRank = 10;
        this->crossMaxIter = 5;
        this->convergenceTol = 1e-10;
        this->roundingTol = 1e-10;
        this->maxIterations = 1000;
        this->kickRank = 2;
        this->verbose = 0;

    }

    SharedC3ApproxData::~SharedC3ApproxData()
    {
        multi_approx_opts_free(this->approxOpts); this->approxOpts = NULL;

        for (size_t ii = 0; ii < this->numVars; ii++){
            one_approx_opts_free_deep(&(this->oneApproxOpts[ii]));
            this->oneApproxOpts[ii] = NULL;
        }
        free(this->oneApproxOpts); this->oneApproxOpts = NULL;
    }

    void SharedC3ApproxData::set_parameter(String var, void * val)
    {
        if (var.compare("initial_ranks") == 0){
            this->startRank = *(size_t*)val;
        }
        else if (var.compare("maximum_num_param") == 0){
            this->maxNum = *(size_t*)val;
        }
        else if (var.compare("start_poly_order") == 0){
            this->startOrder= *(size_t*)val;
        }
        else if (var.compare("rank_adapt") == 0){
            this->rankAdapt = *(size_t*)val;
        }
        else if (var.compare("maximum_rank") == 0){
            this->maxRank = *(size_t*)val;
        }
        else if (var.compare("maximum_cross_approximation_iters") == 0){
            this->crossMaxIter = *(size_t*)val;
        }
        else if (var.compare("convergence_tol") == 0){
            this->convergenceTol = *(double*)val;
        }
        else if (var.compare("rounding_tol") == 0){
            this->roundingTol = *(double*)val;
        }
        else if (var.compare("kickrank") == 0){
            this->kickRank = *(size_t*)val;
        }
        else if (var.compare("verbose") == 0){
            this->verbose = *(size_t*)val;
        }
        else{
            std::cerr << "Unrecognized approximation parameter: " << var << std::endl;
        }

    }

    
    void SharedC3ApproxData::construct_basis(const ShortArray& u_types,
                                             const Pecos::AleatoryDistParams & adp)
    {

        // printf("constructed = %d\n",this->constructed);
        assert (u_types.size() == this->numVars);

        for (size_t i=0; i < this->numVars; ++i){
            // printf("i = %zu\n",i);
            struct OpeOpts * opts = NULL;
            switch (u_types[i]) {
            case Pecos::STD_NORMAL:
                opts = ope_opts_alloc(HERMITE);
                break;
            case Pecos::STD_UNIFORM:
                opts = ope_opts_alloc(LEGENDRE);
                break;
            default:
                PCerr << "Error: unsupported u-space type _opts_set_in SharedC33ApproxData::"
                      << "  " << u_types[i] << "  " 
                      << "distribution_parameters()" << std::endl;
                abort_handler(-1);
                break;
            }
            // printf("push_back\n");
            ope_opts_set_nparams(opts,5);
            one_approx_opts_free_deep(&(this->oneApproxOpts[i]));
            this->oneApproxOpts[i] = one_approx_opts_alloc(POLYNOMIAL,opts);
            // printf("set i\n");
            multi_approx_opts_set_dim(this->approxOpts,i,this->oneApproxOpts[i]);
        }
        // do nothing
    }

    void SharedC3ApproxData::store(size_t index)
    {
        size_t stored_len = storeOne.size();
        if (index == _NPOS || index == stored_len) { // append
            storeOne.push_back(this->oneApproxOpts);
            storeMulti.push_back(this->approxOpts);
        }
        else if (index < stored_len) { // replace
            storeOne[index] = this->oneApproxOpts;
            storeMulti[index] = this->approxOpts;
        }

    }
    
    size_t SharedC3ApproxData::pre_combine(short combine_type)
    {
        (void) (combine_type);
        return 0;
    }

    void SharedC3ApproxData::post_combine(short combine_type)
    {
        (void) (combine_type);
    }

    void SharedC3ApproxData::restore(size_t index)
    {
        (void) (index);        
    }

    void SharedC3ApproxData::remove_stored(size_t index)
    {

        (void)(index);
    }


} // namespace Dakota
