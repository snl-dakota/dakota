/*
Multiple Objective MetaHeuristics Library in C++ MOMHLib++
Copyright (C) 2001 Andrzej Jaszkiewicz

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation (www.gnu.org); 
either version 2.1 of the License, or (at your option) any later 
version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include "stdafx.h"

#include "tmoscpsolution.h"
#include "pma.h"
#include "immogls.h"
#include "mogls.h"
#include "momsls.h"
#include "spea.h"
#include "nsga.h"
#include "nsgaiic.h"
#include "psa.h"
#include "mosa.h"
#include "smosa.h"

// Names of all tested methods 
char* Methods [10] = {
	"PMA",
	"MOGLS",
	"IMMOGLS",
	"MOMSLS",
	"SPEA",
	"NSGA",
	"NSGAIIC",
	"PSA",
	"MOSA",
	"SMOSA"
};

// Names of all instances
char* InstanceNames [44] = {
		"2scp11A",
		"2scp11B",
		"2scp11C",
		"2scp11D",
		"2scp41A",
		"2scp41B",
		"2scp41C",
		"2scp41D",
		"2scp42A",
		"2scp42B",
		"2scp42C",
		"2scp42D",
		"2scp43A",
		"2scp43B",
		"2scp43C",
		"2scp43D",
		"2scp61A",
		"2scp61B",
		"2scp61C",
		"2scp61D",
		"2scp62A",
		"2scp62B",
		"2scp62C",
		"2scp62D",
		"2scp81A",
		"2scp81B",
		"2scp81C",
		"2scp81D",
		"2scp82A",
		"2scp82B",
		"2scp82C",
		"2scp82D",
		"2scp101A",
		"2scp101B",
		"2scp101C",
		"2scp101D",
		"2scp102A",
		"2scp102B",
		"2scp102C",
		"2scp102D",
		"2scp201A",
		"2scp201B",
		"2scp201C",
		"2scp201D"};

/** This class is used only to measure the number of initial solutions
*	in PMA (and in fact in all hybrid genetic algorithm.
*
*	This class is run just once for each instance and then all hybrid genetic algorithm
*	use the measured number of initial solutions*/
class TInitialSolutionsPMA : public TPMA <TMOSCPSolution> {
public:
	int NumberOfInitialSolutions;

	virtual void InitialPopulationFound () {
		NumberOfInitialSolutions = MainPopulation.size ();
 		cout << MainPopulation.size () << " initial solutions generated\n";
	}

};

/** This class is used only to measure the number of initial solutions
*	in PMA (and in fact in all hybrid genetic algorithm.
*
*	This class is run just once for each instance and then all hybrid genetic algorithm
*	use the measured number of initial solutions*/
class TTimeMeasuringPMA : public TPMA <TMOSCPSolution> {
protected:
	clock_t StartingTime;
public:

	clock_t RunningTime;

	virtual void Start () {
		StartingTime = clock ();
	}

	virtual void End () {
		RunningTime = clock () - StartingTime;
		cout << RunningTime << '\t';
	}
};

clock_t AveragePMARunningTime;

/** Specialization of MOGLS used in this experiment */
class TTestMOGLS : public TMOGLS <TMOSCPSolution> {
protected:
	clock_t StartingTime;
public:
	int NumberOfGeneratedSolutions;

	virtual void Start () {
		StartingTime = clock ();
		NumberOfGeneratedSolutions = 0;
	}

	virtual void NewSolutionGenerated (TMOSCPSolution& NewSolution) {
		NumberOfGeneratedSolutions++;
		// Stop the method if its running time is greater or equal to AveragePMARunningTime
		if (clock () - StartingTime >= AveragePMARunningTime)
			bStop = true;
	}
};

/** Specialization of IMMOGLS used in this experiment */
class TTestIMMOGLS : public TIMMOGLS <TMOSCPSolution> {
protected:
	clock_t StartingTime;
public:
	int NumberOfGeneratedSolutions;

	virtual void Start () {
		StartingTime = clock ();
		NumberOfGeneratedSolutions = 0;
	}

	virtual void NewSolutionGenerated (TMOSCPSolution& NewSolution) {
		NumberOfGeneratedSolutions++;
		// Stop the method if its running time is greater or equal to AveragePMARunningTime
		if (clock () - StartingTime >= AveragePMARunningTime)
			bStop = true;
	}
};

/** Specialization of MOMSLS used in this experiment */
class TTestMOMSLS : public TMOMSLS <TMOSCPSolution> {
protected:
	clock_t StartingTime;
public:
	int NumberOfGeneratedSolutions;

	virtual void Start () {
		StartingTime = clock ();
		NumberOfGeneratedSolutions = 0;
	}

	virtual void NewSolutionGenerated (TMOSCPSolution& NewSolution) {
		NumberOfGeneratedSolutions++;
		// Stop the method if its running time is greater or equal to AveragePMARunningTime
		if (clock () - StartingTime >= AveragePMARunningTime)
			bStop = true;
	}
};

/** Specialization of SPEA used in this experiment */
class TTestSPEA : public TSPEA <TMOSCPSolution> {
protected:
	clock_t StartingTime;
public:
	int NumberOfGeneratedSolutions;

	virtual void Start () {
		StartingTime = clock ();
		NumberOfGeneratedSolutions = 0;
	}

	virtual void NewSolutionGenerated (TMOSCPSolution& NewSolution) {
		NumberOfGeneratedSolutions++;
		// Stop the method if its running time is greater or equal to AveragePMARunningTime
		if (clock () - StartingTime >= AveragePMARunningTime)
			bStop = true;
	}
};

/** Specialization of NSGA used in this experiment */
class TTestNSGA : public TNSGA <TMOSCPSolution> {
protected:
	clock_t StartingTime;
public:
	int NumberOfGeneratedSolutions;

	virtual void Start () {
		StartingTime = clock ();
		NumberOfGeneratedSolutions = 0;
	}

	virtual void NewSolutionGenerated (TMOSCPSolution& NewSolution) {
		NumberOfGeneratedSolutions++;
		// Stop the method if its running time is greater or equal to AveragePMARunningTime
		if (clock () - StartingTime >= AveragePMARunningTime)
			bStop = true;
	}
};

/** Specialization of NSGAIIC used in this experiment */
class TTestNSGAIIC : public TNSGAIIC <TMOSCPSolution> {
protected:
	clock_t StartingTime;
public:
	int NumberOfGeneratedSolutions;

	virtual void Start () {
		StartingTime = clock ();
		NumberOfGeneratedSolutions = 0;
	}

	virtual void NewSolutionGenerated (TMOSCPSolution& NewSolution) {
		NumberOfGeneratedSolutions++;
		// Stop the method if its running time is greater or equal to AveragePMARunningTime
		if (clock () - StartingTime >= AveragePMARunningTime)
			bStop = true;
	}
};

/** Specialization of PSA used in this experiment */
class TTestPSA : public TPSA <TMOSCPSolution> {
protected:
	clock_t StartingTime;
	clock_t LastTime;
	clock_t LevelTime;
public:
	int NumberOfGeneratedSolutions;

	virtual void Start () {
		StartingTime = clock ();
		LastTime = StartingTime;
		NumberOfGeneratedSolutions = 0;
		LevelTime = AveragePMARunningTime / 44;
	}

	virtual void NewSolutionGenerated (TMOSCPSolution& NewSolution) {
		NumberOfGeneratedSolutions++;
		clock_t CurrentTime = clock ();
		// If running time on the current temperature level is greater or equal to LevelTime
		if (CurrentTime - LastTime >= LevelTime) {
			LastTime = CurrentTime;
			// This ends the loop on the current level
			iMove = MovesOnTemperatureLevel;
		}
	}
};

/** Specialization of MOSA used in this experiment */
class TTestMOSA : public TMOSA <TMOSCPSolution> {
protected:
	clock_t StartingTime;
	clock_t LastTime;
	clock_t LevelTime;
public:
	int NumberOfGeneratedSolutions;

	virtual void Start () {
		StartingTime = clock ();
		LastTime = StartingTime;
		NumberOfGeneratedSolutions = 0;
		LevelTime = AveragePMARunningTime / 44;
	}

	virtual void NewSolutionGenerated (TMOSCPSolution& NewSolution) {
		NumberOfGeneratedSolutions++;
		clock_t CurrentTime = clock ();
		// If running time on the current temperature level is greater or equal to LevelTime
		if (CurrentTime - LastTime >= LevelTime) {
			LastTime = CurrentTime;
			// This ends the loop on the current level
			iMove = MovesOnTemperatureLevel;
		}
	}
};

/** Specialization of SMOSA used in this experiment */
class TTestSMOSA : public TSMOSA <TMOSCPSolution> {
protected:
	clock_t StartingTime;
	clock_t LastTime;
	clock_t LevelTime;
public:
	int NumberOfGeneratedSolutions;

	virtual void Start () {
		StartingTime = clock ();
		LastTime = StartingTime;
		NumberOfGeneratedSolutions = 0;
		LevelTime = AveragePMARunningTime / 44;
	}

	virtual void NewSolutionGenerated (TMOSCPSolution& NewSolution) {
		NumberOfGeneratedSolutions++;
		clock_t CurrentTime = clock ();
		// If running time on the current temperature level is greater or equal to LevelTime
		if (CurrentTime - LastTime >= LevelTime) {
			LastTime = CurrentTime;
			// This ends the loop on the current level
			iMove = MovesOnTemperatureLevel;
		}
	}
};

// Number of runs of each algorithm on each instance
int NumberOfRuns = 10;

/** Find coverage measure for all pairs of approximations (generated by different methods) on all instances */
void CoverageEvaluation () {
	// Before constructing solutions it is neccessary to set ::NumberOfObjectives and ::Objectives
	NumberOfObjectives = 2;
	Objectives.resize (NumberOfObjectives);
	int iobj;
	for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		Objectives [iobj].ObjectiveType = _Min;
		Objectives [iobj].bActive = true;
	}

	char FileName [100];

	// For each instance
	int iInstance;
	for (iInstance = 0; iInstance < 44; iInstance++) {
		cout << InstanceNames [iInstance] << '\n';

		TSolutionsSet NdSets [10][10];

		// For each method
		int iMethod;
		for (iMethod = 0; iMethod < 10; iMethod++) {
			// For each iteration of the method
			int iIter;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				// Load the set of solutions
				sprintf (FileName, "Results\\%s%s_%d.txt", Methods [iMethod], InstanceNames [iInstance], iIter);
				NdSets [iMethod][iIter].Load (FileName);
				if (NdSets [iMethod][iIter].size () == 0)
					cout << "Error reading " << FileName << '\n';
			}
		}

		// For each pair of methods
		int iMethod1, iMethod2, iIter1, iIter2;
		for (iMethod1 = 0; iMethod1 < 10; iMethod1++) {
			cout << '.';
			for (iMethod2 = iMethod1 + 1; iMethod2 < 10; iMethod2++) {
				// For each pair of iterations
				for (iIter1 = 0; iIter1 < NumberOfRuns; iIter1++) {
					for (iIter2 = 0; iIter2 < NumberOfRuns; iIter2++) {
						// Calculate and save coverage
						double CoverageByMethod1, CoverageByMethod2;
						NdSets [iMethod1][iIter1].CalculateCoverage (NdSets [iMethod2][iIter2], CoverageByMethod1, CoverageByMethod2);
						fstream Stream ("Coverage.TXT", ios::out | ios::ate | ios::app);
						Stream << InstanceNames [iInstance] << '\t';
						Stream << Methods [iMethod1] << '\t';
						Stream << Methods [iMethod2] << '\t';
						Stream << CoverageByMethod1 << '\t';
						Stream << CoverageByMethod2 << '\n';
						Stream.close ();
					}
				}
			}
		}

		// Free alocated memory
		for (iMethod = 0; iMethod < 10; iMethod++) {
			int iIter;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				NdSets [iMethod][iIter].DeleteAll ();
			}
		}
	}

}

/** Find average value of weighted Chebycheff function for all approximations */
void AverageWeightedChebycheffEvaluation () {
	int iInstance;
	char FileName [100];

	// Before constructing solutions it is neccessary to set ::NumberOfObjectives and ::Objectives
	NumberOfObjectives = 2;
	Objectives.resize (NumberOfObjectives);
	int iobj;
	for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		Objectives [iobj].ObjectiveType = _Min;
		Objectives [iobj].bActive = true;
	}

	// Read (approximate) ideal and points for each instance
	TPoint* IdealPoints [44];
	TPoint* NadirPoints [44];

	fstream IdealNadirStream ("IdealNadir.txt", ios::in);
	for (iInstance = 0; iInstance < 44; iInstance++) {
		IdealPoints [iInstance] = new TPoint;
		NadirPoints [iInstance] = new TPoint;
		IdealPoints [iInstance]->Load (IdealNadirStream);
		NadirPoints [iInstance]->Load (IdealNadirStream);
	}
	IdealNadirStream.close ();

	// For each instance
	for (iInstance = 0; iInstance < 44; iInstance++) {
		// Generate and rescale set of weight vectors
		TWeightsSet WeightsSet;
		WeightsSet.GenerateUniformCover (100);
		WeightsSet.Rescale (*(IdealPoints [iInstance]), *(NadirPoints [iInstance]));

		// For each method
		int iMethod;
		for (iMethod = 0; iMethod < 10; iMethod++) {
			int iIter;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				// Load the set of solutions
				sprintf (FileName, "Results\\%s%s_%d.txt", Methods [iMethod], InstanceNames [iInstance], iIter);
				TSolutionsSet NDSet;
				NDSet.Load (FileName);
				if (NDSet.size () == 0)
					cout << "Error reading " << FileName << '\n';

				// Calculate and save the measure
				double AverageWeightedChebycheff = NDSet.AverageScalarizingFunction (*(IdealPoints [iInstance]),
					_Chebycheff, WeightsSet);

				fstream Stream ("AvWeightedCh.TXT", ios::out | ios::ate | ios::app);
				Stream << InstanceNames [iInstance] << '\t';
				Stream << Methods [iMethod] << '\t';
				Stream << iIter << '\t';
				Stream << AverageWeightedChebycheff << '\n';
				Stream.close ();

				NDSet.DeleteAll ();
			}
		}
	}

	// Free allocated memory
	for (iInstance = 0; iInstance < 44; iInstance++) {
		delete IdealPoints [iInstance];
		delete NadirPoints [iInstance];
	}
}

int main()
{
//	AverageWeightedChebycheffEvaluation ();
//	CoverageEvaluation ();
//	exit (0);
	
	int iInstance;
	char FileName [100];

	// For each instance
	for (iInstance = 0; iInstance < 44; iInstance ++) {
		// Makes the process repeatable
		srand (11);
		
		strcpy (FileName, "Instances\\");
		strcat (FileName, InstanceNames [iInstance]);
		strcat (FileName, ".txt");
		if (Problem.Load (FileName)) {
			int NumberOfGeneratedSolutions;
			int iIter;
			
			cout << InstanceNames [iInstance] << " read\n";
			
			fstream Stream ("Results\\LOG.TXT", ios::out | ios::ate | ios::app);
			Stream << InstanceNames [iInstance] << " read\n";;
			Stream.close ();
			
			// Find the number of initial solutions for all 
			// hybrid genetic algorithms
			TInitialSolutionsPMA  InitialSolutionsPMA;
			InitialSolutionsPMA.SetParameters (10, 0, 0, _Chebycheff);
			InitialSolutionsPMA.Run ();
			Stream.open ("Results\\LOG.TXT", ios::out | ios::ate | ios::app);
			Stream << "Initial solutions generated\t" << InitialSolutionsPMA.NumberOfInitialSolutions << '\n';
			Stream.close ();
			
			
			// Find running time that will be used by all methods
			// while running PMA
			clock_t RunningTime = 0;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				TTimeMeasuringPMA TimeMeasuringPMA;
				TimeMeasuringPMA.SetParameters (15, 
					InitialSolutionsPMA.NumberOfInitialSolutions, 
					10, _Chebycheff);
				TimeMeasuringPMA.Run ();
				RunningTime += TimeMeasuringPMA.RunningTime;
				sprintf (FileName, "Results\\PMA%s_%d.txt", InstanceNames [iInstance], iIter);
				TimeMeasuringPMA.pNondominatedSet->Save (FileName);
			}
			cout << "PMA" << '\t' << "AverageRunningTime " << RunningTime / NumberOfRuns << '\n';
			AveragePMARunningTime = RunningTime / NumberOfRuns;
			Stream.open ("Results\\LOG.TXT", ios::out | ios::ate | ios::app);
			Stream << "PMA" << '\t' << "AverageRunningTime " << RunningTime / NumberOfRuns << '\n';
			Stream.close ();
			
			// Run MOGLS
			NumberOfGeneratedSolutions = 0;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				TTestMOGLS TestMOGLS;
				TestMOGLS.SetParameters (15, 
					InitialSolutionsPMA.NumberOfInitialSolutions, 
					100000, _Chebycheff);
				TestMOGLS.Run ();
				cout << '.';
				NumberOfGeneratedSolutions += TestMOGLS.NumberOfGeneratedSolutions;
				sprintf (FileName, "Results\\MOGLS%s_%d.txt", InstanceNames [iInstance], iIter);
				TestMOGLS.pNondominatedSet->Save (FileName);
			}
			cout << "MOGLS" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.open ("Results\\LOG.TXT", ios::out | ios::ate | ios::app);
			Stream << "MOGLS" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.close ();
			
			
			// Run IMMOGLS
			NumberOfGeneratedSolutions = 0;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				TTestIMMOGLS TestIMMOGLS;
				TestIMMOGLS.SetParameters (
					InitialSolutionsPMA.NumberOfInitialSolutions, 
					100000, InitialSolutionsPMA.NumberOfInitialSolutions / 10, _Chebycheff);
				TestIMMOGLS.Run ();
				cout << '.';
				NumberOfGeneratedSolutions += TestIMMOGLS.NumberOfGeneratedSolutions;
				sprintf (FileName, "Results\\IMMOGLS%s_%d.txt", InstanceNames [iInstance], iIter);
				TestIMMOGLS.pNondominatedSet->Save (FileName);
			}
			cout << "IMMOGLS" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.open ("Results\\LOG.TXT", ios::out | ios::ate | ios::app);
			Stream << "IMMOGLS" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.close ();
			
			// Run MOMSLS
			NumberOfGeneratedSolutions = 0;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				TTestMOMSLS TestMOMSLS;
				TestMOMSLS.SetParameters (100000, _Chebycheff);
				TestMOMSLS.Run ();
				cout << '.';
				NumberOfGeneratedSolutions += TestMOMSLS.NumberOfGeneratedSolutions;
				sprintf (FileName, "Results\\MOMSLS%s_%d.txt", InstanceNames [iInstance], iIter);
				TestMOMSLS.pNondominatedSet->Save (FileName);
			}
			cout << "MOMSLS" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.open ("Results\\LOG.TXT", ios::out | ios::ate | ios::app);
			Stream << "MOMSLS" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.close ();
			
			// Run SPEA
			NumberOfGeneratedSolutions = 0;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				TTestSPEA TestSPEA;
				TestSPEA.SetParameters (100, 100000000, 0, 100, 150, true);
				TestSPEA.Run ();
				cout << '.';
				NumberOfGeneratedSolutions += TestSPEA.NumberOfGeneratedSolutions;
				sprintf (FileName, "Results\\SPEA%s_%d.txt", InstanceNames [iInstance], iIter);
				TestSPEA.pNondominatedSet->Save (FileName);
			}
			cout << "SPEA" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.open ("Results\\LOG.TXT", ios::out | ios::ate | ios::app);
			Stream << "SPEA" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.close ();
			
			// Run NSGA
			NumberOfGeneratedSolutions = 0;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				TTestNSGA TestNSGA;
				TestNSGA.SetParameters (100, 100000000, 0, 0.2, true);
				TestNSGA.Run ();
				cout << NumberOfGeneratedSolutions << '.';
				NumberOfGeneratedSolutions += TestNSGA.NumberOfGeneratedSolutions;
				sprintf (FileName, "Results\\NSGA%s_%d.txt", InstanceNames [iInstance], iIter);
				TestNSGA.pNondominatedSet->Save (FileName);
			}
			cout << "NSGA" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.open ("Results\\LOG.TXT", ios::out | ios::ate | ios::app);
			Stream << "NSGA" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.close ();
			
			// Run NSGAIIC
			NumberOfGeneratedSolutions = 0;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				TTestNSGAIIC TestNSGAIIC;
				TestNSGAIIC.SetParameters (100, 100000000, 0, 0.8, true);
				TestNSGAIIC.Run ();
				cout << '.';
				NumberOfGeneratedSolutions += TestNSGAIIC.NumberOfGeneratedSolutions;
				sprintf (FileName, "Results\\NSGAIIC%s_%d.txt", InstanceNames [iInstance], iIter);
				TestNSGAIIC.pNondominatedSet->Save (FileName);
			}
			cout << "NSGAIIC" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.open ("Results\\LOG.TXT", ios::out | ios::ate | ios::app);
			Stream << "NSGAIIC" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.close ();
			
			// Run PSA
			NumberOfGeneratedSolutions = 0;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				TTestPSA TestPSA;
				TestPSA.SetParameters (0.05, 0.0005, 0.9, 0.0003, 1000000, 8);
				TestPSA.Run ();
				cout << '.';
				NumberOfGeneratedSolutions += TestPSA.NumberOfGeneratedSolutions;
				sprintf (FileName, "Results\\PSA%s_%d.txt", InstanceNames [iInstance], iIter);
				TestPSA.pNondominatedSet->Save (FileName);
			}
			cout << "PSA" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.open ("Results\\LOG.TXT", ios::out | ios::ate | ios::app);
			Stream << "PSA" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.close ();
			
			// Run MOSA
			NumberOfGeneratedSolutions = 0;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				TTestMOSA TestMOSA;
				TestMOSA.SetParameters (0.05, 0.0005, 0.9, 1000000, 8);
				TestMOSA.Run ();
				cout << '.';
				NumberOfGeneratedSolutions += TestMOSA.NumberOfGeneratedSolutions;
				sprintf (FileName, "Results\\MOSA%s_%d.txt", InstanceNames [iInstance], iIter);
				TestMOSA.pNondominatedSet->Save (FileName);
			}
			cout << "MOSA" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.open ("Results\\LOG.TXT", ios::out | ios::ate | ios::app);
			Stream << "MOSA" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.close ();
			
			
			// Run SMOSA
			NumberOfGeneratedSolutions = 0;
			for (iIter = 0; iIter < NumberOfRuns; iIter++) {
				TTestSMOSA TestSMOSA;
				TestSMOSA.SetParameters (0.05, 0.0005, 0.9, 1000000);
				TestSMOSA.Run ();
				cout << '.';
				NumberOfGeneratedSolutions += TestSMOSA.NumberOfGeneratedSolutions;
				sprintf (FileName, "Results\\SMOSA%s_%d.txt", InstanceNames [iInstance], iIter);
				TestSMOSA.pNondominatedSet->Save (FileName);
			}
			cout << "SMOSA" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.open ("Results\\LOG.TXT", ios::out | ios::ate | ios::app);
			Stream << "SMOSA" << '\t' << "NumberOfGeneratedSolutions " << NumberOfGeneratedSolutions / NumberOfRuns << '\n';
			Stream.close ();
			
		}
		else
			cout << "Cannot read " << InstanceNames [iInstance] << '\n';
	}
	return 0;
}

