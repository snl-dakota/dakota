/*  _________________________________________________________________________
 *
 *  PICO: A C++ library of scalable branch-and-bound and related methods.
 *  Copyright (c) 2001, Sandia National Laboratories.
 *  This software is distributed under the GNU Lesser General Public License.
 *  For more information, see the README file in the top PICO directory.
 *  _________________________________________________________________________
 */
//
// Maximum Monomial
// serialMonomial.cpp
// Noam Goldberg 9/4/2007


#include <acro_config.h>
#include <utilib/exception_mngr.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <algorithm>


#include "serialMonomial.h"
//#define __DEBUG


using namespace std;


//  operator to write a monomial to stream
std::ostream& operator<<(std::ostream& os, const pebblMonom::monomialObj& obj)
{
  obj.printInMonom(os);
  os << ' ';
  obj.printNotInMonom(os);
  return os;
}


void pebblMonom::monomialObj::printInMonom(std::ostream& os) const
{
  set<size_type>::const_iterator iter;
  for (iter = _varIdxs.begin(); iter != _varIdxs.end(); iter++)
    {
      if (getVarVal(*iter) == IN_MONOM)
	{
	  os << " x" << *iter;
	}
      else if (getVarVal(*iter) == COMPLEMENTED_IN_MONOM)
	{
	  os << " ~x" << *iter;
	}
    }
}


void pebblMonom::monomialObj::printNotInMonom(std::ostream& os) const
{  
  os << "(";
  for (set<size_type>::const_iterator iter = getNotInMonom().begin(); 
       iter != getNotInMonom().end(); 
       iter++)
    {
      if (getVarVal(*iter) == NOT_IN_MONOM)
  	os << " x" << *iter;
    }
  os << ")";
}


bool pebblMonom::monomialObj::covers(const vector<variable_val_t> target) const
{
  set<pebblMonom::size_type>::const_iterator iter;
  for (iter = _varIdxs.begin(); iter != _varIdxs.end(); iter++)
    {
      size_type idx = *iter;
      if (_vars[idx] == IN_MONOM)
	{
	  if (target[idx] != IN_MONOM)
	    return false;
	}
      else if (_vars[idx] == COMPLEMENTED_IN_MONOM)
	{
	  if (target[idx] != COMPLEMENTED_IN_MONOM)
	    return false;
	}
    }
  for (iter = getNotInMonom().begin(); iter != getNotInMonom().end(); iter++)
    {
      size_type idx = *iter;
      if ((_vars[idx] == NOT_IN_MONOM) && (target[idx] != NULL_VAL))
	return false;
    }
  return true;
}


std::ostream& operator<<(std::ostream& os, 
			 const pebblMonom::observation_t& obj)
{
  copy (obj.begin(), obj.end(), ostream_iterator<bool> (os, " "));
  return os;
}

std::ostream& operator<<(std::ostream& os, 
			 const pebblMonom::observationEntry_t& obj)
{
  os << " obs: " << obj.idx << " data: ";
  os << obj.obs;
  os << endl;
  return os;
}

std::ostream& operator<<(std::ostream& os, pebblMonom::branchChoice& bc)
    {
      os << '(' << bc.branch[0].roundedBound << ',' 
	 << bc.branch[1].roundedBound << ',' 
	 << bc.branch[2].roundedBound
  	 << ")-(" << bc.branch[0].whichChild << ',' 
	 << bc.branch[1].whichChild << ','
	 << bc.branch[2].whichChild 
	 << ")-<" << bc.branchVar << '>';
      return os;
    }

using namespace std;
using namespace pebbl;

namespace pebblMonom {


#ifdef ACRO_HAVE_MPI

  void branchChoiceCombiner(void*         invec, 
			    void*         inoutvec, 
			    int*          len, 
			    MPI_Datatype* datatype)
  {
#ifdef ACRO_VALIDATING
    if (*datatype != branchChoice::mpiType)
      {
	cerr << "Datatype error in branchChoiceCombiner\n";
	exit(1);
      }
#endif
    branchChoice* inPtr    = (branchChoice*) invec;
    branchChoice* inOutPtr = (branchChoice*) inoutvec;
    int n = *len;
    for (int i=0; i<n; i++)
      if (inPtr[i] < inOutPtr[i])
	inOutPtr[i] = inPtr[i];
  }


  void branchChoice::setupMPIDatum(void*         address,
				   MPI_Datatype  thisType,
				   MPI_Datatype* type,
				   MPI_Aint      base,
				   MPI_Aint*     disp,
				   int*          blocklen,
				   int           j)
  {
    MPI_Address(address,&(disp[j]));
    disp[j] -= base;
    type[j] = MPI_DOUBLE;
    blocklen[j] = 1;
  }
 

  void branchChoice::setupMPI()
  {
    int arraySize = 4*3 + 1;

    MPI_Datatype type[arraySize];
    int          blocklen[arraySize];
    MPI_Aint     disp[arraySize];
    MPI_Aint     base;
    branchChoice example;

    MPI_Address(&example,&base);

    int j = 0;
    for (int i=0; i<3; i++)
      {
	setupMPIDatum(&(example.branch[i].roundedBound),MPI_DOUBLE,
		      type,base,disp,blocklen,j++);
	setupMPIDatum(&(example.branch[i].exactBound),MPI_DOUBLE,
		      type,base,disp,blocklen,j++);
	setupMPIDatum(&(example.branch[i].whichChild),MPI_INT,
		      type,base,disp,blocklen,j++);
	setupMPIDatum(&(example.branch[i].arrayPosition),MPI_INT,
		      type,base,disp,blocklen,j++);
      }
    setupMPIDatum(&(example.branchVar),MPI_INT,
		  type,base,disp,blocklen,j++);
    MPI_Type_struct(j,blocklen,disp,type,&mpiType);
    MPI_Type_commit(&mpiType);
    MPI_Op_create(branchChoiceCombiner,true,&mpiCombiner);
  }


  void branchChoice::freeMPI()
  {
    MPI_Op_free(&mpiCombiner);
    MPI_Type_free(&mpiType);
  };


  MPI_Datatype branchChoice::mpiType     = MPI_UB;
  MPI_Op       branchChoice::mpiCombiner = MPI_OP_NULL;

#endif


  // monomialObj::monomialObj - CTOR with vector of uncomplemented indices 
  // and vector of complemented indices

  monomialObj::monomialObj(const vector<size_type> & varIdx, 
			   const vector<size_type> & compVarIdx, 
			   size_type varNum) : 
    _vars(varNum, NULL_VAL)
  { 
#ifdef __DEBUG2
    cout << "monomialObj::monomialObj invoked with variable vectors" << endl;
#endif

    vector<size_type>::const_iterator iter;

    for (iter = varIdx.begin(); iter != varIdx.end(); iter++)
      {
	if (*iter < 0 || *iter >= varNum)
	  throw out_of_range("variable index out of range");
	_vars[*iter] = IN_MONOM;
	// insert into set at the end assuming our input vector is sorted
	_varIdxs.insert(_varIdxs.end(),*iter);
      }

    for (iter = compVarIdx.begin(); iter != compVarIdx.end(); iter++)
      {
	if (*iter < 0 || *iter >= varNum)
	  throw out_of_range("complemented variable index out of range");
	_vars[*iter] = COMPLEMENTED_IN_MONOM;
	_varIdxs.insert(*iter);
      }

#ifdef __DEBUG2
    cout << "Out of monomialObj::monomialObj" << endl;
#endif
  }

  //monomialObj::evaluatePoint
  bool monomialObj::evaluatePoint(const observation_t boolVector) 
    const throw (exception)
  {
#ifdef __DEBUG2
    cout << "monomialObj::evaluatePoint invoked" << endl;
    cout.flush();
#endif
    if (highestIdx() >= (size_type) boolVector.size())
      {
	cerr << "ERR, boolVector.size(): " << boolVector.size() 
	     << " monom: " << *this << " highestIdx(): " 
	     << highestIdx() << endl;
	throw out_of_range("monomial highest idx larger than boolVector");
      }
    set<size_type>::const_iterator iter;

    for (iter = _varIdxs.begin(); iter != _varIdxs.end(); iter++)
      {
	if (_vars[*iter]==IN_MONOM && boolVector[*iter] == false)
	  return false;
	else if (_vars[*iter]==COMPLEMENTED_IN_MONOM 
		 && boolVector[*iter] == true)
	  return false;
      }
#ifdef __DEBUG2
    cout << "Out of monomialObj::evaluatePoint" << endl;
    cout.flush();
#endif
    return true;
  }

#ifdef ACRO_HAVE_MPI

  void monomialObj::pack(PackBuffer & outBuff) const
  {
    outBuff << _lastSetIdx << _vars; 
  }

  void monomialObj::unpack(UnPackBuffer & inBuff) 
  {
    inBuff >> _lastSetIdx >> _vars;
    assert (!_vars.empty());
     
    for(size_type i = 0; i < _vars.size(); i++)
      {
	if (_vars[i] == pebblMonom::NOT_IN_MONOM)
	  _notInMonom.insert(_notInMonom.end(),i);
	else if (_vars[i] == pebblMonom::IN_MONOM 
		 || _vars[i] == pebblMonom::COMPLEMENTED_IN_MONOM)
	  _varIdxs.insert(_varIdxs.end(),i);
      }   
  }
#endif

  ////////////////////// maxMonomialData object //////////////////////


  //  Constructors for branching class
  maxMonomialData::maxMonomialData() :
    _nonUniformWt(false),
    _attribNum(0),
    _obsNum(0),
    _improvedBound(true),
    _saveCoverages(false),
    _threeWayBranching(true),
    _roundingQuantum(1e-5),
    _verifyLog(false),
    _vlFile(NULL),
#ifdef ACRO_USING_COIN_CBC
    _iterations(1),
#endif
    _branchingFactor(0)
  {
    min_num_required_args = 1;
    branchingInit(maximization, relTolerance, absTolerance);

    create_categorized_parameter("improvedBound",
				 _improvedBound,
				 "<bool>",
				 "true",
				 "Use improved upper bound function",
				 "Maximum Monomial");

    create_categorized_parameter("saveCoverages",
				 _saveCoverages,
				 "<bool>",
				 "false",
				 "Use O(features + observations) memory "
				 "per search node\n\t"
				 "in an attempt to reduce run time. "
				 "Otherwise, memory\n\tper node is O(features)",
				 "Maximum Monomial");

    create_categorized_parameter("threeWayBranching",
				 _threeWayBranching,
				 "<bool>",
				 "true",
				 "Ternary branching on single features.  "
				 "If false and kBranching\n\tis not set, "
				 "then branching is on all features in "
				 "fixed order",
				 "Maximum Monomial");

#ifdef ACRO_USING_COIN_CBC
    create_categorized_parameter("iterations",
				 _iterations,
				 "<int>",
				 "1",
				 "Number of LP-boosting iterations to run.  "
				 "Each iteration runs\n\ta full branch and "
				 "bound with different observation weights",
				 "Maximum Monomial",
				 utilib::ParameterLowerBound<int>(1));
#endif

    create_categorized_parameter("kBranching",
				 _branchingFactor,
				 "<double>",
				 "1",
				 "Fraction k of 'free' variables F "
				 "considered for branching at each node.\n\t"
				 "Resulting branching factor is 2k|F|+1; "
				 "only used if threeWayBranching\n\tis false",
				 "Maximum Monomial",
				 utilib::ParameterBounds<double>(0.0, 1.0));

    create_categorized_parameter("writeInstances",
				 _writeInstances,
				 "<bool>",
				 "false",
				 "Write an input file for each weighted "
				 "problem solved",
				 "Maximum Monomial");

    create_categorized_parameter("watchSolution",
				 _watchString,
				 "<string>",
				 "",
				 "Solution to watch for: example x3~x12x17",
				 "Maximum Monomial");
 
   create_categorized_parameter("roundingQuantum",
				 _roundingQuantum,
				 "<double>",
				 "1e-5",
				 "Bound rounding granularity for lexical "
				 "strong branching.\n\t"
				 "A value of 0 indicates no rounding.",
				 "Maximum Monomial",
				 utilib::ParameterBounds<double>(0.0, 0.01));

    create_categorized_parameter("mmaVerify",
				 _verifyLog,
				 "<bool>",
				 "false",
				 "Write application-specific verification "
				 "log(s)",
				 "Maximum Monomial");
  };


  //  To read in the problem.
  bool maxMonomialData::setupProblem(int& argc,char**& argv)
  {
    DEBUGPR(5,ucout << "maxMonomialData::setupProblem invoked. "
	    "_improvedBound: " << _improvedBound << endl);
    if (argc <= 1)
      {
	cerr << "No filename specified\n";
	return false;
      }

    try
      {
	ifstream f(argv[1]);
	if (!f)
	  throw domain_error("cannot open file");

	_obsNum = 0;    // reset number of observations
	_attribNum = 0; // reset number of attributes
	_nonUniformWt = false;

	while(!f.eof())
	  {
	    string line;
	    getline(f, line, '\n');

	    if (line.empty())
	      break;  // ignore because line is empty except for carriage return
	  
	    if (_attribNum == 0) // if 0 it must be the first line read
	      {
		string::size_type delim = 0;
		if ((delim = line.find(";")) != string::npos)
		  {
		    DEBUGPR(5,ucout << " nonUniform weights found " << endl);
		    _nonUniformWt = true;
		    // subtract class variable and another one to get idx
		    _attribNum = line.size()-(delim+1)-1; 
		  }
		else
		  {
		    DEBUGPR(5,ucout << "uniform weights are assumed" << endl);

		    _attribNum = line.size()-1; // exclude class variable
		  }
	      }
 
	    int delim = -1;
	    if (_nonUniformWt)
	      {
		delim = line.find(";");
		if (delim == string::npos)
		  throw domain_error("inconsistent row without delim ;");
		_wts.push_back( (double) atof((line.substr(0,delim)).c_str()));
	      }
	      
	    if (line.size()-delim-2 != _attribNum)
	      {
		cerr << line.size() << " " << _attribNum 
		     << " " << line << " " << " delim: " << delim << " ";
		throw domain_error("data file format format: "
				   "inconsistent line size");
	      }
	    observation_t row(_attribNum + 1);
	    for (int i = delim+1; i < line.size(); i++)
	      {
		switch (line[i])
		  {
		  case '1':
		    row[i-delim-1] = true;
		    break;
		  case '0':
		    row[i-delim-1] = false;
		    break;
		  default:
		    throw domain_error("invalid character in file");
		  }
	      }
	    assert(row.size()==_attribNum+1);
	    _dataStore.push_back(row);
	    observationEntry_t obsE(_obsNum++, row);
	    DEBUGPR(20,ucout << obsE << endl);
	    if (!_wts.empty())
	      DEBUGPR(20,ucout <<" wt: " << *_wts.rbegin() << endl);

	    assert(row.size()==_attribNum+1);
	    assert(obsE.obs.size()==_attribNum+1);
	  }

	// if no weights read from file, initialize uniform weights
	if (!_nonUniformWt)
	  _wts.insert(_wts.end(), _obsNum, 1.0/((double)_obsNum));

	f.close();
	calculateVarCovg();

	DEBUGPR(10,ucout << "maxMonomialData::setupProblem complete, "
		"_attribNum:"  << _attribNum << " _obsNum: " 
		<< _obsNum << endl);

	_watching = false;
	const char* cp = _watchString.c_str();
	if (*cp)
	  {
	    _watchVector.resize(_attribNum);
	    for(size_type i=0; i<_attribNum; i++)
	      _watchVector[i] = NULL_VAL;
	    bool gotSomething = false;
	    size_type index = 0;
	    bool compFlag = false;
	    while (true)
	      {
		const char c = *(cp++);
		if (c >= '0' && c <= '9')
		  {
		    index *= 10;
		    index += (c - '0');
		    gotSomething = true;
		    _watching = true;
		  }
		else
		  {
		    if (gotSomething)
		      {
			if (compFlag)
			  _watchVector[index] = COMPLEMENTED_IN_MONOM;
			else
			  _watchVector[index] = IN_MONOM;
			gotSomething = false;
			index = 0;
			compFlag = false;
		      }
		    if (c == 0)
		      break;
		    if (c == '~')
		      compFlag = !compFlag;
		    else if (c != 'x' && c != 'X' && c != ' ' && c != '\t')
		      throw domain_error("bad character in watch parameter");
		  }
	      }
	  }

	DEBUGPR(10,if (_watching)
		     {
		       ucout << "Watching for solution";
		       for(size_type i=0; i<_attribNum; i++)
			 if (_watchVector[i] == IN_MONOM)
			   ucout << " x" << i;
			 else if (_watchVector[i] == COMPLEMENTED_IN_MONOM)
			   ucout << " ~x" << i;
		       ucout << endl;
		     });

#ifdef ACRO_USING_CBC
	if (_verifyLog && _iterations > 1)
	  throw domain_error("--mmaVerify currently only supports "
			     "one iteration of the MMA solver");
#endif

#ifdef __DEBUG2
	DEBUGPR(10,ucout << "_wts: " << endl);
	DEBUGPR(10,ucout << "_attribNum: " << _attribNum <<  endl);
	DEBUGPR(10,ucout << "_dataStore(1): " << endl);
	DEBUGPR(10,ucout << "_branchingFactor: " << _branchingFactor <<  endl);
	copy (_dataStore[1].begin(), 
	      _dataStore[1].end(), 
	      ostream_iterator<bool> (cout, " "));
	cout << endl;
#endif
      }
    catch(exception & e)
      {
	cerr << "error caught: " << e.what() << " lineno: " 
	     << __LINE__ << endl;
	return false;
      }
    catch(...)
      {
	cerr << "unknown error caught: " << __LINE__ << endl;
	return false;
      }

    startVerifyLogIfNeeded();

    return true;
  }


  // Routine added by JE to write out data with weights.  Note that
  // the sign of the observation is just the last attribute in the
  // "_dataStore" vector of vectors.

  void maxMonomialData::writeWeightedData(ostream& os)
  {
    // Set high precision and scientific notation for weights, while
    // saving old flags and precision
    int oldPrecision = os.precision(16);
    std::ios_base::fmtflags oldFlags = os.setf(ios::scientific);

    // Write data
    for (size_type i=0; i<_obsNum; i++)
      {
	os << _wts[i] << ';';
	for (size_type j=0; j<_attribNum+1; j++)
	  if (_dataStore[i][j])
	    os << '1';
	  else
	    os << '0';
	os << endl;
      }

    // Restore stream state
    os.precision(oldPrecision);
    os.flags(oldFlags);
  }


  // writes data with weights to a file whose name we concoct
  // from the iteration number argument; added by JE

  void maxMonomialData::writeInstanceToFile(int iterNum)
  {
    stringstream s;
    s << 'w' << iterNum << '.' << problemName;
    ofstream instanceOutputFile(s.str().c_str());
    writeWeightedData(instanceOutputFile);
    instanceOutputFile.close();
  }


  //maxMonomialData::setWeight
  void maxMonomialData::setWeight(const double vec[], size_type len)
  {
    assert(len == _wts.size());
    for (int i=0; i<len; i++)
      _wts[i]= vec[i];
  }

  // maxMonomialData::getFilteredObsSorted
  void maxMonomialData::
  getFilteredObsSorted(const vector<size_type> & ignoreVarIdxs, 
		       const set<size_type> & filterObsIdxs, 
		       multiset<observationEntry_t, obsLess> &obs) const
  {
    DEBUGPR(40,ucout << "In maxMonomialData::getFilteredObsSorted, "
	    "ignoreVarIdxs.size " << ignoreVarIdxs.size() 
	    << " filterObsIdxs: " << filterObsIdxs.size() << endl);

    try
      {	 
	assert (!ignoreVarIdxs.empty());
	set<size_type>::const_iterator iter;
	for(iter = filterObsIdxs.begin(); iter != filterObsIdxs.end(); iter++)
	  {
	    observation_t obAttr;
	    obAttr.reserve(attribNum()+1-ignoreVarIdxs.size());
	    vector<size_type>::const_iterator varIt;
	    observation_t::const_iterator fromObsIt  
	      = _dataStore[*iter].begin();
	    observation_t::const_iterator toObsIt = _dataStore[*iter].end();
	    for (varIt = ignoreVarIdxs.begin(); 
		 varIt != ignoreVarIdxs.end() 
		   && fromObsIt != _dataStore[*iter].end(); 
		 varIt++)
	      {
		if (toObsIt !=  _dataStore[*iter].end())
		  fromObsIt = toObsIt+1;

		// copy up to next ignoreVarIdx
		toObsIt = _dataStore[*iter].begin()+*varIt;

		// skip if there is nothing to copy		    
		if (fromObsIt == toObsIt) 
		  continue;
		
		if(fromObsIt >= toObsIt)
		  {
		    copy (ignoreVarIdxs.begin(), 
			  ignoreVarIdxs.end(), 
			  ostream_iterator<size_type> (cout, "\n"));
		    cout << endl;
		    cerr << "fromObsIt position: " 
			 << fromObsIt-_dataStore[*iter].begin() 
			 << " toObsIt position: " 
			 <<  toObsIt-_dataStore[*iter].begin() << endl;
		    assert(false);
		  }

		obAttr.insert(obAttr.end(), fromObsIt, toObsIt);
	      }
	    

	    if(obAttr.size() != attribNum()+1-ignoreVarIdxs.size())
	      {
		cout << obAttr.size() << " " <<  attribNum() 
		     << " " << ignoreVarIdxs.size() << endl;
		copy (ignoreVarIdxs.begin(), 
		      ignoreVarIdxs.end(), 
		      ostream_iterator<size_type> (cout, "\n"));
		assert(false);
	      }
	    // create observation entry with its index and observation value
	    observationEntry_t obsE(*iter, obAttr);
	    obs.insert(obsE);
	  }
      }
    catch(exception & e)
      {
	cerr << "error caught: " << e.what() << " lineno: " << __LINE__ << endl;
	abort();
      }
    catch(...)
      {
	cerr << "unknown error caught: " << __LINE__ << endl;
	abort();
      }
  }


  // maxMonomialData::getNthMonomChild

  const monomialObj maxMonomialData::
  getNthMonomChild(const monomialObj &parent, 
		   const size_type n) const throw (exception)
  {
    DEBUGPR(40,ucout << "In maxMonomialData::getNthMonomChild, "
	    "threeWayBranching: " << threeWayBranching() << endl);

    monomialObj ret = parent;
    if (threeWayBranching())
      {
	div_t divResult = div(n,3);
	size_type idx = divResult.quot;
	if (idx >= attribNum())
	  throw out_of_range("child num getNthMonomChild too large");

	if (ret.getVarVal(idx) != NULL_VAL)
	  throw domain_error("variable already set");

	DEBUGPR(50,ucout << "In maxMonomialData::getNthMonomChild, set idx: " 
		<< idx << " value corresponding to rem: " << divResult.rem 
		<< endl);

	if (divResult.rem == 0)
	  ret.appendCompVar(idx);
	else if (divResult.rem == 1)
	  ret.appendVar(idx);
	else
	  ret.setOutOfMonom(idx);
	DEBUGPR(40,ucout << "Out of maxMonomialData::getNthMonomChild, "
		"threeWayBranching: " << threeWayBranching() 
		<< " set idx: " << idx << " value corresponding to rem: " 
		<< divResult.rem << endl);
	return ret;
      }

    size_type newRoughIdx = parent.highestIdx() + n + 1;
    if ((newRoughIdx > 2*attribNum()-1) 
	|| (parent.highestIdx() == attribNum()-1))
      {
	stringstream err;
	err << "ERROR: child index: " << n 
	    << " too large for parent with highest index: " 
	    <<  parent.highestIdx();
	throw out_of_range(err.str());
      }
    else if (newRoughIdx >= _attribNum)
      // wrap around rough idx to compute index of complemented var:
      ret.appendCompVar(newRoughIdx - attribNum() + parent.highestIdx() + 1);
    else
      ret.appendVar(newRoughIdx);
    DEBUGPR(40,ucout << "Out of maxMonomialData::getNthMonomChild"  << endl);

    return ret;
  }

  // maxMonomialData::calculateVarCovg
  void maxMonomialData::calculateVarCovg()
  {
    DEBUGPR(10,ucout << "maxMonomialData::calculateVarCovg invoked" << endl);
    _varPosCovg.resize(attribNum());
    _compVarPosCovg.resize(attribNum());
    _varNegCovg.resize(attribNum());
    _compVarNegCovg.resize(attribNum());

    try
      {
	vector<observation_t>::const_iterator iter;
	size_type idx = 0;
	for (iter = _dataStore.begin(); iter!=_dataStore.end(); iter++)
	  {
	    if (positive(*iter))
	      {
		_posIdx.insert(_posIdx.end(),idx);
		for (size_type i = 0; i < attribNum(); i++)
		  {
		    monomialObj tmp(attribNum());
		    tmp.appendVar(i);
		    if (tmp.evaluatePoint(*iter))
		      _varPosCovg[i].insert(idx);
		    else
		      _compVarPosCovg[i].insert(idx);
		  }
	      }
	    else
	      {
		_negIdx.insert(_negIdx.end(),idx);
		for (size_type i = 0; i < attribNum(); i++)
		  {
		    monomialObj tmp(attribNum());
		    tmp.appendVar(i);
		    if (tmp.evaluatePoint(*iter))
		      _varNegCovg[i].insert(idx);
		    else
		      _compVarNegCovg[i].insert(idx);
		  }
	      }
	    idx++;
	  }
      }
    catch(exception & e)
      {
	cerr << "error caught: " << e.what() << " lineno: " 
	     << __LINE__ << endl;
	abort();
      }
    catch(...)
      {
	cerr << "unknown error caught: " << __LINE__ << endl;
	abort();
      }
  }


  //  maxMonomialData::preprocess - fills the positive, 
  // negative and variable coverage index lists

  void maxMonomialData::preprocess() 
  {
    DEBUGPR(20,ucout << "InfirstMap maxMonomialData::preprocess ");

    DEBUGPR(20,ucout << "out of preprocess, ... Abstolerance=" 
	    << absTolerance << "\n\n");
  }

  //maxMonomialData::getWeight
  double maxMonomialData::getWeight(const set<size_type> & obsIdxs) 
    const throw(exception)
  {
    DEBUGPR(40,ucout << "In maxMonomialData::getWeight" << endl);

    double ret = 0;

    set<size_type>::const_iterator iter;

    for (iter = obsIdxs.begin(); iter != obsIdxs.end(); iter++)
      {  
	if (*iter < 0 || *iter >= _wts.size())
	  throw out_of_range("invalid obs idx, cannot obtain weight");
	ret += _wts[*iter];
      }

    DEBUGPR(50,ucout << "Out of maxMonomialData::getWeight" << endl);

    return ret;
  }

  // getFilteredCovg - get covered observation indices with filter, 
  // empty filter means no filter is applied

  void maxMonomialData::getFilteredCovg(const monomialObj &monom, 
					const set<size_type> & flter, 
					set<size_type> & ret) 
    const throw (exception)
  { 
    DEBUGPR(20,ucout << "In maxMonomialData::getFilteredCovg" << endl);

    if (flter.empty())
      { 
	size_type idx = 0;
	vector<observation_t>::const_iterator iter;
	for (iter = _dataStore.begin(); iter!=_dataStore.end(); iter++)
	  {
	    if (monom.evaluatePoint(*iter))
	      ret.insert(ret.end(),idx);
	    idx++;
	  }
      }
    else
      {
	set<size_type>::const_iterator iter;
	for (iter = flter.begin(); iter != flter.end(); iter++)
	  {
	    if (*iter < 0 || *iter >= _dataStore.size())
	      throw out_of_range("invalid idx in filter"); 
	    if (monom.evaluatePoint(_dataStore[*iter]))
	      ret.insert(ret.end(), *iter);
	  }
      }
    DEBUGPR(20,ucout << "Out of maxMonomialData::getFilteredCovg" << endl);
  }

  // returns the positive observations covered by monomial

  void maxMonomialData::getPosCovg(const monomialObj & monom, 
				   set<size_type> & output) 
    const throw (exception) 
  {
    try
      {
	const set<size_type> & flter = _posIdx;
	monomialObj monomLoc = monom;
	getFilteredCovg(monomLoc, flter, output);
      }
    catch(...)
      {
	throw;
      }
  }

  void maxMonomialData::getNegCovg(const monomialObj & monom, 
				   set<size_type> & output) 
    const throw (exception)
  {
    try
      {
	const set<size_type> & flter = _negIdx;
	monomialObj monomLoc = monom;
	getFilteredCovg(monomLoc, flter, output);

      }
    catch(...)
      {
	throw;
      }
  }

  void maxMonomialData::getCovg(const monomialObj & monom,  
				set<size_type> & output) 
    const throw (exception)
  {
    try
      {
	getFilteredCovg(monom,set<size_type>(), output);
      }
    catch(...)
      {
	throw;
      }
  }


  // getIntersectionWithSortedObs
  // find intersection with a sorted multiset of observations

  void maxMonomialData::
  getIntersectionWithSortedObs(const observationEntry_t & obs,
			       const multiset<observationEntry_t,obsLess> & data, 
			       set<size_type> & ret)
  {
    // perform a binary search:
    obsLess compare = data.key_comp();
    multiset<observationEntry_t,obsLess>::const_iterator iter 
      =  data.lower_bound(obs);
    if (iter != data.end() && (*iter).obs==obs.obs)
      {
	DEBUGPR(10,ucout << "found nonempty intersection in "
		"getIntersectionWithSortedObs !!" << endl);
      }
    while (iter != data.end() && (*iter).obs==obs.obs)
      {
	ret.insert(ret.end(), (*iter).idx);
	iter++;
      }

    if (!ret.empty())
      DEBUGPR(11,ucout << "returning intersection list of size: " 
	      << ret.size() << endl);
  }


  // maxMonomialData::getDistantIntervalVar - returns an index in J
  // \cup C whose covered observations do not have much of an overlap
  // with the interval [fromLeftObs, fromRightObs]

  size_type maxMonomialData::getDistantVar(const monomialObj &monom, 
					   size_type fromLeftObs, 
					   size_type fromRightObs, 
					   bool positive) const
  {
    DEBUGPR(20,ucout << "In maxMonomialData::getDistantIntervalVar" << endl);
    size_type ret = NULL_IDX;
    size_type minOverlap = _obsNum; 

    const set<size_type> & varIdx = monom.getVarIdxs();
    // const because function is const
    const vector<set<size_type> > * varMap = &_varNegCovg;
    const vector<set<size_type> > * compVarMap = &_compVarNegCovg;

    if (positive)
      {
	varMap = &_varPosCovg;
	compVarMap = &_compVarPosCovg;
      }

    set<size_type>::const_iterator iter;
    for (iter=varIdx.begin(); iter!=varIdx.end(); iter++)
      {
	if (monom.getVarVal(*iter) == IN_MONOM)
	  {
	    if ((*varMap)[*iter].empty()) 
 	    {
		ret = *iter;
		break;
            }
	    size_type overlap 
	      = max(min( *(*varMap)[*iter].rbegin(),
			 fromRightObs)-max(*(*varMap)[*iter].begin(), 
					   fromLeftObs), 0 ); 
            if (overlap <= minOverlap)
	      {
		ret = *iter;
		minOverlap = overlap;
	      }
	  }
	else // if complemented
	  {
 		DEBUGPR(200,ucout << "In maxMonomialData::"
			"getDistantIntervalVar,  *iter: " 
			<< *iter << " (*compVarMap)[*iter].size() : " 
			<< (*compVarMap)[*iter].size() << endl);

	    assert (monom.getVarVal(*iter) == COMPLEMENTED_IN_MONOM);
            if ((*compVarMap)[*iter].empty())  
            {
              ret = *iter;
              break;
            }
            size_type overlap 
	      = max(min( *(*compVarMap)[*iter].rbegin(),
			 fromRightObs)-max(*(*compVarMap)[*iter].begin(), 
					   fromLeftObs), 0 ); 
            if (overlap <= minOverlap)
	      {
		ret = *iter;
		minOverlap = overlap;
	      }
	  }
      }
   
    DEBUGPR(20,ucout << "Out of maxMonomialData::getDistantIntervalVar, "
	    "returning " << ret << endl);
    return ret;
  }


  // maxMonomialData::getPosCovgFast
  void maxMonomialData::getPosCovgFast(const monomialObj &monomArg, 
				       set<size_type> & ret) 
    const throw (exception)
  // returns the positive observations covered by monomial
  { 
    DEBUGPR(50,ucout << "maxMonomialData::getPosCovgFast invoked" << endl);

    try
      {
	monomialObj monom = monomArg; // non const copy

	size_type fromLeftObs = 0;
	size_type fromRightObs = _obsNum-1;

	size_type idx = getDistantVar(monom, fromLeftObs, fromRightObs, true);

	if (idx == NULL_IDX)
	  {
	    ret.insert(_posIdx.begin(),_posIdx.end());
	    return;
	  }
  
	variable_val_t val = monom.getVarVal(idx);
	if (val == IN_MONOM)
	  ret = _varPosCovg[idx];
	else if (val == COMPLEMENTED_IN_MONOM)
	  ret = _compVarPosCovg[idx];
        monom.remove(idx);

	while (monom.degree() && !ret.empty())
	  {
	    fromLeftObs = *ret.begin();
	    fromRightObs = *ret.rbegin();
	    idx = getDistantVar(monom, fromLeftObs, fromRightObs, true);

	    variable_val_t val = monom.getVarVal(idx);
            
            set<size_type> tmp;
 	    if  (val == IN_MONOM)
   	      tmp = _varPosCovg[idx];
	    else if (val == COMPLEMENTED_IN_MONOM)
	      tmp = _compVarPosCovg[idx];
	    else
	      throw domain_error("variable must be in monom");
	    set<size_type> outputSet;
	    set_intersection(tmp.begin(),
			     tmp.end(),
			     ret.begin(),
			     ret.end(), 
			     inserter(outputSet,outputSet.end()));
	    ret = outputSet;
	    monom.remove(idx);
	  }
	DEBUGPR(50,ucout << "out of maxMonomialData::getPosCovgFast" << endl);
      }
    catch(exception & e)
      {
	cerr << "error caught: " << e.what() << " lineno: " 
	     << __LINE__ << endl;
	throw;
      }
    catch(...)
      {
	cerr << "unknown error caught: " << __LINE__ << endl;
	throw;
      }
  }


  // maxMonomialData::getNegCovgFast
  // returns the negative observations covered by monomial

  void maxMonomialData::getNegCovgFast(const monomialObj &monomArg, 
				       set<size_type> & ret) 
    const throw (exception)
  { 
    DEBUGPR(50,ucout << "maxMonomialData::getNegCovgFast invoked" << endl);
    try
      {
	monomialObj monom = monomArg; // non const copy

	size_type fromLeftObs = 0; 
        size_type fromRightObs = _obsNum-1;

        size_type idx = getDistantVar(monom, fromLeftObs, fromRightObs, false);

        if (idx == NULL_IDX)
	  {
            ret.insert(_negIdx.begin(),_negIdx.end());
            return;
          }

        variable_val_t val = monom.getVarVal(idx);
        if (val == IN_MONOM)
          ret = _varNegCovg[idx];
        else if (val == COMPLEMENTED_IN_MONOM)
          ret = _compVarNegCovg[idx];
        monom.remove(idx);

        while (monom.degree() && !ret.empty())
          {
            fromLeftObs = *ret.begin();
            fromRightObs = *ret.rbegin();
            idx = getDistantVar(monom, fromLeftObs, fromRightObs, false);

            variable_val_t val = monom.getVarVal(idx);

            set<size_type> tmp;
            if (val == IN_MONOM)
              tmp = _varNegCovg[idx];
            else if (val == COMPLEMENTED_IN_MONOM)
              tmp = _compVarNegCovg[idx];
            else 
              throw domain_error("variable must be in monom");
            set<size_type> outputSet;
            set_intersection(tmp.begin(),
			     tmp.end(),
			     ret.begin(),
			     ret.end(), 
			     inserter(outputSet,outputSet.end()));
            ret = outputSet;
            monom.remove(idx);
          }
	DEBUGPR(50,ucout << "out of maxMonomialData::getNegCovgFast" << endl);
      }
    catch(exception & e)
      {
	cerr << "error caught: " << e.what() << " lineno: " 
	     << __LINE__ << endl;
	throw;
      }
    catch(...)
      {
	cerr << "unknown error caught: " << __LINE__ << endl;
	throw;
      }
  }


  //maxMonomialData::removeIdxs
  void maxMonomialData::removeIdxs(const list<size_type> &notInMonom, 
				   observationEntry_t & obsStrct) const
  {

    list<size_type>::const_reverse_iterator iter;
    // erase in reverse order of var idxs
    for (iter = notInMonom.rbegin(); 
	 iter != notInMonom.rend(); 
	 iter++)
      {
	observation_t::iterator racIter = obsStrct.obs.begin();
	obsStrct.obs.erase((racIter + *iter));
      }
  }


  /////////////////////////// maxMonomSub /////////////////////////

  int maxMonomSub::splitComputation()
  {
    setState(separated);
    int numOfChildren = _children.size();
    DEBUGPR(10,ucout << "maxMonomSub:::splitComputation for: " 
	    << getMonomialObj() << " attribNum: " 
	    << global()->attribNum() << " returning: " 
	    << numOfChildren << endl);
    return numOfChildren;
  }



  // maxMonomSub::maxMonomSubFromData
  void maxMonomSub::maxMonomSubFromData(maxMonomialData* master)
  {
    globalPtr = master;
    DEBUGPR(10,ucout << "maxMonomSub:::maxMonomSubFromData() invoked.\n");

    _posCovgIdx = global()->getPositives();
    _negCovgIdx = global()->getNegatives();

    _posCovg = global()->getWeight(_posCovgIdx);
    _negCovg = global()->getWeight(_negCovgIdx);
    bound = max(_posCovg,_negCovg);
    DEBUGPR(10,ucout << "maxMonomSub:::maxMonomSubFromData() computed "
	    "initial bound: " << bound << endl);
      
    if (global()->threeWayBranching())
      _monom = monomialObj(global()->attribNum(), NULL_VAL);
    else
      _monom = monomialObj(global()->attribNum(), NOT_IN_MONOM);

    _assocSoln = new maxMonomSolution(_monom);
    _assocSoln->value = getObjectiveVal(); // set solution value

    DEBUGPR(20,ucout << "Created blank problem, out of "
	    "maxMonomSub:::maxMonomSubFromData" << endl);
  }


  //makeChild
  branchSub* maxMonomSub::makeChild(int whichChild)
  {
    DEBUGPRX(20,global(), "maxMonomSubThreeWay::makeChild invoked, "
	     "whichChild: " << whichChild << endl);

#ifdef ACRO_VALIDATING
    if (whichChild >= _children.size())
      {
	cerr << "requested whichChild: " << whichChild 
	     << " but there are only: " << _children.size() << endl;
	return NULL;
      }
#endif

    maxMonomSub* ret = _children[whichChild];
    double saveBound = ret->bound;
    ret->branchSubAsChildOf(this);   // Overwrites bound with parent bound
    ret->bound = saveBound;          // Put bound back how it was

    DEBUGPR(10,ucout << "MakeChild produces " << ret 
	    << ' ' << ret->getMonomialObj() << endl);

    //// JE tried removing below as it causes a race condition in the
    //// the parallel code.  Also it could produce strange results if
    //// children are not spawned in order.  The above patch of
    //// 'bound' prevents removing the code from having any
    //// significant effect on performance.

    // if (whichChild < _children.size() - 1)
    //   {
    // 	assert (_children[whichChild]->bound <= bound);
    // 	bound = _children[whichChild+1]->bound; 
    //            //max(_children[whichChild]->bound, getObjectiveVal());
    //   }
    // else if (whichChild ==  _children.size()-1)
    //   {
    // 	// set bound to objective val if all children have been created
    // 	bound = getObjectiveVal();
    //   }

    if (global()->watching() &&
	ret->getMonomialObj().covers(global()->watchVector()))
      {
	ucout << "Watched subproblem: depth ";
	int oldWidth = ucout.width(6);
	ucout <<  ret->depth;
	ucout.width(oldWidth);
	ucout << ' ' << ret << ", child of " << this;
	maxMonomSubThreeWay* t3 = dynamic_cast<maxMonomSubThreeWay*>(this);
	if (t3)
	  ucout << ", branched on x" << t3->getBranchVar();
	ucout << endl;
	  }

    if (global()->verifyLog())
      {
	ostream& vlFile = global()->verifyLogFile();
	vlFile << "create ";
	ret->vlWriteDescription();
	vlFile << ' ' << ret->id.creatingProcessor
	       << ' ' << ret->id.serial
	       << ' ' << ret->bound << ' ';
	vlWriteDescription();
	vlFile << endl;
      }

    _children[whichChild] = NULL;
    DEBUGPRX(20,global(), "maxMonomSubThreeWay::makeChild invoked, "
	     "whichChild: " << whichChild << endl);

    return ret;
  }




  void maxMonomSub::initChild(maxMonomSub* parent,int whichChild) 
    throw (exception)
  {  
    try
      {
	// conjoin with an additional variable or its complement:
	_monom = global()->getNthMonomChild(parent->getMonomialObj(), 
					    whichChild);
      }
    catch(exception & e)
      {
	cerr << e.what() << endl;
	throw;
      }
  }


  // maxMonomSub::findMostNonSeparating - finds set of variable 
  // size ceil(n*K) that are most non-separating

  double maxMonomSub::
  findMostNonSeparating(const vector<size_type> & outOfMonom,
			const vector<size_type> & freeVariables,
			vector<size_type> & ret,
			vector<pair<double,double> > & insepWts) 
    throw (exception)
  {
    DEBUGPR(20,ucout << "maxMonomSub::findMostNonSeparating invoked, "
	    "freeVariables.size: " << freeVariables.size() << endl);

    double totalInsepWt = min(_posCovg,_negCovg);

    if (freeVariables.empty())
      // if all variables fixed then return the least class weight
      return totalInsepWt;

    set<size_type> posSet = _posCovgIdx;
    set<size_type> negSet = _negCovgIdx;
    vector<size_type> freeVar = freeVariables;
    double K = global()->getBranchingFactor();

    //while (freeVar.size() > ceil(freeVariables.size()*K))
    while (!freeVar.empty())
      {
        double maxInsepWt = 0;
        vector<size_type>::iterator mostInsepIdx = freeVar.end();
        set<size_type> mostInsepPos, mostInsepNeg;
        eqvClassVec bestEqvClasses;

        vector<size_type>::iterator it;
        for (it = freeVar.begin(); it != freeVar.end(); it++)
          {
            DEBUGPR(39,ucout << "Now excluding " << *it << endl);
            vector<size_type> tmp = outOfMonom;
	    // insert at the beginning of tmp
            tmp.insert(tmp.end(),freeVar.begin(), it);
	    // insert at end of vector tmp
            tmp.insert(tmp.end(),it+1,freeVar.end());
            set<size_type> posInsep, negInsep;
            eqvClassVec eqvClasses;
            double insepWt = calculateInsepWeight(tmp, posSet, negSet, 
						  posInsep, negInsep, 
						  true, true, eqvClasses);
	    if (insepWt >= maxInsepWt)
              {
                maxInsepWt = insepWt;
                mostInsepIdx = it;
                mostInsepPos = posInsep;
                mostInsepNeg = negInsep;
                bestEqvClasses = eqvClasses;
              }
            else if (insepWt < _insepWt - EPS) 
              {
                cerr << "Monom: " << _monom << " removing var: " 
		     << *it << " gives insepWt: " << insepWt 
		     << " while cur _insepWt: " << _insepWt 
		     << " and maxInsepWt: " << maxInsepWt << endl;
                double insepWt2 = calculateInsepWeight(outOfMonom, 
						       posSet, 
						       negSet, 
						       posInsep, 
						       negInsep, 
						       false);
                cerr << " recomputed _insepWt: " << insepWt2 
		     << " outOfMonom.size(): " << outOfMonom.size()  
		     << " tmp.size():" << tmp.size() << endl;
                assert(false);
              }
          }
        assert(mostInsepIdx != freeVar.end());

        if (freeVar.size() <= ceil(((double)freeVariables.size())*K))
          {
            // compute bounds
            pair<double,double> newInsepWts;
            calculateInsepWeight(bestEqvClasses,*mostInsepIdx, newInsepWts);
            ret.push_back(*mostInsepIdx);
            insepWts.push_back(newInsepWts);
          }
	DEBUGPR(39,ucout << "erasing: " << *mostInsepIdx 
		<< " freeVar.size:  " << freeVar.size() << endl);
	// remove var that maintains the maximum inseparables
        freeVar.erase(mostInsepIdx);
        if (freeVar.size() <= ceil(((double)freeVariables.size())*K))
          {
            if (ret.empty()) // record insep wt when only returned
	      // vars are left to be ordered
              totalInsepWt = maxInsepWt;
          }
        posSet = mostInsepPos;
        negSet = mostInsepNeg;
      }
    reverse(ret.begin(),ret.end());
    reverse(insepWts.begin(), insepWts.end());
    DEBUGPR(20,ucout << "maxMonomSub::findMostNonSeparating finished "
	    "ret.size: " << ret.size() << " totalInsepWt: " << totalInsepWt 
	    << " first var in list: " << ret.front() << endl);
    return totalInsepWt;
  }



  // maxMonomSub::maxMonomSubAsChildOf
  void maxMonomSub::maxMonomSubAsChildOf(maxMonomSub* parent,
					 int          whichChild,
					 double       suppliedBound)
  {
    globalPtr = parent->globalPtr;

    try
      {
	initChild(parent,whichChild);
	vector<size_type> ignoreIdxs;

        DEBUGPR(10,ucout << "maxMonomSub:::maxMonomSubAsChildOf() invoked: "  
		<< whichChild << " monom: " << _monom << " parent monom: " 
		<< parent->_monom << endl);

	size_type lastLitIdx = _monom.highestIdx();
	size_type parentHighestLitIdx = parent->getMonomialObj().highestIdx();

	if ((lastLitIdx == parentHighestLitIdx + 1) 
	    && !global()->threeWayBranching())
	  _insepWt = parent->_insepWt;
	else
	  {
	    for (size_type i = 0; i <= _monom.highestIdx(); i++)
	      {
		if (_monom.getVarVal(i) == NOT_IN_MONOM)
		  // add all skipped variable indices to 
		  // "not in the monomial" vector
		  ignoreIdxs.push_back(i);
	      }
	    assert(global()->threeWayBranching() || 
		   ignoreIdxs.size() + _monom.degree() == lastLitIdx+1);
	  }

	DEBUGPR(40,ucout << "ignoreIdxs = " << ignoreIdxs << endl);
	 
	size_type lastSetIdx = _monom.lastSetIdx(); 
	variable_val_t val = _monom.getVarVal(lastSetIdx);
	
	assert(val != NULL_VAL);

	// Don't perform massive calcuation if bound was already known
	// Not sure how all this will work if trying to save
	// coverages, so don't try it in that case

	if ((suppliedBound >= 0) && global()->getLowNodeMemory())
	  {
	    bound = suppliedBound;
	    return;
	  }

	if (val != NOT_IN_MONOM)
	  if (_monom.degree()==1) // no need for set intersection
	    {
	      DEBUGPR(20,ucout << "In monomial, degree 1\n");
	      _posCovgIdx = global()->getPosVarCovg(lastSetIdx,val); 
	      _negCovgIdx = global()->getNegVarCovg(lastSetIdx,val);
	    }
	  else
	    {
	      DEBUGPR(20,ucout << "maxMonomSub:::maxMonomSubAsChildOf() "
		      << "before set_intersection lastSetIdx: " 
		      << lastSetIdx << endl);
	      set_intersection(parent->getPosCovgIdx().begin(),
			       parent->getPosCovgIdx().end(), 
			       global()->getPosVarCovg(lastSetIdx,val).begin(), 
			       global()->getPosVarCovg(lastSetIdx,val).end(), 
			       inserter(_posCovgIdx,_posCovgIdx.end()));
	      set_intersection(parent->getNegCovgIdx().begin(),
			       parent->getNegCovgIdx().end(), 
			       global()->getNegVarCovg(lastSetIdx,val).begin(), 
			       global()->getNegVarCovg(lastSetIdx,val).end(), 
			       inserter(_negCovgIdx,_negCovgIdx.end()));
	    }
	else // if last set is not in monom
	  {
	    _posCovgIdx = parent->getPosCovgIdx();
	    _negCovgIdx = parent->getNegCovgIdx();
	    DEBUGPR(20,ucout << "Copy coverage from parent, sizes are "
		    << _posCovgIdx.size() << ' ' << _negCovgIdx.size() << endl);
	  }

	DEBUGPR(100,ucout << "Positive coverage: " << _posCovgIdx << endl);
	DEBUGPR(100,ucout << "Negative coverage: " << _negCovgIdx << endl);
	_posCovg = global()->getWeight(_posCovgIdx);
	_negCovg = global()->getWeight(_negCovgIdx);
	DEBUGPR(20,ucout << "First bound attempt -- _posCovg=" << _posCovg 
		<< " _negCovg=" << _negCovg << endl);
	bound = max(_posCovg, _negCovg);

        if (val != NOT_IN_MONOM &&  
	    _posCovg == parent->_posCovg && 
	    _negCovg == parent->_negCovg)
          {
	    DEBUGPR(20,ucout << "Coverage identical to parent.  Death...\n");
            setState(dead);
          }
        else if (global()->canFathom(bound))
          {
	    DEBUGPR(20,ucout << "Fathomable.  Death...\n");
            setState(dead);
          }
        else if (!global()->getImprovedBound()
		 || (_posCovg == 0 && _negCovg == 0))
          {
            _insepWt = 0;
	    DEBUGPR(20,ucout << "Set inseparable weight to 0\n");
          }
        else
          {
            calculateInsepWeight(ignoreIdxs);
	    DEBUGPR(20,ucout << "Calculated _insepWt=" << _insepWt << endl);
          }
        bound = max(_posCovg, _negCovg) - _insepWt;
	DEBUGPR(20,ucout << "Bound is now " << bound << endl);
        if (global()->canFathom(bound))
          {
            setState(dead);
          }

	DEBUGPR(100,ucout << "State is " << stateStringArray[state] << endl);

        if (global()->getLowNodeMemory())
          {
            _posCovgIdx.clear(); // free memory
            _negCovgIdx.clear();
	    DEBUGPR(20,ucout << "Clearing coverages (#1) for " << this 
		    << ' '  << _monom << '\n');
          }

        if (state != dead && val != NOT_IN_MONOM)
          {
            monomialObj tmp = _monom;
            tmp.finalize();
            _assocSoln = new maxMonomSolution(tmp);
            _assocSoln->value = getObjectiveVal();
            maxMonomSolution * soln = new maxMonomSolution(_assocSoln);
	    DEBUGPR(5,ucout << "As-child-of MaxMonom solution " 
		    << " value=" << soln->value << ' '
		    << soln->getMonomialObj() << endl);
	    DEBUGPR(200,ucout << "Solution address " << (void *) soln << endl);
            global()->foundSolution(soln);
#ifdef ACRO_VALIDATING
	    DEBUGPR(200,ucout << "incumbent = " 
		    << (void *) global()->incumbent << endl);
#endif
          }

	DEBUGPR(10,ucout << "maxMonomSub::maxMonomSubAsChildOf(), "
		" bound computed for: " << getMonomialObj() 
		<< " bound: " << bound << ", solution val: " 
		<< getObjectiveVal() << " _posCovg:" << _posCovg 
		<< " _negCovg:" << _negCovg << " insepWt: " 
		<< _insepWt << endl);
      }
    catch(exception & e)
      {
	cerr << e.what() << " child: " << whichChild 
	     << " passed to maxMonomSub::maxMonomSubAsChildOf, for monom: " 
	     << parent->getMonomialObj() << endl;
	abort();
      }
    catch (...)
      {
	abort();
      }

    DEBUGPR(20,ucout << "Out of maxMonomSub:::maxMonomSubAsChildOf()" << endl);
  }


  // maxMonomSub::calculateInsepWeight
  double maxMonomSub::
  calculateInsepWeight(const vector<size_type> & outOfMonomList, 
		       const set<size_type> &posCovgIdx,
		       const set<size_type> &negCovgIdx, 
		       set<size_type> & insepPos, 
		       set<size_type> & insepNeg, 
		       bool returnInsep) const
  {
    eqvClassVec tmp;
    return calculateInsepWeight(outOfMonomList, 
				posCovgIdx, 
				negCovgIdx, 
				insepPos, 
				insepNeg, 
				returnInsep, 
				false, 
				tmp);
  }

  // maxMonomSub::calculateInsepWeight
  // calculate insep weight and return all observations that are 
  // not separated from all observations of the other class

  double maxMonomSub::
  calculateInsepWeight(const vector<size_type> & outOfMonomList, 
		       const set<size_type> &posCovgIdx,
		       const set<size_type> &negCovgIdx, 
		       set<size_type> & insepPos, 
		       set<size_type> & insepNeg, 
		       bool returnInsep, 
		       bool returnInsepEqvClasses, 
		       eqvClassVec & insepEqvClasses) const
  {
    DEBUGPR(40,ucout << getMonomialObj() 
	    << " maxMonomSub::calculateInsepWeight helper "
	    "invoked outOfMonomList.size " << outOfMonomList.size() 
	    << endl);
    if (outOfMonomList.empty())
      return 0;

    vector<size_type> ignoreIdxs = outOfMonomList;
    // remove later - make sure the vector of variable indices is sorted
    sort(ignoreIdxs.begin(), ignoreIdxs.end());

    maxMonomialData::obsLess compar;
    // disregard also the class variable in finding intersections
    ignoreIdxs.push_back(global()->attribNum());
    multiset<observationEntry_t, maxMonomialData::obsLess> 
      sortedNegCovg(compar);
    global()->getFilteredObsSorted(ignoreIdxs, negCovgIdx, sortedNegCovg);
    multiset<observationEntry_t, maxMonomialData::obsLess> 
      sortedPosCovg(compar);
    global()->getFilteredObsSorted(ignoreIdxs, posCovgIdx, sortedPosCovg);

    multiset<observationEntry_t,maxMonomialData::obsLess>::const_iterator 
      iterPos = sortedPosCovg.begin();
    multiset<observationEntry_t,maxMonomialData::obsLess>::const_iterator 
      iterNeg = sortedNegCovg.begin();
    eqvClassEntry cur;

    double insepWt = 0;
    DEBUGPR(45,ucout << "sorted covg pos neg sizes: " 
	    << sortedPosCovg.size() << " " << sortedNegCovg.size() 
	    << endl);

    observation_t lastInsep;
   
    while(iterPos != sortedPosCovg.end() && iterNeg != sortedNegCovg.end())
      {
        if ((*iterPos).obs != lastInsep && (*iterNeg).obs != lastInsep)
          {
            insepWt += min(cur.posWt,cur.negWt);
            DEBUGPR(220,ucout << "pos obs: " << (*iterPos).idx 
		    << " neg obs: " << (*iterNeg).idx 
		    << " added insep wt: " << min(cur.posWt,cur.negWt) 
		    << endl);
            if (min(cur.posWt,cur.negWt) > 0 && returnInsepEqvClasses)
              insepEqvClasses.push_back(cur);
            cur.clear();
          }

        if ( (*iterPos).obs == (*iterNeg).obs )
          {
            cur.posWt += global()->getWeight((*iterPos).idx);
            cur.negWt += global()->getWeight((*iterNeg).idx);

            DEBUGPR(220,ucout << " posWt= " 
		    << global()->getWeight((*iterPos).idx) 
		    << " negWt=" << global()->getWeight((*iterNeg).idx) 
		    << endl);
            DEBUGPR(220,ucout << "pos obs: " << *iterPos << endl);
            DEBUGPR(220,ucout << "neg obs: " << *iterNeg << endl);

            lastInsep = (*iterPos).obs;

	    if (returnInsep)
              {
                insepPos.insert((*iterPos).idx);
                insepNeg.insert((*iterNeg).idx);
              }
            if (returnInsepEqvClasses)
              {
                cur.posObs.insert((*iterPos).idx);
                cur.negObs.insert((*iterNeg).idx);
              }
            iterPos++;
            iterNeg++;
          }
        else if ((*iterPos).obs < (*iterNeg).obs)
          {
            if ((*iterPos).obs == lastInsep)
              {
                if (returnInsep)
                  insepPos.insert((*iterPos).idx);
                if (returnInsepEqvClasses)
                  cur.posObs.insert((*iterPos).idx);
              }
            cur.posWt += global()->getWeight((*iterPos).idx);
            iterPos++;
          }
        else // >
          {
            if ((*iterNeg).obs == lastInsep)
              {
                if (returnInsep)
                  insepNeg.insert((*iterNeg).idx);
                if (returnInsepEqvClasses)
                  cur.negObs.insert((*iterNeg).idx);
              }
            cur.negWt += global()->getWeight((*iterNeg).idx);
            iterNeg++;
          }
      }

    while (iterPos != sortedPosCovg.end() && (*iterPos).obs == lastInsep )
      {
        if (returnInsep)
          insepPos.insert((*iterPos).idx);
        if (returnInsepEqvClasses)
          cur.posObs.insert((*iterPos).idx);

        cur.posWt += global()->getWeight((*iterPos).idx);
        iterPos++;
      }
    while (iterNeg != sortedNegCovg.end() && (*iterNeg).obs == lastInsep)
      {
        if (returnInsep)
          insepNeg.insert((*iterNeg).idx);
        if (returnInsepEqvClasses)
          cur.negObs.insert((*iterNeg).idx);

        cur.negWt += global()->getWeight((*iterNeg).idx);
        iterNeg++;
      }

    insepWt += min(cur.posWt,cur.negWt);
    if (min(cur.posWt,cur.negWt) > 0 && returnInsepEqvClasses)
      insepEqvClasses.push_back(cur);
    DEBUGPR(39,ucout << "got insep wt: " << insepWt << endl);
    return insepWt;
  }

  // maxMonomSub::calculateInsepWeight
  // calculate and set _insepWt attribute

  void maxMonomSub::
  calculateInsepWeight(const vector<size_type> & outOfMonomList)
  {
    DEBUGPR(20,ucout << getMonomialObj() 
	    << " maxMonomSub::calculateInsepWeight invoked outOfMonomList.size "
	    << outOfMonomList.size() << endl);
    // for case where lists end with inseparable observations
    set<size_type> temp1, temp2;
    _insepWt = calculateInsepWeight(outOfMonomList, 
				    _posCovgIdx, 
				    _negCovgIdx, 
				    temp1, 
				    temp2, 
				    false);
    DEBUGPR(20,ucout << "maxMonomSub::calculateInsepWeight finished "
	    "with insepWt: " << _insepWt << endl);
  }

  // calculateInsepWeight - given equivalence classes
  void maxMonomSub::calculateInsepWeight(const eqvClassVec & eqvClasses, 
					 const size_type varIdx, 
					 pair<double,double> & insepWts) const
  {
    insepWts.first = 0;  // insep wt after intersection with var covg
    insepWts.second = 0; // intersection with complement covg
    eqvClassVec::const_iterator iter = eqvClasses.begin();
    for (; iter != eqvClasses.end(); iter++)
      {
        set<size_type> newPosObs, newNegObs;

        set_intersection( (*iter).posObs.begin(), 
			  (*iter).posObs.end(), 
			  global()->getPosVarCovg(varIdx,IN_MONOM).begin(), 
			  global()->getPosVarCovg(varIdx,IN_MONOM).end(), 
			  inserter(newPosObs,newPosObs.end()));

        set_intersection( (*iter).negObs.begin(), 
			  (*iter).negObs.end(), 
			  global()->getNegVarCovg(varIdx,IN_MONOM).begin(), 
			  global()->getNegVarCovg(varIdx,IN_MONOM).end(), 
			  inserter(newNegObs,newNegObs.end()));

        double posWtInMonom = global()->getWeight(newPosObs);
        double negWtInMonom = global()->getWeight(newNegObs);
        insepWts.first += min(posWtInMonom, negWtInMonom);
        insepWts.second += min((*iter).posWt - posWtInMonom, 
			       (*iter).negWt - negWtInMonom);
	//compute insep wt of complement
      }
  }


  void maxMonomialData::startVerifyLogIfNeeded()
  {
    if (verifyLog())
      {
	if (_vlFile == NULL)
	  _vlFile = openVerifyLogFile();
	verifyLogFile() << "attributes " << attribNum() << endl;
	verifyLogFile() << "observations " << obsNum() << endl;
      }
  } 


  maxMonomialData::~maxMonomialData() 
  {
    if (verifyLog())
      {
	verifyLogFile() << "result " << fathomValue() << endl;
	delete _vlFile;    // Doesn't delete file; actually closes it
      }
  }

  void maxMonomSub::setRootComputation()
  {
    DEBUGPR(10,ucout << "maxMonomSub::setRootComputation() invoked." << endl);
    if (global()->verifyLog())
      {
	global()->verifyLogFile() << "create ";
	vlWriteDescription();
	global()->verifyLogFile() << ' ' << id.creatingProcessor
				  << ' ' << id.serial
				  << ' ' << bound 
				  << " root\n";
      }
  }

  // maxMonomSub::~maxMonomSub - DTOR, frees memory in case subproblem "
  // has been associated with a solution object

  maxMonomSub::~maxMonomSub() 
  {
    DEBUGPR(10,ucout << "Destroying " << this << endl);
    if (_assocSoln != NULL) delete _assocSoln;
    childrenVecType::iterator iter = _children.begin();
    for(; iter != _children.end(); iter++) 
      if (*iter != NULL)
	delete *iter;
  }


  // maxMonomSub::boundComputation
  void maxMonomSub::boundComputation(double* controlParam) 
  {
    DEBUGPR(9,ucout << "In boundComputation: " << getMonomialObj() 
	    << " bound: " << bound << endl);
    try
      {
        const int childNum = getFutureChildrenNumber();
	_children.reserve(childNum);

	DEBUGPR(20,ucout << "creating " << childNum << " children"  
		<< " getFutureChildrenNumber(): " 
		<< getFutureChildrenNumber() << " soln highest idx: " 
		<< _monom.highestIdx() << " global()->attribNum(): " 
		<< global()->attribNum() << endl);

	if (global()->getLowNodeMemory())
	  {
	    global()->getPosCovgFast(_monom,_posCovgIdx);
	    global()->getNegCovgFast(_monom,_negCovgIdx);
	  }

	for (int i = 0; i < childNum; i++) // create all children in advance
	  {// store them in a vector sorted in a decreasing bounds
	    maxMonomSub *temp = allocateObject();
	    if (!temp)
	      {
		throw domain_error(" maxMonomSub ptr null after allocation");
	      }
	    temp->maxMonomSubAsChildOf(this, i);
	    
	    if  (global()->canFathom(temp->bound) || temp->state == dead)
	      {
		delete temp;
		continue;
	      }
	    if (!temp)
	      {
		throw domain_error("maxMonomSub ptr null after calling "
				   "maxMonomSubAsChildOf");
	      }
	    _children.insert(_children.end(),temp);
	  } 

	if (global()->getLowNodeMemory())
	  {
	    _posCovgIdx.clear();
	    _negCovgIdx.clear();
	    DEBUGPR(20,ucout << "Clearing coverages (#2) for " << this 
		    << ' '  << _monom << '\n');
	  }

	if (!_children.empty())
	  {
	    sort(_children.begin(), _children.end(), maxMonomSubComp());
	    // set to highest bound of a child
	    bound =  (*_children.begin())->bound;
	  }
      	DEBUGPR(15,ucout << "Out of boundComputation: " 
		<< getMonomialObj() << " bound: " << bound 
		<< " _children.size(): " << _children.size() << endl);

	setState(bounded);
	*controlParam = 1;
      }
    catch(std::bad_alloc const &)
      {
	cerr << "out of memory!" << endl;
	abort();
      }
    catch(exception & e)
      {
	cerr << "error caught: " << e.what() << " lineno: " << __LINE__ << endl;
	abort();
      }
    catch(...)
      {
	cerr << "unknown error caught: " << __LINE__ << endl;
	abort();
      }
  }


  // maxMonomSubThreeWay::boundComputation
  void maxMonomSubThreeWay::boundComputation(double* controlParam) 
  {
    DEBUGPR(9,ucout << "maxMonomSubThreeWay::boundComputation: " 
	    << getMonomialObj() << " bound: " << bound << endl);
    try
      {
	if (global()->getLowNodeMemory())
	  {
	    global()->getPosCovgFast(_monom,_posCovgIdx);
	    global()->getNegCovgFast(_monom,_negCovgIdx);
	  }

	// go over all unfixed variables in order to select a variable 
	// create all children in advance
	// store them in a vector sorted by decreasing bounds

	for (size_type i = 0; 
	     i < global()->attribNum() && state != dead; 
	     i++)
	  if (_monom.getVarVal(i) == NULL_VAL)
	    tryThreeWaySplit(i);

	boundComputationCleanUp(controlParam);

	DEBUGPRP(50,ucout << "Sorted children at end of three-way :");
	DEBUGPRP(50,for (size_type jj=0; jj<_children.size(); jj++)
		      ucout << ' ' << _children[jj]->bound);
	DEBUGPR(50,ucout << endl);

	DEBUGPR(9,ucout << "Out of maxMonomSubThreeWay::boundComputation "
		"for: " << getMonomialObj() << " bound: " << bound << endl);
      }

    catch(std::bad_alloc const &)
      {
	cerr << "out of memory!" << endl;
	abort();
      }
    catch(exception & e)
      {
	cerr << "error caught: " << e.what() << " lineno: " 
	     << __LINE__ << endl;
	abort();
      }
    catch(...)
      {
	cerr << "unknown error caught: " << __LINE__ << endl;
	abort();
      }
  }


  void maxMonomSubThreeWay::boundComputationCleanUp(double* controlParam) 
  {
    if (global()->getLowNodeMemory())
      {
	_posCovgIdx.clear();
	_negCovgIdx.clear();
	DEBUGPR(20,ucout << "Clearing coverages (#3) for " << this 
		<< ' '  << _monom << '\n');
      }

    // Set bound to highest bound of a child; _children[0]->bound should
    // usually work, but if there are two very close bounds that got rounded
    // to the same value, it could possibly be wrong.  If there are no 
    // children, this subproblem is dead, and the bound is -1.
    bound = -1.0;
    for (size_type j=0; j<_children.size(); j++)
      if (_children[j]->bound > bound)
	bound = _children[j]->bound;
      
    if (global()->verifyLog())
      {
	ostream& vlFile = global()->verifyLogFile();
	vlFile << "bound ";
	vlWriteDescription();
	vlFile << ' ' << bound << ' ' << _branchChoice.branchVar;
	for (size_type k=0; k<3; k++)
	  if (_branchChoice.branch[k].whichChild >= 0)
	    vlFile << ' ' << _branchChoice.branch[k].whichChild
		   << ' ' << _branchChoice.branch[k].exactBound;
	vlFile << endl;
      }

  DEBUGPR(10,ucout << "Branching choice is " << _branchChoice << endl);

  if (state != dead)
    setState(bounded);

  *controlParam = 1;
  }


  // Try to split on a single variable and update branching choice 
  // if better than last tentative choice (if any).

  void maxMonomSubThreeWay::tryThreeWaySplit(size_type i)
  {
    const unsigned short VAR_VAL_NUM = 3;

    DEBUGPR(15,ucout << "Trying x" << i << endl);
    childrenVecType temp;
    temp.reserve(VAR_VAL_NUM);
    branchChoice thisChoice(-1,-1,-1,i);
    for (int localj=0; localj<VAR_VAL_NUM; localj++)
      {
	int j = VAR_VAL_NUM*i + localj;
	maxMonomSub * tmp = allocateObject();
	if (!tmp)
	  throw domain_error("maxMonomSub ptr null after allocation");
	tmp->maxMonomSubAsChildOf(this, j);
	if (tmp->canFathom() || (tmp->state == dead))
	  delete tmp;
	else
	  {
	    int position = temp.size();
	    thisChoice.branch[position].set(tmp->bound,
					    global()->getRoundingQuantum(),
					    localj,
					    position);
	    temp.push_back(tmp);
	  }
      }
	    
    if (temp.empty())
      setState(dead);

    DEBUGPR(15,ucout << "Evaluating" << thisChoice << endl);
		
    // select variable based on minimum of children 
    // bounds given in lexicographically decreasing order

    thisChoice.sortBounds();

    DEBUGPR(15,ucout << "Sorted version is " << thisChoice << endl);

    if (thisChoice < _branchChoice)
      {
	childrenVecType::iterator iter;
	for (iter = _children.begin(); iter != _children.end(); iter++)
	  delete *iter;
	_children.resize(temp.size());
	for (size_type jj=0; jj<temp.size(); jj++)
	  _children[jj] = temp[thisChoice.branch[jj].arrayPosition];
	_branchChoice = thisChoice;
	DEBUGPR(15,ucout << "Improves best: " << i << endl);
	DEBUGPR(15,ucout << "Branch choice now: " << _branchChoice << endl);
	DEBUGPRP(15,ucout << "Sorted subproblems:");
        DEBUGPRP(15,for (size_type jj=0; jj<_children.size(); jj++)
		      ucout << ' ' << _children[jj]->bound);
	DEBUGPR(15,ucout << endl);
      }
    else
      {
	childrenVecType::iterator iter;
	for (iter = temp.begin(); iter != temp.end(); iter++)
	  delete *iter;
      }
  }



  ///////////////////// maxMonomSubKWay ///////////////////////////////////
  
  // maxMonomSubKWay::boundComputation

  void maxMonomSubKWay::boundComputation(double* controlParam) 
  {
    DEBUGPR(9,ucout << "In maxMonomSubKWay::boundComputation: " 
	    << getMonomialObj() << " bound: " << bound << endl);
    try
      {
        if (global()->getLowNodeMemory())
          {
            global()->getPosCovgFast(_monom,_posCovgIdx);
            global()->getNegCovgFast(_monom,_negCovgIdx);
          }

        vector<size_type> freeVariables;
        vector<size_type> oldOutOfMonomList;

	// go over all unfixed variables in order to select a variable
        for (int i = 0; i < global()->attribNum(); i++)
          {
            variable_val_t val = _monom.getVarVal(i);
            if (val == NULL_VAL)  // skip fixed variables
              {
                freeVariables.push_back(i);
              }
            else if (val==NOT_IN_MONOM)
              {
                oldOutOfMonomList.push_back(i);
              }
          }
        vector<size_type> outOfMonomList;
        double insepWt = 0;
        vector<pair<double,double> > insepWts;

        insepWt = findMostNonSeparating(oldOutOfMonomList, 
					freeVariables, 
					outOfMonomList, 
					insepWts);
        DEBUGPR(20,ucout << _monom 
		<< " boundComputation, freeVariables.size() = " 
		<< freeVariables.size() << " outOfMonomList.size=" 
		<< outOfMonomList.size() << endl);

	// if equal then do not create child with excluded variables
        if (!outOfMonomList.empty() 
	    && outOfMonomList.size() < freeVariables.size())
	  {
	    maxMonomSub * tmp = new maxMonomSubKWay();
	    if (!tmp)
	      throw domain_error("maxMonomSub ptr null after allocation");
	    ((maxMonomSubKWay *)tmp)->maxMonomSubAsChildOf(this, 
							   outOfMonomList, 
							   insepWt);
	    if  (global()->canFathom(tmp->bound) || tmp->state == dead)
	      delete tmp;
	    else
	      _children.push_back(tmp);
	  }
        vector<size_type>::iterator it;
        vector<pair<double,double> >::const_iterator it2;

        for (it=outOfMonomList.begin(), it2 = insepWts.begin(); 
	     it != outOfMonomList.end(); 
	     it++)
          {
            double compInsep = -1;
            double Insep = -1;
            if (it2 != insepWts.end())
              {
                compInsep = (*it2).second;
                Insep = (*it2).first;
                it2++;
              }

            maxMonomSub * tmp = new maxMonomSubKWay();
            if (!tmp)
              throw domain_error("Cannot allocate maxMonomSub");

            vector<size_type> childOutOfMonom(outOfMonomList.begin(), it);
            assert (childOutOfMonom.empty() || it != outOfMonomList.begin());

            ((maxMonomSubKWay *)tmp)->maxMonomSubAsChildOf(this, 2*(*it), 
							   childOutOfMonom, 
							   compInsep);

	    if (global()->canFathom(tmp->bound) || tmp->state == dead)
              {
                delete tmp;
              }
	    else
	      _children.push_back(tmp);
            DEBUGPR(29,ucout << "created child: " << tmp->getMonomialObj() 
		    << " " << endl);

            tmp = new maxMonomSubKWay();
            if (!tmp)
              throw domain_error("Cannot allocate maxMonomSub");
            ((maxMonomSubKWay *)tmp)->maxMonomSubAsChildOf(this, 2*(*it)+1, 
							   childOutOfMonom, 
							   Insep);
            if (global()->canFathom(tmp->bound) || tmp->state == dead)
              {
                delete tmp;
                continue;
              }
            DEBUGPR(29,ucout << "created child: " << tmp->getMonomialObj() 
		    << " " << endl);
            _children.push_back(tmp);
          }

        if (global()->getLowNodeMemory())
          {
            _posCovgIdx.clear();
            _negCovgIdx.clear();
	    DEBUGPR(20,ucout << "Clearing coverages (#4) for " << this 
		    << ' '  << _monom << '\n');
          }

        if (!_children.empty())
          {
            // order children
            sort(_children.begin(), _children.end(), maxMonomSubComp());
            bound =  (*_children.begin())->bound;
            //,getObjectiveVal()); // set to highest bound of a child
          }
        else
          setState(dead);

        DEBUGPR(9,ucout << "boundComputation finished for: " 
		<< getMonomialObj() << " bound: " << bound <<
                " num of children: " << _children.size() 
		<< " free variable num: " << freeVariables.size() << endl);

        setState(bounded);
        *controlParam = 1;
      }
    catch(std::bad_alloc const &)
      {
        cerr << "out of memory!" << endl;
        abort();
      }
    catch(exception & e)
      {
        cerr << "error caught: " << e.what() << " lineno: " 
	     << __LINE__ << endl;
        abort();
      }
    catch(...)
      {
        cerr << "unknown error caught: " << __LINE__ << endl;
        abort();
      }
  }


  // void maxMonomSubKWay::maxMonomSubFromData(maxMonomialData* master)

  void maxMonomSubKWay::maxMonomSubFromData(maxMonomialData* master)
  {
    maxMonomSub::maxMonomSubFromData(master);
    _monom = monomialObj(global()->attribNum(), NULL_VAL);
  }

  //maxMonomSubKWay::
  //  maxMonomSubAsChildOf(maxMonomSub* parent,vector<size_type> outOfMonomList)

  void maxMonomSubKWay::
  maxMonomSubAsChildOf(maxMonomSubKWay* parent, 
		       const vector<size_type> & outOfMonomList, 
		       double insepWt)
  {
    try 
      {
	globalPtr = parent->globalPtr;
	DEBUGPR(10,ucout << "maxMonomSubKWay:::maxMonomSubAsChildOf() "
		"with outOfMonomList  invoked, list size: " 
		<< outOfMonomList.size() << endl);
	_monom = parent->getMonomialObj();

	for (int i=0; i < outOfMonomList.size(); i++)
	  {
	    
	    DEBUGPR(40,ucout << " with attribute: " << outOfMonomList[i] 
		    << " setting to out of monom " << endl);
	    _monom.setOutOfMonom(outOfMonomList[i]);
	  }
    
	_posCovgIdx = parent->getPosCovgIdx();
	_negCovgIdx = parent->getNegCovgIdx();

	_posCovg = global()->getWeight(_posCovgIdx);
	_negCovg = global()->getWeight(_negCovgIdx);
       
        _insepWt = insepWt;
	bound = max(_posCovg, _negCovg) - _insepWt;
        
        if (global()->canFathom(bound))
          {
            setState(dead);
          }
	
	if (global()->getLowNodeMemory())
	  {
	    _posCovgIdx.clear(); // free memory
	    _negCovgIdx.clear();
	    DEBUGPR(20,ucout << "Clearing coverages (#5) for " << this 
		    << ' '  << _monom << '\n');
	  }

	DEBUGPR(10,ucout << "maxMonomSubKWay::maxMonomSubAsChildOf(), "
		"with outOfMonomList,  bound computed for: " 
		<< getMonomialObj() << " bound: " << bound 
		<< " _posCovg:" << _posCovg << " _negCovg:" 
		<< _negCovg << " insepWt: " << _insepWt << endl);
      }
    catch(exception & e)
      {
	cerr << e.what() << "child: 1st of 2k+1" 
	     << " passed to maxMonomSub::maxMonomSubAsChildOf, for monom: " 
	     << parent->getMonomialObj() << endl;
	abort();
      }
    catch (...)
      {
	abort();
      }

    DEBUGPR(20,ucout << "Out of maxMonomSub:::maxMonomSubAsChildOf() "
	    "with outOfMonomList" << endl);
  }

  // maxMonomSubKWay::maxMonomSubAsChildOf
  void maxMonomSubKWay::
  maxMonomSubAsChildOf(maxMonomSubKWay* parent,
		       int whichChild, 
		       const vector<size_type> & outOfMonomList, 
		       double insepWt)
  {
    globalPtr = parent->globalPtr;

    DEBUGPR(10,ucout << "maxMonomSubKWay:::maxMonomSubAsChildOf() invoked: "  
	    << whichChild << endl);

    try
      {
        const unsigned short VAR_VAL_NUM = 2;
        _monom = parent->getMonomialObj();

        div_t divResult = div(whichChild,VAR_VAL_NUM);
        size_type idx = divResult.quot;
        if (idx >= global()->attribNum())
          throw out_of_range("child num getNthMonomChild too large");

        vector<size_type>::const_iterator it;
        for (it = outOfMonomList.begin(); it != outOfMonomList.end(); it++)
          {
            DEBUGPR(40,ucout << " with attribute: " << *it 
		    << " setting to out of monom " << endl);
            _monom.setOutOfMonom(*it);
          }

        if (divResult.rem == 0)
          _monom.appendCompVar(idx);
        else if (divResult.rem == 1)
	  // set the variable now so it is cached as the 
	  // last set var (and not the out of monom)
          _monom.appendVar(idx);

        size_type lastSetIdx = _monom.lastSetIdx();
        variable_val_t val = _monom.getVarVal(lastSetIdx);

        vector<size_type> ignoreIdxs;
        for (size_type i = 0; i <= global()->attribNum(); i++)
	  {
	    if (_monom.getVarVal(i) == NOT_IN_MONOM)
	      // add all skipped variable indices to 
	      // "not in the monomial" vector
	      ignoreIdxs.push_back(i);
	  }

        assert(val != NULL_VAL);

        if (_monom.degree()==1) // no need for set intersection
          {
            _posCovgIdx = global()->getPosVarCovg(lastSetIdx,val);
            _negCovgIdx = global()->getNegVarCovg(lastSetIdx,val);
          }
        else
          {
            DEBUGPR(20,ucout << "maxMonomSub:::maxMonomSubAsChildOf() "
		    "before set_intersection lastSetIdx: " 
		    << lastSetIdx << endl);
            set_intersection(parent->getPosCovgIdx().begin(),
			     parent->getPosCovgIdx().end(), 
			     global()->getPosVarCovg(lastSetIdx,val).begin(), 
			     global()->getPosVarCovg(lastSetIdx,val).end(), 
			     inserter(_posCovgIdx,_posCovgIdx.end()));
            set_intersection(parent->getNegCovgIdx().begin(),
			     parent->getNegCovgIdx().end(), 
			     global()->getNegVarCovg(lastSetIdx,val).begin(), 
			     global()->getNegVarCovg(lastSetIdx,val).end(), 
			     inserter(_negCovgIdx,_negCovgIdx.end()));
          }

        _posCovg = global()->getWeight(_posCovgIdx);
        _negCovg = global()->getWeight(_negCovgIdx);
        
	//calculateInsepWeight(ignoreIdxs);
	//if (abs(_insepWt-insepWt) > EPS)
	//{
	//  DEBUGPR(20,ucout << "insepWt=" << insepWt 
	//          << " _insepWt=" << _insepWt << endl);
	//  assert(false);
	//}

        _insepWt = insepWt;
        bound = max(_posCovg, _negCovg) - _insepWt;

        if (_posCovg == parent->_posCovg && _negCovg == parent->_negCovg)
          {
            setState(dead); // do not need to consider if
            // conjoining with a literal makes no difference
          }
        //bound = max(_posCovg, _negCovg) - _insepWt;

        if (global()->canFathom(bound))
          {
            setState(dead);
          }
         
        if (state != dead)
	  {
	    monomialObj tmp = _monom;
	    _assocSoln = new maxMonomSolution(tmp);
	    _assocSoln->value = getObjectiveVal(); // set solution value
	    maxMonomSolution * soln = new maxMonomSolution(_assocSoln);
	    global()->foundSolution(soln);
	  }

        if (global()->getLowNodeMemory())
          {
            _posCovgIdx.clear(); // free memory
            _negCovgIdx.clear();
	    DEBUGPR(20,ucout << "Clearing coverages (#6) for " << this 
		    << ' '  << _monom << '\n');
          }

        DEBUGPR(10,ucout << "maxMonomSubKWay::maxMonomSubAsChildOf(), "
		"bound computed for: " << getMonomialObj() << " bound: " 
		<< bound << ", solution val: " << getObjectiveVal() 
		<< " _posCovg:" << _posCovg << " _negCovg:" 
		<< _negCovg << " insepWt: " << _insepWt << endl);
      }
    catch(exception & e)
      {
        cerr << e.what() << " child: " << whichChild 
	     << " passed to maxMonomSub::maxMonomSubAsChildOf, for monom: " 
	     << parent->getMonomialObj() << endl;
        abort();
      }
    catch (...)
      {
        cerr << "unknown exception caught" << endl;
        abort();
      }
    DEBUGPR(20,ucout << "Out of maxMonomSub:::maxMonomSubAsChildOf()" << endl);
  }


  void maxMonomSub::vlWriteDescription()
  {
    for (size_type i = 0; i < global()->attribNum(); i++)
      global()->verifyLogFile() << (int) _monom.getVarVal(i);
  }


  /////////////////////// maxMonomSolution ////////////////////////////////

  void maxMonomSolution::printContents(ostream& outStream) 
  {
    outStream << "monomial: ";
    _monom.printInMonom(outStream);
    outStream << endl;
  }


  maxMonomSolution::maxMonomSolution(maxMonomialData* global) :
    solution(global),
    _monom(global->attribNum())
  {
    typeId = 0;              // Only one solution type for this application
    sense = maximization;
   _seqIter = _monom.getVarIdxs().begin();
  }
  

  maxMonomSolution::maxMonomSolution(const monomialObj & monom) 
  {
    typeId = 0;               // Only one solution type for this application
    _monom = monom;
    sense = maximization;
    _seqIter = _monom.getVarIdxs().begin();
  }


  maxMonomSolution::maxMonomSolution(size_type numVars_) :
    _monom(numVars_)
  {
    typeId = 0;               // Only one solution type for this application
    sense = maximization;
   _seqIter = _monom.getVarIdxs().begin();
  }


  solution* maxMonomSolution::blankClone()
  {
    return new maxMonomSolution(_monom.maxDegree());
  }


  maxMonomSolution::maxMonomSolution(maxMonomSolution* toCopy) 
  { 
    copy(toCopy); 
  };


  void maxMonomSolution::copy(maxMonomSolution* toCopy)
  {
#ifdef __DEBUG
    cout << "maxMonomSolution::copy invoked" << endl;
#endif
    solution::copy(toCopy);
    _monom = toCopy->_monom;
    _seqIter = _monom.getVarIdxs().begin();
  }

  double maxMonomSolution::sequenceData()
  { 
#ifdef __DEBUG
    cout << "maxMonomSolution::sequenceData invoked for:" 
	 << _monom << " abs(retVal): " << *_seqIter+1 << endl;
#endif
    assert(_seqIter != _monom.getVarIdxs().end());
    double returnVal = (double)*_seqIter + 1.0;
    if (_monom.getVarVal(*_seqIter) == NOT_IN_MONOM)
      returnVal = -((double)*_seqIter + 1.0);   
    _seqIter++;
    return (double) returnVal; 
  }

  ::size_type maxMonomSolution::sequenceLength()
  {
#ifdef __DEBUG
    cout << "maxMonomSolution::sequenceLength invoked for:" 
	 << _monom << " length:" << _monom.degree() << endl;
#endif
    return _monom.degree();
  }
 
  void maxMonomSolution::sequenceReset()
  {
    _seqIter = _monom.getVarIdxs().begin();
  }


#ifdef ACRO_HAVE_MPI

  void maxMonomSolution::packContents(PackBuffer & outBuf)
  {
    _monom.pack(outBuf);
  }

  void maxMonomSolution::unpackContents(UnPackBuffer &inBuf)
  {
    _monom.unpack(inBuf);
  }

  int maxMonomSolution::maxContentsBufSize()
  {
    return (_monom.maxDegree() + 1)*sizeof(size_type) + 200;
  }
#endif

} // namespace pebblMonom
