/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

package gov.sandia.dakota.jni.test;

import java.io.File;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

import gov.sandia.dakota.jni.surrogates.java.DakotaSurrogates;

public class DakotaJniTest {

	public static void main(String[] args) throws Exception {
		DakotaJniTest instance = new DakotaJniTest();
		DakotaSurrogates dakotaSurrogates = new DakotaSurrogates();
		
		String message = instance.testGetVariableLabels(dakotaSurrogates);
		if(message != null && !message.isEmpty()) {
			throw new DakotaSurrogatesException(message);
		}
		
		message = instance.testGetResponseLabels(dakotaSurrogates);
		if(message != null && !message.isEmpty()) {
			throw new DakotaSurrogatesException(message);
		}

		message = instance.testGetValue(dakotaSurrogates);
		if(message != null && !message.isEmpty()) {
			throw new DakotaSurrogatesException(message);
		}
		
		System.out.println("Dakota JNI interface was tested successfully!");
	}
	
	public String testGetVariableLabels(DakotaSurrogates dakotaSurrogates) throws DakotaSurrogatesException {
		String errorMessage = "";
		
		//URL testDataFileUrl = DakotaJniTest.class.getResource("/surrogate/exp_poly.s12.response_fn_1.txt");
		//String surrogateFileAbsPath = new File(testDataFileUrl.getFile()).getAbsolutePath();		
		String surrogateFileAbsPath = new File("../unit/poly_test.surr.bin").getAbsolutePath();		
		
		List<String> variableLabels = dakotaSurrogates.getVariableLabels(surrogateFileAbsPath);
		
		if(variableLabels.size() != 2) errorMessage += "\ntestGetVariableLabels failed - Expected 2 variable labels, but got " + variableLabels.size();
		if(!variableLabels.contains("poly_x1")) errorMessage += "\ntestGetVariableLabels failed - Variable label list does not contain poly_x1";
		if(!variableLabels.contains("poly_x2")) errorMessage += "\ntestGetVariableLabels failed - Variable label list does not contain poly_x2";
		
		return errorMessage;
	}
	
	public String testGetResponseLabels(DakotaSurrogates dakotaSurrogates) throws DakotaSurrogatesException {
		String errorMessage = "";
		
		//URL testDataFileUrl = DakotaJniTest.class.getResource("/surrogate/exp_poly.s12.response_fn_1.txt");
		//String surrogateFileAbsPath = new File(testDataFileUrl.getFile()).getAbsolutePath();		
		// also test the .txt file as should be the same
		String surrogateFileAbsPath = new File("../unit/poly_test.surr.txt").getAbsolutePath();		
		
		List<String> responseLabels = dakotaSurrogates.getResponseLabels(surrogateFileAbsPath);
		
		if(responseLabels.size() != 1) errorMessage += "\ntestGetResponseLabels failed - Expected 1 response label, but got " + responseLabels.size();
		if(!responseLabels.contains("poly_f")) errorMessage += "\ntestGetResponseLabels failed - Response label list does not contain poly_f";
		
		return errorMessage;
	}
	
	public String testGetValue(DakotaSurrogates dakotaSurrogates) throws DakotaSurrogatesException {
		String errorMessage = "";
		
		//URL testDataFileUrl = DakotaJniTest.class.getResource("/surrogate/exp_poly.s12.response_fn_1.txt");
		//String surrogateFileAbsPath = new File(testDataFileUrl.getFile()).getAbsolutePath();		
		String surrogateFileAbsPath = new File("../unit/poly_test.surr.bin").getAbsolutePath();		
		

		double x1 = 1.4;
		double x2 = 2.1;

		List<Double> inputValues = new ArrayList<>();
		inputValues.add(x1);
		inputValues.add(x2);
		
		Double value = dakotaSurrogates.getValue(surrogateFileAbsPath, inputValues);
		double value_gold = 1.0 +
		    Math.pow(2.0*x1-1.0, 2.0) + 2.0 * (2.0*x1-1.0) +
		    Math.pow(2.0*x2-1.0, 2.0) + 2.0 * (2.0*x2-1.0);

		double rel_err = Math.abs(value-value_gold)/Math.abs(value_gold);
		// the gold value should be around 24.48
		if(rel_err > 1.0e-12) errorMessage = "testGetValue failed - Expected " + value_gold + ", but got " + value + "; rel_error = " + rel_err + ".";
		return errorMessage;
	}	
}
