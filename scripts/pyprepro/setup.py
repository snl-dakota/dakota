#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import division, print_function, unicode_literals, absolute_import

import pyprepro

from setuptools import setup

setup(
    name='pyprepro',
    py_modules=['pyprepro','dprepro'],
    long_description=open('readme.md').read(),
    version=pyprepro.__version__,
    entry_points = {
        'console_scripts': ['pyprepro=pyprepro:main',
                            'dprepro=pyprepro:main'],
    },
    description='Python preprocessor for simulation input decks',
    author='Justin Winokur and Adam Stephens',
    author_email='jgwinok@sandia.gov',
)
