# Changelog

Newest first

## 20210218.1

* Fixed a bug (#5) with empty inline expressions. Includes tests for this (including CLI usage)

## 20210218.0

* Handles the python 3.8 walrus operator

## 20200619.0

* Adds `--python-include` to pyprepro and dprepro
* Adds `--dakota-include` to pyprepro
* Adds `--simple-parser` to dprepro wherein the built-in parser is used instead of `dakota.interfacing`
* Made it throw an error if `include()` is called incorrectly and return a helpful message. 
