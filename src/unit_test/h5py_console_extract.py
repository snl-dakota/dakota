from __future__ import print_function
import subprocess
import re
import numpy as np

## Capture the output here, once.
_OUTPUT = ""

def extract_moments():
    """Extract the moments from the global _OUTPUT

    Returns: The moments structured as a list of dictionaries.
        The items in the list are for executions, and the 
        key, value pairs in the dictionary are the response
        descriptors and an numpy array of the moments
    """
    global _OUTPUT
    moments = []
    lines_iter = iter(_OUTPUT)
    for line in lines_iter:
        if line.startswith("Sample moment statistics"):
            next(lines_iter)
            moments.append({})
            moments_line = next(lines_iter)
            while moments_line != '':
                tokens = moments_line.split()
                resp_desc = tokens[0]
                moments_values = np.array([float(t) for t in tokens[1:]])
                moments[-1][resp_desc] = moments_values
                moments_line = next(lines_iter)
    return moments

def extract_moment_confidence_intervals():

    """Extract the moment confidence intervals

    Returns: The confidence intervals structured as a list
      of dictionaries. The keys of the dictionary are the
      responses, and the values are a 2D list; 0th dimenion are
      lower and upper, 1st dimenion are mean and standard deviation
      """

    global _OUTPUT
    cis = []
    lines_iter = iter(_OUTPUT)
    for line in lines_iter:
        if line.startswith("95% confidence intervals"):
            cis.append({})
            nline = next(lines_iter) # discard header
            nline = next(lines_iter) # first line of data
            while nline:
                tokens = nline.split()
                response = tokens[0]
                row_values = [float(v) for v in tokens[1:]]
                values = []
                values.append( [row_values[0], row_values[2]])
                values.append( [row_values[1], row_values[3]])
                cis[-1][response] = values
                nline = next(lines_iter)
    return cis


def extract_pdfs():
    """Extract the PDFs from the global _OUTPUT

    Returns: The PDFs with lower and upper bins structured
        as a list of dictionaries. The items in the list
        are for executions, and the key, value pairs in the
        dictionaries are the response descriptors and 2D lists
        of the lower and upper bounds and the densities
        with dimension (num_bins, 3)
    """
    global _OUTPUT
    pdfs = []
    lines_iter = iter(_OUTPUT)
    for line in lines_iter:
        if line.startswith("Probability Density Function (PDF)"):
            pdfs.append({})
            nline = next(lines_iter) # get either 'PDF for <descriptor>:" or a blank line
            while nline != '':  # Loop over the responses
                desc = nline.split()[-1][:-1]
                pdf_for_resp = []
                next(lines_iter)  # Skip heading "Bin Lower..."
                next(lines_iter)  # Skip heading "----..."
                nline = next(lines_iter) # Get the first line of data for this response
                while not nline.startswith("PDF for") and nline != '':  # loop over data
                    values = [float(t) for t in nline.split()]
                    pdf_for_resp.append(values)
                    nline = next(lines_iter)
                pdfs[-1][desc] = pdf_for_resp
    return pdfs

def extract_level_mapping_row(line):
    """Tokenize one row of a level mappings table.

    line: String containing a row

    returns: A list of length 4 containing the table entries. 
        Blank table entries contain None.
    """
    tokens = line.split(None, 1)
    tlen = len(tokens[0])
    precision = tlen - 7 if tokens[0] is '-' else tlen - 6
    width = precision + 7

    result = 4*[None]
    for i in range(0, 4):
        begin = i*(width + 2)
        try:
            result[i] = float(line[begin:begin + width+2])
        except (IndexError, ValueError):
            pass
    return result

def extract_level_mappings():
    """Extract the level mappings from the global _OUTPUT

    Returns: The level mappings are structured as a list of 
        dictionaries. The items in the list are for executions, 
        and the key, value pairs in the dictionaries are the 
        response descriptors and 2D lists of the mappings. The
        lists are (num_rows, 4), with None stored in empty
        elements.
    """

    global _OUTPUT
    mappings = []
    lines_iter = iter(_OUTPUT)
    for line in lines_iter:
        if line.startswith("Level mappings for each"):
            mappings.append({})
            nline = next(lines_iter) # get 'Cumulative Distribution..'
            while nline and not nline.startswith('--'):  # Loop over the responses
                desc = nline.split()[-1][:-1]
                mappings_for_resp = []
                next(lines_iter)  # Skip heading "Response Level.."
                next(lines_iter)  # Skip heading "----..."
                nline = next(lines_iter) # Get the first line of data for this response
                while not nline.startswith("Cumulative Distribution") and \
                        not nline.startswith('--') and \
                        nline:  # loop over data
                    values = extract_level_mapping_row(nline)
                    mappings_for_resp.append(values)
                    nline = next(lines_iter)
                mappings[-1][desc] = mappings_for_resp
    return mappings

def extract_correlations_helper(corr_type = None):
    """Extract the simple/simple rank correlation matrices from the global _OUTPUT

    Returns: A list of all the simple (for corr_type == "pearson") or simple rank (otherwise)
      correlation matrices. The list elements are pairs (2-tuples). The first item is a list
      of the factor labels. The second is a "2D" list of the matrix elements.
    """
    if corr_type == "pearson":
        heading = "Simple Correlation Matrix"
    else:
        heading = "Simple Rank Correlation Matrix"
    correlations = []
    lines_iter = iter(_OUTPUT)
    for line in lines_iter:
        if line.startswith(heading):
            nline = next(lines_iter)
            factors = nline.split()
            num_factors = len(factors)
            coef = []
            for i in range(num_factors):
                coef.append(num_factors*[0.0])
                nline = next(lines_iter)
                tokens = [float(t) for t in nline.split()[1:]]
                for j in range(i+1):
                    coef[i][j] = tokens[j]
            # symmetrize
            for i in range(num_factors):
                for j in range(i+1, num_factors):
                    coef[i][j] = coef[j][i]
            # store
            correlations.append( (factors, coef))
    return correlations

def extract_simple_rank_correlations():
    return extract_correlations_helper("spearman")
        
def extract_simple_correlations():
    return extract_correlations_helper("pearson")

def extract_partial_correlations_helper(corr_type = None):
    """Extract the partial/partial rank correlation matrices from the global _OUTPUT

    Returns: A list of all the partial (for corr_type == "pearson") or partial rank (otherwise)
      correlation matrices. The list elements are dictionaries, with response descriptors as keys.
      The values are pairs (2-tuples). The first item is a list of variables. The second item is
      a list of correlation values.
      """
    if corr_type == "pearson":
        heading = "Partial Correlation Matrix"
    else:
        heading = "Partial Rank Correlation Matrix"
    correlations = []
    lines_iter = iter(_OUTPUT)
    for line in lines_iter:
        if line.startswith(heading):
            nline = next(lines_iter)
            inc_corr = {}
            responses = nline.split()
            for r in responses: 
                inc_corr[r] = ([], [])
            nline = next(lines_iter)
            while(nline):
                tokens = nline.split()
                var = tokens[0]
                val = [float(v) for v in tokens[1:]]
                for i, r in enumerate(responses):
                    inc_corr[r][0].append(var)
                    inc_corr[r][1].append(val[i])
                nline = next(lines_iter)
            correlations.append(inc_corr)
    return correlations

def extract_partial_rank_correlations():
    return extract_partial_correlations_helper("spearman")
        
def extract_partial_correlations():
    return extract_partial_correlations_helper("pearson")

def extract_best_helper(result, labeled=False):
    """Extract the best parameters/residuals/objectives/constraints

    Return as a list of lists. The outer list may be over 
      executions solution sets. (TODO: This may not be expressive enough, 
      because there could be multiple executions and multiple sets). If
      the result is labeled (with variable or response descriptors), the inner
      lists contain tuples of (label, value) are returned.
    """
    heading = "<<<<< Best " + result
    best = []
    lines_iter = iter(_OUTPUT)
    for line in lines_iter:
        if line.startswith(heading):
            nline = next(lines_iter) # the first variable
            best_set = []
            while(nline[0] != "<"):
                # variables may be float, int, or string valued. Try to convert.
                # String variables can contain whitespace, so split from the right
                # to separate the variable descriptor from the value.
                if labeled:
                    val, label = nline.rsplit(None,1)
                    try:
                        val = int(val)
                    except ValueError:
                        try:
                            val = float(val)
                        except ValueError:
                            pass
                    best_set.append( (label, val) )
                else:
                    best_set.append(float(nline))
                nline = next(lines_iter)
            best.append(best_set)
    return best

def extract_best_parameters():
    return extract_best_helper("parameters", True)

def extract_best_residuals():
    return extract_best_helper("residual terms", False)

def extract_best_objectives():
    return extract_best_helper("objective", False)

def extract_best_constraints():
    return extract_best_helper("constraint", False)

def extract_best_parameter_confidence_intervals():
    """Extract parameter confidence intervals

    Return as a list of dictionaries. The keys of the dictionaries are variable
      descriptors, and the values are tuples of lower and upper bounds
    """
    line_re = re.compile(r"\s+(.+?):\s+\[\s+(.+?),\s+(.+)\s+\]")
    cis = []
    lines_iter = iter(_OUTPUT)
    for line in lines_iter:
        if line.startswith("Confidence Intervals on Cal"):
            nline = next(lines_iter) # the first variable
            ci = {}
            while(nline):
                m = line_re.match(nline)
                ci[m.group(1)] = (float(m.group(2)), float(m.group(3)))
                nline = next(lines_iter)
            cis.append(ci)
    return cis

def extract_best_residual_norms():
    """Extract norm

    Returns a list of norms
    """
    norms = []
    lines_iter = iter(_OUTPUT)
    for line in lines_iter:
        if line.startswith("<<<<< Best residual norm"):
            norm = line.split()[5]
            # strip off the semicolon
            norm = norm[:-1]
            norms.append(float(norm))
    return norms



              
def run_dakota(input_file):
    """Run Dakota on the input_file and capture output

    input_file: string containing a path to the input file

    """
    global _OUTPUT
    output = subprocess.check_output(["../dakota",input_file], stderr=subprocess.STDOUT)
    _OUTPUT = output.split('\n')
    
