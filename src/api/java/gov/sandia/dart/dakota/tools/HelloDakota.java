/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

package gov.sandia.dart.dakota.tools;

import java.util.ArrayList;
import gov.sandia.dart.dakota.jni.*;

public class HelloDakota {
    public static void main(String[] args) {	
	new HelloDakota().runParse(new String[] {"dakotajni", "dakota_square.in"});
    }

    public void runParse(String[] argv) {
	ProgramOptions opts = new ProgramOptions();
	opts.input_file(argv[1]);
	opts.exit_mode("throw");

	LibraryEnvironment env = new LibraryEnvironment(opts);

	ProblemDescDB problem_db = env.problem_description_db();

	DakotaWrapper.connect_plugin(problem_db, new SquareFunctor());
	
	env.execute();

    }
    
    public static class my_callback_data {
	public ProblemDescDB db;
    }

    static {
	System.loadLibrary("dakotajni");
    }
}
