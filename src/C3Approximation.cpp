/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ProblemDescDB.hpp"
#include "C3Approximation.hpp"

#include "SharedC3ApproxData.hpp"

// #include "NonDIntegration.hpp"


//#define DEBUG


namespace Dakota {

    void C3Approximation::base_init()
    {

        // initialize function train data structures
        this->ft          = NULL;
        this->ft_gradient = NULL;
        this->ft_hessian  = NULL;
        this->ft_sobol    = NULL;
        ft_derived_functions_init_null(&(this->ft_derived_functions));

        this->optimizer = NULL;
        this->xtrain = NULL;
        this->ytrain = NULL;


        
        if (this->adaptive_construction == 0){
            double absxtol = 1e-10;
            this->optimizer  = c3opt_create(BFGS);
            c3opt_set_maxiter(this->optimizer,this->sharedC3DataRep->maxIterations);
            c3opt_set_gtol   (this->optimizer,this->sharedC3DataRep->solverTol);
            c3opt_set_relftol(this->optimizer,this->sharedC3DataRep->solverTol);
            c3opt_set_absxtol(this->optimizer,absxtol);
            c3opt_set_verbose(this->optimizer,0);

            
        }
        else{
            fprintf(stderr,"Have not implemented adaptive construction for C3Approximation yet\n");
            exit(1);
        }

        this->cv = false;
        this->start_ranks.size(this->dim+1);
        
        ////////////////////////////////////////////
        // Initialize memory for the rest of private data
        this->grad.size(dim);
        this->hess.shape(dim);
        this->moment_vector.size(4);
    }

    
    C3Approximation::
    C3Approximation(ProblemDescDB& problem_db,
                    const SharedApproxData& shared_data,
                    const String& approx_label):
        Approximation(BaseConstructor(), problem_db, shared_data, approx_label),
        sharedC3DataRep((SharedC3ApproxData*)sharedDataRep)
    {


        /////////////////////////////////////////////
        // Setup
        this->dim = sharedDataRep->numVars; // CHECK THIS AG
        this->num_random = 0; // set later
        this->num_det = this->dim - this->num_random;
        this->adaptive_construction = 0; // means do regression

        // this->ind_random = // initialize

        base_init();

    }

    C3Approximation::C3Approximation(const SharedApproxData& shared_data):
        Approximation(NoDBBaseConstructor(), shared_data),
        sharedC3DataRep((SharedC3ApproxData*)sharedDataRep)
    {

        // std::cout << "IN THIS CONSTRUCTOR Lets go\n";
        this->dim = sharedDataRep->numVars; // CHECK THIS AG
        this->num_random = 0; // set later
        this->num_det = this->dim - this->num_random;
        this->adaptive_construction = 0; // means do regression

        // this->ind_random = // initialize
        base_init();
               
    }    

    C3Approximation::~C3Approximation()
    {
        // free(this->ind_random); this->ind_random = NULL;


        // free approximation stuff
        c3opt_free(this->optimizer); this->optimizer   = NULL;
        free(this->xtrain);          this->xtrain      = NULL;
        free(this->ytrain);          this->ytrain      = NULL;

        // Free the rest of the stuff
        // TODO FREE OPTIONS

        function_train_free(this->ft);             this->ft          = NULL;
        ft1d_array_free(this->ft_gradient);        this->ft_gradient = NULL;
        ft1d_array_free(this->ft_hessian);         this->ft_hessian  = NULL;
        c3_sobol_sensitivity_free(this->ft_sobol); this->ft_sobol    = NULL;
        ft_derived_functions_free(&(this->ft_derived_functions));
    }

    void C3Approximation::build()
    {
        printf("Calling build from c3 approximation\n");
        
        // base class implementation checks data set against min required
        Approximation::build();


        for (size_t ii = 0; ii < this->dim+1; ii++){
            this->start_ranks(ii) = this->sharedC3DataRep->startRank;
        }
        this->start_ranks(0) = 1;
        this->start_ranks(this->dim) = 1;

        if (this->adaptive_construction == 0){

            this->ft_opts = this->sharedC3DataRep->approxOpts;
            struct FTRegress * ftr =
                ft_regress_alloc(this->dim,
                                 this->sharedC3DataRep->approxOpts,
                                 this->start_ranks.values());
            ft_regress_set_alg_and_obj(ftr,AIO,FTLS);
	    size_t r_adapt = this->sharedC3DataRep->adaptRank ? 1 : 0;
            ft_regress_set_adapt(   ftr,r_adapt);
            ft_regress_set_maxrank( ftr,this->sharedC3DataRep->maxRank);
            ft_regress_set_kickrank(ftr,this->sharedC3DataRep->kickRank);
            ft_regress_set_roundtol(ftr,this->sharedC3DataRep->roundingTol);
            ft_regress_set_verbose( ftr,this->sharedC3DataRep->verbose);

            c3opt_set_verbose(this->optimizer,this->sharedC3DataRep->verbose);
            c3opt_set_maxiter(this->optimizer,this->sharedC3DataRep->maxIterations);
            c3opt_set_gtol   (this->optimizer,this->sharedC3DataRep->solverTol);
            c3opt_set_relftol(this->optimizer,this->sharedC3DataRep->solverTol);

	    // *** TO DO: Alex will add mappings for latest XML attributes.
	    
            // free if previously built
            function_train_free(this->ft);      this->ft          = NULL;
            ft1d_array_free(this->ft_gradient); this->ft_gradient = NULL;
            ft1d_array_free(this->ft_hessian);  this->ft_hessian  = NULL;

            if (this->cv == true){
                fprintf(stderr,"Warning: not performing CV for C3Approximation\n");
                fprintf(stderr,"         because it is not yet implemented\n");
            }
        
        
	    const Pecos::SurrogateData& approx_data = surrogate_data();
            size_t i, j, num_v = sharedDataRep->numVars;
	    const Pecos::SDVArray& sdv_array = approx_data.variables_data();
	    const Pecos::SDRArray& sdr_array = approx_data.response_data();
            this->ndata = approx_data.points();
        
            // JUST 1 QOI
            // Transfer the training data to the Teuchos arrays used by the GP
            if (this->xtrain != NULL){
                free(this->xtrain); this->xtrain = NULL;
            }
            this->xtrain = (double*)calloc(this->dim*this->ndata,sizeof(double));
            if (this->xtrain == NULL){
                fprintf(stderr,"Failure to allocate memory in C3Approximation for training data\n");
                exit(1);
            }

            if (this->ytrain != NULL){
                free(this->ytrain); this->ytrain = NULL;
            }
            this->ytrain = (double*)calloc(this->ndata,sizeof(double));
            if (this->ytrain == NULL){
                fprintf(stderr,"Failure to allocate memory in C3Approximation for training data\n");
                exit(1);
            }

            // process currentPoints
            for (i=0; i<this->ndata; ++i) {
                const RealVector& c_vars = sdv_array[i].continuous_variables();
                for (j=0; j<num_v; j++){
                    this->xtrain[j + i*this->dim] = c_vars[j];
                }
                this->ytrain[i] = sdr_array[i].response_function();
            }

            // Build FT model
            this->ft = ft_regress_run(ftr,this->optimizer,this->ndata,this->xtrain,this->ytrain);
            this->ft_gradient = function_train_gradient(this->ft);
            this->ft_hessian  = ft1d_array_jacobian(this->ft_gradient);

            ft_regress_free(ftr); ftr = NULL;
        }
        else{
            fprintf(stderr,"Warning: adaptive construction of C3Approximation is not yet implemented\n");
            fprintf(stderr,"         I am doing nothing\n");
        }


    }

    void ft_derived_functions_init_null(struct FTDerivedFunctions * func)
    {
        func->set = 0;
        
        func->ft_squared                = NULL;
        func->ft_cubed                  = NULL;
        func->ft_constant_at_mean       = NULL;
        func->ft_diff_from_mean         = NULL;
        func->ft_diff_from_mean_squared = NULL;
        func->ft_diff_from_mean_cubed = NULL;        

        func->ft_diff_from_mean_tesseracted = NULL;
        func->ft_diff_from_mean_normalized  = NULL;

        func->ft_diff_from_mean_normalized_squared = NULL;
        func->ft_diff_from_mean_normalized_cubed   = NULL;
    
    }
    
    void ft_derived_functions_create(struct FTDerivedFunctions * func,
                                     struct FunctionTrain * ft,
                                     struct MultiApproxOpts * opts)
    {

        // printf("CREATE DERIVED_FUNCTIONS\n");
        
        func->ft_squared     = function_train_product(ft,ft);

        func->ft_cubed       = function_train_product(func->ft_squared,ft);
        // func->ft_tesseracted = function_train_product(func->ft_squared, func->ft_squared);

        func->first_moment        = function_train_integrate_weighted(ft);
        func->ft_constant_at_mean = function_train_constant(-func->first_moment,opts);
        func->ft_diff_from_mean   = function_train_sum(ft,func->ft_constant_at_mean);
        func->ft_diff_from_mean_squared =
            function_train_product(func->ft_diff_from_mean,
                                   func->ft_diff_from_mean);
        func->ft_diff_from_mean_cubed =
            function_train_product(func->ft_diff_from_mean_squared,
                                   func->ft_diff_from_mean);        
        func->ft_diff_from_mean_tesseracted =
            function_train_product(func->ft_diff_from_mean_squared,
                                   func->ft_diff_from_mean_squared);

        func->second_central_moment = function_train_integrate_weighted(func->ft_diff_from_mean_squared); // var
        
        func->third_central_moment = function_train_integrate_weighted(func->ft_diff_from_mean_cubed); // var        
        
        func->fourth_central_moment = function_train_integrate_weighted(func->ft_diff_from_mean_tesseracted);

        func->second_moment = function_train_integrate_weighted(func->ft_squared);
        func->third_moment =  function_train_integrate_weighted(func->ft_cubed);

        func->std_dev =  sqrt(func->second_central_moment);

        func->ft_diff_from_mean_normalized = function_train_copy(func->ft_diff_from_mean);
        function_train_scale(func->ft_diff_from_mean_normalized,1.0/func->std_dev);


        func->ft_diff_from_mean_normalized_squared =
            function_train_product(func->ft_diff_from_mean_normalized,
                                   func->ft_diff_from_mean_normalized);

        func->ft_diff_from_mean_normalized_cubed =
            function_train_product(func->ft_diff_from_mean_normalized_squared,
                                   func->ft_diff_from_mean_normalized);

        func->skewness = function_train_integrate_weighted(func->ft_diff_from_mean_normalized_cubed);
        func->kurtosis = func->fourth_central_moment / func->second_central_moment / func->second_central_moment;

        func->set = 1;
    
    }

    void ft_derived_functions_free(struct FTDerivedFunctions * func)
    {
        function_train_free(func->ft_squared);                func->ft_squared          = NULL;
        function_train_free(func->ft_cubed);                  func->ft_cubed            = NULL;
        function_train_free(func->ft_constant_at_mean);       func->ft_constant_at_mean = NULL;
        function_train_free(func->ft_diff_from_mean);         func->ft_diff_from_mean   = NULL;
        function_train_free(func->ft_diff_from_mean_squared); func->ft_diff_from_mean_squared = NULL;

        function_train_free(func->ft_diff_from_mean_cubed); func->ft_diff_from_mean_cubed = NULL;        

        function_train_free(func->ft_diff_from_mean_tesseracted); func->ft_diff_from_mean_tesseracted = NULL;
        function_train_free(func->ft_diff_from_mean_normalized);  func->ft_diff_from_mean_normalized  = NULL;

        function_train_free(func->ft_diff_from_mean_normalized_squared);
        func->ft_diff_from_mean_normalized_squared = NULL;

        function_train_free(func->ft_diff_from_mean_normalized_cubed);
        func->ft_diff_from_mean_normalized_cubed = NULL;

        func->set = 0;
    
    }


    void C3Approximation::compute_all_sobol_indices(size_t interaction_order)
    {
        if (ft_sobol == NULL){
            ft_sobol = c3_sobol_sensitivity_calculate(this->ft,interaction_order);
        }
        else{
            c3_sobol_sensitivity_free(ft_sobol);
            ft_sobol = c3_sobol_sensitivity_calculate(this->ft,interaction_order);
        }
    }


    Real C3Approximation::main_sobol_index(size_t dim)
    {
        return c3_sobol_sensitivity_get_main(this->ft_sobol,dim);
    }

    Real C3Approximation::total_sobol_index(size_t dim)
    {
        return c3_sobol_sensitivity_get_total(this->ft_sobol,dim);
    }

    void C3Approximation::sobol_iterate_apply(
        void (*f)(double val, size_t ninteract, size_t*interactions,void* arg), void* args)
    {
        c3_sobol_sensitivity_apply_external(this->ft_sobol,f,args);
    }

    void C3Approximation::compute_derived_statistics(bool overwrite)
    {
        if (this->ft_derived_functions.set == 0)
        {
            ft_derived_functions_create(&(this->ft_derived_functions),this->ft,this->ft_opts);
        }
        else if (overwrite == true){
            ft_derived_functions_free(&(this->ft_derived_functions));
            ft_derived_functions_create(&(this->ft_derived_functions),this->ft,this->ft_opts);
        }
    }


    // void C3Approximation::set_variable_types(const ShortArray & utypes, parameters)
    // {
        
    // }
    
    // void C3Approximation::set_random_vars(const SizetArray & arr)
    // {
    //     this->num_random = arr.size();
    //     this->num_det = this->dim - this->num_random;
        
    //     this->ind_random = arr;
    //     // Set 
    // }
        
    void C3Approximation::compute_moments(bool full_stats, bool combined_stats)
    {
        // printf("mean in compute_moments = %G\n",this->mean());
        // printf("length of moment vector = %d\n",this->moment_vector.length());
        this->moment_vector(0) = this->mean();
        this->moment_vector(1) = this->variance();
        this->moment_vector(2) = this->third_central();
        this->moment_vector(3) = this->fourth_central();
    }

    void C3Approximation::compute_moments(const RealVector& x, bool full_stats,
					  bool combined_stats) {
        this->moment_vector(0) = this->mean(x);
        this->moment_vector(1) = this->variance(x);
    }

    const RealVector& C3Approximation:: moments() const{
        return this->moment_vector;
    }

    Real C3Approximation:: moment(size_t i) const{
      return this->moment_vector(i);
    }

    void C3Approximation:: moment(Real mom, size_t i) {
      this->moment_vector(i) = mom;
    }

    Real C3Approximation::mean()
    {
        compute_derived_statistics(false);
        return this->ft_derived_functions.first_moment;
    }

    Real C3Approximation::mean(const RealVector &x)
    {
        // compute_derived_statistics(false);
    
        struct FunctionTrain * ftnonrand =
            function_train_integrate_weighted_subset(this->ft,this->num_random,this->ind_random);

        double out = function_train_eval(ftnonrand,x.values());
        function_train_free(ftnonrand); ftnonrand = NULL;
    
        return out;
    }

    const RealVector& C3Approximation::mean_gradient()
    {
        fprintf(stderr,"mean_gradient() in C3Approximation is not implemented because Alex\n");
        fprintf(stderr,"is not sure how what it means\n");
        exit(1);
    }

    const RealVector & C3Approximation::mean_gradient(const RealVector &x,const SizetArray & dvv)
    {
        fprintf(stderr,"mean_gradient(x,dvv) in C3Approximation is not implemented because Alex\n");
        fprintf(stderr,"is not sure how what it means\n");
        exit(1);        
    }

    Real C3Approximation::variance()
    {
        compute_derived_statistics(false);
        return this->ft_derived_functions.second_central_moment;
    }

    Real C3Approximation::variance(const RealVector &x)
    {
        compute_derived_statistics(false);

        Real mean = this->mean();

        struct FunctionTrain * ftnonrand =
            function_train_integrate_weighted_subset(this->ft_derived_functions.ft_squared,
                                                     this->num_random,this->ind_random);

        // for (size_t ii = 0; ii < this->num_det;ii++){
        //     this->pt_det[ii] = x(ii);
        // }
        // double f2 = function_train_eval(ftnonrand,this->pt_det);
        double f2 = function_train_eval(ftnonrand,x.values());
        double var = f2 - mean*mean;
    
        function_train_free(ftnonrand); ftnonrand = NULL;
    
        return var;
    }

    const RealVector& C3Approximation::variance_gradient()
    {
        // Gradient of the Variance with respect to design variables *NOT VARIANCE OF GRADIENT*
        // I need to manage low-rank representations of the gradient, then evaluate the variance of each
        // of the partial derivatives
        fprintf(stderr,"mean_gradient() in C3Approximation is not implemented because Alex\n");
        fprintf(stderr,"is not sure how what it means\n");
        exit(1);
    }

    const RealVector & C3Approximation::variance_gradient(const RealVector &x,const SizetArray & dvv)
    {
        // size of output is size of dvv, only take gradients with respect to dvv fix design and epistemic at x
        fprintf(stderr,"mean_gradient(x,dvv) in C3Approximation is not implemented because Alex\n");
        fprintf(stderr,"is not sure how what it means\n");
        exit(1);        
    }

    struct FunctionTrain * C3Approximation::subtract_const(Real val)
    {
        struct FunctionTrain * ftconst = function_train_constant(val,this->ft_opts);
        struct FunctionTrain * updated = function_train_sum(this->ft,ftconst);

        function_train_free(ftconst); ftconst = NULL;
        return updated;
    }

    

    Real C3Approximation::covariance(Approximation& approx_2)
    {
        C3Approximation* ft2 = (C3Approximation*)approx_2.approx_rep();
        Real mean1 = this->mean();
        Real mean2 = ft2->mean();

        struct FunctionTrain * fttemp = this->subtract_const(mean1);
        struct FunctionTrain * fttemp2 = ft2->subtract_const(mean2);

        double retval = function_train_inner_weighted(fttemp,fttemp2);

        function_train_free(fttemp);  fttemp = NULL;
        function_train_free(fttemp2); fttemp2 = NULL;

        return retval;
    }

    Real C3Approximation::covariance(const RealVector& x, Approximation& approx_2)
    {
        C3Approximation* ft2 = (C3Approximation*)approx_2.approx_rep();

        fprintf(stderr,"covariance(x,ft2) in C3Approximation is not implemented because Alex\n");
        fprintf(stderr,"is not sure how what it means\n");
        exit(1);
    }


    Real C3Approximation::third_central()
    {
        compute_derived_statistics(false);
        return this->ft_derived_functions.third_central_moment;
    }

    Real C3Approximation::fourth_central()
    {
        compute_derived_statistics(false);
        return this->ft_derived_functions.fourth_central_moment;
    }


    Real C3Approximation::skewness()
    {
        compute_derived_statistics(false);
        return this->ft_derived_functions.skewness;
    }

    Real C3Approximation::kurtosis()
    {
        compute_derived_statistics(false);
        return this->ft_derived_functions.kurtosis;
    }


    static void continuous_to_double_vars(size_t dim,const Variables &vars, double * pt)
    {
        for (size_t ii = 0; ii < dim; ii++){
            pt[ii] = vars.continuous_variable(ii);
        }
    }


    int C3Approximation::min_coefficients() const
    {
        // min number of samples required to build the network is equal to
        // the number of design variables + 1

        // Note: Often this is too few samples.  It is better to have about
        // O(n^2) samples, where 'n' is the number of variables.

        return this->dim+1;
    }

    
// ignore discrete variables for now
    Real C3Approximation::value(const Variables& vars)
    {
        // continuous_to_double_vars(this->dim,vars,this->pt);
        Real retval = function_train_eval(this->ft,vars.continuous_variables().values());
        return retval;
    }


// ignore discrete variables for now
    const RealVector& C3Approximation::gradient(const Variables& vars)
    {
        // continuous_to_double_vars(this->dim,vars,this->pt);
        for (size_t ii = 0; ii < this->dim; ii++){
            grad(ii) = function_train_eval(this->ft_gradient->ft[ii],vars.continuous_variables().values());
        }
        return grad;
    }

// ignore discrete variables for now
    const RealSymMatrix& C3Approximation::hessian(const Variables& vars)
    {
        // continuous_to_double_vars(this->dim,vars,this->pt);
        for (size_t ii = 0; ii < this->dim; ii++){
            for (size_t jj = 0; jj <= ii; jj++){
                hess(ii,jj) = function_train_eval(this->ft_hessian->ft[ii + jj*this->dim],vars.continuous_variables().values());
            }
        }
        return hess;
    }

    /*
    void C3Approximation::store(size_t index)
    {
        ft_derived_functions_free(&(this->ft_derived_functions));
        c3_sobol_sensitivity_free(this->ft_sobol); this->ft_sobol = NULL;
        
        // base class implementation manages approx data
        Approximation::store(index);

        size_t stored_len = storedFT.size();
        if (index == _NPOS || index == stored_len) { // append
            storedFT.push_back(function_train_copy(this->ft));            
        }
        else if (index < stored_len) { // replace
            // function_train_free(storedFT[index]); storedFT[index] = NULL;
            storedFT[index] = function_train_copy(this->ft); // MEMORY LEAK
        }
        else {
            PCerr << "Error: bad index (" << index << ") passed in C3Approximation::store"
                  << std::endl;
            abort_handler(-1);
        }
        
    }


    void C3Approximation::restore(size_t index)
    {
        // base class implementation manages approx data
        Approximation::restore(index);
        // map to Pecos::BasisApproximation

        size_t stored_len = storedFT.size();
        function_train_free(this->ft); this->ft = NULL;
        ft1d_array_free(this->ft_gradient); this->ft_gradient = NULL;
        ft1d_array_free(this->ft_hessian); this->ft_hessian = NULL;
        if (index == _NPOS) {
            this->ft = storedFT.back();
            this->ft_gradient = function_train_gradient(this->ft);
            this->ft_hessian  = ft1d_array_jacobian(this->ft_gradient);
        }
        else if (index < stored_len) {
            this->ft = storedFT[index];
            this->ft_gradient = function_train_gradient(this->ft);
            this->ft_hessian  = ft1d_array_jacobian(this->ft_gradient);           
        }
        else {
            PCerr << "Error: bad index (" << index << ") passed in"
                  << "C3Approximation::restore()" << std::endl;
            abort_handler(-1);
        }

    }


    void C3Approximation::remove_stored(size_t index)
    {
        // base class implementation manages approx data
        Approximation::remove_stored(index);

        size_t stored_len = storedFT.size();
        storedFT.erase(storedFT.begin()+index);
    }
    */
    

} // namespace Dakota
