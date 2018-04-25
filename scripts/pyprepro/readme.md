Preprocessor and template engine for parameterized input files.

A full template engine (based off [Bottlepy's Simple Template Engine][bottle]). This is designed for parameterized input decks and therefore includes some key differences from a general-purpose (web) template engine:

* Immutable variables -- If a variable is immutable, it cannot be reassigned.
* Automatic print/echo of inline expressions


# Simple Example:

    angles = {angle = 35.6}
    speed = {speed = 10}
    Vx = {speed * cos( radians(angle) ) }

Returns:

    angle = 35.6
    speed = 10
    Vx = 8.13100761

# Expressions

Expressions can be of three different forms (with defaults)

* Inline single-line expressions (rendered) [ `{expression}` ]
* Python code single-line  (silent) [ `% expression` ]
* Python code multi-line blocks (silent) [ `{% expression %}` ]

The delineation of these expressisions are user settable

Expressions can contain just about any valid Python code. The only important difference is that blocks must end with `end`. See examples below

# Manual

See [the manual](manual.md) for much for information and examples

# Python Versions

The templates are executed with the same version of python that is used to call this code (or the default in your path). 

The code should run out-of-the-box on python 2.7 and 3+. To run on 2.6, the `argparse` module *may* need to be installed (it may be already on many systems). 

All functionality in the test suites have been tested on python 2.6, *however*, the documentation and most usage is with 2.7 and 3+.

A key difference if using 2.6 is you must specify format specifiers with a name or number.

| Example                 | Versions     |
|-------------------------|--------------|
| `'{0}'.format(name)`    | 2.6, 2.7, 3+ |
| `'{name}'.format(name)` | 2.6, 2.7, 3+ |
| `'{}'.format(name)`     | 2.7, 3+      |

# Functions

The two main functions are `pyprepro` and `dprepro`. `dprepro` leverages `pyprepro` but had some slight modifications to the command line syntax and parsing of includes. 

**FUTURE**

While these both may be directly installed via `pip install pyprepro`, the actual `pyprepro` file itself (**not** *symlink* ending in `.py`) can be copied and placed in any folder and run directly with `python pyprepro`. If you wish to use `dprepro` on its own, it must be able to find `pyprepro`.
