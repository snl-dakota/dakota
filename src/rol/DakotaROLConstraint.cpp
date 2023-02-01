#include "DakotaROLConstraint.hpp"

namespace Dakota {
namespace rol_interface { 

Constraint::Constraint( const Ptr<Cache>&                      cache,
                              Dakota::CONSTRAINT_EQUALITY_TYPE type ) 
  : modelCache(cache), conType(type) {
  if( conType == Dakota::CONSTRAINT_EQUALITY_TYPE::EQUALITY ) {

  }
  else {

  }
}

void Constraint::update( const RealVector& x,
                               UpdateType  type,
                               int         iter ) {
  modelCache->update(c,type,iter);

} // Constraint::update  

void Constraint::value(       RealVector& c,
                        const RealVector& x,
                              UpdateType  type,
                              int         iter ) {
  auto& model = cache->dakotaModel;
   
} // Constraint::value




} // namespace rol_interface
} // namespace Dakota
