/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
