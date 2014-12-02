/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

package gov.sandia.dart.dakota.tools;

import gov.sandia.dart.dakota.jni.DoubleVector;
import gov.sandia.dart.dakota.jni.DoubleMatrix;

public class SquareFunctor extends AbstractDakotaFunctor {
    
    @Override public int evalFunction(DoubleVector dv, int evalId) {
	try {
	    double x = getDouble("x");
	    dv.clear();
	    dv.add(x * x);
	    return 0;
	} catch (Exception ex) {
	    return -1;
	}
    }

}