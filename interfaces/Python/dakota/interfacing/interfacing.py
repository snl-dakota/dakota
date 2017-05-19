from __future__ import print_function
import collections
import re
import sys
import copy

__author__ = 'J. Adam Stephens'
__copyright__ = 'Copyright 2014 Sandia Corporation'
__license__ = 'GNU Lesser General Public License'

#### Exceptions


class ResponseError(Exception):
    pass


class MissingSourceError(Exception):
    pass


class ParamsFormatError(Exception):
    pass

# Constant used to specify an unnamed results file
UNNAMED = True

#### Class definitions

class Parameters(object):
    """Access variables and analysis components from a Dakota parameters file
    
    Parameters objects typically should be constructed by the convenience 
    function ``dakota.interfacing.read_parameters_file``.

    Variable values can be accessed by name or by index using []. Analysis
    components are accessible by index only using the an_comp attribute. The
    Parameters class supports iteration, yielding the index, variable
    descriptor, and variable value.

    Attributes:
        an_comps: List of strings containing the analysis components.
        eval_id: Evaluation id (string).
        eval_num: Evaluation number (final token in eval_id) (int).
        aprepro_format: Boolean indicating whether the parameters file was in
            aprepro (True) or Dakota (False) format.
        descriptors: List of the variable descriptors (read-only)
        num_variables: Number of variables (read-only)
        num_an_comps: Number of analysis components (read-only)
    """

    def __init__(self,aprepro_format=None, variables=None, an_comps=None, 
            eval_id=None):
        self.aprepro_format = aprepro_format
        self._variables = copy.deepcopy(variables)
        self.an_comps = list(an_comps)
        self.eval_id = str(eval_id)
        self.eval_num = int(eval_id.split(":")[-1])
        # Convert variables to the appropriate type. The possible types
        # are int, float, and string. The variables are already strings.
        # TODO: Consider a user option to override this behavior and keep
        # everything as a string, or provide access to the original strings
        for k, v in self._variables.items():
            try:
                self._variables[k] = int(v)
            except ValueError:
                try:
                    self._variables[k] = float(v)
                except ValueError:
                    pass
                pass

    @property
    def descriptors(self):
        return [k for k in self._variables.keys()]

    def __getitem__(self,key):
        if type(key) is int:
            return self._variables[self.descriptors[key]]
        else:
            return self._variables[key]

    def __setitem__(self,key,value):
        if type(key) is int:
            self._variables[self.descriptors[key]] = value
        else:
            self._variables[key] = value

    @property
    def num_variables(self):
        return len(self._variables)

    @property
    def num_an_comps(self):
        return len(self.an_comps)

    def __iter__(self):
        for index, (name, response) in enumerate(self._variables.items()):
            yield index, name, response


# Datatype to hold ASV element for a single response. function, gradient,
# and hession are set to True or False. 
_asvType = collections.namedtuple("ASVType",["function","gradient","hessian"])


# A class to hold the ASV and data for a single response
class Response(object):
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
    def __init__(self, descriptor, num_deriv_vars, ignore_asv, asv):
        self._descriptor = descriptor
        self._num_deriv_vars = num_deriv_vars
        int_asv = int(asv)
        self.asv = _asvType(int_asv & 1 == 1, int_asv & 2 == 2, 
                int_asv &4 == 4)
        self._function = None
        self._gradient = None
        self._hessian = None
        self._ignore_asv = ignore_asv

    @property
    def function(self):
        return self._function

    @function.setter
    def function(self,val):
        if not (self._ignore_asv or self.asv.function):
            raise ResponseError("Function value not requested for '%s'." 
                    % self._descriptor)
        self._function = float(val)

    @property
    def gradient(self):
        # try/except needed bc Results._write_results with ignore_csv = True 
        # tests for existence of this member        
        try: 
            return list(self._gradient) # return a copy
        except TypeError:
            return None

    @gradient.setter
    def gradient(self,val):
        if not (self._ignore_asv or self.asv.gradient):
            raise ResponseError("Gradient not requested for '%s'." 
                    % self._descriptor)
        self._gradient = [float(e) for e in val]
        if len(self._gradient) != self._num_deriv_vars:
            raise ResponseError("Length of gradient must equal number of "
                    "derivative variables.")

    @property
    def hessian(self):
        # try/except needed bc Results._write_results with ignore_csv = True 
        # tests for existence of this member        
        try:
            return copy.deepcopy(self._hessian)
        except:
            return None

    @hessian.setter
    def hessian(self,val):
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
    

class Results(object):
    """Collect response data and write to results file.

    Results objects typically should be constructed by the convenience function
    ``dakota.interfacing.read_parameters_file``.

    Each response is represented by a Response objected, and can be accessed 
    by name or by index using []. The Results class supports iteration, yielding
    the index, response descriptor, and Response object.

    Attributes:
        eval_id: Evaluation id (a string).
        eval_num: Evaluation number (final token in eval_id) (int).
        aprepro_format: Boolean indicating whether the parameters file was in
            aprepro (True) or Dakota (False) format.
        descriptors: List of the response descriptors (read-only)
        num_responses: Number of variables (read-only)
        deriv_vars: List of the derivative variables (read-only)
        num_deriv_vars: Number of derivative variables (read-only)
    """

    def __init__(self, aprepro_format=None, responses=None, 
            deriv_vars=None, eval_id=None, ignore_asv=False, 
            results_file=None):
        self.aprepro_format = aprepro_format
        self.ignore_asv = ignore_asv
        self._deriv_vars = deriv_vars[:]
        num_deriv_vars = len(deriv_vars)
        self._responses = collections.OrderedDict()
        for t, v in responses.items():
            self._responses[t] = Response(t, num_deriv_vars, ignore_asv, 
                    int(v)) 
        self.results_file = results_file
        self.eval_id = eval_id
        self.eval_num = int(eval_id.split(":")[-1])
        self._failed = False

    def __getitem__(self,key):
        if type(key) is int:
            return self._responses[self.descriptors[key]]
        else:
            return self._responses[key]

    @property
    def num_deriv_vars(self):
        return len(self._deriv_vars)

    @property
    def num_responses(self):
        return len(self._responses)

    def _write_results(self,stream, ignore_asv):
        # Write FAIL if set
        if self._failed:
            print("FAIL", file=stream)
            return
        # Write function values
        for t, v in self._responses.items():
            if (v.asv.function or ignore_asv) and v.function is not None:
                print("%24.16E %s" %(v.function, t), file=stream)
        # Write gradients
        for t, v in self._responses.items():
            if (v.asv.gradient or ignore_asv) and v.gradient is not None:
                print("[ ",file=stream, end="")
                for e in v.gradient:
                    print("% 24.16E" % e,file=stream,end="")
                print(" ]",file=stream)
        # Write Hessians
        for t, v in self._responses.items():
            if (v.asv.hessian or ignore_asv) and v.hessian is not None:
                print("[[",file=stream,end="")
                first = True
                for r in v.hessian:
                    if not first:
                        print("\n  ",file=stream,end="")
                    first=False
                    for c in r:
                        print("% 24.16E" % c,file=stream, end="")
                print(" ]]",file=stream)

    def write(self, stream=None, ignore_asv=None):
        """Write the results to the Dakota results file.

        Keyword Args:
            stream: Write results to this I/O stream. Overrides results_file
                specified when the object was constructed.
            ignore_asv: Ignore the active set vector while writing the response
                data to the results file (or stream). Overrides ignore_asv
                setting provided at construct time.

        Raises:
            dakota.interfacing.MissingSourceError: No results_file was provided at 
                construct time, and no stream was provided to the method call.
            dakota.interfacing.ResponseError: A result requested by Dakota is missing 
                (and ignore_asv is False).
        """
        my_ignore_asv = self.ignore_asv
        if ignore_asv is not None:
            my_ignore_asv= ignore_asv

        ## Confirm that user has provided all info requested by Dakota
        if not my_ignore_asv and not self._failed:
            for t, v in self._responses.items():
                if v.asv.function and v.function is None:
                    raise ResponseError("Response '" + t + "' is missing "
                            "requested function result.") 
                if v.asv.gradient and v.gradient is None:
                    raise ResponseError("Response '" + t + "' is missing "
                            "requested gradient result.")
                if v.asv.hessian and v.hessian is None:
                    raise ResponseError("Response '" +t + "' is missing "
                            "requested Hessian result.")

        if stream is None:
            if self.results_file is None:
                raise MissingSourceError("No stream specified and no "
                        "results_file provided at construct time.")
            else:
                with open(self.results_file, "w") as ofp:
                    self._write_results(ofp, my_ignore_asv)
        else:
            self._write_results(stream, my_ignore_asv)

    @property
    def descriptors(self):
        return [k for k in self._responses.keys()]

    @property
    def deriv_vars(self):
        return list(self._deriv_vars)

    def __iter__(self):
        for index, (name, response) in enumerate(self._responses.items()):
            yield index, name, response

    def fail(self):
        """Set the FAIL attribute. 
        
        When the results file is written, it will contain only the word FAIL"""
        self._failed = True

### Free functions and their helpers for constructing objects

# Collections of regexes for parsing aprepro and dprepro formatted Dakota 
# parameter files

_aprepro_re_base = " {{20}}{{ {tag} += +{value} }}\n"
_dakota_re_base = "\s*{value} {tag}\n"

_pRE = {
        "APREPRO":{"num_variables":re.compile(_aprepro_re_base.format(
            value="(?P<value>\d+)", tag="(?P<tag>DAKOTA_VARS)")),
            "num_functions":re.compile(_aprepro_re_base.format(
                value="(?P<value>\d+)", tag="(?P<tag>DAKOTA_FNS)")),
            "num_deriv_vars":re.compile(_aprepro_re_base.format(
                value="(?P<value>\d+)", tag="(?P<tag>DAKOTA_DER_VARS)")),
            "num_an_comps":re.compile(_aprepro_re_base.format(
                value="(?P<value>\d+)", tag="(?P<tag>DAKOTA_AN_COMPS)")),
            "eval_id":re.compile(_aprepro_re_base.format(
                value="(?P<value>\d+(?::\d+)*)", tag="(?P<tag>DAKOTA_EVAL_ID)")),
            "variable":re.compile(_aprepro_re_base.format(
                value="\"?(?P<value>.+?)\"?", tag ="(?P<tag>\S+)")),
            "function":re.compile(_aprepro_re_base.format(
                value="(?P<value>[0-7])", tag="ASV_\d+:(?P<tag>\S+)")),
            "deriv_var":re.compile(_aprepro_re_base.format(
                value="(?P<value>\d+)", tag="DVV_\d+:(?P<tag>\S+)")),
            "an_comp":re.compile(_aprepro_re_base.format(
                value="\"(?P<value>.+?)\"", tag="AC_\d+:(?P<tag>.+?)"))
            },
        "DAKOTA":{"num_variables":re.compile(_dakota_re_base.format(
            value="(?P<value>\d+)", tag="(?P<tag>variables)")),
            "num_functions":re.compile(_dakota_re_base.format(
                value="(?P<value>\d+)", tag="(?P<tag>functions)")),
            "num_deriv_vars":re.compile(_dakota_re_base.format(
                value="(?P<value>\d+)", tag="(?P<tag>derivative_variables)")),
            "num_an_comps":re.compile(_dakota_re_base.format(
                value="(?P<value>\d+)", tag="(?P<tag>analysis_components)")),
            "eval_id":re.compile(_dakota_re_base.format(
                value="(?P<value>\d+(?::\d+)*)", tag="(?P<tag>eval_id)")),
            # A lookahead assertion is required to catch string variables with spaces
            "variable":re.compile("\s*(?P<value>.+?)(?= \S+\n) (?P<tag>\S+)\n"),
            "function":re.compile(_dakota_re_base.format(
                value="(?P<value>[0-7])", tag="ASV_\d+:(?P<tag>\S+)")),
            "deriv_var":re.compile(_dakota_re_base.format(
                value="(?P<value>\d+)", tag="DVV_\d+:(?P<tag>\S+)")),
            "an_comp":re.compile(_dakota_re_base.format(
                value="(?P<value>.+?)", tag="AC_\d+:(?P<tag>.+?)"))
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
    num = int(numRE.match(line).group("value"))
    for i in range(num):
        line = stream.readline()
        m = dataRE.match(line)
        tag = m.group("tag")
        value = m.group("value")
        handle(tag,value)


def _read_parameters_stream(stream=None, ignore_asv=False, results_file=None):
    """Extract the parameters data from the stream."""

    # determine format (dakota or aprepro) by examining the first line
    line = stream.readline()
    dakota_format_match = _pRE["DAKOTA"]["num_variables"].match(line)
    aprepro_format_match = _pRE["APREPRO"]["num_variables"].match(line)
    if aprepro_format_match is not None:
        aprepro_format = True
        useRE = _pRE["APREPRO"]
    elif dakota_format_match is not None: 
        aprepro_format = False
        useRE = _pRE["DAKOTA"]
    else:
        raise ParamsFormatError("Unrecognized parameters file format.")
    
    # Rewind the stream to begin reading blocks
    stream.seek(0)
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
    
    return (Parameters(aprepro_format, variables, an_comps, eval_id),
            Results(aprepro_format, responses, deriv_vars, eval_id, ignore_asv,
                results_file))


def read_parameters_file(parameters_file=None, results_file=None, 
        ignore_asv=False):
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

    Returns:
        A tuple containing a Parameters object and Results object configured 
        based on the parameters file.
            
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
    with open(parameters_file,"r") as ifp:
        parameters_list = ifp.readlines()

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
    with open(parameters_file, "r") as ifp:
        return _read_parameters_stream(ifp, ignore_asv, results_file)


