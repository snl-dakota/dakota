"""
Script to generate the installer for acro.pebbl
"""

import glob
import os

def read(*rnames):
    return open(os.path.join(os.path.dirname(__file__), *rnames)).read()

def _find_packages(path):
    """
    Generate a list of nested packages
    """
    pkg_list=[]
    if not os.path.exists(path):
        return []
    if not os.path.exists(path+os.sep+"__init__.py"):
        return []
    else:
        pkg_list.append(path)
    for root, dirs, files in os.walk(path, topdown=True):
      if root in pkg_list and "__init__.py" in files:
         for name in dirs:
           if os.path.exists(root+os.sep+name+os.sep+"__init__.py"):
              pkg_list.append(root+os.sep+name)
    return map(lambda x:x.replace(os.sep,"."), pkg_list)

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup
packages = _find_packages('acro')


setup(name="acro.pebbl",
      version='1.0',
      maintainer='Jonathan Eckstein',
      maintainer_email='jeckstei@rci.rutgers.edu',
      url = 'https://software.sandia.gov/trac/acro',
      license = 'BSD',
      platforms = ["any"],
      description = 'Python scripts related to the PEBBL software library',
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
            'Topic :: Scientific/Engineering :: Mathematics',
            'Topic :: Software Development :: Libraries :: Python Modules',
            'Topic :: Scientific/Engineering'
        ],
      packages=packages,
      keywords=['utility', 'optimization'],
      scripts= glob.glob("scripts/*")+['test.pebbl'],
      namespace_packages=['acro']
      )

