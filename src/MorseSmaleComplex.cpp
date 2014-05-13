/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        MorseSmaleComplex
//- Description:  Class implementation for MorseSmaleComplex
//- Owner:        Dan Maljovec, Mohamed Ebeida
//- Version: $Id$

#include "MorseSmaleComplex.hpp"
#include <map>
#include <vector>
// From the Dionysus package
#include "topology/persistence-diagram.h"

#ifdef USING_GL
  #include <GL/glut.h>
#endif

//using namespace std;

///////////////////////////////////////////////////
//Vertex
//////////////////////////////////////////////////
Vertex::Vertex(int n, double *p, double _val, int _id)
{
	d = n;
	x = new double[n];
	for(int i=0; i < n; i++)
		x[i] = p[i];

	e = -1;
	PC_UF_min = PC_UF_max = UF_min = UF_max = ID = _id;
	persistence = 0;
	val = _val;
	classification = REGULAR;
}

double Vertex::GetXi(int i)
{
	return x[i];
}

int Vertex::GetIthNeighbor(int i, KNN_Edge * E[])
{
	int neighborID = E[this->e]->end;
	int nextKNN_EdgeID = E[this->e]->nextKNN_Edge;
	for(int j = 0; j < i; j++)
	{
		if(nextKNN_EdgeID != -1)
		{
			neighborID = E[nextKNN_EdgeID]->end;
			nextKNN_EdgeID = E[nextKNN_EdgeID]->nextKNN_Edge;
		}
		else
		{
			neighborID = -1;
		}
	}
	return neighborID;
}

void Vertex::CreateANNpoint(ANNpoint &p)
{
	if(p != NULL)
		delete [] p;
	p = new ANNcoord[d];
	for(int i = 0; i < d; i++)
		p[i] = this->x[i];
}

void Vertex::Union_Max(Vertex * v, Vertex * V[])
{
	Vertex *a = V[this->Find_Max(V)];
	Vertex *b = V[v->Find_Max(V)];
	if(a->val > b->val)
	{
		b->UF_max = this->ID;
		b->PC_UF_max = a->ID;
	}
	else if(a->val == b->val)
	{
		if(a->ID > b->ID)
		{
			b->UF_max = this->ID;
			b->PC_UF_max = a->ID;
		}
		else
		{
			a->UF_max = v->ID;
			a->PC_UF_max = b->ID;
		}
	}
	else
	{
		a->UF_max = v->ID;
		a->PC_UF_max = b->ID;
	}
}
int Vertex::Find_Max(Vertex * V[])
{
	if(this->UF_max == this->ID)
		return this->ID;

	this->PC_UF_max = V[this->PC_UF_max]->Find_Max(V);	
	return this->PC_UF_max;
}

double Vertex::SDistance(Vertex *v)
{
	double sDist = 0;
	for(int i = 0; i < d; i++)
		sDist += (x[i]-v->x[i])*(x[i]-v->x[i]);
	return sDist;
}

void Vertex::Union_Min(Vertex * v, Vertex * V[])
{
	Vertex *a = V[this->Find_Min(V)];
	Vertex *b = V[v->Find_Min(V)];
	if(a->val < b->val)
	{
		b->UF_min = this->ID;
		b->PC_UF_min = a->ID;
	}
	else if(a->val == b->val)
	{
		if(a->ID < b->ID)
		{
			b->UF_min = this->ID;
			b->PC_UF_min = a->ID;
		}
		else
		{
			a->UF_min = v->ID;
			a->PC_UF_min = b->ID;
		}
	}
	else
	{
		a->UF_min = v->ID;
		a->PC_UF_min = b->ID;
	}
}
int Vertex::Find_Min(Vertex * V[])
{
	if(this->UF_min == this->ID)
		return this->ID;

	this->PC_UF_min = V[this->PC_UF_min]->Find_Min(V);	
	return this->PC_UF_min;
}
void Vertex::ResetExtrema()
{
	UF_max = UF_min = PC_UF_min = PC_UF_max = ID;
	persistence = 0;
}
double Vertex::Value() { return val; }
Vertex::Vertex(Vertex &v)
{
	classification = v.classification;
	d = v.d;
	e = v.e;
	ID = v.ID;
	PC_UF_max = v.PC_UF_max;
	PC_UF_min = v.PC_UF_min;
	UF_max = v.UF_max;
	UF_min = v.UF_max;
	val = v.val;
	x = new double[d];
	for(int i = 0; i < d; i++)
		x[i] = v.x[i];
	persistence = 0;
}
Vertex::~Vertex() { delete [] x; }
///////////////////////////////////////////////////
//KNN_Edge
//////////////////////////////////////////////////
KNN_Edge::KNN_Edge(Vertex * _start, Vertex * _end, KNN_Edge * E[], int id)
{
	ID = id;
	E[id] = this;

	start = _start->ID;
	end = _end->ID;

	this->nextKNN_Edge = _start->e;
	_start->e = id;
}
///////////////////////////////////////////////////
//Crystal
//////////////////////////////////////////////////
MS_Crystal::MS_Crystal(Vertex ** _V, int *_vIds, int _count, double _d)
{
	V = _V;
	v = _vIds;
	count = _count;
	minV = v[count-2];
	maxV = v[count-1];
	minP = 0;
	maxP = 100;

	d = _d;
}

MS_Crystal::~MS_Crystal() { delete [] v; }

MS_Crystal::MS_Crystal(MS_Crystal *a, MS_Crystal *b, bool mergeMax, int persistence)
{
	minP = a->maxP = b->maxP = persistence;
	V = a->V;
	v = new int[a->count+b->count];
	int i = 0;
	for( ; i < a->count; i++)
		v[i] = a->v[i];
	for( ; i < a->count+b->count; i++)
		v[i] = b->v[i-a->count];

	count = a->count+b->count;
	d = a->d;

	if(mergeMax)
	{
		if(a->minV != b->minV)
			printf("We have a problem.");
		else
			minV = a->minV;

		if(V[a->maxV]->persistence > V[b->maxV]->persistence)
			maxV = a->maxV;
		else
			maxV = b->maxV;
	}
	else
	{
		if(a->maxV != b->maxV)
			printf("We have a problem.");
		else
			maxV = a->maxV;

		if(V[a->minV]->persistence > V[b->minV]->persistence)
			minV = a->minV;
		else
			minV = b->minV;
	}

	maxP = 100;
}
MS_Crystal::MS_Crystal(MS_Crystal *a, bool mergeMax, int nuExtrema, int persistence)
{
	minP = a->maxP = persistence;
	V = a->V;
	v = new int[a->count+1];
	int i = 0;
	for( ; i < a->count; i++)
		v[i] = a->v[i];
	v[i] = nuExtrema;

	count = a->count+1;
	d = a->d;

	if(mergeMax)
	{
			minV = a->minV;
			maxV = nuExtrema;
	}
	else
	{
			maxV = a->maxV;
			minV = nuExtrema;
	}

	maxP = 100;
}
int MS_Crystal::size() {	return count;	}
int MS_Crystal::GetVertexID(int i) {	return v[i];	}
bool MS_Crystal::Exists(double persistence)
	{ return (persistence >= minP && persistence < maxP); }

int MS_Crystal::MaxV() { return maxV; }
int MS_Crystal::MinV() { return minV; }
///////////////////////////////////////////////////
//MS Complex
//////////////////////////////////////////////////
void swap(int i, int j, Saddle **S)
{
	Saddle *temp = S[i];
	S[i] = S[j];
	S[j] = temp;
}
int partition(Saddle **S, int left, int right, int pivot)
{
	swap(pivot, right, S);//pivot is now in right
	int insertionIdx = left;
	for(int i = left; i < right; i++)
	{
		if(S[i]->persistence < S[right]->persistence)
		{
			swap(i,insertionIdx, S);
			insertionIdx++;
		}
	}
	swap(insertionIdx, right, S);// put pivot in place
	return insertionIdx;
}
void quickSort(Saddle **S, int left, int right)
{
	if(left < right)
	{
		int pivot = (right + left) / 2;
		int nuPivot = partition(S, left, right, pivot);
		quickSort(S,left, nuPivot-1);
		quickSort(S, nuPivot+1, right);
	}
}
void SortSaddles(Saddle **S, int n)
{
	quickSort(S, 0, n-1);
}
MS_Complex::MS_Complex(double  *points, int dimension, int count, int _k, bool perturb)
{
  perturbed = perturb;
	d = dimension-1;
	numKneighbors = _k;
	szV = numV = count;
	//szE = szV*numKneighbors;
  //Edges should be bi-directional
	szE = szV*szV;

	V = new Vertex *[szV];
	E = new KNN_Edge *[szE];

  srand(8);
	double *p = new double[dimension];
	for(int i=0; i < count; i++)
	{
    double eps = (double)rand() / (double)RAND_MAX;
    eps = eps *1e-6;
    if(rand() > RAND_MAX / 2)
      eps = -eps;

		for(int j = 0; j < dimension; j++)
			p[j] = points[i*dimension+j];
    p[dimension-1] += (perturbed ? eps : 0);
		V[i] = new Vertex(d, p, p[d], i);
	}	
	delete [] p;

//  std::cout << "numV=" << numV << std::endl;
//  std::cout << "d=" << d << std::endl;
//  std::cout << "numKneighbors=" << numKneighbors << std::endl;

	KNN();
	Compute();
  maxDist = -1;
}

MS_Complex::MS_Complex(MS_Complex &Complex, double  *new_point)
{
  perturbed = Complex.perturbed;
  d = Complex.d;
  numKneighbors = Complex.numKneighbors;
	szV = numV = Complex.numV + 1;
	//szE = szV*numKneighbors;
  szE = szV*szV;

	V = new Vertex *[szV];
	E = new KNN_Edge *[szE];
	
	double *p = new double[d+1];
	for(int i=0; i < Complex.numV; i++)
	{
		for(int j = 0; j < d; j++)
			p[j] = Complex.V[i]->GetXi(j);
    p[d] = Complex.V[i]->Value();
		V[i] = new Vertex(d, p, p[d], i);
	}	

  double eps = (double)rand() / (double)RAND_MAX;
  eps = eps * 1e-6;
  if(rand() > RAND_MAX / 2)
    eps = -eps;

  V[numV-1] = new Vertex(d, new_point, new_point[d] + (perturbed ? eps : 0), numV-1);
	delete [] p;
	KNN();
	Compute();
  maxDist = -1;
}

void MS_Complex::KNN()
{
	int eIndex = 0;
	int i,k;

	ANNpointArray pa = new ANNpoint[numV];
	for(i = 0; i < numV; i++)
	{
		pa[i] = NULL;
		V[i]->CreateANNpoint(pa[i]);
	}

	SearchStructure = new ANNkd_tree(pa,numV,d);
	ANNidxArray nn_idx;
	ANNdistArray dists;
	for(i = 0; i < numV; i++)
	{		
		nn_idx = new ANNidx[numKneighbors+1];
		dists = new ANNdist[numKneighbors+1];

		SearchStructure->annkSearch(pa[i],numKneighbors+1,nn_idx,dists);

		for(k=1;k<numKneighbors+1;k++)
		{
      if(!DoesEdgeExist(i, nn_idx[k], eIndex))
      {
			  E[eIndex] = new KNN_Edge(V[i],V[nn_idx[k]],E, eIndex);
			  eIndex++;
      }
      if(!DoesEdgeExist(nn_idx[k],i, eIndex))
      {
			  E[eIndex] = new KNN_Edge(V[nn_idx[k]],V[i],E, eIndex);
			  eIndex++;
      }
		}
		delete [] nn_idx;
		delete [] dists;
	}

	for(i = 0; i < numV; i++)
		delete [] pa[i];

	delete [] pa;
	numE = eIndex;
}

void MS_Complex::Compute()
{
  int i,j;
	int maxCount=0;
	int minCount=0;
	double globalMax = V[0]->Value();
	double globalMin = V[0]->Value();
	for(i = 0; i < numV; i++)
	{
		Vertex *v = V[i];

		if(globalMax < v->Value())
			globalMax = v->Value();
		if(globalMin > v->Value())
			globalMin = v->Value();

		//Vertex **neighbors = new Vertex *[numKneighbors];
    std::vector<Vertex *> neighbors;
		//for(int j = 0; j < numKneighbors; j++)
    j = 0;
    while(v->GetIthNeighbor(j,E) != -1)
		{
			//neighbors[j] = V[v->GetIthNeighbor(j,E)];
      neighbors.push_back(V[v->GetIthNeighbor(j,E)]);
      j++;
		}

		double maximum = v->Value();
		double minimum = v->Value();
		Vertex *steepestA = v;
		Vertex *steepestD = v;

		//for(int j = 0; j < numKneighbors; j++)
    j = 0;
    while(v->GetIthNeighbor(j,E) != -1)
		{
			Vertex *currentNeighbor = neighbors[j];

			if(currentNeighbor->Value() > maximum ||
				(currentNeighbor->Value() == maximum &&
				 currentNeighbor->ID > v->ID))
			{
				maximum = currentNeighbor->Value();
				steepestA = currentNeighbor;
			}

			if(currentNeighbor->Value() < minimum  ||
				(currentNeighbor->Value() == minimum &&
				 currentNeighbor->ID < v->ID))
			{
				minimum = currentNeighbor->Value();
				steepestD = currentNeighbor;
			}
      j++;
		}
		if(steepestA == v)
		{
			v->classification = LOCAL_MAX;
			maxCount++;
		}
		else if(steepestD == v)
		{
			v->classification = LOCAL_MIN;
			minCount++;
		}

		v->Union_Max(steepestA, V);
		v->Union_Min(steepestD, V);

		//delete [] neighbors;
	}

	//Go through each vertex, examine its neighbors, if maxima are different, create or update the entry in a map
	// where the key is the two maxima (always use lower maxima as first in pair), if the value of this point is
	// higher then update or create the entry with this point as the pseudo-saddle.
	//When we are done looking at each point, read off the saddles.
	std::map<std::pair<int, int>,int> maxSaddles = std::map<std::pair<int, int>,int>();
	for(i = 0; i < numV; i++)
	{
		if(V[i]->classification == LOCAL_MAX || V[i]->classification == LOCAL_MIN)
			continue;

		//Vertex **neighbors = new Vertex *[numKneighbors];
    std::vector<Vertex *> neighbors;
    j = 0;
    
		//for(int j = 0; j < numKneighbors; j++)
    while(V[i]->GetIthNeighbor(j,E) != -1)
		{
			//neighbors[j] = V[V[i]->GetIthNeighbor(j,E)];
      neighbors.push_back(V[V[i]->GetIthNeighbor(j,E)]);
      j++;
		}
		//for(int Bindex = 0; Bindex < numKneighbors; Bindex++)
    for(int Bindex = 0; Bindex < j; Bindex++)
		{
			int AmaxIndex = V[i]->Find_Max(V);
			int BmaxIndex = neighbors[Bindex]->Find_Max(V);

			if(AmaxIndex != BmaxIndex)
			{
				if(AmaxIndex > BmaxIndex)
				{
					int temp = AmaxIndex;
					AmaxIndex = BmaxIndex;
					BmaxIndex = temp;
				}
				std::pair<int,int> AB(AmaxIndex, BmaxIndex); 
				if(maxSaddles.find(AB) == maxSaddles.end() || V[maxSaddles[AB]]->Value() < V[i]->Value())
				{
					maxSaddles[AB] = i;
				}
			}
		}
		//delete [] neighbors;
	}
	//Saddles between Minima
	std::map<std::pair<int, int>, int> minSaddles = std::map<std::pair<int, int>, int>();
	for(i = 0; i < numV; i++)
	{
		if(V[i]->classification == LOCAL_MAX || V[i]->classification == LOCAL_MIN)
			continue;

		//Vertex **neighbors = new Vertex *[numKneighbors];
    std::vector<Vertex *> neighbors;
    j = 0;
		//for(int j = 0; j < numKneighbors; j++)
    while(V[i]->GetIthNeighbor(j,E) != -1)
		{
			//neighbors[j] = V[V[i]->GetIthNeighbor(j,E)];
      neighbors.push_back(V[V[i]->GetIthNeighbor(j,E)]);
      j++;
		}

		int Bindex = 0;
		//for(; Bindex < numKneighbors; Bindex++)
    for(; Bindex < j; Bindex++)
		{
			int AminIndex = V[i]->Find_Min(V);
			int BminIndex = neighbors[Bindex]->Find_Min(V);

			if(AminIndex != BminIndex)
			{
				if(AminIndex > BminIndex)
				{
					int temp = AminIndex;
					AminIndex = BminIndex;
					BminIndex = temp;
				}
				std::pair<int,int> AB(AminIndex, BminIndex); 
				if(minSaddles.find(AB) == minSaddles.end() || V[minSaddles[AB]]->Value() > V[i]->Value())
				{
					minSaddles[AB] = i;
				}
			}
		}
		//delete [] neighbors;
	}

	///////////////////////
  double eps = (globalMax-globalMin)*1e-7;
	int numSaddles = maxSaddles.size() + minSaddles.size();
	Saddle * *S = new Saddle*[numSaddles];

	std::map<std::pair<int,int>, int>::iterator sIter = maxSaddles.begin();
	//Create Saddles
	int curIdx = 0;
	for( ; sIter != maxSaddles.end(); sIter++)
	{
		S[curIdx] = new Saddle();
		S[curIdx]->idx = sIter->second;

		//Higher index breaks tie and gets to be more persistent
		if(V[sIter->first.first]->Value() < V[sIter->first.second]->Value() ||
			(V[sIter->first.first]->Value() == V[sIter->first.second]->Value() && sIter->first.first < sIter->first.second))
		{
			S[curIdx]->less = sIter->first.first;
			S[curIdx]->more = sIter->first.second;
		}
		else
		{
			S[curIdx]->less = sIter->first.second;
			S[curIdx]->more = sIter->first.first;
		}

    //We don't want zero persistence, so apply some small epsilon (epsilon
    // should depend on the range of values for this complex not arbitrarily
    // chosen as I have done here)
		S[curIdx]->persistence = std::max(eps,fabs(V[sIter->second]->Value() - V[S[curIdx]->less]->Value()));

		if(V[S[curIdx]->less]->persistence == 0 || V[S[curIdx]->less]->persistence > S[curIdx]->persistence)
			V[S[curIdx]->less]->persistence = S[curIdx]->persistence;

		curIdx++;
		V[sIter->second]->classification = SADDLE;
	}

	sIter = minSaddles.begin();
	for( ; sIter != minSaddles.end(); sIter++)
	{
		S[curIdx] = new Saddle();
		S[curIdx]->idx = sIter->second;

		//Higher index breaks tie and gets to be more persistent
		if(V[sIter->first.first]->Value() > V[sIter->first.second]->Value() ||
			(V[sIter->first.first]->Value() > V[sIter->first.second]->Value() && sIter->first.first < sIter->first.second))
		{
			S[curIdx]->less = sIter->first.first;
			S[curIdx]->more = sIter->first.second;
		}
		else
		{
			S[curIdx]->less = sIter->first.second;
			S[curIdx]->more = sIter->first.first;
		}

		S[curIdx]->persistence = std::max(eps,fabs(V[sIter->second]->Value() - V[S[curIdx]->less]->Value()));

		if(V[S[curIdx]->less]->persistence == 0 || V[S[curIdx]->less]->persistence > S[curIdx]->persistence)
			V[S[curIdx]->less]->persistence = S[curIdx]->persistence;

		curIdx++;
		V[sIter->second]->classification = SADDLE;
	}

//Do this at the end, where we actually look at the global min and max values
// as opposed to just reporting the extrema without persistence, this is wrong
// as an extrema's persistence may get updated when we cancel saddles
// This will incorrectly set an extremum's persistence and report the wrong
// global extrema.
//	for(int i = 0; i < numSaddles; i++)
//	{
//		if(V[S[i]->more]->persistence == 0)
//		{
//      printf("Range: %f\n", globalMax-globalMin);
//			V[S[i]->more]->persistence = globalMax-globalMin;
//			if(V[S[i]->more]->classification == LOCAL_MAX)
//			{
//				globalMaxIdx = S[i]->more;
//			}
//			else
//			{
//				globalMinIdx = S[i]->more;
//			}
//		}
//	}

	//Sort Saddles
	SortSaddles(S, numSaddles);
	Saddle *head = S[0];
	for(i = 0; i < numSaddles - 1; i++)
	{
		S[i]->next = S[i+1];
	}
	if(numSaddles > 0)
	  S[numSaddles - 1]->next = NULL;
	else
	  head = S[0] = NULL;

	std::map< std::pair<int, int>, std::vector<Vertex *> > Crystals;
	int cCount = 0;
	for(i = 0; i < numV; i++)
	{
		if(V[i]->classification == LOCAL_MIN || V[i]->classification == LOCAL_MAX)
			continue;

		int minI = V[i]->Find_Min(V);
		int maxI = V[i]->Find_Max(V);
		std::pair<int,int> minMaxPair = std::pair<int,int>(minI,maxI);
		if(Crystals.find(minMaxPair) == Crystals.end())
		{
			Crystals[minMaxPair] = std::vector<Vertex *>();
			Crystals[minMaxPair].push_back(V[i]);
		}
		else
		{
			Crystals[minMaxPair].push_back(V[i]);
		}
	}

	std::map<std::pair<int,int>, std::vector<Vertex *> >::iterator cIter = Crystals.begin();
	numC = 10000;
	std::vector<double> tempPersistencesList;
	tempPersistencesList.push_back(0.0);
	std::vector<MS_Crystal *> tempCrystalList;
	int nextCID = 0;
	while(cIter != Crystals.end())
	{
		int *vIds = new int[cIter->second.size()+2];
		for(j = 0; j < cIter->second.size(); j++)
		{
			vIds[j] = cIter->second[j]->ID;
		}
		vIds[j++] = cIter->first.first;
		vIds[j] = cIter->first.second;

		//C[nextCID] = new MS_Crystal(V,vIds, cIter->second.size()+2,d);
		tempCrystalList.push_back(new MS_Crystal(V,vIds, cIter->second.size()+2,d));

		cIter++;
		nextCID++;
	}
	//for( ; nextCID < numC; nextCID++)
	//	C[nextCID] = NULL;
	//nextCID = Crystals.size();

///PERSISTENCE Calculation
	p_levels = numSaddles;
	V_to_C = new int[numV*p_levels];//Stores the crystal id that a vertex belongs to at a given persistence level
	//Assign every vertex to its original Crystal for every persistence level, update when new crystal emerges
	for(i = 0; i < Crystals.size(); i++)
	{
		for(j = 0; j < tempCrystalList[i]->size(); j++)
		{
			int vIndex = tempCrystalList[i]->GetVertexID(j);
			for(int p = 0; p < p_levels; p++)
				V_to_C[vIndex*p_levels+p] = i;
		}
	}

	int p_level = 0;
	Saddle *current = head;
	while(current != NULL)
	{
		p_level++;
		tempPersistencesList.push_back(V[current->less]->persistence);
		int idxToRemove = current->less;
		int idxToReplace = current->more;
    V[idxToRemove]->persistence = 
    V[current->idx]->persistence = std::max(eps,fabs(V[current->less]->Value() - V[current->idx]->Value()));
    //printf("Combining %d and %d\n", current->idx, idxToRemove);

		//Find the rest of the saddles that have this extremum and delete them, then reenter them into the
		// queue with the new neighboring extremum
		Saddle *findIter = current->next;
		Saddle *trailingFind = current;
		while(findIter != NULL)
		{
			if(findIter->less == idxToRemove)
			{
				if(idxToReplace != findIter->more)
				{
					Saddle *nuSaddle = new Saddle();
					nuSaddle->idx = findIter->idx;
					if(V[idxToReplace]->classification == LOCAL_MAX)
					{
						if(V[idxToReplace]->Value() > V[findIter->more]->Value() ||
							(V[idxToReplace]->Value() == V[findIter->more]->Value() && idxToReplace > findIter->more))
						{
							nuSaddle->more = idxToReplace;
							nuSaddle->less = findIter->more;
						}
						else
						{
							nuSaddle->less = idxToReplace;
							nuSaddle->more = findIter->more;
						}
					}
					else
					{
						if(V[idxToReplace]->Value() < V[findIter->more]->Value() ||
							(V[idxToReplace]->Value() == V[findIter->more]->Value() && idxToReplace > findIter->more))
						{
							nuSaddle->more = idxToReplace;
							nuSaddle->less = findIter->more;
						}
						else
						{
							nuSaddle->less = idxToReplace;
							nuSaddle->more = findIter->more;
						}
					}
					nuSaddle->persistence = std::max(eps,fabs(V[nuSaddle->less]->Value() - V[nuSaddle->idx]->Value()));
					V[nuSaddle->less]->persistence = std::min(nuSaddle->persistence, V[nuSaddle->less]->persistence);

					//Remove the dead saddle
					trailingFind->next = findIter->next;	
					delete findIter;
					findIter = trailingFind->next;

					//Insert the next one (note we will always gain persistence, so we can start
					// looking to place it after the one we deleted)
					Saddle *insert = findIter;
					Saddle *trailingInsert = trailingFind;
					if(insert == NULL || nuSaddle->persistence < insert->persistence)
					{
						trailingInsert->next = nuSaddle;
						nuSaddle->next = insert;
						trailingFind = nuSaddle;
					}
					else
					{
						while(insert != NULL && nuSaddle->persistence > insert->persistence)
						{
							trailingInsert = insert;
							insert = insert->next;
						}
						nuSaddle->next = insert;
						trailingInsert->next = nuSaddle;
					}
				}
				else
				{
					//Remove the dead saddle
					trailingFind->next = findIter->next;	
					delete findIter;
					findIter = trailingFind->next;
				}
			}
			else if(findIter->more == idxToRemove)
			{
				if(idxToReplace != findIter->less)
				{
					Saddle *nuSaddle = new Saddle();
					nuSaddle->idx = findIter->idx;
					if(V[idxToReplace]->classification == LOCAL_MAX)
					{
						if(V[idxToReplace]->Value() > V[findIter->less]->Value() ||
							(V[idxToReplace]->Value() == V[findIter->less]->Value() && idxToReplace > findIter->less))
						{
							nuSaddle->more = idxToReplace;
							nuSaddle->less = findIter->less;
						}
						else
						{
							nuSaddle->more = idxToReplace;
							nuSaddle->less = findIter->less;
						}
					}
					else
					{
						if(V[idxToReplace]->Value() < V[findIter->less]->Value() ||
							(V[idxToReplace]->Value() == V[findIter->less]->Value() && idxToReplace > findIter->less))
						{
							nuSaddle->more = idxToReplace;
							nuSaddle->less = findIter->less;
						}
						else
						{
							nuSaddle->more = idxToReplace;
							nuSaddle->less = findIter->less;
						}
					}
					nuSaddle->persistence = std::max(eps,fabs(V[nuSaddle->less]->Value() - V[nuSaddle->idx]->Value()));
					V[nuSaddle->less]->persistence = std::min(nuSaddle->persistence, V[nuSaddle->less]->persistence);
				
					//Remove the dead saddle
					trailingFind->next = findIter->next;	
					delete findIter;
					findIter = trailingFind->next;

					//Insert the next one (note we will always gain persistence, so we can start
					// looking to place it after the one we deleted)
					Saddle *insert = findIter;
					Saddle *trailingInsert = trailingFind;
					if(insert == NULL || nuSaddle->persistence < insert->persistence)
					{
						trailingInsert->next = nuSaddle;
						nuSaddle->next = insert;
						trailingFind = nuSaddle;
					}
					else
					{
						while(insert != NULL && nuSaddle->persistence > insert->persistence)
						{
							trailingInsert = insert;
							insert = insert->next;
						}
						nuSaddle->next = insert;
						trailingInsert->next = nuSaddle;
					}
				}
				else
				{
					//Remove the dead saddle
					trailingFind->next = findIter->next;	
					delete findIter;
					findIter = trailingFind->next;
				}
			}
			else
			{
				trailingFind = findIter;
				findIter = findIter->next;
			}
		}
		//Do the crystal additions here
		int totalCrystals = tempCrystalList.size();
		for(i = 0; i < totalCrystals; i++)
		{
			if (tempCrystalList[i] == NULL || !tempCrystalList[i]->Exists(p_level-1))
				continue;

			bool success = false;
			if(V[idxToRemove]->classification == LOCAL_MAX && tempCrystalList[i]->MaxV() == idxToRemove)
			{
				for(j = 0; j < totalCrystals; j++)
				{
					if (tempCrystalList[j] == NULL || !tempCrystalList[j]->Exists(p_level-1) || i == j)
						continue;

					if(tempCrystalList[j]->MaxV() == idxToReplace && tempCrystalList[i]->MinV() == tempCrystalList[j]->MinV())
					{
						//C[nextCID] = new MS_Crystal(C[i], C[j], true, p_level);
						MS_Crystal *nuCrystal = new MS_Crystal(tempCrystalList[i], tempCrystalList[j], true, p_level); 
						tempCrystalList.push_back(nuCrystal);
						for(int k = 0; k < nuCrystal->size(); k++)
						{
							int vIndex = nuCrystal->GetVertexID(k);
							for(int p = p_level; p < p_levels; p++)
								V_to_C[vIndex*p_levels+p] = nextCID;
						}
						nextCID++;
						success = true;
						break;
					}
				}
				if(!success) 
				{
					//C[nextCID] = new MS_Crystal(C[i],true,idxToReplace,p_level);
					MS_Crystal *nuCrystal = new MS_Crystal(tempCrystalList[i],true,idxToReplace,p_level);
					tempCrystalList.push_back(nuCrystal);
					for(int k = 0; k < nuCrystal->size(); k++)
					{
						int vIndex = nuCrystal->GetVertexID(k);
						for(int p = p_level; p < p_levels; p++)
							V_to_C[vIndex*p_levels+p] = nextCID;
					}
					nextCID++;
				}
			}
			else if(V[idxToRemove]->classification == LOCAL_MIN && tempCrystalList[i]->MinV() == idxToRemove)
			{
				for(j = 0; j < totalCrystals; j++)
				{
					if (tempCrystalList[j] == NULL || !tempCrystalList[j]->Exists(p_level-1) || i == j)
						continue;

					if(tempCrystalList[j]->MinV() == idxToReplace && tempCrystalList[i]->MaxV() == tempCrystalList[j]->MaxV())
					{
						//C[nextCID] = new MS_Crystal(C[i], C[j], false, p_level);
						MS_Crystal *nuCrystal = new MS_Crystal(tempCrystalList[i], tempCrystalList[j], false, p_level);
						tempCrystalList.push_back(nuCrystal);
						for(int k = 0; k < nuCrystal->size(); k++)
						{
							int vIndex = nuCrystal->GetVertexID(k);
							for(int p = p_level; p < p_levels; p++)
								V_to_C[vIndex*p_levels+p] = nextCID;
						}
						nextCID++;
						success = true;
						break;
					}
				}
				if(!success) 
				{
					//C[nextCID] = new MS_Crystal(C[i],false,idxToReplace,p_level);
					MS_Crystal *nuCrystal = new MS_Crystal(tempCrystalList[i],false,idxToReplace,p_level);
					tempCrystalList.push_back(nuCrystal);
					for(int k = 0; k < nuCrystal->size(); k++)
					{
						int vIndex = nuCrystal->GetVertexID(k);
						for(int p = p_level; p < p_levels; p++)
							V_to_C[vIndex*p_levels+p] = nextCID;
					}
					nextCID++;
				}
			}
		}
		Saddle *trash = current;
		current = current->next;
		delete trash;
	}
	numC= nextCID;
	szP = tempPersistencesList.size();
	persistences = new double[szP];
	C = new MS_Crystal *[tempCrystalList.size()];
	std::vector<MS_Crystal *>::iterator mscIter = tempCrystalList.begin();
	for(int i = 0; i < tempCrystalList.size(); i++)
	{
		C[i] = tempCrystalList[i];
	}
	for(int i = 0; i < tempPersistencesList.size(); i++)
	{
		persistences[i] = tempPersistencesList[i];
	}

  globalMinIdx = 0;
  globalMaxIdx = 0;
  for(i = 0; i < numV; i++)
  {
    if(V[i]->classification == LOCAL_MIN && V[i]->Value() < V[globalMinIdx]->Value())
      globalMinIdx = i;
    else if(V[i]->classification == LOCAL_MAX && V[i]->Value() > V[globalMaxIdx]->Value())
      globalMaxIdx = i;
  }

  for(i = 0; i < numV; i++)
  {
    if((V[i]->classification == LOCAL_MIN || V[i]->classification == LOCAL_MAX) && V[i]->persistence == 0 )
    {
      if(i == globalMinIdx || i == globalMaxIdx)
        V[i]->persistence = V[globalMaxIdx]->Value() - V[globalMinIdx]->Value();
      else
      {
        std::cerr << "ERROR: There exists an extrema with zero persistence!\n\tVertex " 
                  << i << "\n\tClassification = " << V[i]->classification 
                  << "\n\tValue = " << V[i]->Value() << std::endl;
//                  << "\nPrinting points for reconstruction to badMSC.txt" << std::endl;
//        FILE *badMSC =  fopen("badMSC.txt", "w");
//        for(int i = 0; i < numV; i++)
//        {
//          fprintf(badMSC, "%f\t%f\t%f\n", V[i]->GetXi(0), V[i]->GetXi(1), V[i]->Value());
//        }
//        fclose(badMSC);
      }
    }
  }

	delete [] S;
}

double MS_Complex::CompareExtremaCount(MS_Complex &_C)
{
	int maxNumV = _C.numV;
	int minNumV = numV;

	int countThisMin = 0;
	int countThatMin = 0;

	int countThisMax = 0;
	int countThatMax = 0;

	int countThisSaddle = 0;
	int countThatSaddle = 0;

	int _ci = 0;
	for(int i = 0; i < maxNumV; i++, _ci++)
	{
		if(i < numV)
		{
			if(V[i]->classification == 0)
				countThisMin++;
			else if(V[i]->classification == 1)
				countThisMax++;
			else if(V[i]->classification == 2)
				countThisSaddle++;
		}

		if(i < _C.numV)
		{
			if(_C.V[i]->classification == 0)
				countThatMin++;
			else if(_C.V[i]->classification == 1)
				countThatMax++;
			else if(_C.V[i]->classification == 2)
				countThatSaddle++;
		}
		
	}

	return abs(countThisMin - countThatMin + countThisMax - countThatMax + countThisSaddle - countThatSaddle);
}

double MS_Complex::CompareClassChange(MS_Complex &_C)
{
	int maxNumV = _C.numV;
	int minNumV = numV;

	int countChanged = 0;

	int _ci = 0;
	for(int i = 0; i < minNumV; i++, _ci++)
	{
		if(V[i]->classification != _C.V[_ci]->classification)
			countChanged++;
	}

	return countChanged;
}

double MS_Complex::ComparePersistence(MS_Complex &_C)
{
	int maxNumV = _C.numV;
	int minNumV = numV;

	int countChanged = 0;

	int _ci = 0;
	double sumP = 0.0;
	for(int i = 0; i < minNumV; i++, _ci++)
	{
		sumP += fabs(V[i]->persistence - _C.V[_ci]->persistence);
	}

	return (sumP)/(double)(minNumV);
}

double MS_Complex::ComparePersistenceNoSaddles(MS_Complex &_C)
{
	int maxNumV = _C.numV;
	int minNumV = numV;

	int countChanged = 0;

	double sumP = 0.0;
	for(int i = 0; i < minNumV; i++)
	{
		if(V[i]->classification == SADDLE || _C.V[i]->classification == SADDLE)
			continue;
		sumP += fabs(V[i]->persistence - _C.V[i]->persistence);
	}

	return (sumP)/(double)(minNumV);
}


double MS_Complex::CompareBottleneck(MS_Complex &_C)
{
  PersistenceDiagram<double> pDia(1);
  for(int i = 0; i < numV; i++)
  {
    if(V[i]->classification == LOCAL_MIN && i != globalMinIdx)
    {
        double birth = V[i]->Value();
        double death = V[i]->Value() + V[i]->persistence;
        PDPoint<double> nu(birth, death);
        //std::cout  << "MIN: " << nu << std::endl;
        pDia.push_back(nu);
    }
    else if(V[i]->classification == LOCAL_MAX  && i != globalMaxIdx)
    {
        double birth = V[i]->Value() - V[i]->persistence;
        double death = V[i]->Value();
        PDPoint<double> nu(birth, death);
        //std::cout << "MAX: " << nu << std::endl;
        pDia.push_back(nu);
    }
  }
  PDPoint<double> globalMinMax(V[globalMinIdx]->Value(), V[globalMaxIdx]->Value());
  pDia.push_back(globalMinMax);
  PersistenceDiagram<double> pDia2(1);
  for(int i = 0; i < _C.numV; i++)
  {
    if(_C.V[i]->classification == LOCAL_MIN && i != _C.globalMinIdx)
    {
        double birth = _C.V[i]->Value();
        double death = _C.V[i]->Value() + _C.V[i]->persistence;
        PDPoint<double> nu(birth, death);
        //std::cout  << "MIN: " << nu << std::endl;
        pDia2.push_back(nu);
    }
    else if(_C.V[i]->classification == LOCAL_MAX  && i != _C.globalMaxIdx)
    {
        double birth = _C.V[i]->Value() - _C.V[i]->persistence;
        double death = _C.V[i]->Value();
        PDPoint<double> nu(birth, death);
        //std::cout << "MAX: " << nu << std::endl;
        pDia2.push_back(nu);
    }
  }
  PDPoint<double> globalMinMax2(_C.V[_C.globalMinIdx]->Value(), _C.V[_C.globalMaxIdx]->Value());
  pDia.push_back(globalMinMax2);
  //std::cout  << "SIZES: " << pDia.size() << " " << pDia2.size() << std::endl;
  double ret_value = bottleneck_distance(pDia, pDia2);
  return ret_value;
}

double MS_Complex::CompareLabels(MS_Complex &_C, double p)
{
  int diff_labels = 0;
  double testPersistence = p*persistences[szP-1];
  int p_level = 0;

  while(persistences[p_level] < testPersistence && p_level < szP-1)
  {
    p_level++;
  }

  //double testPersistence2 = p*_C.persistences[_C.szP-1];
  int p_level2 = 0;
  while(_C.persistences[p_level2] < testPersistence && p_level2 < _C.szP-1)
  {
    p_level2++;
  }
  
  for(int i = 0; i < numV && i < _C.numV; i++)
  {
    int idx1 = V_to_C[i*p_levels+p_level];
    int idx2 = _C.V_to_C[i*_C.p_levels+p_level2];

    if(idx1 != idx2)
      diff_labels++;
    //if(idx1 < numC && idx2 < _C.numC)
    //  if(C[idx1]->MaxV() != _C.C[idx2]->MaxV() || C[idx1]->MinV() != _C.C[idx2]->MinV())
    //    diff_labels++;
  }

  return diff_labels;
}

double MS_Complex::CloseToExtrema(double *v, double p)
{
  bool first = true;
  double minsDistance;
  double testPersistence = p*persistences[szP-1];
  for(int i = 0; i < numV; i++)
  {
    if(V[i]->classification == REGULAR || V[i]->persistence < testPersistence)
      continue;

      double sDistance = 0.0;
      for(int j = 0; j < d; j++)
        sDistance += (V[i]->GetXi(j)-v[j])*(V[i]->GetXi(j)-v[j]);

      if(first || minsDistance > sDistance)
      {
        first = false;
	minsDistance = sDistance;
      }
  }	
  if(minsDistance == 0)	//obviously don't want to sample the same point twice
    return 0;	
  return 1/(minsDistance);
}

double MS_Complex::FarFromExtrema(double *v, double p)
{
  bool first = true;
  double minsDistance;
  double testPersistence = p*persistences[szP-1];
  int p_level = 0;

  for(int i = 0; i < numV; i++)
  {
    if(V[i]->classification == REGULAR || V[i]->classification == SADDLE || V[i]->persistence < testPersistence)
      continue;

    double sDistance = 0.0;
    for(int j = 0; j < d; j++)
      sDistance += (V[i]->GetXi(j)-v[j])*(V[i]->GetXi(j)-v[j]);

    if(first || minsDistance > sDistance)
    {
      first = false;
      minsDistance = sDistance;
    }

  }		
  return minsDistance;
}

double MS_Complex::FarFromCP(double *v, double p)
{
  bool first = true;
  double minsDistance;
  double testPersistence = p*persistences[szP-1];
  int p_level = 0;

  for(int i = 0; i < numV; i++)
  {
    if(V[i]->classification == REGULAR || V[i]->persistence < testPersistence)
      continue;

    double sDistance = 0.0;
    for(int j = 0; j < d; j++)
      sDistance += (V[i]->GetXi(j)-v[j])*(V[i]->GetXi(j)-v[j]);

    if(first || minsDistance > sDistance)
    {
      first = false;
      minsDistance = sDistance;
    }

  }		
  return minsDistance;
}

double MS_Complex::FarFromSaddle(double *v, double p)
{
  bool first = true;
  double minsDistance;
  double testPersistence = p*persistences[szP-1];
  int p_level = 0;

  for(int i = 0; i < numV; i++)
  {
    if(V[i]->classification == REGULAR || V[i]->classification == LOCAL_MIN || V[i]->classification == LOCAL_MAX || V[i]->persistence < testPersistence)
      continue;

    double sDistance = 0.0;
    for(int j = 0; j < d; j++)
      sDistance += (V[i]->GetXi(j)-v[j])*(V[i]->GetXi(j)-v[j]);

    if(first || minsDistance > sDistance)
    {
      first = false;
      minsDistance = sDistance;
    }

  }		
  return minsDistance;
}

MS_Complex::~MS_Complex()
{
	for(int i = 0; i < numV; i++)
		delete V[i];
	delete [] V;
	for(int i = 0; i < numE; i++)
		delete E[i];
	delete [] E;
	for(int i = 0; i < numC; i++)
		delete C[i];
	delete [] C;
	
	delete [] V_to_C;
  delete [] persistences;
	delete SearchStructure;
}

void MS_Complex::Destroy()
{
	for(int i = 0; i < numV; i++)
		delete V[i];
	delete [] V;
	for(int i = 0; i < numE; i++)
		delete E[i];
	delete [] E;
	for(int i = 0; i < numC; i++)
		delete C[i];
	delete [] C;
	
	delete [] V_to_C;
  delete [] persistences;
	delete SearchStructure;
}

int MS_Complex::GetIthHighestPersistence(int i)
{
  int numCPs = 0;
  for(int j = 0; j < numV; j++)
  {
    if(V[j]->classification != REGULAR)
      numCPs++;
  }
  if(numCPs <= i)
  {
    return -1;
  }
  Saddle **AllExtrema = new Saddle *[numCPs];

  int currentIdx = 0;
  for(int j = 0; j < numV && currentIdx < numCPs; j++)
  {
    if(V[j]->classification != REGULAR)
    {
      AllExtrema[currentIdx] = new Saddle();
      AllExtrema[currentIdx]->idx = j;
      AllExtrema[currentIdx]->persistence = V[j]->persistence;
      AllExtrema[currentIdx]->more = -1;
      AllExtrema[currentIdx]->less = -1;
      AllExtrema[currentIdx]->next = NULL;
      currentIdx++;
    }
  }

  SortSaddles(AllExtrema, numCPs);

  int retValue;
  if((numCPs-1) >= i && i >= 0)
    retValue = AllExtrema[(numCPs-1)-i]->idx;
  else
    retValue = -1;

  delete [] AllExtrema;
  return retValue;
}

int MS_Complex::GetIthHighestPersistenceSaddle(int i)
{
  int numS = 0;

  for(int j = 0; j < numV; j++)
    if(V[j]->classification == SADDLE)
      numS++;

  if(numS <= i)
  {
    return -1;
  }

  Saddle **AllExtrema = new Saddle *[numS];

  int currentIdx = 0;
  for(int j = 0; j < numV && currentIdx < numS; j++)
  {
    if(V[j]->classification != REGULAR)
    {
      AllExtrema[currentIdx] = new Saddle();
      AllExtrema[currentIdx]->idx = j;
      AllExtrema[currentIdx]->persistence = V[j]->persistence;
      AllExtrema[currentIdx]->more = -1;
      AllExtrema[currentIdx]->less = -1;
      AllExtrema[currentIdx]->next = NULL;
      currentIdx++;
    }
  }

  SortSaddles(AllExtrema, numS);
  return AllExtrema[numS-i]->idx;
}

bool   MS_Complex::IsCritical(int i)
{
  int j = i;
  return  j < numV && j >= 0 && V[j]->classification != REGULAR;
}

bool   MS_Complex::IsExtrema(int i)
{
  int j = i;
  return j < numV && j >= 0 && (V[j]->classification == LOCAL_MIN || V[j]->classification == LOCAL_MAX);
}

bool   MS_Complex::IsSaddle(int i)
{
  int j = i;
  return j < numV && j >= 0 && (V[j]->classification == SADDLE);
}

Vertex *   MS_Complex::GetVertex(int i)
{
  int j = i;
  return (j < numV && j >= 0) ? V[j] : NULL;
}

double ScoreTOPOB(MS_Complex &C1, double *x)
{
  MS_Complex C2 = MS_Complex(C1,x);
  return C1.CompareBottleneck(C2);
}

double ScoreTOPOP(MS_Complex &C1, double *x)
{
  MS_Complex C2 = MS_Complex(C1,x);
  return C1.ComparePersistenceNoSaddles(C2);
}

std::vector<int> ScoreTOPOHP(int dimension, int knn,
  double *training, double *trainingY, int n_training, 
  double *candidates, double *candidateY, int n_candidates)
{
  double *p = new double[(dimension+1)*(n_training+n_candidates)];
  for(int i = 0; i < n_training; i++)
  {
    for(int j = 0; j < dimension; j++)
    {
      p[i*(dimension+1)+j] = training[i*dimension+j];
    }
    p[i*(dimension+1)+dimension] = trainingY[i];
  }
  for(int i = 0; i < n_candidates; i++)
  {
    int ioffset = i+n_training;
    for(int j = 0; j < dimension; j++)
    {
      p[ioffset*(dimension+1)+j] = candidates[i*dimension+j];
    }
    p[ioffset*(dimension+1)+dimension] = candidateY[i];
  }
  MS_Complex Complex(p, dimension+1, n_training+n_candidates, knn);
//  std::cout << "DAN AMSC:\n";
//  Complex.Print(std::cout);

  std::vector<int> returnOrder;

  int count = 0;
  for(int i = 0; i < n_candidates; i++)
  {
    int tempIdx = Complex.GetIthHighestPersistence(count++) - n_training;

    //If we don't find any CPs, then just sort them arbitrarily
    // for simplicity, I am returning points of zero persistence (regular)
    // in order they were given.
    if(tempIdx+n_training == -1)
      break;

    //Probably overshooting on the iterations, but should not hurt to do so,
    // we won't add it if it is invalid anyway
    while(tempIdx < 0 && count < Complex.numV)
      tempIdx = Complex.GetIthHighestPersistence(count++) - n_training;

    //Conditional indicates we actually found something useful above
    // and didn't just run out of rope
    if(count < Complex.numV)
      returnOrder.push_back(tempIdx);
  }

  for(int i = 0; i < n_candidates; i++)
  {
    bool found = false;
    for(int j = 0; j < returnOrder.size(); j++)
    {
      if(i == returnOrder[j])
      {
        found = true;
        break;
      }
    }
    if(!found)
      returnOrder.push_back(i);
  }

  delete p;
  return returnOrder;
}

void MS_Complex::Print(std::ostream &out)
{
  out << "Minima: " << std::endl << "\t";
  for(int i = 0; i < numV; i++)
  {
    if(V[i]->classification == LOCAL_MIN)
    {
      for(int j = 0; j < d; j++)
        out << V[i]->GetXi(j) << ' ';
      out << V[i]->Value() << " (p=" << V[i]->persistence << ")" << std::endl << "\t";
    }
  }
  out << std::endl;
  out << "Maxima: " << std::endl;
  for(int i = 0; i < numV; i++)
  {
    if(V[i]->classification == LOCAL_MAX)
    {
      for(int j = 0; j < d; j++)
        out << V[i]->GetXi(j) << ' ';
      out << V[i]->Value() << " (p=" << V[i]->persistence << ")" << std::endl << "\t";
    }
  }
  out << std::endl;
  out << "Saddles: " << std::endl;
  for(int i = 0; i < numV; i++)
  {
    if(V[i]->classification == SADDLE)
    {
      for(int j = 0; j < d; j++)
        out << V[i]->GetXi(j) << ' ';
      out << V[i]->Value() << " (p=" << V[i]->persistence << ")" << std::endl << "\t";
    }
  }
  out << std::endl;
}

int MS_Complex::CountExtrema(double p)
{
  double testPersistence = p*persistences[szP-1];
  int count = 0;

  for(int i = 0; i < numV; i++)
  {
    if((V[i]->classification == LOCAL_MAX || V[i]->classification == LOCAL_MIN) && V[i]->persistence >= testPersistence)
      count++;
  }
  return count;
}
int MS_Complex::CountMaxima(double p)
{
  double testPersistence = p*persistences[szP-1];
  int count = 0;

  for(int i = 0; i < numV; i++)
  {
    if(V[i]->classification == LOCAL_MAX && V[i]->persistence >= testPersistence)
      count++;
  }
  return count;
}

int MS_Complex::CountMinima(double p)
{
  double testPersistence = p*persistences[szP-1];
  int count = 0;

  for(int i = 0; i < numV; i++)
  {
    if(V[i]->classification == LOCAL_MIN && V[i]->persistence >= testPersistence)
      count++;
  }
  return count;
}
int MS_Complex::CountSaddles(double p)
{
  double testPersistence = p*persistences[szP-1];
  int count = 0;

  for(int i = 0; i < numV; i++)
  {
    if(V[i]->classification == SADDLE && V[i]->persistence >= testPersistence)
      count++;
  }
  return count;
}

#ifdef USING_GL
void MS_Complex::Draw(double gMin, double gMax,bool flatMode)
{
	if(d != 2)
		return;

	glLineWidth(1.0);
	glBegin(GL_LINES);
	for(int i = 0; i < numE; i++)
	{
		glColor3f(0.25,0.25,0.25);
		glVertex3f(V[E[i]->start]->GetXi(0),V[E[i]->start]->GetXi(1), flatMode ? 0 : ((V[E[i]->start]->Value()-gMin)/(gMax-gMin) - 1./2.));
		glVertex3f(V[E[i]->end]->GetXi(0),  V[E[i]->end]->GetXi(1),  flatMode ? 0 : ((V[E[i]->end]->Value()-gMin)/(gMax-gMin) - 1./2.));
	}
	glEnd();

	for(int i = 0; i < numV; i++)
	{
		Vertex *curV = V[i];
		if(curV->classification == 0)
			glColor3f(0,0.5,1);
		else if(curV->classification == 1)
			glColor3f(1,0,0);
		else if(curV->classification == 2)
			glColor3f(0,1,0);
		else
			glColor3f(0,0,0);

		glTranslatef(curV->GetXi(0),curV->GetXi(1), flatMode ? 0 : ((curV->Value()-gMin)/(gMax-gMin) - 1./2.));
			glutSolidSphere(curV->classification == 3 ? 0.005 : 0.01,16,16);
		glTranslatef(-curV->GetXi(0),-curV->GetXi(1), -(flatMode ? 0 : ((curV->Value()-gMin)/(gMax-gMin) - 1./2.)));
  }
	for(int i = 0; i < numV; i++)
	{
		Vertex *curV = V[i];
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
		glColor3f(0,0,0);
		glLineWidth(6.0);
		for(int k = 0; k < numKneighbors; k++)
		{
			int nextIdx = curV->GetIthNeighbor(k, E);
			Vertex *nextV = V[nextIdx];
			if(nextIdx == curV->NeighborMax() || nextIdx == curV->NeighborMin())
			{
				glBegin(GL_LINES);
					if(i == nextV->NeighborMax())
						glColor4f(1,0,0,1);
					else if(i == nextV->NeighborMin())
						glColor4f(0.8,1.,1,1);
					else
						glColor4f(0,0,0,0);

					glVertex3f(curV->GetXi(0),curV->GetXi(1), flatMode ? 0 : ((curV->Value()-gMin)/(gMax-gMin) - 1./2.));
					if(nextIdx == curV->NeighborMax())
						glColor4f(1,0,0,1);
					else if(nextIdx == curV->NeighborMin())
						glColor4f(0.8,1.,1,1);
					else
						glColor4f(0,0,0,0);

					glVertex3f(nextV->GetXi(0),nextV->GetXi(1), flatMode ? 0 : ((nextV->Value()-gMin)/(gMax-gMin) - 1./2.));
				glEnd();
			}
		}
    glEnable(GL_LIGHTING);
	}
	glLineWidth(1.0);
  glDisable(GL_BLEND);
}

void MS_Complex::DrawBurst()
{
  double maxSdist = 0;
  if(maxDist < 0)
  {
    for(int vi = 0; vi < numV; vi++)
    {
      for(int vj = vi+1; vj < numV; vj++)
      {
        double sdist = V[vi]->SDistance(V[vj]);
        if(sdist > maxSdist)
          maxSdist = sdist;
      }
    }
    maxDist = sqrt(maxSdist);
  }

  double factor = M_PI/(2.*maxDist);

  double gMin = V[globalMinIdx]->Value();
  double gMax = V[globalMaxIdx]->Value();

  std::vector<int> mins;
  std::vector<int> maxs;
  std::vector<int> saddles;
  for(int i = 0; i < numV; i++)
  {
    if(V[i]->classification == LOCAL_MIN)
      mins.push_back(i);
    if(V[i]->classification == LOCAL_MAX)
      maxs.push_back(i);
    if(V[i]->classification == SADDLE)
      saddles.push_back(i);
  }

  double x = 0;
  double y = 0;
  for(int i = 0; i < mins.size(); i++)
  {
    int midx = mins[i];

    x = (double)(i+1)/(double)(mins.size()+1);
    y = 0.5*((V[midx]->Value() - gMin)/(gMax-gMin)) - 0.5;

    glColor3f(0,0,1);
    glTranslatef( x,  0.5, y);
      glutSolidSphere(0.005,16,16);
    glTranslatef(-x, -0.5, -y);

    glColor3f(0,0,0);
    for(int j = 0; j < numV; j++)
    {
      if(V[j]->Find_Min(V) == midx && midx != j)
      {
        glBegin(GL_LINES);
          glVertex3f(x,0.5,y);
          double radius = 0.5*(V[j]->Value() - V[midx]->Value())/(gMax-gMin);
          double dist = sqrt(V[midx]->SDistance(V[j]));
          double theta = dist*factor + M_PI/2.;
          double xtemp = x + radius*cos(theta);
          double ytemp = y + radius*sin(theta);
          glVertex3f(xtemp, 0.5, ytemp);
        glEnd();
        glTranslatef(xtemp, 0.5, ytemp);
          glutSolidSphere(0.0025,16,16);
        glTranslatef(-xtemp, -0.5, -ytemp);
      }
    }
  }
  for(int i = 0; i < maxs.size(); i++)
  {
    int midx = maxs[i];

    x = (double)(i+1)/(double)(maxs.size()+1);
    y = 0.5*((V[midx]->Value() - gMin)/(gMax-gMin));

    glColor3f(1,0,0);
    glTranslatef( x,  0.5, y);
      glutSolidSphere(0.005,16,16);
    glTranslatef(-x, -0.5, -y);

    glColor3f(0,0,0);
    for(int j = 0; j < numV; j++)
    {
      if(V[j]->Find_Max(V) == midx && midx != j)
      {
        glBegin(GL_LINES);
          glVertex3f(x,0.5,y);
          double radius = 0.5*(V[midx]->Value() - V[j]->Value())/(gMax-gMin);
          double dist = sqrt(V[midx]->SDistance(V[j]));
          double theta = dist*factor + M_PI/2.;
          double xtemp = x + radius*cos(theta);
          double ytemp = y - radius*sin(theta);
          glVertex3f(xtemp, 0.5, ytemp);
        glEnd();
        glTranslatef(xtemp, 0.5, ytemp);
          glutSolidSphere(0.0025,16,16);
        glTranslatef(-xtemp, -0.5, -ytemp);
      }
    }
  }

}
#endif
