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
		
		System.out.println("Dakota JNI interface was tested successfully!");
	}
	
	public String testGetVariableLabels(DakotaSurrogates dakotaSurrogates) throws DakotaSurrogatesException {
		String errorMessage = "";
		
		//URL testDataFileUrl = DakotaJniTest.class.getResource("/surrogate/exp_poly.s12.response_fn_1.txt");
		//String surrogateFileAbsPath = new File(testDataFileUrl.getFile()).getAbsolutePath();		
		String surrogateFileAbsPath = new File("exp_poly.s12.response_fn_1.txt").getAbsolutePath();		
		
		List<String> variableLabels = dakotaSurrogates.getVariableLabels(surrogateFileAbsPath);
		
		if(variableLabels.size() != 8) errorMessage = "testGetVariableLabels failed - Expected 8 variable labels, but got " + variableLabels.size();
		if(!variableLabels.contains("u4")) errorMessage = "testGetVariableLabels failed - Variable label list does not contain u4";
		if(!variableLabels.contains("u5")) errorMessage = "testGetVariableLabels failed - Variable label list does not contain u5";
		if(!variableLabels.contains("d1")) errorMessage = "testGetVariableLabels failed - Variable label list does not contain d1";
		if(!variableLabels.contains("d2")) errorMessage = "testGetVariableLabels failed - Variable label list does not contain d2";
		if(!variableLabels.contains("u6")) errorMessage = "testGetVariableLabels failed - Variable label list does not contain u6";
		if(!variableLabels.contains("s7")) errorMessage = "testGetVariableLabels failed - Variable label list does not contain s7";
		if(!variableLabels.contains("d3")) errorMessage = "testGetVariableLabels failed - Variable label list does not contain d3";
		if(!variableLabels.contains("s8")) errorMessage = "testGetVariableLabels failed - Variable label list does not contain s8";
		
		return errorMessage;
	}
	
	public String testGetResponseLabels(DakotaSurrogates dakotaSurrogates) throws DakotaSurrogatesException {
		String errorMessage = "";
		
		//URL testDataFileUrl = DakotaJniTest.class.getResource("/surrogate/exp_poly.s12.response_fn_1.txt");
		//String surrogateFileAbsPath = new File(testDataFileUrl.getFile()).getAbsolutePath();		
		String surrogateFileAbsPath = new File("exp_poly.s12.response_fn_1.txt").getAbsolutePath();		
		
		List<String> responseLabels = dakotaSurrogates.getResponseLabels(surrogateFileAbsPath);
		
		if(responseLabels.size() != 1) errorMessage = "testGetResponseLabels failed - Expected 1 response label, but got " + responseLabels.size();
		if(!responseLabels.contains("response_fn_1")) errorMessage = "testGetResponseLabels failed - Response label list does not contain response_fn_1";
		
		return errorMessage;
	}
	
	public String testGetValue(DakotaSurrogates dakotaSurrogates) throws DakotaSurrogatesException {
		String errorMessage = "";
		
		//URL testDataFileUrl = DakotaJniTest.class.getResource("/surrogate/exp_poly.s12.response_fn_1.txt");
		//String surrogateFileAbsPath = new File(testDataFileUrl.getFile()).getAbsolutePath();		
		String surrogateFileAbsPath = new File("exp_poly.s12.response_fn_1.txt").getAbsolutePath();		
		
		List<Double> inputValues = new ArrayList<>();
		inputValues.add(1.0);
		inputValues.add(2.0);
		inputValues.add(3.0);
		inputValues.add(4.0);
		inputValues.add(5.0);
		inputValues.add(6.0);
		inputValues.add(7.0);
		inputValues.add(8.0);
		
		Double value = dakotaSurrogates.getValue(surrogateFileAbsPath, inputValues);
		
		if(4484.84908 != value) errorMessage = "testGetValue failed - Expected 4484.84908, but got " + value +".";
		return errorMessage;
	}	
}
