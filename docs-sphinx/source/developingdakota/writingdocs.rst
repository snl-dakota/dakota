""""""""""""""""""""""""""""
Writing Dakota Documentation
""""""""""""""""""""""""""""

===========================
Install Sphinx and Packages
===========================

.. code-block::

	pip install --user -U Sphinx
	pip install --user -U myst-parser
	pip install --user -U sphinx-rtd-theme
	pip install --user -U sphinxcontrib-bibtex
	# If your docutils ends up at 0.18 or greater you may need to downgrade it:
	pip install --user 'docutils<0.18' --force-reinstall
	
========
Building
========

.. code-block::

	cd dakota_source/docs-sphinx
	sphinx-build -b html <source dir> <build dir>