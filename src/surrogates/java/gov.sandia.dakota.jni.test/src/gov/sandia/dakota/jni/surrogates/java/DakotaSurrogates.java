/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

package gov.sandia.dakota.jni.surrogates.java;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import gov.sandia.dakota.jni.test.DakotaSurrogatesException;
import gov.sandia.dakota.jni.test.DakotaSystemPathUtil;
import gov.sandia.dakota.jni.test.OsUtil;

/**
 * JNI interface to the dakota_surrogates library shipped with Dakota.<br><br>
 * Note that dakota_surrogates MUST be accessible from the OS's library path
 * ("DY_LIBRARY_PATH" on Linux, "Path" on Windows) for this class to work.
 * 
 * @author Elliott Ridgway
 *
 */
public class DakotaSurrogates {
	
    ////////////
    // FIELDS //
    ////////////
    
    private static Map<String, Long> surrogatePointerMap;
    private static boolean jniSurrogatesLibraryLoaded;
    
    public static final String BIN_FILE_EXTENSION = "bin"; //$NON-NLS-1$
    public static final String TXT_FILE_EXTENSION = "txt"; //$NON-NLS-1$
    
    public static final String LIBRARY_NAME_WINDOWS = "Release/jni_dakota_surrogates.dll";      //$NON-NLS-1$
    public static final String LIBRARY_NAME_MAC     = "libjni_dakota_surrogates.dylib"; //$NON-NLS-1$
    public static final String LIBRARY_NAME_LINUX   = "libjni_dakota_surrogates.so";    //$NON-NLS-1$
	
	////////////
    // STATIC //
	////////////
    
    static {
        surrogatePointerMap = new HashMap<>();
        jniSurrogatesLibraryLoaded = false;
    }
    
	public static void loadDakotaJniSurrogatesLibrary() throws DakotaSurrogatesException {
		jniSurrogatesLibraryLoaded = false;
		//String dakotaLibPath = DakotaSystemPathUtil.findDakotaLibDirOnSystemPath();
		String dakotaLibPath = new File(".").getAbsolutePath();
		if(dakotaLibPath != null && !dakotaLibPath.isEmpty()) {	
			try {
				File libDir = new File(dakotaLibPath);
				if(libDir.exists()) {
					File jniLibraryFile = null;
					if(OsUtil.isWindows()) {
						jniLibraryFile = new File(libDir, LIBRARY_NAME_WINDOWS);
					} else if(OsUtil.isMac()) {
						jniLibraryFile = new File(libDir, LIBRARY_NAME_MAC);
					} else if(OsUtil.isUnix()) {
						jniLibraryFile = new File(libDir, LIBRARY_NAME_LINUX);
					}
					if(jniLibraryFile != null && jniLibraryFile.exists()) {
						System.load(jniLibraryFile.getAbsolutePath());
						jniSurrogatesLibraryLoaded = true;
					} else if(jniLibraryFile != null) {
						String message = jniLibraryFile.getAbsolutePath() + " does not exist in lib directory.";
						throw new DakotaSurrogatesException(message);
					} else {
						String message = "Unknown operating system";
						throw new DakotaSurrogatesException(message);
					}
				}
			} catch(Exception e) {
				throw new DakotaSurrogatesException(e.getMessage());
			}
		} else {
			throw new DakotaSurrogatesException("Dakota lib directory is not on path - JNI surrogates library cannot be loaded.");
		}
	}
    
    ////////////
    // PUBLIC //
    ////////////
    
    /**
     * 
     * @param surrogateFileAbsPath The absolute file path of the surrogate.
     * @return The ordered List of variable labels from the surrogate.
     * @throws DakotaSurrogatesException 
     */
    public List<String> getVariableLabels(String surrogateFileAbsPath) throws DakotaSurrogatesException {
    	List<String> variableLabels = new ArrayList<>();
    	
    	Long pointer = getOrLoadSurrogate(surrogateFileAbsPath);
    	if(pointer != null) {
			Object[] objArray = getVariables(pointer);
			for(int i = 0; i < objArray.length; i++) {
				if(objArray[i] instanceof String) {
					variableLabels.add((String)objArray[i]);
				}
			}
    	}
		
		return variableLabels;
    }
    
	/**
	 * 
	 * @param surrogateFileAbsPath The absolute file path of the surrogate.
	 * @return The ordered List of response labels from the surrogate. Typically for
	 *         Dakota surrogates, this will be a single response, but the Dakota API
	 *         technically allows for multiple responses to be returned.
	 * @throws DakotaSurrogatesException 
	 */
    public List<String> getResponseLabels(String surrogateFileAbsPath) throws DakotaSurrogatesException {
    	List<String> responseLabels = new ArrayList<>();
    	
    	Long pointer = getOrLoadSurrogate(surrogateFileAbsPath);
    	if(pointer != null) {
			Object[] objArray = getResponses(pointer);
			for(int i = 0; i < objArray.length; i++) {
				if(objArray[i] instanceof String) {
					responseLabels.add((String)objArray[i]);
				}
			}
    	}
		
		return responseLabels;
    }
    
	/**
	 * 
	 * @param surrogateFileAbsPath The absolute file path of the surrogate.
	 * @param inputValues          The list of input values to the surrogate.
	 * @return The single output value of the surrogate.
	 * @throws DakotaSurrogatesException 
	 */
    public Double getValue(String surrogateFileAbsPath, List<Double> inputValues) throws DakotaSurrogatesException {
    	double[] inputValuesArr = asDoubleArrFromDoubleList(inputValues);
    	Long surrogatePointer = getOrLoadSurrogate(surrogateFileAbsPath);
    	if(surrogatePointer != null) {
    		return value(surrogatePointer, inputValuesArr);
    	}
    	return null;
    }
    
	/**
	 * Delete the C++ surrogate object.
	 * 
	 * @param surrogateFileAbsPath The absolute file path of the surrogate.
	 */
    public void deleteSurrogate(String surrogateFileAbsPath) {
    	Long surrogatePointer = surrogatePointerMap.remove(surrogateFileAbsPath);
    	if(surrogatePointer != null) {
    		nativeDeleteSurrogate(surrogatePointer);
    	}
    }
    
    /////////////
    // PRIVATE //
    /////////////

    private Long getOrLoadSurrogate(String surrogateFileAbsPath) throws DakotaSurrogatesException {
    	if(!jniSurrogatesLibraryLoaded) {
    		loadDakotaJniSurrogatesLibrary();
    	}
    	if(jniSurrogatesLibraryLoaded) {
	    	Long pointer = surrogatePointerMap.get(surrogateFileAbsPath);
	    	if(pointer == null) {
	    		boolean isBinary = isBinary(surrogateFileAbsPath);
	    		long surrogatePointer = loadSurrogate(surrogateFileAbsPath, isBinary);
	    		if(surrogatePointer != 0) {
	    			surrogatePointerMap.put(surrogateFileAbsPath, surrogatePointer);
	    			return surrogatePointer;
	    		} else {
	    			throw new IllegalStateException("Surrogate load for " + surrogateFileAbsPath + " failed.");
	    		}
	    	} else {
	    		return pointer;
	    	}
    	}
    	return null;
    }
    
    /////////////
    // UTILITY //
    /////////////

	/**
	 * Dakota supports two surrogate export formats - textual (".txt") and binary
	 * (".bin").
	 * 
	 * @param surrogateFile The surrogate file.
	 * @return Whether the surrogate is a binary file.
	 */
    public static boolean isBinary(File surrogateFile) {
        return isBinary(surrogateFile.getName());
    }
    
	/**
	 * Dakota supports two surrogate export formats - textual (".txt") and binary
	 * (".bin").
	 * 
	 * @param surrogateFileAbsPath The absolute file path to the surrogate.
	 * @return Whether the surrogate is a binary file.
	 */
    public static boolean isBinary(String surrogateFileAbsPath) {
        return surrogateFileAbsPath.endsWith("." + BIN_FILE_EXTENSION);
    }
    
	/**
	 * Converts a {@link List} of {@link Double}s to a primitive double array.
	 * 
	 * @param originalList The original List of Doubles.
	 * @return The primitive double array, or null if a {@link NullPointerException}
	 *         was encountered.
	 * 
	 */
	private double[] asDoubleArrFromDoubleList(List<Double> originalList) {
		try {
			double[] xArr = new double[originalList.size()];
			for(int i = 0; i < xArr.length; i++) {
				xArr[i] = originalList.get(i);
			}
			return xArr;
		} catch(NullPointerException e) {
			return null;
		}
	}    
    
    //////////////
    // OVERRIDE //
    //////////////
    
    @Override  
    protected void finalize() {   
        for(Entry<String, Long> entry : surrogatePointerMap.entrySet()) {
            Long pointer = entry.getValue();
            if(pointer != null) {
                nativeDeleteSurrogate(pointer);
            }
        }
        surrogatePointerMap.clear();
    }   
    
    ////////////////
    // NATIVE JNI //
    ////////////////
   
	/**
	 * Loads the surrogate into memory. This should be called the first time a
	 * surrogate is accessed (in order to put it into memory), but generally this
	 * method should not be used to access the surrogate.
	 * 
	 * @param surrogateFileAbsPath The absolute path to the surrogate file.
	 * @param isBinary             Whether the surrogate is in binary format.
	 * @return A C++ memory pointer to the surrogate. This long value will be
	 *         meaningless to Java, but we can use it as if it were a hash value to
	 *         look up specific loaded surrogates.
	 */
    private native long loadSurrogate(String surrogateFileAbsPath, boolean isBinary);
     
	/**
	 * Get the ordered list of variable labels of a given surrogate.  
	 * 
	 * @param surrogatePointer A long representing the C++ pointer to the surrogate.
	 * @return An Object array that will contain Strings representing the variable
	 *         labels.
	 */
    private native Object[] getVariables(long surrogatePointer);

	/**
	 * Get the list of response labels of a given surrogate. Typically for Dakota
	 * surrogates, this will be a single response, but the Dakota API technically
	 * allows for multiple responses to be returned.
	 * 
	 * @param surrogatePointer A long representing the C++ pointer to the surrogate.
	 * @return An Object array that will contain Strings representing the variable
	 *         labels.
	 */
    private native Object[] getResponses(long surrogatePointer);
    
    /**
	 * Retrieves a single output value provided by a surrogate given a list of input
	 * values.
	 * 
	 * @param surrogatePointer A long representing the C++ pointer to the surrogate.
	 * @param inputValues      The List of input parameter values for the surrogate.
	 * @return The single output value of the surrogate.
	 */
    private native double value(long surrogatePointer, double[] inputValues);
    
	/**
	 * Delete the C++ surrogate object.
	 * 
	 * @param surrogatePointer A long representing the C++ pointer to the surrogate.
	 */
	private native void nativeDeleteSurrogate(long surrogatePointer);

}
