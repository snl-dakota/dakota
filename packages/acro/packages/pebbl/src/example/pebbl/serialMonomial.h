/*  _________________________________________________________________________
 *
 *  PICO: A C++ library of scalable branch-and-bound and related methods.
 *  Copyright (c) 2001, Sandia National Laboratories.
 *  This software is distributed under the GNU Lesser General Public License.
 *  For more information, see the README file in the top PICO directory.
 *  _________________________________________________________________________
 */

/**
 * \file serialMonomial.h
 * \author Noam Goldberg
 *
 *  Example class to use object-oriented branching framework.
 *  Solves the maximum monomial problem
 */

#ifndef pebbl_monomial_h
#define pebbl_monomial_h
#include <vector>
#include <cmath>
#include <algorithm>
#include <set>
#include <iostream>
#include <stdexcept>
#include <functional>

#include <acro_config.h>

#include <utilib/_math.h>
#include <utilib/ParameterSet.h>
#include <pebbl/branching.h>

#ifdef ACRO_HAVE_MPI
#include <utilib/PackBuf.h>
#endif


using namespace std;
using namespace pebbl;


namespace pebblMonom {

  typedef int size_type;
  const size_type NULL_IDX = -1;
  const double MAX_BOUND = 1.0;
  const double EPS = 1e-8;


  typedef vector<bool> observation_t;


  class  observationEntry_t 
  {
  public:

  observationEntry_t(const size_type rowId, const observation_t &ob) : 
    idx(rowId), obs(ob) {}
    size_type idx;
    observation_t obs;
  };

  typedef unsigned char variable_val_t;
  const variable_val_t NULL_VAL              = 0;
  const variable_val_t NOT_IN_MONOM          = 1;
  const variable_val_t IN_MONOM              = 2;
  const variable_val_t COMPLEMENTED_IN_MONOM = 3;


  // monomialObj - object representing a Monomial of Boolean variables

  class monomialObj // object representing a monomial
  {
    
  public:

  monomialObj(const monomialObj & o) : 
    _vars(o._vars), 
      _notInMonom(o._notInMonom), 
      _varIdxs(o._varIdxs), 
      _lastSetIdx(o._lastSetIdx) {}

  monomialObj(size_type varNum = 0, 
	      const variable_val_t initVal = NULL_VAL) : 
    _vars(varNum, initVal), 
      _lastSetIdx(NULL_IDX) {}

    monomialObj(const vector<size_type> & varIdx, 
		const vector<size_type> & compVarIdx, 
		size_type varNum);

    //finalize into a solution
    void finalize() 
    {
      for(int i=0; i < _vars.size(); i++) 
	if (_vars[i]==NULL_VAL) 
	  _vars[i]=NOT_IN_MONOM;
    }

    size_type highestIdx() const 
    {
      if (_varIdxs.empty() && _notInMonom.empty()) 
	return NULL_IDX; 
      else if (_notInMonom.empty()) 
	return *_varIdxs.rbegin(); 
      else if (_varIdxs.empty()) 
	return *_notInMonom.rbegin(); 
      else return max(*_varIdxs.rbegin(),*_notInMonom.rbegin());
    }

    size_type highestMonomIdx() const 
    {
      if (_varIdxs.empty()) 
	return NULL_IDX; 
      return *_varIdxs.rbegin();
    }

    // need to implement compare, read and write operations
    void appendVar(const size_type idx) throw (exception) 
    {
      if (idx < 0 || idx >= maxDegree()) 
	throw out_of_range("idx out of range in append");
      _vars[idx]=IN_MONOM; 
      _varIdxs.insert(_varIdxs.end(),idx);
      _lastSetIdx=idx;
    }

    void appendCompVar(const size_type idx) throw (exception) 
    {
      if (idx < 0 || idx >= maxDegree()) 
	throw out_of_range("idx out of range in append c");
      _vars[idx]=COMPLEMENTED_IN_MONOM;
      _varIdxs.insert(_varIdxs.end(),idx);
      _lastSetIdx=idx;
    }

    void setOutOfMonom(const size_type idx) throw (exception) 
    {
      if (idx < 0 || idx >= maxDegree()) 
	throw out_of_range("idx out of range in append c"); 
      _vars[idx]=NOT_IN_MONOM; 
      _notInMonom.insert(idx);_lastSetIdx=idx;
    }

    bool evaluatePoint(const observation_t boolVector) const throw (exception);

    size_type degree()    const {return _varIdxs.size();}
    size_type maxDegree() const {return _vars.size();}

    variable_val_t getVarVal(size_type idx) const 
    {
      if (idx < 0 || idx >= maxDegree()) 
	return NULL_VAL; 
      return _vars[idx];
    }

    const set<size_type> & getVarIdxs() const {return _varIdxs;}

    void remove(const size_type idx) throw (exception) 
    {
      if (idx < 0 || idx >= maxDegree()) 
	throw invalid_argument("idx out of range in remove");
      _varIdxs.erase(idx);
      _vars[idx] = NOT_IN_MONOM;
    }

    size_type lastSetIdx() const {return _lastSetIdx;}
    
    bool operator==(const monomialObj& obj) 
    {
      return (_vars==obj._vars && _lastSetIdx==obj._lastSetIdx);
    }

    monomialObj& operator=(const monomialObj &o) 
      {
	_vars       = o._vars; 
	_notInMonom = o._notInMonom; 
	_varIdxs    = o._varIdxs; 
	_lastSetIdx = o._lastSetIdx; 
	return *this;
      }

    const set<size_type> & getNotInMonom() const {return _notInMonom;}

    void printInMonom(std::ostream& os) const;
    void printNotInMonom(std::ostream& os) const;

#ifdef ACRO_HAVE_MPI
    void pack(PackBuffer & buff) const;
    void unpack(UnPackBuffer &buff);
#endif

    bool covers(const vector<variable_val_t> target) const;

  private:
    vector<variable_val_t> _vars; // variable values
    set<size_type> _notInMonom;
    // set of variable indices either in monomial or complemented in monomial
    set<size_type> _varIdxs;
    size_type _lastSetIdx;

  };


  // JE auxiliary classes for choosing branching variables

  class branchItem
  {
  public:
    double roundedBound;
    double exactBound;
    int    whichChild;
    int    arrayPosition;

  branchItem() :
    roundedBound(1.0),
      exactBound(1.0),
      whichChild(-1),
      arrayPosition(-1)
	{ };

  branchItem(branchItem& toCopy) :
    roundedBound(toCopy.roundedBound),
      exactBound(toCopy.exactBound),
      whichChild(toCopy.whichChild),
      arrayPosition(toCopy.arrayPosition)
	{ };

    void set(double v) 
    {
      roundedBound  = v;       // Assume already rounded
      exactBound    = v;
      whichChild    = -1;
      arrayPosition = -1;
    };

    void set(double bound,
	     double roundQuantum,
	     int    whichChild_,
	     int    arrayPosition_)
    {
      exactBound = bound;
      if (roundQuantum == 0)
	roundedBound = bound;
      else
	roundedBound = floor(bound/roundQuantum + 0.5)*roundQuantum;
      whichChild    = whichChild_;
      arrayPosition = arrayPosition_;
    };
  };


  class branchChoice 
  {

  public:

    // Data members

    // Note that exactBound is not needed in serial, but saves 
    // an entire broadcast in parallel

    branchItem branch[3];
    int        branchVar;

  branchChoice() :
    branchVar(MAXINT)
    {
      for (int i=0; i<3; i++)
	branch[i].set(MAXDOUBLE);
    };

    branchChoice(double a, double b, double c, int j)
      {
	branch[0].set(a);
	branch[1].set(b);
	branch[2].set(c);
	branchVar = j;
      };

    // Primitive sort, but only three elements

    void sortBounds()
    {
      possibleSwap(0,1);
      possibleSwap(0,2);
      possibleSwap(1,2);
    };

    bool operator<(const branchChoice& other) const
    {
      for(int i=0; i<3; i++)
	{
	  if (branch[i].roundedBound < other.branch[i].roundedBound)
	    return true;
	  else if (branch[i].roundedBound > other.branch[i].roundedBound)
	    return false;
	}
      return branchVar < other.branchVar;
    }

#ifdef ACRO_HAVE_MPI

    static void setupMPI();
    static void freeMPI();

    static MPI_Datatype mpiType;
    static MPI_Op       mpiCombiner;

  protected:

    static void setupMPIDatum(void*         address,
			      MPI_Datatype  thisType,
			      MPI_Datatype* type,
			      MPI_Aint      base,
			      MPI_Aint*     disp,
			      int*          blocklen,
			      int           j);
#endif

  protected:

    void possibleSwap(size_type i1,size_type i2)
    {
      register double roundedBound1 = branch[i1].roundedBound;
      register double roundedBound2 = branch[i2].roundedBound;
      if (roundedBound1 < roundedBound2)
	{
	  branchItem tempItem(branch[i1]);
	  branch[i1] = branch[i2];
	  branch[i2] = tempItem;
	}
    }
	  
  };


#ifdef ACRO_HAVE_MPI

  void branchChoiceCombiner(void*         invec, 
			    void*         inoutvec, 
			    int*          len, 
			    MPI_Datatype* datatype);

#endif


  //  The branching class

  class maxMonomialData : virtual public branching
  {
  public:

    maxMonomialData();
                
    virtual ~maxMonomialData();

    // get data file name to initialize data matrix
    bool setupProblem(int& argc,char**& argv);

    // write data to a file, including weights, to a file that 
    // can be read by setupProble (added by JE)
    void writeWeightedData(ostream& os);
    void writeInstanceToFile(int iterNum);

    void preprocess();

    branchSub* blankSub();

    // returns the positive observations covered by monomial
    void getPosCovg(const monomialObj &, 
		    set<size_type> & output) const throw (exception);

    void getNegCovg(const monomialObj &, 
		    set<size_type> & output) const throw (exception);

    void getCovg(const monomialObj &, 
		 set<size_type> & output) const throw (exception);

    // returns the positive observations covered by monomial
    void getPosCovgFast(const monomialObj &, 
			set<size_type> & output) const throw (exception);

    void getNegCovgFast(const monomialObj &, 
			set<size_type> & output) const throw (exception);

    const observation_t& getObservation(const unsigned int i) 
      const throw (exception) 
      {
#ifdef ACRO_VALIDATING
	if (i >= _dataStore.size()) 
	  throw invalid_argument("invalid idx"); 
#endif
	return _dataStore[i];
      }

    const monomialObj getNthMonomChild(const monomialObj &parent, 
				       const size_type n) 
      const throw (exception);

    double getWeight(const set<size_type> & obsIdxs) const throw (exception);

    double getWeight(size_type idx) const 
    {
      return _wts[idx];
    }

    virtual void setWeight(const double vec[], size_type len);

    size_type attribNum() const {return _attribNum;}
    size_type obsNum()    const {return _obsNum;}

    bool positive(const observation_t & obs) const 
    {
      return obs[attribNum()];
    }

    bool positive(size_type idx) const 
    {
      return (_dataStore[idx])[attribNum()];
    }

    void getFilteredCovg(const monomialObj &, 
			 const set<size_type> & filter, 
			 set<size_type> & output) const throw (exception);

    const set<size_type> & getPositives() const {return _posIdx;}
    const set<size_type> & getNegatives() const {return _negIdx;}

    // obsLess comparator for sorted observations, 
    // use "radix-like sorting" later
    class obsLess : 
    public std::binary_function<const observation_t &,
      const observation_t &,
      bool> 
	{
	public:
	  // comparator created given list of vars not in monom
	obsLess(const list<size_type> & ls) : _ls(ls) {}
	obsLess() : _ls() {}
	  bool operator()(const observationEntry_t &o1, 
			  const observationEntry_t &o2) const; 
	private:
	  list<size_type> _ls;
	};

    void getFilteredObsSorted(const vector<size_type> & ignoreVarIdxs, 
			      const set<size_type> & filterObsIdxs, 
			      multiset<observationEntry_t, 
			      obsLess> &obs) const;

    void getPosObsSorted(const vector<size_type> & ignoreIdxs, 
			 multiset<observationEntry_t, 
			 obsLess> &obs) const 
    {
      getFilteredObsSorted(ignoreIdxs,_posIdx, obs);
    }

    void getNegObsSorted(const vector<size_type> & ignoreIdxs, 
			 multiset<observationEntry_t, 
			 obsLess> &obs) const 
    {
      getFilteredObsSorted(ignoreIdxs,_negIdx, obs);
    }

    void getIntersectionWithSortedObs(const observationEntry_t & obs, 
			    const multiset<observationEntry_t,obsLess> & data,
				      set<size_type> & ret);
      
    const set<size_type> & getPosVarCovg(size_type idx, 
					 variable_val_t val) 
      const throw (exception) 
    {
      assert(val!=NOT_IN_MONOM && idx>=0 && idx < _attribNum); 
      if (val==IN_MONOM) 
	return _varPosCovg[idx]; 
      else 
	return _compVarPosCovg[idx];
    }
     
    const set<size_type> & getNegVarCovg(size_type idx, 
					 variable_val_t val) 
      const throw (exception) 
    {
      assert(val!=NOT_IN_MONOM && idx >=0 && idx < _attribNum);
      if (val==IN_MONOM) 
	return _varNegCovg[idx]; 
      else return _compVarNegCovg[idx];
    }
     
    bool getImprovedBound() const {return _improvedBound;}
    void setImprovedBound(bool improved) {_improvedBound=improved;}
    bool getLowNodeMemory() const {return !_saveCoverages;}

#ifdef ACRO_USING_COIN_CBC
    int getIterations() const {return _iterations;}
#endif

    bool threeWayBranching()    const {return _threeWayBranching;}
    double getBranchingFactor() const {return _branchingFactor;}
    bool writingInstances()     const {return _writeInstances;}
    bool weightsReadIn()        const {return _nonUniformWt;}
    double getRoundingQuantum() const {return _roundingQuantum;}
 
    virtual bool verifyLog() {return _verifyLog;}

    ostream& verifyLogFile() { return *_vlFile; };

    virtual ostream* openVerifyLogFile()
    {
      return new ofstream("verify00000.mmaLog",ios::out);
    }

    void startVerifyLogIfNeeded();

    void setThreeWayBranching(bool threeWay) 
    {
      _threeWayBranching=threeWay;
      _branchingFactor=0;
    }

    void setBranchingFactor(double k) 
    {
      _branchingFactor=k; 
      _threeWayBranching=false;
    }

    bool watching() const { return _watching; };
    const vector<variable_val_t>& watchVector() const { return _watchVector; };

  protected:
    void removeIdxs(const list<size_type> &notInMonom, 
		    observationEntry_t & obs) const;

    size_type getDistantVar(const monomialObj &monom, 
			    size_type fromLeftObs, size_type fromRightObs, 
			    bool positive) const;

    void calculateVarCovg();

    vector<observation_t> _dataStore;

    bool _nonUniformWt;

    // vector of weights corresponding to the observations in _dataStore]
    vector<double> _wts;

    set<size_type> _posIdx;
    set<size_type> _negIdx;

    vector<set<size_type> > _varPosCovg;
    vector<set<size_type> > _varNegCovg;
    vector<set<size_type> > _compVarPosCovg;
    vector<set<size_type> > _compVarNegCovg;

    size_type _attribNum;
    size_type _obsNum;
    bool _improvedBound;
    bool _saveCoverages;
    bool _threeWayBranching;
    bool _writeInstances;
    double _branchingFactor;
    double _roundingQuantum;
    bool _verifyLog;
    ostream* _vlFile;

    std::string _watchString;
    bool        _watching;
    vector<variable_val_t> _watchVector;
    

#ifdef ACRO_USING_COIN_CBC
    int _iterations;
#endif

  }; // maxMonomialData


  class maxMonomSolution : public solution
  {
  public:

    maxMonomSolution(maxMonomSolution * toCopy);

    maxMonomSolution(maxMonomialData* global);

    maxMonomSolution(const monomialObj & monom);

    maxMonomSolution(size_type numVars_);

    solution* blankClone();

    virtual const char* typeDescription() const 
    { 
      return "Max Monomial Solution"; 
    };

    virtual void printContents(std::ostream& s);

    void copy(maxMonomSolution* toCopy);

    ~maxMonomSolution() { };

    const monomialObj & getMonomialObj() const {return _monom;}

    size_type highestIdx() const {return _monom.highestIdx();}

#ifdef ACRO_HAVE_MPI
    void packContents(PackBuffer & outBuf);

    void unpackContents(UnPackBuffer & inBuf);

    int maxContentsBufSize();
#endif

  protected:

    virtual ::size_type sequenceLength();
    virtual double sequenceData();
    virtual void sequenceReset();
    monomialObj _monom;
    set<size_type>::const_iterator _seqIter;
  }; // maxMonomSolution


  //  For use in the constructor compiled code, and also in derived
  //  parallel classes
#ifdef  BRANCHING_CONSTRUCTOR
#undef  BRANCHING_CONSTRUCTOR
#endif
#define BRANCHING_CONSTRUCTOR				\
  branching(maximization,relTolerance(),absTolerance())


  //  The branchSub class
  class maxMonomSub : virtual public branchSub
  {

  public:

    struct eqvClassEntry
    {
    eqvClassEntry() : posWt(0), negWt(0) {}
      double posWt;
      double negWt;
      set<size_type> posObs;
      set<size_type> negObs;
      void clear() {posWt=0; negWt=0;posObs.clear();negObs.clear();}
    };

    typedef vector<eqvClassEntry> eqvClassVec;
 
  maxMonomSub() :
    _assocSoln(NULL), 
      _posCovg(0), 
      _negCovg(0), 
      _insepWt(0) 
	{};

    virtual ~maxMonomSub();

    maxMonomialData* global() const {return globalPtr;};

    branching* bGlobal() const {return global();};

    REFER_DEBUG(global())

    virtual void maxMonomSubFromData(maxMonomialData* master);

    // JE added suppliedBound argument for a case that arises when the
    // bound of the child is already known and we don't want to waste
    // time recomputing it.  The value -1.0 means that the bound is
    // unknown and has to be computed.

    virtual void maxMonomSubAsChildOf(maxMonomSub* parent,
				      int          whichChild,
				      double       suppliedBound = -2.0);

  private:
    virtual void initChild(maxMonomSub* parent,int whichChild) 
      throw (std::exception);

  public:

    // pure virtual inherited from branchSub
    virtual void setRootComputation();

    // pure virtual inherited from branchSub
    void boundComputation(double* controlParam); 

    void getIntersectionWithSortedObs(const observationEntry_t & obs, 
				      const list<size_type> & ignoreVars, 
				      const vector<observationEntry_t> & data,
				      set<size_type> & ret);

    // pure virtual inherited from branchSub
    solution* extractSolution() 
    {
      return new maxMonomSolution(_assocSoln);
    }
  
    // pure virtual inherited from branchSub
    virtual int splitComputation();

    //makeChild
    virtual branchSub* makeChild(int whichChild);

    // Whether a solution is available
    bool candidateSolution() 
    {
      return (_assocSoln!=NULL);
    }

    const monomialObj & getMonomialObj() const {return _monom;}

    const set<size_type> & getPosCovgIdx() const {return _posCovgIdx;}
    const set<size_type> & getNegCovgIdx() const {return _negCovgIdx;}
 
    variable_val_t lastSetValue() const 
    {
      return _monom.getVarVal(_monom.lastSetIdx());
    }

    void setGlobalInfo(maxMonomialData* glbl) {globalPtr = glbl;}  

  protected:

    double findMostNonSeparating(const vector<size_type> & outOfMonom, 
				 const vector<size_type> & freeVariables, 
				 vector<size_type> & ret, 
				 vector<pair<double,double> > & insepWts) 
      throw (exception);
 
    virtual maxMonomSub * allocateObject() const 
    {
      DEBUGPR(20,ucout << "allocating maxMonomSub child" << endl);
      return new maxMonomSub();
    }  

    // sets the attribute _insepWt
    void calculateInsepWeight(const vector<size_type> & outOfMonomList);

    // const, does not set _insepWt
    double calculateInsepWeight(const vector<size_type> & outOfMonomList, 
				const set<size_type> &posCovgIdx,
				const set<size_type> &negCovgIdx, 
				set<size_type> &insepPos, 
				set<size_type> & insepNeg, 
				bool returnNonSep = true) const;

    double calculateInsepWeight(const vector<size_type> & outOfMonomList, 
				const set<size_type> &posCovgIdx,
				const set<size_type> &negCovgIdx, 
				set<size_type> & insepPos, 
				set<size_type> & insepNeg, 
				bool returnInsep, 
				bool returnInsepEqvClasses, 
				eqvClassVec & insepEqvClasses) const;

    // calculate insep weight given eqv classes and variable 
    // added to monomial product
    void calculateInsepWeight(const eqvClassVec & eqvClasses, 
			      const size_type varIdx, 
			      pair<double,double> & insepWts) const;

    int getFutureChildrenNumber() const 
    {
      return 2*(global()->attribNum()-1-_assocSoln->highestIdx());
    }
   
    double getObjectiveVal() const {return abs(_posCovg-_negCovg);}

    void vlWriteDescription();

    maxMonomialData* globalPtr;

    // not NULL if there is a solution associated to the subproblem
    maxMonomSolution* _assocSoln;

    double _posCovg;
    double _negCovg;

    set<size_type> _posCovgIdx;
    set<size_type> _negCovgIdx;

    struct maxMonomSubComp : binary_function<branchSub *, branchSub *, bool>
      {
	bool operator() (branchSub *s1, branchSub *s2) const
	{ return (s2->bound < s1->bound); }
      }; 

    struct maxMonomSubEqComp : binary_function<maxMonomSub *, maxMonomSub *, bool>
      {
	bool operator() (maxMonomSub *s1, maxMonomSub *s2) const
	{ return s1->_monom == s2->_monom;  }
      }; 
 
    typedef vector<maxMonomSub *> childrenVecType; 

    childrenVecType _children;
    monomialObj     _monom;
    double          _insepWt;

  };  // maxMonomSub


  //  The branchSubTreeWay class
  class maxMonomSubThreeWay : public maxMonomSub
  {
  public:

  maxMonomSubThreeWay() : maxMonomSub() { };

  void boundComputation(double* controlParam);

  size_type getBranchVar() const { return _branchChoice.branchVar; };

  protected:
    virtual maxMonomSub * allocateObject() const 
    {
      DEBUGPR(20,ucout << "allocating maxMonomSubThreeWay child" << endl);
      return new maxMonomSubThreeWay();
    }

    void tryThreeWaySplit(size_type i);
    void boundComputationCleanUp(double* controlParam);

    branchChoice _branchChoice;
  };


  //  The branchSubKWay class
  class maxMonomSubKWay : public maxMonomSub
  {
  public:
  maxMonomSubKWay() : maxMonomSub() {};
    void boundComputation(double* controlParam);
    void maxMonomSubAsChildOf(maxMonomSubKWay* parent,
			      int whichChild, 
			      const vector<size_type> & outOfMonomList, 
			      double insepWt);
    void maxMonomSubAsChildOf(maxMonomSubKWay* parent, 
			      const vector<size_type> & outOfMonomList, 
			      double insepWt);
    void maxMonomSubFromData(maxMonomialData* master);
  };


  // Now we have enough information to define...
  inline branchSub* maxMonomialData::blankSub()
  {
    maxMonomSub *temp;
    if (threeWayBranching())
      temp = new maxMonomSubThreeWay();
    else if (getBranchingFactor())
      temp = new maxMonomSubKWay();
    else
      temp = new maxMonomSub();
    temp->maxMonomSubFromData(this);
    return temp;
  };

} // namespace pebbl

std::ostream& operator<<(std::ostream& os, 
			 const pebblMonom::monomialObj & obj);

std::ostream& operator<<(std::ostream& os, 
			 const pebblMonom::observationEntry_t& obj);

std::ostream& operator<<(std::ostream& os, 
			 const pebblMonom::observation_t& obj);

std::ostream& operator<<(std::ostream& os, pebblMonom::branchChoice& bc);


inline bool 
pebblMonom::maxMonomialData::obsLess::operator()(const pebblMonom::observationEntry_t &o1, 
						 const pebblMonom::observationEntry_t &o2) 
const 
{  
  assert(o1.obs.size() == o2.obs.size());
 
  if (!_ls.empty())
    {
      pebblMonom::observationEntry_t one = o1;
      pebblMonom::observationEntry_t two = o2;
      list<size_type>::const_iterator iter;
      for (iter = _ls.begin(); iter != _ls.end(); iter++)
	{
	  one.obs[*iter]=false;
	  two.obs[*iter]=false;
	}
	      
      return (one.obs < two.obs);
    }
  else
    {
      return (o1.obs < o2.obs);
    }
}

#endif
