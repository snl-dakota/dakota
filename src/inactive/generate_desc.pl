#!/usr/bin/perl                                                                                                                              

# Generate metadata file dakota.input.desc for use in DAKOTA JAGUAR                                                                          
# GUI, including:                                                                                                                            
#                                                                                                                                            
#   *. Generate empty tags file with nidrgen                                                                                                 
#   *. Process Doxygen reference manual for meta data (friendly names / links)                                                               
#   *. Group methods, variables, etc., into user-friendly groups                                                                             

# TODO: Ultimately we'll want to add more GUI-friendly information                                                                           
# into the .dox, including shorter "friendly names"                                                                                          
$GLOBAL_VERBOSE=0; # set to 1 for true ; controls some screen prints and heavy logfile use

# Process command-line options (all optional)                                                                                                
use File::Copy;
use Getopt::Long;

my $desc_tags_name = 'dakota.tags.desc';
my $desc_output_name = 'dakota.input.desc';
my $log_name = 'generate_gui_data.log';

GetOptions ('output:s' => \$desc_output_name,
            'tags:s' => \$desc_tags_name,
            'log:s' => \$log_name);

print "INFO: Using intermediate tags file $desc_tags_name, output ",
    "$desc_output_name,\n      and log file $log_name\n";

#generate_templates();                                                                                                                       

# Tags generated separately when working in src                                                                                              
#generate_tags_file($desc_tags_name);                                                                                                        

# Open log file and input .desc file                                                                                                         
open (LOG_FILE, ">$log_name") ||
    die "ERROR: Cannot open $log_name\n$!";

# read whole template dakota.tags.desc file into one scalar $desc                                                                            
# WARNING: being a bit sloppy here with global variable for the modified input                                                               
open (DESC_INPUT, "$desc_tags_name") ||
    die "ERROR: Cannot open $desc_tags_name\n$!";
my $holdTerminator = $/;
undef $/;
$desc = <DESC_INPUT>;
$/ = $holdTerminator;
close(DESC_INPUT);
process_tags_dox();

gui_groups();

gui_defaults();

# write out modified files                                                                                                                   
open (DESC_OUTPUT, ">$desc_output_name") ||
    die "ERROR: Cannot open $desc_output_name\n$!";
print DESC_OUTPUT $desc;
close(DESC_OUTPUT);

close(LOG_FILE);

if ($GLOBAL_VERBOSE == 0) { # delete log and input files
unlink $desc_tags_name;                                                                                                                     
unlink $log_name;                                                                                                                           
}

if ($GLOBAL_VERBOSE) {
    print "INFO: You most likely now want to copy $desc_output_name to \n",
    "      Dakota/src/dakota.input.desc and remake in Dakota/packages/nidr.\n";
}

exit(0);
# ----------------                                                                                                                           
# HELPER FUNCTIONS                                                                                                                           
# ----------------                                                                                                                           

# Add a DESC (optionally match GROUP; will break if kw has a desc from the dox)                                                              
sub kw_add_desc {
##    $desc =~ s/("$_[0]")/$1\n\tDESC "$_[1]"\n/;                                                                                            
    $desc =~ s/("$_[0]" GROUP "[\w\s:,-\[\]]+"|"$_[0]")/$1\n\tDESC "$_[1]"\n/;
}

sub kw_add_desc_global {
##    $desc =~ s/("$_[0]")/$1\n\tDESC "$_[1]"\n/;                                                                                            
    $desc =~ s/("$_[0]" GROUP "[\w\s:,-\[\]]+"|"$_[0]")/$1\n\tDESC "$_[1]"\n/g;
}

# TODO: kw_add_group                                                                                                                         

# Augment existing GROUP information                                                                                                         
sub kw_aug_group {
    $desc =~ s/("$_[0]" GROUP ")/$1$_[1]/;D                                                                                                  
} #"


# Augment existing DESC information (optionally match GROUP info)                                                                            
sub kw_aug_desc {
##    $desc =~ s/("$_[0]"\n\tDESC ")/$1$_[1]/;                                                                                               
    $desc =~ s/("$_[0]" GROUP "[\w\s:,-\[\]]+"\n\tDESC "|"$_[0]"\n\tDESC ")/$1$_[1]/;
} #"                                                                                                                                         

# Augment existing DESC information (optionally match GROUP info) by inserting into                                                          
# existing friendly name {...}                                                                                                               
sub kw_nest_desc {
    $desc =~ s/("$_[0]" GROUP "[\w\s:,-\[\]]+"\n\tDESC "{|"$_[0]"\n\tDESC "{)/$1$_[1]/;
} #"                                                                                                                                         


# Generate the empty tags file                                                                                                               
sub generate_tags_file {

    my $tagfile = shift;

    my @args = ("./nidrgen", "-T", $tagfile, "../../src/dakota.input.nspec");

    system(@args) == 0
        or die "ERROR: System command @args failed: $?";

    if ($GLOBAL_VERBOSE) {
        print "INFO: Generated $tagfile\n\n";
    }
    }

# Process NIDR-generated tags file and Doxygen reference manual into                                                                         
# intermediate meta data for the GUI                                                                                                         
#                                                                                                                                            
# TODO: use more localized anchors to the tables, where possible                                                                             
sub process_tags_dox {

    if ($GLOBAL_VERBOSE) {
        print "INFO: Processing reference manual files.\n";
    }

    # Initialize variables                                                                                                                   
    my $pageroot;
    my $category;
    my $lower_cat;
    my $upper_cat; # needed in double-sub prevention
    my $sub_cat; # dox comment speicified submatch
    my $subsub_cat; # unused dox comment speicified submatch
    my $subsubsub_cat; # unused dox comment speicified submatch
    my $file;
    my $anchor;
    my $keyword;
    my $longname;
    my $filecnt;
    my $total = 0;
    #my $MAX_LENGTH = 30; # maximum length of a long name before WARN
    my $MAX_LENGTH = 1000; # maximum length of a long name before WARN
    # Iterate over Reference Manual files
    ##my @ref_files = <../../docs/Ref_*.dox>;
    my @categories = ("Environment", "Method", "Model", "Variables", 
		      "Interface", "Responses");

    foreach $category (@categories) {

        $lower_cat = lc($category);
        $upper_cat = uc($category);
        $sub_cat = "";
        $subsub_cat = "";
        $subsubsub_cat = "";
        $file = "../docs/oldref/Ref_$category.dox";
        open (REF_FILE, "$file") ||
            die "ERROR: Cannot open file $file\n$!";

        if ($GLOBAL_VERBOSE) {
            print "INFO: Parsing $file\n";
        }
        $filecnt = 0;
        $desccnt=0;
        my $dname="";
        # heavy debugging solution                                                                                                           
	my $olddesc=$desc;
	$dname=sprintf("z.%d",$desccnt);
        if ($GLOBAL_VERBOSE) {
		$desccnt++;
		open D, "> $dname" or die "Can't open $dname : $!:";
		print D $desc;
		close D;
	}

        while (<REF_FILE>) {

            if (/\\page ([\w]+) /) {
                $pageroot = "$1.html#";
                if ($GLOBAL_VERBOSE) {
                    print "INFO: Using root $pageroot\n";
                }
            }

            # could probably do these two matches more elegantly in one line                                                                 
            if (/\\section ([\w]+) /) {
                $anchor = $1;
		$sub_cat="";
		$subsub_cat="";
		$subsubsub_cat="";
                print LOG_FILE "***1 section Using anchor $anchor\n";
            }
            elsif (/\\subsection ([\w]+) /) {
		$sub_cat="";
                $anchor = $1;
                print LOG_FILE "***2 subsection Using anchor $anchor\n";
            }elsif (/\\subsubsection ([\w]+) /) {
                $anchor = $1;
                print LOG_FILE "***3 subsubsection Using anchor $anchor\n";
            }elsif (/<!-- dakota subcat ([\w]+) /) {
		# pick out from dakota comments things than cannot be magically derived.
                $sub_cat = $1;
                print LOG_FILE "***4 dakota subcat $sub_cat\n";
            }

            # need to enforce the following structure (sawtr, longname, keyword)                                                             
            # <tr>[\s]*\n                                                                                                                    
            # <td>Long Name\n   OR  <td>%Long Name\n  (TODO: Long Name can contain \c /)                                                     
            # <td>\c keyword[\s]\n                                                                                                           
            #print LOG_FILE "line is $_\n";                                                                                                  

            # MATCH 3                                                                                                                        
            # keyword is <td>\c keyword                                                                                                      
            # have to guard against options marked with \c                                                                                   
            # as in:                                                                                                                         
            #<td>Optimization type                                                                                                           
            #<td>\c optimization_type                                                                                                        
            #<td>\c minimize | \c maximize                                                                                                   
            #<td>Optional group                                                                                                              
            #<td>\c minimize                                                                                                                 
            # alternately clear long name after it's used?                                                                                   
            if ($longname && /<td>\\c ([\w]+)[\s]*\n/) {
                $keyword = $1;
                #print LOG_FILE "Keyword is $1 whole match is $_\n";
                # for now, save a record of the metadata                                                                                     
                #print LOG_FILE "$keyword,{$longname} $pageroot$anchor\n";
                $filecnt++;
                # replace in tags file, marking it REMOVEME so multiple replacements don't happen                                            
                ##$desc =~ s/\/$keyword"/\/REMOVEME$keyword"\n\tDESC "{$longname} $pageroot$anchor"\n/;                                      
                # REQUIRE matching the category                                                                                              
                #$desc =~ s/("$lower_cat\/[\w\/]*\/)$keyword"/${1}${keyword}REMOVEME"\n\tDESC "{$longname} $pageroot$anchor"\n/;             

		# no double replacements; must prevent lower_cat from matching twice, not just keyword. added leading remove and then convert to uppercase. downcase at end of input.
		if ($sub_cat ne "") {
			# must also prevent underqualified matches of sub_cat in those distinct subsections which share keywords
			# any pair of conflicting sections should both mark their subsections with subcat comments in input dox
                	$desc =~ s/("$lower_cat\/$sub_cat[\w\/]*)\/$keyword"/REMOVEME${1}\/${keyword}REMOVEME"\n\tDESC "{$longname} $pageroot$anchor"\n/;              
		} else {
                	$desc =~ s/("$lower_cat[\w\/]*)\/$keyword"/REMOVEME${1}\/${keyword}REMOVEME"\n\tDESC "{$longname} $pageroot$anchor"\n/;              
		}
		$desc =~ s/REMOVEME"$lower_cat/"$upper_cat/;              
                                                                                                                                             
                ##$desc =~ s/("$lower_cat[\/\w]+)/$keyword"/${1}${keyword}REMOVEME"\n\tDESC "{$longname} $pageroot$anchor"\n/;

                # heavy debugging solution                                                                                                   
		if ($GLOBAL_VERBOSE == 1) {
                	$dname=sprintf("z.%d",$desccnt);
			print LOG_FILE  "dump= $dname\n";
			if ($desc ne $olddesc) {
				$olddesc = $desc;
				$desccnt++;
                		open D, "> $dname" or die "Can't open $dname : $!:";
                		print D $desc;
                		close D;
			}
		}
                if ( length($longname) > $MAX_LENGTH ) {
                    print "WARN: Length ", length($longname), ": $longname\n";
                }
            }

            # MATCH 2                                                                                                                        
            # look for <tr>\n<td> to see longnames                                                                                           
            # consider matching any character here?                                                                                          
            # match 0 or 1 % symbol before the long name (include '\', '/', '(', ')')                                                        
            if ($sawtr && /<td>%?([\w\s-\\\/\(\)]+)\n/) {
                $longname = $1;
                $longname =~ s/\\c //g;   # strip any "\c "                                                                                  
                #print LOG_FILE "  longname assigned: $longname\n"
            }
            else {
                $longname = "";
            }

            # MATCH 1                                                                                                                        
            # look for <tr>\n<td> at end of loop (assumes <tr> precedes longname)                                                            
            if (/<tr>[\s]*\n/) {
                $sawtr=1;
                #print LOG_FILE "   sawtr\n";
            }
            else {
                $sawtr=0;
            }

        }
	# down-case the upper_cat that prevented double replacements
	$desc =~ s/$upper_cat/$lower_cat/g;              
        close(REF_FILE);
        if ($GLOBAL_VERBOSE) {
            print "INFO: Processed $filecnt keywords in $file.\n";
        }
        $total += $filecnt;
    }

    if ($GLOBAL_VERBOSE) {
        print "INFO: Processed $total total keywords.\n";
    }

    # Remove any duplicate aversion tags and write out the file                                                                              
    $desc =~ s/REMOVEME//g;

    if ($GLOBAL_VERBOSE) {
        print "INFO: Reference manual file processing complete.\n\n";
    }
}


# Process NIDR-generated tags file and Doxygen reference manual into                                                                         
# intermediate meta data                                                                                                                     
# TODO: Surrogate types under model?                                                                                                         
sub gui_groups {

    if ($GLOBAL_VERBOSE) {
        print "INFO: Creating GUI groups.\n";
    }

    #                                                                                                                                        
    # Group variables                                                                                                                        
    #                                                                                                                                        
    $desc =~ s/(_design")/$1 GROUP "Design Variables"/g;                                                                                     
    $desc =~ s/(discrete_design_range")/$1 GROUP "Design Variables"/g;
    $desc =~ s/(discrete_design_set")/$1 GROUP "Design Variables"/g;                                                                 
  
    ## NOTE: lognormal and loguniform are covered by these matches                                                                           
    my @uc = ("normal", "uniform", "triangular", "exponential", "beta",
              "gamma", "gumbel", "frechet", "weibull", "histogram_bin");
    foreach (@uc) {
        $desc =~ s/(${_}_uncertain")/$1 GROUP "Continuous Aleatory Uncertain"/g;                                                             
        }                                                                                                                                    
                                                                                                                                             
    # NOTE: negative_binomial and hypergeometric are covered                                                                                 
    my @uc = ("poisson", "binomial", "geometric", "histogram_point");                                                                        
    foreach (@uc) {                                                                                                                          
        $desc =~ s/(${_}_uncertain")/$1 GROUP "Discrete Aleatory Uncertain"/g;
         }

    # force a group to appear on the variables pane                                                                                          
    $desc =~ s/(uncertain_correlation_matrix")/$1 GROUP "Aleatory Uncertain Correlations"/g;                                                 
                                                                                                                                             
    $desc =~ s/(interval_uncertain")/$1 GROUP "Epistemic Uncertain"/g;
    $desc =~ s/(uncertain_set")/$1 GROUP "Epistemic Uncertain"/g;

    $desc =~ s/(continuous_state")/$1 GROUP "State Variables"/g;                                                                             
    $desc =~ s/(discrete_state_range")/$1 GROUP "State Variables"/g;
    $desc =~ s/(discrete_state_set")/$1 GROUP "State Variables"/g;                                                                   

    #                                                                                                                                        
    # Group methods                                                                                                                          
    #                                                                                                                                        
    my @methods;

    # verification
    $desc =~ s/(richardson_extrap")/$1 GROUP "Verification Studies"/g;

    # parameter study                                                                                                                        
    $desc =~ s/(_parameter_study")/$1 GROUP "Parameter Studies"/g;                                                                           
                                                                                                                                             
    # DACE"
    @methods = ("dace", "fsu_cvt", "fsu_quasi_mc", "psuade_moat");
    foreach (@methods) {
        $desc =~ s/($_")/$1 GROUP "DACE"/g;                                                                                                  
        }                                                                                                                                    
                                                                                                                                             
    # NLLS"
    @methods = ("nl2sol", "nlssol_sqp", "nond_bayes_calib", "optpp_g_newton");
    foreach (@methods) {
        $desc =~ s/($_")/$1 GROUP "Calibration"/g;                                                                               
        }                                                                                                                                    
    #                                                                                                                                        
    # Optimization                                                                                                                           
    #                                                                                                                                        
    # Local, Derivative-based                                                                                                                
    # conmin frcg mfd                                                                                                                        
    # dot bfgs frcg mmfd slp sqp                                                                                                             
    # nlpql sqp                                                                                                                              
    # npsol sqp                                                                                                                              
    # optpp cg newton q_newton fd_newton                                                                                                     
                                                                                                                                             
    $desc =~ s/(method\/conmin_[a-z_]*")/$1 GROUP "Optimization: Local, Derivative-based"/g;
    $desc =~ s/(\/dot_[a-z_]*")/$1 GROUP "Optimization: Local, Derivative-based"/g;                                                          
    $desc =~ s/(method\/nlpql_sqp")/$1 GROUP "Optimization: Local, Derivative-based"/g;
    $desc =~ s/(method\/nonlinear_cg")/$1 GROUP "Optimization: Local, Derivative-based"/g;                                                   
    $desc =~ s/(method\/npsol_sqp")/$1 GROUP "Optimization: Local, Derivative-based"/g;
    $desc =~ s/(method\/optpp_[cfnq][a-z_]*")/$1 GROUP "Optimization: Local, Derivative-based"/g;                                            
                                                                                                                                             
    @methods = ("asynch_pattern_search", "coliny_cobyla", "coliny_pattern_search", "coliny_solis_wets", "optpp_pds");                        
    foreach (@methods) {                                                                                                                     
        $desc =~ s/(method\/$_")/$1 GROUP "Optimization: Local, Derivative-free"/g;
        }

           
    @methods = ("ncsu_direct", "coliny_ea", "coliny_direct", "soga", "moga");
    foreach (@methods) {
        $desc =~ s/(method\/$_")/$1 GROUP "Optimization: Global"/g;                                                                          
        }                                                                                                                                    

    @methods = ("dl_solver", "coliny_beta");
    foreach (@methods) {
        $desc =~ s/(method\/$_")/$1 GROUP "Optimization: Other"/g;                                                                          
        }                                                                                                                                    

    # Surrogate-based methods"                                                                                                               
    @methods = ("surrogate_based_local", "surrogate_based_global", "efficient_global");
    foreach (@methods) {
        $desc =~ s/($_")/$1 GROUP "Surrogate-based Methods"/g;                                                                               
    }                                                                                                                                        
                                                                                                                                             
    # UQ methods (omit bayes_cal)"
    @methods = ("global_evidence", "global_interval_est", "global_reliability", "importance_sampling", "local_evidence", "local_interval_est", "local_reliability", "polynomial_chaos", "sampling", "stoch_collocation", "gpais", "adaptive_sampling", "efficient_subspace" );
    foreach (@methods) {
	$desc =~ s/(method\/$_")/$1 GROUP "Uncertainty Quantification"/g;                                                                    
    }                                                                                                                                     
                                                                                                                                             
    # Add response data set groups                                                                                                           
    kw_nest_desc("responses/objective_functions", "{Optimization} ");                                                                    
    kw_nest_desc("responses/calibration_terms", "{Calibration (Least squares)} ");                                                     
    kw_nest_desc("responses/response_functions", "{Generic responses} ");                                                                
                                            
    # Add top-level keyword help text"
    $url = "EnvCommands.html";
    kw_add_desc("environment", "{Environment} The environment specifies the top level technique which will govern the management of iterators and models in the solution of the problem of interest. $url");

    $url = "MethodCommands.html";
    kw_add_desc("method", "{Method} A method specifies the name and controls of an iterative procedure, e.g., a sensitivity analysis, uncertainty quantification, or optimization method. $url");

    $url = "ModelCommands.html";
    kw_add_desc("model", "{Model} A model consists of a model type and maps specified variables through an interface to generate responses. $url");

    $url = "VarCommands.html";
    kw_add_desc("variables", "{Variables} A variables object specifies the parameter set to be iterated by a particular method. $url");

    $url = "InterfCommands.html";
    kw_add_desc("interface", "{Interface} An interface specifies how function evaluations will be performed in order to map a set of parameters into a set of responses. $url");

    $url = "RespCommands.html";
    kw_add_desc("responses", "{Responses} A responses object specifies the data that can be returned to DAKOTA through the interface after the completion of a function evaluation. $url");

    if ($GLOBAL_VERBOSE) {
        print "INFO: GUI groups complete.\n\n";
    }
}


# Defaults and "CHOOSE from the list" help text                                                                                              
# [...] indicates header text                                                                                                                
# @ indicates default selection                                                                                                              
# TODO: revisit delimiters                                                                                                                   
sub gui_defaults {

    # TODO: delimiters for choose text (left and right)                                                                                      
    my $L="\[";
    my $R="\]";
    # character in DESC indicating default                                                                                                   
    my $DEFAULT = "@";

    # -- MODEL: DONE --                                                                                                                      

    # Use model type single by default                                                                                                       
    kw_add_desc("model/nested", "[CHOOSE model type]");
    kw_add_desc("model/single", "$DEFAULT");

    # Require user to select surrogate category                                                                                              
    kw_aug_desc("model/surrogate/global", "[CHOOSE surrogate category]");

    # Select global sub-type                                                                                                                 
    kw_aug_desc("model/surrogate/global/gaussian_process",
                "[CHOOSE surrogate type]");

    kw_add_desc("model/surrogate/global/gaussian_process/trend/constant", "[CHOOSE trend type]");
    kw_add_desc("model/surrogate/global/gaussian_process/trend/quadratic", "$DEFAULT");

    kw_add_desc("model/surrogate/global/polynomial/cubic", "[CHOOSE polynomial order]");

    kw_add_desc("model/surrogate/global/reuse_samples/all", "[CHOOSE reuse scope]");

    kw_add_desc("model/surrogate/global/correction/additive", "[CHOOSE correction type]");
    kw_add_desc("model/surrogate/global/correction/first_order", "[CHOOSE correction order]");

    kw_add_desc("model/surrogate/hierarchical/correction/additive", "[CHOOSE correction type]");
    kw_add_desc("model/surrogate/hierarchical/correction/first_order", "[CHOOSE correction order]");



    # -- VARIABLES: DONE --                                                                                                                  

    # Default to std_deviations for lognormal                                                                                                
    kw_aug_desc("variables/lognormal_uncertain/lambdas", "[CHOOSE characterization]");
    kw_aug_desc("variables/lognormal_uncertain/means", "$DEFAULT");

    # Default to std_deviations for lognormal                                                                                                
    kw_aug_desc("variables/lognormal_uncertain/means/error_factors", "[CHOOSE variance spec.]");
    kw_aug_desc("variables/lognormal_uncertain/means/std_deviations", "$DEFAULT");


    # -- INTERFACE: DONE --                                                                                                                  

    # direct/fork/system/grid (default to fork)                                                                                              
    kw_aug_desc("interface/analysis_drivers/direct", "[CHOOSE interface type]");
    kw_aug_desc("interface/analysis_drivers/fork", "$DEFAULT");

    kw_add_desc("interface/analysis_drivers/failure_capture/abort", "${DEFAULT}[CHOOSE failure mitigation]");

    # evaluation self/static                                                                                                                 
    kw_aug_desc("interface/evaluation_self_scheduling",
                "[CHOOSE evaluation sched.]");

    # analysis self/static                                                                                                                   
    kw_aug_desc("interface/analysis_self_scheduling", "[CHOOSE analysis sched.]");


    # -- RESPONSES: 1 TODO --                                                                                                                

    # Require use to select response type                                                                                                    
    kw_aug_desc("responses/num_least_squares_terms", "[CHOOSE response type]");

    # gradients                                                                                                                              
    kw_add_desc("responses/analytic_gradients", "[CHOOSE gradient type]");
    kw_add_desc("responses/no_gradients", "$DEFAULT");

    # default to dakota as gradient source                                                                                                   
    kw_aug_desc("responses/mixed_gradients/dakota", "${DEFAULT}[CHOOSE gradient source]");
    kw_add_desc("responses/mixed_gradients/central", "[CHOOSE difference interval]");
    kw_add_desc("responses/mixed_gradients/forward", "$DEFAULT");

    # TODO: error in TAGs: no numerical gradient specs present?!?                                                                            
    ##kw_add_desc("responses/numerical_gradients/dakota", "${DEFAULT}[CHOOSE gradient source]");                                             
    ##kw_add_desc("responses/numerical_gradients/central", "[CHOOSE difference interval]");                                                  
    ##kw_add_desc("responses/numerical_gradients/forward", "$DEFAULT");                                                                      

    # Hessians                                                                                                                               
    kw_add_desc("responses/analytic_hessians", "[CHOOSE Hessian type]");
    kw_add_desc("responses/no_hessians", "$DEFAULT");

    kw_add_desc("responses/mixed_hessians/central", "[CHOOSE difference interval]");
    kw_add_desc("responses/mixed_hessians/forward", "$DEFAULT");
    kw_add_desc("responses/mixed_hessians/id_quasi_hessians/bfgs", "[CHOOSE Hessian approx.]");

    kw_add_desc("responses/numerical_hessians/central", "[CHOOSE difference interval]");
    kw_add_desc("responses/numerical_hessians/forward", "$DEFAULT");
    kw_add_desc("responses/quasi_hessians/bfgs", "[CHOOSE Hessian approx.]");



    # -- ENVIRONMENT:DONE --                                                                                                                    

    #kw_aug_desc("environment/hybrid", "[CHOOSE environment type]");

    # If the user selected hybrid, require them to specify the hybrid type                                                                 
    #kw_aug_desc("environment/hybrid/collaborative", "[CHOOSE hybrid type]");


    # -- METHOD --                                                                                                                           
    # TODO: COLINY, JEGA                                                                                                                     

    # Require the user to specify method category                                                                                            
    kw_aug_group("method/dace", "[CHOOSE method category]");

    # Require the user to specify sub-method for each method category                                                                        
    kw_aug_desc("method/dace", "[CHOOSE DACE method]");
    kw_add_desc("method/nl2sol", "[CHOOSE LSQ method]");

    kw_aug_desc("method/centered_parameter_study",
                      "[CHOOSE PSTUDY method]");
    kw_aug_desc("method/nond_global_evidence", "[CHOOSE UQ method]");
    kw_aug_desc("method/efficient_global", "[CHOOSE SB method]");

    # local, derivative-free                                                                                                                 
    kw_aug_desc("method/asynch_pattern_search", "[CHOOSE OPT method]");


    # local, gradient                                                                                                                                               
    kw_add_desc("method/conmin_frcg", "[CHOOSE OPT method]");

    # global                                                                                                                                                        
    kw_aug_desc("method/coliny_direct", "[CHOOSE OPT method]");


    kw_add_desc("method/output/debug", "[CHOOSE output level]");

    # surrogate-based local                                                                                                                                         
    kw_aug_desc("method/surrogate_based_local/approx_method_name", "[CHOOSE sub-method ref.]");

    kw_add_desc("method/surrogate_based_local/approx_subproblem/augmented_lagrangian_objective", "[CHOOSE objective formulation]");
    kw_add_desc("method/surrogate_based_local/approx_subproblem/original_primary", "$DEFAULT");

    kw_add_desc("method/surrogate_based_local/approx_subproblem/linearized_constraints", "[CHOOSE constraint formulation]");
    kw_add_desc("method/surrogate_based_local/approx_subproblem/original_constraints", "$DEFAULT");

    kw_add_desc("method/surrogate_based_local/merit_function/adaptive_penalty_merit", "[CHOOSE merit function]");
    kw_add_desc("method/surrogate_based_local/merit_function/augmented_lagrangian_merit", "$DEFAULT");

    kw_add_desc("method/surrogate_based_local/acceptance_logic/filter", "${DEFAULT}[CHOOSE acceptance logic]");


    # DOT minimize                                                                                                                                                  
    kw_add_desc("method/dot_frcg/optimization_type/maximize", "[CHOOSE optimization sense]");
    kw_add_desc("method/dot_frcg/optimization_type/minimize", "$DEFAULT");

    # OPT++                                                                                                                                                         
    kw_add_desc("method/optpp_q_newton/search_method/gradient_based_line_search", "[CHOOSE line search type]");

    # APPS                                                                                                                                                          
    kw_add_desc("method/asynch_pattern_search/synchronization/blocking", "[CHOOSE synchronization]");
    kw_add_desc("method/asynch_pattern_search/synchronization/nonblocking", "$DEFAULT");
    kw_add_desc("method/asynch_pattern_search/merit_function/merit1", "[CHOOSE merit function]");
    kw_add_desc("method/asynch_pattern_search/merit_function/merit2_smooth", "$DEFAULT");

    # SB global                                                                                                                                                     
    kw_aug_desc("method/surrogate_based_global/approx_method_name", "[CHOOSE sub-method ref.]");

    # NOND*                                                                                                                                                         

    # distribution for all methods                                                                                                                                  
    kw_add_desc_global("method/nond_[a-z_]+/distribution/cumulative", "$DEFAULT");
    kw_add_desc_global("method/nond_[a-z_]+/distribution/complementary", "[CHOOSE distribution type]");

    # compute for all methods                                                                                                                                       
    kw_add_desc_global("method/nond_[a-z_]+/response_levels/compute/probabilities", "$DEFAULT");
    kw_add_desc_global("method/nond_[a-z_]+/response_levels/compute/gen_reliabilities", "[CHOOSE statistic]");


    # PCE                                                                                                                                                           
    kw_aug_desc("method/nond_polynomial_chaos/collocation_points", "[CHOOSE PC control type]");
    #kw_aug_desc("method/nond_polynomial_chaos/expansion_import_file", "[CHOOSE file characterization]");                                                           
    kw_add_desc("method/nond_polynomial_chaos/sample_type/lhs", "${DEFAULT}[CHOOSE sample type]");

    # SC                                                                                                                                                            
    kw_aug_desc("method/nond_stoch_collocation/quadrature_order", "[CHOOSE accuracy control]");
    kw_add_desc("method/nond_stoch_collocation/sample_type/lhs", "${DEFAULT}[CHOOSE sample type]");

    # SAMPLING: DONE                                                                                                                                                
    kw_add_desc("method/nond_sampling/sample_type/incremental_lhs", "[CHOOSE sample type]");
    kw_add_desc("method/nond_sampling/sample_type/lhs", "$DEFAULT");

    # local reliability                                                                                                                                             
    kw_add_desc("method/nond_local_reliability/mpp_search/no_approx", "[CHOOSE MPP search method]");
    kw_add_desc("method/nond_local_reliability/integration/first_order", "@[CHOOSE integration order]");
    kw_add_desc("method/nond_local_reliability/integration/refinement/adapt_import", "[CHOOSE refinement type]");

    # global reliability                                                                                                                                            
    kw_add_desc("method/nond_global_reliability/u_gaussian_process", "[CHOOSE approx. type]");

    # DACE: DONE                                                                                                                                                    
    kw_add_desc("method/dace/box_behnken", "[CHOOSE DACE type]");

    # FSUDACE: DONE                                                                                                                                                 
    kw_add_desc("method/fsu_cvt/trial_type/grid", "[CHOOSE trial type]");
    kw_add_desc("method/fsu_cvt/trial_type/random", "$DEFAULT");
    kw_add_desc("method/fsu_quasi_mc/halton", "[CHOOSE sequence type]");

    # PSTUDIES: DONE                                                                                                                                                
    kw_aug_desc("method/vector_parameter_study/final_point", "[CHOOSE final pt or increment]");
    kw_aug_desc("method/vector_parameter_study/final_point/step_length", "[CHOOSE termination type]");


}
sub generate_templates {

    print "INFO: Generating template files from examples/\n";

    my $outdir = ".";
    my $dest = "$outdir/templates";
    mkdir $dest;

    my $srct = "../../examples/tutorial";
    my $srcm = "../../examples/methods";

    my %templates = (
                     # tutorial files                                                                                                                               
                     "$srct/dakota_mogatest1.in"               => "Optimization_Global_MultiObjective_Genetic_Algorithm.dak",
                     "$srct/dakota_mv.in"                      => "Uncertainty_Quantification_Mean_Value.dak",
                     "$srct/dakota_pce.in"                     => "Uncertainty_Quantification_Polynomial_Chaos.dak",

                     "$srct/dakota_rosenbrock_2d.in"           => "Parameter_Study_Grid_2D.dak",
                     "$srct/dakota_rosenbrock_ea_opt.in"       => "Optimization_Global_Evolutionary_Algorithm.dak",
                     "$srct/dakota_rosenbrock_grad_opt.in"     => "Optimization_Local_Gradient_Based_CONMIN.dak",
                     "$srct/dakota_rosenbrock_ls.in"           => "Calibration_Local_NL2SOL.dak",
                     "$srct/dakota_rosenbrock_nond.in"         => "SA_UQ_Sampling_LHS.dak",
                     "$srct/dakota_rosenbrock_ps_opt.in"       => "Optimization_Local_Pattern_Search.dak",
                     "$srct/dakota_rosenbrock_syscall.in"      => "Optimization_System_Call_TextBook_Example.dak",
                     "$srct/dakota_rosenbrock_vector.in"       => "Parameter_Study_Vector.dak",
                     # "$srct/dakota_rosen_suzuki_conmin_mc.in" => MC.dak",                                                                                         
                     "$srct/dakota_textbook.in"                => "Optimization_Local_Constrained_GradientBased.dak",
                     "$srct/dakota_uq_interval.in"             => "Uncertainty_Quantification_Epistemic_Interval.dak",
                     #                           
                     # methods files                                                                                                                                
                     "$srcm/dakota_addtnl_cantilever.in"       => "Optimization_Local_Cantilever_Example.dak",
                     "$srcm/dakota_addtnl_cylhead.in"          => "Optimization_Local_CylinderHead_Example.dak",
                     "$srcm/dakota_addtnl_rosen_ls.in"         => "Calibration_Local_OPTPP.dak",
                     "$srcm/dakota_addtnl_rosen_opt.in"        => "Optimization_Local_GradientBased_OPTPP.dak",
                     "$srcm/dakota_container.in"               => "Optimization_Local_Container_Example.dak",
                     "$srcm/dakota_hybrid.in"                  => "Optimization_Hybrid_TextBook_Example.dak",
                     "$srcm/dakota_mogatest2.in"               => "Optimization_Global_MOGA_Example2.dak",
                     "$srcm/dakota_mogatest3.in"               => "Optimization_Global_MOGA_Example3.dak",
                     "$srcm/dakota_multiobj1.in"               => "Optimization_Local_MultiObjective.dak",
                     "$srcm/dakota_multistart.in"              => "Optimization_Local_MultiStart.dak",
                     "$srcm/dakota_nls_datafile.in"            => "Calibration_Local_Datafile_Example.dak",
                     "$srcm/dakota_ouu1_tb.in"                 => "Strategy_Optimization_Under_Uncertainty.dak",
                     "$srcm/dakota_pareto.in"                  => "Strategy_Pareto_Optimization.dak",
                     "$srcm/dakota_rosenbrock_ego.in"          => "Optimization_Efficient_Global.dak",
                     "$srcm/dakota_rosenbrock_scaled.in"       => "Optimization_Local_Scaling_Example.dak",
                     "$srcm/dakota_sbo_rosen.in"               => "Strategy_Surrogate_Based_Optimization.dak",
                     "$srcm/dakota_sc.in"                      => "Uncertainty_Quantification_Stochastic_Collocation.dak",
                     "$srcm/dakota_su_mogatest1.in"            => "Optimization_Surrogate_Global.dak",
                     "$srcm/dakota_surr_uq.in"                 => "Uncertainty_Quantification_Surrogate_Based.dak",
                     "$srcm/dakota_uq_cantilever_sop_rel.in"   => "Uncertainty_Quantification_Second_Order_Sampling.dak",
                     "$srcm/dakota_uq_reliability.in"          => "Uncertainty_Quantification_Local_Reliability.dak",
                     "$srcm/dakota_uq_sampling.in"             => "Uncertainty_Quantification_Sampling_LHS.dak",
                     "$srcm/dakota_uq_textbook_dste.in"        => "Uncertainty_Quantification_Interval_Evidence.dak",
                     "$srcm/least_squares_test.dat"            => "least_squares_test.dat"

                   );

    while (($key, $value) = each(%templates)) {

        copy("$key", "$dest/$value") or die "ERROR: copying $key failed.\n";
        if ($GLOBAL_VERBOSE) {
            print "INFO: File $key\n---> $dest/$value\n";
        }
    }
    print "INFO: Done generating templates.\n\n";

}



