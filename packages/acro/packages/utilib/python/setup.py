"""
Script to install UTILIB-related Python utilities.
"""

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup


setup(name="utilib",
      version='1.0',
      maintainer='William Hart',
      maintainer_email='wehart@sandia.gov',
      url = 'https://software.sandia.gov/trac/utilib',
      license = 'BSD',
      platforms = ["any"],
      description = 'Python scripts related to the UTILIB software library',
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
      packages=[],
      keywords=['utility'],
      scripts= ['test.utilib']
      )

