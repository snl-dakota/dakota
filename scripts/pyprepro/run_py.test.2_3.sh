#!/usr/bin/env bash
# Test with both python 2 and 3 using py.test


p2dir=$(dirname $(command which python2))
${p2dir}/py.test run_tests.py --cov-report html --cov pyprepro

p3dir=$(dirname $(command which python3))
${p3dir}/py.test run_tests.py --cov pyprepro
