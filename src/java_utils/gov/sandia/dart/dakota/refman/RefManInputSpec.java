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

import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import gov.sandia.dart.dakota.refman.metadata.InputSpecKeywordMetaData;

/**
 * Store Dakota specification data (from XML or NIDR in reference
 * manual-friendly format). Print subsets of resulting data to Doxygen format.
 * 
 */
public class RefManInputSpec {

	////////////
	// FIELDS //
	////////////

	// container to store final data: a map from keyword (hierarchical) to
	// contents); for now used linked to keep in parsed order for testing (not
	// strictly needed)
	private Map<String, InputSpecKeywordMetaData> spec_data = new LinkedHashMap<>();

	
	/////////////
	// GETTERS //
	/////////////

	public Set<Map.Entry<String, InputSpecKeywordMetaData>> getEntries() {
		return spec_data.entrySet();
	}
	
	public InputSpecKeywordMetaData get(String kwName) {
		return spec_data.get(kwName);
	}

	public boolean containsKey(String kwname) {
		return spec_data.containsKey(kwname);
	}
	

	/////////
	// ADD //
	/////////

	/**
	 * Add a field, value pair at position kw_hier
	 * 
	 * @param kw_hier the hierarchical keyword context from context_string();
	 * @param field key to add
	 * @param value associated value
	 */
	public void addData(String kw_hier, String field, String value) {
		if (spec_data.containsKey(kw_hier)) {
			InputSpecKeywordMetaData kw_md = spec_data.get(kw_hier);
			if(kw_md.fieldAlreadySet(field)) {
				System.out.println("Warning: spec_data multiple insertion of field " + field + " for keyword " + kw_hier);
			} else {
				kw_md.setByField(field, value);
			}
		} else {
			// create the key and insert
			InputSpecKeywordMetaData kw_md = new InputSpecKeywordMetaData();
			kw_md.setByField(field, value);
			spec_data.put(kw_hier, kw_md);
		}
	}
	
	/**
	 * Append data to the "Children" field for immediate children of this keyword
	 * (add a child to the current keyword)
	 * 
	 * @param kw_hier
	 * @param value
	 */
	public void appendChild(String kw_hier, String value) {
		InputSpecKeywordMetaData kw_md = spec_data.get(kw_hier);
		List<String> curr_children = kw_md.getChildren();
		curr_children.add(value);
	}

	/**
	 * Print a hierarhical text file for debugging purposes
	 * @param output_file
	 * @throws IOException
	 */
	public void printDebug(String output_file) throws IOException {
		try(OutputStreamWriter os = new FileWriter(output_file)) {
			for (InputSpecKeywordMetaData kw_md : spec_data.values()) {
				os.append(kw_md.toString());
			}
		}
	}
}
