/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

package gov.sandia.dart.dakota.tools;

import gov.sandia.dart.dakota.jni.*;

public class DakotaUtils {
    public static native void connect_plugin(ProblemDescDB db, String javaClass);
}