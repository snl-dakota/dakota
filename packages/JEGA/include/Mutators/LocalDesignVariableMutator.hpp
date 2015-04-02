/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class LocalDesignVariableMutator.

    NOTES:

        See notes under section "Class Definition" of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Nov 10 12:07:44 2009 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the LocalDesignVariableMutator class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_LOCALDESIGNVARIABLEMUTATOR_HPP
#define JEGA_ALGORITHMS_LOCALDESIGNVARIABLEMUTATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <iosfwd>
#include <GeneticAlgorithmMutator.hpp>










/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/






/*
================================================================================
Namespace Aliases
================================================================================
*/








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {








/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class LocalDesignVariableMutator;







/*
================================================================================
Class Definition
================================================================================
*/


/// Mutates by random design variable reassignment.
/**
 * This mutator introduces random variation by first randomly choosing a design
 * variable of a randomly selected design and reassigning it to a random valid
 * value for that variable.  It then reassigns any next to it that have the same
 * previous value for as many as there are before one different is encountered.
 * No consideration of the current value is given when determining the new
 * value.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class LocalDesignVariableMutator :
    public GeneticAlgorithmMutator
{
    /*
    ============================================================================
    Nested Compounds
    ============================================================================
    */
    private:

        class roadmap
        {
            public:

                std::vector<std::pair<std::size_t, std::size_t> > _varMap;

                std::vector<std::size_t> _varEnds;

                std::vector<std::pair<std::size_t, std::size_t> > _optMap;

                std::vector<std::size_t> _optEnds;

                std::vector<std::vector<std::string> > _dvSuboptList;

                std::vector<std::map<
                    std::string, std::size_t
                    > > _dvSuboptIndexMap;

                std::map<std::string, std::vector<std::size_t> > _dateDVs;

                int _hiOptVar;

                std::size_t _numSCOpts;

                inline
                std::size_t
                GetNumDVs(
                    ) const
                {
                    if(!this->_varEnds.empty())
                        return this->_varEnds.back()+1;

                    return this->_numSCOpts +
                           this->_dvSuboptIndexMap.size();
                }

            public:

                roadmap(
                    ) :
                        _varMap(),
                        _optMap(),
                        _dvSuboptList(),
                        _dvSuboptIndexMap(),
                        _varEnds(),
                        _optEnds(),
                        _dateDVs(),
                        _hiOptVar(-1),
                        _numSCOpts(0)
                {}

                roadmap(
                    const roadmap& copy
                    ) :
                        _varMap(copy._varMap),
                        _optMap(copy._optMap),
                        _dvSuboptList(copy._dvSuboptList),
                        _dvSuboptIndexMap(copy._dvSuboptIndexMap),
                        _varEnds(copy._varEnds),
                        _optEnds(copy._optEnds),
                        _dateDVs(copy._dateDVs),
                        _hiOptVar(copy._hiOptVar),
                        _numSCOpts(copy._numSCOpts)
                {}
        };

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        std::vector<roadmap> _roadmaps;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the proper name of this operator.
        /**
         * \return The string "replace_uniform".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This mutator does mutation by first randomly selecting a Design.
            It then chooses a random design variable and reassigns that
            variable to a random valid value.

            The number of mutations is the rate times the size of the group
            passed in rounded to the nearest whole number.
           \endverbatim.
         *
         * \return A description of the operation of this operator.
         */
        static
        const std::string&
        Description(
            );

        /**
         * \brief Returns a new instance of this operator class for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which the new mutator is to be used.
         * \return A new, default instance of a LocalDesignVariableMutator.
         */
        static
        GeneticAlgorithmOperator*
        Create(
            GeneticAlgorithm& algorithm
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        void
        ReadDesignSpaceFile(
            const std::string& fileName
            );

        void
        ReadDesignSpaceFile(
            std::istream& iFile,
            const std::string& fileName
            );

        void
        ReadMultipleChoiceOptionSection(
            std::istream& iFile,
            roadmap& rMap,
            const std::string& fileName
            );

        void
        ReadSingleChoiceOptionSection(
            std::istream& iFile,
            roadmap& rMap,
            const std::string& fileName
            );

        void
        ReadVariableSection(
            std::istream& iFile,
            roadmap& rMap,
            const std::string& fileName
            );

        std::string
        TrimWhitespace(
            const std::string& str
            );

        void
        PerformFullBlockChangeMutation(
            JEGA::Utilities::Design& des,
            const roadmap& rMap,
            const std::size_t dv,
            const size_t pndv
            );

        void
        PerformMoveBy1Mutation(
            JEGA::Utilities::Design& des,
            const std::size_t dv
            );

        void
        PerformRandomReassignMutation(
            JEGA::Utilities::Design& des,
            const std::size_t dv
            );

        void
        PerformBlockExtensionMutation(
            JEGA::Utilities::Design& des,
            const roadmap& rMap,
            const std::size_t dv,
            const size_t pndv
            );

        void
        PerformVerticalPairFullBlockChangeMutation(
            JEGA::Utilities::Design& des,
            const roadmap& rMap,
            const std::size_t pndv
            );

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Performs the local mutation algorithm.
        /**
         * This mutator only mutates members of the population but places the
         * resulting mutated designs into the children.
         *
         * \param pop The population of designs to which to apply mutation.
         * \param cldrn The group of child designs to which to apply mutation.
         */
        virtual
        void
        Mutate(
            JEGA::Utilities::DesignGroup& pop,
            JEGA::Utilities::DesignGroup& cldrn
            );

        /// Returns the proper name of this operator.
        /**
         * \return See Name().
         */
        virtual
        std::string
        GetName(
            ) const;

        /// Returns a full description of what this operator does and how.
        /**
         * \return See Description().
         */
        virtual
        std::string
        GetDescription(
            ) const;

        /**
         * \brief Creates and returns a pointer to an exact duplicate of this
         *        operator.
         *
         * \param algorithm The GA for which the clone is being created.
         * \return A clone of this operator.
         */
        virtual
        GeneticAlgorithmOperator*
        Clone(
            GeneticAlgorithm& algorithm
            ) const;

        virtual
        bool
        CanProduceInvalidVariableValues(
            ) const;

    protected:

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
         *
         * This version extracts the design space map file name and reads the
         * file.  It then calls the base class version.
         *
         * \param db The database of parameters from which the configuration
         *           information can be retrieved.
         * \return true if the extraction completed successfully and false
         *         otherwise.
         */
        virtual
        bool
        PollForParameters(
            const JEGA::Utilities::ParameterDatabase& db
            );


    private:





    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        void
        ReadFor(
            std::istream& iFile,
            std::string& line,
            const std::string& fileName
            );

        std::string&
        ReadSingleChoiceOptionData(
            std::istream& iFile,
            std::string& line,
            roadmap& rMap,
            const std::string& fileName
            );

        std::string&
        ReadMultipleChoiceOptionData(
            std::istream& iFile,
            std::string& line,
            roadmap& rMap,
            const std::string& fileName
            );

        std::string&
        ReadVariableData(
            std::istream& iFile,
            std::string& line,
            roadmap& rMap,
            const std::string& fileName
            );

        void
        PerformFullBlockChangeMutation(
            JEGA::Utilities::Design& des,
            const roadmap& rMap,
            const std::size_t dv,
            const std::size_t pndv,
            double oldRep,
            double newRep
            );

        std::string
        ReadDateDVInfo(
            const std::string& line,
            roadmap& rm
            );

        double
        GenerateNewRep(
            JEGA::Utilities::Design& des,
            const roadmap& rm,
            const std::size_t dv,
            const std::size_t pndv
            ) const;

        std::size_t
        GetTotalNumTecOpts(
            ) const;

        std::size_t
        GetTotalNumTecVars(
            ) const;

        std::size_t
        GetPriorNDV(
            const roadmap& rm
            ) const;

        std::pair<const roadmap*, std::size_t>
        GetRoadmapAndPriorNDV(
            std::size_t dvNum
            );

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a LocalDesignVariableMutator for use by \a algorithm.
        /**
         * \param algorithm The GA for which this mutator is being constructed.
         */
        LocalDesignVariableMutator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a LocalDesignVariableMutator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        LocalDesignVariableMutator(
            const LocalDesignVariableMutator& copy
            );

        /**
         * \brief Copy constructs a LocalDesignVariableMutator for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this mutator is being constructed.
         */
        LocalDesignVariableMutator(
            const LocalDesignVariableMutator& copy,
            GeneticAlgorithm& algorithm
            );

}; // class LocalDesignVariableMutator


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA








/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/LocalDesignVariableMutator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_LOCALDESIGNVARIABLEMUTATOR_HPP
