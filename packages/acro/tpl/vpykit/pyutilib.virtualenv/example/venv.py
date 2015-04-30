#  _________________________________________________________________________
#
#  PyUtilib: A Python utility library.
#  Copyright (c) 2008 Sandia Corporation.
#  This software is distributed under the BSD License.
#  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
#  the U.S. Government retains certain rights in this software.
#  _________________________________________________________________________
#

def configure(installer):
    installer.default_dirname = 'venv'
    #
    # Add repositories
    #
    installer.add_repository('virtualenv', pypi='virtualenv')
    installer.add_repository('nose', pypi='nose')
    #
    # Return the modified installer
    #
    return installer
