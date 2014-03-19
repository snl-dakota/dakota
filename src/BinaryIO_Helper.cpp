#include "BinaryIO_Helper.hpp"

namespace Dakota {

// initialization of statics

short H5VariableString::numVStrUses = 0;
//hid_t H5VariableString::varStringType = -1;
hid_t H5VariableString::varStringType = H5VariableString::datatype();

#if 0  // WJB: Templated design seems to lead to ambiquities and runtime errors
// stub so storing vector<RealMatrix> will compile
template <>
herr_t HDF5BinaryStream::store_data(const std::string& dset_name,
				    const std::vector<RealMatrix>& buf) const
{
  throw "No implementation for vector<RealMatrix>!";
}

// stub so storing vector<IntVector> will compile
template <>
herr_t HDF5BinaryStream::store_data(const std::string& dset_name,
				    const std::vector<IntVector>& buf) const
{
  throw "No implementation for vector<IntVector>!";
}

// stub so storing vector<RealVector> will compile
template <>
herr_t HDF5BinaryStream::store_data(const std::string& dset_name,
				    const std::vector<RealVector>& buf) const
{
    if ( buf.empty() && exitOnError )
      throw BinaryStream_StoreDataFailure();

    // WJB: much of the "chunking setup" code is nearly identical to the
    // std::vector version, so re-factor for reuse next sprint

    // Create a 2D dataspace sufficient to store first row vector
    std::vector<hsize_t> dims(2);
    dims[0] = buf.size(); dims[1] = buf[0].length(); // WJB: fcnPtr vs PassIn?

    std::vector<hsize_t> max_dims = initializeUnlimitedDims<2>();

    // Create the memory space...
    hid_t mem_space = H5Screate_simple( dims.size(), dims.data(),
                        max_dims.data() );

    // Enable chunking using creation properties
    // Chunk size is that of the longest row vector
    int num_cols = dims[1];
    for(int i=0; i<buf.size(); ++i) {
      num_cols = std::max( num_cols, buf[i].length() ); // WJB: "generic" possible?
    }

    std::vector<hsize_t> chunk_dims = dims;
    chunk_dims[0] = 1; chunk_dims[1] = num_cols;

    hid_t cparms = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_layout(cparms, H5D_CHUNKED);
    H5Pset_chunk( cparms, chunk_dims.size(), chunk_dims.data() );

    // Create a new dataset within the DB using cparms creation properties.

    // size() vs length() and 'size_type' vs 'ScalarType' prevents writing a
    // generic algorithm, common to both std::vector and Teuchos vector
    hid_t dataset = H5Dcreate(binStreamId, dset_name.c_str(),
                      NativeDataTypes<double>::datatype(), mem_space,
                      H5P_DEFAULT, cparms, H5P_DEFAULT);

    herr_t status;
    for(int i=0; i<dims[0]; ++i) {
      std::vector<double> tmp( dims[1] );
      for(int j=0; j<dims[1]; ++j)
        tmp[j] = buf[i][j];

      status = append_data_slab( dset_name, i, tmp.data(), buf[i].length() );
    }

    // WJB - ToDo: free resources
    H5Sclose(mem_space);

    //std::cout << std::endl; 
}


// stub so storing vector<vector<string> > will compile
template <>
herr_t HDF5BinaryStream::
store_data(const std::string& dset_name,
	   const std::vector<std::vector<std::string> >& buf) const
{
  throw "No implementation for vector<vector<string> >!";

}
#endif

} // namespace Dakota
