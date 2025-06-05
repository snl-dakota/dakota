#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Set of tests for pprepo that include both command line processing and 
"""
from __future__ import division, print_function, unicode_literals, absolute_import
from io import open

import os
import sys
import shlex
import shutil
import random
import json
from textwrap import dedent
from itertools import product

# Find the best implementation available on this platform
try:
    from cStringIO import StringIO
except ImportError:
    try:
        from StringIO import StringIO
    except ImportError:
        from io import StringIO

sys.dont_write_bytecode = True
PY3 = False
if sys.version_info >= (3,):
    unicode = str
    PY3 = True
    from itertools import zip_longest
else:
    from itertools import izip_longest as zip_longest
import unittest
############### Need a way to catch exceptions if pytest isn't imported
# This isn't strictly needed or even used but it is included in case
# it proves to be more convenient in the future
# class FailedToRaiseError(BaseException): pass
# class raises(object):
#     """
#     Will allow ONLY specified excptions to pass silently. Will allow other 
#     errors to raise or will and will raise a FailedToRaiseError if there
#     were no errors
#     """
#     def __init__(self, *exceptions):
#         
#         # Expand tuples
#         exceptions = set(exceptions)
#         for exception in list(exceptions): # make a copy
#             if isinstance(exception,(list,tuple)):
#                 for exc in exception:
#                     exceptions.add(exc)
#                 exceptions.remove(exception)
#                 
#         self.exceptions = tuple(exceptions)
#     def __enter__(self):
#         return self
#     def __exit__(self, exc_type, exc_val, traceback):
#         # Did it raise the right exceptions
#         if exc_type in self.exceptions:
#             return True
#         if exc_type is None: 
#             raise FailedToRaiseError

class CLI_Error(object):
    """
    mimic the CLI mode and capute stdout and stderr. See the attribute
    `exit_code` (which is 0 for success otherwise, depends on the exit)    
    """
    def __init__(self):
        self.exit_code = 0
    def __enter__(self):
        self._stdout = sys.stdout
        self._stderr = sys.stderr
        self._CLI_MODE = pyprepro.CLI_MODE
        self._DEBUGCLI = pyprepro.DEBUGCLI
        
        # Monkey patch
        pyprepro.DEBUGCLI = False
        pyprepro.CLI_MODE = True
        self.stdout_tmp = sys.stdout = StringIO()
        self.stderr_tmp = sys.stderr = StringIO()
        return self
    
    def __exit__(self, exc_type, exc_val, traceback):
        self.stdout_tmp.flush()
        self.stderr_tmp.flush()
        
        self.stdout = self.stdout_tmp.getvalue()
        self.stderr = self.stderr_tmp.getvalue()
        
        # Reverse all monkey patch
        pyprepro.CLI_MODE = self._CLI_MODE
        pyprepro.DEBUGCLI = self._DEBUGCLI
        
        sys.stdout = self._stdout
        sys.stderr = self._stderr
        
        # Catch *just* SystemExit
        if isinstance(exc_val,SystemExit):
            self.exit_code = exc_val.code
            return True
        
        

################
import pyprepro

from pyprepro import ImmutableValDict,Immutable,Mutable
from pyprepro import _preparser
from pyprepro import _formatter
from pyprepro import EmptyInlineError

####################

OUTDIR = os.path.join(os.path.dirname(__file__),'test_output')
try:
    shutil.rmtree(OUTDIR)
except OSError:
    pass

try:
    os.makedirs(OUTDIR)
except OSError:
    pass

###################### HELPERS

def read(filename):
    with open(filename,'rt',encoding='utf8') as FF:
        return FF.read()

def compare_lines(A,B,verbose=False):
    """
    Compares the equality of lines with rstrip applied
    """
    A = A.strip()
    B = B.strip()
    for a,b in zip_longest(A.split('\n'),B.split('\n'),fillvalue=''):
        if len(a.strip()) == 0 and len(b.strip()) == 0:
            continue
        if a.rstrip() != b.rstrip():
            if verbose:
                print('a',a.rstrip())
                print('b',b.rstrip())                
            return False
    return True

def shsplit(*args):
    """
    Older python (2.6) requires bytes into shlex.split so enforce that
    """
    if sys.version_info < (2,7):
        args = [pyprepro._touni(a).encode('utf8') for a in args]
    return shlex.split(*args)
    
    
    

###################### /HELPERS

class Immutables(unittest.TestCase):
    
    def test_ImmutableValDict(self):
        """
        This test will check different properties of an ImmutableValDict
    
        Reminder: in python, "A is B" means they are the same object. == is equality.
        Example:   ` [1,2] == [1,2]` (True) but  ` [1,2] is [1,2]` (False)
        """
        mydict = ImmutableValDict()
    
        # These objects will be used as the values
        MM = [1,2,3]
        II = ['a','b','c']
    
        mydict['MM1'] = MM
        mydict['MM2'] = Mutable(MM)
        mydict['II'] = Immutable(II)
    
        # Make sure the entries not ONLY equal, but are the SAME object (even though
        # wrapped in a (Im)mutable
        self.assertTrue(mydict['MM1'] is MM,msg="M1 changed object")
        self.assertTrue(mydict['MM2'] is MM,msg="M2 changed object (called as Mutable)")
        self.assertTrue(mydict['II'] is II,msg="II changed object (called as Immutable)")
    
        # Make sure II is in immutable
        self.assertTrue('II' in mydict.immutables)
    
        # Make sure we cannot change II but (can *change* MM)
        mydict['II'] = 'new I'
        mydict['MM1'] = 'new M'
    
        self.assertTrue(mydict['II'] != 'new I' and mydict['II'] is II)
        self.assertTrue(mydict['MM1'] == 'new M' and mydict['MM1'] is not MM)
    
        # Show that even though the object can't be changed, if the python object
        # itself is mutable (as a list is), it can be modified
        N0 = len(mydict['II'])
        self.assertTrue(N0 == 3)
        mydict['II'].append('d')
        N1 = len(mydict['II'])
        self.assertTrue(N1 == 4)
        self.assertTrue(mydict['II'] is II)
        self.assertTrue(mydict['II'] == ['a','b','c','d'])

        # Show that using update keeps mutability of both
        mydict2 = ImmutableValDict()
        mydict2['ii'] = Immutable('ii')
        mydict2['mm'] = 'mm'
        mydict2['II'] = 'NEW II'
    
        mydict.update(mydict2)
        self.assertTrue(len(mydict) == 5) # since 'II' was repeated, != 6
        self.assertTrue(mydict['II'] is II, "II changed...")
    
        mydict['ii'] = 100
        self.assertTrue(mydict['ii'] != 100 and mydict['ii'] == 'ii')
    
        self.assertTrue(mydict.immutables == set(['II','ii']))

        # Make sure when you delete an item, it gets removed
        self.assertTrue('ii' in mydict.immutables and mydict['ii'] == 'ii')
        del mydict['ii']
        self.assertTrue('ii' not in mydict) # make sure was deleted
        self.assertTrue('ii' not in mydict.immutables) # make sure not still immutable
        self.assertTrue(mydict.immutables == set(['II']))
        
        # Show that you can convert it to immutable
        mydict['II'] = Mutable(mydict['II'])
        self.assertTrue('II' not in mydict.immutables)
        mydict['II'] = 'new I'
        self.assertTrue(mydict['II'] == 'new I' and mydict['II'] is not II)

        # test creating a new dict from an ImmutableValDict -- when passed as an ARG
        olddict = ImmutableValDict()
        olddict['I'] = Immutable(1)
        olddict['M'] = 2
    
        newdict = ImmutableValDict(olddict)
        self.assertTrue(newdict.immutables == set(['I']))
        newdict['I']*= 10
        self.assertTrue(newdict['I'] == 1)
    
        # Show that this fails if not passed as an arg
        newdict = ImmutableValDict(**olddict)
        self.assertTrue(newdict.immutables != set(['I']))
    
        # FINALLY, now show that these properties are maintained when 
        # using an ImmutableValDict inside exec
        env = ImmutableValDict()
        env['Immutable'] = Immutable
        env['Mutable'] = Mutable
        env['outI'] = Immutable('i')
        env['outM'] = 'm'
        
        env['assertTrue'] = self.assertTrue         
        exec_test = """\
            
            assertTrue(outI == 'i')
            assertTrue(outM == 'm')
        
            outI == 'III'
            outM = 'MMM'
        
            assertTrue(outI == 'i')
            assertTrue(outM == 'MMM')
        
            inI = Immutable('I')
            inM = 'M'
        
            inI = 'ii'
            inM = 'mm'
        
            assertTrue(inI == 'I')  # not 'ii'
            assertTrue(inM == 'mm')
        
            im2 = Immutable('a')
            del im2
        """
        exec_test = '\n'.join(ll.lstrip() for ll in exec_test.split('\n')) # Remove leading spaces
        exec(exec_test,env)
    
        # Make sure those are set here
        self.assertTrue('im2' not in env.immutables, "im2 did not get removed")
        self.assertTrue('inI' in env.immutables)
        self.assertTrue(env['inI'] == 'I' )
        env['inI'] = 'iiiiiii'
        self.assertTrue(env['inI'] == 'I' )
    
    def test_immutable(self):
        """
        This tests reading and redefining variables inline, and ensuring spacing
        is maintained
        """
    
        input = """\
        % A = 1
        A = {A = Immutable(A)}
        {A = A + 1},{A + 1},{A = 10}
        % A = Mutable(A)
        {A = A + 1},{A + 1},{A = 10}
        % B = Immutable(100)
        B = {B = 5}
        % del B
        B = {B = 5}
        L = {L = Immutable([1,2,3])}
        L = {L = [3,2,1]}
        % L.append(30)
        {L}
        """
        gold = """\
        A = 1
        1,2,1
        2,3,10
        B = 100
        B = 5
        L = [1, 2, 3]
        L = [1, 2, 3]
        [1, 2, 3, 30]
        """
        output = pyprepro.pyprepro(input)
        self.assertTrue(compare_lines(output,gold)   )

    def test_mutable_overset(self):
        """
        Test that setting an immutable as immutable *still* does not overwrite
        """
        input = """\
        {param = Immutable(0)}  0
        {param = 1}  0
        {param = Mutable(2)}  2
        {param = 3}  3
        {param = Immutable(4)}  4
        {param = 5}  4
        {param = Immutable(Mutable(6))}  4
        {param = 7}  4
        {param = Mutable(Immutable(8))}  8
        {param = 9}  8
        {param}  8"""
        output = pyprepro.pyprepro(input)
        for line in output.strip().split('\n'):
            a,b = line.split()
            self.assertTrue(a == b)

    def test_include_statements(self):
        """
        test include statements from here
        """
    
        input = """\
        {% 
            I = Immutable(1)
            M = 3
        %}
        % include('test_files/inline_include.inp')
    
        ii:{ii}, ii = ii + 1: {ii = ii+1}
        mm:{mm}, mm = mm + 1: {mm = mm+1}
        """
        

        gold = """\
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= in template
Paramaters defined outside. Test immutability too
I: 1; I+1: 2; I = 2*I: 1
M: 3; M+1: 4; M = 2*M: 6

Define parameters here
3.141592654
1.570796327

        ii:3.141592654, ii = ii + 1: 3.141592654
        mm:1.570796327, mm = mm + 1: 2.570796327"""
    
        output = pyprepro.pyprepro(input)

        self.assertTrue(compare_lines(output,gold)   )
    
        ###################
        # Test with depth and from CLI
        cmd = ['test_files/include_test2_0.inp','test_output/include_test2_0.out']
        pyprepro._pyprepro_cli(cmd)
        self.assertTrue(compare_lines(\
            read('test_output/include_test2_0.out'),
            read('test_gold/include_test2.gold')
        ))
    
    def test_bad_include_statement(self):
        """
        Tests when using the incorrect syntax
        """
        input = """
        {I = Immutable(1)}
        {M = 3}
        {include('test_files/inline_include.inp')}
        """
        self.assertRaises(pyprepro.IncludeSyntaxError,pyprepro.pyprepro,input)



class unicode_and_encoding(unittest.TestCase):

    def test_unicode(self):
        """
        Test a file with lots of unicode
        """
        cmd = "test_files/unicode.inp test_output/unicode.out"

        gold=u"""\
    Testing unicode:
    °
    ☀ ☁ ☂ ☃ ☄ ★ ☆ ☇ ☈ ☉ ☊ ☋ ☌ ☍ ☎ ☏ ☐ ☑ ☒ ☓ ☚ ☛ ☜ ☝ ☞ ☟ ☠ ☡ ☢ ☣ ☤ ☥ ☦ ☧ ☨ ☩ ☪ ☫ ☬ ☭ ☮ ☯ ☰ ☱ ☲ ☳ ☴ ☵

    ¡ ¢ £ ¤ ¥ ¦ § ¨ © ª « ¬ ­ ® ¯ ° ± ² ³ ´ µ ¶ · ¸ ¹ º » ¼ ½ ¾ ¿ À Á Â Ã Ä Å Æ Ç È É Ê Ë Ì Í Î Ï Ð Ñ Ò Ó Ô Õ Ö × Ø Ù Ú Û Ü Ý Þ ß à á â ã ä å æ ç è é ê ë ì í î ï ð ñ ò ó ô õ ö ÷ ø ù ú û ü ý þ ÿ
    """
        pyprepro._pyprepro_cli(shsplit(cmd))

        output = read('test_output/unicode.out')
        self.assertTrue(compare_lines(output,gold)   )

    def test_windows_encodings(self):
        """
        Test using different encoding to represent unicode both in the file and the variable
    
        This tests more than just windows but gets the main ones.
        """
        testtxt = u"{A = '°·è'} -- § ¨ © ª « -- {A}"
        gold = u'°·è -- § ¨ © ª « -- °·è'
        encodings = ['utf8','Windows-1252']
        for encoding in encodings:
            self.assertTrue(pyprepro.pyprepro(testtxt.encode(encoding)) == gold)
    
        for filename in ['windows_ansi','windows_unicode','windows_unicode2']:    
            self.assertTrue(compare_lines( pyprepro.pyprepro('test_files/{0}.txt'.format(filename)),
                           read('test_gold/{0}.gold'.format(filename))))
    
class invocation_and_options(unittest.TestCase):

    def test_cli_include(self):
        """
        Tests that variables in a `--include FILE` statement are defined at all,
        defined as immutable, and NOT printed
    
        See the
            test_files/cli_include_main.inp
            test_files/cli_include_inc.inp
        files.
        """
        cmd = "--var cparam=10 --include test_files/cli_include_inc.inp test_files/cli_include_main.inp test_output/cli_include.out"
    
        pyprepro._pyprepro_cli(shsplit(cmd))

        out = read('test_output/cli_include.out')
        gold = read('test_gold/cli_include.gold')
        self.assertTrue(compare_lines(out,gold))
    
    def test_CLI_vars(self):
        """
        Test variables defined at the CLI and make sure they are 
        correctly set as immutables
        """
        # Clean
        cmd = 'test_files/cli_var.inp test_output/cli_var.inp.0'
        pyprepro._pyprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(\
                read('test_output/cli_var.inp.0'),
                read('test_gold/cli_var.gold.0')))
    
        # Set the first one (A)
        # The rest should follow since A is immutable
        cmd = '--var "A=1.2" test_files/cli_var.inp test_output/cli_var.inp.1'
        pyprepro._pyprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(\
                read('test_output/cli_var.inp.1'),
                read('test_gold/cli_var.gold.1')))
            
        # Set C. A & B should still be the original (1.5) but from C down, 
        # it should be based on that.
        # Also test with more spaces
        cmd = '--var "C = 1.2" test_files/cli_var.inp test_output/cli_var.inp.2'
        pyprepro._pyprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(\
                read('test_output/cli_var.inp.2'),
                read('test_gold/cli_var.gold.2')))
    
        # Set "other" to an int
        cmd = '--var "other = 13" test_files/cli_var.inp test_output/cli_var.inp.3'
        pyprepro._pyprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(\
                read('test_output/cli_var.inp.3'),
                read('test_gold/cli_var.gold.3')))
    
        # set "other"  to a string
        # NOTICE: you do not quote it. As it is bash, it is assumed to be a string
        cmd = '--var "other = text" test_files/cli_var.inp test_output/cli_var.inp.4'
        pyprepro._pyprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(\
                read('test_output/cli_var.inp.4'),
                read('test_gold/cli_var.gold.4')))

    def test_cli_no_infile(self):
        cmd = 'fake_file_' + ''.join(random.choice('123456789') for _ in range(10))
        with CLI_Error() as E:
            pyprepro._pyprepro_cli(shsplit(cmd))
        self.assertTrue(E.exit_code != 0)
        self.assertTrue(E.stderr.strip() == 'ERROR: `infile` must be a file or `-` to read from stdin')
            
    def test_change_code_delims(self):
        """
        Test changing the code delimiters. Also test when you do *not* do it
        """
        # This no longer works with setting the code-blocks. That is fine. It is
        # implicitly tested via the "right" tests
    #     wcmd = 'test_files/new_delim.inp test_output/new_delim_wrong.out'
    #     pyprepro._pyprepro_cli(shsplit(wcmd))
    #     assert compare_lines(
    #             read('test_output/new_delim_wrong.out'),
    #             read('test_gold/new_delim_wrong.gold'))

        ccmd = '--inline "[[ ]]" --code "$" --code-block "<{* *>}" test_files/new_delim.inp test_output/new_delim_right.out'
        pyprepro._pyprepro_cli(shsplit(ccmd))
        self.assertTrue(compare_lines(\
                read('test_output/new_delim_right.out'),
                read('test_gold/new_delim_right.gold')))

    def test_json_python(self):
        """
        Tests the --json-include and --python-include of CLI for dprepro and pyprepro plus
        the pyprepro function call. Also test with multiple JSONs to address a prior bug
        """ 
        GOLD = read('test_gold/json_include.gold')
    
        cmd = ('--json-include test_files/json_include_params.json '
               '--python-include test_files/python_include_params.py '
               '--json-include test_files/json_include_params2.json '
               'test_files/json_include.inp test_output/json_include0.inp')
        pyprepro._pyprepro_cli(shsplit(cmd))
        
        # CLI
        self.assertTrue(compare_lines(GOLD,read('test_output/json_include0.inp')))
        
        # Function
        self.assertTrue(compare_lines(GOLD,pyprepro.pyprepro('test_files/json_include.inp',
                                                          json_include=['test_files/json_include_params.json',
                                                                        'test_files/json_include_params2.json'],
                                                          python_include='test_files/python_include_params.py')))
    
        cmd = ('--no-warn '
               '--json-include test_files/json_include_params.json '
               '--python-include test_files/python_include_params.py '
               '--json-include test_files/json_include_params2.json '
               'test_files/dakota_aprepro.1 ' # Needs something. This doesn't matter
               'test_files/json_include.inp '
               'test_output/json_include1.inp')
        pyprepro._dprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(GOLD,read('test_output/json_include1.inp')))


    def test_dakota_include(self):
        """
        Tests the new --dakota-include syntax even when *NOT* using the
        default stylings (of aprepro)
        """
        tpl = """
        x = [[x = 5]]
        {{%
            i1x_y = 99
            tester = True
        %}}
        Tester: [[tester]]
        1x:y: [[i1x_y]]
        """
        gold = """
        x = 0.08889860404
        Tester: True
        1x:y: 3.046260756
        """
    
        res1 = pyprepro.pyprepro(tpl,dakota_include='test_files/dakota_aprepro.1',
                                 code_block='{{% %}}',inline='[[ ]]',warn=False)
        res2 = pyprepro.pyprepro(tpl,dakota_include='test_files/dakota_default.1',
                                 code_block='{{% %}}',inline='[[ ]]',warn=False)
        self.assertTrue(res1 == res2)
        self.assertTrue(compare_lines(res1,gold))
        
        with open('test_output/tmp.tpl','wt') as F:
            F.write(tpl)
        
        cmd = ['--no-warn',
               '--code-block','{{% %}}',
               '--inline','[[ ]]',
               '--dakota-include','test_files/dakota_aprepro.1',
               'test_output/tmp.tpl','test_output/dakota_include.out']
        pyprepro._pyprepro_cli(cmd)
        compare_lines(gold,read('test_output/dakota_include.out'))
        
        cmd = ['--no-warn',
               '--code-block','{{% %}}',
               '--inline','[[ ]]',
               '--dakota-include','test_files/dakota_default.1',
               'test_output/tmp.tpl','test_output/dakota_include.out']
        pyprepro._pyprepro_cli(cmd)
        compare_lines(gold,read('test_output/dakota_include.out'))
        
    def test_function_env_immutable_env(self):
        tpl = """
        {a = 1}
        {b = 2}
        """
        env = {'a':Immutable(5),'b':100}
        
        res = pyprepro.pyprepro(tpl)
        res = [r.strip() for r in res.split('\n') if r.strip()]
        self.assertTrue(res == ['1','2'])
        
        res = pyprepro.pyprepro(tpl,env=env)
        res = [r.strip() for r in res.split('\n') if r.strip()]
        self.assertTrue(res == ['5','2'])
        
        res = pyprepro.pyprepro(tpl,immutable_env=env)
        res = [r.strip() for r in res.split('\n') if r.strip()]
        self.assertTrue(res == ['5','100'])
        
        pyprepro.pyprepro(tpl,immutable_env=env,output='test_output/funout.inp')
        
        with open('test_output/funout.inp') as f:
            res = f.read()
        res = [r.strip() for r in res.split('\n') if r.strip()]
        self.assertTrue(res == ['5','100'])
    
    def test_include_search_path(self):
        gold = dedent("""\
            Top Level!
            First Include (in subdir)
            Second Include (in subdir)
            Third Include (in subdir)
            Third Include (in top)
            Fourth include
        """)
        res = pyprepro.pyprepro('test_files/test_subdir_includes.inp')
        self.assertTrue(compare_lines(res,gold))

    def test_dunder_file_dir(self):
        res = pyprepro.pyprepro('test_files/dunder_file_dir/demo.tpl')
        lines = res.splitlines()
               
        self.assertTrue('foo bar' in lines) # declared variables in the includes
        self.assertTrue('fizz buzz' in lines)
        
        # Create a dict of paths
        paths = {}
        for line in lines:
            if ':' not in line:
                continue
            key,val = line.split(':',1)
            paths[key.strip()] = val.strip()
        
        # Make sure they get reset
        self.assertTrue(paths['Demo0 file'] == paths['Demo1 file'],"demo file")
        self.assertTrue(paths['Demo0 dir']  == paths['Demo1 dir'],"demo dir")
        self.assertTrue(paths['first0 file'] == paths['first1 file'],"first file")
        self.assertTrue(paths['first0 dir'] == paths['first1 dir'],"first dir")
        
        # make sure they get set properly
        self.assertTrue(os.path.dirname(paths['Demo0 file']) == paths['Demo0 dir'])
        self.assertTrue(os.path.dirname(paths['first0 file']) == paths['first0 dir'])
        self.assertTrue(os.path.dirname(paths['second0 file']) == paths['second0 dir'])
    
        self.assertTrue(os.path.join(paths['Demo0 dir'],'first') == paths['first0 dir'])
        self.assertTrue(os.path.join(paths['Demo0 dir'],'first','second') == paths['second0 dir'])
        self.assertTrue(os.path.join(paths['first0 dir'],'second') == paths['second0 dir']) # already checked by associative property but this is more clear
        
class preparser_edge_cases(unittest.TestCase):
    def test_escaping(self):
        """
        Test the different escape methods
        """
    
        input = r"""
        %inline_escape = 'Uh Oh'
        \{inline_escape\} <--- No variable definition
        \{inline_escape = 100\} <--- with variable definition (gets processed differently)

        \% code line
        \{%
            code block
        \%}
        {inline_escape} # should read 'Uh Oh'
        """
    
        gold = """
        {inline_escape} <--- No variable definition
        {inline_escape = 100} <--- with variable definition (gets processed differently)

        % code line
        {%
            code block
        %}
        Uh Oh # should read 'Uh Oh'
        """
    
        output = pyprepro.pyprepro(input)
        self.assertTrue(compare_lines(output,gold))
    
        # More single line tests
        
        # just for speed
        p = pyprepro.pyprepro       
        c = lambda a,b: self.assertTrue(compare_lines(a,b))
        
        # Reminder that r means string literal. So r'\' is '\\' but this is NOT needed in read-files
        c( p(r'{a=5}')      ,r'5')
        c( p(r'\{a=5}')     ,r'{a=5}')
        c( p(r'\{a=5\}')    ,r'{a=5}')
        c( p(r'\\{a=5}')    ,r'\{a=5}')
        c( p(r'\\{a=5\}')   ,r'\{a=5}')
        c( p(r'\\{a=5\\}')  ,r'\{a=5\}')
    
        # test with different delimiters
        c( p(r'[b=2$',inline='[ $'),'2')
        c( p(r'\[b=2$',inline='[ $'),'[b=2$')
        c( p(r'\[b=2\$',inline='[ $'),'[b=2$')
        c( p(r'\\[b=2$',inline='[ $'), r'\[b=2$')
        c( p(r'\\[b=2\$',inline='[ $'), r'\[b=2$')
        c( p(r'\\[b=2\\$',inline='[ $'), r'\[b=2\$')
        
        # Test with nested
        IN = r"""
# Several on a line, some escaped
{ alpha = 0.1 } then \{ beta = alpha \} then { beta = 2.0*alpha }
 
# Nested with escapes
{ alpha = 0.1 } then \{ beta = alpha \} then \{ { beta = 2.0*alpha } \} {beta}"""
        
        GOLD = """
# Several on a line, some escaped
0.1 then { beta = alpha } then 0.2

# Nested with escapes
0.1 then { beta = alpha } then { 0.2 } 0.2"""
        
        c(p(IN),GOLD)
        
    
    def test_multiple_inline_variables(self):
        """
        This tests reading and redefining variables inline, and ensuring spacing
        is maintained
        """
    
        input = """\
        % p = 1
        {p}
        {p = p + 1}
        START,{p = p + 1},{p = p + 1},{p = p + 1},{p = p + 1},{p = p + 1},END
        {p}
        """
        gold = """\
        1
        2
        START,3,4,5,6,7,END
        7
        """
        output = pyprepro.pyprepro(input)
        self.assertTrue(compare_lines(output,gold))

    def test_inline_dict_assignment(self):
        """
        Test for assigning dictionaries.
    
        Note that python2 and 3 may differ in how they present the results due to
        leading `u`. Therefore we test this by removing a lot of characters
    
        NOTE: This behavior may be dissallowed in the future.
        - [ ] Come back to this
        """
        input ="""\
            (( {'A':1,'B':2} ))
            (( mydict={'A':1,'B':2} ))
            ((mydict))
            ((mydict['A']))
            ((mydict['B']))
            """
        out = pyprepro.pyprepro(input,inline="(( ))")
        gold = "    {'A': 1, 'B': 2}\n    {'A': 1, 'B': 2}\n    {'A': 1, 'B': 2}\n    1\n    2\n    "
    
        def _clean(t):
            t = t.replace('u','')
            t = t.replace('"','')
            t = t.replace("'",'')
            t = t.replace('\n','')
            t = t.replace(' ','')
            return t
    
        self.assertTrue(_clean(out) == _clean(gold))

    def test_inline_comparison_and_assignment(self):
        """
        Tests for things like:
            { A += 1 } # Assignment
            { A <= 1 } # comparison
        and also test how this handles quotes and '=' in a function call
        """
        cmd = ['test_files/inline_comparison_and_assignment_colons.inp',
               'test_output/inline_comparison_and_assignment_colons.out']
        pyprepro._pyprepro_cli(cmd)
        self.assertTrue(compare_lines(\
            read('test_output/inline_comparison_and_assignment_colons.out'),
            read('test_gold/inline_comparison_and_assignment_colons.gold')))   
    
    def test_py38_walrus(self):
        if sys.version_info < (3,8):
            return
        self.assertTrue(pyprepro.pyprepro('A = {A:= 10}; {A}') == 'A = 10; 10')
    
    def test_parse_inline_assignment_preparser(self):
        """
        Tests *just* the preparser including regressions
        """
    
        ## Regression
        # The following was found on 2018-01-08. A code block regex is greedy in 
        # certain cases
    
        # This works
        IN = """\
{% A = 10 %}
{ A = 1 }
{% A += 4 %}"""
        GOLD = """\
{% A = 10 %}
\\\\
{% A= 1 %}
{ A }
{% A += 4 %}"""
        syntax = {'code':'%','inline':'{ }','code_block': '{% %}'}
        self.assertTrue(compare_lines(pyprepro._preparser(IN,syntax),GOLD))


        # This tests the regression
        IN = """\
{% A = 10 %}
{ A = 1 }
{% A += 4 
%}"""
    
        GOLD = """\
{% A = 10 %}
\\\\
{% A= 1 %}
{ A }
{% A += 4
%}"""
        self.assertTrue(compare_lines(pyprepro._preparser(IN,syntax),GOLD))
        
    def test_pathological_quotes(self):
        """
        Tests some pathological quoting cases. The relate to OUTER quotes
        """    
    
        # With matched outer quotes
        IN = """ "{A = 1}" {A} """ # The OUTER quotes are the problem
        OUT = pyprepro.pyprepro(IN)
        GOLD = """ "1" 1 """
        self.assertTrue(OUT == GOLD)
    
        # with unmatched outer quotes
        IN = '"{A=1}{A}'
        OUT = pyprepro.pyprepro(IN)
        GOLD = '"11'
        self.assertTrue(OUT == GOLD)
    
        # This fails as per the docs...See "Edge Case: Delimiters inside Python quotes" in manual
        IN = r"""'{A="\}"}'"""
        OUT = pyprepro.pyprepro(IN)
        GOLD = """'}'"""
        self.assertFalse(OUT == GOLD)
        
        # ...so do this as per the docs
        
        IN = r"""% A = '}'N'{A}'""".replace('N','\n')
        OUT = pyprepro.pyprepro(IN)
        GOLD = """'}'"""
        self.assertTrue(OUT == GOLD)

        # With UNmatched inner quotes. We still expect this to fail since it isn't
        self.assertRaises(SyntaxError,pyprepro.pyprepro,'{A = "}adshd}')

        # Handle Empty quotes with proceeded quoted variable cause error
        # Regression for 18 (but fixed with 16)
        IN = """\
            { var1 = '' }
            { var2 = 'this is var2' }
        """
        GOLD = """this is var2"""
        self.assertTrue(compare_lines(pyprepro.pyprepro(IN),GOLD))


    def test_empty_inline(self):
        """
        Tests that empty inline raise an error that is helpful
        """
        # This *should* work
        pyprepro.pyprepro('A = {""}')
        
        # These should not:
        self.assertRaises(EmptyInlineError,pyprepro.pyprepro,'A = {}')
        self.assertRaises(EmptyInlineError,pyprepro.pyprepro,'A = [[ ]]',inline='[[ ]]')
        
        self.assertTrue(pyprepro.pyprepro('A = [[ ]]') == 'A = [[ ]]') # Just to make sure that otherwise passes

        cmd = 'test_files/empty_inline.inp'
        with CLI_Error() as E:
            pyprepro._pyprepro_cli(shsplit(cmd))
        self.assertTrue(E.exit_code != 0)
        self.assertTrue('EmptyInlineError' in E.stderr)
        self.assertTrue('Empty inline expression' in E.stderr)

    def test_unclosed_quotes(self):
        """Regression for #16 """
        
        input ="""\
            {%
            # Bob's idea
            %}
            bob's: { bob = 1.0 }
        """
        gold = """\
            bob's: 1
        """
        self.assertTrue(compare_lines(pyprepro.pyprepro(input),gold))
 
        # EDGE CASE: Ideally this would be assertTrue but at least documented. See "Edge Case: Delimiters inside Python quotes" in manual
        self.assertFalse(pyprepro.pyprepro("{ text = 'inside \} quote' }") == 'inside } quote')
 
class dakota_dprepro(unittest.TestCase):
    def test_dakota_param_names(self):
        """
        This tests when there are colons in the param names as can happen from
        Dakota
        """
        ####### Colons
    
        input ="""\
            { ASV_1:fun1 = 1}
            { ASV_1_fun1 }
            { "ASV_1:fun1" }
        """
        gold = """\
            1
            1
            ASV_1:fun1
        """
        self.assertTrue(compare_lines(pyprepro.pyprepro(input),gold))
    
        input = """\
            { ASV_1:fun1 = 1}
            { ASV_1:fun1 }
        """
    
        self.assertRaises(SyntaxError,pyprepro.pyprepro,input)
    
        input = """\
            % ASV_1:fun1 = 1
            { ASV_1_fun1 }
        """
    
        # Should fail since you can only assign with : in inline block
        self.assertRaises((SyntaxError,NameError),pyprepro.pyprepro,input)
        # Note: We add NameError since later version of python take `ASV_1:fun1` 
        # to be variable annotation but it can't find fun1

        ########## Leading integers
        input ="""\
            { 5param = 10}
            { i5param }
            { "i5param" }
        """
        gold = """\
            10
            10
            i5param
        """
        self.assertTrue(compare_lines(pyprepro.pyprepro(input),gold))
    

        input = """\
            { 5param = 10}
            { 5param }
        """
    
        # Should fail since you can't with leading integer
        self.assertRaises(SyntaxError,pyprepro.pyprepro,input)

        input = """\
            % 5param = 1
            { i5param }
        """
    
        # Should fail since you can only assign with leadin int in inline block
        self.assertRaises(SyntaxError,pyprepro.pyprepro,input)

    def test_dakota_params_mode(self):
        """
        Test reading dakota params whether aprepro style or not.
        Also tests the dprepro mode of PARAMFILE INPUT OUTPUT
        """
        gold = read('test_gold/read_from_dakota.gold')
    
        # A note on this test. Inside the template file is "{várïåbłę}". If run
        # with python2, that variable name will be fixed via the dprepro parser
        # BUT there is no easy way to fix it within the "read_from_dakota.inp"
        # template. Therefore, we instead make a modify the template
        # ahead of time and store it in the output when using python2
      
        templatepath = 'test_files/read_from_dakota.inp'
        if sys.version_info[0] == 2:
            with open(templatepath,'rt',encoding='utf8') as F:
                temp = F.read().replace(u'{várïåbłę}','{variabe}')
            templatepath = 'test_output/read_from_dakota.inp'
            with open(templatepath,'wt',encoding='utf8') as F:
                F.write(temp)

        ###### NOTE: DPREPRO

        # dakota style with 3 inputs. NOTE: call dprepro
        cmd = ' --no-warn test_files/dakota_default.1 {0} test_output/dakota.1'.format(templatepath)
        pyprepro._dprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(read('test_output/dakota.1'),gold))
    
        # aprepo style with 3 inputs. NOTE: call dprepro
        cmd = '--no-warn test_files/dakota_aprepro.1 {0} test_output/dakota.2'.format(templatepath)
        pyprepro._dprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(read('test_output/dakota.2'),gold))
        
        # json style with 3 inputs. NOTE: call dprepro
        cmd = '--no-warn test_files/dakota_json.1 {0} test_output/dakota.3'.format(templatepath)
        pyprepro._dprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(read('test_output/dakota.3'),gold))

        # json style with 3 inputs. NOTE: call dprepro. Use --simple-parser
        cmd = '--simple-parser --no-warn test_files/dakota_json.1 {0} test_output/dakota.4'.format(templatepath)
        pyprepro._dprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(read('test_output/dakota.4'),gold))

    def test_malformed_dakota_and_simple(self):
        """
        Test using the --simple-parser including malformed Dakota.
        """
        gold = read('test_gold/read_from_dakota.gold')
    
        # see note above
        templatepath = 'test_files/read_from_dakota.inp'
        if sys.version_info[0] == 2:
            with open(templatepath,'rt',encoding='utf8') as F:
                temp = F.read().replace(u'{várïåbłę}','{variabe}')
            templatepath = 'test_output/read_from_dakota.inp'
            with open(templatepath,'wt',encoding='utf8') as F:
                F.write(temp)
        
        # dakota style using a file that would break the non dakota.interfacing parser
        
        cmd = ' --no-warn --simple-parser test_files/dakota_malformed_default.1 {0} test_output/dakota.5'.format(templatepath)
        pyprepro._dprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(read('test_output/dakota.5'),gold))
    
        # aprepo style with 3 inputs. NOTE: call dprepro
        cmd = '--no-warn --simple-parser test_files/dakota_malformed_aprepro.1 {0} test_output/dakota.6'.format(templatepath)
        pyprepro._dprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(read('test_output/dakota.6'),gold))
        

    def test_dakota_interfacing(self):
        """Test dakota.interfacing capabilities when it is present"""
        try:
            import dakota.interfacing as di
        except ImportError:
            sys.stderr.write("Skipping tests for dakota.interfacing because it could not be imported.\n")
            return
        gold = read('test_gold/dprepro_di.gold')
        cmd = '--no-warn test_files/dakota_default.1 test_files/dprepro_di.inp test_output/dakota.3'
        pyprepro._dprepro_cli(shsplit(cmd))
        self.assertTrue(compare_lines(read('test_output/dakota.3'),gold))



class special_functions(unittest.TestCase):
    def test_all_vars(self):
        """
        That showing all variables. And *only* those (since math is defined)
        """    
        input="""\
        {a = sin(pi/4)}
        % b = 'test'
        {b}
        {all_vars()}
        """
        res = pyprepro.pyprepro(input) 
    
        # do not compare since pprint (used in all_vars()) may not be consistent
        res = [line.strip() for line in res.split('\n') if len(line.strip())>0]
        D = eval(res[-1])
        self.assertTrue(['a','b'] == sorted(D.keys()))

    def test_vset(self):
        """Quick test of vset"""
        IN1 = """\
        // Defaults:
        // param1 = {param1 = 10.3 }
        % param2 = Immutable(-5)
        // param2 = {param2 = 9.9 }"""

        IN2 = """\
        // Defaults:
        // {vset('param1',10.3)}
        % param2 = Immutable(-5)
        // {vset('param2',9.9)}"""

        GOLD = """\
        // Defaults:
        // param1 = 10.3
        // param2 = -5"""
    
        OUT1 = pyprepro.pyprepro(IN1)
        OUT2 = pyprepro.pyprepro(IN2)
    
        self.assertTrue(compare_lines(OUT1,GOLD))
        self.assertTrue(compare_lines(OUT2,GOLD)) # by transitive property, OUT1 == OUT2


class misc(unittest.TestCase):

    def test_formatting(self):
        """
        Test first using string formatting to control the output
        then setting a format
        """
    
        input = """\
        {pi}
        {'%9.2e' % pi}
        {'{0:9.2e}'.format(pi)}
        """
        gold1 = """\
        3.141592654
         3.14e+00
         3.14e+00
        """
        gold2 = """\
        3.1
         3.14e+00
         3.14e+00
        """
        output1 = pyprepro.pyprepro(input)
        output2 = pyprepro.pyprepro(input,fmt='%3.1f')
        output3 = pyprepro.pyprepro(input,fmt='{0:3.1f}')
    
        self.assertTrue(compare_lines(output1,gold1))
        self.assertTrue(compare_lines(output2,gold2))
        self.assertTrue(compare_lines(output3,gold2))
        
        try:
            import numpy as np
            res = pyprepro.pyprepro('num = {num}',env={'num':np.float64(10.2)})
            self.assertTrue(compare_lines(res,'num = 10.2'))
        except ImportError:
            sys.stderr.write("Skipping tests for np.float64 formatting since cannot find NumPy\n")

    def test_white_space(self):
        """
        Check that trialing \\ before a code block works as expected
        """
        # Test via the command line since python (not pyprepro) gets wonky around
        # some of the strings
        cmd = ' --no-warn test_files/white_space.inp test_output/white_space.out'
        pyprepro._pyprepro_cli(shsplit(cmd))
        
        self.assertTrue(compare_lines(read('test_output/white_space.out'),
                                   read('test_gold/white_space.gold')))
    
    def test_division(self):
        """
        Make sure 3/2 == 1.5
        """
        self.assertTrue(pyprepro.pyprepro('{3/2}') == '1.5')

    def test_non_templated(self):
        """
        Test when the input has no templating
            1: String inpu
            2: File input
        """
    
        self.assertTrue(pyprepro.pyprepro('test nothing').strip() == 'test nothing')
    
    def test_data_exfiltration(self):
    
        tpl = dedent("""\
            param1 = {param1}
            param2 = {param2 = 20}
            param3 = {param3 = 30}
            
            % with open('test_output/exfilt.json','wt') as fp:
            %   fp.write(json_dumps(indent=1))
            % end
            """
            )
        with open('test_output/exfilt.inp','wt') as fp:
            fp.write(tpl)
        
        # Test on the CLI
        cmd = 'test_output/exfilt.inp test_output/exfilt.out --var "param1 = 1" --var "param2 = 2"'
        pyprepro._pyprepro_cli(shsplit(cmd))
        
        self.assertTrue(read('test_output/exfilt.out').strip() == \
                        'param1 = 1\nparam2 = 2\nparam3 = 30')
        
        with open('test_output/exfilt.json') as fp:
            exfilt = json.load(fp)            
        
        # These should be there but we don't care about the value
        self.assertTrue(exfilt.pop('fp',None))
        
        # Make sure it got written
        self.assertTrue(exfilt == {'param1': 1, 'param2': 2, 'param3': 30})
        
        ## Test the module and render
        exfilt2 = pyprepro.render(
            tpl,
            immutable_env=dict(param1=1,param2=2),
        )
        self.assertTrue(exfilt2.pop('fp',None))
        self.assertTrue(exfilt2 == {'param1': 1, 'param2': 2, 'param3': 30})
        assert not isinstance(exfilt2,ImmutableValDict)
        
        exfilt3 = pyprepro.render(
            tpl,
            immutable_env=dict(param1=1,param2=2),
            keep_immutable=True,
        )
        assert isinstance(exfilt3,ImmutableValDict)
    
        # Related to #13. Special terms should render them
        exfilt4 = pyprepro.render("{gamma = 2}\n{cos = 4}")
        self.assertTrue(exfilt4 == {'cos': 4, 'gamma': 2})

        exfilt4 = pyprepro.render("empty template", immutable_env={"gamma": 2, "cos": 4})
        self.assertTrue(exfilt4 == {'cos': 4, 'gamma': 2})

        #exfilt4 = pyprepro.render("{lambda = 123}\n")
        #self.assertTrue(exfilt4 == {'lambda': 123})

        empty = pyprepro.render("nothing")
        # assert len(empty) == 0, "Should render nothing" # Removed for now
        if not len(empty) == 0:
            sys.stderr.write("\n\nWARNING: render without templates should return nothing but failed\n\n")
            sys.stderr.flush()


        
    
class error_capture(unittest.TestCase):
    def test_name(self):
        """
        Test that it fails when presented with an undefined variable
        """
        input="""\
        {param1 = 10}
        {param2}
        """
        if sys.version_info < (2,7):
            print('Skipping error_capture tests for 2.6',file=sys.stderr)
            return

        with open('_testin.inp','w') as F:
            F.write(input)

        with CLI_Error() as E:
            pyprepro.pyprepro('_testin.inp')
        
        # make sure it failed
        self.assertTrue(E.exit_code != 0)
        stderr = pyprepro._touni(E.stderr)
        gold = '''
Error occurred
    Exception: NameError
    Message: name 'param2' is not defined
'''
        self.assertTrue(compare_lines(stderr,gold))
        
        # Also test when it is called
        with CLI_Error() as E2: 
            pyprepro.pyprepro("%include('_testin.inp')")
        self.assertTrue(E.exit_code != 0)
        self.assertTrue(compare_lines(pyprepro._touni(E2.stderr),gold))

        os.remove('_testin.inp')
        
        # Via module
        self.assertRaises(NameError,pyprepro.pyprepro,input) # Should fail
    
    def test_syntax(self):
        input="""\
        This should throw a SyntaxError on line 4

        Error Statement: {'{0}'.format('hi'}
        """
        if sys.version_info < (2,7):
            print('Skipping error_capture tests for 2.6',file=sys.stderr)
            return
        with open('_testin.inp','w') as F:
            F.write(input)

        with CLI_Error() as E:
            pyprepro.pyprepro('_testin.inp')
                    
        # make sure it failed
        self.assertTrue(E.exit_code != 0)
        stderr = pyprepro._touni(E.stderr)
        line_num_of_error = 4
        if not PY3:
            error_message = 'invalid syntax'
        elif sys.version_info >= (3,10):
            error_message = "'(' was never closed"
            line_num_of_error = 1
        else:
            error_message = 'unexpected EOF while parsing'
        gold = '''
Error occurred
    Exception: SyntaxError
    Filename: {}
    Approximate Line Number: {}
    Message: {}
'''.format(os.path.abspath('_testin.inp'), line_num_of_error, error_message)
        self.assertTrue(compare_lines(stderr,gold))
        
        # Also test when it is called
        with CLI_Error() as E2: 
            pyprepro.pyprepro("%include('_testin.inp')")
        self.assertTrue(E.exit_code != 0)
        self.assertTrue(compare_lines(pyprepro._touni(E2.stderr),gold))

        os.remove('_testin.inp')
        
        # Via module
        self.assertRaises(SyntaxError,pyprepro.pyprepro,input) # Should fail

class inline_spec(unittest.TestCase):
    def test_inline_spec(self):
        
        input0 = dedent("""\
        HEADER
        Specify < num = 3 >
        * numsq = num**2
        <T for i in range(4): T>
        i: <i>, <numsq>
        * end
        This should do {nothing} {{at all}}
        % and this should show
        """)
        
        gold = dedent("""
        Specify 3
        i: 0, 9
        i: 1, 9
        i: 2, 9
        i: 3, 9
        This should do {nothing} {{at all}}
        % and this should show
        """)
        
        # This is actually 75 tests of every possible combination
        comments =  '// ', '# ','% ', '$ ',''
        leadings = '','D','PY'
        spaceeqs = ' ',' = ','=','= ',' ='
        
        for comment,leading,spaceeq in product(comments,leadings,spaceeqs):
            fmt = dict(comment=comment,leading=leading,spaceeq=spaceeq)
            HEADER = dedent("""\
                {comment}{leading}PREPRO_CODE{spaceeq}*
                {comment}{leading}PREPRO_INLINE{spaceeq}< >
                {comment}{leading}PREPRO_CODE_BLOCK{spaceeq}<T T>""".format(**fmt))
            
            input = input0.replace('HEADER',HEADER)
            # Nothing specified
            self.assertTrue(compare_lines(pyprepro.pyprepro(input),gold))
            
            # Commands specified
            self.assertTrue(compare_lines(pyprepro.pyprepro(input, # Doesn't matter the settings
                                       inline='hasfd asfd',code='ran dom',code_block='bl ock'),gold))
            
            # Same thing set    
            self.assertTrue(compare_lines(pyprepro.pyprepro(input, # Doesn't matter the settings
                                       inline='< >',code='*',code_block='<T T>'),gold))
        
        # Test from a test file
        gold2 = dedent("""
        New val: 2 # 2
        New val: 3 # 3
        New Val 9 # 9""")

        pyprepro._pyprepro_cli(shsplit('test_files/spec_regular.inp test_output/spec_reg.inp'))
        self.assertTrue(compare_lines(gold2,read('test_output/spec_reg.inp')))

        pyprepro._pyprepro_cli(shsplit('test_files/spec_regular.inp test_output/spec_reg.inp --code "!" --inline "<< >>" --code-block "{{% %}}"'))
        self.assertTrue(compare_lines(gold2,read('test_output/spec_reg.inp')))      
    
    def test_inline_spec_nestedinclude(self):
        """
        This tests an include with its own inline spec going multiple levels
        """
        
        gold = dedent("""\
            @ 0, set val0 = 0 and val1 = 1
            Levels: {0: 100}
            ---
            @ 1, set val0 = 1 and val1 = 1
            Levels: {0: 100, 1: 101}
            ---
            @ 2, set val0 = 2 and val1 = 1
            Change immutable: val1 = 4
            Levels: {0: 100, 1: 101, 2: 102}
            +++
            @1 {0: 100, 1: 101, 2: 102}
            +++
            @0 {0: 100, 1: 101, 2: 102}""")
        
        pyprepro._pyprepro_cli(shsplit('test_files/spec_include0.inp test_output/spec_include.inp'))
        self.assertTrue(compare_lines(gold,read('test_output/spec_include.inp')))    

        # With a specified one. Make sure the syntax is changed
        gold2 = dedent("""\
            @ 0, set val0 = fixed 1 and val1 = 1
            Levels: {0: 100}
            ---
            @ 1, set val0 = fixed 1 and val1 = 1
            Levels: {0: 100, 1: 101}
            ---
            @ 2, set val0 = fixed 1 and val1 = 1
            Change immutable: val1 = 4
            Levels: {0: 100, 1: 101, 2: 102}
            +++
            @1 {0: 100, 1: 101, 2: 102}
            +++
            @0 {0: 100, 1: 101, 2: 102}""")
        pyprepro._pyprepro_cli(shsplit('--include test_files/spec_includeII.inp test_files/spec_include0.inp test_output/spec_include2.inp'))
        self.assertTrue(compare_lines(gold2,read('test_output/spec_include2.inp')))

class Regressions(unittest.TestCase):
    def test_same_inline_open_close(self):
        """
        Test things like --inline "@ @" where the open and close are the same.
        Reported on 2022-05-31 and fixed on 2022-06-01
        """
        
        ## Function call. This should be sufficient. No need to test CLI
        self.assertTrue(pyprepro.pyprepro("@ a @", inline="@ @", env={"a": 1}) == "1")
        self.assertTrue(pyprepro.pyprepro("@@ a @@", inline="@@ @@", env={"a": 1}) == "1")
        self.assertTrue(pyprepro.pyprepro("$ a $", inline="$ $", env={"a": 1}) == "1")
        self.assertTrue(pyprepro.pyprepro("$@$ a $@$", inline="$@$ $@$", env={"a": 1}) == "1")
        self.assertTrue(pyprepro.pyprepro("% a = 1\n@ a @", inline="@ @",) == "1")

        ## Inline
        self.assertTrue(pyprepro.pyprepro(dedent(
                    """
                    # PYPREPRO_INLINE @ @
                    % a = 1
                    @ a @""")).strip() == '1')

        
        
if __name__ == '__main__':
    unittest.main()


