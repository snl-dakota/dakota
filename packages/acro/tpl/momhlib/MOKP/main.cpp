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

#include "mosa.h"
#include "psa.h"
#include "smosa.h"
#include "mogls.h"
#include "pma.h"
#include "immogls.h"
#include "momsls.h"
#include "spea.h"
#include "nsga.h"
#include "nsgaii.h"
#include "nsgaiic.h"
#include "mokpproblem.h"
#include "mokpsolution.h"

#include <stdio.h>

int main () 
{
char ProblemFileName [100] = {0};
char ResultFileName [100] = {0};
char s [100] = {0};
int iInitialPopulationSizes [9] = {150, 200, 250, 200, 250, 300, 250, 300, 350};
int iMOSASolutions [9] = {15, 20, 25, 20, 25, 30, 25, 30, 35};
int iMOSAGeneratingPopulationsSizes [9] = {10, 12, 15, 20, 25, 30, 30, 40, 50};
int iPSAGeneratingPopulationsSizes [9] = {5, 5, 6, 10, 12, 15, 15, 20, 25};

//	srand ((unsigned)time (NULL));
	int iProblem = 0;
	int iNumberOfObjectives; for (iNumberOfObjectives = 2; iNumberOfObjectives <= 4; iNumberOfObjectives++) {
		int iNumberofItems; for (iNumberofItems = 250; iNumberofItems <= 750; iNumberofItems += 250) {

			strcpy (ProblemFileName, "knapsack_");
			sprintf (s, "%i", iNumberofItems);
			strcat (ProblemFileName, s);
			strcat (ProblemFileName, ".");
			sprintf (s, "%i", iNumberOfObjectives);
			strcat (ProblemFileName, s);

			bool res = Problem.Load (ProblemFileName);
			if (!res) {
				cout << "Cannot read " << ProblemFileName << '\n';
			}
			else {
				fstream Stream ("results\\LOG.TXT", ios::out | ios::ate | ios::app);
				Stream << "Problem " << ProblemFileName << '\n';
				Stream.close ();

				cout << "Problem " << ProblemFileName << " read" << '\n';

				TPSA <TMOKPSolution> MOKPPSA;
				TMOSA <TMOKPSolution> MOKPMOSA;
				TSMOSA <TMOKPSolution> MOKPSMOSA;
				TMOGLS <TMOKPSolution> MOKPMOGLS;
				TIMMOGLS <TMOKPSolution> MOKPIMMOGLS;
				TPMA <TMOKPSolution> MOKPPMA;
				TMOMSLS <TMOKPSolution> MOKPMOMSLS;
                TSPEA <TMOKPSolution> MOKPSPEA;
                TNSGAII <TMOKPSolution> MOKPNSGAII;
                TNSGAIIC <TMOKPSolution> MOKPNSGAIIC;
                TNSGA <TMOKPSolution> MOKPNSGA;
				
				int iIter; for (iIter = 0; iIter < 1; iIter++) {

					cout << "SPEA\n";
					{
                        MOKPSPEA.SetParameters(iInitialPopulationSizes [iProblem], 50, 0.2, 10000,15000,true);
						time_t StartingTime = time (NULL);
						MOKPSPEA.Run ();
						time_t EndingTime = time (NULL);
						time_t RunningTime = EndingTime - StartingTime;
						sprintf (ResultFileName, "results\\SPEA_Nd_%i_%i_%i.txt", iNumberofItems, iNumberOfObjectives, iIter);
						MOKPSPEA.pNondominatedSet->Save (ResultFileName);
						fstream Stream ("results\\LOG.TXT", ios::out | ios::ate | ios::app);
						Stream << "SPEA\t" << EndingTime - StartingTime << '\n';
						Stream.close ();
					}


					cout << "NSGA\n";
					{
						MOKPNSGA.SetParameters(iInitialPopulationSizes [iProblem], 50, 0.2, 0.4, true);
						time_t StartingTime = time (NULL);
						MOKPNSGA.Run ();
						time_t EndingTime = time (NULL);
						time_t RunningTime = EndingTime - StartingTime;
						sprintf (ResultFileName, "results\\NSGA_Nd_%i_%i_%i.txt", iNumberofItems, iNumberOfObjectives, iIter);
						MOKPNSGA.pNondominatedSet->Save (ResultFileName);
						fstream Stream ("results\\LOG.TXT", ios::out | ios::ate | ios::app);
						Stream << "NSGA\t" << EndingTime - StartingTime << '\n';
						Stream.close ();
					}


					cout << "NSGAII\n";
					{
						MOKPNSGAII.SetParameters(iInitialPopulationSizes [iProblem], 50, 0.2, true);
						time_t StartingTime = time (NULL);
						MOKPNSGAII.Run ();
						time_t EndingTime = time (NULL);
						time_t RunningTime = EndingTime - StartingTime;
						sprintf (ResultFileName, "results\\NSGAII_Nd_%i_%i_%i.txt", iNumberofItems, iNumberOfObjectives, iIter);
						MOKPNSGAII.pNondominatedSet->Save (ResultFileName);
						fstream Stream ("results\\LOG.TXT", ios::out | ios::ate | ios::app);
						Stream << "NSGAII\t" << EndingTime - StartingTime << '\n';
						Stream.close ();
					}


					cout << "NSGAIIC\n";
					{
						MOKPNSGAIIC.SetParameters(iInitialPopulationSizes [iProblem], 50, 0.2, 0.5,true);
						time_t StartingTime = time (NULL);
						MOKPNSGAIIC.Run ();
						time_t EndingTime = time (NULL);
						time_t RunningTime = EndingTime - StartingTime;
						sprintf (ResultFileName, "results\\SPEA_Nd_%i_%i_%i.txt", iNumberofItems, iNumberOfObjectives, iIter);
						MOKPSPEA.pNondominatedSet->Save (ResultFileName);
						fstream Stream ("results\\LOG.TXT", ios::out | ios::ate | ios::app);
						Stream << "SPEA\t" << EndingTime - StartingTime << '\n';
						Stream.close ();
					}
                    
					cout << "MOGLS\n";
					{
						MOKPMOGLS.SetParameters (20, iInitialPopulationSizes [iProblem], 50, _Linear);
						time_t StartingTime = time (NULL);
						MOKPMOGLS.Run ();
						time_t EndingTime = time (NULL);
						time_t RunningTime = EndingTime - StartingTime;
						sprintf (ResultFileName, "results\\MOGLS_Nd_%i_%i_%i.txt", iNumberofItems, iNumberOfObjectives, iIter);
						MOKPMOGLS.pNondominatedSet->Save (ResultFileName);
						fstream Stream ("results\\LOG.TXT", ios::out | ios::ate | ios::app);
						Stream << "MOGLS\t" << EndingTime - StartingTime << '\n';
						Stream.close ();
					}


					cout << "PMA\n";
					{
						MOKPPMA.SetParameters (20, iInitialPopulationSizes [iProblem], 50, _Chebycheff);
						time_t StartingTime = time (NULL);
						MOKPPMA.Run ();
						time_t EndingTime = time (NULL);
						time_t RunningTime = EndingTime - StartingTime;
						sprintf (ResultFileName, "results\\PMA_Nd_%i_%i_%i.txt", iNumberofItems, iNumberOfObjectives, iIter);
						MOKPPMA.pNondominatedSet->Save (ResultFileName);
						fstream Stream ("results\\LOG.TXT", ios::out | ios::ate | ios::app);
						Stream << "PMA\t" << EndingTime - StartingTime << '\n';
						Stream.close ();
					}

					cout << "IMMOGLS\n";
					{
						MOKPIMMOGLS.SetParameters (iInitialPopulationSizes [iProblem], 50, iInitialPopulationSizes [iProblem] / 10, _Chebycheff);
						time_t StartingTime = time (NULL);
						MOKPIMMOGLS.Run ();
						time_t EndingTime = time (NULL);
						time_t RunningTime = EndingTime - StartingTime;
						sprintf (ResultFileName, "results\\IMMOGLS_Nd_%i_%i_%i.txt", iNumberofItems, iNumberOfObjectives, iIter);
						MOKPIMMOGLS.pNondominatedSet->Save (ResultFileName);
						fstream Stream ("results\\LOG.TXT", ios::out | ios::ate | ios::app);
						Stream << "IMMOGLS\t" << EndingTime - StartingTime << '\n';
						Stream.close ();
					}

					cout << "PSA\n";
					{
						int MovesOnTemperatureLevel = (int)(500 * iInitialPopulationSizes [iProblem] / (44.0 * (iPSAGeneratingPopulationsSizes [iProblem])));
						MOKPPSA.SetParameters (0.05, 0.0005, 0.9, 0.0003, MovesOnTemperatureLevel, iPSAGeneratingPopulationsSizes [iProblem]);
						time_t StartingTime = time (NULL);
						MOKPPSA.Run ();
						time_t EndingTime = time (NULL);
						time_t RunningTime = EndingTime - StartingTime;
						sprintf (ResultFileName, "results\\PSA_Nd_%i_%i_%i.txt", iNumberofItems, iNumberOfObjectives, iIter);
						MOKPPSA.pNondominatedSet->Save (ResultFileName);
						fstream Stream ("results\\LOG.TXT", ios::out | ios::ate | ios::app);
						Stream << "PSA\t" << EndingTime - StartingTime << '\n';
						Stream.close ();
					}

					cout << "MOSA\n";
					{
						int MovesOnTemperatureLevel = (int)(500 * iInitialPopulationSizes [iProblem] / (44.0 * (iMOSAGeneratingPopulationsSizes [iProblem])));
						MOKPMOSA.SetParameters (0.05, 0.0005, 0.9, MovesOnTemperatureLevel, iMOSAGeneratingPopulationsSizes [iProblem]);
						time_t StartingTime = time (NULL);
						MOKPMOSA.Run ();
						time_t EndingTime = time (NULL);
						time_t RunningTime = EndingTime - StartingTime;
						sprintf (ResultFileName, "results\\MOSA_Nd_%i_%i_%i.txt", iNumberofItems, iNumberOfObjectives, iIter);
						MOKPMOSA.pNondominatedSet->Save (ResultFileName);
						fstream Stream ("results\\LOG.TXT", ios::out | ios::ate | ios::app);
						Stream << "MOSA\t" << EndingTime - StartingTime << '\n';
						Stream.close ();
					}

					cout << "SMOSA\n";
					{
						int MovesOnTemperatureLevel = (int)(500 * iInitialPopulationSizes [iProblem] / 44.0);
						MOKPSMOSA.SetParameters (0.05, 0.0005, 0.9, MovesOnTemperatureLevel);
						time_t StartingTime = time (NULL);
						MOKPSMOSA.Run ();
						time_t EndingTime = time (NULL);
						time_t RunningTime = EndingTime - StartingTime;
						sprintf (ResultFileName, "results\\SMOSA_Nd_%i_%i_%i.txt", iNumberofItems, iNumberOfObjectives, iIter);
						MOKPSMOSA.pNondominatedSet->Save (ResultFileName);
						fstream Stream ("results\\LOG.TXT", ios::out | ios::ate | ios::app);
						Stream << "SMOSA\t" << EndingTime - StartingTime << '\n';
						Stream.close ();
					}

					cout << "MOMSLS\n";
					{
						MOKPMOMSLS.SetParameters (iInitialPopulationSizes [iProblem] * 50, _Chebycheff);
						time_t StartingTime = time (NULL);
						MOKPMOMSLS.Run ();
						time_t EndingTime = time (NULL);
						time_t RunningTime = EndingTime - StartingTime;
						sprintf (ResultFileName, "results\\MOMSLS_Nd_%i_%i_%i.txt", iNumberofItems, iNumberOfObjectives, iIter);
						MOKPMOMSLS.pNondominatedSet->Save (ResultFileName);
						fstream Stream ("results\\LOG.TXT", ios::out | ios::ate | ios::app);
						Stream << "MOMSLS\t" << EndingTime - StartingTime << '\n';
						Stream.close ();
					}

				}
			}
			iProblem++;
		}
	}

	return (0);
}
