/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once

#include <memory>
#include <boost/filesystem/path.hpp>
#include "PRPMultiIndex.hpp"

namespace bfs = boost::filesystem;

namespace Dakota {

class Response;

/// exception class for function evaluation failures from batch
class FunctionBatchEvalFailure: public FunctionEvalFailure
{
public:
  FunctionBatchEvalFailure(const std::string& msg, const ParamResponsePair& in_prp, Response& in_response): 
	  FunctionEvalFailure(msg),
	  prp(in_prp),
	  response(in_response)
  { /* empty ctor */ }
  const ParamResponsePair& prp;
  Response& response; 
};

class ResultsFileReader {
    public:
	/// Get a pointer to an appropriate derived class of ResultsfileReader
        static std::unique_ptr<ResultsFileReader> get_reader(unsigned short results_file_format, bool labeled);
    
	/// Read results for a single evaluation
        virtual void read_results_file(Response& response, const bfs::path &results_path, const int id) const = 0;
	/// Read results for a batch of evaluations
        virtual void read_results_file(PRPQueue& prp_queue, const std::string &results_path, const int batch_id, IntSet& completion_set) const = 0;

    protected:
	/// Hidden constructor; the static get_reader function should be used.
        ResultsFileReader(bool labeled);

        /// Expect labeled (dakota format) results file
        bool labeledFlag;
};

    
}
