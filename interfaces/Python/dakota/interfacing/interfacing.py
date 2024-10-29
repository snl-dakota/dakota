# -*- coding: utf-8 -*-
from __future__ import print_function, unicode_literals
from io import open
import io
import collections
import copy
import functools
import json
import linecache
import math
import pprint
import re
import sys
from typing import Generator, Iterable, List, NewType, Tuple, Union, Dict
import copy
from . import dprepro as dprepro_mod

__author__ = 'J. Adam Stephens'
__copyright__ = 'Copyright 2014-2024 National Technology & Engineering Solutions of Sandia, LLC (NTESS)'
__license__ = 'GNU Lesser General Public License'

PYTHON3 = True if sys.version_info[0] == 3 else False 
PYTHON2 = True if sys.version_info[0] == 2 else False

if PYTHON3:
    from itertools import zip_longest
else:
    from itertools import izip_longest as zip_longest

#### Exceptions


class ResponseError(Exception):
    pass

class ResultsError(Exception):
    pass

class MissingSourceError(Exception):
    pass

class ParamsFormatError(Exception):
    pass

class BatchSettingError(Exception):
    pass

class BatchWriteError(Exception):
    pass

class ResultsUpdateError(Exception):
    pass

class BadTypesOverride(Exception):
    pass

class EvalNumberError(Exception):
    pass

# Constants
# specify an unnamed results file
UNNAMED = True
# file formats
APREPRO = 1
STANDARD = 1
JSON = 2
DIRECT = 3

#### Types

Key = NewType('Key', Union[int, str])
VarValue = NewType('VarValue', Union[int, str, float])
Gradient = NewType('Gradient', Iterable[float])
Hessian = NewType ('Hessian', Iterable[Iterable[float]])
FileFormat = NewType('FileFormat', int)

#### Class definitions

class Parameters:
    """Access variables and analysis components from a Dakota parameters file
    
    Parameters objects typically should be constructed by the convenience 
    function ``dakota.interfacing.read_parameters_file``.

    Variable values can be accessed by name or by index using []. Analysis
    components are accessible by index only using the an_comp attribute.
    Paramaters objects can be iterated, yielding the variable descriptors.
    Other ways to iterate the object are the items() method, which yields
    tuples of descriptor and variable value, and values() method, which
    yields only the values.

    Attributes:
        an_comps: List of strings containing the analysis components.
        eval_id: Evaluation id (string).
        eval_num: Evaluation number (final token in eval_id) (int).
        format: constant indicating format of the file, APREPRO (1), STANDARD (2), JSON (3), or DIRECT (4)
        descriptors: List of the variable descriptors (read-only)
        num_variables: Number of variables (read-only)
        num_an_comps: Number of analysis components (read-only)
        metadata: Names of requested metadata fields
        num_metadata: Number of requested metadata fields (read-only)
    """

    def __init__(self, format: FileFormat, variables: Dict[str, VarValue], an_comps: List[str], 
            eval_id: str, metadata: List[str], infer_types: True, types: Union[list, dict]=None) -> None:
        self.format = format
        if format not in (APREPRO, STANDARD, JSON, DIRECT):
            raise TypeError("format parameter must be APREPRO, STANDARD, JSON, or DIRECT")
        self._variables = copy.deepcopy(variables)
        self.an_comps = list(an_comps)
        self.eval_id = str(eval_id)
        self.eval_num = int(eval_id.split(":")[-1])
        self.metadata = list(metadata)
        if self.format in (APREPRO, STANDARD):  # JSON and DIRECT formats are presumed to have the proper types
            if isinstance(types, list):
                self._set_types_from_list(types)
            elif isinstance(types, dict):
                self._set_types_from_dict(infer_types, types)
            elif infer_types:
                self._infer_types()
        self._batch = False

    def _set_types_from_list(self, types: dict) -> None:
        if len(types) != len(self._variables):
            raise BadTypesOverride("Length of types list is %d but number of variables is %d" %(len(types), len(self._variables)))
        for t, (k, v) in zip(types, self._variables.items()):
            self._variables[k] = t(v)

    def _set_types_from_dict(self, infer_types: bool, types: dict) -> None:
        for k, v in self._variables.items():
            if k in types:
                self._variables[k] = types[k](v)
            elif infer_types:
                self._variables[k] = self._infer_single_type(v)

    def _infer_types(self) -> None:
        # Convert variables to the appropriate type. The possible types
        # are int, float, and string. The variables are already strings.
        # TODO: Consider a user option to override this behavior and keep
        # everything as a string, or provide access to the original strings
        for k, v in self._variables.items():
            self._variables[k] = self._infer_single_type(v)

    def _infer_single_type(self, v: str) -> VarValue:
        # string variables often contain underscores and might be mistaken for
        # floats or ints
        converted = v
        if '_' not in v:    
            try:
                converted = int(v)
            except ValueError:
                try:
                    converted = float(v)
                except ValueError:
                    pass
                pass
        return converted    


    @property
    def descriptors(self):
        return [k for k in self._variables.keys()]

    def __getitem__(self, key: Key):
        if type(key) is int:
            return self._variables[self.descriptors[key]]
        else:
            return self._variables[key]

    def __setitem__(self, key: Key, value: VarValue):
        if type(key) is int:
            self._variables[self.descriptors[key]] = value
        else:
            self._variables[key] = value

    @property
    def num_variables(self) -> int:
        return len(self._variables)

    @property
    def num_an_comps(self) -> int:
        return len(self.an_comps)

    @property
    def num_metadata(self) -> int:
        return len(self.metadata)

    def __iter__(self) -> Generator[str, None, None]:
        for name in self._variables:
            yield name

    def items(self) -> Generator[Tuple[str, VarValue], None, None]:
        if PYTHON3:
            it = self._variables.items()
        else:
            it = self._variables.iteritems()
        for name, var in it:
            yield name, var
    
    def values(self) -> VarValue:
        if PYTHON3:
            it = self._variables.values()
        else:
            it = self._variables.itervalues()
        for value in self._variables.values():
            yield value

    def _set_batch(self, flag):
        self._batch = flag

    @property
    def batch(self) -> bool:
        return self._batch

    def __deepcopy__(self, memo) -> 'Parameters':
        cls = self.__class__
        result = cls.__new__(cls)
        memo[id(self)] = result
        for k, v in self.__dict__.items():
            setattr(result, k, copy.deepcopy(v, memo))
        return result
 
# Datatype to hold ASV element for a single response. function, gradient,
# and hession are set to True or False. 
_ASVType = collections.namedtuple("_ASVType",["function","gradient","hessian"])


# A class to hold the ASV and data for a single response
class Response:
    """Active set vector and data for a single response.

    Instances of this class are constructed by Results objects.

    Attributes:
        asv: namedtuple with three members, function, gradient, and hessian.
            Each are a boolean indicating whether Dakota requested the
            associated information for the response. namedtuples can be
            accessed by index or by member.
        function: Function value for the response. A ResponseError
            is raised if Dakota did not request the function value (and
            ignore_asv is False).
        gradient: Gradient for the response. Gradients must be a 1D iterable
            of values that can be converted to float. A ResponseError
            is raised if Dakota did not request the gradient (and ignore_asv is
            False), or if the number of elements does not equal the number of 
            derivative variables.
        hessian: Hessian value for the response. Hessians must be an iterable
            of iterables (e.g. a 2D numpy array or list of lists). A 
            ResponseError is raised if Dakota did not request the Hessian 
            (and ignore_asv is False), or if the dimension does not correspond 
            correctly with the number of derivative variables.    
    """
    def __init__(self, descriptor: str, num_deriv_vars: int, ignore_asv: bool, asv: int) -> None:
        self._descriptor = descriptor
        self._num_deriv_vars = num_deriv_vars
        int_asv = int(asv)
        self.asv = _ASVType(int_asv & 1 == 1, int_asv & 2 == 2, 
                int_asv &4 == 4)
        self._function = None
        self._gradient = None
        self._hessian = None
        self._ignore_asv = ignore_asv
        self._batch = False
    
    def shape_matches(self, other: 'Response') -> bool:
        """True if the "shape" of other matches self

           Shape includes the descriptor, derivative variables, asv,
           ignore_asv flag, and batch flag"""
        return self._descriptor == other._descriptor and \
           self._num_deriv_vars == other._num_deriv_vars and \
           self.asv == other.asv and \
           self._ignore_asv == other._ignore_asv and \
           self._batch == other._batch
    

    @property
    def function(self) -> float:
        return self._function

    @function.setter
    def function(self, val: float) -> None:
        if not (self._ignore_asv or self.asv.function):
            raise ResponseError("Function value not requested for '%s'." 
                    % self._descriptor)
        self._function = float(val)

    @property
    def gradient(self) -> Union[Gradient, None]:
        # try/except needed bc Results._write_results with ignore_csv = True 
        # tests for existence of this member        
        try: 
            return list(self._gradient) # return a copy
        except TypeError:
            return None

    @gradient.setter
    def gradient(self, val: Gradient) -> None:
        if not (self._ignore_asv or self.asv.gradient):
            raise ResponseError("Gradient not requested for '%s'." 
                    % self._descriptor)
        self._gradient = [float(e) for e in val]
        if len(self._gradient) != self._num_deriv_vars:
            raise ResponseError("Length of gradient must equal number of "
                    "derivative variables.")

    @property
    def hessian(self) -> Union[Hessian, None]:
        # try/except needed bc Results._write_results with ignore_csv = True 
        # tests for existence of this member        
        try:
            return copy.deepcopy(self._hessian)
        except:
            return None

    @hessian.setter
    def hessian(self, val: Hessian) -> None:
        if not (self._ignore_asv or self.asv.hessian):
            raise ResponseError("Hessian not requested for '%s'." 
                    % self._descriptor)
        ### validate dimensions
        rctr = 0
        for r in val:
            rctr += 1
            cctr = 0
            for c in r:
                cctr += 1
            if cctr != self._num_deriv_vars:
                raise ResponseError("Hessian must be square and size "
                        "num_deriv_variables.")
        if rctr != self._num_deriv_vars:
            raise ResponseError("Hessian must be square and size "
                    "num_deriv_variables.")
        ### copy the Hessian
        self._hessian = []
        for r in val:
            row = []
            for c in r:
                row.append(float(c))
            self._hessian.append(row)
    
    def __deepcopy__(self, memo) -> 'Response':
        cls = self.__class__
        result = cls.__new__(cls)
        memo[id(self)] = result
        for k, v in self.__dict__.items():
            setattr(result, k, copy.deepcopy(v, memo))
        return result
 
class IndexableOrderedDict(collections.OrderedDict):
    """Specialization of OrderedDict that allows access via key or index."""

    def __getitem__(self, key):
        if type(key) is int:
            key_from_index = list(self.keys())[key]
            return super(IndexableOrderedDict, self).__getitem__(key_from_index)
        else:
            return super(IndexableOrderedDict, self).__getitem__(key)

    def __setitem__(self, key, value):
        if type(key) is int:
            # This will only work if the key already exists... raise KeyError?
            key_from_index = list(self.keys())[key]
            return super(IndexableOrderedDict, self).__setitem__(key_from_index, value)
        else:
            return super(IndexableOrderedDict, self).__setitem__(key, value)


class Results:
    """Collect response data and write to results file.

    Results objects typically should be constructed by the convenience function
    ``dakota.interfacing.read_parameters_file``.

    Each response is represented by a Response object, and can be accessed
    by name or by index using []. Results objects support iteration, yielding
    the response descriptors. Other iterators are provided by the items() method,
    which yields a tuple of descriptor and Response object, and responses(),
    which yields Response objects.

    Attributes:
        eval_id: Evaluation id (a string).
        eval_num: Evaluation number (final token in eval_id) (int).
        metadata: Dictionary indexable by metadata field name or by index
        aprepro_format: Boolean indicating whether the parameters file was in
            aprepro (True) or Dakota (False) format.
        descriptors: List of the response descriptors (read-only)
        num_responses: Number of variables (read-only)
        deriv_vars: List of the derivative variables (read-only)
        num_deriv_vars: Number of derivative variables (read-only)
        ignore_asv: If True, response set will be validated against ASV before writing
        results_file: Name of results file that will be written
    """

    def __init__(self, format: int, responses: dict, 
            deriv_vars: List[str], eval_id: str, metadata: List[str],
            ignore_asv: bool=False, results_file: str=None):
        self.format = format
        if format not in (APREPRO, STANDARD, JSON, DIRECT):
            raise TypeError("format parameter must be APREPRO, STANDARD, JSON, or DIRECT")
        self.ignore_asv = ignore_asv
        self._deriv_vars = deriv_vars[:]
        num_deriv_vars = len(deriv_vars)
        self._responses = collections.OrderedDict()
        for t, v in responses.items():
            self._responses[t] = Response(t, num_deriv_vars, ignore_asv, 
                    int(v)) 
        self.metadata = IndexableOrderedDict()
        for m in metadata:
            self.metadata[m] = None
        self.results_file = results_file
        self.eval_id = eval_id
        self.eval_num = int(eval_id.split(":")[-1])
        self._failed = False
        self._batch = False

    def __getitem__(self, key: Key) -> Response:
        if type(key) is int:
            return self._responses[self.descriptors[key]]
        else:
            return self._responses[key]

    def format_matches(self, other: 'Results') -> bool:
        matches = True
        matches = matches and (self.format == other.format)
        matches = matches and (self.ignore_asv == other.ignore_asv)
        matches = matches and all(s == o for s, o in 
                                  zip_longest(self._deriv_vars, other._deriv_vars))
        matches = matches and all(s[1].shape_matches(o[1]) for s, o in
                                  zip_longest(self._responses.items(), other._responses.items()))
        matches = matches and (self.results_file == other.results_file)
        matches = matches and (self.eval_id == other.eval_id)
        matches = matches and (self.eval_num == other.eval_num)
        matches = matches and (self._batch == other._batch)
        return matches
 
    def __deepcopy__(self, memo) -> 'Results':
        cls = self.__class__
        result = cls.__new__(cls)
        memo[id(self)] = result
        for k, v in self.__dict__.items():
            setattr(result, k, copy.deepcopy(v, memo))
        return result
        
    @property
    def num_deriv_vars(self) -> int:
        return len(self._deriv_vars)

    @property
    def num_responses(self) -> int:
        return len(self._responses)

    def write(self, stream: io.IOBase=None, ignore_asv:bool=None, json: bool=False) -> None:
        """Write the results to the Dakota results file.

        Keyword Args:
            stream: Write results to this I/O stream. Overrides results_file
                specified when the object was constructed.
            ignore_asv: Ignore the active set vector while writing the response
                data to the results file (or stream). Overrides ignore_asv
                setting provided at construct time.

        Raises:
            dakota.interfacing.MissingSourceError: No results_file was provided 
                at construct time, and no stream was provided to the method
                call.
            dakota.interfacing.ResponseError: A result requested by Dakota is 
                missing (and ignore_asv is False).
            dakota.interfacing.ResultsError: A metadata result requested by
                Dakota is missing.
        """
        if self._batch:
            raise BatchWriteError("write() called on Results object in " + \
                    "batch mode. Write using BatchResults container")
            
        write_function = _write_json_results if json else _write_standard_results
        my_ignore_asv = self.ignore_asv
        if ignore_asv is not None:
            my_ignore_asv = ignore_asv

        _check_against_asv(self, my_ignore_asv)
        if stream is None:
            if self.results_file is None:
                raise MissingSourceError("No stream specified and no "
                        "results_file provided at construct time.")
            else:
                with open(self.results_file, "w", encoding='utf8') as ofp:
                    write_function(ofp, self, my_ignore_asv)
        else:
            write_function(stream, self, my_ignore_asv)

    def direct_results_dict(self, ignore_asv: Union[bool, None]=None) -> dict:
        """Create and return a direct python interface results dictionary.

        Keyword Args:

        Raises:
        """
        my_ignore_asv = self.ignore_asv
        if ignore_asv is not None:
            my_ignore_asv = ignore_asv

        _check_against_asv(self, my_ignore_asv)

        results_dict = {}
        results_dict['fns'] = []
        results_dict['fnGrads'] = []
        results_dict['fnHessians'] = []
        for t, v in self._responses.items():
            if v.asv.function:
                results_dict['fns'].append(v.function)
            if v.asv.gradient:
                results_dict['fnGrads'].append(v.gradient)
            if v.asv.hessian:
                results_dict['fnHessians'].append(v.hessian)
        
        results_dict['metadata'] = [value for label, value in self.metadata.items()] 
        return results_dict
    

    def _set_batch(self, flag: bool) -> None:
        self._batch = flag

    @property
    def descriptors(self) -> List[str]:
        return [k for k in self._responses.keys()]

    @property
    def deriv_vars(self) -> List[str]:
        return list(self._deriv_vars)

    def __iter__(self) -> Generator[str, None, None]:
        for name in self._responses:
            yield name

    def items(self) -> Generator[Tuple[str, Response], None, None]:
        if PYTHON3:
            it = self._responses.items()
        else:
            it = self._responses.iteritems()
        for name, response in it:
            yield name, response
    
    def responses(self) -> Generator[Response, None, None]:
        if PYTHON3:
            it = self._responses.values()
        else:
            it = self._responses.itervalues()
        for value in self._responses.values():
            yield value

    def fail(self) -> None:
        """Set the FAIL attribute. 
        
        When the results file is written, it will contain only the word FAIL"""
        self._failed = True
        
    def failed(self) -> bool:
        return self._failed

    @property
    def batch(self) -> bool:
        return self._batch

                
class BatchParameters:
    """Access variables and analysis components from a batch parameters file

    BatchParameters objects are simple collections of Parameters objects. 
    Individual Parameters objects can be access by index or by iterating 
    the BatchParameters object.

    BatchParameters objects typically should be constructed by the convenience
    function ``dakota.interfacing.read_parameters_file``.

    Attributes:
        batch_id: Batch id (string).
    """

    def __init__(self, param_sets: List[Parameters]) -> None:
        self._eval_params = param_sets
        eval_id = self._eval_params[0].eval_id
        tokens = eval_id.split(":")
        try:
            self._batch_id = tokens[-2]
        except IndexError:
            raise BatchSettingError("batch set to True, but parameters " +
                              "file does not appear to be a batch file.")
        for p in self._eval_params:
            p._set_batch(True)

    @property
    def batch_id(self) -> str:
        return self._batch_id

    def __getitem__(self,index) -> Parameters:
        return self._eval_params[index]

    def __len__(self) -> int:
        return len(self._eval_params)

    def __iter__(self) -> Generator[Parameters, None, None]:
        for p in self._eval_params:
            yield p

    def __deepcopy__(self, memo) -> 'BatchParameters':
        cls = self.__class__
        result = cls.__new__(cls)
        memo[id(self)] = result
        for k, v in self.__dict__.items():
            setattr(result, k, copy.deepcopy(v, memo))
        return result
 
class BatchResults:
    """Collect response data and write to results file for a batch evaluation.

    BatchResults objects should be constructed by the convenience function
    ``dakota.interfacing.read_parameters_file``.

    BatchResults objects are simple collections of the Results objects for a 
    batch evaluation. The individual Results objects can be accessed by index
    or by iterating the BatchResults object.

    Attributes:
        batch_id: batch id (string).
        ignore_asv: True if the underlying Results objects will perform ASV
            checking when .write() is called
        results_file: Name of results file that will be written
    """

    def __init__(self, results_sets: List[Results]) -> None:
        self._eval_results = results_sets
        eval_id = self._eval_results[0].eval_id
        tokens = eval_id.split(":")
        try:
            self._batch_id = tokens[-2]
        except IndexError:
            raise BatchSettingError("batch set to True, but parameters " +
                              "file does not appear to be a batch file.")
        self._ignore_asv = self._eval_results[0].ignore_asv
        self._results_file = self._eval_results[0].results_file
        for r in self._eval_results:
            r._set_batch(True)

    @property
    def batch_id(self) -> str:
        return self._batch_id

    def __getitem__(self, index: int) -> Results:
        return self._eval_results[index]

    def __setitem__(self, index: int, other: Results) -> None:
        self._eval_results[index] = other

    def write(self, stream:io.IOBase=None, ignore_asv:bool=None, json: bool=False) -> None:
        my_ignore_asv = self._ignore_asv
        if ignore_asv is not None:
            my_ignore_asv=ignore_asv
        
        write_function = self._write_batch_to_json if json else self._write_batch_to_standard        

        if stream is None:
            if self._results_file is None:
                raise MissingSourceError("No stream specified and no "
                        "results_file provided at construct time.")
            else:
                with open(self._results_file, "w", encoding='utf8') as ofp:
                    write_function(ofp, my_ignore_asv)
        else:
            for r in self._eval_results:
                write_function(stream, my_ignore_asv)

                
                
    def _write_batch_to_standard(self, stream: io.IOBase, ignore_asv: bool) -> None:
        for r in self._eval_results:
            stream.write("#\n")
            _write_standard_results(stream, r, ignore_asv)
            
    def _write_batch_to_json(self, stream: io.IOBase, ignore_asv: bool) -> None:
        json_batch = [_encode_results_as_json(r, ignore_asv) for r in self._eval_results]
        json.dump(json_batch, stream)
        
    def direct_results_dict(self):
        return [r.direct_results_dict() for r in self._eval_results]
            
                
    def __len__(self) -> int:
        return len(self._eval_results)

    def __iter__(self) -> int:
        for r in self._eval_results:
            yield r

    def __deepcopy__(self, memo) -> 'BatchResults':
        cls = self.__class__
        result = cls.__new__(cls)
        memo[id(self)] = result
        for k, v in self.__dict__.items():
            setattr(result, k, copy.deepcopy(v, memo))
        return result
 

class BatchSplitter:
    """Split a batch parameters files into individual parameters files
    
    - Content of individual parameters files as list of newline terminated lines
      can be accessed by index using [].
    - Iterating the object yields content of individual parameter files as lists of
      newline terminated lines.

    Keyword Args:
        parameters_file (str): Name of batch parameters file; if none, 1st command line argument is used

    Attributes:
        batch_id: ID of the batch
        eval_nums: List of evaluation numbers
        format: format of parameters file ("DAKOTA" or "APREPRO")
        parameters_file: Name of parameters file
    """
    def __init__(self, parameters_file: str=None):
        self._parameters_file = sys.argv[1] if parameters_file is None else parameters_file
        self._format = self._detect_format()
        self._ending_line_numbers, self._batch_id, self._eval_nums = self._parse_parameters_file()

    def _detect_format(self):
        """Detect format of parameters file
        
        Returns:
            "DAKOTA" or "APREPRO"
        """
        with open(self._parameters_file, "r") as f:
            line = f.readline()
        dakota_format_match = _pRE["DAKOTA"]["num_variables"].match(line)
        aprepro_format_match = _pRE["APREPRO"]["num_variables"].match(line)
        if aprepro_format_match is not None:
            return "APREPRO"
        elif dakota_format_match is not None: 
            return "DAKOTA"
        else:
            raise ParamsFormatError("Unrecognized parameters file format.")

    def _parse_parameters_file(self) -> Tuple[List[int], str, List[int]]:
        """Return a list of ending line numbers for each parameter set
        
        Line numbers begin at 1, and ending line numbers are for the first line
        after the parameter set.
        """
        line_numbers = []
        eval_ids = []
        with open(self._parameters_file, "r") as f:
            # Advance past the first line so that the 0th batch is recorded
            # when the 1st evaluation is reached
            f.readline() 
            for i, line in enumerate(f, start=2):
                if _pRE[self._format]["num_variables"].match(line) is not None:
                    line_numbers.append(i)
                eval_id_m = _pRE[self._format]["eval_id"].match(line)
                if eval_id_m is not None:
                   eval_ids.append(eval_id_m.group("value"))
            line_numbers.append(i+1)
        try:
            batch_id = eval_ids[0].split(":")[-2]
        except IndexError:
            raise BatchSettingError("Parameters file does not appear to be a batch file.")
        eval_nums = [int(eval_id.split(":")[-1]) for eval_id in eval_ids]
        return line_numbers, batch_id, eval_nums
    
    def __getitem__(self, index: int) -> List[str]:
        """Return list of lines for parameter set with index idx
        
        Lines end with newline
        """
        lines = []
        try:
            start_line_num = 1 if index == 0 else self._ending_line_numbers[index-1]
            for i in range(start_line_num, self._ending_line_numbers[index]):
                lines.append(linecache.getline(self._parameters_file, i))
        except IndexError:
            raise IndexError(f"no evaluate with index {index}")
        return lines

    @property
    def batch_id(self) -> str:
        return self._batch_id

    @property
    def eval_nums(self) -> List[int]:
        return self._eval_nums
    
    def __len__(self) -> int:
        """Return size of batch"""
        return len(self._ending_line_numbers)
    
    def __iter__(self) -> Generator[List[str], None, None]:
        for i in range(len(self)):
            yield self[i]

    @property
    def parameters_file(self) -> str:
        return self._parameters_file
    
    @property
    def format(self) -> FileFormat:
        return STANDARD if self._format == "DAKOTA" else APREPRO

    def write(self, filename:  str, index: int = None, eval_num: int = None) -> None:
        """Write the parameter set referred to either by 0-based index or Dakota evaluation number to file"""
        if (index is None and eval_num is None) or (index is not None and eval_num is not None):
            raise ValueError("BatchSplitter.write must be called with either an index or eval_num")
        idx = self._index_from_eval_num(eval_num) if eval_num is not None else index
        lines = self[idx]
        with open(filename, "w") as f:
            f.writelines(lines)
    
    def _index_from_eval_num(self, eval_num: int) -> int:
        try:
            idx = self._eval_nums.index(eval_num)
        except ValueError:
            raise EvalNumberError(f"Batch does not contain a parameter set with evaluation number {eval_num}")
        return idx

### Free functions for reading and writing parameters and results files

def _write_standard_results(stream: io.IOBase, results: Results, ignore_asv: bool) -> None:
    # Write FAIL if set
    if results.failed():
        print("FAIL", file=stream)
        return
    # Write function values
    for t, v in results.items():
        if (v.asv.function or ignore_asv) and v.function is not None:
            print("%24.16E %s" %(v.function, t), file=stream)
    # Write gradients
    for t, v in results.items():
        if (v.asv.gradient or ignore_asv) and v.gradient is not None:
            print("[ ",file=stream, end="")
            for e in v.gradient:
                print(" %24.16E" % e,file=stream,end="")
            print(" ]",file=stream)
    # Write Hessians
    for t, v in results.items():
        if (v.asv.hessian or ignore_asv) and v.hessian is not None:
            print("[[",file=stream,end="")
            first = True
            for r in v.hessian:
                if not first:
                    print("\n  ",file=stream,end="")
                first=False
                for c in r:
                    print(" %24.16E" % c,file=stream, end="")
            print(" ]]",file=stream)
    # Write metadata
    for k, v in results.metadata.items():
        print("%24.16E %s" %(v, k), file=stream)
        
        
def _write_json_results(stream: io.IOBase, results: Results, ignore_asv: bool) -> None:
    data_for_writing = _encode_results_as_json(results, ignore_asv)
    json.dump(data_for_writing, stream, indent=4)
    
    
def _encode_results_as_json(results: Results, ignore_asv: bool) -> dict:

    _check_against_asv(results, ignore_asv)
    if results.failed():
        return {"fail": True}
    
    eval_results = {}
    
    # Write function values
    eval_results["functions"] = {}
    for t, v in results.items():
        if (v.asv.function or ignore_asv) and v.function is not None:
            eval_results["functions"][t] = _replace_non_finite(v.function)
    if not eval_results["functions"]:
        del eval_results["functions"]
        
    # Write gradients
    eval_results["gradients"] = {}
    for t, v in results.items():
        if (v.asv.gradient or ignore_asv) and v.gradient is not None:
            eval_results["gradients"][t] = [_replace_non_finite(e) for e in v.gradient]
    if not eval_results["gradients"]:
        del eval_results["gradients"]
            
    # Write Hessians
    eval_results["hessians"] = {}
    for t, v in results.items():
        if (v.asv.hessian or ignore_asv) and v.hessian is not None:
            eval_results["hessians"][t] = [[_replace_non_finite(e) for e in row] for row in v.hessian]
    if not eval_results["hessians"]:
        del eval_results["hessians"]
        
    # Write metadata
    eval_results["metadata"] = {}
    for k, v in results.metadata.items():
        eval_results["metadata"][k] = _replace_non_finite(v)
    if not eval_results["metadata"]:
        del eval_results["metadata"]
    
    return eval_results       
    

def _replace_non_finite(value: Union[float, str]) -> Union[float, str]:
    result = value
    if math.isnan(value):
        result = "NaN"
    elif math.isinf(value):
        result = "Inf" if value > 0 else "-Inf"
    return result              


def _check_against_asv(results: Results, ignore_asv: bool) -> None:
    
    ## Confirm that user has provided all info requested by Dakota
    if not ignore_asv and not results.failed():
        for t, v in results.items():
            if v.asv.function and v.function is None:
                raise ResponseError("Response '" + t + "' is missing "
                        "requested function result.") 
            if v.asv.gradient and v.gradient is None:
                raise ResponseError("Response '" + t + "' is missing "
                        "requested gradient result.")
            if v.asv.hessian and v.hessian is None:
                raise ResponseError("Response '" +t + "' is missing "
                        "requested Hessian result.")

    if not results.failed():
        for k, v in results.metadata.items():
            if v is None:
                raise ResultsError("Results object is missing requested "
                        "metadata field '" + k + "'")


### Free functions and their helpers for constructing objects

# Collections of regexes for parsing aprepro and dprepro formatted Dakota 
# parameter files

_aprepro_re_base = r" {{20}}{{ {tag} += +{value} }}\n"
_dakota_re_base = r"\s*{value} {tag}\n"

_pRE = {
        "APREPRO":{"num_variables":re.compile(_aprepro_re_base.format(
            value=r"(?P<value>\d+)", tag=r"(?P<tag>DAKOTA_VARS)")),
            "num_functions":re.compile(_aprepro_re_base.format(
                value=r"(?P<value>\d+)", tag=r"(?P<tag>DAKOTA_FNS)")),
            "num_deriv_vars":re.compile(_aprepro_re_base.format(
                value=r"(?P<value>\d+)", tag=r"(?P<tag>DAKOTA_DER_VARS)")),
            "num_an_comps":re.compile(_aprepro_re_base.format(
                value=r"(?P<value>\d+)", tag=r"(?P<tag>DAKOTA_AN_COMPS)")),
            "num_metadata":re.compile(_aprepro_re_base.format(
                value=r"(?P<value>\d+)", tag=r"(?P<tag>DAKOTA_METADATA)")),
            "eval_id":re.compile(_aprepro_re_base.format(
                value=r"(?P<value>\d+(?::\d+)*)",
                tag="(?P<tag>DAKOTA_EVAL_ID)")),
            "variable":re.compile(_aprepro_re_base.format(
                value=r"\"?(?P<value>.+?)\"?", tag =r"(?P<tag>\S+)")),
            "function":re.compile(_aprepro_re_base.format(
                value=r"(?P<value>[0-7])", tag=r"ASV_\d+:(?P<tag>\S+)")),
            "deriv_var":re.compile(_aprepro_re_base.format(
                value=r"(?P<value>\d+)", tag=r"DVV_\d+:(?P<tag>\S+)")),
	    "an_comp":re.compile(_aprepro_re_base.format(
		value=r"\"(?P<value>.+?)\"", tag=r"AC_\d+:(?P<tag>.+?)")),
	    "metadata":re.compile(_aprepro_re_base.format(
		value=r"\"(?P<value>.+?)\"", tag=r"(?P<tag>MD_\d+)"))
	    },
	"DAKOTA":{"num_variables":re.compile(_dakota_re_base.format(
            value=r"(?P<value>\d+)", tag=r"(?P<tag>variables)")),
            "num_functions":re.compile(_dakota_re_base.format(
                value=r"(?P<value>\d+)", tag=r"(?P<tag>functions)")),
            "num_deriv_vars":re.compile(_dakota_re_base.format(
                value=r"(?P<value>\d+)", tag=r"(?P<tag>derivative_variables)")),
            "num_an_comps":re.compile(_dakota_re_base.format(
                value=r"(?P<value>\d+)", tag=r"(?P<tag>analysis_components)")),
            "num_metadata":re.compile(_dakota_re_base.format(
                value=r"(?P<value>\d+)", tag=r"(?P<tag>metadata)")),
            "eval_id":re.compile(_dakota_re_base.format(
                value=r"(?P<value>\d+(?::\d+)*)", tag=r"(?P<tag>eval_id)")),
            # A lookahead assertion is required to catch string variables with
            # spaces
            "variable":re.compile(r"\s*(?P<value>.+?)(?= \S+\n) (?P<tag>\S+)\n"),
            "function":re.compile(_dakota_re_base.format(
                value=r"(?P<value>[0-7])", tag=r"ASV_\d+:(?P<tag>\S+)")),
            "deriv_var":re.compile(_dakota_re_base.format(
                value=r"(?P<value>\d+)", tag=r"DVV_\d+:(?P<tag>\S+)")),
            "an_comp":re.compile(_dakota_re_base.format(
                value=r"(?P<value>.+?)", tag=r"AC_\d+:(?P<tag>.+?)")),
            "metadata":re.compile(_dakota_re_base.format(
                value=r"(?P<value>.+?)", tag=r"(?P<tag>MD_\d+)"))
            }
        }


def _extract_block(stream, numRE, dataRE, handle):
    """Extract a block of information from a Dakota parameters file.

    stream is expected to be queued up to the beginning of the block.
    
    numRE and dataRE are regexes used to extract the number of items
    and the items themselves, respectively. They must support groups
    named 'tag' and 'value'.

    handle is a function to store the extracted items."""

    line = stream.readline()
    m = numRE.match(line)
    if m is None:
        raise ParamsFormatError("Improper format for section header in " + \
                "parameters file")
    num = int(numRE.match(line).group("value"))
    for i in range(num):
        line = stream.readline()
        m = dataRE.match(line)
        if m is None:
            raise ParamsFormatError("Improperly formatted item in " + \
                    "parameters file")
        tag = m.group("tag")
        value = m.group("value")
        handle(tag,value)


def _extract_optional_block(stream, numRE, dataRE, handle):
    """Optionally extract a block of information from a Dakota parameters file.

    Same API as _extract_block, except will not raise exception on
    missing block. Will still raise on poorly formatted item within
    the block."""

    beginning_pos = stream.tell()
    line = stream.readline()
    m = numRE.match(line)
    stream.seek(beginning_pos)
    # block appears to exist, try to read in earnest
    if m is not None:
        _extract_block(stream, numRE, dataRE, handle)


def _read_eval_from_stream(stream=None, ignore_asv=False, results_file=None, infer_types=True, types=None):
    """Extract the parameters data from the stream."""

    # determine format (dakota or aprepro) by examining the first line
    beginning_pos = stream.tell() # position won't be 0 in the batch case
    line = stream.readline()
    if not line or line == '\n':
        return None, None
    dakota_format_match = _pRE["DAKOTA"]["num_variables"].match(line)
    aprepro_format_match = _pRE["APREPRO"]["num_variables"].match(line)
    if aprepro_format_match is not None:
        format = APREPRO
        useRE = _pRE["APREPRO"]
    elif dakota_format_match is not None: 
        format = STANDARD
        useRE = _pRE["DAKOTA"]
    else:
        raise ParamsFormatError("Unrecognized parameters file format.")
    
    # Rewind the stream to begin reading blocks
    stream.seek(beginning_pos)
    # Read variable values
    variables = collections.OrderedDict()
    def store_variables(t, v):
        variables[t]=v
    _extract_block(stream, useRE["num_variables"], useRE["variable"], 
            store_variables)
    # Read functions block
    responses = collections.OrderedDict()
    def store_responses(t, v):
        responses[t]=v
    _extract_block(stream, useRE["num_functions"], useRE["function"], 
            store_responses)
    # Read derivative variables block
    deriv_vars = []
    def store_deriv_vars(t,v):
        deriv_vars.append(t)
    _extract_block(stream, useRE["num_deriv_vars"], useRE["deriv_var"], 
            store_deriv_vars)

    # Read analysis components
    an_comps = []
    def store_an_comps(t, v):
        an_comps.append(v)
    _extract_block(stream, useRE["num_an_comps"], useRE["an_comp"], 
            store_an_comps)

    # Read eval_id
    m = useRE["eval_id"].match(stream.readline())
    eval_id = m.group("value")

    # Read metadata requests
    metadata = []
    def store_metadata(t, v):
        metadata.append(v)
    _extract_optional_block(stream, useRE["num_metadata"], useRE["metadata"],
                            store_metadata)
    return (Parameters(format, variables, an_comps, eval_id, metadata, infer_types, types),
            Results(format, responses, deriv_vars, eval_id, metadata, ignore_asv, results_file))
    

def _read_parameters_stream(stream=None, ignore_asv=False, batch=False, 
    results_file=None, infer_types=True, types=None):
    """Extract all evaluations from stream"""
    param_sets = [] # list of Parameters objects
    results_sets = [] # list of Results objects
    
    param_sets, results_sets = _read_evals_from_stream(stream, ignore_asv, results_file, infer_types, types)
       
    if batch:
        return BatchParameters(param_sets), BatchResults(results_sets)
    elif len(param_sets) > 1 and not batch:
        raise BatchSettingError("batch is False, but parameters for " +
                     "multiple evaluations found in parameters file.") 
    else:
        return param_sets[0], results_sets[0]


def _read_evals_from_stream(stream: io.IOBase, ignore_asv: bool, results_file: str, infer_types: bool, types) -> Tuple[List[Parameters], List[Results]]:
    try:
        return _read_evals_from_json(stream, ignore_asv, results_file, infer_types, types)
    except json.decoder.JSONDecodeError:
        stream.seek(0)
    
    param_sets = []
    results_sets = []
    while True:
        p, r = _read_eval_from_stream(stream, ignore_asv, results_file, infer_types, types)
        if p and r: # p and r == None indicates end of stream
            param_sets.append(p)
            results_sets.append(r)
        else:
            break
    return param_sets, results_sets


def _read_evals_from_json(stream, ignore_asv, results_file, infer_types, types):
    j = json.load(stream)
    if not isinstance(j, list):
        j = [j]
    params_set = []
    results_set = []
    for params in j:
        try:
            var_labels = [v["label"] for v in params["variables"]]
            values = [v["value"] for v in params["variables"]]
            var_dict = collections.OrderedDict()
            for label, value in zip(var_labels, values):
                var_dict[label] = value
        except KeyError:
            raise ParamsFormatError("'variables' portion of JSON-format evaluation missing or not formatted as expected")
        try:            
            response_labels = [r["label"] for r in params["responses"]]
            active_set_vector = [r["active_set"] for r in params["responses"]]
            responses_dict = collections.OrderedDict()
            for label, active_set in zip(response_labels, active_set_vector):
                responses_dict[label] = active_set
        except KeyError:
            raise ParamsFormatError("'responses' portion of JSON-format evaluation missing or not formatted as expected")
        try:
            deriv_vars = [var_labels[i-1] for i in params["derivative_variables"]]
        except (KeyError, IndexError):
            raise ParamsFormatError("'derivative_variables' portion of JSON-format evaluation missing or not formatted as expected")
        try:
            an_comps = [a["component"] for a in params["analysis_components"]]
        except KeyError:
            raise ParamsFormatError("'analysis_components' portion of JSON-format evaluation missing or not formatted as expected")
        try:
            eval_id = params["eval_id"]
        except KeyError:
            raise ParamsFormatError("'eval_id' portion of JSON-format evaluation missing")
        try:
            metadata = params["metadata"]
        except KeyError:
            raise ParamsFormatError("'metadata' portion of JSON-format evaluation missing")
        params_set.append(Parameters(JSON, var_dict, an_comps, eval_id, metadata, infer_types, types))
        results_set.append(Results(JSON, responses_dict, deriv_vars, eval_id, metadata, ignore_asv, results_file))
    return params_set, results_set


def _extract_value(label: str, labels: List[str], values: List[VarValue]) -> VarValue:
    try:
        index = labels.index(label)
        value = values[index]
    except ValueError:
        value = None
    return value
    


def _read_params_from_dict(parameters=None):
    """Process parameters and results using Dakota parameters disctionary from python interface driver.
    
    Keyword Args:

    Returns:
            
    Raises:
    """
    batch = isinstance(parameters, list)
    if not batch:
        parameters = [parameters]
    params_list = []
    results_list = []
    for p in parameters:
        variables = collections.OrderedDict()
        for label in p["variable_labels"]:
            for var_type in ("cv", "div", "dsv", "drv"):
                var_val = _extract_value(label, p[f"{var_type}_labels"], p[var_type])
                if var_val:
                    break
            variables[label] = var_val
        
        responses = collections.OrderedDict()
        for label, active_set in zip(p["function_labels"], p["asv"]):
            responses[label] = active_set
        
        dvv = []
        for id in p["dvv"]:
            dvv.append(p["variable_labels"][id-1])
        
        params_list.append(Parameters(DIRECT, variables, p["analysis_components"], p["eval_id"], p["metadata_labels"], False))
        results_list.append(Results(DIRECT, responses, dvv, p["eval_id"], p["metadata_labels"], False))
    if batch:
        params = BatchParameters(params_list)
        results = BatchResults(results_list)
    else:
        params = params_list[0]
        results = results_list[0]
    return params, results
    


def read_parameters_file(parameters_file: Union[str, None] = None, results_file : Union[str, None] = None, 
        ignore_asv: bool=False, batch: bool=False, infer_types: bool=True, types: Union[list, dict]=None):
    """Read and parse the Dakota parameters file.
    
    Keyword Args:
        parameters_file: Pathname to the Dakota parameters file. If not
            provided, the first command line argument will be used.
        results_file: Pathname to the Dakota results file. If not provided
            or set to None, the second command line argument will be used.
            Setting to dakota.interfacing.UNNAMED leaves the file unnamed, 
            and a stream must be specified in the call to Results.write().
        ignore_asv: If True, ignore the active set vector when setting
            responses on the returned Results object.
        batch: Set to True when performing batch evaluations.
        infer_types: If True, variable types will be guessed. Otherwise, they
            will remain strings. Settings provided using the `types` argument
            override type inference.
        types: A dict or list of type overrides. If a list, the length
            must equal the number of variables. If a dict, variables will be
            matched by descriptor. Extra keys will be ignored.

    Returns:
        Two cases:
        1) For a batch evaluation, return a tuple containing a BatchParameters
        object and BatchResults object configured based on the parameters file.
        2) For a single evaluation, return a tuple containing a Parameters
        object and Results object configured based on the parameters file.
            
    Raises:
        dakota.interfacing.MissingSourceError: Parameters or results filename is
            not provided and cannot be read from the command line arguments.

        dakota.interfacing.ParamsFormatError: The Dakota parameters file was not
            valid. 
    """
    ### Determine the name of the parameters file and read it in
    if parameters_file is None:
        try:
            parameters_file = sys.argv[1]
        except IndexError:
            raise MissingSourceError("No parameters filename provided and no "
                    "command line argument.")

    ### Determine the name of the results file
    if results_file is None:
        try:
            results_file = sys.argv[2]
        except IndexError:
            raise MissingSourceError("No results filename provided and no "
                    "command line argument.")
    elif results_file == UNNAMED:
        results_file = ""

    ### Open and parse the parameters file
    with open(parameters_file, "r", encoding='utf8') as ifp:
        return _read_parameters_stream(ifp, ignore_asv, batch, results_file, infer_types, types)


def dprepro(template: Union[str, io.IOBase], parameters: Parameters=None, results: Results=None, include: dict=None,
        output: Union[str, io.IOBase]=None, fmt: str='%0.10g', code: str='%', code_block: str='{% %}', inline: str='{ }',
        warn: bool=True):
    """Call dprepro to process a template
    
    The documentation for the command-line version of dprepro explains
    template formatting and how the Dakota parameters are made available
    within the template. This function optionally makes a Dakota Results object
    available under the name DakotaResults.

    Arguments:
        template: If template is a string, it is treated as the name of a file
            that contains a template. If it's a file-like object with a .read()
            method, the template will be read from it.

    Keyword Args:
        parameters: Parameters object. The object itself is available for use
            within the template using the name DakotaParams. The variables will
            also be extracted and made directly available.
        results: Results object. The object itself is available for use
            within the template using the name DakotaResults.
        include: Dict of other items to make available for substitution. Items
            in include will overwrite items from the Parameters object.
        output: If output is a string, it is assumed to be the name of a file to 
            write the processed template to. If it's a file-like object, the
            processed template will be written to it. If None (the default), the
            processed template will be returned as a string.
        fmt: Default format for numerical fields. Default: '%0.10g'
        code: Delimiter for a code line. Default: '%'
        code_block: Delimiters for a code block. Default: '{% %}'
        inline: Delimiters for inline substitution. Default: '{ }'
        warn: Whether to print warnings

    Returns:
        If no output is specified, returns a string containing the substituted
            template.
    """

    env = {}
    if include is None:
        include = {}
    # Construct the env from parameters, results, and include
    if isinstance(parameters,Parameters):
        env["DakotaParams"] = parameters
        for d, v in parameters.items():
            env[d] = v
    if isinstance(results,Results):
        env["DakotaResults"] = results
    env.update(include)
    
    # Call the template engine
    output_string = dprepro_mod.dprepro(include=env, template=template, fmt=fmt,
            code=code, code_block=code_block, inline=inline, warn=warn)

    # Output
    if output is None:
        return output_string
    elif hasattr(output, "write"):
        output.write(output_string)
    else:
        with open(output,"w",encoding="utf-8") as f:
            f.write(output_string)



def python_interface(fn):
    """ Decorator to wrap direct Python callbacks """
    @functools.wraps(fn)
    def wrapper(direct_interface_dict):
        params, results = _read_params_from_dict(direct_interface_dict)
        results = fn(params, results)
        return results.direct_results_dict()
    return wrapper


