/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 VPSApproximation
//- Description: Class implementation for Voronoi Piecewise Surrogate Approximation
//- Owner:       Mohamed Ebeida and Ahmad Rushdi
//- Checked by:
//- Version:
//-------------------------------------------------------------------------
#include "VPSApproximation.hpp"
#include "DakotaIterator.hpp"
#include "DakotaResponse.hpp"



//#define DEBUG
//#define DEBUG_FULL

//#define DEBUG_TEST_FUNCTION


namespace Dakota
{

    //initialization of statics

    /// default constructor
    VPSApproximation* VPSApproximation::VPSinstance(NULL);

    
    /// standard constructor (to call VPS from an input deck)
    
    VPSApproximation::VPSApproximation(const ProblemDescDB& problem_db,
                                       const SharedApproxData& shared_data,
                                       const String& approx_label):
                                       Approximation(BaseConstructor(), problem_db, shared_data, approx_label),
                                       _disc_min_jump(problem_db.get_real("model.surrogate.discont_jump_thresh")),
                                       _disc_min_grad(problem_db.get_real("model.surrogate.discont_grad_thresh"))
    {

        const String& surrogate_type = problem_db.get_string("model.surrogate.type");
      
        if (surrogate_type != "global_kriging" && surrogate_type != "global_polynomial" &&
	        surrogate_type != "global_radial_basis")
        {
            Cerr << "\nError: Domain decomposition option is not available for " << surrogate_type
                 << " surrogate; consider polynomial regression, GP, or RBF" << std::endl;
            abort_handler(-1);
        }

        // Default subsurrogate is LS_polynomial
        _vps_subsurrogate = LS;
        _vps_subsurrogate_basis = polynomial;
        
        // switch to GP or radial basis functions if requested
        if (surrogate_type == "global_kriging") _vps_subsurrogate = GP;
        else if (surrogate_type == "global_radial_basis") _vps_subsurrogate_basis = radial;

	
        if (_vps_subsurrogate == LS && _vps_subsurrogate_basis == polynomial)
        {
            surrogateOrder = problem_db.get_short("model.surrogate.polynomial_order");
        }

        std::cout << ".: VPS :.   Initializing, Surrogate order " << surrogateOrder << "." << std::endl;
        std::cout << ".: VPS :.   Initializing, Surrogate type " << surrogate_type <<  "." << std::endl;
        
        _use_derivatives = problem_db.get_bool("model.surrogate.derivative_usage");
        
        if (_use_derivatives) std::cout << ".: VPS :.   Derivatives' information will be used for approximation." << std::endl;
        else                  std::cout << ".: VPS :.   Approximation will not use derivatives' information." << std::endl;

    }
    
    /// Alternate constructor (to call VPS from another method like POF-darts)
    
    VPSApproximation::VPSApproximation(const SharedApproxData& shared_data):
                                       Approximation(NoDBBaseConstructor(), shared_data)
    {
        SharedSurfpackApproxData* dat = dynamic_cast<SharedSurfpackApproxData*> (shared_data.data_rep());
        
        surrogateOrder = dat->approxOrder;
        
        //std::cout << ".: VPS :.   Initializing, Surrogate order = " << surrogateOrder << "." << std::endl;
        
        _disc_min_jump = DBL_MAX; _disc_min_grad = DBL_MAX;
        
    }
    
    /// destructor
    VPSApproximation::~VPSApproximation()
    {
        VPS_destroy_global_containers();
    }
    
    
    //////////////////////////////////////////////////////////////
    // VPS METHODS
    //////////////////////////////////////////////////////////////
    
    bool VPSApproximation::VPS_execute()
    {
        
        #ifdef DEBUG_TEST_FUNCTION
        //*** Test function for debugging only
        _vps_test_function = UnitSphere;
        // Functions available for debugging: {SmoothHerbie, Herbie, Cone, Cross, UnitSphere, Linear34}
        #endif
        
        initiate_random_number_generator(1234567890);
        
        clock_t start_time, end_time; double cpu_time, total_time(0.0);
        
        start_time = clock();
        
        _num_GMRES = 0;
        
        
        // Create and fill in VPS containers
        VPS_create_containers();
               
        // update neighbors
        //std::cout << "updating neighbors!" << std::endl;
        for (size_t isample = 0; isample < _num_inserted_points; isample++)
        {
            VPS_retrieve_neighbors(isample, false);
        }
        //std::cout << "updating neighbors ... DONE." << std::endl;
        
        
        if (_vps_subsurrogate == LS && _vps_subsurrogate_basis == radial)
        {
            std::cout<<".: VPS :.   Constructing Radial Basis Functions.";
            
            for (size_t isample = 0; isample < _num_inserted_points; isample++)
            {
                build_radial_basis_function(isample);
            }
            //std::cout << " done!" << std::endl;
        }
        
        // initiate extended neighbors with seed neighbors
        //std::cout << "extending neighbors!" << std::endl;
        for (size_t isample = 0; isample < _num_inserted_points; isample++)
        {
            size_t num_neighbors = 0;
            if (_sample_neighbors[isample] != 0) num_neighbors = _sample_neighbors[isample][0];
            _vps_ext_neighbors[isample] = new size_t[num_neighbors + 1];
            _vps_ext_neighbors[isample][0] = num_neighbors;
            for (size_t ineighbor = 1; ineighbor <= num_neighbors; ineighbor++)
            {
                _vps_ext_neighbors[isample][ineighbor] = _sample_neighbors[isample][ineighbor];
            }
        }
        //std::cout << "extending neighbors ... DONE" << std::endl;

        
        if (_vps_subsurrogate == LS)
        {
            // extend neighbors for all points to match the desired order per cell
            // std::cout << "adjusting extending neighbors!" << std::endl;
            VPS_adjust_extend_neighbors_of_all_points();
            // std::cout << "adjusting extending neighbors - done!" << std::endl;
        }
         
        if (_vps_subsurrogate == GP)
        {
            size_t num_layers(2);
            for (size_t ilayer  = 0; ilayer < num_layers; ilayer++)
            {
                for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
                {
                    VPS_extend_neighbors(ipoint);
                }
            }
        }
        
        if (_vps_subsurrogate == LS && _vps_subsurrogate_basis == polynomial)
        {
            // get furthest neighbor for each LS surrogate
            for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
            {
                _vps_dfar[ipoint] = 0.0;
                for (size_t i = 1; i <= _vps_ext_neighbors[ipoint][0]; i++)
                {
                    size_t neighbor = _vps_ext_neighbors[ipoint][i];
                    double dst = 0.0;
                    for (size_t idim = 0; idim < _n_dim; idim++)
                    {
                        double dx = _sample_points[ipoint][idim] - _sample_points[neighbor][idim];
                        dst += dx * dx;
                    }
                    dst = std::sqrt(dst);
                    if (dst > _vps_dfar[ipoint]) _vps_dfar[ipoint] = dst;
                }
            }
        }
        
        // build local surrogates
        for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
        {
            VPS_build_local_surrogate(ipoint);
        }
        
        end_time = clock();
        cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC; total_time += cpu_time;
        
        if (_vps_subsurrogate == LS)
        {
            size_t av_num_basis(0);
            for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++) av_num_basis+= _num_cell_basis_functions[ipoint];
            av_num_basis/= _num_inserted_points;
            
            //
            std::cout << std::endl;
            std::cout << "-- Least Squares results --" << std::endl;
            std::cout << ".: VPS :.   Average Number of polynomial coefficients = " << std::fixed << av_num_basis << "." << std::endl;
            std::cout << ".: VPS :.   Number of GMRES solves = " << std::fixed << _num_GMRES << "." << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << "================================================================== " << std::endl;
        std::cout << ".: VPS :.   Global surrogate constructed in " << std::fixed << cpu_time << " seconds." << std::endl;
        
        #ifdef DEBUG_TEST_FUNCTION
        std::vector<double> contours;
        contours.push_back(_f_min - 2 * (_f_max - _f_min));
        size_t num_contours(20);
        for (size_t i = 0; i < num_contours; i++) contours.push_back(_f_min + (1.0/num_contours) * i * (_f_max - _f_min));
        contours.push_back(_f_max + 2 * (_f_max - _f_min));
        isocontouring_solid("vps_surrogate.ps", false, true, contours);
        isocontouring_solid("vps_test_function.ps", true, false, contours);
        plot_neighbors();
        #endif
        
        return true;
    }
    
    void VPSApproximation::VPS_create_containers()
    {
        #ifdef DEBUG_TEST_FUNCTION
        std::cout << std::endl;
        std::cout << ".: VPS Debug Mode :." << std::endl;
        std::cout << "======================" << std::endl;
        #else
        std::cout << std::endl;
        std::cout << ".: VPS Running Mode :." << std::endl;
        std::cout << "======================" << std::endl;
        #endif
        
        // =======================
        // Problem Info
        //std::cout<< ".: VPS :.   Transfering data to surrogate ... " << std::endl;
        
        std::cout << std::endl;
        std::cout << "-- Problem Info --" << std::endl;
        
        numObs = approxData.points(); // number of points
        size_t num_v = sharedDataRep->numVars;  // number of variables
        
        std::cout<< ".: VPS :.   Constructing a surrogate using " <<  numObs << " sample points." << std::endl;
        _num_inserted_points = numObs;
        
        std::cout<< ".: VPS :.   Problem dimensions = " << num_v << "." << std::endl;
        _n_dim = num_v;
        
        std::cout<< ".: VPS :.   Surrogate order = " << surrogateOrder << "." << std::endl;
        _vps_order = surrogateOrder;
        
        // =======================
        // Surrogate type
        std::cout << std::endl;
        std::cout << "-- Surrogate Type --" << std::endl;
        
        if (_vps_subsurrogate == LS)
        {
            if( _vps_subsurrogate_basis == polynomial)
                std::cout << ".: VPS :.   Solving Least Squares (LS) for a polynomial regression surrogate." << std::endl;
            else
                std::cout << ".: VPS :.   Solving Least Squares (LS) for a Radial Basis Functions (RBF) surrogate." << std::endl;
        }
        else if (_vps_subsurrogate == GP)
        {
            std::cout << ".: VPS :.   Constructing a Gaussian Process (GP) surrogate." << std::endl;
        }
        
        // =======================
        // Discontinuity Detection
        std::cout << std::endl;
        std::cout << "-- Discontinuity Detection Capability --" << std::endl;
        
        if (_disc_min_jump < 1E-10)
        {
            _disc_min_jump = DBL_MAX;
            std::cout << ".: VPS :.   Based on function evaluations: disabled." << std::endl;
        }
        else
        {
            std::cout << ".: VPS :.   Based on function evaluations: Enabled." << std::endl;
            std::cout << ".: VPS :.   Discontinuity jump threshold = " << _disc_min_jump << std::endl;
        }

        if (_disc_min_grad < 1E-10)
        {
            _disc_min_grad = DBL_MAX;
            std::cout << ".: VPS :.   Based on gradients: disabled." << std::endl;
        }
        else
        {
            std::cout << ".: VPS :.   Based on gradients: Enabled." << std::endl;
            std::cout << ".: VPS :.   Discontinuity gradient threshold = " << _disc_min_grad << std::endl;
        }
        // =======================
        
        // domain bounds
        _xmin = new double[_n_dim];
        _xmax = new double[_n_dim];
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            _xmin[idim] = DBL_MAX;
            _xmax[idim] = -DBL_MAX;
        }

        _sample_points = new double*[_num_inserted_points];
        _fval = new double[_num_inserted_points];
        
        _sample_neighbors = new size_t*[_num_inserted_points];
        _vps_ext_neighbors = new size_t*[_num_inserted_points];
        
        for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
        {
            _sample_neighbors[ipoint] = 0;
            _vps_ext_neighbors[ipoint] = 0;
        }
        if (_vps_subsurrogate == LS)
        {
            _num_cell_basis_functions = new size_t[_num_inserted_points];
            if (_vps_subsurrogate_basis == radial)
            {
                _sample_basis = new double**[_num_inserted_points];
            }
        }
        
        _sample_vsize = new double[_num_inserted_points];
        
        if (_vps_subsurrogate == LS && _vps_subsurrogate_basis == polynomial) _vps_dfar = new double[_num_inserted_points];
        
        // process currentPoints from approx data
        _f_min = DBL_MAX;
        _f_max = -_f_min;
        
        #ifdef DEBUG_TEST_FUNCTION
        
        if (_vps_test_function == SmoothHerbie || _vps_test_function == Herbie)
        {
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                _xmin[idim] = -2.0;
                _xmax[idim] = 2.0;
            }
        }
        else if (_vps_test_function == Cone)
        {
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                _xmin[idim] = -1.0;
                _xmax[idim] =  1.0;
            }
        }
        else if (_vps_test_function == Cross)
        {
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                _xmin[idim] = 0.0;
                _xmax[idim] = 1.0;
            }
        }
        else if (_vps_test_function == Linear34)
        {
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                _xmin[idim] = 0.0;
                _xmax[idim] = 2.0;
            }
        }
        else
        {
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                _xmin[idim] = 0.0;
                _xmax[idim] = 1.0;
            }
        }
        
        _diag = 0.0;
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            double DX = _xmax[idim] - _xmin[idim];
            _diag += DX * DX;
        }
        _diag = std::sqrt(_diag);

        // generate Poisson-disk sample
        size_t num_points(0);
        double h_mps = _diag / std::sqrt((double) _n_dim);
        
        double r_mps = 0.8 * h_mps / pow(_num_inserted_points, 1.0 / _n_dim);
        //std::cout << ".: VPS :.   DEBUG: h = " << h_mps << " , r_MPS = " << r_mps;
        generate_poisson_disk_sample(r_mps);
        //std::cout << " ... done!" << std::endl;
        for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
        {
            _fval[ipoint] = f_test(_sample_points[ipoint]);
            //_fval[ipoint] = 1.0;
            if (_fval[ipoint] < _f_min) _f_min = _fval[ipoint];
            if (_fval[ipoint] > _f_max) _f_max = _fval[ipoint];
        }
        
        // =======================
        // Gradients and Hessians
        std::cout << std::endl;
        std::cout << "-- Gradients & Hessians --" << std::endl;
        
        // Retrieve function gradients: If first point has gradients, I am assuming all points have
        if (_use_derivatives && approxData.response_active_bits(0) & 2)
        {
            _use_gradient = true;
            _fgrad = new double*[_num_inserted_points];
            for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
            {
                double* fn_grad = grad_f_test(_sample_points[ipoint]);
                _fgrad[ipoint] = new double[_n_dim];
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    _fgrad[ipoint][idim] = fn_grad[idim] * (_xmax[idim] - _xmin[idim]);
                    
                    //std::cout<< "*** For ipoint = " << ipoint << " and idim = " << idim << ":" << std::endl;
                    //std::cout<< "*** VPS::  fgrad = " << _fgrad[ipoint][idim] << std::endl;

                }
                delete[] fn_grad;
            }
            std::cout<< ".: VPS :.   Using gradient information." << std::endl;
        }
        else
        {
            _use_gradient = false;
            std::cout<< ".: VPS :.   Not using gradient information." << std::endl;
        }
        
        // Retrieve function hessians: If first point has gradients, I am assuming all points have
        if (_use_derivatives && approxData.response_active_bits(0) & 4)
        {
            _use_hessian = true;
            _fhess = new double**[_num_inserted_points];
            for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
            {
                double** fn_hessian = hessian_f_test(_sample_points[ipoint]);
                _fhess[ipoint] = new double*[_n_dim];
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    _fhess[ipoint][idim] = new double[_n_dim];
                    for (size_t jdim = 0; jdim < _n_dim; jdim++)
                    {
                        _fhess[ipoint][idim][jdim] = fn_hessian[idim][jdim] * (_xmax[idim] - _xmin[idim]) * (_xmax[jdim] - _xmin[jdim]);
                    }
                    delete[] fn_hessian[idim];
                }
                delete[] fn_hessian;
            }
            std::cout<< ".: VPS :.   Using hessian information." << std::endl;
        }
        else
        {
            _use_hessian = false;
            std::cout<< ".: VPS :.   Not using hessian information." << std::endl;
        }
        
        // =====
        // scale input domain to be a unit box
        for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
        {
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                _sample_points[ipoint][idim] = (_sample_points[ipoint][idim] - _xmin[idim]) / (_xmax[idim] - _xmin[idim]);
            }
        }
        // =====

        #else
        
        // Retrieve function values
        for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
        {
            const RealVector& c_vars = approxData.continuous_variables(ipoint);
            
            _sample_points[ipoint] = new double[_n_dim];
            
            _diag = 0.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                _sample_points[ipoint][idim] = c_vars[idim];
                if (_sample_points[ipoint][idim] < _xmin[idim]) _xmin[idim] = _sample_points[ipoint][idim];
                if (_sample_points[ipoint][idim] > _xmax[idim]) _xmax[idim] = _sample_points[ipoint][idim];
                double DX = _xmax[idim] - _xmin[idim];
                _diag += DX * DX;
            }
            _diag = std::sqrt(_diag);
                
            // response from approxData
            _fval[ipoint] = approxData.response_function(ipoint);
            
            if (_fval[ipoint] < _f_min) _f_min = _fval[ipoint];
            if (_fval[ipoint] > _f_max) _f_max = _fval[ipoint];
        }
        
        _diag = std::sqrt((double)_n_dim);
        
        // =======================
        // Gradients and Hessians
        std::cout << std::endl;
        std::cout << "-- Gradients & Hessians --" << std::endl;
        
        // Retrieve function gradients: If first point has gradient, I am assuming all points have gradients
        if (_use_derivatives && approxData.response_active_bits(0) & 2)
        {
            _use_gradient = true;
            _fgrad = new double*[_num_inserted_points];
            for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
            {
                RealVector fn_grad = approxData.response_gradient(ipoint);
                _fgrad[ipoint] = new double[_n_dim];
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    _fgrad[ipoint][idim] = fn_grad[idim] * (_xmax[idim] - _xmin[idim]);
                }
            }
            std::cout<< ".: VPS :.   Using gradient information." << std::endl;
        }
        else
        {
            _use_gradient = false;
            std::cout<< ".: VPS :.   Not using gradient information." << std::endl;
        }
        
        // Retrieve function hessians: If first point has hessian, I am assuming all points have hessians
        if (_use_derivatives && approxData.response_active_bits(0) & 4)
        {
            _use_hessian = true;
            _fhess = new double**[_num_inserted_points];
            for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
            {
                RealSymMatrix fn_hessian = approxData.response_hessian(ipoint);
                _fhess[ipoint] = new double*[_n_dim];
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    _fhess[ipoint][idim] = new double[_n_dim];
                    for (size_t jdim = 0; jdim < _n_dim; jdim++)
                    {
                        _fhess[ipoint][idim][jdim] = fn_hessian(idim, jdim) * (_xmax[idim] - _xmin[idim]) * (_xmax[jdim] - _xmin[jdim]);
                    }
                }
            }
            std::cout<< ".: VPS :.   Using hessian information." << std::endl;
        }
        else
        {
            _use_hessian = false;
            std::cout<< ".: VPS :.   Not using hessian information." << std::endl;
        }
        
        // =====
        // scale input domain to be a unit box
        for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
        {
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                _sample_points[ipoint][idim] = (_sample_points[ipoint][idim] - _xmin[idim]) / (_xmax[idim] - _xmin[idim]);
            }
        }
        // =====
        
        #endif
        
        
        if (_vps_subsurrogate == LS)
        {
            if (_vps_subsurrogate_basis == polynomial)
            {
                // retrive powers of the polynomial expansion
                _vps_t = new size_t**[_num_inserted_points];
                for (size_t icell = 0; icell < _num_inserted_points; icell++)
                {
                    retrieve_permutations(_num_cell_basis_functions[icell], _vps_t[icell], _n_dim, _vps_order, true, _vps_order);
                }
            }
            _vps_w = new double*[_num_inserted_points];
        }
        else if (_vps_subsurrogate == GP)
        {
            // create the data to configure the surrogate
            // String approx_type("global_gaussian");  // Dakota GP
            String approx_type;
            
            approx_type = "global_kriging";  // Surfpack GP
            
            UShortArray approx_order;
            short data_order = 1;  // assume only function values
            short output_level = QUIET_OUTPUT;
            
            sharedData = SharedApproxData(approx_type, approx_order, _n_dim, data_order, output_level);
            
            // build one gp for each Voronoi Cell
            for (size_t i = 0; i < _num_inserted_points; ++i)
            {
                gpApproximations.push_back(Approximation(sharedData));
            }
        }
        else
        {
            std::cout<< ".: VPS :.   ERROR! Unknown Surrogate Type! " << std::endl;
        }
    }
    
    void VPSApproximation::VPS_retrieve_neighbors(size_t ipoint, bool update_point_neighbors)
    {
        if (_num_inserted_points == 1)
        {
            _sample_vsize[0] = 0.5 * _diag;
            return;
        }
        
        size_t* tmp_neighbors = new size_t[_num_inserted_points];
        
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
            
            // trim line spoke with domain boundaries (0-1)
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
                    double t = _sample_points[ipoint][idim] / (_sample_points[ipoint][idim] - tmp_pnt[idim]);
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
            
            double h(0.0);
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double dx = _sample_points[ipoint][idim] - _sample_points[ineighbor][idim];
                h += dx * dx;
            }
            h = std::sqrt(h);
            
            double jump = fabs(_fval[ipoint] - _fval[ineighbor]);
            double grad = fabs(_fval[ipoint] - _fval[ineighbor]) / h;
            
            if ((jump > _disc_min_jump) || (grad > _disc_min_grad)) new_neighbor = false; // gradient of two point exceeds threshold

            
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
        
        if (_sample_neighbors[ipoint] != 0)
        {
            delete[] _sample_neighbors[ipoint];
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
                VPS_retrieve_neighbors(tmp_neighbors[i], false);
            }
        }
        delete[] tmp_neighbors;
    }


    void VPSApproximation::VPS_adjust_extend_neighbors_of_all_points()
    {
        for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
        {
            while (_vps_ext_neighbors[ipoint][0] < 2 * _num_cell_basis_functions[ipoint] && _vps_ext_neighbors[ipoint][0] < _num_inserted_points - 1)
            {
                size_t num_old_neighbors = _vps_ext_neighbors[ipoint][0];
                VPS_extend_neighbors(ipoint);
                
                size_t num_new_neighbors = _vps_ext_neighbors[ipoint][0];
                if (num_old_neighbors == num_new_neighbors) break; // No more neighbors to add
            }
        }
    }
    
    void VPSApproximation::VPS_extend_neighbors(size_t ipoint)
    {
        // initiate tmp_neighbors with old extended neighbors
        size_t num_ext_neigbors = _vps_ext_neighbors[ipoint][0];
        size_t* tmp_neighbors = new size_t[_num_inserted_points];
        for (size_t i = 1; i <= _vps_ext_neighbors[ipoint][0]; i++) tmp_neighbors[i - 1] = _vps_ext_neighbors[ipoint][i];
        
        for (size_t i = 1; i <= _vps_ext_neighbors[ipoint][0]; i++)
        {
            size_t neighbor = _vps_ext_neighbors[ipoint][i];
            for (size_t j = 1; j <= _sample_neighbors[neighbor][0]; j++) // extend neighbors of ipoint using direct neighbors of neighbor
            {
                size_t ext_neighbor = _sample_neighbors[neighbor][j];
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
        
        if (_vps_ext_neighbors[ipoint] != 0) delete[] _vps_ext_neighbors[ipoint];
        _vps_ext_neighbors[ipoint] = new size_t[num_ext_neigbors + 1];
        _vps_ext_neighbors[ipoint][0] = num_ext_neigbors;
        for (size_t i = 0; i < num_ext_neigbors; i++)
        {
            _vps_ext_neighbors[ipoint][i + 1] = tmp_neighbors[i];
        }
        delete[] tmp_neighbors;
    }
    
    
    void VPSApproximation::VPS_build_local_surrogate(size_t cell_index)
    {
        if (_vps_subsurrogate == LS)
        {
            VPS_LS_retrieve_weights(cell_index);
        }
        else if (_vps_subsurrogate == GP)
        {
            const SDVArray& training_vars = approxData.variables_data();
            const SDRArray& training_resp = approxData.response_data();
            
            for (size_t j = 0; j <= _vps_ext_neighbors[cell_index][0]; j++) // loop over neighbors
            {
                size_t neighbor = cell_index;
                if (j > 0) neighbor = _vps_ext_neighbors[cell_index][j];
                
                gpApproximations[cell_index].add(training_vars[neighbor], false);
                
                gpApproximations[cell_index].add(training_resp[neighbor], false);
            }
            gpApproximations[cell_index].build();
        }
        else
        {
            std::cout<< ".: VPS :.   ERROR! Unknown Surrogate Type! " << std::endl;
        }
    }
    
    double VPSApproximation::VPS_evaluate_surrogate(double* x)
    {
        double* x_vps = new double[_n_dim];
        for (size_t idim = 0; idim < _n_dim; idim++) x_vps[idim] = (x[idim] - _xmin[idim]) / (_xmax[idim] - _xmin[idim]);
        
        size_t iclosest = retrieve_closest_cell(x_vps);
        
        if (_vps_subsurrogate == LS)
        {
            // LS Surrogate
            double f_VPS = 0.0;
            for (size_t ibasis = 0; ibasis < _num_cell_basis_functions[iclosest]; ibasis++)
            {
                double wi = _vps_w[iclosest][ibasis];
                
                double yi = evaluate_basis_function(x_vps, iclosest, ibasis);
                f_VPS += wi * yi;
                
                //if ((fabs(x[0] - 1) < 1E-10) && (fabs(x[1] - 1) < 1E-10))
                //{
                //    std::cout<< "(x,y)[iclosest] = (" << _sample_points[iclosest][0] << "," << _sample_points[iclosest][1] << ")" << std::endl;
                //    std::cout<< "f[iclosest] = " << _fval[iclosest] << std::endl;
                //    std::cout << "Weight = " << wi << std::endl;
                //    std::cout << "Poly term = " << yi << std::endl;
                //}
                    
            }
            delete [] x_vps;
            return f_VPS;
        }
        else if (_vps_subsurrogate == GP)
        {
            // GP Surrogate
            
            RealVector c_vars(Teuchos::View, const_cast<Real*>(x_vps), _n_dim);
            
            double f_gp = gpApproximations[iclosest].value(c_vars);
            
            delete [] x_vps;
            return f_gp;
        }
        else
        {
            std::cout<< ".: VPS :.   ERROR! Unknown Surrogate Type! " << std::endl;
        }
        delete[] x_vps;
        return 0.0;
    }
    
    
    void VPSApproximation::VPS_destroy_global_containers()
    {
        delete[] _xmin;
        delete[] _xmax;
        delete[] _fval;
        delete[] _sample_vsize;
        
        for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
        {
            delete[] _sample_points[ipoint];
            delete[] _sample_neighbors[ipoint];
            delete[] _vps_ext_neighbors[ipoint];
            
            if (_vps_subsurrogate == LS)
            {
                delete[] _vps_w[ipoint];
            }
        }
        delete[] _sample_points;
        delete[] _sample_neighbors;
        delete[] _vps_ext_neighbors;
        
        if (_vps_subsurrogate == LS)
        {
            delete[] _vps_w;
            if (_vps_subsurrogate_basis == polynomial)
            {
                delete[] _vps_dfar;
                for (size_t icell = 0; icell < _num_inserted_points; icell++)
                {
                    for (size_t ibasis = 0; ibasis < _num_cell_basis_functions[icell]; ibasis++)
                    {
                        delete[] _vps_t[icell][ibasis];
                    }
                    delete[] _vps_t[icell];
                }
                delete[] _vps_t;
            }
            else if (_vps_subsurrogate_basis == radial)
            {
                for (size_t icell = 0; icell < _num_inserted_points; icell++)
                {
                    size_t num_basis = _num_cell_basis_functions[icell];
                    for (size_t ibasis = 0; ibasis < num_basis; ibasis++)
                    {
                        delete[] _sample_basis[icell][ibasis];
                    }
                    delete[] _sample_basis[icell];
                }
                delete[] _sample_basis;
            }
            delete[] _num_cell_basis_functions;
        }
        else if (_vps_subsurrogate == LS)
        {
            gpApproximations.clear();
        }
    }

    
    //////////////////////////////////////////////////////////////
    // Least Square Sub Surrogate METHODS
    //////////////////////////////////////////////////////////////
    
    void VPSApproximation::retrieve_permutations(size_t &m, size_t** &perm, size_t num_dim, size_t upper_bound,
                                                 bool force_sum_constraint, size_t sum_constraint)
    {
        size_t* t = new size_t[num_dim];
        for (size_t idim = 0; idim < num_dim; idim++) t[idim] = 0;
        
        // count output
        m = 0;
        size_t k_dim(num_dim - 1);
        while (true)
        {
            while (t[k_dim] <= upper_bound)
            {
                bool valid(true);
                
                if (force_sum_constraint)
                {
                    size_t s_const(0);
                    for (size_t idim = 0; idim < num_dim; idim++) s_const += t[idim];
                    
                    if (s_const > upper_bound)
                    {
                        valid = false;
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
        
        //std::cout<< "Number of permutations = " << m << std::endl;
        
        perm = new size_t*[m];
        for (size_t i = 0; i < m; i++)
        {
            perm[i] = new size_t[num_dim];
            for (size_t idim = 0; idim < num_dim; idim++) perm[i][idim] = 0;
        }
        
        // Taxi Counter to evaluate the surrogate
        for (size_t idim = 0; idim < num_dim; idim++) t[idim] = 0;
        k_dim = num_dim - 1;
        m = 0; // index of alpha
        
        while (true)
        {
            while (t[k_dim] <= upper_bound)
            {
                bool valid(true);
                
                if (force_sum_constraint)
                {
                    size_t s_const(0);
                    for (size_t idim = 0; idim < num_dim; idim++) s_const += t[idim];
                    
                    if (s_const > upper_bound)
                    {
                        valid = false;
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
        
        // re-order perm:
        size_t num_basis = m;
        //std::cout << "*** VPS:: num_basis = " << num_basis << std::endl;
        if (num_basis >= _n_dim + 1)
        {
            // reorder first order terms
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                // search fot the corresponding basis and bring it to its proper location
                for (size_t ibasis = 0; ibasis < num_basis; ibasis++)
                {
                    size_t sum(0), sig_dim(0);
                    for (size_t jdim = 0; jdim < _n_dim; jdim++)
                    {
                        sum += perm[ibasis][jdim];
                        if (perm[ibasis][jdim] == 1) sig_dim = jdim;
                    }
                    if (sum == 1 && sig_dim == idim)
                    {
                        // move this basis to location idim + 1
                        size_t* tmp = perm[ibasis];
                        perm[ibasis] = perm[idim + 1];
                        perm[idim + 1] = tmp;
                        break;
                    }
                }
            }
        }
        
        if (num_basis >= _n_dim * (_n_dim + 1) / 2 + _n_dim + 1)
        {
            // reorder second order terms
            size_t iloc(_n_dim + 1);
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                for (size_t jdim = idim; jdim < _n_dim; jdim++)
                {
                    // search fot the corresponding basis and bring it to its proper location
                    for (size_t ibasis = 0; ibasis < num_basis; ibasis++)
                    {
                        size_t sum(0), sig_dim_i(_n_dim), sig_dim_j(_n_dim);
                        for (size_t kdim = 0; kdim < _n_dim; kdim++)
                        {
                            sum += perm[ibasis][kdim];
                            if (perm[ibasis][kdim] > 0 && sig_dim_i == _n_dim)
                            {
                                sig_dim_i = kdim;
                                sig_dim_j = sig_dim_i;
                            }
                            else if (perm[ibasis][kdim] > 0 && sig_dim_i < _n_dim) sig_dim_j = kdim;
                        }
                        if (sum == 2 && sig_dim_i == idim && sig_dim_j == jdim)
                        {
                            // move this basis to location iloc
                            size_t* tmp = perm[ibasis];
                            perm[ibasis] = perm[iloc];
                            perm[iloc] = tmp; iloc++;
                            break;
                        }
                    }
                }
            }
        }
        
        /*
        for (size_t ibasis = 0; ibasis < num_basis; ibasis++)
        {
            for (size_t idim = 0; idim < num_dim; idim++) std::cout << perm[ibasis][idim] << " ";
            std::cout << std::endl;
        }
        */
        
        
        delete[] t;
    }
    
    
    void VPSApproximation::build_radial_basis_function(size_t icell)
    {
        std::vector<double*> rbfs;
        
        double* dart = new double[_n_dim];
        double r = _sample_vsize[icell];
        
        // initial basis lies at the seed of the cell
        for (size_t idim = 0; idim < _n_dim; idim++) dart[idim] = _sample_points[icell][idim];
        
        size_t num_basis = 0;
        // Adding first basis
        rbfs.push_back(dart);
        dart = new double[_n_dim];
        num_basis++;
        
        size_t num_successive_misses(0), max_num_successive_misses(100);
        size_t iactive = 0;
        while (true)
        {
            // sample new basis (points from the surface of the active ball
            
            // sample point uniformly from an r sphere
            double sf = 0.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double sum(0.0);
                // select 12 random numbers from 0.0 to 1.0
                for (size_t i = 0; i < 12; i++) sum += generate_a_random_number();
                sum -= 6.0;
                dart[idim] = sum;
                sf += dart[idim] * dart[idim];
            }
            sf = r / std::sqrt(sf);
            for (size_t idim = 0; idim < _n_dim; idim++) dart[idim] *= sf;

            // translate to match active cell origin
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                dart[idim]+= rbfs[iactive][idim];
            }
            
            bool valid_dart(true);
            for (size_t ibasis = 0; ibasis < num_basis; ibasis++)
            {
                double dst_sq = 0.0;
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    double dx = dart[idim] - rbfs[ibasis][idim];
                    dst_sq += dx * dx;
                }
                if (dst_sq < r * r - 1E-10)
                {
                    valid_dart = false; break;
                }
            }
            if (valid_dart)
            {
                num_successive_misses = 0;
                // add a new basis
                rbfs.push_back(dart);
                dart = new double[_n_dim];
                num_basis++;
            }
            else
            {
                num_successive_misses++;
                if (num_successive_misses == max_num_successive_misses) break; // iactive++; create only a single layer of basis functions around seed
            }
        }
        _num_cell_basis_functions[icell] = num_basis;
        _sample_basis[icell] = new double*[num_basis]; 
        for (size_t ibasis = 0; ibasis < num_basis; ibasis++) _sample_basis[icell][ibasis] = rbfs[ibasis];
        rbfs.clear();
        delete[] dart;
        return;
    }
    // =======================
    void VPSApproximation::VPS_LS_retrieve_weights(size_t cell_index)
    {
        size_t num_basis = _num_cell_basis_functions[cell_index];
        size_t num_basis_resolved(0);
        
        _vps_w[cell_index] = new double[num_basis];

        if (_use_derivatives && _use_gradient && surrogateOrder >= 1 && _vps_subsurrogate_basis == polynomial)
        {
            _vps_w[cell_index][0] = _fval[cell_index];
            num_basis_resolved++;
            
            double* gradf = _fgrad[cell_index];
            for (size_t idim = 1; idim <= _n_dim; idim++)
            {
                _vps_w[cell_index][idim] = gradf[idim - 1];
                num_basis_resolved++;
            }
            delete[] gradf;
        }
        
        if (_use_derivatives && _use_hessian && surrogateOrder >= 2 && _vps_subsurrogate_basis == polynomial)
        {
            size_t iloc = _n_dim + 1;
         
            double** H_f = _fhess[cell_index];
            
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                for (size_t jdim = idim; jdim < _n_dim; jdim++)
                {
                    _vps_w[cell_index][iloc] = H_f[idim][jdim];
                    
                    if (jdim == idim) _vps_w[cell_index][iloc] *= 0.5;
                    
                    num_basis_resolved++;
                    
                    iloc++;
                }
                delete[] H_f[idim];
            }
            
            delete[] H_f;
            
        }
        
        //std::cout << "*** num_basis = " << num_basis << std::endl;
        //std::cout << "*** num_basis_resolved = " << num_basis_resolved << std::endl;

        if (num_basis_resolved == num_basis)
        {
            // Case (enough info from evals/grads/hessians)
            // No neighbor tracking or regression is needed!
            return;
        }
        
        size_t num_neighbors =_vps_ext_neighbors[cell_index][0];

        if (num_basis > num_basis_resolved + num_neighbors + 1) num_basis = num_basis_resolved + num_neighbors + 1;
        
        // Case (not enough info from evals/grads/hessians)
        // Grab neighbors and perform regression
        
        // Extend system matrix to account for gradient and Hessian information
        // H * w = b
        
        double** H = new double*[num_basis]; // columns of the LS matrix
        size_t num_rows = num_basis_resolved + num_neighbors + 1;
        double*  b = new double[num_rows];
        
        for (size_t ibasis = 0; ibasis < num_basis; ibasis++)
        {
            H[ibasis] = new double[num_rows];
            
            for (size_t irow = 0; irow < num_basis_resolved; irow++)
            {
                if (ibasis == irow) H[ibasis][irow] = 1.0;
                else                H[ibasis][irow] = 0.0;
            }
            
            for (size_t irow = num_basis_resolved; irow < num_rows; irow++)
            {
                size_t neighbor = cell_index;
                if (irow > num_basis_resolved) neighbor = _vps_ext_neighbors[cell_index][irow - num_basis_resolved];
                
                H[ibasis][irow] = evaluate_basis_function(_sample_points[neighbor], cell_index, ibasis);
            }
        }
        
        // Extend function evals to account for gradient and Hessian information
        for (size_t irow = 0; irow < num_basis_resolved; irow++)
        {
            b[irow] = _vps_w[cell_index][irow];
        }
        
        for (size_t irow = num_basis_resolved; irow < num_rows; irow++)
        {
            size_t neighbor = cell_index;
            if (irow > num_basis_resolved) neighbor = _vps_ext_neighbors[cell_index][irow - num_basis_resolved];
            b[irow] = _fval[neighbor];
        }
        
        //////////////////////////////////////////////////////////////////////////////////

        if (num_basis_resolved > 0)
        {
            // std::cout << "******* Number of bases resvoled " << num_basis_resolved << std::endl;
            // Case (Some coefficients have been already found)
            // Reconstruct "reduced system", and solve for the rest of coefficients

            
            // Unkown w coefficients --> f2 = f2 - H3 * w1
            for (size_t irow = num_basis_resolved; irow < num_rows; irow++)
            {
                for (size_t ibasis = 0; ibasis < num_basis_resolved; ibasis++)
                {
                    b[irow] -= H[ibasis][irow] * _vps_w[cell_index][ibasis];
                }
            }
            
            for (size_t irow = num_basis_resolved; irow < num_rows; irow++)
            {
                for (size_t ibasis = 0; ibasis < num_basis_resolved; ibasis++)
                {
                    H[ibasis][irow] = 0.0;
                }
            }
        }
        //////////////////////////////////////////////////////////////////////////////////
        
        // Solve a constrined Least Squares problem
        
        constrained_LeastSquare(num_basis, num_neighbors + 1, H, _vps_w[cell_index], b);
        
        for (size_t ibasis = 0; ibasis < num_basis; ibasis++)
        {
            // std::cout << "******* Basis Coefficient " << _vps_w[cell_index][ibasis] << std::endl;
            delete[] H[ibasis];
        }
        delete[] H;
        delete[] b;
    }
    
    double VPSApproximation::evaluate_basis_function(double* x, size_t icell, size_t ibasis)
    {
        if (_vps_subsurrogate_basis == polynomial)
        {
            double* y = new double[_n_dim];
            // shift origin
            for (size_t idim = 0; idim < _n_dim; idim++) y[idim] = (x[idim] - _sample_points[icell][idim]);// / _vps_dfar[icell];
            double f_basis = vec_pow_vec(_n_dim, y, _vps_t[icell][ibasis]);
            delete[] y;
            return f_basis;
        }
        else if (_vps_subsurrogate_basis == radial)
        {
            double h  = _sample_vsize[icell];
            
            size_t basis_index = icell;
                       
            double dst_sq = 0.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double dx = x[idim] - _sample_basis[icell][ibasis][idim];
                dst_sq += dx * dx;
            }
            
            double r_basis = 4.0 * h;
            double r_sq = r_basis * r_basis;
            
            return exp(-dst_sq / r_sq);
        }
    }
    // =======================
    int VPSApproximation::constrained_LeastSquare(size_t n, size_t m, double** H, double* w, double* f)
    {
        bool constrained = true;
        // Solving H w = b while strictly satsifying the first equation
        // H is n columns each has m rows
        // w is n rows
        // f is m rows 
        
        if (fabs(H[0][0]) < 1E-10)
        {
            std::cout << ".: VPS :.   Contrained Least Square: Dividing by zero." << std::endl;
            return 1;
        }
        
        // taking away constrained equation
        double* crow = new double[n]; double fo = f[0];
        double* ccol = new double[m];
        if (constrained)
        {
            // storing constrained row
            for (size_t i = 0; i < n; i++)
            {
                crow[i] = H[i][0]; H[i][0] = 0.0;
            }
            // storing constrained row
            for (size_t j = 0; j < m; j++)
            {
                ccol[j] = H[0][j]; H[0][j] = 0.0;
            }
            ccol[0] = crow[0];
            
            // inserting constrained equation into remaining equations
            for (size_t j = 1; j < m; j++)
            {
                for (size_t i = 1; i < n; i++)
                {
                    H[i][j] -= ccol[j] * crow[i] / crow[0];
                }
                f[j] -= ccol[j] * fo / crow[0];
            }
            H[0][0] = 1.0; f[0] = 0.0;
        }
        
        //printMatrix(n, m, H);
        
        double** A = new double*[n];
        double** LD = new double*[n];
        double* b = new double[n];
        
        for (size_t row = 0; row < n; row++)
        {
            A[row] = new double[n];
            LD[row] = new double[n];
            for (size_t col = 0; col < n; col++)
            {
                A[row][col] = vec_dot_vec(m, H[row], H[col]);
            }
            b[row] = vec_dot_vec(m, H[row], f);
        }
        
        //printMatrix(n, n, A);
        
        if (Cholesky(n, A, LD))
        {
            Cholesky_solver(n, LD, b, w);
        }
        else
        {
            GMRES(n, A, b, w, 1E-6);
            _num_GMRES++;
        }
        
        // Applying constraint
        if (constrained)
        {
            double sum_constraint = vec_dot_vec(n, crow, w);
            w[0] = (fo - sum_constraint) / crow[0];
        
            for (size_t j = 0; j < m; j++)
            {
                H[0][j] = ccol[j];
                for (size_t i = 1; i < n; i++)
                {
                    H[i][j] += crow[i] / crow[0];
                }
                f[j] += fo / crow[0];
            }
            for (size_t i = 0; i < n; i++)
            {
                H[i][0] = crow[i];
            }
            f[0] = fo;
        }
        
        delete[] crow;
        delete[] ccol;
        
        for (size_t row = 0; row < n; row++)
        {
            delete[] A[row];
            delete[] LD[row];
        }
        delete[] A;
        delete[] LD;
        delete[] b;
        return 0;
    }
    
    void VPSApproximation::printMatrix(size_t m, size_t n, double** M)
    {
        for (size_t i = 0; i < m; i++)
        {
            for (size_t j = 0; j < n; j++) std::cout << M[i][j] << " ";
            std::cout << std::endl;
        }
        
    }
    

    double VPSApproximation::vec_dot_vec(size_t n, double* vec_a, double* vec_b)
    {
        double dot = 0.0;
        for (size_t i = 0; i < n; i++) dot+= vec_a[i] * vec_b[i];
        return dot;
    }
    
    double VPSApproximation::vec_pow_vec(size_t num_dim, double* vec_a, size_t* vec_b)
    {
        double ans = 1.0;
        for (size_t idim = 0; idim < num_dim; idim++)
        {
            ans *= pow(vec_a[idim], (int) vec_b[idim]);
        }
        return ans;
    }
    
    bool VPSApproximation::Cholesky(int n, double** A, double** LD)
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
                    LD[i][j] = std::sqrt(A[i][i] - sum);
                }
                else        LD[i][j] = 1.0 / LD[j][j] * (A[i][j] - sum);
                
                LD[j][i] = LD[i][j];
            }
        }
        return true;
    }
    
    void VPSApproximation::Cholesky_solver(int n, double** LD, double* b, double* x)
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
    
    void VPSApproximation::GMRES(size_t n, double** A, double* b, double* x, double eps)
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
        beta = std::sqrt(beta);
        
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
            wNorm = std::sqrt(wNorm);
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
            
            double delta = std::sqrt(pow(RT[im][im], 2) + pow(HT[im][im + 1], 2));
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
            rNorm = std::sqrt(rNorm);
            
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
    
    //////////////////////////////////////////////////////////////
    // General METHODS
    //////////////////////////////////////////////////////////////
    void VPSApproximation::initiate_random_number_generator(unsigned long x)
    {
        //assert(sizeof (double) >= 54) ;
        
        cc = 1.0 / 9007199254740992.0; // inverse of 2^53rd power
        size_t i;
        size_t qlen = indx = sizeof Q / sizeof Q[0];
        for (i = 0; i < qlen; i++) Q[i] = 0;
        
        double c = 0.0; zc = 0.0;	/* current CSWB and SWB `borrow` */
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
    
    double VPSApproximation::generate_a_random_number()
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
    
    size_t VPSApproximation::retrieve_closest_cell(double* x)
    {
        size_t iclosest = _num_inserted_points;
        double dmin = DBL_MAX;
        for (size_t ipoint = 0; ipoint < _num_inserted_points; ipoint++)
        {
            double dd = 0.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double dx = x[idim] - _sample_points[ipoint][idim];
                dd += dx * dx;
            }
            if (dd < dmin)
            {
                dmin = dd;
                iclosest = ipoint;
            }
        }
        return iclosest;
    }
    
    bool VPSApproximation::trim_line_using_Hyperplane(size_t num_dim,                               // number of dimensions
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
    // Debugging METHODS
    //////////////////////////////////////////////////////////////
    
    double VPSApproximation::f_test(double* x)
    {
        if (_vps_test_function == SmoothHerbie)
        {
            double fval = 1.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double xm = x[idim] - 1.0;
                double xp = x[idim] + 1.0;
                double wherb = exp(- xm * xm) + exp(-0.8 * xp * xp) ;
                fval *= wherb;
            }
            fval = -fval;
            return fval;
        }
        else if (_vps_test_function == Herbie)
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
        else if (_vps_test_function == Cone)
        {
            double fval = 0.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double xm = x[idim];
                fval += xm * xm;
            }
            fval = sqrt(fval);
            return fval;
        }
        else if (_vps_test_function == Cross)
        {
            double fval = 1.0;
            const double pi = 3.14159265358979324;
            double dpow = 1.0 / _n_dim;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                fval *= 0.5 * (1.0 + std::cos(2.0 * pi * x[idim]));
            }
            fval = std::pow(fval, dpow);
            return fval;
        }
        else if (_vps_test_function == UnitSphere)
        {
            // step function in sphere
            double h = 0.0;
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double dx = x[idim];
                h += dx * dx;
            }
            h = std::sqrt(h);
            //if (h < 0.5) return 1.0;
            if ((h < 0.5) || (h > 1.0)) return 1.0;
            return 0.0;
        }
        else if (_vps_test_function == Linear34)
        {
            // linear :: 3* x_1 + 4 * x_2 + \sum x_i
            double fval = 3.0 * x[0] + 4 * x[1];
            for (size_t idim = 2; idim < _n_dim; idim++)
            {
                fval += x[idim];
            }
            return fval;
        }
        return 0.0;
    }
    
    double* VPSApproximation::grad_f_test(double* x)
    {
        double eps = 1E-4;
        double* grad = new double[_n_dim];
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            x[idim] += eps;
            double fp = f_test(x);
            x[idim] -= 2 * eps;
            double fm = f_test(x);
            x[idim] += eps;
            grad[idim] = (fp - fm) / (2 * eps);
        }
        return grad;
    }
    
    double** VPSApproximation::hessian_f_test(double* x)
    {
        double eps = 1E-4;
        double** H = new double*[_n_dim];
        for (size_t idim = 0; idim < _n_dim; idim++)
            H[idim] = new double[_n_dim];
            
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            x[idim] += eps;
            double* gradi_fp = grad_f_test(x);
            x[idim] -= 2 * eps;
            double* gradi_fm = grad_f_test(x);
            x[idim] +=eps;
            
            for (size_t jdim = 0; jdim < _n_dim; jdim++)
            {
                H[idim][jdim] = (gradi_fp[jdim] - gradi_fm[jdim]) / (2 * eps);
            }
            delete[] gradi_fp; delete[] gradi_fm;
        }
        return H;
    }


    
    void VPSApproximation::generate_poisson_disk_sample(double r)
    {
        double r_sq = r * r;
        double* dart = new double[_n_dim];
        size_t num_points = 0;
        while (num_points < _num_inserted_points)
        {
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double u = generate_a_random_number();
                dart[idim] = _xmin[idim] + u * (_xmax[idim] - _xmin[idim]);
            }
            bool valid_dart(true);
            for (size_t ipoint = 0; ipoint < num_points; ipoint++)
            {
                double dst_sq(0.0);
                for (size_t idim = 0; idim < _n_dim; idim++)
                {
                    double dx = dart[idim] - _sample_points[ipoint][idim];
                    dst_sq += dx * dx;
                }
                //std::cout << "num_point = " << num_points << " , dst_sq = " << dst_sq << " , r_sq = "<< r_sq << std::endl;
                if (dst_sq < r_sq)
                {
                    valid_dart = false;
                    break;
                }
            }
            if (valid_dart)
            {
                //std::cout << "ipoint = " << num_points << " : x = ";
                //for (size_t idim = 0; idim < _n_dim; idim++) std::cout << dart[idim] << " ";
                //std::cout << std::endl;
                
                _sample_points[num_points] = dart;
                dart = new double[_n_dim];
                num_points++;
            }
        }
        delete[] dart;
    }
    
    void VPSApproximation::generate_MC_sample()
    {
        double* dart = new double[_n_dim];
        size_t num_points = 0;
        while (num_points < _num_inserted_points)
        {
            for (size_t idim = 0; idim < _n_dim; idim++)
            {
                double u = generate_a_random_number();
                dart[idim] = _xmin[idim] + u * (_xmax[idim] - _xmin[idim]);
            }
            _sample_points[num_points] = dart;
            dart = new double[_n_dim];
            num_points++;
        }
        delete[] dart;
    }

    
    void VPSApproximation::isocontouring(std::string file_name, bool plot_test_function, bool plot_surrogate, std::vector<double> contours)
    {
        std::fstream file(file_name.c_str(), std::ios::out);
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
        file << " gsave" << std::endl;
        file << " grestore" << std::endl;
        file << " 0 0 0 setrgbcolor" << std::endl;
        file << " 0.01 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        double* xx = new double[2];
        double sx = 0.001 * (_xmax[0] - _xmin[0]);
        double sy = 0.001 * (_xmax[1] - _xmin[1]);
        for (size_t i = 0; i < 1000; i++)
        {
            double xo = _xmin[0] + i * sx;
            for (size_t j = 0; j < 1000; j++)
            {
                for (size_t ifunc = 0; ifunc < 2; ifunc++)
                {
                    double fo(0.0), f1(0.0), f2(0.0), f3(0.0);
                    
                    double yo = _xmin[1] + j * sy;
                    xx[0] = xo; xx[1] = yo;
                    if (plot_test_function && ifunc != 1)  fo = f_test(xx);
                    else if (plot_surrogate) fo = VPS_evaluate_surrogate(xx);
                    xx[0] = xo+sx; xx[1] = yo;
                    if (plot_test_function && ifunc != 1)  f1 = f_test(xx);
                    else if (plot_surrogate) f1 = VPS_evaluate_surrogate(xx);
                    xx[0] = xo + sx; xx[1] = yo + sy;
                    if (plot_test_function && ifunc != 1)  f2 = f_test(xx);
                    else if (plot_surrogate) f2 = VPS_evaluate_surrogate(xx);
                    xx[0] = xo; xx[1] = yo + sy;
                    if (plot_test_function && ifunc != 1)  f3 = f_test(xx);
                    else if (plot_surrogate) f3 = VPS_evaluate_surrogate(xx);
                    
                    size_t num_isocontours = contours.size();
                    for (size_t icont = 0; icont < num_isocontours; icont++)
                    {
                        double contour = contours[icont];
                        
                        size_t num_points(0);
                        double x1, y1, x2, y2;
                        
                        if ((fo > contour && f1 < contour) || (fo < contour && f1 > contour))
                        {
                            double h = sx * (contour - fo) / (f1 - fo);
                            x1 = xo + h;
                            y1 = yo; num_points++;
                        }
                        if ((f1 > contour && f2 < contour) || (f1 < contour && f2 > contour))
                        {
                            double h = sy * (contour - f1) / (f2 - f1);
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
                        if ((f2 > contour && f3 < contour) || (f2 < contour && f3 > contour))
                        {
                            double h = sx * (contour - f2) / (f3 - f2);
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
                        if ((f3 > contour && fo < contour) || (f3 < contour && fo > contour))
                        {
                            double h = sy * (contour - f3) / (fo - f3);
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
                            if (ifunc == 0)
                            {
                                file << "newpath" << std::endl;
                                file << x1 * scale << " " << y1 * scale << " moveto" << std::endl;
                                file << x2 * scale << " " << y2 * scale << " lineto" << std::endl;
                                file << "closepath" << std::endl;
                                file << "gsave" << std::endl;
                                file << "grestore" << std::endl;
                                file << "0 0 0" << " setrgbcolor" << std::endl;
                                file << "0.02 setlinewidth" << std::endl;
                                file << "stroke" << std::endl;
                            }
                            else
                            {
                                double gs = icont * 1.0 / num_isocontours;
                                file << "newpath" << std::endl;
                                file << x1 * scale << " " << y1 * scale << " moveto" << std::endl;
                                file << x2 * scale << " " << y2 * scale << " lineto" << std::endl;
                                
                                file << "closepath" << std::endl;
                                file << "gsave" << std::endl;
                                file << "grestore" << std::endl;
                                
                                double r, g, b;
                                
                                if (gs < 0.25)     r = 1.0;
                                else if (gs < 0.5) r = 1.0 - 16.0 * (gs - 0.25) * (gs - 0.25);
                                else               r = 0.0;
                                
                                double go(0.25), gn(1.0 - go);
                                if (gs < go)      g = gs / go;
                                else if (gs < gn) g = 1.0;
                                else              g = 1.0 / (1.0 - gn) - gs / (1.0 - gn);
                                
                                if (gs < 0.5)       b = 0.0;
                                else if (gs < 0.75) b = 1.0 - 16.0 * (gs - 0.75) * (gs - 0.75);
                                else                b = 1.0;
                                
                                file << r << " " << g << " " << b << " setrgbcolor" << std::endl;
                                
                                file << "0.02 setlinewidth" << std::endl;
                                file << "stroke" << std::endl;
                            }
                        }
                    }
                }
            }
        }
        
        delete[] xx;
        
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
    
    void VPSApproximation::isocontouring_solid(std::string file_name, bool plot_test_function, bool plot_surrogate, std::vector<double> contours)
    {
        
        //std::cout << ".: VPS Debug Mode :. Plotting ps files .... " << std::endl;
        
        std::fstream file(file_name.c_str(), std::ios::out);
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
        file << " gsave" << std::endl;
        file << " grestore" << std::endl;
        file << " 0 0 0 setrgbcolor" << std::endl;
        file << " 0.01 setlinewidth" << std::endl;
        file << " stroke" << std::endl;
        file << "} def" << std::endl;
        
        std::vector<double> poly_x;
        std::vector<double> poly_y;
        
        size_t num_cells(100);
        double* xx = new double[2];
        
        //xx[0] = 1.0; xx[1] = 1.0;
        //double ff = VPS_evaluate_surrogate(xx);
        //std::cout << " Function value at origin = " << ff << std::endl;

        double sx = 1.0 * (_xmax[0] - _xmin[0]) / num_cells;
        double sy = 1.0 * (_xmax[1] - _xmin[1]) / num_cells;
        for (size_t i = 0; i < num_cells; i++)
        {
            double xo = _xmin[0] + i * sx;
            for (size_t j = 0; j < num_cells; j++)
            {
                double fo(0.0), f1(0.0), f2(0.0), f3(0.0);
                
                double yo = _xmin[1] + j * sy;
                xx[0] = xo; xx[1] = yo;
                if (plot_test_function)  fo = f_test(xx);
                else                     fo = VPS_evaluate_surrogate(xx);
                xx[0] = xo+sx; xx[1] = yo;
                if (plot_test_function)  f1 = f_test(xx);
                else                     f1 = VPS_evaluate_surrogate(xx);
                xx[0] = xo + sx; xx[1] = yo + sy;
                if (plot_test_function)  f2 = f_test(xx);
                else                     f2 = VPS_evaluate_surrogate(xx);
                xx[0] = xo; xx[1] = yo + sy;
                if (plot_test_function)  f3 = f_test(xx);
                else                     f3 = VPS_evaluate_surrogate(xx);
                
                //std::cout << "fo = " << fo << " , f1 = " << f1 << " , f2 = " << f2 << " , f3 = " << f3 << std::endl;
                
                size_t num_isocontours = contours.size();
                for (size_t icont = 0; icont < num_isocontours; icont++)
                {
                    double contour = contours[icont];
                    double contour_m = - 1000.00;
                    if (icont > 0) contour_m = contours[icont - 1];
                    
                    //std::cout<< "contour_m = " << contour_m << " , contour = " << contour << std::endl;
                    
                    poly_x.clear(); poly_y.clear();
                    
                    // moving right
                    if (fo >= contour_m - 1E-10 && fo < contour + 1E-10)
                    {
                        poly_x.push_back(xo);
                        poly_y.push_back(yo);
                        if ((fo > contour && f1 < contour) || (fo < contour && f1 > contour))
                        {
                            double h = sx * (contour - fo) / (f1 - fo);
                            poly_x.push_back(xo + h);
                            poly_y.push_back(yo);
                        }
                        else if ((fo > contour_m && f1 < contour_m) || (fo < contour_m && f1 > contour_m))
                        {
                            double h = sx * (contour_m - fo) / (f1 - fo);
                            poly_x.push_back(xo + h);
                            poly_y.push_back(yo);
                        }
                    }
                    else if ((fo > contour_m && f1 < contour_m) || (fo < contour_m && f1 > contour_m))
                    {
                        double hm = sx * (contour_m - fo) / (f1 - fo);
                        double h = hm;
                        if ((fo > contour && f1 < contour) || (fo < contour && f1 > contour))
                        {
                            h = sx * (contour - fo) / (f1 - fo);
                        }
                        if (h < hm)
                        {
                            double tmp = h; h = hm; hm = tmp;
                        }
                        poly_x.push_back(xo + hm);
                        poly_y.push_back(yo);
                        
                        if (h - hm > 1E-10)
                        {
                            poly_x.push_back(xo + h);
                            poly_y.push_back(yo);
                        }
                    }
                    else if ((fo > contour && f1 < contour) || (fo < contour && f1 > contour))
                    {
                        double h = sx * (contour - fo) / (f1 - fo);
                        poly_x.push_back(xo + h);
                        poly_y.push_back(yo);
                    }
                    
                    // moving up
                    if (f1 >= contour_m - 1E-10 && f1 < contour + 1E-10)
                    {
                        poly_x.push_back(xo + sx);
                        poly_y.push_back(yo);
                        if ((f1 > contour && f2 < contour) || (f1 < contour && f2 > contour))
                        {
                            double h = sy * (contour - f1) / (f2 - f1);
                            poly_x.push_back(xo + sx);
                            poly_y.push_back(yo + h);
                        }
                        else if ((f1 > contour_m && f2 < contour_m) || (f1 < contour_m && f2 > contour_m))
                        {
                            double h = sy * (contour_m - f1) / (f2 - f1);
                            poly_x.push_back(xo + sx);
                            poly_y.push_back(yo + h);
                        }
                        
                    }
                    else if ((f1 > contour_m && f2 < contour_m) || (f1 < contour_m && f2 > contour_m))
                    {
                        double hm = sy * (contour_m - f1) / (f2 - f1);
                        double h = hm;
                        if ((f1 > contour && f2 < contour) || (f1 < contour && f2 > contour))
                        {
                            h = sy * (contour - f1) / (f2 - f1);
                        }
                        if (h < hm)
                        {
                            double tmp = h; h = hm; hm = tmp;
                        }
                        poly_x.push_back(xo + sx);
                        poly_y.push_back(yo + hm);
                        
                        if (h - hm > 1E-10)
                        {
                            poly_x.push_back(xo + sx);
                            poly_y.push_back(yo + h);
                        }
                    }
                    else if ((f1 > contour && f2 < contour) || (f1 < contour && f2 > contour))
                    {
                        double h = sy * (contour - f1) / (f2 - f1);
                        poly_x.push_back(xo + sx);
                        poly_y.push_back(yo + h);
                    }
                    
                    // moving left
                    if (f2 >= contour_m - 1E-10 && f2 < contour + 1E-10)
                    {
                        poly_x.push_back(xo + sx);
                        poly_y.push_back(yo + sy);
                        if ((f2 > contour && f3 < contour) || (f2 < contour && f3 > contour))
                        {
                            double h = sx * (contour - f2) / (f3 - f2);
                            poly_x.push_back(xo + sx - h);
                            poly_y.push_back(yo + sy);
                        }
                        else if ((f2 > contour_m && f3 < contour_m) || (f2 < contour_m && f3 > contour_m))
                        {
                            double h = sx * (contour_m - f2) / (f3 - f2);
                            poly_x.push_back(xo + sx - h);
                            poly_y.push_back(yo + sy);
                        }
                    }
                    else if ((f2 > contour_m && f3 < contour_m) || (f2 < contour_m && f3 > contour_m))
                    {
                        double hm = sx * (contour_m - f2) / (f3 - f2);
                        double h = hm;
                        if ((f2 > contour && f3 < contour) || (f2 < contour && f3 > contour))
                        {
                            h = sx * (contour - f2) / (f3 - f2);
                        }
                        if (h < hm)
                        {
                            double tmp = h; h = hm; hm = tmp;
                        }
                        poly_x.push_back(xo + sx - hm);
                        poly_y.push_back(yo + sy);
                        
                        if (h - hm > 1E-10)
                        {
                            poly_x.push_back(xo + sx - h);
                            poly_y.push_back(yo + sy);
                        }
                    }
                    else if ((f2 > contour && f3 < contour) || (f2 < contour && f3 > contour))
                    {
                        double h = sx * (contour - f2) / (f3 - f2);
                        poly_x.push_back(xo + sx - h);
                        poly_y.push_back(yo + sy);
                    }
                    
                    // moving down
                    if (f3 >= contour_m - 1E-10 && f3 < contour + 1E-10)
                    {
                        poly_x.push_back(xo);
                        poly_y.push_back(yo + sy);
                        if ((f3 > contour && fo < contour) || (f3 < contour && fo > contour))
                        {
                            double h = sy * (contour - f3) / (fo - f3);
                            poly_x.push_back(xo);
                            poly_y.push_back(yo + sy - h);
                        }
                        else if ((f3 > contour_m && fo < contour_m) || (f3 < contour_m && fo > contour_m))
                        {
                            double h = sy * (contour_m - f3) / (fo - f3);
                            poly_x.push_back(xo);
                            poly_y.push_back(yo + sy - h);
                        }
                    }
                    else if ((f3 > contour_m && fo < contour_m) || (f3 < contour_m && fo > contour_m))
                    {
                        double hm = sy * (contour_m - f3) / (fo - f3);
                        double h = hm;
                        if ((f3 > contour && fo < contour) || (f3 < contour && fo > contour))
                        {
                            h = sy * (contour - f3) / (fo - f3);
                        }
                        if (h < hm)
                        {
                            double tmp = h; h = hm; hm = tmp;
                        }
                        poly_x.push_back(xo);
                        poly_y.push_back(yo + sy - hm);
                        
                        if (h - hm > 1E-10)
                        {
                            poly_x.push_back(xo);
                            poly_y.push_back(yo + sy - h);
                        }
                    }
                    else if ((f3 > contour && fo < contour) || (f3 < contour && fo > contour))
                    {
                        double h = sy * (contour - f3) / (fo - f3);
                        poly_x.push_back(xo);
                        poly_y.push_back(yo + sy - h);
                    }
                    
                    
                    size_t num_corners(poly_x.size());
                    if (num_corners > 1)
                    {
                        double gs = 1.0 - icont * 1.0 / num_isocontours;
                        file << "newpath" << std::endl;
                        file << poly_x[0] * scale << " " << poly_y[0] * scale << " moveto" << std::endl;
                        //std::cout<< "*** x = " <<  poly_x[0] << ", y = " << poly_y[0] << std::endl;
                        for (size_t icorner = 1; icorner < num_corners; icorner++)
                        {
                            file << poly_x[icorner] * scale << " " << poly_y[icorner] * scale << " lineto" << std::endl;
                            //std::cout << "*** x = " <<  poly_x[icorner] << ", y = " << poly_y[icorner] << std::endl;
                        }
                        //std::cout << std::endl;
                        
                        file << "closepath" << std::endl;
                        file << "gsave" << std::endl;
                        file << "grestore" << std::endl;
                        
                        double r, g, b;
                        
                        if (gs < 0.25)     r = 1.0;
                        else if (gs < 0.5) r = 1.0 - 16.0 * (gs - 0.25) * (gs - 0.25);
                        else               r = 0.0;
                        
                        double go(0.25), gn(1.0 - go);
                        if (gs < go)      g = gs / go;
                        else if (gs < gn) g = 1.0;
                        else              g = 1.0 / (1.0 - gn) - gs / (1.0 - gn);
                        
                        if (gs < 0.5)       b = 0.0;
                        else if (gs < 0.75) b = 1.0 - 16.0 * (gs - 0.75) * (gs - 0.75);
                        else                b = 1.0;
                        
                        file << r << " " << g << " " << b << " setrgbcolor" << std::endl;
                        file << " fill" << std::endl;
                    }
                }
            }
        }
        delete[] xx;
        
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
    
    
    void VPSApproximation::plot_neighbors()
    {
        std::stringstream ss;
        ss << "vps_neighbors.ps";
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
        file << " 0.005 setlinewidth" << std::endl;
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
        
        for (size_t isample = 0; isample < _num_inserted_points; isample++)
        {
            size_t num_neighbors = 0;
            if (_sample_neighbors[isample] != 0) num_neighbors = _sample_neighbors[isample][0];
            
            for (size_t i = 1; i <= num_neighbors; i++)
            {
                size_t neighbor = _sample_neighbors[isample][i];
                double sample_x = _xmin[0] + _sample_points[isample][0] * (_xmax[0] - _xmin[0]);
                double sample_y = _xmin[1] + _sample_points[isample][1] * (_xmax[1] - _xmin[1]);
                double neighbor_x = _xmin[0] + _sample_points[neighbor][0] * (_xmax[0] - _xmin[0]);
                double neighbor_y = _xmin[1] + _sample_points[neighbor][1] * (_xmax[1] - _xmin[1]);
                
                // draw a line between isample and neighbor
                file << sample_x * scale << "  " << sample_y * scale << "  ";
                file << neighbor_x * scale << "  " << neighbor_y * scale << "  ";
                file << "blueseg"     << std::endl;
            }
        }
        
        double s(0.002 * _diag);
        for (size_t index = 0; index < _num_inserted_points; index++)
        {
            // plot vertex
            double sample_x = _xmin[0] + _sample_points[index][0] * (_xmax[0] - _xmin[0]);
            double sample_y = _xmin[1] + _sample_points[index][1] * (_xmax[1] - _xmin[1]);

            file << sample_x * scale << "  " << sample_y * scale << "  " << s * scale << " ";
            file << "blackfcirc" << std::endl; // non-failure disk
        }
        
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
    
    //////////////////////////////////////////////////////////////////////////////////////////
    ////// Inherited method from parent class
    //////////////////////////////////////////////////////////////////////////////////////////


    int VPSApproximation::min_coefficients() const
    {
        // min number of samples required to build the network is equal to
        // the number of design variables + 1

        // Note: Often this is too few samples.  It is better to have about
        // O(n^2) samples, where 'n' is the number of variables.

        return sharedDataRep->numVars + 1;
    }

    int VPSApproximation::num_constraints() const
    {
        return (approxData.anchor()) ? 1 : 0;
    }



    void VPSApproximation::build()
    {
        // base class implementation checks data set against min required
        Approximation::build();

        // Build a VPS surrogate model using the sampled data
        VPS_execute();
    }


    Real VPSApproximation::value(const Variables& vars)
    {
        
        VPSmodel_apply(vars.continuous_variables(),false,false);
        return approxValue;
    }


    const RealVector& VPSApproximation::gradient(const Variables& vars)
    {
        VPSmodel_apply(vars.continuous_variables(),false,true);
        return approxGradient;
    }


    Real VPSApproximation::prediction_variance(const Variables& vars)
    {
        VPSmodel_apply(vars.continuous_variables(),true,false);
        return approxVariance;
    }



    void VPSApproximation::VPSmodel_build()
    {
        
    }

    void VPSApproximation::VPSmodel_apply(const RealVector& approx_pt, bool variance_flag, bool gradients_flag)
    {
        double* x = new double[_n_dim];
    
        for (size_t idim = 0; idim < _n_dim; idim++)
        {
            x[idim] = approx_pt[idim];
        }
        approxValue = VPS_evaluate_surrogate(x);
        delete[] x;
    }


} // namespace Dakota
