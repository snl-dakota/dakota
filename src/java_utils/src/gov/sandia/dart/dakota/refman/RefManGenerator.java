/*******************************************************************************
 * Sandia Analysis Workbench Integration Framework (SAW)
 * Copyright 2016 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * This software is distributed under the Eclipse Public License.
 * For more information see the files copyright.txt and license.txt
 * included with the software.
 ******************************************************************************/
package gov.sandia.dart.dakota.refman;

import java.io.File;
import java.io.FileNotFoundException;

import gov.sandia.dart.dakota.nidr.NIDRToRefManSpec;
import gov.sandia.dart.dakota.refman.print.KeywordPageRSTPrinter;
import gov.sandia.dart.dakota.xml.XMLToRefManSpec;

/**
 * Generate the documentation from the topic tree, meta data files, and spec data.<br><br>
 * Supports both Doxygen and ReStructuredText.
 * 
 * @author Brian Adams, Elliott Ridgway
 *
 */
public class RefManGenerator {
	
	////////////
	// FIELDS //
	////////////
	
	private static final String USAGE_MSG = 
		"Usage: tool.java dakota.(xml|input.nspec) metadata_dir output_dir";
	
	private static final int EXPECTED_ARG_COUNT = 3;
	
	////////////
	// PUBLIC //
	////////////
	
	public void run(String args[]) throws Exception {
		if (args.length < EXPECTED_ARG_COUNT || args.length > EXPECTED_ARG_COUNT) {
			System.err.println(USAGE_MSG);
			System.exit(1);
		}
		String input_spec = args[0];
		String kw_md_dir = args[1];
		String output_dir = args[2];
				
		verifyFile(input_spec);
		verifyFile(kw_md_dir);
		verifyFile(output_dir);
		
		RefManInputSpec spec_data = parseIntoSpecTree(output_dir, input_spec);
		RefManMetaData meta_data = new RefManMetaData(kw_md_dir, spec_data);
		
		KeywordPageRSTPrinter printer = new KeywordPageRSTPrinter(); // Change this line to change the type of printer
		printer.setDelayedConversion(false);
		printer.print(output_dir, spec_data, meta_data);
	}
	
	/////////////
	// PRIVATE //
	/////////////
	
	private RefManInputSpec parseIntoSpecTree(String outputDir, String input_spec) throws Exception {
		RefManInputSpec spec_data; 
		if (input_spec.endsWith("xml")) {
			XMLToRefManSpec xml2spec = new XMLToRefManSpec(input_spec);
			xml2spec.parse();
			spec_data = xml2spec.refman_spec_data();
		}
		else if (input_spec.endsWith("nspec")) {
			NIDRToRefManSpec nidr2spec = new NIDRToRefManSpec(input_spec);
			nidr2spec.parse();
			spec_data = nidr2spec.refman_spec_data();
		}
		else {
			throw new IllegalStateException(USAGE_MSG);
		}
		spec_data.printDebug(outputDir + "/DEBUG-nspec.txt");
		return spec_data;
	}
	
	private void verifyFile(String filepath) throws FileNotFoundException {
		File file = new File(filepath);
		if(!file.exists()) {
			throw new FileNotFoundException(filepath + " does not exist");
		}
	}
	
	//////////
	// MAIN //
	//////////
	
	public static void main(String[] args) throws Exception {
		RefManGenerator app = new RefManGenerator();
		app.run(args);
	}
}
