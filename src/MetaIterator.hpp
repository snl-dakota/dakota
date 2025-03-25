/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef META_ITERATOR_H
#define META_ITERATOR_H

#include "dakota_data_types.hpp"
#include "DakotaIterator.hpp"
#include "DakotaModel.hpp"
#include "IteratorScheduler.hpp"


namespace Dakota {


/// Base class for meta-iterators.

/** This base class shares code for concurrent and hybrid
    meta-iterators, where the former supports multi-start and Pareto
    set iteration and the latter supports sequential, embedded, and
    collaborative hybrids. */

class MetaIterator: public Iterator
{
public:

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize() override;
  
protected:
  
  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  MetaIterator(ProblemDescDB& problem_db);
  /// alternate constructor
  MetaIterator(ProblemDescDB& problem_db, std::shared_ptr<Model> model);
  /// destructor
  ~MetaIterator() override;

  //
  //- Heading: Virtual function redefinitions
  //

  void post_run(std::ostream& s) override;

  //
  //- Heading: Convenience member functions
  //

  /// check that a model identified by pointer has the same id as the
  /// iteratedModel passed through the ctor chain
  void check_model(const String& method_ptr, const String& model_ptr);

  /// initialize the_iterator and the_model based on method_ptr
  void allocate_by_pointer(const String& method_ptr, std::shared_ptr<Iterator>& the_iterator,
			   std::shared_ptr<Model>& the_model);
  /// initialize the_iterator based on method_string
  void allocate_by_name(const String& method_string, const String& model_ptr,
			std::shared_ptr<Iterator>& the_iterator,	std::shared_ptr<Model>& the_model);

  /// estimate minimum and maximum processors per iterator needed for
  /// init_iterator_parallelism(); instantiates the_iterator and the_model
  /// as needed, but on minimal processor ranks (is later augmented by
  /// allocate_by_pointer())
  std::pair<int, int> estimate_by_pointer(const String& method_ptr,
					  std::shared_ptr<Iterator>& the_iterator,
					  std::shared_ptr<Model>& the_model);
  /// estimate minimum and maximum processors per iterator needed for
  /// init_iterator_parallelism(); instantiates the_iterator and the_model
  /// as needed, but on minimal processor ranks (is later augmented by
  /// allocate_by_name())
  std::pair<int, int> estimate_by_name(const String& method_string,
				       const String& model_ptr,
				       std::shared_ptr<Iterator>& the_iterator,
				       std::shared_ptr<Model>& the_model);

  //
  //- Heading: Data members
  //

  /// scheduler for concurrent execution of Iterators
  IteratorScheduler iterSched;

  /// maximum number of concurrent sub-iterator executions
  int maxIteratorConcurrency;

private:

  //
  //- Heading: Convenience member functions
  //

};

} // namespace Dakota

#endif
