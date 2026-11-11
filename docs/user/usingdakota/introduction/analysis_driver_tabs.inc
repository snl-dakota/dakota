..
   Shared analysis-driver tabs for the black-box Rosenbrock example, included
   by couplingtosimulations.rst and helloworld.rst. Edit the drivers here only.
   TODO (regret): ship these drivers, ros.template and rosenbrock_bb.py in
   dakota-examples with regression tests, and literalinclude them instead.

.. tab:: Python

   This version uses Dakota's ``dakota.interfacing`` Python module to read
   the parameters file, fill in the template, and write the results file.

   .. code-block:: python

      #!/usr/bin/env python3
      # Analysis driver for the black-box Rosenbrock example.
      # Dakota runs it as:  python3 driver.py params.in results.out

      import subprocess
      import sys

      import dakota.interfacing as di

      # Read the parameters file Dakota wrote, and set up the results file.
      params, results = di.read_parameters_file()

      # --------------
      # PRE-PROCESSING
      # --------------
      # Put this evaluation's x1 and x2 into the simulation's input file.
      di.dprepro(template="ros.template", parameters=params, output="ros.in")

      # ---------
      # EXECUTION
      # ---------
      subprocess.run([sys.executable, "rosenbrock_bb.py"], check=True)

      # ---------------
      # POST-PROCESSING
      # ---------------
      # Find the function value in the simulation's output, and return it to
      # Dakota as the first (and only) response.
      with open("ros.out") as f:
          for line in f:
              if "Function value" in line:
                  value = float(line.split("=")[1])

      results[0].function = value
      results.write()

.. tab:: Bash

   This version uses the ``dprepro`` command to fill in the template, and
   ``grep`` and ``awk`` to find the value in the output.

   .. code-block:: bash

      #!/usr/bin/env bash
      # Analysis driver for the black-box Rosenbrock example.
      # Dakota runs it as:  driver.sh params.in results.out

      params=$1
      results=$2

      # --------------
      # PRE-PROCESSING
      # --------------
      # Put this evaluation's x1 and x2 into the simulation's input file.
      dprepro "$params" ros.template ros.in

      # ---------
      # EXECUTION
      # ---------
      python3 rosenbrock_bb.py

      # ---------------
      # POST-PROCESSING
      # ---------------
      # Find the function value in the simulation's output, and return it to
      # Dakota as the first (and only) response.
      grep 'Function value' ros.out | awk '{print $4}' > "$results"

   .. note::

      ``dprepro`` runs with the ``python`` command. If your system has only
      ``python3``, use ``dprepro3`` instead, which runs ``dprepro`` with
      ``python3``.

.. tab:: Windows (PowerShell)

   This version runs ``dprepro`` with Python to fill in the template, and
   uses ``Select-String`` to find the value in the output.

   .. code-block:: powershell

      # Analysis driver for the black-box Rosenbrock example.
      # Dakota runs it as:  powershell.exe -File driver.ps1 params.in results.out

      $params = $args[0]
      $results = $args[1]

      # --------------
      # PRE-PROCESSING
      # --------------
      # Put this evaluation's x1 and x2 into the simulation's input file.
      $dprepro = (Get-Command "dprepro").Path
      python3 $dprepro $params ros.template ros.in

      # ---------
      # EXECUTION
      # ---------
      python3 rosenbrock_bb.py

      # ---------------
      # POST-PROCESSING
      # ---------------
      # Find the function value in the simulation's output, and return it to
      # Dakota as the first (and only) response.
      $line = Select-String -Path ros.out -Pattern "Function value"
      $value = ($line.Line -split "=")[1].Trim()
      Set-Content -Path $results -Value $value -Encoding ASCII
