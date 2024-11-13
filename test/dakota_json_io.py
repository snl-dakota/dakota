#!/usr/bin/env python3
import dakota.interfacing as di
import json
import sys

c = [2.0,
     0.5,
    -1.0,
     0.7,
    -2.0,
     0.1,
    -0.5,
     1.5,
     0.2,
    -0.4]


def function(x1, x2):
    f = c[0]*x1**3 + \
        c[1]*x2**3 + \
        c[2]*x1**2*x2 + \
        c[3]*x1*x2**2 + \
        c[4]*x1**2 + \
        c[5]*x2**2 + \
        c[6]*x1*x2 + \
        c[7]*x1 + \
        c[8]*x2 + \
        c[9]
    return f


def gradient(x1, x2):
    dfdx1 = 3*c[0]*x1**2 + \
            2*c[2]*x1*x2 + \
            c[3]*x2**2 + \
            2*c[4]*x1 + \
            c[6]*x2 + \
            c[7]
            
    dfdx2 = 3*c[1]*x2**2 + \
            c[2]*x1**2 + \
            2*c[3]*x1*x2 + \
            2*c[5]*x2 + \
            c[6]*x1 + \
            c[8]

    dfdx = [dfdx1, dfdx2]
    return dfdx

def hessian(x1, x2):
    d2fdx12 = 6*c[0]*x1 + \
              2*c[2]*x2 + \
              2*c[4]
    d2fdx1x2 = 2*c[2]*x1 + \
               2*c[3]*x2 + \
               c[6]
    d2fdx22 = 6*c[1]*x2 + \
              2*c[3]*x1 + \
              2*c[5]

    d2f = [
            [d2fdx12, d2fdx1x2],
            [d2fdx1x2, d2fdx22]
          ]
    return d2f


def read_json_parameters(filename, batch, metadata, an_comp):
    with open(filename, "r") as f:
        params = json.load(f)
    variables = []
    response_labels = []
    asv = []
    if not batch:
        params = [params]
    for p in params:
        assert("x1" == p["variables"][0]["label"])
        assert("x2" == p["variables"][1]["label"])
        eval_variables = [v["value"] for v in p["variables"]]
        eval_labels = [r["label"] for r in p["responses"]] 
        assert(eval_labels[0] == 'f')
        eval_asv = [r["active_set"] for r in p["responses"]]
        if metadata:
            assert("metadata" in p and p["metadata"][0] == "time")
        if an_comp:
            assert(p["analysis_components"][0]["component"] == "present" and
                   p["analysis_components"][0]["driver"].endswith("dakota_json_io.py not_batch metadata an_comp json json"))
        variables.append(eval_variables)
        response_labels.append(eval_labels)
        asv.append(eval_asv)
    return variables, response_labels, asv


def compute_asv(results):
    asv = []
    for eval_results in results:
        eval_asv = []
        for r in eval_results.responses():
            active_set = 0
            if r.asv.function:
                active_set |= 1
            if r.asv.gradient:
                active_set |= 2
            if r.asv.hessian:
                active_set |= 4
            eval_asv.append(active_set)
        asv.append(eval_asv)
    return asv


def write_dakota_results(results, labels, results_file, batch_flag):
    with open(results_file, "w") as f:
        for eval_results, eval_labels in zip(results, labels):
            if batch_flag:
                f.write("#\n")
            if "functions" in eval_results:
                for label in eval_labels:
                    f.write(f"{eval_results['functions'][label]} {label}\n")
            if "gradients" in eval_results:
                for label in eval_labels:
                    gradient = eval_results["gradients"][label]
                    f.write("[ ")
                    f.write(" ".join(str(g) for g in gradient))
                    f.write(" ]\n")
            if "hessians" in eval_results:
                for label in eval_labels:
                    hessian = eval_results["hessians"][label]
                    f.write("[[ ")
                    for row in hessian:
                        f.write(" ".join(str(r) for r in row))
                        f.write("\n")
                    f.write(" ]]\n")

if __name__ == '__main__':
    batch_flag = (sys.argv[1] == 'batch')
    metadata_flag = (sys.argv[2] == 'metadata')
    an_comp_flag = (sys.argv[3] == 'an_comp')
    params_format = sys.argv[4]
    results_format = sys.argv[5]
    params_file = sys.argv[6]
    results_file = sys.argv[7]

    format_map = {'aprepro': di.APREPRO,
            'standard': di.STANDARD,
            'json': di.JSON
            }

    if params_format == 'json':
        variables, function_labels, asv = read_json_parameters(params_file, batch_flag, metadata_flag, an_comp_flag)
    else:
        variables = []
        function_labels = []
        params, results = di.read_parameters_file(params_file, results_file, batch=batch_flag)
        if not batch_flag:
            params = [params]
            results = [results]
        for p, r in zip(params, results):
            assert(p.format == format_map[params_format])
            eval_vars = [value for value in p.values()]
            eval_labels = r.descriptors
            variables.append(eval_vars)
            function_labels.append(eval_labels)
        asv = compute_asv(results)

    computed = []
    for eval_asv, eval_labels, eval_vars in zip(asv, function_labels, variables):
        eval_computed = {}
        if eval_asv[0] & 1:
            eval_computed["functions"] = {eval_labels[0]: function(*eval_vars)}
        if eval_asv[0] & 2:
            eval_computed["gradients"] = {eval_labels[0]: gradient(*eval_vars)}
        if eval_asv[0] & 4:
            eval_computed["hessians"] = {eval_labels[0]: hessian(*eval_vars)}
        if metadata_flag:
            eval_computed["metadata"] = {"time": 5.0}
        computed.append(eval_computed)

    if results_format == 'json':
        with open(results_file, "w") as f:
            if batch_flag:
                json.dump(computed, f)
            else:
                json.dump(computed[0], f)
    elif params_format ==  'json': # manually write dakota format results (we don't have a di.Results object)
        write_dakota_results(computed, function_labels, results_file, batch_flag)
    else:  # put results into results object
        for eval_asv, eval_results, eval_labels, eval_computed in zip(asv, results, function_labels, computed):
            if eval_asv[0] & 1:
                eval_results[eval_labels[0]].function = eval_computed["functions"][eval_labels[0]]
            if eval_asv[0] & 2:
                eval_results[eval_labels[0]].gradient = eval_computed["gradients"][eval_labels[0]]
            if eval_asv[0] & 4:
                eval_results[eval_labels[0]].hessian = eval_computed["hessians"][eval_labels[0]]
            if metadata_flag:
                eval_results.metadata["time"] = 5.0
        if batch_flag:
            results.write()
        else:
            results[0].write()

