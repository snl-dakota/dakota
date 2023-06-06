/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

package gov.sandia.dakota.jni.test;

public class DakotaSurrogatesException extends Exception {
	private static final long serialVersionUID = 6616649245494199370L;
	
	public DakotaSurrogatesException(String message) {
		super(message);
	}
}
