.. _inputspec-main:

"""""""""""""""""
Dakota Input Spec
"""""""""""""""""

.. _inputspec-keywords:

===============
Dakota Keywords
===============

Valid Dakota input keywords are dictated by dakota.xml, included in source and binary distributions of Dakota. This specification file is used with the NIDR[29] parser to validate user input and is therefore the definitive source for input syntax, capability options, and optional and required capability sub-parameters for any given Dakota version. A more readable variant of the specification dakota.input.summary is also distributed.

While complete, users may find dakota.input.summary overwhelming or confusing and will likely derive more benefit from adapting example input files to a particular problem. Some examples can be found here: Sample Input Files. Advanced users can master the many input specification possibilities by understanding the structure of the input specification file.

.. _inputspec-overview:

===================
Input Spec Overview
===================

Refer to the dakota.input.summary file, in Input Spec Summary, for all current valid input keywords.

- The summary describes every keyword including:

  - Whether it is required or optional
  - Whether it takes ARGUMENTS (always required) Additional notes about ARGUMENTS can be found here: Specifying Arguments.
  - Whether it has an ALIAS, or synonym
  - Which additional keywords can be specified to change its behavior
  
- Additional details and descriptions are described in Keywords Area
- For additional details on NIDR specification logic and rules, refer to[29] (Gay, 2008).

.. _inputspec-summary:

==================
Input Spec Summary
==================

This file is derived automatically from dakota.xml, which is used in the generation of parser system files that are compiled into the Dakota executable. Therefore, these files are the definitive source for input syntax, capability options, and associated data inputs. Refer to the Developers Manual information on how to modify the input specification and propagate the changes through the parsing system.

Key features of the input specification and the associated user input files include:

- In the input specification, required individual specifications simply appear, optional individual and group specifications are enclosed in [], required group specifications are enclosed in (), and either-or relationships are denoted by the | symbol. These symbols only appear in dakota.input.summary; they must not appear in actual user input files.
- Keyword specifications (i.e., environment, method, model, variables, interface, and responses) begin with the keyword possibly preceded by white space (blanks, tabs, and newlines) both in the input specifications and in user input files. For readability, keyword specifications may be spread across several lines. Earlier versions of Dakota (prior to 4.1) required a backslash character (\) at the ends of intermediate lines of a keyword. While such backslashes are still accepted, they are no longer required.
- Some of the keyword components within the input specification indicate that the user must supply INTEGER, REAL, STRING, INTEGERLIST, REALLIST, or STRINGLIST data as part of the specification. In a user input file, the "=" is optional, data in a LIST can be separated by commas or whitespace, and the STRING data are enclosed in single or double quotes (e.g., 'text_book' or "text_book").
- In user input files, input is largely order-independent (except for entries in lists of data), case insensitive, and white-space insensitive. Although the order of input shown in the Sample Input Files generally follows the order of options in the input specification, this is not required.
- In user input files, specifications may be abbreviated so long as the abbreviation is unique. For example, the npsol_sqp specification within the method keyword could be abbreviated as npsol, but dot_sqp should not be abbreviated as dot since this would be ambiguous with other DOT method specifications.
- In both the input specification and user input files, comments are preceded by #.
- ALIAS refers to synonymous keywords, which often exist for backwards compatability. Users are encouraged to use the most current keyword.


.. code-block::

    KEYWORD01 environment
    	[ tabular_data ALIAS tabular_graphics_data
    	  [ tabular_data_file ALIAS tabular_graphics_file STRING ]
    	  [ ( custom_annotated
    	      [ header ]
    	      [ eval_id ]
    	      [ interface_id ]
    	      )
    	  | annotated
    	  | freeform ]
    	  ]
    	[ output_file STRING ]
    	[ error_file STRING ]
    	[ read_restart STRING
    	  [ stop_restart INTEGER >= 0 ]
    	  ]
    	[ write_restart STRING ]
    	[ output_precision INTEGER >= 0 ]
    	[ results_output
    	  [ results_output_file STRING ]
    	  [ text ]
    	  [ hdf5
    	    [ model_selection
    	      top_method
    	      | none
    	      | all_methods
    	      | all
    	      ]
    	    [ interface_selection
    	      none
    	      | simulation
    	      | all
    	      ]
    	    ]
    	  ]
    	[ graphics ]
    	[ check ]
    	[ pre_run
    	  [ input STRING ]
    	  [ output STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  ]
    	[ run
    	  [ input STRING ]
    	  [ output STRING ]
    	  ]
    	[ post_run
    	  [ input STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ output STRING ]
    	  ]
    	[ top_method_pointer ALIAS method_pointer STRING ]
    
    KEYWORD method
    	[ id_method STRING ]
    	[ output
    	  debug
    	  | verbose
    	  | normal
    	  | quiet
    	  | silent
    	  ]
    	[ final_solutions INTEGER >= 0 ]
    	( hybrid
    	  ( sequential ALIAS uncoupled
    	    ( method_name_list STRINGLIST
    	      [ model_pointer_list STRINGLIST ]
    	      )
    	    | method_pointer_list STRINGLIST
    	    [ iterator_servers INTEGER > 0 ]
    	    [ iterator_scheduling
    	      dedicated
    	      | peer
    	      ]
    	    [ processors_per_iterator INTEGER > 0 ]
    	    )
    	  |
    	  ( embedded ALIAS coupled
    	    ( global_method_name STRING
    	      [ global_model_pointer STRING ]
    	      )
    	    | global_method_pointer STRING
    	    ( local_method_name STRING
    	      [ local_model_pointer STRING ]
    	      )
    	    | local_method_pointer STRING
    	    [ local_search_probability REAL ]
    	    [ iterator_servers INTEGER > 0 ]
    	    [ iterator_scheduling
    	      dedicated
    	      | peer
    	      ]
    	    [ processors_per_iterator INTEGER > 0 ]
    	    )
    	  |
    	  ( collaborative
    	    ( method_name_list STRINGLIST
    	      [ model_pointer_list STRINGLIST ]
    	      )
    	    | method_pointer_list STRINGLIST
    	    [ iterator_servers INTEGER > 0 ]
    	    [ iterator_scheduling
    	      dedicated
    	      | peer
    	      ]
    	    [ processors_per_iterator INTEGER > 0 ]
    	    )
    	  )
    	|
    	( multi_start
    	  ( method_name STRING
    	    [ model_pointer STRING ]
    	    )
    	  | method_pointer STRING
    	  [ random_starts INTEGER
    	    [ seed INTEGER ]
    	    ]
    	  [ starting_points REALLIST ]
    	  [ iterator_servers INTEGER > 0 ]
    	  [ iterator_scheduling
    	    dedicated
    	    | peer
    	    ]
    	  [ processors_per_iterator INTEGER > 0 ]
    	  )
    	|
    	( pareto_set
    	  ( method_name ALIAS opt_method_name STRING
    	    [ model_pointer ALIAS opt_model_pointer STRING ]
    	    )
    	  | method_pointer ALIAS opt_method_pointer STRING
    	  [ random_weight_sets INTEGER
    	    [ seed INTEGER ]
    	    ]
    	  [ weight_sets ALIAS multi_objective_weight_sets REALLIST ]
    	  [ iterator_servers INTEGER > 0 ]
    	  [ iterator_scheduling
    	    dedicated
    	    | peer
    	    ]
    	  [ processors_per_iterator INTEGER > 0 ]
    	  )
    	|
    	( branch_and_bound
    	  method_pointer STRING
    	  |
    	  ( method_name STRING
    	    [ model_pointer STRING ]
    	    )
    	  [ scaling ]
    	  )
    	|
    	( surrogate_based_local
    	  method_pointer ALIAS approx_method_pointer STRING
    	  | method_name ALIAS approx_method_name STRING
    	  model_pointer ALIAS approx_model_pointer STRING
    	  [ soft_convergence_limit INTEGER ]
    	  [ truth_surrogate_bypass ]
    	  [ approx_subproblem
    	    original_primary
    	    | single_objective
    	    | augmented_lagrangian_objective
    	    | lagrangian_objective
    	    original_constraints
    	    | linearized_constraints
    	    | no_constraints
    	    ]
    	  [ merit_function
    	    penalty_merit
    	    | adaptive_penalty_merit
    	    | lagrangian_merit
    	    | augmented_lagrangian_merit
    	    ]
    	  [ acceptance_logic
    	    tr_ratio
    	    | filter
    	    ]
    	  [ constraint_relax
    	    homotopy
    	    ]
    	  [ trust_region
    	    [ initial_size REALLIST ]
    	    [ minimum_size REAL ]
    	    [ contract_threshold REAL ]
    	    [ expand_threshold REAL ]
    	    [ contraction_factor REAL ]
    	    [ expansion_factor REAL ]
    	    ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ constraint_tolerance REAL ]
    	  )
    	|
    	( surrogate_based_global
    	  method_pointer ALIAS approx_method_pointer STRING
    	  | method_name ALIAS approx_method_name STRING
    	  model_pointer ALIAS approx_model_pointer STRING
    	  [ replace_points ]
    	  [ max_iterations INTEGER >= 0 ]
    	  )
    	|
    	( dot_frcg
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ constraint_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( dot_mmfd
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ constraint_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( dot_bfgs
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ constraint_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( dot_slp
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ constraint_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( dot_sqp
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ constraint_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( conmin_frcg
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ constraint_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( conmin_mfd
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ constraint_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( dl_solver STRING
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( npsol_sqp
    	  [ verify_level INTEGER ]
    	  [ function_precision REAL ]
    	  [ linesearch_tolerance REAL ]
    	  [ convergence_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ constraint_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( nlssol_sqp
    	  [ verify_level INTEGER ]
    	  [ function_precision REAL ]
    	  [ linesearch_tolerance REAL ]
    	  [ convergence_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ constraint_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( nlpql_sqp
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( optpp_cg
    	  [ max_step REAL ]
    	  [ gradient_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( optpp_q_newton
    	  [ search_method
    	    value_based_line_search
    	    | gradient_based_line_search
    	    | trust_region
    	    | tr_pds
    	    ]
    	  [ merit_function
    	    el_bakry
    	    | argaez_tapia
    	    | van_shanno
    	    ]
    	  [ steplength_to_boundary REAL ]
    	  [ centering_parameter REAL ]
    	  [ max_step REAL ]
    	  [ gradient_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( optpp_fd_newton
    	  [ search_method
    	    value_based_line_search
    	    | gradient_based_line_search
    	    | trust_region
    	    | tr_pds
    	    ]
    	  [ merit_function
    	    el_bakry
    	    | argaez_tapia
    	    | van_shanno
    	    ]
    	  [ steplength_to_boundary REAL ]
    	  [ centering_parameter REAL ]
    	  [ max_step REAL ]
    	  [ gradient_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( optpp_g_newton
    	  [ search_method
    	    value_based_line_search
    	    | gradient_based_line_search
    	    | trust_region
    	    | tr_pds
    	    ]
    	  [ merit_function
    	    el_bakry
    	    | argaez_tapia
    	    | van_shanno
    	    ]
    	  [ steplength_to_boundary REAL ]
    	  [ centering_parameter REAL ]
    	  [ max_step REAL ]
    	  [ gradient_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( optpp_newton
    	  [ search_method
    	    value_based_line_search
    	    | gradient_based_line_search
    	    | trust_region
    	    | tr_pds
    	    ]
    	  [ merit_function
    	    el_bakry
    	    | argaez_tapia
    	    | van_shanno
    	    ]
    	  [ steplength_to_boundary REAL ]
    	  [ centering_parameter REAL ]
    	  [ max_step REAL ]
    	  [ gradient_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( optpp_pds
    	  [ search_scheme_size INTEGER ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( demo_tpl
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ variable_tolerance REAL ]
    	  [ solution_target ALIAS solution_accuracy REAL ]
    	  [ options_file STRING ]
    	  )
    	|
    	( rol
    	  [ max_iterations INTEGER >= 0 ]
    	  [ variable_tolerance REAL ]
    	  [ gradient_tolerance REAL ]
    	  [ constraint_tolerance REAL ]
    	  [ options_file STRING ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( asynch_pattern_search ALIAS coliny_apps
    	  [ initial_delta REAL ]
    	  [ contraction_factor REAL ]
    	  [ variable_tolerance REAL ]
    	  [ solution_target ALIAS solution_accuracy REAL ]
    	  [ synchronization
    	    blocking
    	    | nonblocking
    	    ]
    	  [ merit_function
    	    merit_max
    	    | merit_max_smooth
    	    | merit1
    	    | merit1_smooth
    	    | merit2
    	    | merit2_smooth
    	    | merit2_squared
    	    ]
    	  [ constraint_penalty REAL ]
    	  [ smoothing_factor REAL ]
    	  [ constraint_tolerance REAL ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( mesh_adaptive_search
    	  [ initial_delta REAL ]
    	  [ variable_tolerance REAL ]
    	  [ function_precision REAL ]
    	  [ seed INTEGER > 0 ]
    	  [ history_file STRING ]
    	  [ display_format STRING ]
    	  [ variable_neighborhood_search REAL ]
    	  [ neighbor_order INTEGER > 0 ]
    	  [ display_all_evaluations ]
    	  [ use_surrogate
    	    inform_search
    	    | optimize
    	    ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( nowpac
    	  [ trust_region
    	    [ initial_size REALLIST ]
    	    [ minimum_size REAL ]
    	    [ contract_threshold REAL ]
    	    [ expand_threshold REAL ]
    	    [ contraction_factor REAL ]
    	    [ expansion_factor REAL ]
    	    ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( snowpac
    	  [ seed INTEGER > 0 ]
    	  [ trust_region
    	    [ initial_size REALLIST ]
    	    [ minimum_size REAL ]
    	    [ contract_threshold REAL ]
    	    [ expand_threshold REAL ]
    	    [ contraction_factor REAL ]
    	    [ expansion_factor REAL ]
    	    ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( moga
    	  [ fitness_type
    	    layer_rank
    	    | domination_count
    	    ]
    	  [ replacement_type
    	    elitist
    	    | roulette_wheel
    	    | unique_roulette_wheel
    	    |
    	    ( below_limit REAL
    	      [ shrinkage_fraction ALIAS shrinkage_percentage REAL ]
    	      )
    	    ]
    	  [ niching_type
    	    radial REALLIST
    	    | distance REALLIST
    	    |
    	    ( max_designs REALLIST
    	      [ num_designs INTEGER >= 2 ]
    	      )
    	    ]
    	  [ convergence_type
    	    metric_tracker
    	    [ percent_change REAL ]
    	    [ num_generations INTEGER >= 0 ]
    	    ]
    	  [ postprocessor_type
    	    orthogonal_distance REALLIST
    	    ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ population_size INTEGER >= 0 ]
    	  [ log_file STRING ]
    	  [ print_each_pop ]
    	  [ initialization_type
    	    simple_random
    	    | unique_random
    	    | flat_file STRING
    	    ]
    	  [ crossover_type
    	    multi_point_binary INTEGER
    	    | multi_point_parameterized_binary INTEGER
    	    | multi_point_real INTEGER
    	    |
    	    ( shuffle_random
    	      [ num_parents INTEGER > 0 ]
    	      [ num_offspring INTEGER > 0 ]
    	      )
    	    [ crossover_rate REAL ]
    	    ]
    	  [ mutation_type
    	    bit_random
    	    | replace_uniform
    	    |
    	    ( offset_normal
    	      [ mutation_scale REAL ]
    	      )
    	    |
    	    ( offset_cauchy
    	      [ mutation_scale REAL ]
    	      )
    	    |
    	    ( offset_uniform
    	      [ mutation_scale REAL ]
    	      )
    	    [ mutation_rate REAL ]
    	    ]
    	  [ seed INTEGER > 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( soga
    	  [ fitness_type
    	    merit_function
    	    [ constraint_penalty REAL ]
    	    ]
    	  [ replacement_type
    	    elitist
    	    | favor_feasible
    	    | roulette_wheel
    	    | unique_roulette_wheel
    	    ]
    	  [ convergence_type
    	    ( best_fitness_tracker
    	      [ percent_change REAL ]
    	      [ num_generations INTEGER >= 0 ]
    	      )
    	    |
    	    ( average_fitness_tracker
    	      [ percent_change REAL ]
    	      [ num_generations INTEGER >= 0 ]
    	      )
    	    ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ population_size INTEGER >= 0 ]
    	  [ log_file STRING ]
    	  [ print_each_pop ]
    	  [ initialization_type
    	    simple_random
    	    | unique_random
    	    | flat_file STRING
    	    ]
    	  [ crossover_type
    	    multi_point_binary INTEGER
    	    | multi_point_parameterized_binary INTEGER
    	    | multi_point_real INTEGER
    	    |
    	    ( shuffle_random
    	      [ num_parents INTEGER > 0 ]
    	      [ num_offspring INTEGER > 0 ]
    	      )
    	    [ crossover_rate REAL ]
    	    ]
    	  [ mutation_type
    	    bit_random
    	    | replace_uniform
    	    |
    	    ( offset_normal
    	      [ mutation_scale REAL ]
    	      )
    	    |
    	    ( offset_cauchy
    	      [ mutation_scale REAL ]
    	      )
    	    |
    	    ( offset_uniform
    	      [ mutation_scale REAL ]
    	      )
    	    [ mutation_rate REAL ]
    	    ]
    	  [ seed INTEGER > 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( coliny_pattern_search
    	  [ constant_penalty ]
    	  [ no_expansion ]
    	  [ expand_after_success INTEGER ]
    	  [ pattern_basis
    	    coordinate
    	    | simplex
    	    ]
    	  [ stochastic ]
    	  [ total_pattern_size INTEGER ]
    	  [ exploratory_moves
    	    multi_step
    	    | adaptive_pattern
    	    | basic_pattern
    	    ]
    	  [ synchronization
    	    blocking
    	    | nonblocking
    	    ]
    	  [ contraction_factor REAL ]
    	  [ constraint_penalty REAL ]
    	  [ initial_delta REAL ]
    	  [ variable_tolerance REAL ]
    	  [ solution_target ALIAS solution_accuracy REAL ]
    	  [ seed INTEGER > 0 ]
    	  [ show_misc_options ]
    	  [ misc_options STRINGLIST ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( coliny_solis_wets
    	  [ contract_after_failure INTEGER ]
    	  [ no_expansion ]
    	  [ expand_after_success INTEGER ]
    	  [ constant_penalty ]
    	  [ contraction_factor REAL ]
    	  [ constraint_penalty REAL ]
    	  [ initial_delta REAL ]
    	  [ variable_tolerance REAL ]
    	  [ solution_target ALIAS solution_accuracy REAL ]
    	  [ seed INTEGER > 0 ]
    	  [ show_misc_options ]
    	  [ misc_options STRINGLIST ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( coliny_cobyla
    	  [ initial_delta REAL ]
    	  [ variable_tolerance REAL ]
    	  [ solution_target ALIAS solution_accuracy REAL ]
    	  [ seed INTEGER > 0 ]
    	  [ show_misc_options ]
    	  [ misc_options STRINGLIST ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( coliny_direct
    	  [ division
    	    major_dimension
    	    | all_dimensions
    	    ]
    	  [ global_balance_parameter REAL ]
    	  [ local_balance_parameter REAL ]
    	  [ max_boxsize_limit REAL ]
    	  [ min_boxsize_limit REAL ]
    	  [ constraint_penalty REAL ]
    	  [ solution_target ALIAS solution_accuracy REAL ]
    	  [ seed INTEGER > 0 ]
    	  [ show_misc_options ]
    	  [ misc_options STRINGLIST ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( coliny_ea
    	  [ population_size INTEGER > 0 ]
    	  [ initialization_type
    	    simple_random
    	    | unique_random
    	    | flat_file STRING
    	    ]
    	  [ fitness_type
    	    linear_rank
    	    | merit_function
    	    ]
    	  [ replacement_type
    	    random INTEGER
    	    | chc INTEGER
    	    | elitist INTEGER
    	    [ new_solutions_generated INTEGER ]
    	    ]
    	  [ crossover_rate REAL ]
    	  [ crossover_type
    	    two_point
    	    | blend
    	    | uniform
    	    ]
    	  [ mutation_rate REAL ]
    	  [ mutation_type
    	    replace_uniform
    	    |
    	    ( offset_normal
    	      [ mutation_scale REAL ]
    	      [ mutation_range INTEGER ]
    	      )
    	    |
    	    ( offset_cauchy
    	      [ mutation_scale REAL ]
    	      [ mutation_range INTEGER ]
    	      )
    	    |
    	    ( offset_uniform
    	      [ mutation_scale REAL ]
    	      [ mutation_range INTEGER ]
    	      )
    	    [ non_adaptive ]
    	    ]
    	  [ constraint_penalty REAL ]
    	  [ solution_target ALIAS solution_accuracy REAL ]
    	  [ seed INTEGER > 0 ]
    	  [ show_misc_options ]
    	  [ misc_options STRINGLIST ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( coliny_beta
    	  beta_solver_name STRING
    	  [ solution_target ALIAS solution_accuracy REAL ]
    	  [ seed INTEGER > 0 ]
    	  [ show_misc_options ]
    	  [ misc_options STRINGLIST ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( nl2sol
    	  [ function_precision REAL ]
    	  [ absolute_conv_tol REAL ]
    	  [ x_conv_tol REAL ]
    	  [ singular_conv_tol REAL ]
    	  [ singular_radius REAL ]
    	  [ false_conv_tol REAL ]
    	  [ initial_trust_radius REAL ]
    	  [ covariance INTEGER ]
    	  [ regression_diagnostics ]
    	  [ convergence_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ speculative ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( nonlinear_cg
    	  [ misc_options STRINGLIST ]
    	  [ convergence_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( ncsu_direct
    	  [ solution_target ALIAS solution_accuracy REAL ]
    	  [ min_boxsize_limit REAL ]
    	  [ volume_boxsize_limit REAL ]
    	  [ convergence_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( genie_opt_darts
    	  [ seed INTEGER > 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( genie_direct
    	  [ seed INTEGER > 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ scaling ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( efficient_global
    	  [ initial_samples INTEGER ]
    	  [ seed INTEGER > 0 ]
    	  [ batch_size INTEGER >= 1
    	    [ exploration INTEGER >= 0 ]
    	    [ synchronization
    	      blocking
    	      | nonblocking
    	      ]
    	    ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ x_conv_tol REAL ]
    	  [ gaussian_process ALIAS kriging
    	    ( surfpack
    	      [ export_model
    		[ filename_prefix STRING ]
    		( formats
    		  [ text_archive ]
    		  [ binary_archive ]
    		  )
    		]
    	      )
    	    | dakota
    	    |
    	    ( experimental
    	      [ export_model
    		[ filename_prefix STRING ]
    		( formats
    		  [ text_archive ]
    		  [ binary_archive ]
    		  )
    		]
    	      [ options_file STRING ]
    	      )
    	    ]
    	  [ use_derivatives ]
    	  [ import_build_points_file ALIAS import_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( surrogate_based_uq
    	  [ samples_on_emulator ALIAS samples INTEGER ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ probability_refinement ALIAS sample_refinement
    	    import
    	    | adapt_import
    	    | mm_adapt_import
    	    [ refinement_samples INTEGERLIST ]
    	    ]
    	  [ final_moments
    	    none
    	    | standard
    	    | central
    	    ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | reliabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ reliability_levels REALLIST
    	    [ num_reliability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ variance_based_decomp
    	    [ interaction_order INTEGER > 0 ]
    	    [ drop_tolerance REAL ]
    	    ]
    	  [ diagonal_covariance
    	  | full_covariance ]
    	  [ import_approx_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ seed INTEGER > 0 ]
    	  [ fixed_seed ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( function_train
    	  [ p_refinement
    	    ( uniform
    	      increment_start_rank
    	      | increment_start_order
    	      | increment_max_rank
    	      | increment_max_order
    	      | increment_max_rank_order
    	      )
    	    ]
    	  [ max_refinement_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ metric_scale
    	    relative
    	    | absolute
    	    ]
    	  [ refinement_metric
    	    level_mappings
    	    | covariance
    	    ]
    	  [ regression_type
    	    ls
    	    |
    	    ( rls2
    	      l2_penalty REAL
    	      )
    	    ]
    	  [ max_solver_iterations INTEGER >= 0 ]
    	  [ max_cross_iterations INTEGER >= 0 ]
    	  [ solver_tolerance REAL ]
    	  [ response_scaling ]
    	  [ tensor_grid ]
    	  collocation_points INTEGER
    	  | collocation_ratio REAL
    	  [ rounding_tolerance REAL ]
    	  [ arithmetic_tolerance REAL ]
    	  [ start_order ALIAS order INTEGER >= 0
    	    [ dimension_preference REALLIST ]
    	    ]
    	  [ adapt_order ]
    	  [ kick_order INTEGER > 0 ]
    	  [ max_order INTEGER >= 0 ]
    	  [ max_cv_order_candidates INTEGER >= 0 ]
    	  [ start_rank ALIAS rank INTEGER >= 0 ]
    	  [ adapt_rank ]
    	  [ kick_rank INTEGER > 0 ]
    	  [ max_rank INTEGER >= 0 ]
    	  [ max_cv_rank_candidates INTEGER >= 0 ]
    	  [ samples_on_emulator ALIAS samples INTEGER ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ probability_refinement ALIAS sample_refinement
    	    import
    	    | adapt_import
    	    | mm_adapt_import
    	    [ refinement_samples INTEGERLIST ]
    	    ]
    	  [ final_moments
    	    none
    	    | standard
    	    | central
    	    ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | reliabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ reliability_levels REALLIST
    	    [ num_reliability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ variance_based_decomp
    	    [ interaction_order INTEGER > 0 ]
    	    [ drop_tolerance REAL ]
    	    ]
    	  [ diagonal_covariance
    	  | full_covariance ]
    	  [ import_approx_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ seed INTEGER > 0 ]
    	  [ fixed_seed ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( multifidelity_function_train
    	  [ p_refinement
    	    ( uniform
    	      increment_start_rank
    	      | increment_start_order
    	      | increment_max_rank
    	      | increment_max_order
    	      | increment_max_rank_order
    	      )
    	    ]
    	  [ max_refinement_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ metric_scale
    	    relative
    	    | absolute
    	    ]
    	  [ refinement_metric
    	    level_mappings
    	    | covariance
    	    ]
    	  [ statistics_mode
    	    active
    	    | combined
    	    ]
    	  [ allocation_control
    	    greedy
    	    ]
    	  [ discrepancy_emulation
    	    distinct ALIAS paired
    	    | recursive
    	    ]
    	  [ rounding_tolerance REAL ]
    	  [ arithmetic_tolerance REAL ]
    	  [ regression_type
    	    ls
    	    |
    	    ( rls2
    	      l2_penalty REAL
    	      )
    	    ]
    	  [ max_solver_iterations INTEGER >= 0 ]
    	  [ max_cross_iterations INTEGER >= 0 ]
    	  [ solver_tolerance REAL ]
    	  [ response_scaling ]
    	  [ tensor_grid ]
    	  [ collocation_points_sequence ALIAS pilot_samples INTEGERLIST ]
    	  [ collocation_ratio REAL ]
    	  [ start_order_sequence ALIAS order_sequence INTEGERLIST
    	    [ dimension_preference REALLIST ]
    	    ]
    	  [ adapt_order ]
    	  [ kick_order INTEGER > 0 ]
    	  [ max_order INTEGER >= 0 ]
    	  [ max_cv_order_candidates INTEGER >= 0 ]
    	  [ start_rank_sequence ALIAS rank_sequence INTEGERLIST ]
    	  [ adapt_rank ]
    	  [ kick_rank INTEGER > 0 ]
    	  [ max_rank INTEGER >= 0 ]
    	  [ max_cv_rank_candidates INTEGER >= 0 ]
    	  [ samples_on_emulator ALIAS samples INTEGER ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ probability_refinement ALIAS sample_refinement
    	    import
    	    | adapt_import
    	    | mm_adapt_import
    	    [ refinement_samples INTEGERLIST ]
    	    ]
    	  [ final_moments
    	    none
    	    | standard
    	    | central
    	    ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | reliabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ reliability_levels REALLIST
    	    [ num_reliability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ variance_based_decomp
    	    [ interaction_order INTEGER > 0 ]
    	    [ drop_tolerance REAL ]
    	    ]
    	  [ diagonal_covariance
    	  | full_covariance ]
    	  [ import_approx_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ seed_sequence INTEGERLIST ]
    	  [ fixed_seed ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( multilevel_function_train
    	  [ max_iterations INTEGER >= 0 ]
    	  [ allocation_control
    	    ( estimator_variance
    	      [ estimator_rate REAL ]
    	      )
    	    | rank_sampling
    	    ]
    	  [ convergence_tolerance REAL ]
    	  [ metric_scale
    	    relative
    	    | absolute
    	    ]
    	  [ refinement_metric
    	    level_mappings
    	    | covariance
    	    ]
    	  [ discrepancy_emulation
    	    distinct ALIAS paired
    	    | recursive
    	    ]
    	  [ rounding_tolerance REAL ]
    	  [ arithmetic_tolerance REAL ]
    	  [ regression_type
    	    ls
    	    |
    	    ( rls2
    	      l2_penalty REAL
    	      )
    	    ]
    	  [ max_solver_iterations INTEGER >= 0 ]
    	  [ max_cross_iterations INTEGER >= 0 ]
    	  [ solver_tolerance REAL ]
    	  [ response_scaling ]
    	  [ tensor_grid ]
    	  [ collocation_points_sequence ALIAS pilot_samples INTEGERLIST ]
    	  [ collocation_ratio REAL ]
    	  [ start_order_sequence ALIAS order_sequence INTEGERLIST
    	    [ dimension_preference REALLIST ]
    	    ]
    	  [ adapt_order ]
    	  [ kick_order INTEGER > 0 ]
    	  [ max_order INTEGER >= 0 ]
    	  [ max_cv_order_candidates INTEGER >= 0 ]
    	  [ start_rank_sequence ALIAS rank_sequence INTEGERLIST ]
    	  [ adapt_rank ]
    	  [ kick_rank INTEGER > 0 ]
    	  [ max_rank INTEGER >= 0 ]
    	  [ max_cv_rank_candidates INTEGER >= 0 ]
    	  [ samples_on_emulator ALIAS samples INTEGER ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ probability_refinement ALIAS sample_refinement
    	    import
    	    | adapt_import
    	    | mm_adapt_import
    	    [ refinement_samples INTEGERLIST ]
    	    ]
    	  [ final_moments
    	    none
    	    | standard
    	    | central
    	    ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | reliabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ reliability_levels REALLIST
    	    [ num_reliability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ variance_based_decomp
    	    [ interaction_order INTEGER > 0 ]
    	    [ drop_tolerance REAL ]
    	    ]
    	  [ diagonal_covariance
    	  | full_covariance ]
    	  [ import_approx_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ seed_sequence INTEGERLIST ]
    	  [ fixed_seed ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( polynomial_chaos ALIAS nond_polynomial_chaos
    	  [ p_refinement
    	    uniform
    	    |
    	    ( dimension_adaptive
    	      sobol
    	      | decay
    	      | generalized
    	      )
    	    ]
    	  [ max_refinement_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ metric_scale
    	    relative
    	    | absolute
    	    ]
    	  [ refinement_metric
    	    level_mappings
    	    | covariance
    	    ]
    	  ( quadrature_order INTEGER
    	    [ dimension_preference REALLIST ]
    	    [ nested
    	    | non_nested ]
    	    )
    	  |
    	  ( sparse_grid_level INTEGER
    	    [ dimension_preference REALLIST ]
    	    [ restricted
    	    | unrestricted ]
    	    [ nested
    	    | non_nested ]
    	    )
    	  | cubature_integrand INTEGER
    	  |
    	  ( expansion_order INTEGER
    	    [ dimension_preference REALLIST ]
    	    [ basis_type
    	      tensor_product
    	      | total_order
    	      |
    	      ( adapted
    		[ advancements INTEGER ]
    		[ soft_convergence_limit INTEGER ]
    		)
    	      ]
    	    ( collocation_points INTEGER
    	      [ ( least_squares
    		  [ svd
    		  | equality_constrained ]
    		  )
    	      |
    	      ( orthogonal_matching_pursuit ALIAS omp
    		[ noise_tolerance REALLIST ]
    		]
    	      | basis_pursuit ALIAS bp
    	      |
    	      ( basis_pursuit_denoising ALIAS bpdn
    		[ noise_tolerance REALLIST ]
    		]
    	      |
    	      ( least_angle_regression ALIAS lars
    		[ noise_tolerance REALLIST ]
    		]
    	      |
    	      ( least_absolute_shrinkage ALIAS lasso
    		[ noise_tolerance REALLIST ]
    		[ l2_penalty REAL ]
    		]
    	      [ cross_validation
    		[ noise_only ]
    		[ max_cv_order_candidates INTEGER >= 0 ]
    		]
    	      [ ratio_order REAL ]
    	      [ response_scaling ]
    	      [ use_derivatives ]
    	      [ tensor_grid ]
    	      [ reuse_points ALIAS reuse_samples ]
    	      [ max_solver_iterations INTEGER >= 0 ]
    	      )
    	    |
    	    ( collocation_ratio REAL
    	      [ ( least_squares
    		  [ svd
    		  | equality_constrained ]
    		  )
    	      |
    	      ( orthogonal_matching_pursuit ALIAS omp
    		[ noise_tolerance REALLIST ]
    		]
    	      | basis_pursuit ALIAS bp
    	      |
    	      ( basis_pursuit_denoising ALIAS bpdn
    		[ noise_tolerance REALLIST ]
    		]
    	      |
    	      ( least_angle_regression ALIAS lars
    		[ noise_tolerance REALLIST ]
    		]
    	      |
    	      ( least_absolute_shrinkage ALIAS lasso
    		[ noise_tolerance REALLIST ]
    		[ l2_penalty REAL ]
    		]
    	      [ cross_validation
    		[ noise_only ]
    		[ max_cv_order_candidates INTEGER >= 0 ]
    		]
    	      [ ratio_order REAL ]
    	      [ response_scaling ]
    	      [ use_derivatives ]
    	      [ tensor_grid ]
    	      [ reuse_points ALIAS reuse_samples ]
    	      [ max_solver_iterations INTEGER >= 0 ]
    	      )
    	    |
    	    ( expansion_samples INTEGER
    	      [ reuse_points ALIAS reuse_samples ]
    	      )
    	    [ import_build_points_file ALIAS import_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      [ active_only ]
    	      ]
    	    )
    	  |
    	  ( orthogonal_least_interpolation ALIAS least_interpolation ALIAS oli
    	    collocation_points INTEGER
    	    [ tensor_grid INTEGERLIST ]
    	    [ reuse_points ALIAS reuse_samples ]
    	    [ import_build_points_file ALIAS import_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      [ active_only ]
    	      ]
    	    )
    	  | import_expansion_file STRING
    	  [ askey
    	  | wiener ]
    	  [ normalized ]
    	  [ export_expansion_file STRING ]
    	  [ samples_on_emulator ALIAS samples INTEGER ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ probability_refinement ALIAS sample_refinement
    	    import
    	    | adapt_import
    	    | mm_adapt_import
    	    [ refinement_samples INTEGERLIST ]
    	    ]
    	  [ final_moments
    	    none
    	    | standard
    	    | central
    	    ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | reliabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ reliability_levels REALLIST
    	    [ num_reliability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ variance_based_decomp
    	    [ interaction_order INTEGER > 0 ]
    	    [ drop_tolerance REAL ]
    	    ]
    	  [ diagonal_covariance
    	  | full_covariance ]
    	  [ import_approx_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ seed INTEGER > 0 ]
    	  [ fixed_seed ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( multifidelity_polynomial_chaos
    	  [ p_refinement
    	    uniform
    	    |
    	    ( dimension_adaptive
    	      sobol
    	      | decay
    	      | generalized
    	      )
    	    ]
    	  [ max_refinement_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ metric_scale
    	    relative
    	    | absolute
    	    ]
    	  [ refinement_metric
    	    level_mappings
    	    | covariance
    	    ]
    	  [ statistics_mode
    	    active
    	    | combined
    	    ]
    	  [ allocation_control
    	    greedy
    	    ]
    	  [ discrepancy_emulation
    	    distinct ALIAS paired
    	    | recursive
    	    ]
    	  ( quadrature_order_sequence INTEGERLIST
    	    [ dimension_preference REALLIST ]
    	    [ nested
    	    | non_nested ]
    	    )
    	  |
    	  ( sparse_grid_level_sequence INTEGERLIST
    	    [ dimension_preference REALLIST ]
    	    [ restricted
    	    | unrestricted ]
    	    [ nested
    	    | non_nested ]
    	    )
    	  |
    	  ( expansion_order_sequence INTEGERLIST
    	    [ dimension_preference REALLIST ]
    	    [ basis_type
    	      tensor_product
    	      | total_order
    	      |
    	      ( adapted
    		[ advancements INTEGER ]
    		[ soft_convergence_limit INTEGER ]
    		)
    	      ]
    	    ( collocation_ratio REAL
    	      [ collocation_points_sequence ALIAS pilot_samples INTEGERLIST ]
    	      [ ( least_squares
    		  [ svd
    		  | equality_constrained ]
    		  )
    	      |
    	      ( orthogonal_matching_pursuit ALIAS omp
    		[ noise_tolerance REALLIST ]
    		]
    	      | basis_pursuit ALIAS bp
    	      |
    	      ( basis_pursuit_denoising ALIAS bpdn
    		[ noise_tolerance REALLIST ]
    		]
    	      |
    	      ( least_angle_regression ALIAS lars
    		[ noise_tolerance REALLIST ]
    		]
    	      |
    	      ( least_absolute_shrinkage ALIAS lasso
    		[ noise_tolerance REALLIST ]
    		[ l2_penalty REAL ]
    		]
    	      [ cross_validation
    		[ noise_only ]
    		[ max_cv_order_candidates INTEGER >= 0 ]
    		]
    	      [ ratio_order REAL ]
    	      [ response_scaling ]
    	      [ use_derivatives ]
    	      [ tensor_grid ]
    	      [ reuse_points ALIAS reuse_samples ]
    	      [ max_solver_iterations INTEGER >= 0 ]
    	      )
    	    |
    	    ( expansion_samples_sequence INTEGERLIST
    	      [ reuse_points ALIAS reuse_samples ]
    	      )
    	    [ import_build_points_file ALIAS import_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      [ active_only ]
    	      ]
    	    )
    	  |
    	  ( orthogonal_least_interpolation ALIAS least_interpolation ALIAS oli
    	    collocation_points_sequence INTEGERLIST
    	    [ tensor_grid INTEGERLIST ]
    	    [ reuse_points ALIAS reuse_samples ]
    	    [ import_build_points_file ALIAS import_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      [ active_only ]
    	      ]
    	    )
    	  [ askey
    	  | wiener ]
    	  [ normalized ]
    	  [ export_expansion_file STRING ]
    	  [ samples_on_emulator ALIAS samples INTEGER ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ probability_refinement ALIAS sample_refinement
    	    import
    	    | adapt_import
    	    | mm_adapt_import
    	    [ refinement_samples INTEGERLIST ]
    	    ]
    	  [ final_moments
    	    none
    	    | standard
    	    | central
    	    ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | reliabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ reliability_levels REALLIST
    	    [ num_reliability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ variance_based_decomp
    	    [ interaction_order INTEGER > 0 ]
    	    [ drop_tolerance REAL ]
    	    ]
    	  [ diagonal_covariance
    	  | full_covariance ]
    	  [ import_approx_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ seed_sequence INTEGERLIST ]
    	  [ fixed_seed ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( multilevel_polynomial_chaos
    	  [ max_iterations INTEGER >= 0 ]
    	  [ allocation_control
    	    ( estimator_variance
    	      [ estimator_rate REAL ]
    	      )
    	    | rip_sampling
    	    ]
    	  [ convergence_tolerance REAL ]
    	  [ metric_scale
    	    relative
    	    | absolute
    	    ]
    	  [ refinement_metric
    	    level_mappings
    	    | covariance
    	    ]
    	  [ discrepancy_emulation
    	    distinct ALIAS paired
    	    | recursive
    	    ]
    	  ( expansion_order_sequence INTEGERLIST
    	    [ dimension_preference REALLIST ]
    	    [ basis_type
    	      tensor_product
    	      | total_order
    	      |
    	      ( adapted
    		[ advancements INTEGER ]
    		[ soft_convergence_limit INTEGER ]
    		)
    	      ]
    	    ( collocation_ratio REAL
    	      [ collocation_points_sequence ALIAS pilot_samples INTEGERLIST ]
    	      [ ( least_squares
    		  [ svd
    		  | equality_constrained ]
    		  )
    	      |
    	      ( orthogonal_matching_pursuit ALIAS omp
    		[ noise_tolerance REALLIST ]
    		]
    	      | basis_pursuit ALIAS bp
    	      |
    	      ( basis_pursuit_denoising ALIAS bpdn
    		[ noise_tolerance REALLIST ]
    		]
    	      |
    	      ( least_angle_regression ALIAS lars
    		[ noise_tolerance REALLIST ]
    		]
    	      |
    	      ( least_absolute_shrinkage ALIAS lasso
    		[ noise_tolerance REALLIST ]
    		[ l2_penalty REAL ]
    		]
    	      [ cross_validation
    		[ noise_only ]
    		[ max_cv_order_candidates INTEGER >= 0 ]
    		]
    	      [ ratio_order REAL ]
    	      [ response_scaling ]
    	      [ use_derivatives ]
    	      [ tensor_grid ]
    	      [ reuse_points ALIAS reuse_samples ]
    	      [ max_solver_iterations INTEGER >= 0 ]
    	      )
    	    |
    	    ( expansion_samples_sequence INTEGERLIST
    	      [ reuse_points ALIAS reuse_samples ]
    	      )
    	    [ import_build_points_file ALIAS import_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      [ active_only ]
    	      ]
    	    )
    	  |
    	  ( orthogonal_least_interpolation ALIAS least_interpolation ALIAS oli
    	    collocation_points_sequence INTEGERLIST
    	    [ tensor_grid INTEGERLIST ]
    	    [ reuse_points ALIAS reuse_samples ]
    	    [ import_build_points_file ALIAS import_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      [ active_only ]
    	      ]
    	    )
    	  [ askey
    	  | wiener ]
    	  [ normalized ]
    	  [ export_expansion_file STRING ]
    	  [ samples_on_emulator ALIAS samples INTEGER ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ probability_refinement ALIAS sample_refinement
    	    import
    	    | adapt_import
    	    | mm_adapt_import
    	    [ refinement_samples INTEGERLIST ]
    	    ]
    	  [ final_moments
    	    none
    	    | standard
    	    | central
    	    ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | reliabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ reliability_levels REALLIST
    	    [ num_reliability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ variance_based_decomp
    	    [ interaction_order INTEGER > 0 ]
    	    [ drop_tolerance REAL ]
    	    ]
    	  [ diagonal_covariance
    	  | full_covariance ]
    	  [ import_approx_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ seed_sequence INTEGERLIST ]
    	  [ fixed_seed ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( stoch_collocation ALIAS nond_stoch_collocation
    	  [ ( p_refinement
    	      uniform
    	      |
    	      ( dimension_adaptive
    		sobol
    		| generalized
    		)
    	      )
    	  |
    	  ( h_refinement
    	    uniform
    	    |
    	    ( dimension_adaptive
    	      sobol
    	      | generalized
    	      )
    	    | local_adaptive
    	    ]
    	  [ max_refinement_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ metric_scale
    	    relative
    	    | absolute
    	    ]
    	  [ refinement_metric
    	    level_mappings
    	    | covariance
    	    ]
    	  ( quadrature_order INTEGER
    	    [ dimension_preference REALLIST ]
    	    [ nested
    	    | non_nested ]
    	    )
    	  |
    	  ( sparse_grid_level INTEGER
    	    [ dimension_preference REALLIST ]
    	    [ nodal
    	    | hierarchical ]
    	    [ restricted
    	    | unrestricted ]
    	    [ nested
    	    | non_nested ]
    	    )
    	  [ piecewise
    	  | askey
    	  | wiener ]
    	  [ use_derivatives ]
    	  [ samples_on_emulator ALIAS samples INTEGER ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ probability_refinement ALIAS sample_refinement
    	    import
    	    | adapt_import
    	    | mm_adapt_import
    	    [ refinement_samples INTEGERLIST ]
    	    ]
    	  [ final_moments
    	    none
    	    | standard
    	    | central
    	    ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | reliabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ reliability_levels REALLIST
    	    [ num_reliability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ variance_based_decomp
    	    [ interaction_order INTEGER > 0 ]
    	    [ drop_tolerance REAL ]
    	    ]
    	  [ diagonal_covariance
    	  | full_covariance ]
    	  [ import_approx_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ seed INTEGER > 0 ]
    	  [ fixed_seed ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( multifidelity_stoch_collocation
    	  [ ( p_refinement
    	      uniform
    	      |
    	      ( dimension_adaptive
    		sobol
    		| generalized
    		)
    	      )
    	  |
    	  ( h_refinement
    	    uniform
    	    |
    	    ( dimension_adaptive
    	      sobol
    	      | generalized
    	      )
    	    | local_adaptive
    	    ]
    	  [ max_refinement_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ metric_scale
    	    relative
    	    | absolute
    	    ]
    	  [ refinement_metric
    	    level_mappings
    	    | covariance
    	    ]
    	  [ statistics_mode
    	    active
    	    | combined
    	    ]
    	  [ allocation_control
    	    greedy
    	    ]
    	  [ discrepancy_emulation
    	    distinct ALIAS paired
    	    | recursive
    	    ]
    	  ( quadrature_order_sequence INTEGERLIST
    	    [ dimension_preference REALLIST ]
    	    [ nested
    	    | non_nested ]
    	    )
    	  |
    	  ( sparse_grid_level_sequence INTEGERLIST
    	    [ dimension_preference REALLIST ]
    	    [ nodal
    	    | hierarchical ]
    	    [ restricted
    	    | unrestricted ]
    	    [ nested
    	    | non_nested ]
    	    )
    	  [ piecewise
    	  | askey
    	  | wiener ]
    	  [ use_derivatives ]
    	  [ samples_on_emulator ALIAS samples INTEGER ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ probability_refinement ALIAS sample_refinement
    	    import
    	    | adapt_import
    	    | mm_adapt_import
    	    [ refinement_samples INTEGERLIST ]
    	    ]
    	  [ final_moments
    	    none
    	    | standard
    	    | central
    	    ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | reliabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ reliability_levels REALLIST
    	    [ num_reliability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ variance_based_decomp
    	    [ interaction_order INTEGER > 0 ]
    	    [ drop_tolerance REAL ]
    	    ]
    	  [ diagonal_covariance
    	  | full_covariance ]
    	  [ import_approx_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ seed_sequence INTEGERLIST ]
    	  [ fixed_seed ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( sampling ALIAS nond_sampling
    	  [ samples ALIAS initial_samples INTEGER ]
    	  [ seed INTEGER > 0 ]
    	  [ fixed_seed ]
    	  [ sample_type
    	    lhs
    	    | random
    	    | incremental_lhs
    	    | incremental_random
    	    |
    	    ( low_discrepancy ALIAS qmc
    	      [ rank_1_lattice
    		[ no_random_shift ]
    		[ m_max INTEGER >= 0 ]
    		[ generating_vector
    		  inline INTEGERLIST
    		  | file STRING
    		  |
    		  ( predefined
    		    kuo
    		    | cools_kuo_nuyens
    		    )
    		  ]
    		[ ordering
    		  natural
    		  | radical_inverse
    		  ]
    		]
    	      [ digital_net ALIAS sobol_sequence
    		[ no_digital_shift ]
    		[ no_scrambling ]
    		[ integer_format
    		  most_significant_bit_first
    		  | least_significant_bit_first
    		  ]
    		[ m_max INTEGER >= 0 ]
    		[ t_max INTEGER >= 0 ]
    		[ t_scramble INTEGER >= 0 ]
    		[ generating_matrices
    		  inline INTEGERLIST
    		  | file STRING
    		  |
    		  ( predefined
    		    joe_kuo
    		    | sobol_order_2
    		    )
    		  ]
    		[ ordering
    		  natural
    		  | gray_code
    		  ]
    		]
    	      )
    	    ]
    	  [ refinement_samples INTEGERLIST ]
    	  [ d_optimal
    	    [ candidate_designs INTEGER > 0
    	    | leja_oversample_ratio REAL ]
    	    ]
    	  [ variance_based_decomp
    	    [ drop_tolerance REAL ]
    	    [ vbd_sampling_method
    	      ( binned
    		[ num_bins INTEGER ]
    		)
    	      | pick_and_freeze
    	      ]
    	    ]
    	  [ backfill ]
    	  [ principal_components
    	    [ percent_variance_explained REAL ]
    	    ]
    	  [ wilks
    	    [ order INTEGER ]
    	    [ confidence_level REAL ]
    	    [ one_sided_lower ]
    	    [ one_sided_upper ]
    	    [ two_sided ]
    	    ]
    	  [ std_regression_coeffs ]
    	  [ tolerance_intervals
    	    [ coverage REAL ]
    	    [ confidence_level REAL ]
    	    ]
    	  [ final_moments
    	    none
    	    | standard
    	    | central
    	    ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | reliabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ reliability_levels REALLIST
    	    [ num_reliability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( multilevel_sampling ALIAS multilevel_mc ALIAS mlmc
    	  [ seed_sequence INTEGERLIST ]
    	  [ fixed_seed ]
    	  [ pilot_samples ALIAS initial_samples INTEGERLIST ]
    	  [ solution_mode
    	    ( online_pilot
    	      [ relaxation
    		factor_sequence REALLIST
    		| fixed_factor REAL
    		| recursive_factor REAL
    		]
    	      [ final_statistics
    		estimator_performance
    		|
    		( qoi_statistics
    		  [ final_moments
    		    none
    		    | standard
    		    | central
    		    ]
    		  [ distribution
    		    cumulative
    		    | complementary
    		    ]
    		  )
    		]
    	      )
    	    |
    	    ( offline_pilot
    	      [ final_statistics
    		estimator_performance
    		|
    		( qoi_statistics
    		  [ final_moments
    		    none
    		    | standard
    		    | central
    		    ]
    		  [ distribution
    		    cumulative
    		    | complementary
    		    ]
    		  )
    		]
    	      )
    	    | online_projection
    	    | offline_projection
    	    ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ weighted
    	    [ search_model_graphs
    	      [ model_selection ]
    	      no_recursion
    	      | full_recursion
    	      ]
    	    [ sqp
    	    | nip
    	    | global_local
    	    | competed_local ]
    	    [ solver_metric
    	      average_estimator_variance
    	      |
    	      ( norm_estimator_variance
    		[ norm_order REAL >= 1. ]
    		)
    	      | max_estimator_variance
    	      ]
    	    ]
    	  [ export_sample_sequence
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ allocation_target
    	    mean
    	    |
    	    ( variance
    	      [ optimization ]
    	      )
    	    |
    	    ( standard_deviation
    	      [ optimization ]
    	      )
    	    |
    	    ( scalarization
    	      [ scalarization_response_mapping REALLIST ]
    	      [ optimization ]
    	      )
    	    ]
    	  [ qoi_aggregation
    	    sum
    	    | max
    	    ]
    	  [ convergence_tolerance REAL ]
    	  [ convergence_tolerance_type
    	    relative
    	    | absolute
    	    ]
    	  [ convergence_tolerance_target
    	    variance_constraint
    	    | cost_constraint
    	    ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( multifidelity_sampling ALIAS multifidelity_mc ALIAS mfmc
    	  [ seed_sequence INTEGERLIST ]
    	  [ fixed_seed ]
    	  [ pilot_samples ALIAS initial_samples INTEGERLIST ]
    	  [ solution_mode
    	    ( online_pilot
    	      [ relaxation
    		factor_sequence REALLIST
    		| fixed_factor REAL
    		| recursive_factor REAL
    		]
    	      [ final_statistics
    		estimator_performance
    		|
    		( qoi_statistics
    		  [ final_moments
    		    none
    		    | standard
    		    | central
    		    ]
    		  [ distribution
    		    cumulative
    		    | complementary
    		    ]
    		  )
    		]
    	      )
    	    |
    	    ( offline_pilot
    	      [ final_statistics
    		estimator_performance
    		|
    		( qoi_statistics
    		  [ final_moments
    		    none
    		    | standard
    		    | central
    		    ]
    		  [ distribution
    		    cumulative
    		    | complementary
    		    ]
    		  )
    		]
    	      )
    	    | online_projection
    	    | offline_projection
    	    ]
    	  [ numerical_solve
    	    [ fallback
    	    | override ]
    	    [ sqp
    	    | nip
    	    | global_local
    	    | competed_local ]
    	    [ solver_metric
    	      average_estimator_variance
    	      |
    	      ( norm_estimator_variance
    		[ norm_order REAL >= 1. ]
    		)
    	      | max_estimator_variance
    	      ]
    	    ]
    	  [ search_model_graphs
    	    [ model_selection ]
    	    no_recursion
    	    | full_recursion
    	    ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ export_sample_sequence
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ convergence_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( multilevel_multifidelity_sampling ALIAS multilevel_multifidelity_mc ALIAS mlmfmc
    	  [ seed_sequence INTEGERLIST ]
    	  [ fixed_seed ]
    	  [ pilot_samples ALIAS initial_samples INTEGERLIST ]
    	  [ solution_mode
    	    ( online_pilot
    	      [ relaxation
    		factor_sequence REALLIST
    		| fixed_factor REAL
    		| recursive_factor REAL
    		]
    	      [ final_statistics
    		estimator_performance
    		|
    		( qoi_statistics
    		  [ final_moments
    		    none
    		    | standard
    		    | central
    		    ]
    		  [ distribution
    		    cumulative
    		    | complementary
    		    ]
    		  )
    		]
    	      )
    	    |
    	    ( offline_pilot
    	      [ final_statistics
    		estimator_performance
    		|
    		( qoi_statistics
    		  [ final_moments
    		    none
    		    | standard
    		    | central
    		    ]
    		  [ distribution
    		    cumulative
    		    | complementary
    		    ]
    		  )
    		]
    	      )
    	    | online_projection
    	    | offline_projection
    	    ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ export_sample_sequence
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ convergence_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( approximate_control_variate ALIAS acv_sampling
    	  acv_independent_sampling ALIAS acv_is
    	  | acv_multifidelity ALIAS acv_mf
    	  | acv_recursive_diff ALIAS acv_rd
    	  [ search_model_graphs
    	    [ model_selection ]
    	    no_recursion
    	    | kl_recursion
    	    |
    	    ( partial_recursion
    	      depth_limit INTEGER
    	      )
    	    | full_recursion
    	    ]
    	  [ pilot_samples ALIAS initial_samples INTEGERLIST ]
    	  [ solution_mode
    	    ( online_pilot
    	      [ relaxation
    		factor_sequence REALLIST
    		| fixed_factor REAL
    		| recursive_factor REAL
    		]
    	      [ final_statistics
    		estimator_performance
    		|
    		( qoi_statistics
    		  [ final_moments
    		    none
    		    | standard
    		    | central
    		    ]
    		  [ distribution
    		    cumulative
    		    | complementary
    		    ]
    		  )
    		]
    	      )
    	    |
    	    ( offline_pilot
    	      [ final_statistics
    		estimator_performance
    		|
    		( qoi_statistics
    		  [ final_moments
    		    none
    		    | standard
    		    | central
    		    ]
    		  [ distribution
    		    cumulative
    		    | complementary
    		    ]
    		  )
    		]
    	      )
    	    | online_projection
    	    | offline_projection
    	    ]
    	  [ truth_fixed_by_pilot ]
    	  [ sqp
    	  | nip
    	  | global_local
    	  | competed_local ]
    	  [ solver_metric
    	    average_estimator_variance
    	    |
    	    ( norm_estimator_variance
    	      [ norm_order REAL >= 1. ]
    	      )
    	    | max_estimator_variance
    	    ]
    	  [ seed_sequence INTEGERLIST ]
    	  [ fixed_seed ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ export_sample_sequence
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ convergence_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( multilevel_blue
    	  [ group_throttle
    	    mfmc_groups
    	    | common_groups
    	    | group_size INTEGER > 0
    	    | best_conditioned INTEGER > 0
    	    | rcond_tolerance REAL >= 0
    	    ]
    	  [ pilot_samples ALIAS initial_samples INTEGERLIST
    	    [ independent ]
    	    ]
    	  [ solution_mode
    	    ( online_pilot
    	      [ relaxation
    		factor_sequence REALLIST
    		| fixed_factor REAL
    		| recursive_factor REAL
    		]
    	      [ final_statistics
    		estimator_performance
    		|
    		( qoi_statistics
    		  [ final_moments
    		    none
    		    | standard
    		    | central
    		    ]
    		  [ distribution
    		    cumulative
    		    | complementary
    		    ]
    		  )
    		]
    	      )
    	    |
    	    ( offline_pilot
    	      [ final_statistics
    		estimator_performance
    		|
    		( qoi_statistics
    		  [ final_moments
    		    none
    		    | standard
    		    | central
    		    ]
    		  [ distribution
    		    cumulative
    		    | complementary
    		    ]
    		  )
    		]
    	      )
    	    | online_projection
    	    | offline_projection
    	    ]
    	  [ sqp
    	  | nip
    	  | global_local
    	  | competed_local ]
    	  [ solver_metric
    	    average_estimator_variance
    	    |
    	    ( norm_estimator_variance
    	      [ norm_order REAL >= 1. ]
    	      )
    	    | max_estimator_variance
    	    ]
    	  [ seed_sequence INTEGERLIST ]
    	  [ fixed_seed ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ export_sample_sequence
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ convergence_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( importance_sampling ALIAS nond_importance_sampling
    	  [ samples ALIAS initial_samples INTEGER ]
    	  [ seed INTEGER > 0 ]
    	  import
    	  | adapt_import
    	  | mm_adapt_import
    	  [ refinement_samples INTEGERLIST ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( gpais ALIAS gaussian_process_adaptive_importance_sampling
    	  [ build_samples ALIAS samples INTEGER ]
    	  [ seed INTEGER > 0 ]
    	  [ samples_on_emulator INTEGER ]
    	  [ import_build_points_file ALIAS import_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( adaptive_sampling ALIAS nond_adaptive_sampling
    	  [ initial_samples ALIAS samples INTEGER ]
    	  [ seed INTEGER > 0 ]
    	  [ samples_on_emulator INTEGER ]
    	  [ fitness_metric
    	    predicted_variance
    	    | distance
    	    | gradient
    	    ]
    	  [ batch_selection
    	    naive
    	    | distance_penalty
    	    | topology
    	    | constant_liar
    	    ]
    	  [ refinement_samples INTEGERLIST ]
    	  [ import_build_points_file ALIAS import_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ misc_options STRINGLIST ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( pof_darts ALIAS nond_pof_darts
    	  build_samples ALIAS samples INTEGER
    	  [ seed INTEGER > 0 ]
    	  [ lipschitz
    	    local
    	    | global
    	    ]
    	  [ samples_on_emulator INTEGER ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( rkd_darts ALIAS nond_rkd_darts
    	  build_samples ALIAS samples INTEGER
    	  [ seed INTEGER > 0 ]
    	  [ lipschitz
    	    local
    	    | global
    	    ]
    	  [ samples_on_emulator INTEGER ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( global_evidence ALIAS nond_global_evidence
    	  [ samples INTEGER ]
    	  [ seed INTEGER > 0 ]
    	  [ ( sbgo
    	      [ gaussian_process ALIAS kriging
    		surfpack
    		| dakota
    		|
    		( experimental
    		  [ options_file STRING ]
    		  )
    		]
    	      [ use_derivatives ]
    	      [ import_build_points_file ALIAS import_points_file STRING
    		[ ( custom_annotated
    		    [ header ]
    		    [ eval_id ]
    		    [ interface_id ]
    		    )
    		| annotated
    		| freeform ]
    		[ active_only ]
    		]
    	      [ export_approx_points_file ALIAS export_points_file STRING
    		[ ( custom_annotated
    		    [ header ]
    		    [ eval_id ]
    		    [ interface_id ]
    		    )
    		| annotated
    		| freeform ]
    		]
    	      )
    	  |
    	  ( ego
    	    [ gaussian_process ALIAS kriging
    	      ( surfpack
    		[ export_model
    		  [ filename_prefix STRING ]
    		  ( formats
    		    [ text_archive ]
    		    [ binary_archive ]
    		    )
    		  ]
    		)
    	      | dakota
    	      |
    	      ( experimental
    		[ export_model
    		  [ filename_prefix STRING ]
    		  ( formats
    		    [ text_archive ]
    		    [ binary_archive ]
    		    )
    		  ]
    		[ options_file STRING ]
    		)
    	      ]
    	    [ use_derivatives ]
    	    [ import_build_points_file ALIAS import_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      [ active_only ]
    	      ]
    	    [ export_approx_points_file ALIAS export_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    ]
    	  | ea
    	  | lhs ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( global_interval_est ALIAS nond_global_interval_est
    	  [ samples INTEGER ]
    	  [ seed INTEGER > 0 ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ max_function_evaluations INTEGER >= 0 ]
    	  [ ( sbgo
    	      [ gaussian_process ALIAS kriging
    		surfpack
    		| dakota
    		|
    		( experimental
    		  [ options_file STRING ]
    		  )
    		]
    	      [ use_derivatives ]
    	      [ import_build_points_file ALIAS import_points_file STRING
    		[ ( custom_annotated
    		    [ header ]
    		    [ eval_id ]
    		    [ interface_id ]
    		    )
    		| annotated
    		| freeform ]
    		[ active_only ]
    		]
    	      [ export_approx_points_file ALIAS export_points_file STRING
    		[ ( custom_annotated
    		    [ header ]
    		    [ eval_id ]
    		    [ interface_id ]
    		    )
    		| annotated
    		| freeform ]
    		]
    	      )
    	  |
    	  ( ego
    	    [ gaussian_process ALIAS kriging
    	      ( surfpack
    		[ export_model
    		  [ filename_prefix STRING ]
    		  ( formats
    		    [ text_archive ]
    		    [ binary_archive ]
    		    )
    		  ]
    		)
    	      | dakota
    	      |
    	      ( experimental
    		[ export_model
    		  [ filename_prefix STRING ]
    		  ( formats
    		    [ text_archive ]
    		    [ binary_archive ]
    		    )
    		  ]
    		[ options_file STRING ]
    		)
    	      ]
    	    [ use_derivatives ]
    	    [ import_build_points_file ALIAS import_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      [ active_only ]
    	      ]
    	    [ export_approx_points_file ALIAS export_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    ]
    	  | ea
    	  | lhs ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( bayes_calibration ALIAS nond_bayes_calibration
    	  ( queso
    	    chain_samples ALIAS samples INTEGER
    	    [ seed INTEGER > 0 ]
    	    [ rng
    	      mt19937
    	      | rnum2
    	      ]
    	    [ emulator
    	      ( gaussian_process ALIAS kriging
    		surfpack
    		| dakota
    		[ build_samples INTEGER ]
    		[ posterior_adaptive ]
    		[ import_build_points_file ALIAS import_points_file STRING
    		  [ ( custom_annotated
    		      [ header ]
    		      [ eval_id ]
    		      [ interface_id ]
    		      )
    		  | annotated
    		  | freeform ]
    		  [ active_only ]
    		  ]
    		)
    	      |
    	      ( pce
    		[ p_refinement
    		  uniform
    		  |
    		  ( dimension_adaptive
    		    sobol
    		    | decay
    		    | generalized
    		    )
    		  ]
    		[ max_refinement_iterations INTEGER >= 0 ]
    		( quadrature_order INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( sparse_grid_level INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ restricted
    		  | unrestricted ]
    		  [ nested
    		  | non_nested ]
    		  )
    		| cubature_integrand INTEGER
    		|
    		( expansion_order INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ basis_type
    		    tensor_product
    		    | total_order
    		    |
    		    ( adapted
    		      [ advancements INTEGER ]
    		      [ soft_convergence_limit INTEGER ]
    		      )
    		    ]
    		  ( collocation_points INTEGER
    		    [ ( least_squares
    			[ svd
    			| equality_constrained ]
    			)
    		    |
    		    ( orthogonal_matching_pursuit ALIAS omp
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    | basis_pursuit ALIAS bp
    		    |
    		    ( basis_pursuit_denoising ALIAS bpdn
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_angle_regression ALIAS lars
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_absolute_shrinkage ALIAS lasso
    		      [ noise_tolerance REALLIST ]
    		      [ l2_penalty REAL ]
    		      ]
    		    [ cross_validation
    		      [ noise_only ]
    		      [ max_cv_order_candidates INTEGER >= 0 ]
    		      ]
    		    [ ratio_order REAL ]
    		    [ response_scaling ]
    		    [ use_derivatives ]
    		    [ tensor_grid ]
    		    [ reuse_points ALIAS reuse_samples ]
    		    [ max_solver_iterations INTEGER >= 0 ]
    		    )
    		  |
    		  ( collocation_ratio REAL
    		    [ ( least_squares
    			[ svd
    			| equality_constrained ]
    			)
    		    |
    		    ( orthogonal_matching_pursuit ALIAS omp
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    | basis_pursuit ALIAS bp
    		    |
    		    ( basis_pursuit_denoising ALIAS bpdn
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_angle_regression ALIAS lars
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_absolute_shrinkage ALIAS lasso
    		      [ noise_tolerance REALLIST ]
    		      [ l2_penalty REAL ]
    		      ]
    		    [ cross_validation
    		      [ noise_only ]
    		      [ max_cv_order_candidates INTEGER >= 0 ]
    		      ]
    		    [ ratio_order REAL ]
    		    [ response_scaling ]
    		    [ use_derivatives ]
    		    [ tensor_grid ]
    		    [ reuse_points ALIAS reuse_samples ]
    		    [ max_solver_iterations INTEGER >= 0 ]
    		    )
    		  |
    		  ( expansion_samples INTEGER
    		    [ reuse_points ALIAS reuse_samples ]
    		    )
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  [ posterior_adaptive ]
    		  )
    		|
    		( orthogonal_least_interpolation ALIAS least_interpolation ALIAS oli
    		  collocation_points INTEGER
    		  [ tensor_grid INTEGERLIST ]
    		  [ reuse_points ALIAS reuse_samples ]
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  [ posterior_adaptive ]
    		  )
    		| import_expansion_file STRING
    		[ askey
    		| wiener ]
    		[ normalized ]
    		[ export_expansion_file STRING ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      |
    	      ( ml_pce
    		[ allocation_control
    		  ( estimator_variance
    		    [ estimator_rate REAL ]
    		    )
    		  | rip_sampling
    		  ]
    		[ discrepancy_emulation
    		  distinct ALIAS paired
    		  | recursive
    		  ]
    		( expansion_order_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ basis_type
    		    tensor_product
    		    | total_order
    		    |
    		    ( adapted
    		      [ advancements INTEGER ]
    		      [ soft_convergence_limit INTEGER ]
    		      )
    		    ]
    		  ( collocation_ratio REAL
    		    [ collocation_points_sequence ALIAS pilot_samples INTEGERLIST ]
    		    [ ( least_squares
    			[ svd
    			| equality_constrained ]
    			)
    		    |
    		    ( orthogonal_matching_pursuit ALIAS omp
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    | basis_pursuit ALIAS bp
    		    |
    		    ( basis_pursuit_denoising ALIAS bpdn
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_angle_regression ALIAS lars
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_absolute_shrinkage ALIAS lasso
    		      [ noise_tolerance REALLIST ]
    		      [ l2_penalty REAL ]
    		      ]
    		    [ cross_validation
    		      [ noise_only ]
    		      [ max_cv_order_candidates INTEGER >= 0 ]
    		      ]
    		    [ ratio_order REAL ]
    		    [ response_scaling ]
    		    [ use_derivatives ]
    		    [ tensor_grid ]
    		    [ reuse_points ALIAS reuse_samples ]
    		    [ max_solver_iterations INTEGER >= 0 ]
    		    )
    		  |
    		  ( expansion_samples_sequence INTEGERLIST
    		    [ reuse_points ALIAS reuse_samples ]
    		    )
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  )
    		|
    		( orthogonal_least_interpolation ALIAS least_interpolation ALIAS oli
    		  collocation_points_sequence INTEGERLIST
    		  [ tensor_grid INTEGERLIST ]
    		  [ reuse_points ALIAS reuse_samples ]
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  )
    		[ askey
    		| wiener ]
    		[ normalized ]
    		[ export_expansion_file STRING ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      |
    	      ( mf_pce
    		[ p_refinement
    		  uniform
    		  |
    		  ( dimension_adaptive
    		    sobol
    		    | decay
    		    | generalized
    		    )
    		  ]
    		[ max_refinement_iterations INTEGER >= 0 ]
    		[ allocation_control
    		  greedy
    		  ]
    		[ discrepancy_emulation
    		  distinct ALIAS paired
    		  | recursive
    		  ]
    		( quadrature_order_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( sparse_grid_level_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ restricted
    		  | unrestricted ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( expansion_order_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ basis_type
    		    tensor_product
    		    | total_order
    		    |
    		    ( adapted
    		      [ advancements INTEGER ]
    		      [ soft_convergence_limit INTEGER ]
    		      )
    		    ]
    		  ( collocation_ratio REAL
    		    [ collocation_points_sequence ALIAS pilot_samples INTEGERLIST ]
    		    [ ( least_squares
    			[ svd
    			| equality_constrained ]
    			)
    		    |
    		    ( orthogonal_matching_pursuit ALIAS omp
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    | basis_pursuit ALIAS bp
    		    |
    		    ( basis_pursuit_denoising ALIAS bpdn
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_angle_regression ALIAS lars
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_absolute_shrinkage ALIAS lasso
    		      [ noise_tolerance REALLIST ]
    		      [ l2_penalty REAL ]
    		      ]
    		    [ cross_validation
    		      [ noise_only ]
    		      [ max_cv_order_candidates INTEGER >= 0 ]
    		      ]
    		    [ ratio_order REAL ]
    		    [ response_scaling ]
    		    [ use_derivatives ]
    		    [ tensor_grid ]
    		    [ reuse_points ALIAS reuse_samples ]
    		    [ max_solver_iterations INTEGER >= 0 ]
    		    )
    		  |
    		  ( expansion_samples_sequence INTEGERLIST
    		    [ reuse_points ALIAS reuse_samples ]
    		    )
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  )
    		|
    		( orthogonal_least_interpolation ALIAS least_interpolation ALIAS oli
    		  collocation_points_sequence INTEGERLIST
    		  [ tensor_grid INTEGERLIST ]
    		  [ reuse_points ALIAS reuse_samples ]
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  )
    		[ askey
    		| wiener ]
    		[ normalized ]
    		[ export_expansion_file STRING ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      |
    	      ( sc
    		[ ( p_refinement
    		    uniform
    		    |
    		    ( dimension_adaptive
    		      sobol
    		      | generalized
    		      )
    		    )
    		|
    		( h_refinement
    		  uniform
    		  |
    		  ( dimension_adaptive
    		    sobol
    		    | generalized
    		    )
    		  | local_adaptive
    		  ]
    		[ max_refinement_iterations INTEGER >= 0 ]
    		( quadrature_order INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( sparse_grid_level INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ nodal
    		  | hierarchical ]
    		  [ restricted
    		  | unrestricted ]
    		  [ nested
    		  | non_nested ]
    		  )
    		[ piecewise
    		| askey
    		| wiener ]
    		[ use_derivatives ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      |
    	      ( mf_sc
    		[ ( p_refinement
    		    uniform
    		    |
    		    ( dimension_adaptive
    		      sobol
    		      | generalized
    		      )
    		    )
    		|
    		( h_refinement
    		  uniform
    		  |
    		  ( dimension_adaptive
    		    sobol
    		    | generalized
    		    )
    		  | local_adaptive
    		  ]
    		[ max_refinement_iterations INTEGER >= 0 ]
    		[ allocation_control
    		  greedy
    		  ]
    		[ discrepancy_emulation
    		  distinct ALIAS paired
    		  | recursive
    		  ]
    		( quadrature_order_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( sparse_grid_level_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ nodal
    		  | hierarchical ]
    		  [ restricted
    		  | unrestricted ]
    		  [ nested
    		  | non_nested ]
    		  )
    		[ piecewise
    		| askey
    		| wiener ]
    		[ use_derivatives ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      ]
    	    [ standardized_space ]
    	    [ logit_transform ]
    	    [ export_chain_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    [ dram
    	    | delayed_rejection
    	    | adaptive_metropolis
    	    | metropolis_hastings
    	    | multilevel ]
    	    [ pre_solve
    	      sqp
    	      | nip
    	      | none
    	      ]
    	    [ proposal_covariance
    	      ( prior
    		[ multiplier REAL > 0.0 ]
    		)
    	      |
    	      ( derivatives
    		[ update_period INTEGER ]
    		)
    	      |
    	      ( values REALLIST
    		diagonal
    		| matrix
    		)
    	      |
    	      ( filename STRING
    		diagonal
    		| matrix
    		)
    	      ]
    	    [ options_file STRING ]
    	    )
    	  |
    	  ( gpmsa
    	    chain_samples ALIAS samples INTEGER
    	    [ seed INTEGER > 0 ]
    	    [ rng
    	      mt19937
    	      | rnum2
    	      ]
    	    build_samples INTEGER
    	    [ import_build_points_file ALIAS import_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    [ standardized_space ]
    	    [ logit_transform ]
    	    [ gpmsa_normalize ]
    	    [ export_chain_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    [ dram
    	    | delayed_rejection
    	    | adaptive_metropolis
    	    | metropolis_hastings ]
    	    [ proposal_covariance
    	      ( prior
    		[ multiplier REAL > 0.0 ]
    		)
    	      |
    	      ( derivatives
    		[ update_period INTEGER ]
    		)
    	      |
    	      ( values REALLIST
    		diagonal
    		| matrix
    		)
    	      |
    	      ( filename STRING
    		diagonal
    		| matrix
    		)
    	      ]
    	    [ options_file STRING ]
    	    )
    	  |
    	  ( wasabi
    	    pushforward_samples INTEGER
    	    [ seed INTEGER > 0 ]
    	    [ emulator
    	      ( gaussian_process ALIAS kriging
    		surfpack
    		| dakota
    		[ build_samples INTEGER ]
    		[ posterior_adaptive ]
    		[ import_build_points_file ALIAS import_points_file STRING
    		  [ ( custom_annotated
    		      [ header ]
    		      [ eval_id ]
    		      [ interface_id ]
    		      )
    		  | annotated
    		  | freeform ]
    		  [ active_only ]
    		  ]
    		)
    	      |
    	      ( pce
    		[ p_refinement
    		  uniform
    		  |
    		  ( dimension_adaptive
    		    sobol
    		    | decay
    		    | generalized
    		    )
    		  ]
    		[ max_refinement_iterations INTEGER >= 0 ]
    		( quadrature_order INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( sparse_grid_level INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ restricted
    		  | unrestricted ]
    		  [ nested
    		  | non_nested ]
    		  )
    		| cubature_integrand INTEGER
    		|
    		( expansion_order INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ basis_type
    		    tensor_product
    		    | total_order
    		    |
    		    ( adapted
    		      [ advancements INTEGER ]
    		      [ soft_convergence_limit INTEGER ]
    		      )
    		    ]
    		  ( collocation_points INTEGER
    		    [ ( least_squares
    			[ svd
    			| equality_constrained ]
    			)
    		    |
    		    ( orthogonal_matching_pursuit ALIAS omp
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    | basis_pursuit ALIAS bp
    		    |
    		    ( basis_pursuit_denoising ALIAS bpdn
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_angle_regression ALIAS lars
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_absolute_shrinkage ALIAS lasso
    		      [ noise_tolerance REALLIST ]
    		      [ l2_penalty REAL ]
    		      ]
    		    [ cross_validation
    		      [ noise_only ]
    		      [ max_cv_order_candidates INTEGER >= 0 ]
    		      ]
    		    [ ratio_order REAL ]
    		    [ response_scaling ]
    		    [ use_derivatives ]
    		    [ tensor_grid ]
    		    [ reuse_points ALIAS reuse_samples ]
    		    [ max_solver_iterations INTEGER >= 0 ]
    		    )
    		  |
    		  ( collocation_ratio REAL
    		    [ ( least_squares
    			[ svd
    			| equality_constrained ]
    			)
    		    |
    		    ( orthogonal_matching_pursuit ALIAS omp
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    | basis_pursuit ALIAS bp
    		    |
    		    ( basis_pursuit_denoising ALIAS bpdn
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_angle_regression ALIAS lars
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_absolute_shrinkage ALIAS lasso
    		      [ noise_tolerance REALLIST ]
    		      [ l2_penalty REAL ]
    		      ]
    		    [ cross_validation
    		      [ noise_only ]
    		      [ max_cv_order_candidates INTEGER >= 0 ]
    		      ]
    		    [ ratio_order REAL ]
    		    [ response_scaling ]
    		    [ use_derivatives ]
    		    [ tensor_grid ]
    		    [ reuse_points ALIAS reuse_samples ]
    		    [ max_solver_iterations INTEGER >= 0 ]
    		    )
    		  |
    		  ( expansion_samples INTEGER
    		    [ reuse_points ALIAS reuse_samples ]
    		    )
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  [ posterior_adaptive ]
    		  )
    		|
    		( orthogonal_least_interpolation ALIAS least_interpolation ALIAS oli
    		  collocation_points INTEGER
    		  [ tensor_grid INTEGERLIST ]
    		  [ reuse_points ALIAS reuse_samples ]
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  [ posterior_adaptive ]
    		  )
    		| import_expansion_file STRING
    		[ askey
    		| wiener ]
    		[ normalized ]
    		[ export_expansion_file STRING ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      |
    	      ( ml_pce
    		[ allocation_control
    		  ( estimator_variance
    		    [ estimator_rate REAL ]
    		    )
    		  | rip_sampling
    		  ]
    		[ discrepancy_emulation
    		  distinct ALIAS paired
    		  | recursive
    		  ]
    		( expansion_order_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ basis_type
    		    tensor_product
    		    | total_order
    		    |
    		    ( adapted
    		      [ advancements INTEGER ]
    		      [ soft_convergence_limit INTEGER ]
    		      )
    		    ]
    		  ( collocation_ratio REAL
    		    [ collocation_points_sequence ALIAS pilot_samples INTEGERLIST ]
    		    [ ( least_squares
    			[ svd
    			| equality_constrained ]
    			)
    		    |
    		    ( orthogonal_matching_pursuit ALIAS omp
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    | basis_pursuit ALIAS bp
    		    |
    		    ( basis_pursuit_denoising ALIAS bpdn
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_angle_regression ALIAS lars
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_absolute_shrinkage ALIAS lasso
    		      [ noise_tolerance REALLIST ]
    		      [ l2_penalty REAL ]
    		      ]
    		    [ cross_validation
    		      [ noise_only ]
    		      [ max_cv_order_candidates INTEGER >= 0 ]
    		      ]
    		    [ ratio_order REAL ]
    		    [ response_scaling ]
    		    [ use_derivatives ]
    		    [ tensor_grid ]
    		    [ reuse_points ALIAS reuse_samples ]
    		    [ max_solver_iterations INTEGER >= 0 ]
    		    )
    		  |
    		  ( expansion_samples_sequence INTEGERLIST
    		    [ reuse_points ALIAS reuse_samples ]
    		    )
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  )
    		|
    		( orthogonal_least_interpolation ALIAS least_interpolation ALIAS oli
    		  collocation_points_sequence INTEGERLIST
    		  [ tensor_grid INTEGERLIST ]
    		  [ reuse_points ALIAS reuse_samples ]
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  )
    		[ askey
    		| wiener ]
    		[ normalized ]
    		[ export_expansion_file STRING ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      |
    	      ( mf_pce
    		[ p_refinement
    		  uniform
    		  |
    		  ( dimension_adaptive
    		    sobol
    		    | decay
    		    | generalized
    		    )
    		  ]
    		[ max_refinement_iterations INTEGER >= 0 ]
    		[ allocation_control
    		  greedy
    		  ]
    		[ discrepancy_emulation
    		  distinct ALIAS paired
    		  | recursive
    		  ]
    		( quadrature_order_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( sparse_grid_level_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ restricted
    		  | unrestricted ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( expansion_order_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ basis_type
    		    tensor_product
    		    | total_order
    		    |
    		    ( adapted
    		      [ advancements INTEGER ]
    		      [ soft_convergence_limit INTEGER ]
    		      )
    		    ]
    		  ( collocation_ratio REAL
    		    [ collocation_points_sequence ALIAS pilot_samples INTEGERLIST ]
    		    [ ( least_squares
    			[ svd
    			| equality_constrained ]
    			)
    		    |
    		    ( orthogonal_matching_pursuit ALIAS omp
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    | basis_pursuit ALIAS bp
    		    |
    		    ( basis_pursuit_denoising ALIAS bpdn
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_angle_regression ALIAS lars
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_absolute_shrinkage ALIAS lasso
    		      [ noise_tolerance REALLIST ]
    		      [ l2_penalty REAL ]
    		      ]
    		    [ cross_validation
    		      [ noise_only ]
    		      [ max_cv_order_candidates INTEGER >= 0 ]
    		      ]
    		    [ ratio_order REAL ]
    		    [ response_scaling ]
    		    [ use_derivatives ]
    		    [ tensor_grid ]
    		    [ reuse_points ALIAS reuse_samples ]
    		    [ max_solver_iterations INTEGER >= 0 ]
    		    )
    		  |
    		  ( expansion_samples_sequence INTEGERLIST
    		    [ reuse_points ALIAS reuse_samples ]
    		    )
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  )
    		|
    		( orthogonal_least_interpolation ALIAS least_interpolation ALIAS oli
    		  collocation_points_sequence INTEGERLIST
    		  [ tensor_grid INTEGERLIST ]
    		  [ reuse_points ALIAS reuse_samples ]
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  )
    		[ askey
    		| wiener ]
    		[ normalized ]
    		[ export_expansion_file STRING ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      |
    	      ( sc
    		[ ( p_refinement
    		    uniform
    		    |
    		    ( dimension_adaptive
    		      sobol
    		      | generalized
    		      )
    		    )
    		|
    		( h_refinement
    		  uniform
    		  |
    		  ( dimension_adaptive
    		    sobol
    		    | generalized
    		    )
    		  | local_adaptive
    		  ]
    		[ max_refinement_iterations INTEGER >= 0 ]
    		( quadrature_order INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( sparse_grid_level INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ nodal
    		  | hierarchical ]
    		  [ restricted
    		  | unrestricted ]
    		  [ nested
    		  | non_nested ]
    		  )
    		[ piecewise
    		| askey
    		| wiener ]
    		[ use_derivatives ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      |
    	      ( mf_sc
    		[ ( p_refinement
    		    uniform
    		    |
    		    ( dimension_adaptive
    		      sobol
    		      | generalized
    		      )
    		    )
    		|
    		( h_refinement
    		  uniform
    		  |
    		  ( dimension_adaptive
    		    sobol
    		    | generalized
    		    )
    		  | local_adaptive
    		  ]
    		[ max_refinement_iterations INTEGER >= 0 ]
    		[ allocation_control
    		  greedy
    		  ]
    		[ discrepancy_emulation
    		  distinct ALIAS paired
    		  | recursive
    		  ]
    		( quadrature_order_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( sparse_grid_level_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ nodal
    		  | hierarchical ]
    		  [ restricted
    		  | unrestricted ]
    		  [ nested
    		  | non_nested ]
    		  )
    		[ piecewise
    		| askey
    		| wiener ]
    		[ use_derivatives ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      ]
    	    [ standardized_space ]
    	    ( data_distribution
    	      ( gaussian
    		means REALLIST
    		( covariance REALLIST
    		  diagonal
    		  | matrix
    		  )
    		)
    	      | obs_data_filename STRING
    	      )
    	    [ posterior_samples_import_filename STRING ]
    	    [ generate_posterior_samples
    	      [ posterior_samples_export_filename STRING ]
    	      ]
    	    [ evaluate_posterior_density
    	      [ posterior_density_export_filename STRING ]
    	      ]
    	    )
    	  |
    	  ( dream
    	    chain_samples ALIAS samples INTEGER
    	    [ seed INTEGER > 0 ]
    	    [ chains INTEGER >= 3 ]
    	    [ num_cr INTEGER >= 1 ]
    	    [ crossover_chain_pairs INTEGER >= 0 ]
    	    [ gr_threshold REAL > 0.0 ]
    	    [ jump_step INTEGER >= 0 ]
    	    [ emulator
    	      ( gaussian_process ALIAS kriging
    		surfpack
    		| dakota
    		[ build_samples INTEGER ]
    		[ posterior_adaptive ]
    		[ import_build_points_file ALIAS import_points_file STRING
    		  [ ( custom_annotated
    		      [ header ]
    		      [ eval_id ]
    		      [ interface_id ]
    		      )
    		  | annotated
    		  | freeform ]
    		  [ active_only ]
    		  ]
    		)
    	      |
    	      ( pce
    		[ p_refinement
    		  uniform
    		  |
    		  ( dimension_adaptive
    		    sobol
    		    | decay
    		    | generalized
    		    )
    		  ]
    		[ max_refinement_iterations INTEGER >= 0 ]
    		( quadrature_order INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( sparse_grid_level INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ restricted
    		  | unrestricted ]
    		  [ nested
    		  | non_nested ]
    		  )
    		| cubature_integrand INTEGER
    		|
    		( expansion_order INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ basis_type
    		    tensor_product
    		    | total_order
    		    |
    		    ( adapted
    		      [ advancements INTEGER ]
    		      [ soft_convergence_limit INTEGER ]
    		      )
    		    ]
    		  ( collocation_points INTEGER
    		    [ ( least_squares
    			[ svd
    			| equality_constrained ]
    			)
    		    |
    		    ( orthogonal_matching_pursuit ALIAS omp
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    | basis_pursuit ALIAS bp
    		    |
    		    ( basis_pursuit_denoising ALIAS bpdn
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_angle_regression ALIAS lars
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_absolute_shrinkage ALIAS lasso
    		      [ noise_tolerance REALLIST ]
    		      [ l2_penalty REAL ]
    		      ]
    		    [ cross_validation
    		      [ noise_only ]
    		      [ max_cv_order_candidates INTEGER >= 0 ]
    		      ]
    		    [ ratio_order REAL ]
    		    [ response_scaling ]
    		    [ use_derivatives ]
    		    [ tensor_grid ]
    		    [ reuse_points ALIAS reuse_samples ]
    		    [ max_solver_iterations INTEGER >= 0 ]
    		    )
    		  |
    		  ( collocation_ratio REAL
    		    [ ( least_squares
    			[ svd
    			| equality_constrained ]
    			)
    		    |
    		    ( orthogonal_matching_pursuit ALIAS omp
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    | basis_pursuit ALIAS bp
    		    |
    		    ( basis_pursuit_denoising ALIAS bpdn
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_angle_regression ALIAS lars
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_absolute_shrinkage ALIAS lasso
    		      [ noise_tolerance REALLIST ]
    		      [ l2_penalty REAL ]
    		      ]
    		    [ cross_validation
    		      [ noise_only ]
    		      [ max_cv_order_candidates INTEGER >= 0 ]
    		      ]
    		    [ ratio_order REAL ]
    		    [ response_scaling ]
    		    [ use_derivatives ]
    		    [ tensor_grid ]
    		    [ reuse_points ALIAS reuse_samples ]
    		    [ max_solver_iterations INTEGER >= 0 ]
    		    )
    		  |
    		  ( expansion_samples INTEGER
    		    [ reuse_points ALIAS reuse_samples ]
    		    )
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  [ posterior_adaptive ]
    		  )
    		|
    		( orthogonal_least_interpolation ALIAS least_interpolation ALIAS oli
    		  collocation_points INTEGER
    		  [ tensor_grid INTEGERLIST ]
    		  [ reuse_points ALIAS reuse_samples ]
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  [ posterior_adaptive ]
    		  )
    		| import_expansion_file STRING
    		[ askey
    		| wiener ]
    		[ normalized ]
    		[ export_expansion_file STRING ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      |
    	      ( ml_pce
    		[ allocation_control
    		  ( estimator_variance
    		    [ estimator_rate REAL ]
    		    )
    		  | rip_sampling
    		  ]
    		[ discrepancy_emulation
    		  distinct ALIAS paired
    		  | recursive
    		  ]
    		( expansion_order_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ basis_type
    		    tensor_product
    		    | total_order
    		    |
    		    ( adapted
    		      [ advancements INTEGER ]
    		      [ soft_convergence_limit INTEGER ]
    		      )
    		    ]
    		  ( collocation_ratio REAL
    		    [ collocation_points_sequence ALIAS pilot_samples INTEGERLIST ]
    		    [ ( least_squares
    			[ svd
    			| equality_constrained ]
    			)
    		    |
    		    ( orthogonal_matching_pursuit ALIAS omp
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    | basis_pursuit ALIAS bp
    		    |
    		    ( basis_pursuit_denoising ALIAS bpdn
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_angle_regression ALIAS lars
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_absolute_shrinkage ALIAS lasso
    		      [ noise_tolerance REALLIST ]
    		      [ l2_penalty REAL ]
    		      ]
    		    [ cross_validation
    		      [ noise_only ]
    		      [ max_cv_order_candidates INTEGER >= 0 ]
    		      ]
    		    [ ratio_order REAL ]
    		    [ response_scaling ]
    		    [ use_derivatives ]
    		    [ tensor_grid ]
    		    [ reuse_points ALIAS reuse_samples ]
    		    [ max_solver_iterations INTEGER >= 0 ]
    		    )
    		  |
    		  ( expansion_samples_sequence INTEGERLIST
    		    [ reuse_points ALIAS reuse_samples ]
    		    )
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  )
    		|
    		( orthogonal_least_interpolation ALIAS least_interpolation ALIAS oli
    		  collocation_points_sequence INTEGERLIST
    		  [ tensor_grid INTEGERLIST ]
    		  [ reuse_points ALIAS reuse_samples ]
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  )
    		[ askey
    		| wiener ]
    		[ normalized ]
    		[ export_expansion_file STRING ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      |
    	      ( mf_pce
    		[ p_refinement
    		  uniform
    		  |
    		  ( dimension_adaptive
    		    sobol
    		    | decay
    		    | generalized
    		    )
    		  ]
    		[ max_refinement_iterations INTEGER >= 0 ]
    		[ allocation_control
    		  greedy
    		  ]
    		[ discrepancy_emulation
    		  distinct ALIAS paired
    		  | recursive
    		  ]
    		( quadrature_order_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( sparse_grid_level_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ restricted
    		  | unrestricted ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( expansion_order_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ basis_type
    		    tensor_product
    		    | total_order
    		    |
    		    ( adapted
    		      [ advancements INTEGER ]
    		      [ soft_convergence_limit INTEGER ]
    		      )
    		    ]
    		  ( collocation_ratio REAL
    		    [ collocation_points_sequence ALIAS pilot_samples INTEGERLIST ]
    		    [ ( least_squares
    			[ svd
    			| equality_constrained ]
    			)
    		    |
    		    ( orthogonal_matching_pursuit ALIAS omp
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    | basis_pursuit ALIAS bp
    		    |
    		    ( basis_pursuit_denoising ALIAS bpdn
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_angle_regression ALIAS lars
    		      [ noise_tolerance REALLIST ]
    		      ]
    		    |
    		    ( least_absolute_shrinkage ALIAS lasso
    		      [ noise_tolerance REALLIST ]
    		      [ l2_penalty REAL ]
    		      ]
    		    [ cross_validation
    		      [ noise_only ]
    		      [ max_cv_order_candidates INTEGER >= 0 ]
    		      ]
    		    [ ratio_order REAL ]
    		    [ response_scaling ]
    		    [ use_derivatives ]
    		    [ tensor_grid ]
    		    [ reuse_points ALIAS reuse_samples ]
    		    [ max_solver_iterations INTEGER >= 0 ]
    		    )
    		  |
    		  ( expansion_samples_sequence INTEGERLIST
    		    [ reuse_points ALIAS reuse_samples ]
    		    )
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  )
    		|
    		( orthogonal_least_interpolation ALIAS least_interpolation ALIAS oli
    		  collocation_points_sequence INTEGERLIST
    		  [ tensor_grid INTEGERLIST ]
    		  [ reuse_points ALIAS reuse_samples ]
    		  [ import_build_points_file ALIAS import_points_file STRING
    		    [ ( custom_annotated
    			[ header ]
    			[ eval_id ]
    			[ interface_id ]
    			)
    		    | annotated
    		    | freeform ]
    		    [ active_only ]
    		    ]
    		  )
    		[ askey
    		| wiener ]
    		[ normalized ]
    		[ export_expansion_file STRING ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      |
    	      ( sc
    		[ ( p_refinement
    		    uniform
    		    |
    		    ( dimension_adaptive
    		      sobol
    		      | generalized
    		      )
    		    )
    		|
    		( h_refinement
    		  uniform
    		  |
    		  ( dimension_adaptive
    		    sobol
    		    | generalized
    		    )
    		  | local_adaptive
    		  ]
    		[ max_refinement_iterations INTEGER >= 0 ]
    		( quadrature_order INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( sparse_grid_level INTEGER
    		  [ dimension_preference REALLIST ]
    		  [ nodal
    		  | hierarchical ]
    		  [ restricted
    		  | unrestricted ]
    		  [ nested
    		  | non_nested ]
    		  )
    		[ piecewise
    		| askey
    		| wiener ]
    		[ use_derivatives ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      |
    	      ( mf_sc
    		[ ( p_refinement
    		    uniform
    		    |
    		    ( dimension_adaptive
    		      sobol
    		      | generalized
    		      )
    		    )
    		|
    		( h_refinement
    		  uniform
    		  |
    		  ( dimension_adaptive
    		    sobol
    		    | generalized
    		    )
    		  | local_adaptive
    		  ]
    		[ max_refinement_iterations INTEGER >= 0 ]
    		[ allocation_control
    		  greedy
    		  ]
    		[ discrepancy_emulation
    		  distinct ALIAS paired
    		  | recursive
    		  ]
    		( quadrature_order_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ nested
    		  | non_nested ]
    		  )
    		|
    		( sparse_grid_level_sequence INTEGERLIST
    		  [ dimension_preference REALLIST ]
    		  [ nodal
    		  | hierarchical ]
    		  [ restricted
    		  | unrestricted ]
    		  [ nested
    		  | non_nested ]
    		  )
    		[ piecewise
    		| askey
    		| wiener ]
    		[ use_derivatives ]
    		[ diagonal_covariance
    		| full_covariance ]
    		)
    	      ]
    	    [ standardized_space ]
    	    [ export_chain_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    )
    	  |
    	  ( muq
    	    chain_samples ALIAS samples INTEGER
    	    [ seed INTEGER > 0 ]
    	    [ rng
    	      mt19937
    	      | rnum2
    	      ]
    	    [ export_chain_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    [ ( adaptive_metropolis
    		[ period_num_steps INTEGER ]
    		[ starting_step INTEGER ]
    		[ adapt_scale REAL ]
    		)
    	    |
    	    ( delayed_rejection
    	      [ num_stages INTEGER ]
    	      [ scale_type STRING ]
    	      [ delay_scale REAL ]
    	      ]
    	    |
    	    ( dili
    	      [ hessian_type STRING ]
    	      [ adapt_interval INTEGER ]
    	      [ adapt_start INTEGER ]
    	      [ adapt_end INTEGER ]
    	      [ initial_weight INTEGER ]
    	      [ hess_tolerance REAL ]
    	      [ lis_tolerance REAL ]
    	      [ ses_num_eigs INTEGER ]
    	      [ ses_rel_tol REAL ]
    	      [ ses_abs_tol REAL ]
    	      [ ses_exp_rank INTEGER ]
    	      [ ses_overs_factor INTEGER ]
    	      [ ses_block_size INTEGER ]
    	      ]
    	    |
    	    ( dram
    	      [ num_stages INTEGER ]
    	      [ scale_type STRING ]
    	      [ delay_scale REAL ]
    	      [ period_num_steps INTEGER ]
    	      [ starting_step INTEGER ]
    	      [ adapt_scale REAL ]
    	      ]
    	    |
    	    ( multilevel_mcmc
    	      [ initial_chain_samples INTEGER ]
    	      target_variance REAL
    	      [ greedy_resampling_factor REAL ]
    	      [ subsampling_steps INTEGERLIST ]
    	      ]
    	    |
    	    ( mala
    	      [ step_size REAL ]
    	      ]
    	    | metropolis_hastings ]
    	    [ pre_solve
    	      sqp
    	      | nip
    	      | none
    	      ]
    	    [ proposal_covariance
    	      ( prior
    		[ multiplier REAL > 0.0 ]
    		)
    	      |
    	      ( derivatives
    		[ update_period INTEGER ]
    		)
    	      |
    	      ( values REALLIST
    		diagonal
    		| matrix
    		)
    	      |
    	      ( filename STRING
    		diagonal
    		| matrix
    		)
    	      ]
    	    )
    	  [ experimental_design
    	    initial_samples ALIAS samples INTEGER
    	    num_candidates INTEGER > 0
    	    [ max_hifi_evaluations INTEGER >= 0 ]
    	    [ batch_size INTEGER >= 1 ]
    	    [ import_candidate_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    [ ksg2 ]
    	    ]
    	  [ calibrate_error_multipliers
    	    one
    	    | per_experiment
    	    | per_response
    	    | both
    	    [ hyperprior_alphas REALLIST
    	      hyperprior_betas REALLIST
    	      ]
    	    ]
    	  [ burn_in_samples INTEGER ]
    	  [ posterior_stats
    	    [ kl_divergence ]
    	    [ mutual_info
    	      [ ksg2 ]
    	      ]
    	    [ kde ]
    	    ]
    	  [ chain_diagnostics
    	    [ confidence_intervals ]
    	    ]
    	  [ model_evidence
    	    [ mc_approx ]
    	    [ evidence_samples INTEGER ]
    	    [ laplace_approx ]
    	    ]
    	  [ model_discrepancy
    	    [ discrepancy_type
    	      ( gaussian_process ALIAS kriging
    		[ trend_order INTEGER >= 0 ]
    		)
    	      |
    	      ( polynomial
    		[ basis_order INTEGER >= 0 ]
    		)
    	      ]
    	    [ num_prediction_configs INTEGER >= 0 ]
    	    [ prediction_configs REALLIST ]
    	    [ import_prediction_configs STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    [ export_discrepancy_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    [ export_corrected_model_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    [ export_corrected_variance_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    ]
    	  [ sub_sampling_period INTEGER ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ convergence_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ model_pointer STRING ]
    	  [ scaling ]
    	  )
    	|
    	( dace
    	  grid
    	  | random
    	  | oas
    	  | lhs
    	  | oa_lhs
    	  | box_behnken
    	  | central_composite
    	  [ samples INTEGER ]
    	  [ seed INTEGER > 0 ]
    	  [ fixed_seed ]
    	  [ main_effects ]
    	  [ quality_metrics ]
    	  [ variance_based_decomp
    	    [ drop_tolerance REAL ]
    	    [ vbd_sampling_method
    	      ( binned
    		[ num_bins INTEGER ]
    		)
    	      | pick_and_freeze
    	      ]
    	    ]
    	  [ symbols INTEGER ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( fsu_cvt
    	  [ samples INTEGER ]
    	  [ seed INTEGER > 0 ]
    	  [ fixed_seed ]
    	  [ latinize ]
    	  [ quality_metrics ]
    	  [ variance_based_decomp
    	    [ drop_tolerance REAL ]
    	    [ vbd_sampling_method
    	      ( binned
    		[ num_bins INTEGER ]
    		)
    	      | pick_and_freeze
    	      ]
    	    ]
    	  [ trial_type
    	    grid
    	    | halton
    	    | random
    	    ]
    	  [ num_trials INTEGER ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( psuade_moat
    	  [ partitions INTEGERLIST ]
    	  [ samples INTEGER ]
    	  [ seed INTEGER > 0 ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( local_evidence ALIAS nond_local_evidence
    	  [ sqp
    	  | nip ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( local_interval_est ALIAS nond_local_interval_est
    	  [ sqp
    	  | nip ]
    	  [ convergence_tolerance REAL ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( local_reliability ALIAS nond_local_reliability
    	  [ mpp_search
    	    x_taylor_mean
    	    | u_taylor_mean
    	    | x_taylor_mpp
    	    | u_taylor_mpp
    	    | x_two_point
    	    | u_two_point
    	    | x_multi_point
    	    | u_multi_point
    	    | no_approx
    	    [ sqp
    	    | nip ]
    	    [ integration
    	      first_order
    	      | second_order
    	      [ probability_refinement ALIAS sample_refinement
    		import
    		| adapt_import
    		| mm_adapt_import
    		[ refinement_samples INTEGERLIST ]
    		[ seed INTEGER > 0 ]
    		]
    	      ]
    	    ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | reliabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ reliability_levels REALLIST
    	    [ num_reliability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ final_moments
    	    none
    	    | standard
    	    | central
    	    ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( global_reliability ALIAS nond_global_reliability
    	  [ initial_samples INTEGER ]
    	  x_gaussian_process ALIAS x_kriging
    	  | u_gaussian_process ALIAS u_kriging
    	  [ ( surfpack
    	      [ export_model
    		[ filename_prefix STRING ]
    		( formats
    		  [ text_archive ]
    		  [ binary_archive ]
    		  )
    		]
    	      )
    	  | dakota
    	  |
    	  ( experimental
    	    [ export_model
    	      [ filename_prefix STRING ]
    	      ( formats
    		[ text_archive ]
    		[ binary_archive ]
    		)
    	      ]
    	    [ options_file STRING ]
    	    ]
    	  [ import_build_points_file ALIAS import_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    ]
    	  [ export_approx_points_file ALIAS export_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    ]
    	  [ use_derivatives ]
    	  [ seed INTEGER > 0 ]
    	  [ rng
    	    mt19937
    	    | rnum2
    	    ]
    	  [ response_levels REALLIST
    	    [ num_response_levels INTEGERLIST ]
    	    [ compute
    	      probabilities
    	      | gen_reliabilities
    	      [ system
    		series
    		| parallel
    		]
    	      ]
    	    ]
    	  [ probability_levels REALLIST
    	    [ num_probability_levels INTEGERLIST ]
    	    ]
    	  [ gen_reliability_levels REALLIST
    	    [ num_gen_reliability_levels INTEGERLIST ]
    	    ]
    	  [ distribution
    	    cumulative
    	    | complementary
    	    ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ convergence_tolerance REAL ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( fsu_quasi_mc
    	  halton
    	  | hammersley
    	  [ latinize ]
    	  [ quality_metrics ]
    	  [ variance_based_decomp
    	    [ drop_tolerance REAL ]
    	    [ vbd_sampling_method
    	      ( binned
    		[ num_bins INTEGER ]
    		)
    	      | pick_and_freeze
    	      ]
    	    ]
    	  [ samples INTEGER ]
    	  [ fixed_sequence ]
    	  [ sequence_start INTEGERLIST ]
    	  [ sequence_leap INTEGERLIST ]
    	  [ prime_base INTEGERLIST ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ model_pointer STRING ]
    	  )
    	|
    	( vector_parameter_study
    	  final_point REALLIST
    	  | step_vector REALLIST
    	  num_steps INTEGER
    	  [ model_pointer STRING ]
    	  )
    	|
    	( list_parameter_study
    	  list_of_points REALLIST
    	  |
    	  ( import_points_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ eval_id ]
    		[ interface_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ active_only ]
    	    )
    	  [ model_pointer STRING ]
    	  )
    	|
    	( centered_parameter_study
    	  step_vector REALLIST
    	  steps_per_variable ALIAS deltas_per_variable INTEGERLIST
    	  [ model_pointer STRING ]
    	  )
    	|
    	( multidim_parameter_study
    	  partitions INTEGERLIST
    	  [ model_pointer STRING ]
    	  )
    	|
    	( richardson_extrap
    	  estimate_order
    	  | converge_order
    	  | converge_qoi
    	  [ refinement_rate REAL ]
    	  [ convergence_tolerance REAL ]
    	  [ max_iterations INTEGER >= 0 ]
    	  [ model_pointer STRING ]
    	  )
    
    KEYWORD model
    	[ id_model STRING ]
    	( single ALIAS simulation
    	  [ interface_pointer STRING ]
    	  [ solution_level_control STRING ]
    	  [ solution_level_cost REALLIST
    	  | cost_recovery_metadata STRING ]
    	  )
    	|
    	( surrogate
    	  [ id_surrogates INTEGERLIST ]
    	  ( global
    	    ( experimental_gaussian_process
    	      [ trend
    		none
    		| constant
    		| linear
    		| reduced_quadratic
    		| quadratic
    		]
    	      [ num_restarts INTEGER > 1 ]
    	      [ nugget REAL > 0
    	      | find_nugget INTEGER ]
    	      [ options_file STRING ]
    	      [ export_approx_variance_file STRING
    		[ ( custom_annotated
    		    [ header ]
    		    [ eval_id ]
    		    [ interface_id ]
    		    )
    		| annotated
    		| freeform ]
    		]
    	      [ export_model
    		[ filename_prefix STRING ]
    		( formats
    		  [ text_archive ]
    		  [ binary_archive ]
    		  )
    		]
    	      [ import_model
    		[ filename_prefix STRING ]
    		text_archive
    		| binary_archive
    		]
    	      )
    	    |
    	    ( gaussian_process ALIAS kriging
    	      ( dakota
    		[ point_selection ]
    		[ trend
    		  constant
    		  | linear
    		  | reduced_quadratic
    		  ]
    		)
    	      |
    	      ( surfpack
    		[ trend
    		  constant
    		  | linear
    		  | reduced_quadratic
    		  | quadratic
    		  ]
    		[ optimization_method STRING ]
    		[ max_trials INTEGER > 0 ]
    		[ nugget REAL > 0
    		| find_nugget INTEGER ]
    		[ correlation_lengths REALLIST ]
    		[ export_model
    		  [ filename_prefix STRING ]
    		  ( formats
    		    [ text_archive ]
    		    [ binary_archive ]
    		    [ algebraic_file ]
    		    [ algebraic_console ]
    		    )
    		  ]
    		[ import_model
    		  [ filename_prefix STRING ]
    		  text_archive
    		  | binary_archive
    		  ]
    		)
    	      [ export_approx_variance_file STRING
    		[ ( custom_annotated
    		    [ header ]
    		    [ eval_id ]
    		    [ interface_id ]
    		    )
    		| annotated
    		| freeform ]
    		]
    	      )
    	    |
    	    ( mars
    	      [ max_bases INTEGER ]
    	      [ interpolation
    		linear
    		| cubic
    		]
    	      [ export_model
    		[ filename_prefix STRING ]
    		( formats
    		  [ text_archive ]
    		  [ binary_archive ]
    		  )
    		]
    	      [ import_model
    		[ filename_prefix STRING ]
    		text_archive
    		| binary_archive
    		]
    	      )
    	    |
    	    ( moving_least_squares
    	      [ basis_order ALIAS poly_order INTEGER >= 0 ]
    	      [ weight_function INTEGER ]
    	      [ export_model
    		[ filename_prefix STRING ]
    		( formats
    		  [ text_archive ]
    		  [ binary_archive ]
    		  )
    		]
    	      [ import_model
    		[ filename_prefix STRING ]
    		text_archive
    		| binary_archive
    		]
    	      )
    	    |
    	    ( function_train
    	      [ regression_type
    		ls
    		|
    		( rls2
    		  l2_penalty REAL
    		  )
    		]
    	      [ max_solver_iterations INTEGER >= 0 ]
    	      [ max_cross_iterations INTEGER >= 0 ]
    	      [ solver_tolerance REAL ]
    	      [ response_scaling ]
    	      [ tensor_grid ]
    	      [ rounding_tolerance REAL ]
    	      [ arithmetic_tolerance REAL ]
    	      [ start_order ALIAS order INTEGER >= 0
    		[ dimension_preference REALLIST ]
    		]
    	      [ adapt_order ]
    	      [ kick_order INTEGER > 0 ]
    	      [ max_order INTEGER >= 0 ]
    	      [ max_cv_order_candidates INTEGER >= 0 ]
    	      [ start_rank ALIAS rank INTEGER >= 0 ]
    	      [ adapt_rank ]
    	      [ kick_rank INTEGER > 0 ]
    	      [ max_rank INTEGER >= 0 ]
    	      [ max_cv_rank_candidates INTEGER >= 0 ]
    	      )
    	    |
    	    ( neural_network
    	      [ max_nodes ALIAS nodes INTEGER ]
    	      [ range REAL ]
    	      [ random_weight INTEGER ]
    	      [ export_model
    		[ filename_prefix STRING ]
    		( formats
    		  [ text_archive ]
    		  [ binary_archive ]
    		  [ algebraic_file ]
    		  [ algebraic_console ]
    		  )
    		]
    	      [ import_model
    		[ filename_prefix STRING ]
    		text_archive
    		| binary_archive
    		]
    	      )
    	    |
    	    ( radial_basis
    	      [ bases INTEGER ]
    	      [ max_pts INTEGER ]
    	      [ min_partition INTEGER ]
    	      [ max_subsets INTEGER ]
    	      [ export_model
    		[ filename_prefix STRING ]
    		( formats
    		  [ text_archive ]
    		  [ binary_archive ]
    		  [ algebraic_file ]
    		  [ algebraic_console ]
    		  )
    		]
    	      [ import_model
    		[ filename_prefix STRING ]
    		text_archive
    		| binary_archive
    		]
    	      )
    	    |
    	    ( polynomial
    	      basis_order INTEGER >= 0
    	      | linear
    	      | quadratic
    	      | cubic
    	      [ export_model
    		[ filename_prefix STRING ]
    		( formats
    		  [ text_archive ]
    		  [ binary_archive ]
    		  [ algebraic_file ]
    		  [ algebraic_console ]
    		  )
    		]
    	      [ import_model
    		[ filename_prefix STRING ]
    		text_archive
    		| binary_archive
    		]
    	      )
    	    |
    	    ( experimental_polynomial
    	      basis_order INTEGER >= 0
    	      [ options_file STRING ]
    	      [ export_model
    		[ filename_prefix STRING ]
    		( formats
    		  [ text_archive ]
    		  [ binary_archive ]
    		  )
    		]
    	      [ import_model
    		[ filename_prefix STRING ]
    		text_archive
    		| binary_archive
    		]
    	      )
    	    |
    	    ( experimental_python
    	      class_path_and_name STRING
    	      )
    	    [ domain_decomposition
    	      [ cell_type STRING ]
    	      [ support_layers INTEGER ]
    	      [ discontinuity_detection
    		jump_threshold REAL
    		| gradient_threshold REAL
    		]
    	      ]
    	    [ total_points INTEGER
    	    | minimum_points
    	    | recommended_points ]
    	    [ ( dace_method_pointer STRING
    		[ auto_refinement
    		  [ max_iterations INTEGER > 0 ]
    		  [ max_function_evaluations INTEGER > 0 ]
    		  [ convergence_tolerance REAL ]
    		  [ soft_convergence_limit INTEGER >= 0 ]
    		  [ cross_validation_metric STRING
    		    [ folds INTEGER > 0 ]
    		    ]
    		  ]
    		)
    	    | truth_model_pointer ALIAS actual_model_pointer STRING ]
    	    [ reuse_points ALIAS reuse_samples
    	      all
    	      | region
    	      | none
    	      ]
    	    [ import_build_points_file ALIAS import_points_file ALIAS samples_file STRING
    	      [ ( custom_annotated
    		  [ header
    		    [ use_variable_labels ]
    		    ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      |
    	      ( annotated
    		[ use_variable_labels ]
    		]
    	      | freeform ]
    	      [ active_only ]
    	      ]
    	    [ export_approx_points_file ALIAS export_points_file STRING
    	      [ ( custom_annotated
    		  [ header ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      | annotated
    	      | freeform ]
    	      ]
    	    [ use_derivatives ]
    	    [ correction
    	      zeroth_order
    	      | first_order
    	      | second_order
    	      additive
    	      | multiplicative
    	      | combined
    	      ]
    	    [ metrics ALIAS diagnostics STRINGLIST
    	      [ cross_validation
    		[ folds INTEGER
    		| percent REAL ]
    		]
    	      [ press ]
    	      ]
    	    [ import_challenge_points_file ALIAS challenge_points_file STRING
    	      [ ( custom_annotated
    		  [ header
    		    [ use_variable_labels ]
    		    ]
    		  [ eval_id ]
    		  [ interface_id ]
    		  )
    	      |
    	      ( annotated
    		[ use_variable_labels ]
    		]
    	      | freeform ]
    	      [ active_only ]
    	      ]
    	    )
    	  |
    	  ( multipoint
    	    tana
    	    | qmea
    	    truth_model_pointer ALIAS actual_model_pointer STRING
    	    )
    	  |
    	  ( local
    	    taylor_series
    	    truth_model_pointer ALIAS actual_model_pointer STRING
    	    )
    	  |
    	  ( ensemble
    	    ( ordered_model_fidelities ALIAS model_fidelity_sequence STRINGLIST
    	      [ correction
    		zeroth_order
    		| first_order
    		| second_order
    		additive
    		| multiplicative
    		| combined
    		]
    	      )
    	    |
    	    ( truth_model_pointer ALIAS actual_model_pointer STRING
    	      [ approximation_models ALIAS unordered_model_fidelities STRINGLIST ]
    	      )
    	    )
    	  )
    	|
    	( nested
    	  [ optional_interface_pointer STRING
    	    [ optional_interface_responses_pointer STRING ]
    	    ]
    	  ( sub_method_pointer STRING
    	    [ iterator_servers INTEGER > 0 ]
    	    [ iterator_scheduling
    	      dedicated
    	      | peer
    	      ]
    	    [ processors_per_iterator INTEGER > 0 ]
    	    [ primary_variable_mapping STRINGLIST ]
    	    [ secondary_variable_mapping STRINGLIST ]
    	    [ primary_response_mapping REALLIST ]
    	    [ secondary_response_mapping REALLIST ]
    	    [ identity_response_mapping ]
    	    )
    	  )
    	|
    	( active_subspace ALIAS subspace
    	  truth_model_pointer ALIAS actual_model_pointer STRING
    	  [ initial_samples INTEGER ]
    	  [ sample_type
    	    lhs
    	    | random
    	    ]
    	  [ truncation_method
    	    [ bing_li ]
    	    [ constantine ]
    	    [ energy
    	      [ truncation_tolerance REAL ]
    	      ]
    	    [ cross_validation
    	      [ minimum
    	      | relative
    	      | decrease ]
    	      [ relative_tolerance REAL ]
    	      [ decrease_tolerance REAL ]
    	      [ max_rank INTEGER ]
    	      [ exhaustive ]
    	      ]
    	    ]
    	  [ dimension INTEGER ]
    	  [ bootstrap_samples INTEGER ]
    	  [ build_surrogate
    	    [ refinement_samples INTEGERLIST ]
    	    ]
    	  [ normalization
    	    mean_value
    	    | mean_gradient
    	    | local_gradient
    	    ]
    	  )
    	|
    	( adapted_basis
    	  ( truth_model_pointer ALIAS actual_model_pointer STRING
    	    sparse_grid_level INTEGER
    	    |
    	    ( expansion_order INTEGER
    	      collocation_ratio REAL
    	      )
    	    [ dimension INTEGER ]
    	    [ rotation_method
    	      unranked
    	      | ranked
    	      ]
    	    )
    	  [ truncation_tolerance REAL ]
    	  )
    	|
    	( random_field
    	  [ build_source
    	    rf_data_file STRING
    	    | dace_method_pointer STRING
    	    |
    	    ( analytic_covariance
    	      squared_exponential
    	      | exponential
    	      )
    	    ]
    	  [ expansion_form
    	    karhunen_loeve
    	    | principal_components
    	    ]
    	  [ expansion_bases INTEGER ]
    	  [ truncation_tolerance REAL ]
    	  propagation_model_pointer STRING
    	  )
    	[ variables_pointer STRING ]
    	[ responses_pointer STRING ]
    	[ hierarchical_tagging ]
    
    KEYWORD variables
    	[ id_variables STRING ]
    	[ active
    	  all
    	  | design
    	  | uncertain
    	  | aleatory
    	  | epistemic
    	  | state
    	  ]
    	[ mixed
    	| relaxed ]
    	[ continuous_design INTEGER > 0
    	  [ initial_point ALIAS cdv_initial_point REALLIST ]
    	  [ lower_bounds ALIAS cdv_lower_bounds REALLIST ]
    	  [ upper_bounds ALIAS cdv_upper_bounds REALLIST ]
    	  [ scale_types ALIAS cdv_scale_types STRINGLIST ]
    	  [ scales ALIAS cdv_scales REALLIST ]
    	  [ descriptors ALIAS cdv_descriptors STRINGLIST ]
    	  ]
    	[ discrete_design_range INTEGER > 0
    	  [ initial_point ALIAS ddv_initial_point INTEGERLIST ]
    	  [ lower_bounds ALIAS ddv_lower_bounds INTEGERLIST ]
    	  [ upper_bounds ALIAS ddv_upper_bounds INTEGERLIST ]
    	  [ descriptors ALIAS ddv_descriptors STRINGLIST ]
    	  ]
    	[ discrete_design_set
    	  [ integer INTEGER > 0
    	    [ elements_per_variable ALIAS num_set_values INTEGERLIST ]
    	    elements ALIAS set_values INTEGERLIST
    	    [ categorical STRINGLIST
    	      [ adjacency_matrix INTEGERLIST ]
    	      ]
    	    [ initial_point INTEGERLIST ]
    	    [ descriptors STRINGLIST ]
    	    ]
    	  [ string INTEGER > 0
    	    [ elements_per_variable ALIAS num_set_values INTEGERLIST ]
    	    elements ALIAS set_values STRINGLIST
    	    [ adjacency_matrix INTEGERLIST ]
    	    [ initial_point STRINGLIST ]
    	    [ descriptors STRINGLIST ]
    	    ]
    	  [ real INTEGER > 0
    	    [ elements_per_variable ALIAS num_set_values INTEGERLIST ]
    	    elements ALIAS set_values REALLIST
    	    [ categorical STRINGLIST
    	      [ adjacency_matrix INTEGERLIST ]
    	      ]
    	    [ initial_point REALLIST ]
    	    [ descriptors STRINGLIST ]
    	    ]
    	  ]
    	[ normal_uncertain INTEGER > 0
    	  means ALIAS nuv_means REALLIST
    	  std_deviations ALIAS nuv_std_deviations REALLIST
    	  [ lower_bounds ALIAS nuv_lower_bounds REALLIST ]
    	  [ upper_bounds ALIAS nuv_upper_bounds REALLIST ]
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS nuv_descriptors STRINGLIST ]
    	  ]
    	[ lognormal_uncertain INTEGER > 0
    	  ( lambdas ALIAS lnuv_lambdas REALLIST
    	    zetas ALIAS lnuv_zetas REALLIST
    	    )
    	  |
    	  ( means ALIAS lnuv_means REALLIST
    	    std_deviations ALIAS lnuv_std_deviations REALLIST
    	    | error_factors ALIAS lnuv_error_factors REALLIST
    	    )
    	  [ lower_bounds ALIAS lnuv_lower_bounds REALLIST ]
    	  [ upper_bounds ALIAS lnuv_upper_bounds REALLIST ]
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS lnuv_descriptors STRINGLIST ]
    	  ]
    	[ uniform_uncertain INTEGER > 0
    	  lower_bounds ALIAS uuv_lower_bounds REALLIST
    	  upper_bounds ALIAS uuv_upper_bounds REALLIST
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS uuv_descriptors STRINGLIST ]
    	  ]
    	[ loguniform_uncertain INTEGER > 0
    	  lower_bounds ALIAS luuv_lower_bounds REALLIST
    	  upper_bounds ALIAS luuv_upper_bounds REALLIST
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS luuv_descriptors STRINGLIST ]
    	  ]
    	[ triangular_uncertain INTEGER > 0
    	  modes ALIAS tuv_modes REALLIST
    	  lower_bounds ALIAS tuv_lower_bounds REALLIST
    	  upper_bounds ALIAS tuv_upper_bounds REALLIST
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS tuv_descriptors STRINGLIST ]
    	  ]
    	[ exponential_uncertain INTEGER > 0
    	  betas ALIAS euv_betas REALLIST
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS euv_descriptors STRINGLIST ]
    	  ]
    	[ beta_uncertain INTEGER > 0
    	  alphas ALIAS buv_alphas REALLIST
    	  betas ALIAS buv_betas REALLIST
    	  lower_bounds ALIAS buv_lower_bounds REALLIST
    	  upper_bounds ALIAS buv_upper_bounds REALLIST
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS buv_descriptors STRINGLIST ]
    	  ]
    	[ gamma_uncertain INTEGER > 0
    	  alphas ALIAS gauv_alphas REALLIST
    	  betas ALIAS gauv_betas REALLIST
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS gauv_descriptors STRINGLIST ]
    	  ]
    	[ gumbel_uncertain INTEGER > 0
    	  alphas ALIAS guuv_alphas REALLIST
    	  betas ALIAS guuv_betas REALLIST
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS guuv_descriptors STRINGLIST ]
    	  ]
    	[ frechet_uncertain INTEGER > 0
    	  alphas ALIAS fuv_alphas REALLIST
    	  betas ALIAS fuv_betas REALLIST
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS fuv_descriptors STRINGLIST ]
    	  ]
    	[ weibull_uncertain INTEGER > 0
    	  alphas ALIAS wuv_alphas REALLIST
    	  betas ALIAS wuv_betas REALLIST
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS wuv_descriptors STRINGLIST ]
    	  ]
    	[ histogram_bin_uncertain INTEGER > 0
    	  [ pairs_per_variable ALIAS num_pairs INTEGERLIST ]
    	  abscissas ALIAS huv_bin_abscissas REALLIST
    	  ordinates ALIAS huv_bin_ordinates REALLIST
    	  | counts ALIAS huv_bin_counts REALLIST
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS huv_bin_descriptors STRINGLIST ]
    	  ]
    	[ poisson_uncertain INTEGER > 0
    	  lambdas REALLIST
    	  [ initial_point INTEGERLIST ]
    	  [ descriptors STRINGLIST ]
    	  ]
    	[ binomial_uncertain INTEGER > 0
    	  probability_per_trial ALIAS prob_per_trial REALLIST
    	  num_trials INTEGERLIST
    	  [ initial_point INTEGERLIST ]
    	  [ descriptors STRINGLIST ]
    	  ]
    	[ negative_binomial_uncertain INTEGER > 0
    	  probability_per_trial ALIAS prob_per_trial REALLIST
    	  num_trials INTEGERLIST
    	  [ initial_point INTEGERLIST ]
    	  [ descriptors STRINGLIST ]
    	  ]
    	[ geometric_uncertain INTEGER > 0
    	  probability_per_trial ALIAS prob_per_trial REALLIST
    	  [ initial_point INTEGERLIST ]
    	  [ descriptors STRINGLIST ]
    	  ]
    	[ hypergeometric_uncertain INTEGER > 0
    	  total_population INTEGERLIST
    	  selected_population INTEGERLIST
    	  num_drawn INTEGERLIST
    	  [ initial_point INTEGERLIST ]
    	  [ descriptors STRINGLIST ]
    	  ]
    	[ histogram_point_uncertain
    	  [ integer INTEGER > 0
    	    [ pairs_per_variable ALIAS num_pairs INTEGERLIST ]
    	    abscissas INTEGERLIST
    	    counts REALLIST
    	    [ initial_point INTEGERLIST ]
    	    [ descriptors STRINGLIST ]
    	    ]
    	  [ string INTEGER > 0
    	    [ pairs_per_variable ALIAS num_pairs INTEGERLIST ]
    	    abscissas STRINGLIST
    	    counts REALLIST
    	    [ initial_point STRINGLIST ]
    	    [ descriptors STRINGLIST ]
    	    ]
    	  [ real INTEGER > 0
    	    [ pairs_per_variable ALIAS num_pairs INTEGERLIST ]
    	    abscissas REALLIST
    	    counts REALLIST
    	    [ initial_point REALLIST ]
    	    [ descriptors STRINGLIST ]
    	    ]
    	  ]
    	[ uncertain_correlation_matrix REALLIST ]
    	[ continuous_interval_uncertain ALIAS interval_uncertain INTEGER > 0
    	  [ num_intervals ALIAS iuv_num_intervals INTEGERLIST ]
    	  [ interval_probabilities ALIAS interval_probs ALIAS iuv_interval_probs REALLIST ]
    	  lower_bounds REALLIST
    	  upper_bounds REALLIST
    	  [ initial_point REALLIST ]
    	  [ descriptors ALIAS iuv_descriptors STRINGLIST ]
    	  ]
    	[ discrete_interval_uncertain INTEGER > 0
    	  [ num_intervals INTEGERLIST ]
    	  [ interval_probabilities ALIAS interval_probs ALIAS range_probabilities ALIAS range_probs REALLIST ]
    	  lower_bounds INTEGERLIST
    	  upper_bounds INTEGERLIST
    	  [ initial_point INTEGERLIST ]
    	  [ descriptors STRINGLIST ]
    	  ]
    	[ discrete_uncertain_set
    	  [ integer INTEGER > 0
    	    [ elements_per_variable ALIAS num_set_values INTEGERLIST ]
    	    elements ALIAS set_values INTEGERLIST
    	    [ set_probabilities ALIAS set_probs REALLIST ]
    	    [ categorical STRINGLIST ]
    	    [ initial_point INTEGERLIST ]
    	    [ descriptors STRINGLIST ]
    	    ]
    	  [ string INTEGER > 0
    	    [ elements_per_variable ALIAS num_set_values INTEGERLIST ]
    	    elements ALIAS set_values STRINGLIST
    	    [ set_probabilities ALIAS set_probs REALLIST ]
    	    [ initial_point STRINGLIST ]
    	    [ descriptors STRINGLIST ]
    	    ]
    	  [ real INTEGER > 0
    	    [ elements_per_variable ALIAS num_set_values INTEGERLIST ]
    	    elements ALIAS set_values REALLIST
    	    [ set_probabilities ALIAS set_probs REALLIST ]
    	    [ categorical STRINGLIST ]
    	    [ initial_point REALLIST ]
    	    [ descriptors STRINGLIST ]
    	    ]
    	  ]
    	[ continuous_state INTEGER > 0
    	  [ initial_state ALIAS csv_initial_state REALLIST ]
    	  [ lower_bounds ALIAS csv_lower_bounds REALLIST ]
    	  [ upper_bounds ALIAS csv_upper_bounds REALLIST ]
    	  [ descriptors ALIAS csv_descriptors STRINGLIST ]
    	  ]
    	[ discrete_state_range INTEGER > 0
    	  [ initial_state ALIAS dsv_initial_state INTEGERLIST ]
    	  [ lower_bounds ALIAS dsv_lower_bounds INTEGERLIST ]
    	  [ upper_bounds ALIAS dsv_upper_bounds INTEGERLIST ]
    	  [ descriptors ALIAS dsv_descriptors STRINGLIST ]
    	  ]
    	[ discrete_state_set
    	  [ integer INTEGER > 0
    	    [ elements_per_variable ALIAS num_set_values INTEGERLIST ]
    	    elements ALIAS set_values INTEGERLIST
    	    [ categorical STRINGLIST ]
    	    [ initial_state INTEGERLIST ]
    	    [ descriptors STRINGLIST ]
    	    ]
    	  [ string INTEGER > 0
    	    [ elements_per_variable ALIAS num_set_values INTEGERLIST ]
    	    elements ALIAS set_values STRINGLIST
    	    [ initial_state STRINGLIST ]
    	    [ descriptors STRINGLIST ]
    	    ]
    	  [ real INTEGER > 0
    	    [ elements_per_variable ALIAS num_set_values INTEGERLIST ]
    	    elements ALIAS set_values REALLIST
    	    [ categorical STRINGLIST ]
    	    [ initial_state REALLIST ]
    	    [ descriptors STRINGLIST ]
    	    ]
    	  ]
    	[ linear_inequality_constraint_matrix REALLIST ]
    	[ linear_inequality_lower_bounds REALLIST ]
    	[ linear_inequality_upper_bounds REALLIST ]
    	[ linear_inequality_scale_types STRINGLIST ]
    	[ linear_inequality_scales REALLIST ]
    	[ linear_equality_constraint_matrix REALLIST ]
    	[ linear_equality_targets REALLIST ]
    	[ linear_equality_scale_types STRINGLIST ]
    	[ linear_equality_scales REALLIST ]
    
    KEYWORD interface
    	[ id_interface STRING ]
    	[ analysis_drivers STRINGLIST
    	  [ input_filter STRING ]
    	  [ output_filter STRING ]
    	  ( system
    	    [ parameters_file STRING ]
    	    [ results_file STRING ]
    	    [ parameters_format
    	      standard
    	      | aprepro
    	      | json
    	      ]
    	    [ results_format
    	      ( standard
    		[ labeled ]
    		)
    	      | json
    	      ]
    	    [ file_tag ]
    	    [ file_save ]
    	    [ work_directory
    	      [ named STRING ]
    	      [ directory_tag ALIAS dir_tag ]
    	      [ directory_save ALIAS dir_save ]
    	      [ link_files STRINGLIST ]
    	      [ copy_files STRINGLIST ]
    	      [ replace ]
    	      ]
    	    [ allow_existing_results ]
    	    [ verbatim ]
    	    )
    	  |
    	  ( fork
    	    [ parameters_file STRING ]
    	    [ results_file STRING ]
    	    [ parameters_format
    	      standard
    	      | aprepro
    	      | json
    	      ]
    	    [ results_format
    	      ( standard
    		[ labeled ]
    		)
    	      | json
    	      ]
    	    [ file_tag ]
    	    [ file_save ]
    	    [ work_directory
    	      [ named STRING ]
    	      [ directory_tag ALIAS dir_tag ]
    	      [ directory_save ALIAS dir_save ]
    	      [ link_files STRINGLIST ]
    	      [ copy_files STRINGLIST ]
    	      [ replace ]
    	      ]
    	    [ allow_existing_results ]
    	    [ verbatim ]
    	    )
    	  |
    	  ( direct
    	    [ processors_per_analysis INTEGER > 0 ]
    	    )
    	  |
    	  ( plugin
    	    library_path STRING
    	    )
    	  | matlab
    	  |
    	  ( python
    	    [ numpy ]
    	    )
    	  | scilab
    	  | grid
    	  [ analysis_components STRINGLIST ]
    	  ]
    	[ algebraic_mappings STRING ]
    	[ failure_capture
    	  abort
    	  | retry INTEGER
    	  | recover REALLIST
    	  | continuation
    	  ]
    	[ deactivate
    	  [ active_set_vector ]
    	  [ evaluation_cache ]
    	  [ strict_cache_equality
    	    [ cache_tolerance REAL ]
    	    ]
    	  [ restart_file ]
    	  ]
    	[ ( batch
    	    [ size INTEGER > 0 ]
    	    )
    	|
    	( asynchronous
    	  [ evaluation_concurrency INTEGER > 0 ]
    	  [ local_evaluation_scheduling
    	    dynamic
    	    | static
    	    ]
    	  [ analysis_concurrency INTEGER > 0 ]
    	  ]
    	[ evaluation_servers INTEGER > 0 ]
    	[ evaluation_scheduling
    	  dedicated
    	  |
    	  ( peer
    	    dynamic
    	    | static
    	    )
    	  ]
    	[ processors_per_evaluation INTEGER > 0 ]
    	[ analysis_servers INTEGER > 0 ]
    	[ analysis_scheduling
    	  dedicated
    	  | peer
    	  ]
    
    KEYWORD responses
    	[ id_responses STRING ]
    	[ descriptors ALIAS response_descriptors STRINGLIST ]
    	( objective_functions ALIAS num_objective_functions INTEGER >= 0
    	  [ sense STRINGLIST ]
    	  [ primary_scale_types ALIAS objective_function_scale_types STRINGLIST ]
    	  [ primary_scales ALIAS objective_function_scales REALLIST ]
    	  [ weights ALIAS multi_objective_weights REALLIST ]
    	  [ nonlinear_inequality_constraints ALIAS num_nonlinear_inequality_constraints INTEGER >= 0
    	    [ lower_bounds ALIAS nonlinear_inequality_lower_bounds REALLIST ]
    	    [ upper_bounds ALIAS nonlinear_inequality_upper_bounds REALLIST ]
    	    [ scale_types ALIAS nonlinear_inequality_scale_types STRINGLIST ]
    	    [ scales ALIAS nonlinear_inequality_scales REALLIST ]
    	    ]
    	  [ nonlinear_equality_constraints ALIAS num_nonlinear_equality_constraints INTEGER >= 0
    	    [ targets ALIAS nonlinear_equality_targets REALLIST ]
    	    [ scale_types ALIAS nonlinear_equality_scale_types STRINGLIST ]
    	    [ scales ALIAS nonlinear_equality_scales REALLIST ]
    	    ]
    	  [ scalar_objectives ALIAS num_scalar_objectives INTEGER >= 0 ]
    	  [ field_objectives ALIAS num_field_objectives INTEGER >= 0
    	    lengths INTEGERLIST
    	    [ num_coordinates_per_field INTEGERLIST ]
    	    [ read_field_coordinates ]
    	    ]
    	  )
    	|
    	( calibration_terms ALIAS least_squares_terms ALIAS num_least_squares_terms INTEGER >= 0
    	  [ scalar_calibration_terms INTEGER >= 0 ]
    	  [ field_calibration_terms INTEGER >= 0
    	    lengths INTEGERLIST
    	    [ num_coordinates_per_field INTEGERLIST ]
    	    [ read_field_coordinates ]
    	    ]
    	  [ primary_scales ALIAS calibration_term_scales ALIAS least_squares_term_scales REALLIST ]
    	  [ weights ALIAS calibration_weights ALIAS least_squares_weights REALLIST ]
    	  [ ( calibration_data
    	      [ data_directory STRING ]
    	      [ num_experiments INTEGER >= 0 ]
    	      [ num_config_variables INTEGER >= 0 ]
    	      [ experiment_variance_type ALIAS variance_type STRINGLIST ]
    	      [ scalar_data_file STRING
    		[ ( custom_annotated
    		    [ header ]
    		    [ exp_id ]
    		    )
    		| annotated
    		| freeform ]
    		]
    	      [ interpolate ]
    	      )
    	  |
    	  ( calibration_data_file ALIAS least_squares_data_file STRING
    	    [ ( custom_annotated
    		[ header ]
    		[ exp_id ]
    		)
    	    | annotated
    	    | freeform ]
    	    [ num_experiments INTEGER >= 0 ]
    	    [ num_config_variables INTEGER >= 0 ]
    	    [ experiment_variance_type ALIAS variance_type STRINGLIST ]
    	    ]
    	  [ simulation_variance REALLIST ]
    	  [ nonlinear_inequality_constraints ALIAS num_nonlinear_inequality_constraints INTEGER >= 0
    	    [ lower_bounds ALIAS nonlinear_inequality_lower_bounds REALLIST ]
    	    [ upper_bounds ALIAS nonlinear_inequality_upper_bounds REALLIST ]
    	    [ scale_types ALIAS nonlinear_inequality_scale_types STRINGLIST ]
    	    [ scales ALIAS nonlinear_inequality_scales REALLIST ]
    	    ]
    	  [ nonlinear_equality_constraints ALIAS num_nonlinear_equality_constraints INTEGER >= 0
    	    [ targets ALIAS nonlinear_equality_targets REALLIST ]
    	    [ scale_types ALIAS nonlinear_equality_scale_types STRINGLIST ]
    	    [ scales ALIAS nonlinear_equality_scales REALLIST ]
    	    ]
    	  )
    	|
    	( response_functions ALIAS num_response_functions INTEGER >= 0
    	  [ scalar_responses ALIAS num_scalar_responses INTEGER >= 0 ]
    	  [ field_responses ALIAS num_field_responses INTEGER >= 0
    	    lengths INTEGERLIST
    	    [ num_coordinates_per_field INTEGERLIST ]
    	    [ read_field_coordinates ]
    	    ]
    	  )
    	no_gradients
    	| analytic_gradients
    	|
    	( mixed_gradients
    	  id_numerical_gradients INTEGERLIST
    	  id_analytic_gradients INTEGERLIST
    	  [ method_source ]
    	  [ ( dakota
    	      [ ignore_bounds ]
    	      [ relative
    	      | absolute
    	      | bounds ]
    	      )
    	  | vendor ]
    	  [ interval_type ]
    	  [ forward
    	  | central ]
    	  [ fd_step_size ALIAS fd_gradient_step_size REALLIST ]
    	  )
    	|
    	( numerical_gradients
    	  [ method_source ]
    	  [ ( dakota
    	      [ ignore_bounds ]
    	      [ relative
    	      | absolute
    	      | bounds ]
    	      )
    	  | vendor ]
    	  [ interval_type ]
    	  [ forward
    	  | central ]
    	  [ fd_step_size ALIAS fd_gradient_step_size REALLIST ]
    	  )
    	no_hessians
    	|
    	( numerical_hessians
    	  [ fd_step_size ALIAS fd_hessian_step_size REALLIST ]
    	  [ relative
    	  | absolute
    	  | bounds ]
    	  [ forward
    	  | central ]
    	  )
    	|
    	( quasi_hessians
    	  ( bfgs
    	    [ damped ]
    	    )
    	  | sr1
    	  )
    	| analytic_hessians
    	|
    	( mixed_hessians
    	  [ id_numerical_hessians INTEGERLIST
    	    [ fd_step_size ALIAS fd_hessian_step_size REALLIST ]
    	    ]
    	  [ relative
    	  | absolute
    	  | bounds ]
    	  [ forward
    	  | central ]
    	  [ id_quasi_hessians INTEGERLIST
    	    ( bfgs
    	      [ damped ]
    	      )
    	    | sr1
    	    ]
    	  [ id_analytic_hessians INTEGERLIST ]
    	  )
    	[ metadata STRINGLIST ]
