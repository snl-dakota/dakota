/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

package gov.sandia.dakota.jni.test;

public class OsUtil {
	
	private static final String OS = System.getProperty("os.name").toLowerCase(); 
	
	/**
	 * @return True if the current operating system is Windows.
	 */
	public static boolean isWindows() {
        return OS.indexOf("win") != -1;
    }
	
	/**
	 * @return True if the current operating system is Mac.
	 */
	public static boolean isMac() {
        return OS.indexOf("mac") != -1;
    }
	
	/**
	 * @return True if the current operating system is a Unix variant.
	 */
	public static boolean isUnix() {
        return OS.indexOf("nix") >= 0 || OS.indexOf("nux") >= 0 || OS.indexOf("aix") > 0;
    }
}
