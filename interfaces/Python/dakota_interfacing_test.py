#!/usr/bin/env python
from __future__ import print_function
import copy
import os
import unittest

__author__ = 'J. Adam Stephens'
__copyright__ = 'Copyright 2014-2024 National Technology & Engineering Solutions of Sandia, LLC (NTESS)'
__license__ = 'GNU Lesser General Public License'

try: # Python 2/3 compatible import of StringIO
    import StringIO
except ImportError:
    import io as StringIO
import dakota.interfacing as di
from dakota.interfacing import parallel

# 6.15 and older format
apreproParamsNoMetadata = """                    { DAKOTA_VARS     =                      3 }
                    { x1              =  7.488318331306800e-01 }
                    { x2              =  2.188638686202466e-01 }
                    { dussv_1         =                      "foo bar" }
                    { DAKOTA_FNS      =                      1 }
                    { ASV_1:response_fn_1 =                      %d }
                    { DAKOTA_DER_VARS =                      2 }
                    { DVV_1:x1        =                      1 }
                    { DVV_2:x2        =                      2 }
                    { DAKOTA_AN_COMPS =                      2 }
                    { AC_1:first.sh   =                      "a b" }
                    { AC_2:first.sh   =                      "b" }
                    { DAKOTA_EVAL_ID  =                      1 }
"""

# 6.16 and newer format
apreproParams = apreproParamsNoMetadata + """                    { DAKOTA_METADATA =                      1 }
                    { MD_1            =                      "seconds" }
"""

# 6.15 and older format
standardParamsNoMetadata = """                                          3 variables
                      7.488318331306800e-01 x1
                      2.188638686202466e-01 x2
                                    foo bar dussv_1
                                          1 functions
                                          %d ASV_1:response_fn_1
                                          2 derivative_variables
                                          1 DVV_1:x1
                                          2 DVV_2:x2
                                          2 analysis_components
                                        a b AC_1:first.sh
                                          b AC_2:first.sh
                                          1 eval_id
"""

# 6.16 and newer format
standardParams = standardParamsNoMetadata + """                                          1 metadata
                                    seconds MD_1
"""

standardSingleEvalBatchParams = """                                          3 variables
                      7.488318331306800e-01 x1
                      2.188638686202466e-01 x2
                                    foo bar dussv_1
                                          1 functions
                                          1 ASV_1:response_fn_1
                                          2 derivative_variables
                                          1 DVV_1:x1
                                          2 DVV_2:x2
                                          0 analysis_components
                                        2:1 eval_id
                                          0 metadata
"""


standardBatchParams = """                                          2 variables
                      2.109465794009156e-01 x1
                     -9.675715913879684e-01 x2
                                          1 functions
                                          1 ASV_1:obj_fn
                                          2 derivative_variables
                                          1 DVV_1:x1
                                          2 DVV_2:x2
                                          0 analysis_components
                                        1:1 eval_id
                                          1 metadata
                                    seconds MD_1
                                          2 variables
                      3.222614371054804e-01 x1
                      4.946014218730854e-02 x2
                                          1 functions
                                          1 ASV_1:obj_fn
                                          2 derivative_variables
                                          1 DVV_1:x1
                                          2 DVV_2:x2
                                          0 analysis_components
                                        1:2 eval_id
                                          1 metadata
                                    seconds MD_1
"""

apreproBatchParams = """                    { DAKOTA_VARS     =                      3 }
                    { x1              =  7.488318331306800e-01 }
                    { x2              =  2.188638686202466e-01 }
                    { dussv_1         =                      "foo bar" }
                    { DAKOTA_FNS      =                      1 }
                    { ASV_1:response_fn_1 =                  1 }
                    { DAKOTA_DER_VARS =                      2 }
                    { DVV_1:x1        =                      1 }
                    { DVV_2:x2        =                      2 }
                    { DAKOTA_AN_COMPS =                      2 }
                    { AC_1:first.sh   =                      "a b" }
                    { AC_2:first.sh   =                      "b" }
                    { DAKOTA_EVAL_ID  =                    1:1 }
                    { DAKOTA_METADATA =                      1 }
                    { MD_1            =                      "seconds" }
                    { DAKOTA_VARS     =                      3 }
                    { x1              =  7.488318331306800e-01 }
                    { x2              =  2.188638686202466e-01 }
                    { dussv_1         =                      "foo bar" }
                    { DAKOTA_FNS      =                      1 }
                    { ASV_1:response_fn_1 =                  1 }
                    { DAKOTA_DER_VARS =                      2 }
                    { DVV_1:x1        =                      1 }
                    { DVV_2:x2        =                      2 }
                    { DAKOTA_AN_COMPS =                      2 }
                    { AC_1:first.sh   =                      "a b" }
                    { AC_2:first.sh   =                      "b" }
                    { DAKOTA_EVAL_ID  =                    1:2 }
                    { DAKOTA_METADATA =                      1 }
                    { MD_1            =                      "seconds" }
"""


# Dictionaries for testing the direct interface
direct_dense_params = {
    "variables": 5, 
    "functions": 3, 
    "metadata": 1, 
    "variable_labels": ["cdv_1", "ddsiv_1", "ddssv_1", "ddsrv_1", "uuv_1"],
    "function_labels": ["obj_fn", "nln_ineq_con_1", "nln_ineq_con_2"],
    "metadata_labels": ["baz"],
    "cv": [0.9414315689355135, 0.5], 
    "cv_labels": ["cdv_1", "uuv_1"], 
    "div": [7], 
    "div_labels": ["ddsiv_1"], 
    "dsv": ["baz"], 
    "dsv_labels": ["ddssv_1"], 
    "drv": [3.14], 
    "drv_labels": ["ddsrv_1"], 
    "asv": [7, 7, 7], 
    "dvv": [1], 
    "analysis_components": ["foo"],
    "eval_id": 1
}

direct_sparse_params = {
    "variables": 1, 
    "functions": 1, 
    "metadata": 0, 
    "variable_labels": ["cdv_1"],
    "function_labels": ["obj_fn"],
    "metadata_labels": [],
    "cv": [0.5], 
    "cv_labels": ["cdv_1"], 
    "div": [], 
    "div_labels": [], 
    "dsv": [], 
    "dsv_labels": [], 
    "drv": [], 
    "drv_labels": [], 
    "asv": [1], 
    "dvv": [1], 
    "analysis_components": [],
    "eval_id": 1
}

# JSON params

# Dakota 6.21 and newer
jsonParams = """
{
    "analysis_components": [
        {
            "component": "a b",
            "driver": "batchdriver.py"
        },
        {
            "component": "b",
            "driver": "batchdriver.py"
        }
    ],
    "derivative_variables": [
        1,
        2
    ],
    "eval_id": "1",
    "metadata": [
        "seconds"
    ],
    "responses": [
        {
            "active_set": 1,
            "label": "response_fn_1"
        }
    ],
    "variables": [
        {
            "label": "x1",
            "value": 7.488318331306800e-01 
        },
        {
            "label": "x2",
            "value": 2.188638686202466e-01 
        },
        {
            "label": "dussv_1",
            "value": "foo bar"
        }
    ]
}
"""
jsonBatchParams = """
[
    {
        "analysis_components": [],
        "derivative_variables": [
            1,
            2
        ],
        "eval_id": "1:1",
        "metadata": [
            "seconds"
        ],
        "responses": [
            {
                "active_set": 1,
                "label": "obj_fn"
            }
        ],
        "variables": [
            {
                "label": "x1",
                "value": 0.2109465794009156
            },
            {
                "label": "x2",
                "value": -0.9675715913879684
            }
        ]
    },
    {
        "analysis_components": [],
        "derivative_variables": [
            1,
            2
        ],
        "eval_id": "1:2",
        "metadata": [
            "seconds"
        ],
        "responses": [
            {
                "active_set": 1,
                "label": "obj_fn"
            }
        ],
        "variables": [
            {
                "label": "x1",
                "value": 0.3222614371054804
            },
            {
                "label": "x2",
                "value": 0.04946014218730854
            }
        ]
    }
]
"""

# Helper functions needed for Python < 2.7
def set_function(r):
    r["response_fn_1"].function = 5.0

def set_gradient(r):
    r["response_fn_1"].gradient = [1.0189673084127668E-266, -6.3508646783183408E-264]

def set_hessian(r):
    r["response_fn_1"].hessian = [[1.0, 2.0],[2.0,3.0]]

def set_metadata(r, key, value):
    r.metadata[key] = value


class dakotaInterfacingTestCase(unittest.TestCase):

    def test_aprepro_format(self):
        """Confirm that aprepro format Parameters files are parsed correctly."""
        pio = StringIO.StringIO(apreproParams % 1)
        p, r = di.interfacing._read_parameters_stream(stream=pio, results_file="results.out")
        self.assertEqual(p.num_variables, 3)
        self.assertEqual(p.descriptors, ["x1","x2","dussv_1"])
        self.assertAlmostEqual(p["x1"], 7.488318331306800e-01)
        self.assertAlmostEqual(p["x2"], 2.188638686202466e-01)
        self.assertEqual(p["dussv_1"], "foo bar")
        self.assertEqual(p.num_an_comps, 2)
        self.assertEqual(p.an_comps[0], "a b")
        self.assertEqual(p.an_comps[1], "b")
        self.assertEqual(p.eval_id, "1")
        self.assertEqual(p.num_metadata, 1)
        self.assertEqual(p.metadata[0], "seconds")
        self.assertEqual(p.format, di.APREPRO)
        
        self.assertEqual(r.num_responses, 1)
        self.assertEqual(r.descriptors, ["response_fn_1"])
        self.assertEqual(r["response_fn_1"].asv, (True, False, False))
        self.assertEqual(r.num_deriv_vars, 2)
        self.assertEqual(r.deriv_vars, ["x1","x2"])
        self.assertEqual(r.format, di.APREPRO)
        self.assertEqual(r.results_file, "results.out")

    def test_metadataless_aprepro_format(self):
        """Coarse test parsing legacy metadata-less aprepro format Parameters files."""
        pio = StringIO.StringIO(apreproParamsNoMetadata % 1)
        p, r = di.interfacing._read_parameters_stream(stream=pio, results_file="results.out")
        self.assertEqual(p.num_variables, 3)
        self.assertEqual(p.num_metadata, 0)
        self.assertEqual(p.format, di.APREPRO)

    def test_standard_format(self):
        """Confirm that Dakota format Parameters files are parsed correctly."""
        pio = StringIO.StringIO(standardParams % 1)
        p, r = di.interfacing._read_parameters_stream(stream=pio, results_file="results.out")
        self.assertEqual(p.num_variables, 3)
        self.assertEqual(p.descriptors, ["x1","x2","dussv_1"])
        self.assertAlmostEqual(p["x1"], 7.488318331306800e-01)
        self.assertAlmostEqual(p["x2"], 2.188638686202466e-01)
        self.assertEqual(p["dussv_1"], "foo bar")
        self.assertEqual(p.num_an_comps, 2)
        self.assertEqual(p.an_comps[0], "a b")
        self.assertEqual(p.an_comps[1], "b")
        self.assertEqual(p.eval_id, "1")
        self.assertEqual(p.num_metadata, 1)
        self.assertEqual(p.metadata[0], "seconds")
        self.assertEqual(p.format, di.STANDARD)
        
        self.assertEqual(r.num_responses, 1)
        self.assertEqual(r.descriptors, ["response_fn_1"])
        self.assertEqual(r["response_fn_1"].asv, (True, False, False))
        self.assertEqual(r.num_deriv_vars, 2)
        self.assertEqual(r.deriv_vars, ["x1","x2"])
        self.assertEqual(r.eval_id, "1")
        self.assertEqual(r.format, di.STANDARD)
        self.assertEqual(r.results_file, "results.out")

    def test_metadataless_standard_format(self):
        """Coarse test parsing legacy metadata-less dakota format Parameters files."""
        pio = StringIO.StringIO(standardParamsNoMetadata % 1)
        p, r = di.interfacing._read_parameters_stream(stream=pio, results_file="results.out")
        self.assertEqual(p.num_variables, 3)
        self.assertEqual(p.num_metadata, 0)
        self.assertEqual(p.format, di.STANDARD)

    def test_json_format(self):
        """Confirm that Dakota format Parameters files are parsed correctly."""
        pio = StringIO.StringIO(jsonParams)
        p, r = di.interfacing._read_parameters_stream(stream=pio, results_file="results.out")
        self.assertEqual(p.num_variables, 3)
        self.assertEqual(p.descriptors, ["x1","x2","dussv_1"])
        self.assertAlmostEqual(p["x1"], 7.488318331306800e-01)
        self.assertAlmostEqual(p["x2"], 2.188638686202466e-01)
        self.assertEqual(p["dussv_1"], "foo bar")
        self.assertEqual(p.num_an_comps, 2)
        self.assertEqual(p.an_comps[0], "a b")
        self.assertEqual(p.an_comps[1], "b")
        self.assertEqual(p.eval_id, "1")
        self.assertEqual(p.num_metadata, 1)
        self.assertEqual(p.metadata[0], "seconds")
        self.assertEqual(p.format, di.JSON)
        
        self.assertEqual(r.num_responses, 1)
        self.assertEqual(r.descriptors, ["response_fn_1"])
        self.assertEqual(r["response_fn_1"].asv, (True, False, False))
        self.assertEqual(r.num_deriv_vars, 2)
        self.assertEqual(r.deriv_vars, ["x1","x2"])
        self.assertEqual(r.eval_id, "1")
        self.assertEqual(r.format, di.JSON)
        self.assertEqual(r.results_file, "results.out")

    def test_asv(self):
        """Results behaves according to the ASV when response data is set."""
        # Function only
        pio = StringIO.StringIO(standardParams % 1)
        p, r = di.interfacing._read_parameters_stream(stream=pio)
        set_function(r)
        self.assertRaises(di.interfacing.ResponseError, set_gradient, r)
        self.assertRaises(di.interfacing.ResponseError, set_hessian, r)
        set_metadata(r, "seconds", 42.0)
        r.write(StringIO.StringIO())
        # Gradient only
        pio = StringIO.StringIO(standardParams % 2)
        p, r = di.interfacing._read_parameters_stream(stream=pio)
        self.assertRaises(di.interfacing.ResponseError, set_function, r)
        set_gradient(r)
        self.assertRaises(di.interfacing.ResponseError, set_hessian, r)
        set_metadata(r, 0, 42.0)
        r.write(StringIO.StringIO())
        # Hessian only
        pio = StringIO.StringIO(standardParams % 4)
        p, r = di.interfacing._read_parameters_stream(stream=pio)
        self.assertRaises(di.interfacing.ResponseError, set_function, r)
        self.assertRaises(di.interfacing.ResponseError, set_gradient, r)
        set_hessian(r)
        set_metadata(r, "seconds", 42.0)
        r.write(StringIO.StringIO())
       
    def test_ignore_asv(self):
        """Confirm that ASV is ignored."""
        # Test exceptions
        for i in range(1,8):
            sio = StringIO.StringIO(standardParams % i)
            p, r = di.interfacing._read_parameters_stream(stream=sio,ignore_asv=True)
            set_function(r) 
            set_gradient(r) 
            set_hessian(r)
            set_metadata(r, 0, 42.0)
            r.write(StringIO.StringIO())
        # Test write-time ignoring
        sio = StringIO.StringIO(standardParams % 3)
        p, r = di.interfacing._read_parameters_stream(stream=sio,ignore_asv=False)
        set_function(r)
        set_metadata(r, "seconds", 42.0)
        rio = StringIO.StringIO()
        r.write(stream=rio,ignore_asv=True)
        expected = """  5.0000000000000000E+00 response_fn_1
  4.2000000000000000E+01 seconds
"""
        self.assertEqual(rio.getvalue(), expected)
    
    def test_results_write(self):
        """Verify Written test results"""
        sio = StringIO.StringIO(standardParams % 7)
        p, r = di.interfacing._read_parameters_stream(stream=sio)
        set_function(r) 
        set_gradient(r) 
        set_hessian(r)
        set_metadata(r, "seconds", 42.0)
        rio = StringIO.StringIO()
        r.write(stream=rio)
        expected = """  5.0000000000000000E+00 response_fn_1
[   1.0189673084127668E-266 -6.3508646783183408E-264 ]
[[   1.0000000000000000E+00   2.0000000000000000E+00
     2.0000000000000000E+00   3.0000000000000000E+00 ]]
  4.2000000000000000E+01 seconds
"""
        self.assertEqual(rio.getvalue(), expected)
        # Test simulation failure flag
        r.fail()
        rio = StringIO.StringIO()
        r.write(stream=rio)
        expected = "FAIL\n"
        self.assertEqual(rio.getvalue(), expected)

    def test_standard_batch_support(self):
        """Verify that standard format batch params and results files are handled correctly"""
        sio = StringIO.StringIO(standardBatchParams)
        self.batch_support_helper(sio)

    def test_json_batch_support(self):
        """Verify that standard format batch params and results files are handled correctly"""
        sio = StringIO.StringIO(jsonBatchParams)
        self.batch_support_helper(sio)

    def batch_support_helper(self, stream):
        num_vars = 2
        num_fns = 1
        x1_vals = [2.109465794009156e-01, 3.222614371054804e-01]
        batch_id = 1
        eval_ids = ["1:1", "1:2"]
        
        p, r = di.interfacing._read_parameters_stream(stream, False, True, None)
        # correct number of parameter sets and response sets
        self.assertEqual(len(p),2)
        self.assertEqual(len(r),2)
        # batch ids
        self.assertEqual(p.batch_id, "1") 
        self.assertEqual(r.batch_id, "1") 
        # Indexing
        for i in range(len(p)):
            self.assertAlmostEqual(p[i]["x1"], x1_vals[i])
            self.assertEqual(r[i].eval_id, eval_ids[i])
        with self.assertRaises(IndexError):
            p[2]
        with self.assertRaises(IndexError):
            r[2]
        
        # Test iteration
        for i, pi in enumerate(p):
             self.assertEqual(pi.eval_id, eval_ids[i])
        # Test write locking
        with self.assertRaises(di.interfacing.BatchWriteError):
            r[0].write("foo")
        # Verify writes
        r[0][0].function = 1.0
        set_metadata(r[0], "seconds", 42.0)
        r[1][0].function = 2.0
        set_metadata(r[1], 0, 43.0)
        results_file = StringIO.StringIO()
        r.write(results_file)
        results_strings = results_file.getvalue().split("\n")
        # Eval 1
        self.assertEqual(results_strings[0], "#")
        val, label = results_strings[1].split()
        self.assertAlmostEqual(float(val),1.0)
        val, label = results_strings[2].split()
        self.assertAlmostEqual(float(val),42.0)
        # Eval 2
        self.assertEqual(results_strings[3], "#")
        val, label = results_strings[4].split()
        self.assertAlmostEqual(float(val),2.0)
        val, label = results_strings[5].split()
        self.assertAlmostEqual(float(val),43.0)
    
    def test_single_eval_batch(self):
        """Verify that batch objects are returned for batch = True"""
        pio = StringIO.StringIO(standardSingleEvalBatchParams)
        p, r = di.interfacing._read_parameters_stream(stream=pio, 
                           batch=True, results_file="results.out")
        self.assertIsInstance(p, di.interfacing.BatchParameters)
        self.assertEqual(len(p), 1)
        self.assertIsInstance(r, di.interfacing.BatchResults)
        self.assertEqual(len(r), 1)

    def test_batch_getitem(self):
        """Verify that Results objects are returned correctly by getitem"""
        sio = StringIO.StringIO(standardBatchParams)
        batch_params, batch_results = di.interfacing._read_parameters_stream(sio, False, True, None)
        temp_results = batch_results[0]
        self.assertTrue(temp_results is batch_results[0])

    def test_batch_setitem(self):
        """Verify that Results objects are not copied by BatchParameters setter"""
       
        sio = StringIO.StringIO(standardBatchParams)
        batch_params, batch_results = di.interfacing._read_parameters_stream(sio, False, True, None)
        temp_results = batch_results[0]
        batch_results[0] = temp_results
        self.assertTrue(temp_results is batch_results[0])  

    def test_batch_setting_exception(self):
        """Ensure BatchSettingError is raised when batch=True"""
        pio = StringIO.StringIO(standardParams % 1)
        with self.assertRaises(di.interfacing.BatchSettingError):
            p, r = di.interfacing._read_parameters_stream(stream=pio, 
                           batch=True, results_file="results.out")

    def test_type_inference(self):
        """With infer_types set to False, verify variables are all strings"""
        pio = StringIO.StringIO(standardParams % 1)
        p, r = di.interfacing._read_parameters_stream(stream=pio, results_file="results.out", infer_types=True)
        self.assertIsInstance(p["x1"], float)
        self.assertIsInstance(p["x2"], float)
        self.assertIsInstance(p["dussv_1"], str)

    def test_no_type_inference(self):
        """With infer_types set to False, verify variables are all strings"""
        pio = StringIO.StringIO(standardParams % 1)
        p, r = di.interfacing._read_parameters_stream(stream=pio, results_file="results.out", infer_types=False)
        self.assertIsInstance(p["x1"], str)
        self.assertIsInstance(p["x2"], str)
        self.assertIsInstance(p["dussv_1"], str)

    def test_types_list(self):
        """With infer_types set to False, verify variables are all strings"""
        pio = StringIO.StringIO(standardParams % 1)
        p, r = di.interfacing._read_parameters_stream(stream=pio, results_file="results.out", types=[str]*3)
        self.assertIsInstance(p["x1"], str)
        self.assertIsInstance(p["x2"], str)
        self.assertIsInstance(p["dussv_1"], str)

    def test_types_list_wrong_length(self):
        """With infer_types set to False, verify variables are all strings"""
        pio = StringIO.StringIO(standardParams % 1)
        with self.assertRaises(di.BadTypesOverride):
            di.interfacing._read_parameters_stream(stream=pio, results_file="results.out", types=[str]*2)

    def test_types_list(self):
        """With infer_types set to False, verify variables are all strings"""
        pio = StringIO.StringIO(standardParams % 1)
        types = {"x1": str, "x2": str, "dussv_1": str}
        p, r = di.interfacing._read_parameters_stream(stream=pio, results_file="results.out", types=types)
        self.assertIsInstance(p["x1"], str)
        self.assertIsInstance(p["x2"], str)
        self.assertIsInstance(p["dussv_1"], str)

    def test_results_metadata(self):
        """Verify metadata by index vs. label"""
        sio = StringIO.StringIO(standardParams % 3)
        p, r = di.interfacing._read_parameters_stream(stream=sio)
        set_function(r)
        set_gradient(r)
        # set via label, retrieve via index
        set_metadata(r, "seconds", 1.2)
        self.assertAlmostEqual(r.metadata[0], 1.2)
        # set via index, retrieve via label
        set_metadata(r, 0, 3.4)
        self.assertAlmostEqual(r.metadata["seconds"], 3.4)

    def test_dprepro(self):
        """Verify that templates are substituted correctly"""
 
        sio = StringIO.StringIO(standardParams % 3)
        p, r = di.interfacing._read_parameters_stream(stream=sio)

        # Test insertion of DakotaParams and DakotaResults
        tpl = """{"%3.1f" % DakotaParams["x1"]}
{"%3.1f" % x1}
{DakotaResults[0].asv.function}
"""
        result = di.dprepro(tpl, parameters=p, results=r)
        solution = "0.7\n0.7\nTrue\n"
        self.assertEqual(result,solution) 
        # Test insertion of extra "include" parameters
        extra = {"foo":5}
        tpl = """{"%3.1f" % DakotaParams["x1"]}
{"%3.1f" % x1}
{DakotaResults[0].asv.function}
{foo}
"""  
        result = di.dprepro(tpl, parameters=p, results=r, include=extra)
        solution = "0.7\n0.7\nTrue\n5\n"
        self.assertEqual(result,solution)
        # Test without DakotaParams or DakotaResults
        tpl = "{foo}"
        result = di.dprepro(tpl,include=extra)
        solution = "5"
        self.assertEqual(result,solution)
 
    def test_slurm_info(self):
        """Info correctly extracted from the environment."""
        # Check env without SLURM_JOBID set
        try:
            del os.environ["SLURM_JOBID"]
        except KeyError:
            pass
        self.assertRaises(parallel.MgrEnvError,parallel._get_job_info)

        # Set JOBID and set TASKS_PER_NODE
        os.environ["SLURM_JOBID"] = "1337"
        os.environ["SLURM_TASKS_PER_NODE"] = "16(x2)"
        nnodes, tpn, job_id = parallel._get_job_info()
        self.assertEqual(nnodes, 2)
        self.assertEqual(tpn, 16)
        self.assertEqual(job_id,"1337")

        # Single node
        os.environ["SLURM_TASKS_PER_NODE"] = "16"
        nnodes, tpn, job_id = parallel._get_job_info()
        self.assertEqual(nnodes, 1)
        self.assertEqual(tpn, 16)

    def test_node_list(self):
        """Relative node list is built correctly."""

        self.assertEqual( "+n0",
                parallel._get_node_list(tile=0, tile_size=16,
                                             tasks_per_node=16))
        self.assertEqual( "+n0,+n1",
                parallel._get_node_list(tile=0, tile_size=32,
                                             tasks_per_node=16))
        self.assertEqual( "+n0",
                parallel._get_node_list(tile=0, tile_size=8,
                                             tasks_per_node=16))
        self.assertEqual( "+n0",
                parallel._get_node_list(tile=1, tile_size=8,
                                             tasks_per_node=16))
        self.assertEqual( "+n1",
                parallel._get_node_list(tile=0, tile_size=16,
                    tasks_per_node=16, dedicated_master='NODE'))

        self.assertEqual( "+n1,+n2",
                parallel._get_node_list(tile=0, tile_size=32,
                    tasks_per_node=16, dedicated_master='NODE'))

        self.assertEqual( "+n0",
                parallel._get_node_list(tile=0, tile_size=8,
                    tasks_per_node=16, dedicated_master='TILE'))

        self.assertEqual( "+n1",
                parallel._get_node_list(tile=1, tile_size=8,
                    tasks_per_node=16, dedicated_master='TILE'))

        self.assertEqual( "+n1",
                parallel._get_node_list(tile=0, tile_size=16,
                    tasks_per_node=16, dedicated_master='TILE'))



    def test_static_tile(self):
        """Correct tile is returned"""
        self.assertEqual(0,
                parallel._calc_static_tile(eval_num=1, num_tiles=4))
        self.assertEqual(3,
                parallel._calc_static_tile(eval_num=4, num_tiles=4))
        self.assertEqual(0,
                parallel._calc_static_tile(eval_num=5, num_tiles=4))
        self.assertEqual(0,
                parallel._calc_static_tile(eval_num=9, num_tiles=4))
        self.assertEqual(0,
                parallel._calc_static_tile(eval_num=1, num_tiles=1))
        self.assertEqual(0,
                parallel._calc_static_tile(eval_num=100, num_tiles=1))

    def test_num_tiles(self):
        self.assertEqual(10,
                parallel._calc_num_tiles(tile_size=16, tasks_per_node=16,
                    num_nodes=10))
        self.assertEqual(1,
                parallel._calc_num_tiles(tile_size=16, tasks_per_node=16,
                    num_nodes=1))
        self.assertEqual(9,
                parallel._calc_num_tiles(tile_size=16, tasks_per_node=16,
                    num_nodes=10, dedicated_master='NODE'))
        self.assertEqual(9,
                parallel._calc_num_tiles(tile_size=16, tasks_per_node=16,
                    num_nodes=10, dedicated_master='TILE'))
        self.assertEqual(18,
                parallel._calc_num_tiles(tile_size=8, tasks_per_node=16,
                    num_nodes=10, dedicated_master='NODE'))
        self.assertEqual(19,
                parallel._calc_num_tiles(tile_size=8, tasks_per_node=16,
                    num_nodes=10, dedicated_master='TILE'))
        self.assertEqual(1,
                parallel._calc_num_tiles(tile_size=8, tasks_per_node=16,
                    num_nodes=1, dedicated_master='TILE'))
        self.assertRaises(parallel.ResourceError,parallel._calc_num_tiles,
                    tile_size=8, tasks_per_node=16, num_nodes=1, 
                    dedicated_master='NODE')
        self.assertRaises(parallel.ResourceError,parallel._calc_num_tiles,
                    tile_size=16, tasks_per_node=16, num_nodes=1, 
                    dedicated_master='TILE')


class PythonDirectInterfaceTest(unittest.TestCase):
    def test_parameters_from_dict_variable_order(self):
        params, _ = di.interfacing._read_params_from_dict(direct_dense_params)
        labels = [label for label in params]
        self.assertListEqual(direct_dense_params["variable_labels"], labels)
        
    def test_parameters_from_dict_variable_types(self):        
        params, _ = di.interfacing._read_params_from_dict(direct_dense_params)
        expected_types = [float, int, str, float, float]
        actual_types = [type(value) for label, value in params.items()]
        self.assertListEqual(expected_types, actual_types)

    def test_parameters_from_dict_variable_types(self):        
        params, _ = di.interfacing._read_params_from_dict(direct_dense_params)
        expected_types = [float, int, str, float, float]
        actual_types = [type(value) for label, value in params.items()]
        self.assertListEqual(expected_types, actual_types)

    def test_parameters_from_dict_variable_values(self):
        params, _ = di.interfacing._read_params_from_dict(direct_dense_params)
        expected_values = []
        expected_values.append(direct_dense_params["cv"][0])
        expected_values += direct_dense_params["div"]
        expected_values += direct_dense_params["dsv"]
        expected_values += direct_dense_params["drv"]
        expected_values.append(direct_dense_params["cv"][1])
        actual_values = [value for label, value in params.items()]
        for e, a in zip(expected_values, actual_values):
            self.assertAlmostEqual(e, a)

    def test_parameters_from_dict_asv(self):
        _, results = di.interfacing._read_params_from_dict(direct_dense_params)
        def asv_to_int(asv):
            r = 0
            if asv.function:
                r = r | 1
            if asv.gradient:
                r = r | 2
            if asv.hessian:
                r = r | 4
            return r

        expected_asv = direct_dense_params["asv"]
        actual_asv = [asv_to_int(response.asv) for label, response in results.items()]
        self.assertListEqual(expected_asv, actual_asv)

    def test_parameters_from_dict_dvv(self):
        _, results = di.interfacing._read_params_from_dict(direct_dense_params)
        expected_dvv = [direct_dense_params["cv_labels"][id-1] for id in direct_dense_params["dvv"]]
        self.assertListEqual(expected_dvv, results.deriv_vars)
    
    def test_parameters_from_dict_eval_id(self):
        params, results = di.interfacing._read_params_from_dict(direct_dense_params)
        expected_eid = str(direct_dense_params["eval_id"])
        params_eid = params.eval_id
        results_eid = results.eval_id
        self.assertEqual(expected_eid, params_eid)
        self.assertEqual(expected_eid, results_eid)

    def test_parameters_from_dict_function_labels(self):
        _, results = di.interfacing._read_params_from_dict(direct_dense_params)
        expected_labels = direct_dense_params["function_labels"] 
        actual_labels = [label for label in results]
        self.assertListEqual(expected_labels, actual_labels)

    def test_parameters_from_dict_an_comps(self):
        params, _ = di.interfacing._read_params_from_dict(direct_dense_params)
        expected_ac = direct_dense_params["analysis_components"]
        actual_ac = params.an_comps
        self.assertListEqual(expected_ac, actual_ac)

    def test_parameters_from_dict_metadata(self):
        _, results = di.interfacing._read_params_from_dict(direct_dense_params)
        expected_md = direct_dense_params["metadata_labels"]
        actual_md = [md for md in results.metadata]
        self.assertListEqual(expected_md, actual_md)

    def test_parameters_from_dict_num_vars_funcs(self):
        params, results = di.interfacing._read_params_from_dict(direct_dense_params)
        expected_nv = direct_dense_params["variables"]
        expected_nf = direct_dense_params["functions"]
        actual_nv = params.num_variables
        actual_nf = results.num_responses
        self.assertEqual(expected_nv, actual_nv)
        self.assertEqual(expected_nf, actual_nf)

    def test_return_direct_results_dict(self):
        _, results = di.interfacing._read_params_from_dict(direct_dense_params)
        for i in range(3):
            results[i].function = i + 1.0
            results[i].gradient = [i + 1.0]
            results[i].hessian = [[i + 1.0]]
        results.metadata[0] = "baz"
        d = results.return_direct_results_dict()
        self.assertIsInstance(d, dict)
        
        expected_fns = [1.0, 2.0, 3.0]
        expected_grads = [[1.0], [2.0], [3.0]]
        expected_hess = [ [[1.0]], [[2.0]], [[3.0]] ]

        for i in range(3):
            self.assertAlmostEqual(expected_fns[i], d["fns"][i])
            self.assertAlmostEqual(expected_grads[i][0], d["fnGrads"][i][0]) 
            self.assertAlmostEqual(expected_hess[i][0][0], d["fnHessians"][i][0][0]) 
 
    def test_python_interface_decorator(self):
        @di.python_interface
        def driver(params, results):
            self.assertIsInstance(params, di.Parameters)
            self.assertIsInstance(results, di.Results)
            for i in range(3):
                results[i].function = i + 1.0
                results[i].gradient = [i + 1.0]
                results[i].hessian = [[i + 1.0]]
            results.metadata[0] = "baz"
            return results

        d = driver(direct_dense_params)
        self.assertIsInstance(d, dict)
        self.assertTrue("fns" in d)
        self.assertIsInstance(d["fns"], list)
        self.assertTrue("fnGrads" in d)
        self.assertIsInstance(d["fnGrads"], list)
        self.assertTrue("fnHessians" in d)
        self.assertIsInstance(d["fnHessians"], list)

    def test_return_direct_results_dict_asv_problem(self):
        _, results = di.interfacing._read_params_from_dict(direct_dense_params)
        with self.assertRaises(di.ResponseError):
            results.return_direct_results_dict()

    def test_return_direct_results_dict_metadata_problem(self):
        _, results = di.interfacing._read_params_from_dict(direct_dense_params)
        for i in range(3):
            results[i].function = i + 1.0
            results[i].gradient = [i + 1.0]
            results[i].hessian = [[i + 1.0]]
        with self.assertRaises(di.ResultsError):
            results.return_direct_results_dict()

    def test_parameters_from_dict_sparse(self):
        params, results = di.interfacing._read_params_from_dict(direct_sparse_params)
        labels = [label for label in params]
        self.assertListEqual(direct_sparse_params["variable_labels"], labels)
        self.assertEqual(params[0], direct_sparse_params["cv"][0])
        self.assertEqual(results.num_responses, direct_sparse_params["functions"])
        results[0].function = 0.0
        results.return_direct_results_dict()
        
class TestDeepCopy(unittest.TestCase):
    def test_results(self):
        """Results objects are correctly deepcopied"""
        pio = StringIO.StringIO(standardParams % 3)
        _, r = di.interfacing._read_parameters_stream(stream=pio, results_file="results.out")
        r["response_fn_1"].gradient = [1.0, 2.0]
        cr = copy.deepcopy(r)
        self.assertFalse(r is cr)
        self.assertFalse(r._responses is cr._responses)
        self.assertFalse(r["response_fn_1"]._gradient is cr["response_fn_1"]._gradient)
        self.assertListEqual(r["response_fn_1"].gradient, cr["response_fn_1"].gradient)

    def test_parameters(self):
        """Results objects are correctly deepcopied"""
        pio = StringIO.StringIO(standardParams % 3)
        p, _ = di.interfacing._read_parameters_stream(stream=pio, results_file="results.out")
        cp = copy.deepcopy(p)
        self.assertFalse(p is cp)
        self.assertFalse(p._variables is cp._variables)
        self.assertFalse(p.metadata is cp.metadata)
        self.assertListEqual(p.metadata, cp.metadata)

    def test_batch_parameters(self):
        sio = StringIO.StringIO(standardBatchParams)
        p, _ = di.interfacing._read_parameters_stream(sio, False, True, None)
        cp = copy.deepcopy(p)
        self.assertFalse(cp is p)
        self.assertFalse(cp[0] is p[0])

    def test_batch_results(self):
        sio = StringIO.StringIO(standardBatchParams)
        _, r = di.interfacing._read_parameters_stream(sio, False, True, None)
        cr = copy.deepcopy(r)
        self.assertFalse(cr is r)
        self.assertFalse(cr[0] is r[0])
        self.assertTrue(cr[0].format_matches(r[0]))

# todo: test iteration, integer access

class TestBatchSplitter(unittest.TestCase):
    def setUp(self) -> None:
        self.dakota_batch_params_file = "test_batch_splitter_dakota_batch.in"
        with open(self.dakota_batch_params_file, "w") as f:
            print(standardBatchParams, file=f)
        
        batch_lines = standardBatchParams.split('\n')
        batch_lines = [line + "\n" for line in batch_lines]
        self.dakota_batch_params_lines = [batch_lines[:12], batch_lines[12:]]

        self.aprepro_batch_params_file = "test_batch_splitter_aprepro_batch.in"
        with open(self.aprepro_batch_params_file, "w") as f:
            print(apreproBatchParams, file=f)
        batch_lines = apreproBatchParams.split('\n')
        batch_lines = [line + "\n" for line in batch_lines]
        self.aprepro_batch_params_lines = [batch_lines[:15], batch_lines[15:]]

    def tearDown(self) -> None:
        os.remove(self.dakota_batch_params_file)
        os.remove(self.aprepro_batch_params_file)

    def test_len_dakota_batch_params(self):
        b = di.BatchSplitter(self.dakota_batch_params_file)
        self.assertEqual(len(b), 2)

    def test_dakota_nums(self):
        b = di.BatchSplitter(self.dakota_batch_params_file)
        self.assertEqual(b.batch_id, "1")
        self.assertListEqual(b.eval_nums,[1, 2])

    def test_format_dakota_batch_params(self):
        b = di.BatchSplitter(self.dakota_batch_params_file)
        self.assertEqual(b.format, "DAKOTA")

    def test_get_dakota_batch_params(self):
        b = di.BatchSplitter(self.dakota_batch_params_file)
        for i in range(len(b)):
            self.assertListEqual(b[i], self.dakota_batch_params_lines[i])
        
    def test_iterate_dakota_batch_params(self):
        b = di.BatchSplitter(self.dakota_batch_params_file)
        for in_params, base_params in zip(b, self.dakota_batch_params_lines):
            self.assertListEqual(in_params, base_params)

    def test_format_aprepro_batch_params(self):
        b = di.BatchSplitter(self.aprepro_batch_params_file)
        self.assertEqual(b.format, "APREPRO")
 
    def test_get_aprepro_batch_params(self):
        b = di.BatchSplitter(self.aprepro_batch_params_file)
        for i in range(len(b)):
            self.assertListEqual(b[i], self.aprepro_batch_params_lines[i])

    def test_aprepro_nums(self):
        b = di.BatchSplitter(self.aprepro_batch_params_file)
        self.assertEqual(b.batch_id, "1")
        self.assertListEqual(b.eval_nums,[1, 2])


    

unittest.main()
