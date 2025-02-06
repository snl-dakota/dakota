/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

package gov.sandia.dakota.jni.test;

import java.io.File;

/**
 * Path utilities for Dakota.
 * 
 * @author Elliott Ridgway
 *
 */
public class DakotaSystemPathUtil {
    
    /**
     * Using Dakota's presence on the system path, try to locate Dakota's "lib" directory.
     * 
     * @return The path to the lib directory, or an empty String if it could not be found.
     */
    public static String findDakotaLibDirOnSystemPath() {
        String systemPath = OsUtil.isWindows() ? System.getenv("Path") : System.getenv("PATH"); //$NON-NLS-1$ //$NON-NLS-2$
        String[] paths = systemPath.split(";");
        for(int i = (paths.length - 1); i >= 0; i--) {
            if(paths[i].contains("dakota") && paths[i].contains("lib")) { //$NON-NLS-1$ //$NON-NLS-2$
                File dakotaLibDir = new File(paths[i]);
                if(dakotaLibDir.exists() && dakotaLibDir.isDirectory()) {
                	return dakotaLibDir.getAbsolutePath();
                }
            }
        }
        return ""; //$NON-NLS-1$
    }    
}
