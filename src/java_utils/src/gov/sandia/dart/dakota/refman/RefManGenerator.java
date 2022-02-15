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
import gov.sandia.dart.dakota.refman.print.KeywordPrinter;
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
		"Usage: tool.java dakota.(xml|input.nspec) metadata_dir topic_file topic_metadata_dir topics_intro_file output_dir";
	
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
		// String topic_tree_file = args[2];
		// String topic_md_dir = args[3];
		// String topics_intro_file = args[4];
		String output_dir = args[2];
				
		verifyFile(input_spec);
		verifyFile(kw_md_dir);
		// verifyFile(topic_tree_file);
		// verifyFile(topic_md_dir);
		// verifyFile(topics_intro_file);
		verifyFile(output_dir);
		
		RefManInputSpec spec_data = parseIntoSpecTree(output_dir, input_spec);
		// RefManTopicTree topic_tree = parseTopicTree(topic_tree_file);
		RefManMetaData meta_data = new RefManMetaData(kw_md_dir, spec_data);
		
		KeywordPrinter printer = new KeywordPageRSTPrinter(); // Change this line to change the type of printer
		// printTopicPages(topic_tree, meta_data, topic_md_dir, topics_intro_file);
		printer.print(output_dir, spec_data, meta_data);
	}
	
	////////////////
	// INPUT SPEC //
	////////////////
	
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

	////////////
	// TOPICS //
	////////////
	/*
	private RefManTopicTree parseTopicTree(
			String outputDir, String topic_tree_file) throws IOException {
		RefManTopicTree topic_tree = new RefManTopicTree(topic_tree_file);
		OutputStreamWriter topic_debug = new FileWriter(outputDir + "/DEBUG-topics.txt");
		topic_tree.print_debug(topic_debug);
		topic_debug.close();
		return topic_tree;
	}
	
	private void printTopicPages(
			String outputDir,
			RefManTopicTree topic_tree, RefManMetaData meta_data, String topic_md_dir,
			String topics_intro_file) throws IOException {
		
		// print now that topics inserted from keywords topics appear in wrong order --
		// shouldn't matter
		OutputStreamWriter topic_os = new FileWriter(outputDir + "/DakotaTopics.dox");
		topic_tree.print_doxygen(topic_md_dir, topics_intro_file, meta_data, topic_os);
		topic_os.close();
	}
	*/
	
	/////////////
	// UTILITY //
	/////////////
	
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
