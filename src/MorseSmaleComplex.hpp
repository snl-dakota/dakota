/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        MorseSmaleComplex
//- Description:  Class declaration for MorseSmaleComplex
//- Owner:        Dan Maljovec, Mohamed Ebeida
//- Version: $Id$

#ifndef MS_COMPLEX_H
#define MS_COMPLEX_H

#include "ANN/ANN.h"
#include "ANN/ANNperf.h"
#include "ANN/ANNx.h"

#include <vector>
#include <cstdlib>
#include <cstdio>

#pragma once
//using namespace std;

#define LOCAL_MIN 0
#define LOCAL_MAX 1
#define SADDLE 2
#define REGULAR 3

class KNN_Edge;

class Vertex
{
public:
	Vertex(int n, double *p, double _val, int _id);
	void CreateANNpoint(ANNpoint &p);
	double GetXi(int i);
	void Union_Max(Vertex * v, Vertex * V[]);
	int Find_Max(Vertex * V[]);
	void Union_Min(Vertex * v, Vertex * V[]);
	int Find_Min(Vertex * V[]);
	int GetIthNeighbor(int i, KNN_Edge * E[]);
	void ResetExtrema();
	double Value();
  double SDistance(Vertex *v);
	Vertex(Vertex &v);
	~Vertex();

	int NeighborMax() { return UF_max;}
	int NeighborMin() { return UF_min;}

	int e;
	int classification;		//0 = minimum, 1=maximmum, 2=saddle, 3=regular
	int ID;
	double persistence;

private:
	double *x;
	double val;
	int d;
	int UF_max; //The neighboring maximumm
	int UF_min; //The neighboring minimum
	int PC_UF_max; //The MS-Cell-max
	int PC_UF_min; //The MS-Cell-min
};

struct Saddle
{
	int idx;
	int more;
	int less;
	double persistence;
	Saddle *next;
};

class KNN_Edge
{
public:
	KNN_Edge(Vertex * _start, Vertex * _end, KNN_Edge * E[], int id);
	int ID;
	int start,end;
	int nextKNN_Edge;
};

class MS_Crystal
{
public:
	MS_Crystal(Vertex ** _V, int *_vIds, int _count, double _d);
	MS_Crystal(MS_Crystal *a, MS_Crystal *b, bool mergeMax, int persistence);
	MS_Crystal(MS_Crystal *a, bool mergeMax, int nuExtrema, int persistence);
	~MS_Crystal();
	int size();
	int GetVertexID(int i);
	bool Exists(double persistence);

	int MaxV();
	int MinV();

	double minP;
	double maxP;

private:
	int minV;
	int maxV;
	Vertex **V;
	int *v;
	int count;
	int d;
};

class MS_Complex
{
public:
	MS_Complex(double  *points, int dimension, int count, int _k=15, bool perturb=false);
	MS_Complex(MS_Complex &C, double *new_point);
	~MS_Complex();
	void Destroy();
	void KNN();
	void Compute();
  void Print(std::ostream &out);
	Vertex * *V;
	KNN_Edge * *E;
	MS_Crystal * *C;
	//Saddle * *S;
	int szV;
	int szE;
	int numV;
	int numE;
	int numC;
	int numKneighbors;
	int d;
	int *V_to_C;
	int p_levels;
	int globalMinIdx;
	int globalMaxIdx;
	double *persistences;
  double maxDist;
	int szP;

#ifdef USING_GL
	void Draw(double gMin, double gMax,bool flatMode);
	void DrawBurst();
#endif

	double CompareExtremaCount(MS_Complex &_C);
  double CompareClassChange(MS_Complex &_C);
  double ComparePersistence(MS_Complex &_C);
	double ComparePersistenceNoSaddles(MS_Complex &_C);
  double CompareBottleneck(MS_Complex &_C);
  double CompareLabels(MS_Complex &_C, double p);
	double CloseToExtrema(double *v, double p);
	double FarFromExtrema(double *v, double p);
  double FarFromSaddle(double *v, double p);
  double FarFromCP(double *v, double p);
  int    GetIthHighestPersistence(int i);
  int    GetIthHighestPersistenceSaddle(int i);
  bool   IsCritical(int i);
  bool   IsExtrema(int i);
  bool   IsMaximum(int i)
  {
    return i < numV && i >= 0 && V[i]->classification == LOCAL_MAX;
  }
  bool   IsSaddle(int i);
  Vertex * GetVertex(int i);

  int CountExtrema(double p=0);
  int CountMaxima(double p=0);
  int CountMinima(double p=0);
  int CountSaddles(double p=0);

private:
  bool perturbed;
  bool DoesEdgeExist(int v1, int v2, int maxIndex)
  {
    for(int i = 0; i < maxIndex; i++)
      if(E[i]->start == v1 && E[i]->end == v2)
        return true;
    return false;
  }
	ANNkd_tree *SearchStructure;
};
double ScoreTOPOB(MS_Complex &C, double *x);
double ScoreTOPOP(MS_Complex &C, double *x);
std::vector<int> ScoreTOPOHP(int dimension, int knn,
  double *training, double *trainingY, int n_training, 
  double *candidates, double *candidateY, int n_candidates);


#endif //MS_COMPLEX_H
