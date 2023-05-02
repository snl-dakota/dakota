# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
sys.path.insert(0, os.path.abspath("./_extensions"))
sys.path.append(os.path.abspath("./_pygments"))

from lexer.dakota import DakotaLexer

# -- Project information -----------------------------------------------------

project = 'dakota'
copyright = '2023, Sandia National Laboratories'
author = 'Sandia National Laboratories'

# The major project version, used as the replacement for
# |version|. For example, for the Python documentation, this may be
# something like 2.6.
#
# For Dakota this will be substituted at build time by CMake
version = ''

# The full project version, used as the replacement for |release| and
# e.g. in the HTML templates. For example, for the Python
# documentation, this may be something like 2.6.0rc1.
#
# For Dakota this will be substituted at build time by CMake, and may
# include an appended stable '+'
release = ''


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ['myst_parser', 'sphinxcontrib.bibtex', 'dakota',
              'sphinx.ext.imgmath']
#              'sphinx.ext.mathjax']

# Dakota input file syntax highlighting
highlight_language = "dakota"
pygments_style = 'style.dakota.DakotaStyle'

source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown'
}

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

# -- Options for BibTeX citation input ---------------------------------------
bibtex_bibfiles = ['../../docs/Dakota.bib']

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

html_theme_options = {
    'navigation_depth': -1,
}

html_css_files = [
    'css/dakota_theme.css',
    'css/sandiaheaderlite.css'
]

html_logo = 'img/dakota_Arrow_Name_Tag_horiz_transparent.png'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# Default to mathjax for better build speed during development
html_math_renderer = 'mathjax'
# Jenkins will override to images for packaging
#html_math_renderer = 'imgmath'

# Enable numref
numfig = True

# Could consider numbering equations with chapter or even section
# instead of globally across document, but would require uses
# :eq:`label`; https://github.com/sphinx-doc/sphinx/pull/4166

##math_numfig = True
##numfig_secnum_depth = 1
