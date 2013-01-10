# Helper script to concatentate testing results and generate summary report
#
# Generates dakota_results.log, dakota_diffs.out, and dakota_pdiffs.out

include(DakotaProcessTestResults)

create_diff_file("${Dakota_BINARY_DIR}" "test/dakota_results.log"
  "dakota_diffs.out")
create_diff_file("${Dakota_BINARY_DIR}" "test/dakota_results.log"
  "dakota_pdiffs.out")
