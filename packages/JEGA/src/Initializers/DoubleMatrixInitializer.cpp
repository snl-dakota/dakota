/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implemenation of class DoubleMatrixInitializer.

    NOTES:

        See notes of DoubleMatrixInitializer.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Wed Jul 12 12:47:50 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the DoubleMatrixInitializer class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <Initializers/DoubleMatrixInitializer.hpp>
#include <Initializers/RandomUniqueInitializer.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>





/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace JEGA::Utilities;








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {








/*
================================================================================
Static Member Data Definitions
================================================================================
*/








/*
================================================================================
Mutators
================================================================================
*/

void
DoubleMatrixInitializer::SetDesignMatrix(
    const JEGA::DoubleMatrix& designs
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_designs = designs;

    JEGAIFLOG_CF_II(this->_designs.empty(), this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(), this->GetName() + ": An empty matrix has been "
        "supplied to the double matrix initializer.")
        )

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The design matrix has "
        "been assigned and now contains ") << this->_designs.size()
        << " entries."
        )

}








/*
================================================================================
Accessors
================================================================================
*/








/*
================================================================================
Public Methods
================================================================================
*/

const string&
DoubleMatrixInitializer::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("double_matrix");
    return ret;
}

const string&
DoubleMatrixInitializer::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This initializer creates JEGA Design class structures from the "
        "numerical values contained in a double matrix.  The values must be "
        "organized such that the first \"NDV\" values are the design "
        "variables, the next \"NOF\" values are the objective functions and "
        "the last \"NCN\" values are the constraints.  The objectives and "
        "constraints are not required but if ALL are supplied, they will be "
        "recorded and the resulting Design will be labeled evaluated and "
        "evaluators may then choose not to re-evaluate them.  Note that the "
        "double matrix is a vector of vectors and thus not all entries need "
        "to have the same length.  So it is possible to have some evaluated "
        "and some non-evaluated designs in the same matrix."
        );
    return ret;
}

GeneticAlgorithmOperator*
DoubleMatrixInitializer::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new DoubleMatrixInitializer(algorithm);
}







/*
================================================================================
Subclass Visible Methods
================================================================================
*/
bool
DoubleMatrixInitializer::ReadDesignValues(
    Design& into,
    const DoubleVector& from
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // We will need the counts of variables, objectives, and constraints.
    // we can get them from the target.
    const DesignTarget& target = into.GetDesignTarget();

    const size_t ndv = target.GetNDV();
    const size_t nof = target.GetNOF();
    const size_t ncn = target.GetNCN();

    // If the size of the vector is too small for the number of design
    // variables, then we can't use it.
    if(from.size() < ndv) return false;

    // prepare to iterate the vector
    DoubleVector::const_iterator it(from.begin());

    for(size_t dv=0; dv<ndv && it!=from.end(); ++it, ++dv)
        into.SetVariableValue(dv, *it);

    // if the size of the vector is such that all constraints and objectives
    // are present, then use them.
    if(from.size() >= (ndv + nof + ncn))
    {
        for(size_t of=0; of<nof && it!=from.end(); ++it, ++of)
            into.SetVariableValue(of, *it);

        for(size_t cn=0; cn<ncn && it!=from.end(); ++it, ++cn)
            into.SetVariableValue(cn, *it);

        into.SetEvaluated(true);
        target.RecordAllConstraintViolations(into);
    }
    else into.SetEvaluated(false);

    return true;
}




/*
================================================================================
Subclass Overridable Methods
================================================================================
*/

string
DoubleMatrixInitializer::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DoubleMatrixInitializer::Name();
}

string
DoubleMatrixInitializer::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DoubleMatrixInitializer::Description();
}

GeneticAlgorithmOperator*
DoubleMatrixInitializer::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new DoubleMatrixInitializer(*this, algorithm);
}


void
DoubleMatrixInitializer::Initialize(
    DesignGroup& into
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Performing initialization.")
        )

    // get the target for the designs we create.
    DesignTarget& target = into.GetDesignTarget();

    // find out the requested minimum number of Designs
    const std::size_t minsize = this->GetSize();

    JEGA_LOGGING_IF_ON(
        const std::size_t prevSize = into.GetSize();
        )

    // For each vector in the matrix, create and read a design.  Keep track
    // of how many are insufficient for reading.
    size_t failedReads = 0;

    for(DoubleMatrix::const_iterator it(this->_designs.begin());
        it!=this->_designs.end(); ++it)
    {
        Design* des = target.GetNewDesign();
        if(!ReadDesignValues(*des, *it))
        {
            ++failedReads;
            target.TakeDesign(des);
        }
        else into.Insert(des);
    }

    JEGAIFLOG_CF_II(failedReads > 0, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(), this->GetName() + ": ") << failedReads <<
            " entries in the design matrix had fewer elements than "
            "the required number of design variables and thus failed "
            "to result in a design."
        )

    JEGAIFLOG_CF_II(into.IsEmpty(), this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": No designs were read from "
            "the design matrix.  All initial designs will be "
            "generated by the random unique initializer.")
        )

    JEGAIFLOG_CF_II(!into.IsEmpty(), this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(), this->GetName() + ": ") << into.SizeDV() <<
            " total designs were were read from the design matrix."
        )

    // now we have to make sure that we have enough according to the
    // requested minimum size.
    if(into.GetSize() < minsize)
    {
        // we will use the random unique initializer to fill
        // in the rest.

        JEGALOG_II(this->GetLogger(), lverbose(), this,
            ostream_entry(lverbose(), this->GetName() + ": Only ")
                << into.GetSize() << " of " << minsize
                << " of the desired number of designs were found in the "
                "supplied matrix.  Using the random unique initializer to "
                "generate the remaining "
                << (minsize - into.GetSize()) << " designs."
            )


        RandomUniqueInitializer subiniter(GetAlgorithm());
        subiniter.SetSize(minsize - into.GetSize());
        subiniter.Initialize(into);
    }

    JEGAIFLOG_CF_II_F(into.IsEmpty(), this->GetLogger(), this,
        text_entry(lfatal(), this->GetName() + ": Unable to generate any "
                   "initial designs even with the help of the random unique "
                   "initializer.")
        )

    JEGAIFLOG_CF_II_F(into.GetSize() < 2, this->GetLogger(), this,
        text_entry(lfatal(), this->GetName() + ": Unable to generate minimum "
                   "of 2 initial designs even with the help of the random "
                   "unique initializer.")
        )

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": ")
            << (into.GetSize() - prevSize) << " total designs read or created."
        )

    JEGALOG_II(this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(), this->GetName() +
            ": Final initial population size: ") << into.GetSize() << "."
        )

    // Set the size to the number of successful reads.
    this->SetSize(into.GetSize());
}

bool
DoubleMatrixInitializer::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetDoubleMatrixFromDB(
        db, "method.jega.design_matrix", _designs
        );

    JEGAIFLOG_CF_II(!success || _designs.empty(), this->GetLogger(), lquiet(),
        this, text_entry(lquiet(), GetName() + ": No design matrix or empty "
            "design matrix obtained for initialization.")
        )

    // allow this to pass through and perhaps a matrix will be supplied at a
    // later time.  If not, it is up to Initialize to deal with this.

    return this->GeneticAlgorithmInitializer::PollForParameters(db);
}






/*
================================================================================
Private Methods
================================================================================
*/






/*
================================================================================
Structors
================================================================================
*/

DoubleMatrixInitializer::DoubleMatrixInitializer(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmInitializer(algorithm),
        _designs()
{
    EDDY_FUNC_DEBUGSCOPE

    // override the default requested initial size to zero.
    // For this initializer, this is the min size.  A value of
    // 0 will result in using only the Designs found in the matrix.
    this->SetSize(0);
}

DoubleMatrixInitializer::DoubleMatrixInitializer(
    const DoubleMatrixInitializer& copy
    ) :
        GeneticAlgorithmInitializer(copy),
        _designs(copy._designs)
{
    EDDY_FUNC_DEBUGSCOPE
}

DoubleMatrixInitializer::DoubleMatrixInitializer(
    const DoubleMatrixInitializer& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmInitializer(copy, algorithm),
        _designs(copy._designs)
{
    EDDY_FUNC_DEBUGSCOPE
}








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA
