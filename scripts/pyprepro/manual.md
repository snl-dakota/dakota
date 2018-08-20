# Preface

[TOC]

All examples in the manual, except where otherwise noted, use the default syntax of `{  }` to be inline printed expressions, `%` to be a single line Python code and `{% %}` to be block Python code.

# Introduction

`pyprepro` is a robust preprocessor and template engine designed for parameterized simulation input decks. It features simple parameter substitution, setting of immutable (fixed) variable names, and enables full access to all of Python. As such, it can contain loops, conditionals, arrays (lists) and more!

# Expressions

Expressions can be of three different forms (with defaults)

* Inline single-line expressions (rendered) [ `{expression}` ]
* Python code single-line  (silent) [ `% expression` ]
* Python code multi-line blocks (silent) [ `{% expression (that can be over many line) %}` ]

The delineation of these expressions are user settable

Expressions can contain just about any valid Python code. The only important difference is that blocks must end with `end`. See examples below.

## Inline Expressions

Inline expressions are delineated with `{expression}`  and **always display**. 

Consider:

    param1 = {param1 = 10}
    param2 = {param1 + 3}
    param3 = {param3 = param1**2}

Returns:

    param1 = 10
    param2 = 13
    param3 = 100

In this example, the first and third line both display a value *and* set the parameter.

## Python Single Line Code

Using a `%` at the start of a line will tell pyprepo that it is a single-line code expression. Consider the following example

    % param1 = pi/4
    The new value is {sin(param1)}

return:
    
    The new value is 0.7071067812
    
Furthermore, single lines can be used for Python logic and loops. Again, recall that unlike traditional Python, the blocks must have an explicit `end`. This example also demonstrates looping over an array (explained below). Notice that indentation is ignored.

    % angles = [0,pi/4,pi/2,3*pi/4,pi]
    % for angle in angles:
    cos({angle}) = { cos(angle)}
    % end

Returns:

    cos(0) = 1
    cos(0.7853981634) = 0.7071067812
    cos(1.570796327) = 6.123233996e-17
    cos(2.35619449) = -0.7071067812
    cos(3.141592654) = -1

## Multi-line expressions

Finally, pyprepo will accept multiple lines without needing to start each explicitly. Indentation is *still* ignored

    {%
    # Can have comments too!
    txt = ''
    for ii in range(10):
        txt += ' {}'.format(ii)
    end
    %}
    txt: {txt}

returns:

    txt:  0 1 2 3 4 5 6 7 8 9

## Changing delimiters

As noted in the `--help` for pyprepro, the actual delimiters can be changed. This is useful when the defaults would require extensive escaping. 

For code blocks (default `{% %}`), the innermost characters cannot be any of "`{}[]()`".

## Escaping delimiters

All delimiters can be escaped with a leading `\`. A double `\\` followed by the delimiter will return `\`. For example:

    {A=5}
    \{A=5\}
    \\{A=5\\}

returns

    5
    {A=5}
    \{A=5\}  

Note that escaping the trailing delimiter (e.g. `\}`) is optional.

# Immutable Variables

Variables can be fixed such that they cannot be redefined (without explicitly allowing it). 

    % param = Immutable(10)
    % param = 20 
    {param}

will still output:

    10

since `param` is `Immutable`. To explicitly make a variable mutable again, call it with `Mutable()`:

    set             : \{ param = Immutable(10) \} : { param = Immutable(10) }           
    try to reset    : \{ param = 20 \}            : { param = 20 }          
    make mutable    : \{ param = Mutable(21) \}   : { param = Mutable(21) } 
    reset           : \{ param = 20 \}            : { param = 20 }         

returns:

    set             : { param = Immutable(10) } : 10
    try to reset    : { param = 20 }            : 10
    make mutable    : { param = Mutable(21) }   : 21
    reset           : { param = 20 }            : 20

Note that any variable set via `--include` on the command line will be **set as Immutable**. This is useful for templating and overriding a default.

`MyTemplate.inp`:

    param1 = {param1 = 10}
    param2 = {param2 = pi}
    
If called directly:

    param1 = 10
    param2 = 3.141592654
    
However, if called `pyprepro --var "param1=30" <inputfile>`:

    param1 = 30
    param2 = 3.141592654

Or, if you create a file:

`MyInclude.inp`:

    {param1 = 32}

And call `pyprepro --include MyInclude.inp <inputfile>`:

    param1 = 32
    param2 = 3.141592654

## Cavets:

(*This is an advanced usage and, in general, will not be encountered*)

While the variable name is reserved, the value can still be changed if it is a mutable Python object ("mutable" has different meanings for Python objects than is used in pyprepro). For example:

    % param = Immutable( [1,2,3])
    % param.append(4)   # This will work because it is modifying the object
    % param = ['a','b','c']   # This won't because it is redefining
    {param}

Will output:

    [1, 2, 3, 4]


# Command line help

See:

    $ Python pyprepro -h

for additional help

# General Coding

The language of pyprepro is Python with a single, slight modification. In normal Python, indentation delineates blocks. However, in pyprepro, indentation is ignored and blocks must have an `end` whether they are part of multi-line code (`{% %}`) or part of single line operation  (`%`).

## Python Coding Tips.

For the most part, if you are familiar with other interpreted languages such as Matlab, coding is Python is very similar.

The key notes are:

* Blocks have `:` at the end of their statement. For example, `if CONDITION:` *with* the colon
* Indentation **USUALLY** matters but **DOESN'T** in pyprepo
    * As such, you must include a `end` statement
* Arrays are zero-based
* Exponentiation is double `**`. Example: `x**y` ("x to the y")

## Conditionals

Python has the standard set of conditionals. Recall the conditional block declaration must end with a `:` and the entire block must have an `end` statement (again, this is not in normal Python). Consider the following example:

    % param = 10.5
    % if param == 10.0:
    param is 10! See: {param}
    % else:
    param does not equal 10, it is {param}
    % end

    % if 10 <= param <= 11:
    param ({param}) is between 10 and 11
    % else:
    param is out of scope
    % end

results in:

    param does not equal 10, it is 10.5

    param (10.5) is between 10 and 11

Boolean operations are also possible using simple `and`, `or`, and `not` syntax

    % param = 10.5
    % if param >= 10 and param <= 11:
    param is in [10 11]
    % else:
    param is NOT in [10,11]
    % end

returns:

    param is in [10 11]

## Loops

Python contains `for` loops that iterate over arbitrary arrays or with an index. As with conditionals, the declaration must end with `:` and the block must have an `end`.

To iterate over an index, from 0 to 4, use the  `range` command

    % for ii in range(5):
    {ii}
    % end

will return:

    0
    1
    2
    3
    4

You can also iterate over objects in an array (list):

    % animals = ['cat','mouse','dog','lion']
    % for animal in animals:
    I want a {animal}
    %end

results in 

    I want a cat
    I want a mouse
    I want a dog
    I want a lion

## Arrays

Arrays are **zero indexed** and can also have negative indices representing the end of the array. They are references by `myarray[index]`

Consider:

    % animals = ['cat','mouse','dog','lion']
    {animals[0]}
    {animals[-1]}

will result in

    cat
    lion


Note that pyprepro will *try* to nicely format arrays for printing. For certain types, it may not work well.

    {theta = [0,45,90,135,180,225,270,315]}

(with `{ }` to print input) results in

    [0, 45, 90, 135, 180, 225, 270, 315]


### Math on arrays

Unlike some tools (e.g. Matlab) you cannot do math on all elements of arrays directly. However, there are two possibilities.

The first will *always* work:

    % theta = [0,45,90,135,180,225,270,315] 
    { [ sin(pi*th/180) for th in theta ] }


results in 

    [0, 0.7071067812, 1, 0.7071067812, 1.224646799e-16, -0.7071067812, -1, -0.7071067812]

The alternative is to use NumPy *if* it is installed.

    % theta = [0,45,90,135,180,225,270,315]
    % import numpy as np
    % theta = np.array(theta) # Redefine as numpy array
    { np.sin(pi*theta/180) }


will return:

    [0, 0.7071067812, 1, 0.7071067812, 1.224646799e-16, -0.7071067812, -1, -0.7071067812]

## Strings

Python has  extremely powerful and extensive string support. Strings can be initialized in any of the following ways:

    {mystring1="""
    multi-line
    string inline
    """}
    {mystring1}
    {% mystring2 = '''
    another multi-line example
    but in a block
    ''' %}
    mystring2: {mystring2}
    
    Single quotes: {'singe'}
    Double quotes: {'double'}

and it returns

    multi-line
    string inline


    multi-line
    string inline

    mystring2:
    another multi-line example
    but in a block


    Single quotes: singe
    Double quotes: double

The choice of single `'` or `"` can be driven by convenience.

Strings can be joined by adding them:

    {%
    a = 'A'
    b = 'B'
    %}
    {a + ' ' + b}

returns:

    A B


### Custom Functions

You can define any arbitrary functions using either `def` or `lambda`

Consider the following: (note, we use indentation here for readability but indentation *is ignored* and the function definition is terminated with `end`):

    {%
    def myfun1(param):
        return (param + 1) ** 2 + 3
    end

    myfun2 = lambda param: (param + 1) ** 2 + 5
    %}
    {myfun1(1.2)}
    {myfun2(1.2)}
    { [ myfun1(x) for x in [1,2,3,4] ] }

returns:

    7.84
    9.84
    [7, 12, 19, 28]

# Auxiliary Functions

There are a series of auxiliary functions to help. The primary one is the `include`

## Include

Using 
 
    % include('path/to/include.txt')

Will insert the contents of `'path/to/include.txt'`. Inside `'path/to/include.txt'`, there can be new variable definitions and/or it can access older ones. Note that unlike the command-line `--include`, there is *no (im)mutability assigned* for these unless explicit for each parameter!

The code will search for the include text first in the path of the original template file and then in the path where `pyprepro` is executed.

## Immutable and Mutable

As explained elsewhere, variables can be defined as `Immutable(value)` or `Mutable(value)`. If a variable is Immutable, the value cannot be reset unless *explicitly* made mutable.

Note: files called with the command line `--include` have all of their variables be immutable. But this does not affect those with the `include()` function

## Print all variables

`all_vars()` and `all_var_names()` print out all *defined* variables. Consider the following that also demonstrates setting a comment string (two ways)

    % param1 = 1
    {param2 = 'two'}
    all variables and values: {all_vars()}
    all varables: {all_var_names()}

    {all_var_names(comment='//')}
    // {all_var_names()} <--- Don't do this

returns:

    two
    all variables and values: {'param1': 1, 'param2': u'two'}
    all varables: ['param2', 'param1']

    // ['param2', 'param1']
    // ['param2', 'param1'] <--- Don't do this

Notice the empty `()` at the end of `all_vars` and `all_var_names`. If possible, it is *better* to use `comment='//'` syntax since the result of these can be multiple lines.


## Set global print format

As discussed elseware, the print format can be set on a per item basis by manually converting to a string. Alternatively, it can be (re)set globally inside the template (as well as at the command line).

    {pi}
    % setfmt('%0.3e')
    {pi}
    % setfmt() # resets
    {pi}

returns:

    3.141592654
    3.142e+00
    3.141592654

### Aside: Set output format individually

The following demonstrates setting the output for a specific line. The key is to use Python to convert the value into a string which is then displayed:

    {pi}
    { '%0.3f' % pi }

Will output:
    
    3.141592654
    3.142

## Using Defaults undefined parameters

Directly calling undefined parameters will result in an error. There is no *universal* default value. However, there are the following functions:

* `get` -- get param with optional default
* `defined` -- determine if the variable is defined

The usage is explained in the following examples:

    Defined Parameter:
    % param1 = 'one'
    { get('param1') } <-- one
    { get('param1','ONE') } <-- one

    Undefined Paramater
    { get('param2') } <-- *blank*
    { get('param2',0) } <-- 0

    Check if defined: { defined('param2') }

    % if defined('param2'):
    param2 is defined: {param2}
    % else:
    param2 is undefined
    % end

returns:

    Defined Parameter:
    one <-- one
    one <-- one

    Undefined Paramater
     <-- *blank*
    0 <-- 0

    Check if defined: False

    param2 is undefined

But notice if you have the following:

    {param3}

you will get the following error:

    Error occurred:
        NameError: name 'param3' is not defined

## Mathematical Functions

All of the Python `math` module in imported with the functions:

      acos       degrees     gamma   radians  
      acosh      erf         hypot   sin      
      asin       erfc        isinf   sinh      
      asinh      exp         isnan   sqrt      
      atan       expm1       ldexp   tan       
      atan2      fabs        lgamma  tanh      
      atanh      factorial   log     trunc     
      ceil       floor       log10   
      copysign   fmod        log1p   
      cos        frexp       modf             
      cosh       fsum                               
   
Also included are the following constants
   
| Name                     | value         |
|--------------------------|---------------|
| `pi`,`PI`                | 3.141592654   |
| `e`,`E`                  | 2.718281828   |
| `tau` (`2*pi`)           | 6.283185307   |
| `deg` (`180/pi`)         | 57.29577951   |
| `rad` (`pi/180`)         | 0.01745329252 |
| `phi` (`(sqrt(5)+1 )/2`) | 1.618033989   |
 
Note that all trig functions are assuming radians. See [Python's `math` library](https://docs.Python.org/3/library/math.html) for more details. To compute based on degrees, convert first:

    { tan( radians(45) )}
    { tan( 45*rad)}
    { degrees( atan(1) )}
    { atan(1) * deg }

returns:

    1
    1
    45
    45

## Other Functions

Other functions can be imported. All of Python is available. For example, to get a random number, you can do:

    % from random import random,seed
    % seed(1)
    {A = random()}

Returns (may depend on the system)

    0.1343642441








