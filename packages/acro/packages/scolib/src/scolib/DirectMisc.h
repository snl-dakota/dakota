/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

/**
 * \file DirectMisc.h
 *
 * Misc utilities used with colin::Direct.
 */

//
// Coliny_DirectMisc.h
//

#ifndef scolib_DirectMisc_h
#define scolib_DirectMisc_h

#include <acro_config.h>
#include <colin/AppResponse.h>
#include <colin/real.h>
#include <utilib/std_headers.h>
#include <utilib/stl_auxiliary.h>
#ifdef ACRO_HAVE_MPI
#include <mpi.h>
#endif

namespace scolib {

namespace direct {

using utilib::max;
using colin::real;
using std::sqrt;

template <class VectorT>
inline double l2_norm(VectorT& x)
{
size_type len=x.size();
double z=0.0;
for (size_type i=0; i<len; i++)
  z += x[i]*x[i];
return sqrt(z);
}


class boxsort 
  {
    public:
    	unsigned int numb;
    	double o1,o2,o3;
    
   	boxsort()
    	   {numb = 0; o1 = 0; o2 = 0; o3 = 0; }
    	boxsort(int a, double b, double c, double d)
    	   {numb = a; o1 = b; o2 = c; o3 = d; }
      
    	friend bool operator<(const boxsort &b1, const boxsort &b2);
   	friend bool operator>(const boxsort &b1, const boxsort &b2);
   	friend bool operator==(const boxsort &b1, const boxsort &b2);
    	friend bool operator!=(const boxsort &b1, const boxsort &b2);
    
  };
  
inline bool operator<(const boxsort &b1,const boxsort &b2)
  {
    if (b1.o2 != b2.o2)
       return b1.o2 < b2.o2;
    else if ((b1.o1 != b2.o1))
       return b1.o1 < b2.o1;
    else
       return b1.o3 > b2.o3;
  }

inline bool operator>(const boxsort &b1,const boxsort &b2)
  {
    if (b1.o2 != b2.o2)
       return b1.o2 > b2.o2;
    else if ((b1.o1 != b2.o1))
       return b1.o1 > b2.o1;
    else 
       return b1.o3 < b2.o3;
  }

inline bool operator==(const boxsort &b1,const boxsort &b2)
  {
    return ((b1.o3 == b2.o3) && (b1.o2 == b2.o2) && (b1.o1 == b2.o1));
  }

inline bool operator!=(const boxsort &b1,const boxsort &b2)
  {
    return ((b1.o3 != b2.o3) || (b1.o2 != b2.o2) || (b1.o1 != b2.o1));
  }



class point 
{
public:

  point()
 	{
	Pt_finished = false;
	parent_finished = false;
	feasible=true;
        size = 0.0;
	//cVal = 0.0;
	}

   point(int n, int m)
  	{
	x.resize(n);
	l.resize(n);
	Pt_finished = false;
	parent_finished = false;
	feasible=true;
        size = 0.0;
	//cVal = 0.0;
	}

  point(double v, std::vector<double> x1, std::vector<double> l1)
  	{
	Val = v;
	x = x1;
	l = l1;
	Pt_finished = false;
	parent_finished = false;
	feasible=true;
        size = 0.0;
	//cVal = 0.0;
	}

  point(const point &p)
	{
	*this = p;
	}

  point& operator=(const point& p)
	{
	Val = p.Val;
        response = p.response;
	//fVal = p.fVal;
        //cVal = p.cVal;
	x = p.x;
	l = p.l;
	feasible=p.feasible;
  	Pt_finished = false;
	parent_finished = false;
        size = p.size;
  	return *this;
	}
	
  ///
  void reset()
	{size = l2_norm(l);}

  ///
  double size;

  ///
  double max_dimension()
	{ return max(l); }

  /// Application response for this point
  colin::AppResponse response;

  ///
  real constraint(int i)
	{
	std::vector<real> cvals;
	response.get(colin::cf_info,cvals);
	return cvals[i];
	}

  /// Value of this point
  real Val;

  /// The L2 norm of the constraint value violations
  //real cVal;

  /// The function value for this point
  //real fVal;

  /// Center point of this box
  std::vector<double> x;

  /// Box edge length about this point
  std::vector<double> l;
    
  ///
  bool feasible;

  ///
  bool Pt_finished;

  ///
  bool parent_finished;

  ///
  void write(std::ostream& os) const
	{
	os << "Val   " << Val << std::endl;
	//os << "cVal  " << cVal << std::endl;
	os << "x     " << x << std::endl;
	os << "l     " << l << std::endl;
        os << "size: " << size << std::endl;
	//std::vector<real> cvals;
	//response->get(colin::cf_info,cvals);
	//os << "c     " << cvals << std::endl;
        os << "fsbl: " << (int)feasible << std::endl;
	}

};
  


//
// Used to sort boxes by size and then value
//
class simple_sort
{
public:

  simple_sort(unsigned int o, std::vector<point>* center_)
    	: order(o), center(center_) {}
    	  
  /// Index into the list of boxes
  unsigned int order;

  ///
  std::vector<point>* center;

  ///
  double size() const
	{return (*center)[order].size;}

  double Val() const
	{return (*center)[order].Val;}
};
  

inline bool operator<(const simple_sort &b1,const simple_sort &b2)
{
if (b1.size() != b2.size())
   return b1.size() < b2.size();
else
   return b1.Val() < b2.Val();
}

inline bool operator>(const simple_sort &b1,const simple_sort &b2)
{
if (b1.size() != b2.size())
   return b1.size() > b2.size();
else
   return b1.Val() > b2.Val();
}

inline bool operator==(const simple_sort &b1,const simple_sort &b2)
{
return ((b1.size() == b2.size()) && (b1.Val() == b2.Val()) && 
        (b1.order == b2.order));
}

inline bool operator!=(const simple_sort &b1,const simple_sort &b2)
{
return ((b1.size() != b2.size()) || (b1.Val() != b2.Val()) || 
	(b1.order != b2.order));
}

  
class NeighborHood
{
public:

  NeighborHood()
	{}

  NeighborHood(std::vector<double>& v, int nn[3], double d)
	{V = v; n[0] = nn[0]; n[1] = nn[1]; n[2] = nn[2]; dist = d;}

  NeighborHood(const NeighborHood &N)
  	{*this = N;}

  NeighborHood& operator=(const NeighborHood &N)
  	{V = N.V; n[0] = N.n[0]; n[1] = N.n[1]; n[2] = N.n[2]; dist = N.dist;
  	 return *this;}
  	   
  std::vector<double> V;

  unsigned int n[3];

  double dist;
};
 
inline bool operator<(const NeighborHood &b1,const NeighborHood &b2)
{ return (b1.dist < b2.dist); }

inline bool operator>(const NeighborHood &b1,const NeighborHood &b2)
{ return (b1.dist > b2.dist); }

inline bool operator==(const NeighborHood &b1,const NeighborHood &b2)
{ return (b1.dist == b2.dist); }

inline bool operator!=(const NeighborHood &b1,const NeighborHood &b2)
{ return (b1.dist != b2.dist); }

  
struct bvector
{
bool is_p_basis;
std::vector<double> V1;
};
  
  
struct vec
{
std::vector<double> V;
};
    
  
struct mesg
{
///
std::vector<double> send;

#ifdef ACRO_HAVE_MPI
///
MPI_Request check;
#endif
};
    
}

}

#endif
