/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "StandardParametersFileWriter.hpp"

#include <iomanip>
#include <string>

namespace Dakota {

void StandardParametersFileWriter::write_evaluation_to_stream(
    const Variables& vars, const ActiveSet& set, const Response& response,
    const std::vector<std::string>& an_comps, const StringArray& asv_labels,
    const StringArray& dvv_labels, const StringArray& ac_labels,
    const StringArray& md_tags, std::string full_eval_id,
    std::ofstream& parameter_stream) const {
  const ShortArray& asv = set.request_vector();
  const SizetArray& dvv = set.derivative_vector();
  const StringArray& md_labels = response.shared_data().metadata_labels();
  size_t i, asv_len = asv.size(), dvv_len = dvv.size(),
            ac_len = an_comps.size(), md_len = md_labels.size();

  using std::setw;
  int prec = write_precision;  // for restoration
  // write_precision = 16; // 17 total digits: full double precision
  write_precision = 15;  // 16 total digits: preserves desirable roundoff in
                         //                  last digit
  int w = write_precision + 7;

  std::string sp22("                     ");
  parameter_stream << sp22 << setw(w) << vars.tv() << " variables\n"
                   << vars << sp22 << setw(w) << asv_len << " functions\n";
  //<< setiosflags(ios::left);
  array_write(parameter_stream, asv, asv_labels);
  parameter_stream << sp22 << setw(w) << dvv_len << " derivative_variables\n";
  array_write(parameter_stream, dvv, dvv_labels);
  parameter_stream << sp22 << setw(w) << ac_len << " analysis_components\n";
  array_write(parameter_stream, an_comps, ac_labels);
  parameter_stream << sp22 << setw(w) << full_eval_id << " eval_id\n";
  parameter_stream << sp22 << setw(w) << md_len << " metadata\n";
  array_write(parameter_stream, md_labels, md_tags);
  write_precision = prec;
}

}  // namespace Dakota
