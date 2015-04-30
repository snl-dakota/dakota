
"""
Script to generate the installer for acro-admin.
"""

import glob
import os

def read(*rnames):
    return open(os.path.join(os.path.dirname(__file__), *rnames)).read()

from setuptools import setup

scripts = glob.glob("scripts/*")

setup(name='acro-admin',
      version='1.0',
      maintainer='William Hart',
      maintainer_email='wehart@sandia.gov',
      license = 'BSD',
      platforms = ["any"],
      description = 'Administrative scripts for acro',
      long_description = read('README.txt'),
      classifiers = [
            'Development Status :: 4 - Beta',
            'Intended Audience :: End Users/Desktop',
            'Intended Audience :: Science/Research',
            'License :: OSI Approved :: BSD License',
            'Natural Language :: English',
            'Operating System :: Microsoft :: Windows',
            'Operating System :: Unix',
            'Programming Language :: Python',
            'Programming Language :: Unix Shell',
            'Topic :: Software Development :: Libraries :: Python Modules'
        ],
      packages=['acro-admin'],
      keywords=['utility'],
      scripts=scripts
      )

