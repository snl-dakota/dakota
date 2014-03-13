#include <stdio.h>

static char* Driver_script=0;
static char* Params_file=0;
static char* Results_file=0;

int start_grid_computing(char *analysis_driver_script, 
			 char *params_file, 
			 char *results_file)
{
Driver_script = analysis_driver_script;
Params_file = params_file;
Results_file = results_file;
return 0;
}


int perform_analysis(char *iteration_num)
{
}


int* get_jobs_completed()
{
return 0;
}


int stop_grid_computing()
{
return 0;
}
