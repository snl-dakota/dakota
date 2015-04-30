#!/usr/bin/env python

import sys
from os.path import join, abspath, dirname
from inspect import getfile, currentframe

here = dirname( abspath( getfile( currentframe() ) ) )
base = abspath( join( here, '..' ) )

# NOTE: insert this pyutilib.virtualenv path *second*.  
#   '' (the CWD) should always be first.
sys.path.insert(1, base)

from pyutilib.virtualenv.vpy_create import vpy_create
vpy_create('vpy', join(base, 'pyutilib', 'virtualenv', 'vpy_install.py'))
