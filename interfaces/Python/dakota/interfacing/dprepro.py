#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import division, print_function, unicode_literals, absolute_import
import os
import re
import sys
import copy
from io import open
import argparse
import functools
import random
import pprint
from collections import defaultdict
import json
import math
import keyword

sys.dont_write_bytecode = True

py3 = False
pyv = sys.version_info
if pyv >= (3,):
    py3 = True
    xrange = range
    unicode = str
    
__version__ = '20180426'

__all__ = ['pyprepro','Immutable','Mutable','ImmutableValDict','dprepro','convert_dakota']

DESCRIPTION="""\
%(prog)s -- python input deck pre-proscessor and template engine.

version: __version__

""".replace('__version__',__version__)

EPILOG = """\
Fallback Flags
-----------------
Will also accept `--(left/right)-delimiter` as an alias to the 
respective parts of `--inline`. 

Sources:
--------
Built from BottlePy's SimpleTemplateEngine[1] with changes to better match
the behavior of APREPRO[2] and DPREPRO[3] and more tuned to simulation 
input files

[1]: https://bottlepy.org/docs/dev/stpl.html
[2]: https://github.com/gsjaardema/seacas
[3]: https://dakota.sandia.gov/
"""

DEBUGCLI = False

###########################################################################
############################# Global Settings #############################
###########################################################################
# These may be changed from within the main function. Globals
# are *not* an ideal approach, but it makes it easier to combine
# different Bottle code w/o turning it into its own class
DEFAULT_FMT = '%0.10g'
DEFAULT_FMT0 = DEFAULT_FMT # Store the original in case `setfmt` is called

BLOCK_START = '{%'
BLOCK_CLOSE = '%}'
LINE_START = '%' 
INLINE_START = '{'
INLINE_END = '}'

CLI_MODE = False # Reset in the if __name__ == '__main__'
###########################################################################
############################## Main Functions #############################
###########################################################################
def pyprepro(tpl,include_files=None,json_include=None,env=None,fmt='%0.10g',
    code='%',code_block='{% %}',inline='{ }'):
    """
    Main pyprepro function.
    
    Arguments:
    ----------
    tpl:
        Either a string representing the template or a filename. Will 
        automatically read if a filename.
    
    Options:
    --------
    include_files: (filename string or list of filenames)
        Files to be read *first* and take precendance (i.e. set as immutable). 
        No output from these are printed!
    
    json_include: (filename string or list of filenames)
        JSON files of variables to be read that take precendance 
        (i.e. set as immutable). These are read after include_files
    
    env: (dictionary)
        A dictionary of additional settings. If passed as an ImmutableValDict,
        immutability of the params will be maintained
    
    fmt:
        String formatting code for numerical output. Can be overidden inline
        with (for example) `{ "%5.2e" % var }`, Can specify with '%' or '{}' 
        notation
    
    code: ['%']
        Specify the string that, when it is the first non-whitespace character
        on a line, signifies a code line
    
    code_block: ['{% %}']
        Specify the open and closing strings to delineate a code block.
        Note tha the inner-most character must *not* be any of "{}[]()"
    
    inline: ['{ }']
        Specify the open and closing strings to specify an inline expression.
        Use a space to separate the start and the end.
    
    Returns:
    ---------
    resulting string from the template
    
    """
    # (re)set some globals from this function call.
    global DEFAULT_FMT,DEFAULT_FMT0
    global LINE_START,INLINE_START,INLINE_END,BLOCK_START,BLOCK_CLOSE

    DEFAULT_FMT = DEFAULT_FMT0 = fmt
    LINE_START = code
    INLINE_START,INLINE_END = inline.split()
    BLOCK_START,BLOCK_CLOSE = code_block.split()
    
    _check_block_syntax()
    
    if include_files is None:
        include_files = []
    if json_include is None:
        json_include = []
    
    if isinstance(include_files,(str,unicode)):
        include_files = [include_files]
    if isinstance(json_include,(str,unicode)):
        json_include = [json_include]
    
    # The broken_bottle code is designed (modified) such that when an 
    # environment is passed in, that environment is modified and not copied
    # Alternatively, if none is passed in, you can use `return_env` to
    # get the output. 
    
    if env is None:
        env = ImmutableValDict()
    elif not isinstance(env,ImmutableValDict):
        # Make sure env is ImmutableValDict
        # IMPORTANT: pass the incoming env as an arg and not kw 
        #            to ensure immutability is maintained.
        env = ImmutableValDict(env)
    
    # Parse all include files. Do not send in the environment since we will
    # reserve that for later.
    for include in include_files:
        _,subenv = _template(include,return_env=True)
        
        # remove the initial variables (even though they will be the same for all
        for init_var in INIT_VARS: # init_vars is a set. May init_vars 
            del subenv[init_var]
        
        # Update the main but set as immutable
        for key,val in subenv.items():
            env[key] = Immutable(val)
    
    for json_file in json_include:
        with open(json_file,'rb') as F:
            subenv = json.loads(_touni(F.read()))
        for key,val in subenv.items():
            env[key] = Immutable(val)
        
        
    
    # perform the final evaluation. Note that we do *NOT* pass `**env` since that
    # would create a copy.
    output = _template(tpl,env=env)     

    return output

def _parse_cli(argv,positional_include=False):
    """
    Handle command line input.
    
    
    Inputs:
        argv: The command line argumnets. Ex: sys.argv[1:]
    
    Options:
        positional_include [False]
            If True, will expect a *single* include file as the first 
            positional argument. Otherwise, will allow for any number
            of includes via --include (this toggle is to change behavior
            for dprepro)
    """
    
    parser = argparse.ArgumentParser(\
            description=DESCRIPTION,
            epilog=EPILOG,
            formatter_class=argparse.RawDescriptionHelpFormatter)
            
    parser.add_argument('--code',default='%',metavar='CHAR',
        help='["%(default)s"] Specify the string to delineate a single code line')
    parser.add_argument('--code-block',default='{% %}',metavar='"OPEN CLOSE"',
        help=('["%(default)s"] Specify the open and close of a code block. NOTE: '
              'the inner-most character must *not* be any of "{}[]()"')) 
    parser.add_argument('--inline',default='{ }',metavar='"OPEN CLOSE"',
        help=('["%(default)s"] Specify the open and close of inline '
              'code/variables to print')) # out of order but makes more sense
              
#     parser.add_argument('--dakota-params',action='store_true',
#         help=('Tells %(prog)s that command-line include files are Dakota ' 
#               'parameter files. Note: does *NOT* affect the `include()` '
#               'function inside templates. All ":" in variables names are converted to "_".'))
    
    if positional_include:
        parser.add_argument('include', help='Include (parameter) file.')
    else:
        parser.add_argument('-I','--include',metavar='FILE',action='append',default=[],
            help=('Specify a file to read before parsing input. '
                  'Note: All variables read from the --include will be take precedence '
                  '(i.e. be immutable). You later make them mutable if necessary. '
                  'Can specify more than one and they will be read in order. '
                ))
            
    parser.add_argument('--json-include',metavar='FILE',
                        help=('Specify a JSON formatted file to load variables '
                              'directly. As with `--include`, all variables will '
                              'be immutable. These will be read *after* other included '
                              'files'))
    
    parser.add_argument('--no-warn',action='store_false',default=True,dest='warn',
        help = ('Silence warning messages.'))
            
    parser.add_argument('--output-format',default='%0.10g',dest='fmt',
        help=("['%(default)s'] Specify the default float format. Note that this can "
              "be easily overridden inline as follows: `{'%%3.8e' %% param}`. "
              "Specify in either %%-notation or {}.format() notation."))
    parser.add_argument('--var',metavar='"var=value"',action='append',default=[],
        help = ('Specify variables to predefine. They will be defined as '
                'immutable. Use quotes to properly delineate'))

    # Positional arguments. In reality, this is set this way so
    # the help text will format correctly. We will rearrange arguments
    # post-parsing so that all but the last two are command line.
    
    # include is set above based on positional_include
    parser.add_argument('infile', help='Specify the input file. Or set as `-` to read stdin')
    parser.add_argument('outfile', nargs='?',
        help='Specify the output file. Otherwise, will print to stdout') 

    ## dprepro fallbacks:
    parser.add_argument('--left-delimiter',help=argparse.SUPPRESS)
    parser.add_argument('--right-delimiter',help=argparse.SUPPRESS)
    
    # Version
    parser.add_argument('-v', '--version', action='version', 
        version='%(prog)s-' + __version__,help="Print the version and exit")
    
    
    # Hidden debug
    parser.add_argument('--debug',action='store_true',help=argparse.SUPPRESS)
    
    args = parser.parse_args(argv)
    
    if args.debug:
        global DEBUGCLI
        DEBUGCLI = True

    ########## Handle Dakota fallbacks

    left,right = args.inline.split()
    left  = args.left_delimiter  if args.left_delimiter  else left
    right = args.right_delimiter if args.right_delimiter else right
    args.inline = left + ' ' + right
        
    del args.left_delimiter
    del args.right_delimiter

    # Evaluate additional vars from command line (as immutable)  
    env = ImmutableValDict()
    for addvar in args.var:
        # TODO: support strings that contain =
        addvar = addvar.split('=',2)
        if len(addvar) != 2:
            sys.stderr.write('ERROR: --var must be of the form `--var "var=value"`\n')
            sys.exit()
        key,val = addvar
        
        key = key.strip()
        # Try to convert it to a float. 
        try:
            val = float(val)
        except ValueError:
            val = val.strip()
        
        env[key] = Immutable(val)
    
    # Read stdin if needed
    if args.infile == '-':
        args.infile = _touni(sys.stdin.read())
    
    return args,env

def _pyprepro_cli(argv):
    """
    Actual CLI parser
    """
    try:
        args,env = _parse_cli(argv)
        
        output = pyprepro(args.infile,
                     include_files=args.include,
                     json_include=args.json_include,
                     env=env,
                     fmt=args.fmt,
                     code=args.code,
                     code_block=args.code_block,
                     inline=args.inline,
                )
    except (NameError,BlockCharacterError) as E:
        if DEBUGCLI:
            raise
            
        sys.stderr.write('Error occurred:\n  ' + E.args[0] + '\n')        
        sys.exit(2)
    
    if args.outfile is None:
        sys.stdout.write(output)
    else:
        with open(args.outfile,'wt',encoding='utf8') as FF:
            FF.write(output)
            
###########################################################################
############################# Helper Functions ############################
###########################################################################

class BlockCharacterError(Exception):
    pass

def _check_block_syntax():
    """
    Confirm that the open and closing blocks inner-most characters
    are not any of "{}[]()"
    """
    if BLOCK_START[-1] in "{}[]()" or BLOCK_CLOSE[0] in "{}[]()":
        raise BlockCharacterError('Cannot have inner-most code block be any of "{}[]()" ')
    
def _mult_replace(text,*A,**replacements):
    """
    Simple tool to replace text with replacements dictionary.
    Input can be either `param=val` or (param,val) tuples.
    
    Can also invert if _invert=True
    """
    invert = replacements.pop('_invert',False)
    for item in A:
        if isinstance(item,dict):
            replacements.update(item)
    
    for key,val in replacements.items():
        if invert:
            val,key = key,val
        text = text.replace(key,val)
    return text

def _formatter(*obj):
    """
    Perform the formatting for output
    """
    # Unexpand tuples
    if len(obj) == 1:
        obj = obj[0] 
    else:
        return '(' + ','.join(_formatter(o) for o in obj) + ')'
        
    if obj is None:
        return ''     
    if isinstance(obj,Immutable):
        obj = obj.val
    if isinstance(obj,(unicode,str)):
        return obj
    if isinstance(obj,bytes):
        return _formatter(_touni(obj))
    if isinstance(obj,bool):
        return '{0}'.format(obj) # True or False
    
    try:
        if '%' in DEFAULT_FMT:
            return DEFAULT_FMT % obj  # numerical
        elif '{' in DEFAULT_FMT:
            return DEFAULT_FMT.format(obj)
    except: pass
    
    # See if it is numpy (w/o importing numpy)
    if hasattr(obj,'tolist'):
        obj = obj.tolist()
    
    # Special case for lists of certain types
    if isinstance(obj,list):
        if len(obj) == 1: # Single item
            return _formatter(obj[0])
        newobj = []
        for subobj in obj:
            if not isinstance(subobj,(Immutable,unicode,str,bytes,bool,int,float)):
                break
            newobj.append(_formatter(subobj))
        else: # for-else only gets called if the for loop never had a break
            return '[' + ', '.join(newobj) + ']'

    # Fallback to pprint
    try:
        return pprint.pformat(obj,indent=1)
    except:
        pass

    # give up!
    return repr(obj)

def _preparser(text):
    """
    This is a PREPARSER before sending anything to Bottle.
    
    It parses out inline syntax of `{ param = val }` so that it will still 
    define `param`. It will also make sure the evaluation is NOT inside 
    of %< and %} blocks (by parsing them out first).
    
    It also handles escaped inline assigments
    
    Can also handle complex siutations such as:
    
        {p = 10}
        start,{p = p+1},{p = p+1},{p = p+1},end
        {p}
    
    which will turn into the following.
    
        \\
        {% p = 10 %}
        { p }
        start,\\
        {% p = p+1 %}
        { p },\\
        {% p = p+1 %}
        { p },\\
        {% p = p+1 %}
        { p },end
        {p}
    
    and will (eventually) render
        
        10
        start,11,12,13,end
        13

    This will also fix assignments made such as 
        
        { ASV_1:fun1 = 1 }
    
    to 
    
        { ASV_1_fun1 = 1 }
    
    and can handle lines such as { p += 1 }
    """
    # Clean up
    text = _touni(text)
    text = text.replace(u'\ufeff', '') # Remove BOM from windows
    text = text.replace('\r','') # Remove `^M` characters
    
    # Remove any code blocks and replace with random text
    code_rep = defaultdict(lambda:_rnd_str(20))    # will return random string but store it
    _,text = _delim_capture(text,'{0} {1}'.format(BLOCK_START,BLOCK_CLOSE), # delim_capture does NOT want re.escape
                            lambda t:code_rep[t])
    
    #if text != 'BLA': import ipdb;ipdb.set_trace()
    
    # Convert single line expression "% expr" and convert them to "{% expr %}" 
    search  =  "^([\t\f ]*)LINE_START(.*)".replace('LINE_START',re.escape(LINE_START))
    replace = r"\1{0} \2 {1}".format(BLOCK_START,BLOCK_CLOSE)
    text = re.sub(search,replace,text)
    
    # and then remove them too!
    _,text = _delim_capture(text,'{0} {1}'.format(BLOCK_START,BLOCK_CLOSE), # delim_capture does NOT want re.escape
                            lambda t:code_rep[t])
    
    ###### Bracket Escaping
    # Apply escaping to things like '\{' --> "{" and "\\{" --> "\{"
    # by replacing them with a variable. First, remove all inline, then find
    # the offending lines, replace them, then add back in the inline
    inline_rep = defaultdict(lambda:_rnd_str(20)) 
    _,text = _delim_capture(text,
                            '{0} {1}'.format(INLINE_START,INLINE_END), # do not use re escaped
                            lambda t:inline_rep[t]) 
    
    # Replace '\{' with a variable version of '{ _INLINE_START }'. Make sure it is not escaped
    text = re.sub(r'(?<!\\)\\{0}'.format(re.escape(INLINE_START)),
                  r'{0} _INLINE_START {1}'.format(INLINE_START,INLINE_END),
                  text) 
    
    # replace '\\{' with '\{ _INLINE_START }' since it is escaped
    text = re.sub(r'\\\\{0}'.format(re.escape(INLINE_START)),
                  r'{0}_eINLINE_START{1}'.format(INLINE_START,INLINE_END),
                  text)

    # Replace '\}' with a variable version of '{ _INLINE_END }'. Make sure it is not escaped
    text = re.sub(r'(?<!\\)\\{0}'.format(re.escape(INLINE_END)),
                  r'{0} _INLINE_END {1}'.format(INLINE_START,INLINE_END),
                  text) # reminder r"\\" will *still* be "\" to regex
    
    # replace '\\{' with '\{ _INLINE_END }' since it is escaped
    text = re.sub(r'\\\\{0}'.format(re.escape(INLINE_END)),
                  r'{0}_eINLINE_END{1}'.format(INLINE_START,INLINE_END),
                  text)
                  
    # Sub back in the other removed inline expressions
    text = _mult_replace(text,inline_rep,_invert=True)       
    ###### /Bracket Escaping
    
    # Apply _inline_fix to all inline assignments
    _,text = _delim_capture(text,
                            '{0} {1}'.format(INLINE_START,INLINE_END), # do not use re escaped
                            _inline_fix)
                
    
    # Re-add the code blocks with an inverted dict
    return _mult_replace(text,code_rep,_invert=True)

def _inline_fix(capture):
    """
    Replace the matched line in a ROBUST manner to allow multiple definitions
    on each line
    """
    # Take EVERYTHING and then remove the outer.

    match = capture[len(INLINE_START):-len(INLINE_END)].strip() # Remove open and close brackets
    
    # Need to decide if this is a {param} or {var=param}
    # But need to be careful for:
    #
    #   {var = "text}"}
    #   {function(p="}")}
    #
    # Do this by splitting at '=' but make sure there are no
    # disallowed characters. Check for assignment (+=) and comparison (<=)
    #
    # Also fixes lines such as  {ASV_1:fun1 = 1}, {ASV_1:fun1} but will *ignore*
    # {"ASV_1:fun1"}
    
    def _fix_varnames(name):
        """
        Fix variable names
        * remove colons
        * Add `i` to leading integers
        """
        name = name.strip().replace(':','_')
        if name[0] in '0123456789':
            name = 'i' + name
        return name
        
    parts = match.split('=',1)
    if len(parts) != 2: # *must* be just {param}
        return capture # Do NOT fix since we dissallow variables like "A:B".
                         # They will already have been converted to "A_B"

    var,val = parts # Can't be more than two
    var = var.strip()
    if any(c in var for c in ['"',"'",'(',')']):  # something like {function(p="}")}
        return capture # Do not fix. See above
    
    opperator = '='
    
    # is it a modified assignment operator (e.g. "+=","<<=") but NOT comparison (e.g. "<=").
    # Check first for assignment and ONLY then can you check for comparison.
    assignment_mods = ['+', '-', '*', '/', '%', '//', '**', '&', '|', '^', '>>', '<<'] # += -= *=, etc
    comparison_mods = ['=','!','>','<'] # ==,!=, etc
    
    is_assignment = False
    for v in assignment_mods:
        if var.endswith(v):
            is_assignment = True
            var = var[:-len(v)]
            opperator = v + opperator
            break
            
    if not is_assignment:
        for v in comparison_mods:
            if var.endswith(v):
                var = var[:-len(v)]
                var = _fix_varnames(var)   # { A <= 10 } and/or {A:1 <= 10} becomes {A_1<=10}
                opperator = v + opperator
                return INLINE_START + var + opperator + val + INLINE_END 
               
    # Fix disallowed var names
    var = _fix_varnames(var)
    
    # Set the value
    return ''.join([r'\\','\n', 
                    BLOCK_START,' ',var,opperator,val,' ',BLOCK_CLOSE,'\n', 
                    INLINE_START,' ',var.strip(),' ',INLINE_END])


def _delim_capture(txt,delim,sub=None):
    '''
    Combination of regex and some hacking to LAZY capture text between 
    the delims *while* accounting for quotes. 
    
    Returns the captured group INCLUDING the delimiters
    
    For example, consider delim = "{% %}", it will handle:
        '{%testing%}'             --> {%testing%}             (1)
        '{%test"%}"ing%}'         --> {%test"%}"ing%}         (1)
        '{%te"""%}" """sting%}'   --> {%te"""%}" """sting%}   (1)
        '{%TE"%}"%}{%STING%}'     --> {%TE"%}"%}, {%STING%}   (2)
        '"{%test"%}"ing%}"'       --> {%test"%}"ing%}         (1)
    
    (notice it handles quotes around the matches)
    
    This is an alternative to more complex regexes such as those discussed
    in https://stackoverflow.com/a/22184202/3633154
    
    inputs:
        txt     : The input text
        delim   : Space-separated delimiters. DO NOT re.escape them!
    
    options:
        sub     : [None] text to replace the capture or function. 
                  NOTE: if it is a function, it will be passed the string only
                  and *not* the SRE_Match object
    
    returns:
        captured: List of captured items NOT subbed
        txt     : Resulting txt (potentially with the subs)
    ''' 
    
    # Algorithm:
    #   1. Find the first opening of a block
    #       a. If none was found, add the rest of the text to the out
    #          and break
    #       b. Add all preceding text to the output and trim it off txt 
    #   2. Remove all quoted strings from remaining text
    #   3. Split at the closing block.
    #       a. If not found, replace quoted txt, add to output, and break.
    #          This is a poorly formed file!!!
    #   4. Replace quoted txt in both the capture block and remaining text.
    #      Also re-add the closing text since it was removed in split
    #   5. Store capture block (and sub if applicable)
    #   6. Continue until break
    
    # Set up the regexes and the output
    OPEN,CLOSE = delim.split()
    rOPEN,rCLOSE = [re.escape(d) for d in (OPEN,CLOSE)]
    reOPEN = re.compile(r'(?<!\\)' + rOPEN) # Checks for escape
    reCLOSE = re.compile(rCLOSE)
    
    reQUOTE = re.compile(r"""
         '{3}(?:[^\\]|\\.|\n)+?'{3}        # 3 single ticks
        |\"{3}(?:[^\\]|\\.|\n)+?\"{3}      # 3 double ticks
        |\".+?\"                           # 1 double tick
        |'.+?'                             # 1 single tick
        """,flags=re.MULTILINE|re.DOTALL|re.UNICODE|re.VERBOSE)  # Regex to capture quotes   
    
    outtxt = []
    captured = []
    
    while True:
        match = reOPEN.search(txt)
        if not match:
            outtxt.append(txt)
            break
        
        outtxt.append(txt[:match.start()])
        txt = txt[match.start():]
        
        # Remove all correctly quoted material (i.e. has matching quotes)
        quote_rep = defaultdict(lambda:_rnd_str(20))            # will return random string but store it
        txt = reQUOTE.sub(lambda m:quote_rep[m.group(0)],txt)   # Replace quotes with random string
        
        # Find the end
        try:
            cap,txt = reCLOSE.split(txt,1)
        except ValueError: # There was no close. Restore txt and break
            outtxt.append(_mult_replace(txt,quote_rep,_invert=True))
            break
        
        # Restore both captured and txt
        cap = _mult_replace(cap,quote_rep,_invert=True) + CLOSE
        txt = _mult_replace(txt,quote_rep,_invert=True)
        
        captured.append(cap)
        
        # Apply sub and then add to outtxt
        if sub is not None:
            if callable(sub): # callabe
                cap = sub(cap)
            else:
                cap = sub
        outtxt.append(cap)
        
    return captured,''.join(outtxt)

###### Functions for inside templates  

def _vartxt(env,return_values=True,comment=None):
    """
    small helper to print the variables in the environment.
    
    If comment is set, will prepend all lines with the comment character
    """
    subenv = dict((k,v) for k,v in env.items() if k not in INIT_VARS)
    
    if return_values:
        txt = pprint.pformat(subenv,indent=1)
    else:
        txt = pprint.pformat(list(subenv.keys()),indent=1)

    if comment is None:
        return txt
    
    if not any(comment.endswith(c) for c in " \t"): 
        comment += ' ' # make sure ends with space
    
    return '\n'.join(comment + t for t in txt.split('\n'))

def _setfmt(fmt=None):
    """
    (re)set the global formatting. If passed None, will reset to initial
    """
    global DEFAULT_FMT
    DEFAULT_FMT = fmt if fmt is not None else DEFAULT_FMT0

def _vset(key,val,env=None):
    """
    Used inside the templates (with partial(_vset,env=env) ) to set a variable
    and also print the name.
    """
    if env is None:
        raise ValueError('Must specify an env')
    env[key] = val
    return '{0} = {1}'.format(key,env[key]) # use env[key] for val since it may be immutable
    

####### This is the main driver of immutability inside of eval statements
class ImmutableValDict(dict):
    """
    A regular dict with the ability to set Immutable key and values.
    
    For example:
        D = ImmutableValDict()
        D['a'] = Immutable(10)
        D['a'] = 20
        D['a'] == 20 # False
        D['a'] == 10 # True
    
    In the above, the key 'a' is not overritten. But, the value itself 
    may be mutable:
    
        D = ImmutableValDict()
        D['b'] = Immutable([1,2]) # Lists are mutable but 'b' will be fixed
        D['b'].append(3)
        D['b'] == [1,2,3] # True
    
    Note, you *could* do:
    
        D = ImmutableValDict()
        obj = [1,2,3]
        c = Immutable(obj)
        D['c'] = c
        
        # But note:
        D['c'] is obj # True -- same object
        c is obj # False    

    """
    def __init__(self, *args, **kwargs):
        # This has to be overridden to call __setitem__
        self.__locked = set() # define first since update will use it
        self.update(*args, **kwargs)
    
    def __setitem__(self,key,item):
        """
        Set the key but only the previously defined item is not
        already immutable
        """
        if isinstance(item,Mutable): # Check first since Mutable inherits Immutable
            item = item.val
            if key in self.immutables:
                self.immutables.remove(key)
                
        if key in self.immutables:
            return
        
        if isinstance(item,Immutable):
            self.__locked.add(key)
            item = item.val
        super(ImmutableValDict,self).__setitem__(key,item)
    
    def __delitem__(self,key):
        if key in self.__locked:
            self.immutables.remove(key)
        super(ImmutableValDict,self).__delitem__(key)
    
    def update(self, *args, **kwargs):
        """
        Update the keys in the dictionary
        """
        # This has to be overridden to call __setitem__ and to
        # keep immutability of vars if the input is an ImmutableValDict
        for k, v in dict(*args, **kwargs).items():
            self[k] = v
        # Update the locked keys if args[0] is an ImmutableValDict
        if len(args)>0 and isinstance(args[0],ImmutableValDict):
            self.immutables.update(args[0].immutables)
        
    @property
    def immutables(self):
        return self.__locked
    
class Immutable:
    """
    Container object for ImmutableValDict
    """
    __slots__ = ('val',)
    def __init__(self,val):
        self.val = val
    def __repr__(self):
        return '(Immutable(' + self.val.__repr__() + ')'
    __str__ = __repr__

class Mutable(Immutable):
    """
    Container object for ImmutableValDict
    """
    def __repr__(self):
        return '(Mutable(' + self.val.__repr__() + ')'   

def _rnd_str(N=10):
    CH = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
    return ''.join(random.choice(CH) for _ in range(N))

###########################################################################
############################ dprepro functions ############################
###########################################################################
# dprepro is designed to be called directly by dakota [1] and follows a 
# similar syntax. The biggest difference is that dprepro takes an include 
# file as a positional argument and that include file will *always* be
# of one of two Dakota formats:
# 
#     val param
# 
# or
# 
#     {param = val}
#     
# (the latter will work in aprepro iff the inline syntax is not changed
# 
# [1]: http://dakota.sandia.gov

def _add_di_paths():
    di_path = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)),"..","share","dakota","Python"))
    sys.path.append(di_path)

diwarning = """
WARNING: dprepro could not find dakota.interfacing module. Make sure either
         the PYTHONPATH environment is correctly set and/or dprepro has not
         been moved from its original installed location
         
         dprepro will fallback to defining all dakota settings in the
         environment.
""" # This is not called automatically
def convert_dakota(input_file):
    """
    Convert files to be the correct format and return the env
    """
    # Note: Dakota files can be
    #     val param
    # by default or
    #     { val = param }
    # in aprepro mode (regardless of the delinators set here)
    env = {}
    ### TMP
    if isinstance(input_file,(list,tuple)):
        assert len(input_file) == 1,"WARNING: Does  not handle multple yet"
        input_file = input_file[0]
    ### /TMP
   
    N = None
    # Use pyprepro's _touni since it is more robust to windows encoding
    with open(input_file,'rb') as F:
        lines = _touni(F.read()).strip().split('\n') 
        
    for n,line in enumerate(lines):
        line = line.strip()
        if len(line) == 0:
            continue
        if line.startswith('{'): # aprepro "{key = value}"
            line = line[1:-1]
            key,val = line.split('=',1)
            val = val.strip()
            if val[0] in ['"',"'"]:
                # it's a string with quotes
                val = val[1:-1]
                
        else:                   # dakota " value key "
             # Need to split but also have to worry about string (and spaces in strings)
             # so do an rsplit
             val,key = line.rsplit(None,1)
             
        try:
            val = float(val)
        except ValueError:
            val = val.strip()
        
        key = key.strip()
        env[key] = val
            
        # The first line is the parameters. Assume it can be read
        # but add a fallback if not
        if n == 0:
            try:
                N = int(val)
            except ValueError: # Could not be read
                N = float('inf')
        
        # Only do the parameters themselves. Not the other ASV... stuff
        if n >= N: # n starts at 0
            break
    
    return env


def _fix_param_name(param,warn=False):
    """
    Fix param/key names to be valid python. If warn == True, will add a
    warning to stderr

    1. Convert characters that are not alphanumeric or _ to _. Alphanumeric
       means not just ascii, but includes many Unicode characters.
    2. Python 2 allows only ascii alphanumeric (+ _) identifiers, so "normalize"
       everything to ascii. E.g. ñ -> n.
    """
    param = _touni(param) # Ensure the string is unicode in case passed bytes
    
    param0 = param # string are immutable so it won't be affect by changes below
    param = re.compile("\W",flags=re.UNICODE).sub('_',param) # Allow unicode on python2 (and compile first for 2.6)
    if re.match("\d",param[0],flags=re.UNICODE):
        param = 'i' + param
    while keyword.iskeyword(param):
        param += "_"

    # unicode check for python2
    add_unicode_warn = False
    if not py3:
        import unicodedata
        param0u = param
        param = unicodedata.normalize('NFKD', param).encode('ascii','ignore') # https://www.peterbe.com/plog/unicode-to-ascii convert to ascii
        param = unicode(param)
        add_unicode_warn = param0u != param 

    if param0 != param and warn:
        txt = (u'WARNING: Paramater "{0}" is not a valid name.\n'
               u'         Converted to "{1}"\n'.format(param0,param))
        
        if pyv < (2,7):
            txt = txt.encode('utf8')
        
        sys.stderr.write(txt)
        
        if add_unicode_warn:
            sys.stderr.write('         Unicode characters in variable name.\n'
                             '         Must use python3!\n')

    return param

def _dprepro_cli(argv):
    """
    CLI parser
    """
    # Import dakota.interfacing here to avoid circular import
    _add_di_paths()
    try:
        import dakota.interfacing as di
    except ImportError:
        di = None
    
    args,env = _parse_cli(argv,positional_include=True)

    params = None
    results = None
    # Convert Dakota
    if di is None:
        if args.warn:
            sys.stderr.write(diwarning + '\n') # print the error message
        env2 = convert_dakota(args.include) # ToDo: Check with Adam on how this should be passed
        env.update(env2)
    else:
        try:
            params, results = di.read_parameters_file(parameters_file=args.include,results_file=di.UNNAMED)
        except di.ParamsFormatError as e:
            sys.stderr.write("Error occurred: " + e.args[0] + "\n")
            sys.exit(2)

        env["DakotaParams"] = params
        for d, v in params.items():
            env[d] = v
        env["DakotaResults"] = results
        for d, v in results.items():
            env[d] = v

    try:
        output = dprepro(include=env,
                    template = args.infile,
                    fmt=args.fmt,
                    code=args.code,
                    code_block=args.code_block,
                    inline=args.inline,
                    json_include=args.json_include,
                    warn=args.warn
                    )
    except Exception as E:
        # the _template has a catch but this will be the last resort.
        if DEBUGCLI:
            raise
            
        sys.stderr.write('Error occurred:\n  ' + E.args[0] + '\n')        
        sys.exit(2)
    
    if args.outfile is None:
        sys.stdout.write(output)
    else:
        with open(args.outfile,'wt',encoding='utf8') as FF:
            FF.write(output)

def dprepro(include=None, template=None, output=None, fmt='%0.10g', code='%', 
            code_block='{% %}', inline='{ }',warn=True,**kwargs):
    """Validate Dakota parameters and insert them into a template
    
    Keyword Args:
        
        include(dict): Items to make available for substitution
        template(str or IO object): Template. If it has .read(), will be 
            treated like a file. Otherwise, assumed to be the name of a file.
        output(str or IO object): If None (the default), the substituted
            template will be returned as a string. If it has .write(), will
            be treated like a file. Otherwise, assumed to be the name of a file.
        fmt(str): Default format for numerical fields. Default: '%0.10g'
        code(str): Delimiter for a code line. Default: '%'
        code_block(str): Delimiters for a code block. Default: '{% %}'
        inline(str): Delimiters for inline substitution. Default: '{ }'
        warn(bool): Whether or not to warn the user of invalid parameter names

    Returns:
        If no output is specified, returns a string containing the substituted
            template.
    """
    # Construct the env from parameters, results, and include
    env = ImmutableValDict()
    
    if include is None:
        include = {}
    for key, val in include.items():
        param0 = key
        param = _fix_param_name(param0,warn=warn)
        if param0 != param and warn:
            txt = """         Or, may be accessed via "DakotaParams['{0}']"\n""".format(param0)
            if sys.version_info < (2,7):
                txt = txt.encode('utf8')
            sys.stderr.write(txt)
        env[param] = Immutable(val)
    
    # read in the template if needed
    use_template = template
    if hasattr(template,"read"):
        use_template = template.read()

    # Call the template engine
    output_string = pyprepro(tpl=use_template, 
                             env=env, 
                             fmt=fmt, 
                             code=code,
                             code_block=code_block, 
                             inline=inline,**kwargs)

    # Output
    if output is None:
        return output_string
    elif hasattr(output, "write"):
        output.write(output_string)
    else:
        with open(output,"wt") as f:
            f.write(output_string)





###########################################################################
####################### BottlePy Extracted Functions ######################
###########################################################################
# This is all pulled from Bottle with lots of little changes to make it work
# 
# A NON-EXHAUSTIVE list of changes are below:
# 
# Major:
# 
# * Changed the default environment to ImmutableValDict
# * Added Immutable and Mutable functions to be passed it
# * All text is routed through _preparser (3 places...I think)
# * Ability to return the environment
# * Adjusted scope so that if a variable is parsed in an include, it is present
#   in the parent. (the env object is passed in and NEVER copied)
# 
# Minor:
# 
# * {{ }} syntax to { } (though settable)
# * No HTML escaping
# * No caching
# * math namespace is imported
# * Simply decide if input is filename or string based on whether the file
#   exists
# * Fix for local files with absolute system paths
# * Commented out rebase
#
##############################################################################
# Copyright (c) 2017, Marcel Hellkamp.
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#############################################################################

#TEMPLATE_PATH = "./"
TEMPLATES = {}
DEBUG = True # Will also turn off caching


class TemplateError(Exception):
    pass

def _touni(s, enc=None, err='strict'):
    if enc is None:
        # This ordering is intension since, anecdotally, some Windows-1252 will
        # be decodable as UTF-16. The chardet module is the "correct" answer
        # but we don't want to add the dependency
        enc = ['utf8','Windows-1252','utf16','ISO-8859-1',]
    
    if isinstance(enc,(str,unicode)):
        enc = [enc]
    
    if isinstance(s, bytes):
        for e in enc:
            try:
                return s.decode(e, err)
            except UnicodeDecodeError:
                pass
    
    return unicode("" if s is None else s)
    
class _cached_property(object):
    """ A property that is only computed once per instance and then replaces
        itself with an ordinary attribute. Deleting the attribute resets the
        property. """

    def __init__(self, func):
        _update_wrapper(self, func)
        self.func = func

    def __get__(self, obj, cls):
        if obj is None: return self
        value = obj.__dict__[self.func.__name__] = self.func(obj)
        return value
# A bug in functools causes it to break if the wrapper is an instance method
def _update_wrapper(wrapper, wrapped, *a, **ka):
    try:
        functools.update_wrapper(wrapper, wrapped, *a, **ka)
    except AttributeError:
        pass
class _BaseTemplate(object):
    """ Base class and minimal API for _template adapters """
    extensions = ['tpl', 'html', 'thtml', 'stpl']
    settings = {}  #used in prepare()
    defaults = {}  #used in render()

    def __init__(self,
                 source=None,
                 name=None,
                 lookup=None,
                 encoding='utf8', **settings):
        """ Create a new _template.
        If the source parameter (str or buffer) is missing, the name argument
        is used to guess a _template filename. Subclasses can assume that
        self.source and/or self.filename are set. Both are strings.
        The lookup, encoding and settings parameters are stored as instance
        variables.
        The lookup parameter stores a list containing directory paths.
        The encoding parameter should be used to decode byte strings or files.
        The settings parameter contains a dict for engine-specific settings.
        """
        self.name = name
        self.source = _preparser(source.read()) if hasattr(source, 'read') else source
        self.filename = source.filename if hasattr(source, 'filename') else None
        self.lookup = [os.path.abspath(x) for x in lookup] if lookup else []
        self.encoding = encoding
        self.settings = self.settings.copy()  # Copy from class variable
        self.settings.update(settings)  # Apply
        if not self.source and self.name:
            self.filename = self.search(self.name, self.lookup)
            if not self.filename:
                raise TemplateError('Template %s not found.' % repr(name))
        if not self.source and not self.filename:
            raise TemplateError('No _template specified.')
        self.prepare(**self.settings)

    @classmethod
    def search(cls, name, lookup=None):
        """ Search name in all directories specified in lookup.
        First without, then with common extensions. Return first hit. """
        #if not lookup:
        #    raise depr(0, 12, "Empty _template lookup path.", "Configure a _template lookup path.")
        #if os.path.isabs(name):
        #    raise depr(0, 12, "Use of absolute path for _template name.",
        #               "Refer to _templates with names or paths relative to the lookup path.")
        
        # JW: Search full system name first:
        if os.path.isfile(name):
            return os.path.abspath(name)
        
        for spath in lookup:
            spath = os.path.abspath(spath) + os.sep
            fname = os.path.abspath(os.path.join(spath, name))
            if not fname.startswith(spath): continue
            if os.path.isfile(fname): return fname
            for ext in cls.extensions:
                if os.path.isfile('%s.%s' % (fname, ext)):
                    return '%s.%s' % (fname, ext)

    @classmethod
    def global_config(cls, key, *args):
        """ This reads or sets the global settings stored in class.settings. """
        if args:
            cls.settings = cls.settings.copy()  # Make settings local to class
            cls.settings[key] = args[0]
        else:
            return cls.settings[key]

    def prepare(self, **options):
        """ Run preparations (parsing, caching, ...).
        It should be possible to call this again to refresh a _template or to
        update settings.
        """
        raise NotImplementedError

    def render(self, *args, **kwargs):
        """ Render the _template with the specified local variables and return
        a single byte or unicode string. If it is a byte string, the encoding
        must match self.encoding. This method must be thread-safe!
        Local variables may be provided in dictionaries (args)
        or directly, as keywords (kwargs).
        """
        raise NotImplementedError


class _SimpleTemplate(_BaseTemplate):
    def prepare(self,
                escape_func=lambda a:a,
                noescape=True,
                syntax=None, **ka):
        self.cache = {}
        enc = self.encoding
        self._str = _formatter
        self._escape = lambda x: escape_func(_touni(x, enc))
        self.syntax = syntax
        if noescape:
            self._str, self._escape = self._escape, self._str

    @_cached_property
    def co(self):
        return compile(self.code, self.filename or '<string>', 'exec')

    @_cached_property
    def code(self):
        source = self.source
        if not source:
            with open(self.filename, 'rb') as f:
                source = f.read()
        try:
            source, encoding = _touni(source), 'utf8'
        except UnicodeError:
            raise depr(0, 11, 'Unsupported _template encodings.', 'Use utf-8 for _templates.')
        source = _preparser(source)
        parser = _StplParser(source, encoding=encoding, syntax=self.syntax)
        code = parser.translate()
        self.encoding = parser.encoding
        return code

    def _rebase(self, _env, _name=None, **kwargs):
        _env['_rebase'] = (_name, kwargs)

    def _include(self, _env, _name=None, **kwargs):
        env = _env # Use the same namespace/environment rather than a copy
        env.update(kwargs)
        if _name not in self.cache:
            self.cache[_name] = self.__class__(name=_name, lookup=self.lookup, syntax=self.syntax)
    
        return self.cache[_name].execute(env['_stdout'], env)


    def execute(self, _stdout, kwargs):
        env = kwargs # Use the same namespace/environment rather than a copy
        
        # Math + constants
        env.update( dict((k,v) for k,v in vars(math).items() if not k.startswith('__'))    )
        env.update({'tau':2*math.pi,
                    'deg':180/math.pi,
                    'rad':math.pi/180,
                    'E':math.e,
                    'PI':math.pi,
                    'phi':(math.sqrt(5)+1)/2,
                    })
        
        # Other helpful functions (esp. to make sure py3 works the same)
        env.update({'unicode':unicode,  # set at top for py2
                    'xrange':xrange,    # "..."
                   })
                    
        # pyprepro Functions
        env.update({
            '_stdout': _stdout,
            '_printlist': _stdout.extend,
            'include': functools.partial(self._include, env),
            #'rebase': functools.partial(self._rebase, env),
            #'_rebase': None,
            '_str': self._str,
            '_escape': self._escape,
            'get': env.get,
            #'setdefault': env.setdefault,
            'defined': env.__contains__,
            '_copy':copy.copy,
            # Added:    
            'vset':functools.partial(_vset,env=env),
            'Immutable':Immutable,
            'Mutable':Mutable,
            'setfmt':_setfmt,
            'all_vars':lambda **k: _vartxt(env,return_values=True,**k),
            'all_var_names':lambda **k: _vartxt(env,return_values=False,**k),
        })
        
        # String literals of escape characters
        env.update({
            '_BLOCK_START':BLOCK_START,
            '_BLOCK_CLOSE':BLOCK_CLOSE,
            '_LINE_START':LINE_START,
            '_INLINE_START':INLINE_START,
            '_eINLINE_START': '\\' + INLINE_START,
            '_INLINE_END':INLINE_END,
            '_eINLINE_END':'\\' + INLINE_END,
            })

        
        exec_(self.co,env)
        
        if env.get('_rebase'):
            subtpl, rargs = env.pop('_rebase')
            rargs['base'] = ''.join(_stdout)  #copy stdout
            del _stdout[:]  # clear stdout
            return self._include(env, subtpl, **rargs)
        return env

    def render(self,env=None):
        """ Render the _template using keyword arguments as local variables. """
        if env is None:
            env = ImmutableValDict()
        stdout = []
        env = self.execute(stdout, env)
        return ''.join(stdout), env # Return both now


class StplSyntaxError(TemplateError):pass


class _StplParser(object):
    """ Parser for stpl _templates. """
    _re_cache = {}  #: Cache for compiled re patterns

    # This huge pile of voodoo magic splits python code into 8 different tokens.
    # We use the verbose (?x) regex mode to make this more manageable

    _re_tok = _re_inl = r'''(?mx)(        # verbose and dot-matches-newline mode
        [urbURB]*
        (?:  ''(?!')
            |""(?!")
            |'{6}
            |"{6}
            |'(?:[^\\']|\\.)+?'
            |"(?:[^\\"]|\\.)+?"
            |'{3}(?:[^\\]|\\.|\n)+?'{3}
            |"{3}(?:[^\\]|\\.|\n)+?"{3}
        )
    )'''

    _re_inl = _re_tok.replace(r'|\n', '')  # We re-use this string pattern later

    _re_tok += r'''
        # 2: Comments (until end of line, but not the newline itself)
        |(\#.*)

        # 3: Open and close (4) grouping tokens
        |([\[\{\(])
        |([\]\}\)])

        # 5,6: Keywords that start or continue a python block (only start of line)
        |^([\ \t]*(?:if|for|while|with|try|def|class)\b)
        |^([\ \t]*(?:elif|else|except|finally)\b)

        # 7: Our special 'end' keyword (but only if it stands alone)
        |((?:^|;)[\ \t]*end:{0,1}[\ \t]*(?=(?:%(block_close)s[\ \t]*)?\r?$|;|\#))

        # 8: A customizable end-of-code-block _template token (only end of line)
        |(%(block_close)s[\ \t]*(?=\r?$))

        # 9: And finally, a single newline. The 10th token is 'everything else'
        |(\r?\n)
    '''

    # Match the start tokens of code areas in a _template
    _re_split = r'''(?m)^[ \t]*(\\?)((%(line_start)s)|(%(block_start)s))'''
    # Match inline statements (may contain python strings)
    _re_inl = r'''%%(inline_start)s((?:%s|[^'"\n]+?)*?)%%(inline_end)s''' % _re_inl

    # default_syntax = '{% %} % { }'

    def __init__(self, source, syntax=None, encoding='utf8'):
        self.source, self.encoding = _touni(source, encoding), encoding
        self.set_syntax(' '.join( [ BLOCK_START, 
                                    BLOCK_CLOSE, 
                                    LINE_START, 
                                    INLINE_START, 
                                    INLINE_END,
                                  ]))
        self.code_buffer, self.text_buffer = [], []
        self.lineno, self.offset = 1, 0
        self.indent, self.indent_mod = 0, 0
        self.paren_depth = 0

    def get_syntax(self):
        """ Tokens as a space separated string (default: {% %} % {{ }}) """
        return self._syntax

    def set_syntax(self, syntax):
        self._syntax = syntax
        self._tokens = syntax.split()
        if syntax not in self._re_cache:
            names = 'block_start block_close line_start inline_start inline_end'
            etokens = map(re.escape, self._tokens)
            pattern_vars = dict(zip(names.split(), etokens))
            patterns = (self._re_split, self._re_tok, self._re_inl)
            patterns = [re.compile(p % pattern_vars) for p in patterns]
            self._re_cache[syntax] = patterns
        self.re_split, self.re_tok, self.re_inl = self._re_cache[syntax]

    syntax = property(get_syntax, set_syntax)

    def translate(self):
        if self.offset: raise RuntimeError('Parser is a one time instance.')
        while True:
            m = self.re_split.search(self.source, pos=self.offset)
            if m:
                text = self.source[self.offset:m.start()]
                self.text_buffer.append(text)
                self.offset = m.end()
                if m.group(1):  # Escape syntax
                    line, sep, _ = self.source[self.offset:].partition('\n')
                    self.text_buffer.append(self.source[m.start():m.start(1)] +
                                            m.group(2) + line + sep)
                    self.offset += len(line + sep)
                    continue
                self.flush_text()
                self.offset += self.read_code(self.source[self.offset:],
                                              multiline=bool(m.group(4)))
            else:
                break
        self.text_buffer.append(self.source[self.offset:])
        self.flush_text()
        return ''.join(self.code_buffer)

    def read_code(self, pysource, multiline):
        code_line, comment = '', ''
        offset = 0
        while True:
            m = self.re_tok.search(pysource, pos=offset)
            if not m:
                code_line += pysource[offset:]
                offset = len(pysource)
                self.write_code(code_line.strip(), comment)
                break
            code_line += pysource[offset:m.start()]
            offset = m.end()
            _str, _com, _po, _pc, _blk1, _blk2, _end, _cend, _nl = m.groups()
            if self.paren_depth > 0 and (_blk1 or _blk2):  # a if b else c
                code_line += _blk1 or _blk2
                continue
            if _str:  # Python string
                code_line += _str
            elif _com:  # Python comment (up to EOL)
                comment = _com
                if multiline and _com.strip().endswith(self._tokens[1]):
                    multiline = False  # Allow end-of-block in comments
            elif _po:  # open parenthesis
                self.paren_depth += 1
                code_line += _po
            elif _pc:  # close parenthesis
                if self.paren_depth > 0:
                    # we could check for matching parentheses here, but it's
                    # easier to leave that to python - just check counts
                    self.paren_depth -= 1
                code_line += _pc
            elif _blk1:  # Start-block keyword (if/for/while/def/try/...)
                code_line, self.indent_mod = _blk1, -1
                self.indent += 1
            elif _blk2:  # Continue-block keyword (else/elif/except/...)
                code_line, self.indent_mod = _blk2, -1
            elif _end:  # The non-standard 'end'-keyword (ends a block)
                self.indent -= 1
            elif _cend:  # The end-code-block _template token (usually '%}')
                if multiline: multiline = False
                else: code_line += _cend
            else:  # \n
                self.write_code(code_line.strip(), comment)
                self.lineno += 1
                code_line, comment, self.indent_mod = '', '', 0
                if not multiline:
                    break

        return offset

    def flush_text(self):
        text = ''.join(self.text_buffer)
        del self.text_buffer[:]
        if not text: return
        parts, pos, nl = [], 0, '\\\n' + '  ' * self.indent
        for m in self.re_inl.finditer(text):
            prefix, pos = text[pos:m.start()], m.end()
            if prefix:
                parts.append(nl.join(map(repr, prefix.splitlines(True))))
            if prefix.endswith('\n'): parts[-1] += nl
            parts.append(self.process_inline(m.group(1).strip()))
        if pos < len(text):
            prefix = text[pos:]
            lines = prefix.splitlines(True)
            if lines[-1].endswith('\\\\\n'): lines[-1] = lines[-1][:-3]
            elif lines[-1].endswith('\\\\\r\n'): lines[-1] = lines[-1][:-4]
            parts.append(nl.join(map(repr, lines)))
        code = '_printlist((%s,))' % ', '.join(parts)
        self.lineno += code.count('\n') + 1
        self.write_code(code)

    @staticmethod
    def process_inline(chunk):
        if chunk[0] == '!': return '_str(%s)' % chunk[1:]
        return '_escape(%s)' % chunk

    def write_code(self, line, comment=''):
        code = '  ' * (self.indent + self.indent_mod)
        code += line.lstrip() + comment + '\n'
        self.code_buffer.append(code)


def _template(tpl, env=None, return_env=False):
    """
    Get a rendered _template as a string iterator.
    You can use a name, a filename or a _template string as first parameter.
    Template rendering arguments can be passed as dictionaries
    or directly (as keyword arguments).
    """
    try:
        if env is None:
            env = ImmutableValDict()
        
        # This was changed to first see if the file exists. If it does,
        # it is assumed to be a path. Otherwise, assumed it to be text
     
        settings = {}
        tpl = _touni(tpl)
    
        # Try to determine if it is a file or a template string
        
        isfile = False
        try:
            if os.path.exists(tpl):
                isfile = True
        except:pass # Catch any kind of error
    
        if not isfile:  # template string
            lookup = ['./'] # Just have the lookup be in this path
            tpl = _preparser(tpl)
            tpl_obj = _SimpleTemplate(source=tpl, lookup=lookup, **settings)
        else: # template file
            # set the lookup. It goes in order so first check directory
            # of the original template and then the current.
            lookup = [os.path.dirname(tpl) + '/.','./']
            tpl_obj = _SimpleTemplate(name=tpl, lookup=lookup, **settings)

        # Added the option to return the environment, but this is really not needed
        # if env is set.
    
        rendered,env =  tpl_obj.render(env)
    
        if not return_env:
            return rendered
        return rendered,env
    except Exception as E:
        if CLI_MODE and DEBUGCLI:
            err = E.__class__.__name__
            desc = unicode(E)
            sys.stderr.write('Error occurred:\n  {0}: {1}\n'.format(err,desc))        
            sys.exit(2)
        else:
            raise
########################### six extracted codes ###########################
# This is pulled from the python six module (see links below) to work 
# around some python 2.7.4 issues
# Links:
#   https://github.com/benjaminp/six
#   https://pypi.python.org/pypi/six
#   http://pythonhosted.org/six/
##############################################################################
# Copyright (c) 2010-2018 Benjamin Peterson
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#############################################################################

if py3:
    exec('exec_ = exec')
else:
    def exec_(_code_, _globs_=None, _locs_=None):
        """Execute code in a namespace."""
        if _globs_ is None:
            frame = sys._getframe(1)
            _globs_ = frame.f_globals
            if _locs_ is None:
                _locs_ = frame.f_locals
            del frame
        elif _locs_ is None:
            _locs_ = _globs_
        exec("""exec _code_ in _globs_, _locs_""")

##############################################################################

# Global set of keys from an empty execution:
INIT_VARS = set(_template('BLA',return_env=True)[-1].keys())

def main():
    CLI_MODE = True 
    cmdname = sys.argv[0].lower()
    path, execname = os.path.split(cmdname)
    if execname.rstrip(".py") == 'dprepro':
        sys.exit(_dprepro_cli(sys.argv[1:]))
    sys.exit(_pyprepro_cli(sys.argv[1:]))

# When called via command line
if __name__ == '__main__':
    main()    





