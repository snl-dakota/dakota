#include "BinaryIO_Helper.hpp"

namespace Dakota {

// initialization of statics

short H5VariableString::numVStrUses = 0;
//hid_t H5VariableString::varStringType = -1;
hid_t H5VariableString::varStringType = H5VariableString::datatype();

} // namespace Dakota
