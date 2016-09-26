"""Read Dakota parameter files and write Dakota response files"""
from __future__ import print_function
import collections
import re
import sys
import copy


#### Exceptions

class ResponseError(Exception):
    """Response data is improperly specified.
    
    Example issues:
        - Gradients or Hessians are wrongly sized.
        - Requested information for a response is missing at write time.
        - Unrequested information has been provided.
    """
    pass

class MissingSourceError(Exception):
    """A filename or stream was not provided and cannot be inferred."""
    pass

class ParamsFormatError(Exception):
    """Parameters file does not have a recognized format."""
    pass


#### Class definitions

class Parameters(object):
    """Conveniently access variables from a Dakota parameters file
    
    Parameters objects should be constructed by the function read_parameters_file.
    """

    def __init__(self,aprepro_format=None, variables=None, an_comps=None, eval_id=None):
        self.aprepro_format = aprepro_format
        self._variables = copy.deepcopy(variables)
        self.an_comps = list(an_comps)
        self.eval_id = eval_id

    @property
    def descriptors(self):
        """Return a list of variable descriptors."""
        return self._variables.keys()

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
        """Iterate over index, variable name, and variable."""
        for index, (name, response) in enumerate(self._variables.iteritems()):
            yield index, name, response


_asvType = collections.namedtuple("ASVType",["function","gradient","hessian"])


# A class to hold the ASV and data for a single response
class Response(object):
    def __init__(self, descriptor, num_deriv_vars, ignore_asv, asv):
        self._descriptor = descriptor
        self._num_deriv_vars = num_deriv_vars
        int_asv = int(asv)
        self.asv = _asvType(int_asv & 1 == 1, int_asv & 2 == 2, int_asv &4 == 4)
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
            raise ResponseError("Function value not requested for '%s'." % self._descriptor)
        self._function = float(val)

    @property
    def gradient(self):
        # try/except needed bc Results._write_results with ignore_csv = True 
        # tests for existence of this member        
        try: 
            return list(self._gradient) # return a copy
        except:
            return None

    @gradient.setter
    def gradient(self,val):
        if not (self._ignore_asv or self.asv.gradient):
            raise ResponseError("Gradient not requested for '%s'." % self._descriptor)
        self._gradient = [float(e) for e in val]
        if len(self._gradient) != self._num_deriv_vars:
            raise ResponseError("Length of gradient must equal number of derivative variables.")

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
            raise ResponseError("Hessian not requested for '%s'." % self._descriptor)
        ### validate dimensions
        rctr = 0
        for r in val:
            rctr += 1
            cctr = 0
            for c in r:
                cctr += 1
            if cctr != self._num_deriv_vars:
                raise ResponseError("Hessian must be square and size num_deriv_variables.")
        if rctr != self._num_deriv_vars:
            raise ResponseError("Hessian must be square and size num_deriv_variables.")
        ### copy upper triangular of val into self._hessian
        coffset = 0
        self._hessian = []
        for r in val:
            cctr = 0
            row = []
            for c in r:
                if cctr < coffset:
                    row.append(0.0)
                    continue
                row.append(float(c))
            self._hessian.append(row)
        ### symmetrize self._hessian
        for i in xrange(self._num_deriv_vars):
            for j in xrange(i+1, self._num_deriv_vars):
                self._hessian[j][i] = self._hessian[i][j]

class Results(object):
    """ASV, response data container, and results file writer."""
    def __init__(self, aprepro_format=None, responses=None, deriv_vars=None, eval_id=None, ignore_asv=False, results_file=None):
        self.aprepro_format = aprepro_format
        self.ignore_asv = ignore_asv
        self._deriv_vars = deriv_vars[:]
        num_deriv_vars = len(deriv_vars)
        self._responses = collections.OrderedDict()
        for t, v in responses.iteritems():
            self._responses[t] = Response(t, num_deriv_vars, ignore_asv, int(v)) 
        self.results_file = results_file
        self.eval_id = eval_id

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
        # Write function values
        for t, v in self._responses.iteritems():
            if (v.asv.function or ignore_asv) and v.function is not None:
                print("%24.16E %s" %(v.function, t), file=stream)
        # Write gradients
        for t, v in self._responses.iteritems():
            if (v.asv.gradient or ignore_asv) and v.gradient is not None:
                print("[ ",file=stream, end="")
                for e in v.gradient:
                    print("% 24.16E" % e,file=stream,end="")
                print(" ]",file=stream)
        # Write Hessians
        for t, v in self._responses.iteritems():
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

        Use the output stream if provided, otherwise open a file with the name
        provided to the constructor. If no name was provided, raise MissingSourceError.

        ignore_asv overrides the setting provided at construct time. If it is True,
        then ASV checking is disabled and all available response data is written.
        
        Raise a ResponseError if a result requested by Dakota is missing and ignore_asv
        is False.
        """
        my_ignore_asv = self.ignore_asv
        if ignore_asv is not None:
            my_ignore_asv= ignore_asv

        ## Confirm that user has provided all info requested by Dakota
        if not my_ignore_asv:
            for t, v in self._responses.iteritems():
                if v.asv.function and v.function is None:
                    raise ResponseError("Response '" + t + "' is missing requested function result.") 
                if v.asv.gradient and v.gradient is None:
                    raise ResponseError("Response '" + t + "' is missing requested gradient result.")
                if v.asv.hessian and v.hessian is None:
                    raise ResponseError("Response '" +t + "' is missing requested Hessian result.")

        if stream is None:
            if self.results_file is None:
                raise MissingSourceError("No stream specified and no results_file provided at construct time.")
            else:
                with open(self.results_file, "w") as ofp:
                    self._write_results(ofp, my_ignore_asv)
        else:
            self._write_results(stream, my_ignore_asv)

    @property
    def descriptors(self):
        """Return a list of the response descriptors."""
        return self._responses.keys()

    @property
    def deriv_vars(self):
        """Return a list of the derivative variables."""
        return list(self._deriv_vars)

    def iteritems(self):
        for i, (k, v) in enumerate(self._responses.iteritems()):
            yield i, k, v

    def __iter__(self):
        """Iterate over index, response name, and response."""
        for index, (name, response) in enumerate(self._responses.iteritems()):
            yield index, name, response



### Free functions and their helpers for constructing objects

# Collections of regexes for parsing aprepro and dprepro formatted Dakota parameter files

_aprepro_re_base = " {{20}}{{ {tag} += +{value} }}\n"
_dakota_re_base = "\s*{value} {tag}\n"

_pRE = {
        "APREPRO":{"num_variables":re.compile(_aprepro_re_base.format(value="(?P<value>\d+)",
            tag="(?P<tag>DAKOTA_VARS)")),
            "num_functions":re.compile(_aprepro_re_base.format(value="(?P<value>\d+)",
                tag="(?P<tag>DAKOTA_FNS)")),
            "num_deriv_vars":re.compile(_aprepro_re_base.format(value="(?P<value>\d+)",
                tag="(?P<tag>DAKOTA_DER_VARS)")),
            "num_an_comps":re.compile(_aprepro_re_base.format(value="(?P<value>\d+)",
                tag="(?P<tag>DAKOTA_AN_COMPS)")),
            "eval_id":re.compile(_aprepro_re_base.format(value="(?P<value>\d+)",
                tag="(?P<tag>DAKOTA_EVAL_ID)")),
            "variable":re.compile(_aprepro_re_base.format(value="\"?(?P<value>.+?)\"?",
                tag ="(?P<tag>\S+)")),
            "function":re.compile(_aprepro_re_base.format(value="(?P<value>[1-7])",
                tag="ASV_\d+:(?P<tag>\S+)")),
            "deriv_var":re.compile(_aprepro_re_base.format(value="(?P<value>\d+)",
                tag="DVV_\d+:(?P<tag>\S+)")),
            "an_comp":re.compile(_aprepro_re_base.format(value="\"(?P<value>.+?)\"",
                tag="AC_\d+:(?P<tag>.+?)"))
            },
        "DAKOTA":{"num_variables":re.compile(_dakota_re_base.format(value="(?P<value>\d+)",
            tag="(?P<tag>variables)")),
            "num_functions":re.compile(_dakota_re_base.format(value="(?P<value>\d+)",
                tag="(?P<tag>functions)")),
            "num_deriv_vars":re.compile(_dakota_re_base.format(value="(?P<value>\d+)",
                tag="(?P<tag>derivative_variables)")),
            "num_an_comps":re.compile(_dakota_re_base.format(value="(?P<value>\d+)",
                tag="(?P<tag>analysis_components)")),
            "eval_id":re.compile(_dakota_re_base.format(value="(?P<value>\d+)",
                tag="(?P<tag>eval_id)")),
            # A lookahead assertion is required to catch string variables with spaces
            "variable":re.compile("\s*(?P<value>.+?)(?= \S+\n) (?P<tag>\S+)\n"),
            "function":re.compile(_dakota_re_base.format(value="(?P<value>[1-7])",
                tag="ASV_\d+:(?P<tag>\S+)")),
            "deriv_var":re.compile(_dakota_re_base.format(value="(?P<value>\d+)",
                tag="DVV_\d+:(?P<tag>\S+)")),
            "an_comp":re.compile(_dakota_re_base.format(value="(?P<value>.+?)",
                tag="AC_\d+:(?P<tag>.+?)"))
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
    for i in xrange(num):
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
    _extract_block(stream, useRE["num_variables"], useRE["variable"], store_variables)
    # Read functions block
    responses = collections.OrderedDict()
    def store_responses(t, v):
        responses[t]=v
    _extract_block(stream, useRE["num_functions"], useRE["function"], store_responses)
    # Read derivative variables block
    deriv_vars = []
    def store_deriv_vars(t,v):
        deriv_vars.append(t)
    _extract_block(stream, useRE["num_deriv_vars"], useRE["deriv_var"], store_deriv_vars)

    # Read analysis components
    an_comps = []
    def store_an_comps(t, v):
        an_comps.append(v)
    _extract_block(stream, useRE["num_an_comps"], useRE["an_comp"], store_an_comps)

    # Read eval_id
    m = useRE["eval_id"].match(stream.readline())
    eval_id = m.group("value")
    
    return (Parameters(aprepro_format, variables, an_comps, eval_id),
            Results(aprepro_format, responses, deriv_vars, eval_id, ignore_asv, results_file))


def read_parameters_file(parameters_file=None, results_file=None, ignore_asv=False):
    """Read Dakota parameters file and return Parameters object and Results object.
    
    The parameters_file and results_file keywords contain the names of the 
    Dakota parameters and results files. If they are not provided, the 1st and 
    2nd command line arguments will be used.

    MissingSourceError is raised if no filenames are provided or can be inferred,
    and also when the parameters file cannot be opened.
    """
    ### Determine the name of the parameters file and read it in
    if parameters_file is None:
        try:
            parameters_file = sys.argv[1]
        except IndexError:
            raise MissingSourceError("No parameters filename provided and no command line argument.")
    with open(parameters_file,"r") as ifp:
        parameters_list = ifp.readlines()

    ### Determine the name of the results file
    if results_file is None:
        try:
            results_file = sys.argv[2]
        except IndexError:
            raise MissingSourceError("No results filename provided and no command line argument.")

    ### Open and parse the parameters file
    with open(parameters_file, "r") as ifp:
        return _read_parameters_stream(ifp, ignore_asv, results_file)


