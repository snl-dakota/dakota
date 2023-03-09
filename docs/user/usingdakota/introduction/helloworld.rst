.. _helloworld-main:

""""""""""""""""""""""""""
Dakota Beginner's Tutorial
""""""""""""""""""""""""""

This section is intended for users who are new to Dakota, to demonstrate the basics of running a simple example.

===========
First Steps
===========

1. After :ref:`downloading Dakota <setupdakota-download>`, make sure that Dakota runs. You should see Dakota version information when you type ``dakota -v``.
2. Create a working directory.
3. Copy ``rosen_multidim.in`` from the ``dakota/share/dakota/examples/users/`` directory to your working directory.
4. From the working directory, run:

.. code-block:

   dakota -i rosen multidim.in -o rosen multidim.out > rosen multidim.stdout

.. _helloworld-results:

==================
What should happen
==================

Dakota outputs a large amount of information to help users track progress. Four files should have been created:

1. The **screen output** has been redirected to the file ``rosen_multidim.stdout``. The contents are messages from Dakota and notes about the progress of the iterator (i.e. method/algorithm).
2. The **output file** `rosen_multidim.out` contains information about the function evaluations.
3. `rosen_multidim.dat` is created due to the specification of a **tabular data file.** This summarizes the variables and responses for each function evaluation.
4. `dakota.rst` is a **restart file**. If a Dakota analysis is interrupted, it can be often be restarted without losing all progress. Dakota has some data processing capabilities for output analysis. The output file will contain the relevant results. In this case, the output file has details about each of the 81 function evaluations. For more advanced or customized data processing or visualization, the tabular data file can be imported into another analysis tool.

=========
What now?
=========

- Assuming Dakota ran successfully, :ref:`skim the three text files <output>` (restart files are in a binary format).
- :ref:`Learn more about this example's parameter study method, and the rosenbrock test problem. <examples-gettingstarted-main>`
- :ref:`Explore the many types of studies that are possible in Dakota <studytypes-main>`.
- :ref:`Learn the syntax needed to use these methods <inputfile-main>`.
- :ref:`Learn how to use your own analysis code with Dakota <couplingtosimulations-main>`.

===============
Video Resources
===============

.. image:: img/DakotaSimpleExampleScreencastTeaser.png
   :target: https://www.youtube.com/watch?v=ofi13UTq_Is&list=PLouetuxaIMDo-NMFXT-hlHYhOkePLrayY&index=1
   :alt: Watch Screencast 1.1: Running a Simple Example in Dakota
