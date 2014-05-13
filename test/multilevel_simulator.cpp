/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//******************************MULTILEVEL**************************************
// Version 1.0, Boyd Schimel, 8-29-2000
//
// This program computes relative parallel efficencies of different 
// partitioning schemes for a parallel multilevel algorithms.
//
// It is up to the user to supply values/equations for the
// alg_times class functions compute_time(), serial_time(),
// communication_time() and set_phases(). Default definitions 
// for these functions are defined as virtual functions in class 
// algorithm_times. 
//
// The program is executed with the command
//
// multilevel [-a -d -m -h -l -p -s] <input file>
//
// Parallel effenciency and other data is written to stdout.
// Command line parameters are defines as follows:
//
// Flag
//  -a    Output all possible partitioning schemes. The default
//        is to output only partitioning schemes which do not
//        have idle partitions of processors during computation.
//        Partitioning schemes which don't include all of the 
//        available processors are still output in the default 
//        case.
//  -d    Use deterministic serial, communication and computation
//        times.
//  -m    medium performance scheduling -- eliminate masters,
//        if present, from levels where the number of partitions is
//        equal to one for a given partitioning scheme 
//  -h    high performance scheduling -- medium performance scheduling
//        plus eliminate masters, if present, from levels where
//        the number of partitions is equal to the maximum concurrency 
//        for a given partitioning scheme.
//  -l    skew the schedule so that extra processors are distributed
//        to the lowest numbered partitions in a level.
//  -p    Output only the partitioning schemes that use exactly
//        the requested number of processors
//  -s    The number of processors listed in the input file are
//        used for simulations only. Any additional processors
//        needed for masters are automaticly added to the total.
//
//  Input file:
//   The input file has the following line format:
//
//   first line:    processors  algorithm_levels iterations
//   lvls 1 to k-1: master_flag  static_phases stochastic_phases max_concurrency
//   last level:    static_phases stochastic_phases pmin pmax
//
//   where:
//      processors           total number of processors available
//      algorithm_levels     total number of levels (k) of parallelism
//      iterations           number of times the algorithm is executed
//                           (for computation of statistical parameters)
//      master_flag          partitions on the kth level require (1)
//                           or do not require (0) a master
//      static_phases        minimum number of phases for the level
//      stochastic_phases    phases uniformly distributed over range 
//                           [0,stochastic_phases] are added to 
//                           static_phases for each experiment and level
//      max_concurrency      maximum number of partitions for level
//                           the level 
//      pmin                 minimum number of processors which can
//                           be used to solve a simulation
//      pmax                 maximum number of processors which can
//                           be used to solve a simulation
//   All inputs are integer.
//  
//  Output:
//   Output is written to stdout in tabular form. The table of results
//   contains one row for each partitioning scheme, where:
//
//     efficency = Relative efficiency
//     wall time = Wallclock time required for the partitioning
//     sim. time = Time for a single simulation with sim. proc.
//                 processors.
//     %usage    = Percentage use of compute processors. This number can
//                 be influenced by idle partitions of processors,
//                 or idleness due to stochasticity.
//     min proc. = Minimum number of processors necessary for
//                 a partitioning scheme.
//     sim. proc.= Number of processors used for an individual
//                 simulation
//     masters   = Number of masters used for the partitioning scheme
//     ser.sims. = Number of serial executions of the simulation code.
//     partitions(tot. level 1 2 ... ) =
//                 The first column under this heading is the total
//                 number of concurrent simulations possible. The
//                 columns that follow give the number of partitions
//                 for each level.
//
//******************************************************************************

#include <stdlib.h>
#include <iostream.h>
#include <iomanip.h>
#include <fstream.h>
#include <string.h>
#include <math.h>

#include "RNG.h"
#include "Random.h"
#include "Uniform.h"
#include "Normal.h"
#include "Exponential.h"
#include "Cauchy.h"
#include "errmsg.h"
#include "CRandVar.extern.h"
#include "default_rng.h"
#include "DUniform.h"

#define BASE_LEVEL    1
#define LIST_START   -1
#define LIST_END     -2

unsigned int Levels=0, Iterations, Pkmin, Pkmax, LevelsWithMaster=0; 
unsigned int Processors=0, SimProcessors, RemProcessors, ComputeProcessors;
unsigned int RequestedProcessors;
unsigned int AllPermutations=0, DeterministicTimes=0;
unsigned int MediumPerformance=0, HighPerformance=0;
unsigned int BalancedSchedule=1;
unsigned int ExactProcessorCount=0, SupplyMasters=0;
int Experiment=0,Seed;
double Pmin_time=0.0, Pmin_work, Usage;

PM_LCG *RNGen = &default_rng;
Exponential *Rnd = new Exponential(RNGen);
PM_LCG *DRNGen = &default_rng;
DUniform *DRnd = new DUniform(DRNGen);

class list_item {
public:
  double value;
  int prev,next;
};

class linked_list {
public:
  linked_list(unsigned int);
  ~linked_list();
  unsigned int len,start,end;
  list_item *list;
  double maximum();
  void set_maximum();
  double& operator [] (int);
  void sort_inc();
};

linked_list::linked_list(unsigned int lngth)
{
  len=lngth;
  list = new list_item[len];

  // initialize values and lengths
  for (int i=0;i<len;i++) {
    list[i].value=0.0;
    list[i].prev=i-1;
    list[i].next=i+1;
  }
  list[len-1].next=-1;
  start=0;
  end=len-1;

}
linked_list::~linked_list()
{
  delete list;
}
double linked_list::maximum()
{
  double list_max = list[0].value;

  for(int i=1;i<len;i++)
    if(list[i].value>list_max)
      list_max=list[i].value;

  return(list_max);
}

double& linked_list::operator[](int indx)
{
if(indx==LIST_START)
  return list[start].value;
else if(indx==LIST_END)
  return list[end].value;
else {
  return list[indx].value;
  }
}

void linked_list::set_maximum()
{
  //set all members to the maximum of the list
  double list_max = maximum();

  for(int i=0;i<len;i++)
    list[i].value = list_max;
    
}

void linked_list::sort_inc()
{

  // this should be replaced with a binary search for large problems

  //find position position of modified value in list
  int i = end,j;
  for (int count=len;count>0;count--)
    if (list[start].value<list[i].value) 
      i=list[i].prev;
    else
      break;

  if(i!=start) {
    // set new start (minimum) for list
    j=start; 
    if(list[start].next!=-1) {
      start=list[start].next;
      list[start].prev=-1;
    }

    //link modified value into list
    if(i!=end) {   
      list[j].prev=i;
      list[j].next=list[i].next;
      list[list[i].next].prev=j;
      list[i].next=j;
    }
    else {
      list[i].next=j;
      list[j].prev=i;
      list[j].next=-1;
      end = j;
    }
  }
}

class algorithm_times {
public:
  // the functions defined here can be overriden by supplying
  // nonvirtual replacments in class alg_times.

  virtual double communication_time();
  virtual double compute_time(unsigned int);
  virtual double serial_time();
  virtual void set_phases();

  unsigned int phases, stochastic_phases, static_phases;
};

double algorithm_times::communication_time()
{
  if(DeterministicTimes)
    return 0.0000;
  else
    return 0.0000;
}

double algorithm_times::compute_time(unsigned int prcsrs)
{
  // default: times based on 128, 256, and 512 processor Serria runs
  // compute times are not reliable estimates outside the range of 
  // 128-512.

  if(DeterministicTimes)
    return 3.0987046839e+00 - 5.2230402501e+01/(double)prcsrs
             + 2.5829136200e+01*exp(-1.7331570067e-02*(double)prcsrs);
  else {
    //exponential distribution with a mean of 1
    double r = (*Rnd)(); 
    double fn = 3.0987046839e+00 - 5.2230402501e+01/((double)prcsrs)
             + 2.5829136200e+01*exp(-1.7331570067e-02*(double)prcsrs);
      return fn*(0.9 + 0.1*r);
  }
}

double algorithm_times::serial_time()
{
  if(DeterministicTimes)
    return 0.0000;
  else
    return 0.0000;
}

void algorithm_times::set_phases()
{
//sets the number of phases for the level
  if(stochastic_phases) {
    int ir = 0;
    do {
      ir++;
    }while((*DRnd)(1,stochastic_phases)!=stochastic_phases);
    phases = static_phases + ir;
  }
  else 
    phases = static_phases;
}

class alg_times: public algorithm_times {
//  enable any of the following function definitions to override
//  the default (virtual) functions defined in class algorithm_times.
//  You must supply any functions that you enable here.

//  double communication_time();
//  double compute_time(unsigned int);
//  double serial_time();
//  void set_phases();

};

class algorithm: public alg_times {
private:
  unsigned int level, master, partitions, max_concurrency;
  unsigned int processors_in_partition; 
  algorithm *sublevel;
  algorithm *superlevel;

public:
  double dynamic_wallclock_time(unsigned int);
  unsigned int get_master() { return master; };
  unsigned int permute_partitions();
  double pmin_wallclock_time(); 
  void populate(unsigned int, ifstream&, algorithm *);  
  void print_partitions();
  unsigned int processor_index(unsigned int);
  int properties(unsigned int &, unsigned int &, unsigned int & );
  void reset_partitions();
  double static_wallclock_time(unsigned int);
};

double algorithm::dynamic_wallclock_time(unsigned int p_index = 0 )
{
  int i,j;
  linked_list wall_times(partitions);

  //set the number of phases at this level
  set_phases();

  if(level<Levels) {
    for(i=0;i<phases;i++) {
      j=0;
      while(j<max_concurrency) {
        if(level==BASE_LEVEL) {
          processors_in_partition = ComputeProcessors/partitions;
          if(ComputeProcessors%partitions>wall_times.start)
            processors_in_partition++;
        }
        else {
          processors_in_partition = superlevel->processors_in_partition/partitions;
          if(superlevel->processors_in_partition%partitions>wall_times.start)
            processors_in_partition++;
        }
        if(master)
          wall_times[LIST_START] += sublevel->dynamic_wallclock_time(p_index
            + sublevel->processor_index(wall_times.start));
        else
          wall_times[LIST_START] += sublevel->static_wallclock_time(p_index
            + sublevel->processor_index(wall_times.start));
        wall_times[LIST_START] += 2.0*sublevel->communication_time();
        wall_times.sort_inc();
        j++;
      }
    }
    wall_times[LIST_END] += serial_time();
    return(wall_times[LIST_END]);
  }
  else {
    double comp_time;
    if(BalancedSchedule) { 
      comp_time = compute_time(superlevel->processors_in_partition);
      Usage += comp_time*superlevel->processors_in_partition;
      return(comp_time);
    }
    else //skewed schedule
      if(p_index<=RemProcessors) {
        comp_time = compute_time(SimProcessors+1);
        Usage += comp_time*(SimProcessors+1);
        return(comp_time);
      }
      else {
        comp_time = compute_time(SimProcessors);
        Usage += comp_time*SimProcessors;
        return(comp_time);
      }
  }
}

double algorithm::pmin_wallclock_time() 
{
  double pmin_wtime = 0.0;
  int i,j;

  // set the number of phases this level
  set_phases();

  if(level < Levels) {
    for(i=0;i<phases;i++)
      for(j=0;j<max_concurrency;j++) {
        pmin_wtime += sublevel->pmin_wallclock_time();
        pmin_wtime += 2.0*communication_time();
      }
    pmin_wtime += serial_time();
    return(pmin_wtime);
  }
  else {
    double comp_time = compute_time(Pkmin);
    Usage += comp_time*Pkmin;
    return(comp_time);   
  }
    
}

unsigned int algorithm::permute_partitions()
{
  if(level < Levels)
    if(partitions < max_concurrency) {
      if(AllPermutations)
        // test all possible partitionings
        partitions++;
      else
        // don't test permutations that have idle partitions
        do {
          partitions++;
        }while((partitions<max_concurrency) && max_concurrency%partitions);
      return 1;
    }
    else {
      partitions = 1;
      return sublevel->permute_partitions();
    }
  else
   // all permutations have been tested
   return 0;
}

void algorithm::populate(unsigned int lvl, ifstream& alg_in, algorithm *Algs)
{
  // set algorithm level
  level = lvl;

  // initialize the number of partitions
  partitions = 1;

  if(level < Levels) {
 
    // input algorithm constants
    alg_in >> master >> static_phases >> stochastic_phases >> max_concurrency;
    // output constants
    cout << setw(5) << level << setw(8) << master << setw(9) << static_phases;
    cout << setw(12) << stochastic_phases << setw(14) << max_concurrency << "\n";
    if(master)
      LevelsWithMaster++;
    // construct link to the previous level
    if(level != BASE_LEVEL)
      superlevel = &Algs[level-BASE_LEVEL-1];
    // construct link to the next sublevel
    sublevel = &Algs[level-BASE_LEVEL+1];
    // populate the next sublevel
    sublevel->populate(level+1, alg_in, Algs);
    
    }
  else { 
    // last level, input min. processors per simulation
    alg_in >> static_phases >> stochastic_phases >> Pkmin >> Pkmax;
    //initialize other variables
    master=0;
    // output constants
    cout << setw(5) << level << "       -        " << static_phases;
    cout << setw(12) << stochastic_phases << "    pkmin = " << Pkmin; 
    cout << "    pkmax = " << Pkmax << "\n";
    // close algorithm definitions file
    alg_in.close();
    if(level != BASE_LEVEL)
      superlevel = &Algs[level-BASE_LEVEL-1];
  }
}

void algorithm::print_partitions()
{
  if(level < Levels) {
    cout << setw(4) << partitions;
    sublevel->print_partitions();
  }

  return;
}

unsigned int algorithm::processor_index(unsigned int count)
{
  if(level < Levels)
    return sublevel->processor_index(count*partitions);
  else
    return count+1; 
}

void algorithm::reset_partitions() 
{
  if(level<Levels) {
    partitions = 1;
    sublevel->reset_partitions();
  }
  
  return;

}

int algorithm::properties(unsigned int &concurrency_product, unsigned int
  &masters, unsigned int &seriality)
{
  if(level == BASE_LEVEL) {
    //initialize
    concurrency_product=1;
    if((HighPerformance && partitions < max_concurrency && partitions > 1) ||
      (MediumPerformance && partitions > 1) || (!HighPerformance && !MediumPerformance))
      masters=master;
    else
      masters=0;
    seriality=1;
  }
  if(level < Levels) {
    concurrency_product *= partitions;
    if((HighPerformance && sublevel->partitions < sublevel->max_concurrency &&
      partitions > 1) || (MediumPerformance && sublevel->partitions > 1) || 
      (!HighPerformance && !MediumPerformance))
      masters += concurrency_product*sublevel->master;
    if(max_concurrency%partitions == 0)
      seriality *= phases*(max_concurrency/partitions);
    else
      seriality *= phases*(1+max_concurrency/partitions);
    return sublevel->properties(concurrency_product,masters,seriality);
  }
  else {
    seriality *= phases;
    if(SupplyMasters) {
      ComputeProcessors = RequestedProcessors;
      Processors = RequestedProcessors+masters;
    }
    else {
      ComputeProcessors = RequestedProcessors-masters;
      Processors = RequestedProcessors;
    }
    SimProcessors = (ComputeProcessors)/concurrency_product;
    RemProcessors = (ComputeProcessors)%concurrency_product;
    if(SimProcessors < Pkmin || SimProcessors > Pkmax)
      return -1;
    else
      return SimProcessors;
  }
}

double algorithm::static_wallclock_time(unsigned int p_index = 0 )
{
  int i,j,k;
  linked_list wall_times(partitions);

  //set the number of phases at this level
  set_phases();

  if(level<Levels) {
    for(i=0;i<phases;i++) {
      j=0;
      k=0;
      while(j<max_concurrency) {
        if(level==BASE_LEVEL) {
          processors_in_partition = ComputeProcessors/partitions;
          if(ComputeProcessors%partitions>k)
            processors_in_partition++;
        }
        else {
          processors_in_partition = superlevel->processors_in_partition/partitions;
          if(superlevel->processors_in_partition%partitions>k)
            processors_in_partition++;
        }
        if(master)
          wall_times[k] += sublevel->dynamic_wallclock_time(p_index
            + sublevel->processor_index(k));
        else
          wall_times[k] += sublevel->static_wallclock_time(p_index
            + sublevel->processor_index(k));
        wall_times[k] += 2.0*sublevel->communication_time();
        j++;
        k++;
        if(k==partitions)
          k=0;
      }
    wall_times.set_maximum();
    }
    return(wall_times.maximum()+serial_time());
  }
  else {
    double comp_time;
    if(BalancedSchedule) { 
      comp_time = compute_time(superlevel->processors_in_partition);
      Usage += comp_time*superlevel->processors_in_partition;
      return(comp_time);
    }
    else //skewed schedule
      if(p_index<=RemProcessors) {
        comp_time = compute_time(SimProcessors+1);
        Usage += comp_time*(SimProcessors+1);
        return(comp_time);
      }
      else {
        comp_time = compute_time(SimProcessors);
        Usage += comp_time*SimProcessors;
        return(comp_time);
      }
  }
}

class partitioning {
private:
  double sim_time, wall_time, comm_time, ser_time, p_usg;
  double eff, max_eff, stdev_eff;
  unsigned int concurrency_product, masters, seriality, min_prcsrs;
  int sim_prcsrs;
  algorithm *alg;
  void efficency();
  void output_properties();
  void percent_processor_usage();
  double simulation_time();
  void total_processors();
  void wallclock_time();

public:
  partitioning(algorithm *);
  int algorithm_properties();
  void output_heading();
  void output_tail();
  void properties();
  void reset();
};

partitioning::partitioning(algorithm *alg_ptr)
{
  alg=alg_ptr;
  max_eff = 0.0;
}

int partitioning::algorithm_properties()
{
  sim_prcsrs = alg->properties(concurrency_product, masters, seriality);
  if(ExactProcessorCount && RemProcessors!=0)
    return 0;
  else
    return sim_prcsrs;
}

void partitioning::properties()
{
  double tmp;

  sim_time = 0.0;
  wall_time = 0.0;
  Usage = 0.0;

  int exp_old = Experiment;
  for(int i=0;i<Iterations;i++) {

    // reseed the random number generator for each experiment
    Experiment++;
    Seed = 100000*Experiment+94528;
    RNGen->reseed(Seed);
    DRNGen->reseed(Seed);

    // compute the relative time for a single simulation
    sim_time += simulation_time();

    // compute the relative wall clock time
      if(alg->get_master())
        tmp = alg->dynamic_wallclock_time();
      else
        tmp = alg->static_wallclock_time();
      wall_time += tmp;
      eff_coef_var += tmp*tmp;
  }
  Experiment = exp_old;  

  sim_time = sim_time/(double)Iterations;
  eff_coef_var -= wall_time*wall_time/(double)Iterations;
  wall_time = wall_time/(double)Iterations;

  // compute the total number of processors needed
  total_processors();

  // compute the relative efficency
  efficency();

  // compute the percent usage of avaliable processors
  percent_processor_usage();

  // output properties for this partitioning
  output_properties();
}

void partitioning::efficency()
{
  // compute the relative efficency
  eff = Pmin_work/(Processors*wall_time);
  eff_coef_var = 100.0*pow(eff_coef_var,0.5)/eff

  if(eff > max_eff)
    // save the current maximum efficency
    max_eff = eff;
}

void partitioning::output_heading()
{
  // output the table heading for the partioning properties data
  cout << "\n" << setfill('-') << setw(111) << "\n" << setfill(' ');
  cout << "efficency  wall time  sim. time  %usage min proc. sim.proc. ";
  cout << "masters ser.sims.  partitions(tot. level 1 2 ... ) \n";
  cout << setfill('-') << setw(111) << "\n" << setfill(' ');
}

void partitioning::output_properties()
{

    // output properties for the partitioning
    cout << setprecision(5) << setw(8) << eff;
    cout << setw(13) << wall_time;
    cout << setw(11) << sim_time;
    cout << setprecision(1) << setw(7) << p_usg;
    cout << setw(7) << min_prcsrs;
    cout << setw(9) << sim_prcsrs;
    cout << setw(9) << masters;
    cout << setw(9) << seriality;
    cout << setw(9) << concurrency_product << "  ";
    // output the partitions at each level
    alg->print_partitions();
    cout << endl;

}

void partitioning::output_tail()
{
  cout << setprecision(7) << "maximum efficency = " << max_eff << "\n\n";
}

void partitioning::percent_processor_usage()
{
  p_usg = 100.0*Usage/(wall_time*(double)(ComputeProcessors*Iterations));
}


double partitioning::simulation_time()
{
  return alg->compute_time(sim_prcsrs);
}

void partitioning::total_processors()
{
  min_prcsrs = masters + concurrency_product*sim_prcsrs;
}

void partitioning::wallclock_time()
{
  wall_time = ser_time + 2.0*comm_time + seriality*sim_time;
}

void partitioning::reset()
{
  max_eff=0.0;  
}

int main(int argc, char** argv)
{
  int i;

  // process command line parameters
  if(argc < 2)
    exit(0);
  for(i=1;i<=argc-2;i++) {
    if(!strcmp(argv[i],"-a"))
      AllPermutations=1;
    if(!strcmp(argv[i],"-d"))
      DeterministicTimes=1;
    if(!strcmp(argv[i],"-m")){
      MediumPerformance=1;
      HighPerformance=0;
    }
    if(!strcmp(argv[i],"-h")){
      HighPerformance=1;
      MediumPerformance=0;
    }
    if(!strcmp(argv[i],"-l"))
      BalancedSchedule=0;
    if(!strcmp(argv[i],"-p"))
      ExactProcessorCount=1;
    if(!strcmp(argv[i],"-s"))
      SupplyMasters=1;
  }

  // open the algorithim definition file
  ifstream alg_in(argv[argc-1]);

  // input the number of algorithm levels
  alg_in >> RequestedProcessors >> Levels >> Iterations;

  cout.precision(7);
  cout.setf(ios::right);
  cout.setf(ios::fixed);

  // output heading for input data
  cout << "\nProcessors = " << RequestedProcessors 
       << "  Levels = " << Levels 
       << "  Iterations = " << Iterations << "\n";
  cout << "--------------------------------------------------------------\n";
  cout << "                        phases               maximum\n";
  cout << "  level   master   static    stochastic    concurrency\n";
  cout << "--------------------------------------------------------------\n";

  // instanciate the algorithm array
  algorithm *Algs = new algorithm[Levels];
  algorithm *alg = &Algs[0];
  // link and populate each algorithm level with data, print
  alg->populate(BASE_LEVEL, alg_in, Algs);

  // instanciate the partitioning properties class
  partitioning part(alg);

  // compute pmin work
  for(i=0;i<Iterations;i++) {
    // reseed the random number generators for each experiment
    Experiment++;
    Seed = 100000*Experiment+94528;
    RNGen->reseed(Seed);
    DRNGen->reseed(Seed);

    Pmin_time += alg->pmin_wallclock_time(); 
  }
  Pmin_time = Pmin_time/(double)Iterations;
  cout << "T(pmin) = " << setprecision(5) << setw(9) << Pmin_time << "\n";
  Pmin_work = Pkmin*Pmin_time;
  
  alg->reset_partitions();
  part.reset();

  // output heading for properties data
  if(LevelsWithMaster == 0)
    cout << "\nStatic Scheduling:";
  else if(LevelsWithMaster == Levels-1)
    cout << "\nSelf Scheduling:";
  else
    cout << "\nMixed Static/Self Scheduling:";

  part.output_heading();

  // test each possible partitioning -- static case
  do {
  // compute/set properties for the current partitioning
  int sim_prcsrs = part.algorithm_properties();

  if(sim_prcsrs > 0) // if there is at least one processor per simulation
    // compute and output properties for this partitioning
    part.properties();

  // permute the partitioning scheme
  } while(alg->permute_partitions());

  part.output_tail(); 

  delete Algs; 
  delete Rnd;
}
