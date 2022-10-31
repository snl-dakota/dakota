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
/**
 * 
 */
package gov.sandia.dart.dakota.refman;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

import gov.sandia.dart.dakota.refman.metadata.RefManKeywordMetaData;

/**
 * @author Brian Adams
 *
 */
public class RefManMetaData {
	
	////////////
	// FIELDS //
	////////////
	
	private Map<String, RefManKeywordMetaData> metadataStore = new LinkedHashMap<>();
	// private RefManTopicTree topicTree; // topic tree for storing keywords associated with a topic
	
	/////////////////
	// CONSTRUCTOR //
	/////////////////
	
	public RefManMetaData(String dirname, RefManInputSpec spec_data) throws IOException {
		parse_metadata_files(dirname, spec_data);
	}
	
	/*
	public RefManMetaData(String dirname, RefManTopicTree topic_tree, RefManInputSpec spec_data) throws IOException {
		topicTree = topic_tree;
		parse_metadata_files(dirname, spec_data);
	}
	*/
	
	/////////////
	// GETTERS //
	/////////////
	
	public String getBlurb(String hier_kw) {
		if(metadataStore.containsKey(hier_kw) && !metadataStore.get(hier_kw).getBlurb().isBlank()) {
			return metadataStore.get(hier_kw).getBlurb();
		}
		System.err.println("Warning: Metadata for keyword " + hier_kw + " has no blurb");
		return "";	
	}
	
	public Set<Map.Entry<String, RefManKeywordMetaData>> getEntries() {
		return metadataStore.entrySet();
	}
	
	public RefManKeywordMetaData getKeywordMetadata(String kwname) {
		return metadataStore.get(kwname);
	}
	
	public boolean containsKey(String kwname) {
		return metadataStore.containsKey(kwname);
	}
	
	///////////
	// PARSE //
	///////////
	
	
	/**
	 * Read data consisting of either DUPLICATE-* (in which case we defer to the DUPLICATE-* file)
	 * OR
	 * Field:: Single line string
	 * Field:: Multi-line
	 *      string
	 * NextField::
	 * 
	 * @param metadata_dir
	 * @param spec_data
	 * @throws IOException
	 */
	private void parse_metadata_files(String metadata_dir, RefManInputSpec spec_data) throws IOException {
		
		// iterate over directory contents
		File mddir = new File(metadata_dir);
		if (!mddir.isDirectory()) {
			System.err.println("Specified metadata path " + metadata_dir + " is not a directory!");
		}
		// the metadata file name mdfile is the hierarchical keyword name
		for (String mdfile: mddir.list()) {
			
			// don't process DUPLICATE- files
			if (mdfile.startsWith("DUPLICATE-") || mdfile.startsWith(".svn"))
				continue;
			
			// Make sure the metadata file is a valid keyword.  If not, warn and skip.
			boolean filename_in_nspec = spec_data.containsKey(mdfile);
			if (!filename_in_nspec) {
				System.err.println("Warning: keyword metadata file " + mdfile + " exists, but no such keyword in .nspec. Skipping.");
				continue;
			}
			
			// get the first line and check for a DUPLICATE tag, then delegate
			try(BufferedReader mdin = new BufferedReader(new InputStreamReader(new FileInputStream(metadata_dir + "/" + mdfile)))) { 
				int max_length_first_line = 4096;
				mdin.mark(max_length_first_line);
				String line = mdin.readLine();
				if (line == null) {
					System.out.println("Warning: empty metadata file " + mdfile);
				}
				else {
					if (line.contains("DUPLICATE-")) {
						// read this keyword's data from the duplicate file
						String dupfile = line.trim();
						File duplicate_file = new File(metadata_dir + "/" + dupfile);
						if (duplicate_file.isFile()) {
							try(BufferedReader dupin = new BufferedReader(new InputStreamReader(new FileInputStream(duplicate_file)))) {
								read_fields(mdfile, dupin);
							}
						}
						else {
							System.err.println("Warning: Missing DUPLICATE metadata file " + duplicate_file);
						}
							
					}
					else {
						// rewind to file start
						mdin.reset();
						read_fields(mdfile, mdin);
					}
				}
			}
		}
	}
	
	// process data for a single keyword
	private void read_fields(String mdfile, BufferedReader mdin) throws IOException {

		// a map from KW name to it's contents map
		RefManKeywordMetaData mdcontents = new RefManKeywordMetaData();
		
		String line = "";
		String field = "";
		String data = "";
		
		while ((line = mdin.readLine()) != null) {
			if(lineStartsWithDakotaSectionHeader(line)) {
				// encountered a new field; print previous data, then parse
				if (!field.isEmpty()) {
					mdcontents.setByField(field, data);
					/* associate this keyword with its topics
					if (field.equals(RefManKeywordMetaData.TOPICS)) {
						append_topics(mdfile, data);
					}
					*/
				}	
				// tokenize on :: (double colon) into field + data
				String [] splitline = line.split(":{2}?", 2);
				field = splitline[0].trim();
				// data may be absent or on next line; not trimming here to preserve user formatting
				if (splitline.length > 1) {
					data = splitline[1] + "\n";
				} else {
					data = "\n";
				}
			}
			else {
				data += line + "\n";
			}
		}
		// the last field found won't be printed in the loop above
		if (!field.isEmpty()) {
			mdcontents.setByField(field, data);
			// associate this keyword with its topics
			/*if (field.equals("Topics")) {
				append_topics(mdfile, data);
			}*/
		}	
		metadataStore.put(mdfile, mdcontents);
	}
	
	/*
	private void append_topics(String kw, String topic_list) {
		// must guard against topicTree pointer; doesn't have to be present
		// tokenize on comma or whitespace including newline and trim to get keyword
		if (!topic_list.isEmpty() && topicTree != null) {
			String[] split_topics = topic_list.split("[,\\s]");
			for (String topic : split_topics) {
				if (!topic.trim().isEmpty()) {
					topicTree.add_subkeyword(topic.trim(), kw);
				}
			}
		}
	}
	*/
	
	public static boolean lineStartsWithDakotaSectionHeader(String line) {
		return line.startsWith("Blurb::") ||
			   line.startsWith("Description::") ||
			   line.startsWith("Topics::") ||
			   line.startsWith("Examples::") ||
			   line.startsWith("Theory::") ||
			   line.startsWith("Faq::") ||
			   line.startsWith("See_Also::");
	}
}
