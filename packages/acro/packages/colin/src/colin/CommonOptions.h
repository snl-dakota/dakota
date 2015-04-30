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

#ifndef colin_CommonOptions_h
#define colin_CommonOptions_h

#include <colin/real.h>
#include <utilib/PropertyDict.h>
#include <utilib/PropertyValidators.h>

namespace colin {

///
/// A class that defines commonly used options.  By default, these are 
/// disabled.  But they can be enabled within the class that 
/// inherits from this class.
///
class CommonOptions
{
   // WORKAROUND: pgCC 9 apparently cannot automatically identify the
   // type of the pointer to a global template function.  As a result,
   // we will explicitly cast the validation callback pointers to their
   // actual function pointer type.  Note that this does not appear to
   // be a problem with pgCC 10, and when we deprecate support for pgCC
   // 9, we should also remove these explicit casts.
   typedef bool(*validate_fcn_t)( const utilib::ReadOnly_Property&, 
                                  const utilib::Any& );

public:

    ///
    CommonOptions(utilib::PropertyDict& properties)
    {
    debug=0;
    {
       utilib::Privileged_Property p(debug);
       // See WORKAROUND note above
       p.validate().connect( static_cast<validate_fcn_t>
                             (&utilib::PropertyValidators::Nonnegative<int>) );
       p.set_readonly();
       properties.declare( "debug", p );
    }
    //option.add("debug",debug, "");
    //option.validate("debug",debug,utilib::ParameterLowerBound<int>(0));
    //option.disable("debug");

    max_neval=0;
    {
       utilib::Privileged_Property p(max_neval);
       // See WORKAROUND note above
       p.validate().connect( static_cast<validate_fcn_t>
                             (&utilib::PropertyValidators::Nonnegative<int>) );
       p.set_readonly();
       properties.declare( "max-neval", p );
    }
    //option.add("max-neval",max_neval, "");
    //option.validate("max_neval",debug,utilib::ParameterLowerBound<int>(0));
    //option.disable("max-neval");

    max_neval_curr=0;
    {
       utilib::Privileged_Property p(max_neval_curr);
       // See WORKAROUND note above
       p.validate().connect( static_cast<validate_fcn_t>
                             (&utilib::PropertyValidators::Nonnegative<int>) );
       p.set_readonly();
       properties.declare( "max-neval-curr", p );
    }
    //option.add("max-neval-curr",max_neval_curr, "");
    //option.validate("max_neval_curr",debug,utilib::ParameterLowerBound<int>(0));
    //option.disable("max-neval-curr");

    max_iters=0;
    {
       utilib::Privileged_Property p(max_iters);
       // See WORKAROUND note above
       p.validate().connect( static_cast<validate_fcn_t>
                             (&utilib::PropertyValidators::Nonnegative<int>) );
       p.set_readonly();
       properties.declare( "max-iters", p );
    }
    //option.add("max-iters",max_iters, "");
    //option.validate("max_iters",debug,utilib::ParameterLowerBound<int>(0));
    //option.disable("max-iters");

    max_time=0.0;
    {
       utilib::Privileged_Property p(max_time);
       // See WORKAROUND note above
       p.validate().connect(static_cast<validate_fcn_t>
                            (&utilib::PropertyValidators::Nonnegative<double>));
       p.set_readonly();
       properties.declare( "max-time", p );
    }
    //option.add("max-time",max_time, "");
    //option.validate("max_time",max_time,utilib::ParameterLowerBound<double>(0.0));
    //option.disable("max-time");

    accuracy=colin::real::negative_infinity;
    {
       utilib::Privileged_Property p(accuracy);
       p.set_readonly();
       properties.declare( "sufficient_objective_value", p );
    }
    //option.add("accuracy",accuracy, "");
    //option.alias("accuracy", "sufficient_objective_value");
    //option.disable("accuracy");

    seed=0;
    properties.declare( "seed", utilib::Privileged_Property(seed) );
    //option.add("seed",seed, "");
    // This is enabled, by default, since the EXACT testing framework assumes
    // that optimizers can have their random number seed set

    double mcheps = DBL_EPSILON;
    constraint_tolerance=sqrt(mcheps);
    {
       utilib::Privileged_Property p(constraint_tolerance);
       // See WORKAROUND note above
       p.validate().connect(static_cast<validate_fcn_t>
                            (&utilib::PropertyValidators::Nonnegative<double>));
       p.set_readonly();
       properties.declare( "constraint_tolerance", p );
    }
    //option.add("constraint_tolerance",constraint_tolerance, "");
    //option.validate("constraint_tolerance",constraint_tolerance,utilib::ParameterLowerBound<double>(0.0));
    //option.disable("constraint_tolerance");
    }

    ///
    int debug;

    ///
    int max_neval;

    ///
    int max_neval_curr;

    ///
    double max_time;

    ///
    int max_iters;

    ///
    colin::real accuracy;

    ///
    double constraint_tolerance;

    ///
    int seed;

};

}

#endif
