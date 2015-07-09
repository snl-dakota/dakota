#include "dakota_stat_util.hpp"

namespace Dakota {

BootstrapSampler<RealMatrix>::BootstrapSampler(uint data_size,
                                               const RealMatrix& orig_data) :
              BootstrapSamplerBase<RealMatrix>::BootstrapSamplerBase(data_size),
              origData(Teuchos::View, orig_data, orig_data.numRows(),
                       orig_data.numCols())
{

}

BootstrapSampler<RealMatrix>::~BootstrapSampler()
{
    /* empty destructor */
}

void BootstrapSampler<RealMatrix>::operator()(uint num_samp,
                                              RealMatrix& boostrapped_sample)
{
  if(num_samp > dataSize)
  {
    std::cout << "Warning: number of boostrap samples requested is larger"
              << "than the data size used for the empirical distribution."
              << std::endl;
  }

  if(num_samp != boostrapped_sample.numCols() ||
     origData.numRows() != boostrapped_sample.numRows())
  {
    boostrapped_sample.shapeUninitialized(origData.numRows(), num_samp);
  }

  for(int i = 0; i < num_samp; ++i)
  {
    Teuchos::setCol(Teuchos::getCol(Teuchos::View, origData,
                                    boostrapSampler(bootstrapRNG)),
                    i, boostrapped_sample);
  }
}

RealMatrix BootstrapSampler<RealMatrix>::operator()(uint num_samp)
{
  if(num_samp > dataSize)
  {
    std::cout << "Warning: number of boostrap samples requested is larger"
              << "than the data size used for the empirical distribution."
              << std::endl;
  }

  RealMatrix bootstrappedSample(origData.numRows(), num_samp, false);

  for(int i = 0; i < num_samp; ++i)
  {
    Teuchos::setCol(Teuchos::getCol(Teuchos::View, origData,
                                    boostrapSampler(bootstrapRNG)),
                    i, bootstrappedSample);
  }

  return bootstrappedSample;
}

}
