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

sys.dont_write_bytecode = True

if sys.version_info >= (3,):
    unicode = str
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

################
import pyprepro

from pyprepro import ImmutableValDict,Immutable,Mutable
from pyprepro import _preparser
from pyprepro import _formatter

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

def compare_lines(A,B):
    """
    Compares the equality of lines with rstrip applied
    """
    for a,b in zip_longest(A.split('\n'),B.split('\n'),fillvalue=''):
        if len(a.strip()) == 0 and len(b.strip()) == 0:
            continue
        if a.rstrip() != b.rstrip():
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
        self.assert_(mydict['MM1'] is MM,msg="M1 changed object")
        self.assert_(mydict['MM2'] is MM,msg="M2 changed object (called as Mutable)")
        self.assert_(mydict['II'] is II,msg="II changed object (called as Immutable)")
    
        # Make sure II is in immutable
        self.assert_('II' in mydict.immutables)
    
        # Make sure we cannot change II but (can *change* MM)
        mydict['II'] = 'new I'
        mydict['MM1'] = 'new M'
    
        self.assert_(mydict['II'] != 'new I' and mydict['II'] is II)
        self.assert_(mydict['MM1'] == 'new M' and mydict['MM1'] is not MM)
    
        # Show that even though the object can't be changed, if the python object
        # itself is mutable (as a list is), it can be modified
        N0 = len(mydict['II'])
        self.assert_(N0 == 3)
        mydict['II'].append('d')
        N1 = len(mydict['II'])
        self.assert_(N1 == 4)
        self.assert_(mydict['II'] is II)
        self.assert_(mydict['II'] == ['a','b','c','d'])

        # Show that using update keeps mutability of both
        mydict2 = ImmutableValDict()
        mydict2['ii'] = Immutable('ii')
        mydict2['mm'] = 'mm'
        mydict2['II'] = 'NEW II'
    
        mydict.update(mydict2)
        self.assert_(len(mydict) == 5) # since 'II' was repeated, != 6
        self.assert_(mydict['II'] is II, "II changed...")
    
        mydict['ii'] = 100
        self.assert_(mydict['ii'] != 100 and mydict['ii'] == 'ii')
    
        self.assert_(mydict.immutables == set(['II','ii']))

        # Make sure when you delete an item, it gets removed
        self.assert_('ii' in mydict.immutables and mydict['ii'] == 'ii')
        del mydict['ii']
        self.assert_('ii' not in mydict) # make sure was deleted
        self.assert_('ii' not in mydict.immutables) # make sure not still immutable
        self.assert_(mydict.immutables == set(['II']))
        
        # Show that you can convert it to immutable
        mydict['II'] = Mutable(mydict['II'])
        self.assert_('II' not in mydict.immutables)
        mydict['II'] = 'new I'
        self.assert_(mydict['II'] == 'new I' and mydict['II'] is not II)

        # test creating a new dict from an ImmutableValDict -- when passed as an ARG
        olddict = ImmutableValDict()
        olddict['I'] = Immutable(1)
        olddict['M'] = 2
    
        newdict = ImmutableValDict(olddict)
        self.assert_(newdict.immutables == set(['I']))
        newdict['I']*= 10
        self.assert_(newdict['I'] == 1)
    
        # Show that this fails if not passed as an arg
        newdict = ImmutableValDict(**olddict)
        self.assert_(newdict.immutables != set(['I']))
    
        # FINALLY, now show that these properties are maintained when 
        # using an ImmutableValDict inside exec
        env = ImmutableValDict()
        env['Immutable'] = Immutable
        env['Mutable'] = Mutable
        env['outI'] = Immutable('i')
        env['outM'] = 'm'
        
        env['assert_'] = self.assert_         
        exec_test = """\
            
            assert_(outI == 'i')
            assert_(outM == 'm')
        
            outI == 'III'
            outM = 'MMM'
        
            assert_(outI == 'i')
            assert_(outM == 'MMM')
        
            inI = Immutable('I')
            inM = 'M'
        
            inI = 'ii'
            inM = 'mm'
        
            assert_(inI == 'I')  # not 'ii'
            assert_(inM == 'mm')
        
            im2 = Immutable('a')
            del im2
        """
        exec_test = '\n'.join(ll.lstrip() for ll in exec_test.split('\n')) # Remove leading spaces
        exec(exec_test,env)
    
        # Make sure those are set here
        self.assert_('im2' not in env.immutables, "im2 did not get removed")
        self.assert_('inI' in env.immutables)
        self.assert_(env['inI'] == 'I' )
        env['inI'] = 'iiiiiii'
        self.assert_(env['inI'] == 'I' )
    
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
        self.assert_(compare_lines(output,gold)   )

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

        self.assert_(compare_lines(output,gold)   )
    
        ###################
        # Test with depth and from CLI
        cmd = ['test_files/include_test2_0.inp','test_output/include_test2_0.out']
        pyprepro._pyprepro_cli(cmd)
        self.assert_(compare_lines(\
            read('test_output/include_test2_0.out'),
            read('test_gold/include_test2.gold')
        ))


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
        self.assert_(compare_lines(output,gold)   )

    def test_windows_encodings(self):
        """
        Test using different encoding to represent unicode both in the file and the variable
    
        This tests more than just windows but gets the main ones.
        """
        testtxt = u"{A = '°·è'} -- § ¨ © ª « -- {A}"
        gold = u'°·è -- § ¨ © ª « -- °·è'
        encodings = ['utf8','Windows-1252']
        for encoding in encodings:
            self.assert_(pyprepro.pyprepro(testtxt.encode(encoding)) == gold)
    
        for filename in ['windows_ansi','windows_unicode','windows_unicode2']:    
            self.assert_(compare_lines( pyprepro.pyprepro('test_files/{0}.txt'.format(filename)),
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
        self.assert_(compare_lines(out,gold))
    
    def test_CLI_vars(self):
        """
        Test variables defined at the CLI and make sure they are 
        correctly set as immutables
        """
        # Clean
        cmd = 'test_files/cli_var.inp test_output/cli_var.inp.0'
        pyprepro._pyprepro_cli(shsplit(cmd))
        self.assert_(compare_lines(\
                read('test_output/cli_var.inp.0'),
                read('test_gold/cli_var.gold.0')))
    
        # Set the first one (A)
        # The rest should follow since A is immutable
        cmd = '--var "A=1.2" test_files/cli_var.inp test_output/cli_var.inp.1'
        pyprepro._pyprepro_cli(shsplit(cmd))
        self.assert_(compare_lines(\
                read('test_output/cli_var.inp.1'),
                read('test_gold/cli_var.gold.1')))
            
        # Set C. A & B should still be the original (1.5) but from C down, 
        # it should be based on that.
        # Also test with more spaces
        cmd = '--var "C = 1.2" test_files/cli_var.inp test_output/cli_var.inp.2'
        pyprepro._pyprepro_cli(shsplit(cmd))
        self.assert_(compare_lines(\
                read('test_output/cli_var.inp.2'),
                read('test_gold/cli_var.gold.2')))
    
        # Set "other" to an int
        cmd = '--var "other = 13" test_files/cli_var.inp test_output/cli_var.inp.3'
        pyprepro._pyprepro_cli(shsplit(cmd))
        self.assert_(compare_lines(\
                read('test_output/cli_var.inp.3'),
                read('test_gold/cli_var.gold.3')))
    
        # set "other"  to a string
        # NOTICE: you do not quote it. As it is bash, it is assumed to be a string
        cmd = '--var "other = text" test_files/cli_var.inp test_output/cli_var.inp.4'
        pyprepro._pyprepro_cli(shsplit(cmd))
        self.assert_(compare_lines(\
                read('test_output/cli_var.inp.4'),
                read('test_gold/cli_var.gold.4')))

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
        self.assert_(compare_lines(\
                read('test_output/new_delim_right.out'),
                read('test_gold/new_delim_right.gold')))

    def test_json(self):
        """
        Tests the --json-include of CLI for dprepro and pyprepro plus
        the pyprepro function call
        """ 
        GOLD = read('test_gold/json_include.gold')
    
        cmd = '--json-include test_files/json_include_params.json test_files/json_include.inp test_output/json_include0.inp'
        pyprepro._pyprepro_cli(shsplit(cmd))
        self.assert_(compare_lines(GOLD,read('test_output/json_include0.inp')))
        self.assert_(compare_lines(GOLD,pyprepro.pyprepro('test_files/json_include.inp',json_include='test_files/json_include_params.json')))
    
        cmd = '--no-warn --json-include test_files/json_include_params.json test_files/dakota_aprepro.1 test_files/json_include.inp test_output/json_include1.inp'
        pyprepro._dprepro_cli(shsplit(cmd))
        self.assert_(compare_lines(GOLD,read('test_output/json_include1.inp')))

class preparser_edge_cases(unittest.TestCase):
    def test_escaping(self):
        """
        Test the different escape methods
        """
    
        input = """\
        %inline_escape = 'Uh Oh'
        \{inline_escape\} <--- No variable definition
        \{inline_escape = 100\} <--- with variable definition (gets processed differently)

        \% code line
        \{%
            code block
        \%}
        {inline_escape} # should read 'Uh Oh'
        """
    
        gold = """\
        {inline_escape} <--- No variable definition
        {inline_escape = 100} <--- with variable definition (gets processed differently)

        % code line
        {%
            code block
        %}
        Uh Oh # should read 'Uh Oh'
        """
    
        output = pyprepro.pyprepro(input)
        self.assert_(compare_lines(output,gold))
    
        # More single line tests
        
        # just for speed
        p = pyprepro.pyprepro       
        c = lambda a,b: self.assert_(compare_lines(a,b))
        
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
        c( p(r'\\[b=2$',inline='[ $'),'\[b=2$')
        c( p(r'\\[b=2\$',inline='[ $'),'\[b=2$')
        c( p(r'\\[b=2\\$',inline='[ $'),'\[b=2\$')
        
        # Test with nested
        IN = """\
# Several on a line, some escaped
{ alpha = 0.1 } then \{ beta = alpha \} then { beta = 2.0*alpha }
 
# Nested with escapes
{ alpha = 0.1 } then \{ beta = alpha \} then \{ { beta = 2.0*alpha } \} {beta}"""
        
        GOLD = """\
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
        self.assert_(compare_lines(output,gold))

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
    
        self.assert_(_clean(out) == _clean(gold))

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
        self.assert_(compare_lines(\
            read('test_output/inline_comparison_and_assignment_colons.out'),
            read('test_gold/inline_comparison_and_assignment_colons.gold')))   
    
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
        self.assert_(compare_lines(pyprepro._preparser(IN),GOLD))


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
        self.assert_(compare_lines(pyprepro._preparser(IN),GOLD))

        
    def test_pathological_quotes(self):
        """
        Tests some pathological quoting cases. The relate to OUTER quotes
        """    
    
        # With matched outer quotes
        IN = """ "{A = 1}" {A} """ # The OUTER quotes are the problem
        OUT = pyprepro.pyprepro(IN)
        GOLD = """ "1" 1 """
        self.assert_(OUT == GOLD)
    
        # with unmatched outer quotes
        IN = '"{A=1}{A}'
        OUT = pyprepro.pyprepro(IN)
        GOLD = '"11'
        self.assert_(OUT == GOLD)
    
        IN = """'{A="}"}'"""
        OUT = pyprepro.pyprepro(IN)
        GOLD = """'}'"""
        self.assert_(OUT == GOLD)

        # With UNmatched inner quotes. We still expect this to fail since it isn't
        self.assertRaises(SyntaxError,pyprepro.pyprepro,'{A = "}adshd}')


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
        self.assert_(compare_lines(pyprepro.pyprepro(input),gold))
    
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
        self.assert_(compare_lines(pyprepro.pyprepro(input),gold))
    

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
    
       ## A note on this test. Inside the template file is "{várïåbłę}". If run
       ## with python2, that variable name will be fixed via the dprepro parser
       ## BUT there is no easy way to fix it within the "read_from_dakota.inp"
       ## template. Therefore, we instead make a modify the template
       ## ahead of time and store it in the output when using python2
      
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
        self.assert_(compare_lines(read('test_output/dakota.1'),gold))
    
        # aprepo style with 3 inputs. NOTE: call dprepro
        cmd = '--no-warn test_files/dakota_aprepro.1 {0} test_output/dakota.2'.format(templatepath)
        pyprepro._dprepro_cli(shsplit(cmd))
        self.assert_(compare_lines(read('test_output/dakota.2'),gold))

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
        self.assert_(compare_lines(read('test_output/dakota.3'),gold))



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
        self.assert_(['a','b'] == sorted(D.keys()))

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
    
        self.assert_(compare_lines(OUT1,GOLD))
        self.assert_(compare_lines(OUT2,GOLD)) # by transitive property, OUT1 == OUT2


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
    
        self.assert_(compare_lines(output1,gold1))
        self.assert_(compare_lines(output2,gold2))
        self.assert_(compare_lines(output3,gold2))

    def test_division(self):
        """
        Make sure 3/2 == 1.5
        """
        self.assert_(pyprepro.pyprepro('{3/2}') == '1.5')

    def test_failure_no_var(self):
        """
        Test that it fails when presented with an undefined variable
        """
        input="""\
        {param1 = 10}
        {param2}
        """
    
        # Test via module
        self.assertRaises(NameError,pyprepro.pyprepro,input) # Should fail
    
        # test via cli
        with open('_testin.inp','w') as F:
            F.write(input)
            
        self.assertRaises(SystemExit,pyprepro._pyprepro_cli,['_testin.inp',['_out.inp']])
        os.remove('_testin.inp')
        #os.remove('_out.inp')


    def test_non_templated(self):
        """
        Test when the input has no templating
            1: String inpu
            2: File input
        """
    
        self.assert_(pyprepro.pyprepro('test nothing').strip() == 'test nothing')


    
if __name__ == '__main__':
    unittest.main()
# 
#     # Run all tests
#     failed = []
#     for name,test in [(n,l) for n,l in locals().copy().items() if callable(l) and n.startswith('test_')]:
#         try:
#             test()        
#             print('Passed: ' + name)
#         except Exception as E:
#             print('FAILED: ' + name)
#             print(' Exception',E)
#             failed.append(name)
#     
#     print('-'*40)
#     if len(failed) == 0:
#         print('SUCCESS')
#     else:
#         for name in failed:
#             print('FAILED: ' + name)
# #     sys.exit()



