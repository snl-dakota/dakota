from __future__ import print_function
import os
import subprocess
import re
import copy
from collections import OrderedDict
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

def extract_multi_start_results():
    """Extract results summary from a multi_start study, including
    initial points, best points, and best responses"""
    lines_iter = iter(_OUTPUT)
    for line in lines_iter:
        if line.startswith("<<<<< Results summary:"):
            break
    label_line = next(lines_iter)
    # 1. split the labels
    # 2. throw away the first token ("set_id")
    # 3. Grab labels that don't end in *. These are the starting points.
    # 4. Grab labels that do end in *. These are the best points. This list may be
    #    longer because it can include non-continuous design variables
    # 5. Grab remaining labels, which are all responses.
    all_labels = label_line.split()[1:]
    num_starts = 0
    start_labels = []
    num_best = 0
    best_labels = []
    for label in all_labels:
        if label[-1] is '*':
            break
        num_starts += 1
        start_labels.append(label)
    for label in all_labels[num_starts:]:
        if label[-1] is not '*':
            break
        num_best += 1
        best_labels.append(label[:-1]) # snip off *
    num_funcs = len(all_labels) - num_best - num_starts
    func_labels = all_labels[num_best + num_starts:]
    # Begin reading values
    results = {"start_labels":start_labels,
            "best_labels":best_labels,
            "function_labels":func_labels,
            "starts":[],
            "best":[],
            "functions":[]}
    
    while True:
        value_line = lines_iter.next().strip()
        if not value_line:
            break
        values = []
        for value in value_line.split()[1:]:
            try:
                values.append(int(value))
            except ValueError:
                try:
                    values.append(float(value))
                except ValueError:
                    values.append(value)
        results["starts"].append(values[:num_starts])
        results["best"].append(values[num_starts:num_starts+num_best])
        results["functions"].append(values[num_starts+num_best:])
    return results
        
def extract_pareto_set_results():
    """Extract results summary from a pareto_set study. These
    include the weights, variables, and functions."""
    lines_iter = iter(_OUTPUT)
    for line in lines_iter:
        if line.startswith("<<<<< Results summary:"):
            break
    label_line = next(lines_iter)
    # 1. split the labels. These are weights, best params, best responses
    # 2. throw away the first token ("set_id") 
    # 3. Grab weight labels (w\d+). These are the weights. The number responses equals the
    #    number of weights.
    # 4. Grab the best parameters labels (by count: total number - 2*number of weights)
    # 5. Grab the best responses
    all_labels = label_line.split()[1:]
    num_weights = 0
    weight_labels = []
    num_best = 0
    best_labels = []
    weight_re = re.compile("w\d+$")
    for label in all_labels:
        if weight_re.match(label) is None:
            break
        num_weights += 1
        weight_labels.append(label)
    num_best = len(all_labels) - num_weights*2
    for label in all_labels[num_weights:-num_weights]:
        best_labels.append(label)
    num_funcs = num_weights
    func_labels = all_labels[-num_weights:]
    # Begin reading values
    results = {"weight_labels":weight_labels,
            "best_labels":best_labels,
            "function_labels":func_labels,
            "weights":[],
            "best":[],
            "functions":[]}
    
    while True:
        value_line = lines_iter.next().strip()
        if not value_line:
            break
        values = []
        for value in value_line.split()[1:]:
            try:
                values.append(int(value))
            except ValueError:
                try:
                    values.append(float(value))
                except ValueError:
                    values.append(value)
        results["weights"].append(values[:num_weights])
        results["best"].append(values[num_weights:-num_funcs])
        results["functions"].append(values[-num_funcs:])
    return results
        






def read_tabular_data(tabular_file):
    """Read an annotated format Dakota tabular file. Convert everything
    to floating point numbers. Return an ordered dict of string:lists"""
    data = OrderedDict()
    with open(tabular_file,"r") as f:
        columns = f.readline().split()
        for c in columns:
            data[c] = []
        for row in f:
            tokens = row.split()
            data["%eval_id"].append(int(tokens[0]))
            data["interface"].append(tokens[1])
            for c, t in zip(columns[2:], tokens[2:]):
                try:
                    data[c].append(int(t))
                except ValueError:
                    try:
                        data[c].append(float(t))
                    except ValueError:
                        data[c].append(t)
    return data

def restart_variables(row):

    variables = {}
    num_relaxed_di = int(row[18])
    num_relaxed_dr_index = 18+num_relaxed_di+1
    num_relaxed_dr = int(row[num_relaxed_dr_index])

    num_cont_vars_index = num_relaxed_dr_index + num_relaxed_dr + 1
    num_cont_vars = int(row[num_cont_vars_index])
    if num_cont_vars != 0:
        cont_vars = OrderedDict()
        start = num_cont_vars_index+1
        end = start + num_cont_vars*2
        for i in range(start, end, 2):
            cont_vars[row[i+1]] = float(row[i])
        variables["continuous"] = cont_vars
    num_di_vars_index = num_cont_vars_index + 2*num_cont_vars + 1
    num_di_vars = int(row[num_di_vars_index])
    if num_di_vars != 0:
        di_vars = OrderedDict()
        start = num_di_vars_index+1
        end = start + num_di_vars*2
        for i in range(start, end, 2):
            di_vars[row[i+1]] = int(row[i])
        variables["discrete_int"] = di_vars
    num_ds_vars_index = num_di_vars_index + 2*num_di_vars + 1
    num_ds_vars = int(row[num_ds_vars_index])
    if num_ds_vars != 0:
        ds_vars = OrderedDict()
        start = num_ds_vars_index+1
        end = start + num_ds_vars*2
        for i in range(start, end, 2):
            ds_vars[row[i+1]] = row[i]
        variables["discrete_string"] = ds_vars
    num_dr_vars_index = num_ds_vars_index + 2*num_ds_vars + 1
    num_dr_vars = int(row[num_dr_vars_index])
    if num_dr_vars != 0:
        dr_vars = OrderedDict()
        start = num_dr_vars_index+1
        end = start + num_dr_vars*2
        for i in range(start, end, 2):
            dr_vars[row[i+1]] = row[i]
        variables["discrete_real"] = dr_vars
    return variables

def restart_response(row):
    data = {}
    data["interface"] = row[0]
    num_functions = int(row[2])
    num_deriv_vars = int(row[3])
    grad_flag = row[4] == "1"
    hess_flag = row[5] == "1"
    asv_start_index = 6
    asv_end_index = asv_start_index + num_functions
    data["asv"] = [int(a) for a in row[asv_start_index:asv_end_index]]
    dvv_start_index = asv_end_index 
    dvv_end_index = dvv_start_index + num_deriv_vars
    data["dvv"] = [int(a) for a in row[dvv_start_index:dvv_end_index]]
    labels_start_index = dvv_end_index 
    labels_end_index = labels_start_index + num_functions
    labels = row[labels_start_index:labels_end_index]
    data["response"] = OrderedDict()
    for d in labels:
        data["response"][d] = {}
    fv_index = labels_end_index 
    for a, d in zip(data["asv"], labels):
        if a & 1:
            data["response"][d]["function"] = float(row[fv_index])
            fv_index += 1
    if grad_flag:
        for a, d in zip(data["asv"], labels):
            if a & 2:
                g_start = fv_index
                g_end = fv_index + num_deriv_vars
                data["response"][d]["gradient"] = [float(c) for c in row[g_start:g_end]]
                fv_index = g_end 
    if hess_flag:
        for a, d in zip(data["asv"], labels):
            if a & 4:
                h_start = fv_index
                h_end = fv_index + sum(range(num_deriv_vars+1))
                data["response"][d]["hessian"] = [float(c) for c in row[h_start:h_end]]
                fv_index = h_end 
    data["eval_id"] = int(row[-1])
    return data

def read_restart_file(restart_file):
    file_stem = restart_file.rsplit(".", 1)[0]
    neutral_file = file_stem + ".neu"
    with open(os.devnull,"w") as fnull:
        subprocess.call(["../dakota_restart_util",
                         "to_neutral",
                          restart_file,
                          neutral_file], stdout=fnull)
    # data structure
    #{
    #    "variables":{"continuous":{columns:[]},
    #                 "discrete_int":{columns:[]},
    #                 "discrete_string":{columns:[]},
    #                 "discrete_real":{columns:[]},
    #                 }
    #    "response":{"label":[{"function":val, "gradient":[], "hessian":[]}]},
    #    "asv":[ [] ],
    #    "eval_id":[],
    #    "dvv":[ [] ],
    #    "interface":[],
    #
    #}
    with open(neutral_file,"r") as f:
        file_data = f.readlines()
    # Allocate storage
    data = {}
    var_r = restart_variables(file_data[0].split())
    data["variables"] = {}
    for t, vs in var_r.items(): # iterate continuous, di, ds, dr
        data["variables"][t] = OrderedDict()
        for d, v in vs.items():
            data["variables"][t][d] = []
    resp_r = restart_response(file_data[1].split())
    data["response"] = OrderedDict()
    for d, r in resp_r["response"].items():
        data["response"][d] = []
    data["asv"] = []
    data["eval_id"] = []
    data["dvv"] = []
    data["interface"] = []

    for i in range(0,len(file_data),2):
        var_row = file_data[i].split()
        resp_row = file_data[i+1].split()
        var_r = restart_variables(var_row)
        resp_r = restart_response(resp_row)
        for t, vs in var_r.items():
            for d, v in vs.items():
                data["variables"][t][d].append(v)
        for d, r in resp_r["response"].items():
            data["response"][d].append(copy.deepcopy(r))
        data["asv"].append(resp_r["asv"][:])
        data["eval_id"].append(resp_r["eval_id"])
        data["dvv"].append(resp_r["dvv"][:])
        data["interface"].append(resp_r["interface"])
    return data

              
def run_dakota(dakota, input_file):
    """Run Dakota on the input_file and capture output

    input_file: string containing a path to the input file

    """
    global _OUTPUT
    output = subprocess.check_output([dakota,input_file], stderr=subprocess.STDOUT)
    _OUTPUT = output.split('\n')
    
