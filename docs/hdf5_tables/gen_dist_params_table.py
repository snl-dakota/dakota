#!/usr/bin/env python
# coding: utf-8
from __future__ import print_function
import h5py
import numpy as np

# Generate the table of distribution parameters using HDF5 output from
# the unit test hdf5_dist_params.py. To use:
# Run hdf5_dist_params.py to generate dist_params.h5
# Run this script in the same folder. It prints the html table to stdout,
# so you'll need to redirect it to a file.


def get_field_type(in_item):
    item = in_item
    dim = "scalar"
    if in_item.shape:
        dim = "vector"
        item = item[0]
    if isinstance(item, str):
        return ("string", dim)
    elif item.dtype == np.float64:
        return ("real", dim)
    elif item.dtype == np.int32 or item.dtype == np.uint32:
        return ("integer", dim)
    else:
        print("ERROR: %s" % type(obj))

def print_header():
    print("""<table  class="spec-table">
<tr>
 <th width="2%" class="border-heavy-right"></th>
 <th class="border-heavy-right" colspan="4"><b>Distribution Parameters</b> </td>
</tr>
<tr>
 <th class="border-heavy-right"></th>
 <th class="border-heavy-right" width="30%"><b>Variable Type</b> </td>
 <th class="border-heavy-right" width="30%"><b>Parameter Name</b> </td>
 <th class="border-heavy-right" width="20%"><b>Type</b> </td>
 <th class="border-heavy-right" width="20%"><b>Rank</b> </td>
</tr>
""")

def print_variable_type(var_type=None, parameters=None):
    """Parameters is expected to be a list of tuples. Each list element
    is a separate parameter. The tuple is (name, type, dim)"""
    num_params = len(parameters)
    print("""<tr>
 <th width="2%%" class="border-heavy-right" rowspan="%d"></th>
 <td class="border-light-right" rowspan="%d" valign="middle">%s</td>
 <td class="border-light-right">%s</td>
 <td class="border-light-right">%s</td>
 <td class="border-light-right">%s</td>
</tr>""" % (num_params, num_params, var_type, 
    parameters[0][0], 
    parameters[0][1],
    parameters[0][2]))
    for pname, ptype, pdim in parameters[1:]:
        print("""</tr>
<tr>
 <td class="border-light-right">%s</td>
 <td class="border-light-right">%s</td>
 <td class="border-light-right">%s</td>
</tr>""" % (pname, ptype, pdim))

def print_footer():
    print("</table>")



all_types = {}
with h5py.File("dist_params.h5","r") as h:
    md = h["models/simulation/NO_MODEL_ID/metadata/variable_parameters"]
    for name, data in md.items():
        all_types[name] = {"first_id":data.dims[0][1][0],
                "params":[]}
        for f in data.dtype.names:
            field_type, field_dim = get_field_type(data[0][f])
            all_types[name]["params"].append((f, field_type, field_dim))

sorted_types = sorted(all_types, key=lambda x: all_types[x]["first_id"])

print_header()
for t in sorted_types:
    print_variable_type(t, all_types[t]["params"])
print_footer()

