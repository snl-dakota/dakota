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
package gov.sandia.dart.dakota;

import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.Map.Entry;

// generate the Doxygen from the topic tree, meta data files, and spec data
public class RefManGenDoxygen {

	public static void main(String[] args) throws Exception {
		// TODO Auto-generated method stub

		String usage_msg = 
				"Usage: tool.java dakota.(xml|input.nspec) metadata_dir topic_file topic_metadata_dir topics_intro_file output_dir";
		
		if (args.length < 6 || args.length > 6) {
			System.err.println(usage_msg);
			System.exit(1);
		}
		String input_spec = args[0];
		String kw_md_dir = args[1];
		String topic_tree_file = args[2];
		String topic_md_dir = args[3];
		String topics_intro_file = args[4];
		String output_dir = args[5];
				
		// TODO: check for existence of all the above
		
		// Now parse the XML or NIDR specification into a spec data tree
		// TODO: count and report keyword stats
		RefManInputSpec spec_data = new RefManInputSpec(); 
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
			System.err.println(usage_msg);
			System.exit(1);
		}
		spec_data.printDebug(output_dir + "/DEBUG-nspec.txt");
		
		// parse the topic tree file into a data structure before parsing keywords from spec
		// so we can insert keywords into topics
		RefManTopicTree topic_tree = new RefManTopicTree(topic_tree_file);
		OutputStreamWriter topic_debug = new FileWriter(output_dir + "/DEBUG-topics.txt");
		topic_tree.print_debug(topic_debug);
		topic_debug.close();
		
		// parse the developer-generated keyword metadata into a data structure and insert topics into the RefManTopicTree
		RefManMetaData meta_data = new RefManMetaData(kw_md_dir, topic_tree, spec_data);
		// for debugging only now
		//meta_data.print_doxygen();

		// print now that topics inserted from keywords
		// topics appear in wrong order -- shouldn't matter
		OutputStreamWriter topic_os = new FileWriter(output_dir + "/DakotaTopics.dox");
		topic_tree.print_doxygen(topic_md_dir, topics_intro_file, meta_data, topic_os);
		topic_os.close();
		
		// Iterate based on the spec, not the data files.  Then appeal to the data files for information
		OutputStreamWriter kw_os = new FileWriter(output_dir + "/DakotaKeywords.dox");
		
		// print Doxygen for all keywords, managing the sequencing of output
		for (Entry<String, RefManInputSpec.KeywordMetaData> mdentry: spec_data.get_entries()) {
			
			// the data in the spec
			String kwname = mdentry.getKey();
			
			// for each, check whether meta data exists, if not, warn
			boolean has_metadata = meta_data.entry_exists(kwname);
			if (!has_metadata) {
				System.err.println("Warning: Missing metadata entry for keyword " + kwname);
			}
			
			doxy_page_title(kwname, kw_os);
			doxy_breadcrumbs(kwname, kw_os);
			
			if (has_metadata) {
				meta_data.doxy_blurb(kwname, kw_os);
				meta_data.doxy_topics(kwname, kw_os);
			}
			
			// need meta data for the blurb
			spec_data.doxy_print(kwname, kw_os, meta_data);
			
			if (has_metadata) {
				meta_data.doxy_verbatim(kwname, "Description", "Description", kw_os);
				meta_data.doxy_verbatim(kwname, "Examples", "Examples", kw_os);
				meta_data.doxy_verbatim(kwname, "Theory", "Theory", kw_os);
				meta_data.doxy_see_also(kwname, kw_os);			
				meta_data.doxy_verbatim(kwname, "Faq", "FAQ", kw_os);
			}
		}
		kw_os.close();
	}
	
	static public void doxy_page_title(String hierarchical_kwname, OutputStreamWriter os) throws IOException {		
		String [] split_kw = hierarchical_kwname.split("-");
		String name = split_kw[split_kw.length - 1];
		os.append("\\page " + hierarchical_kwname + " " + name + "\n");
	}
	
	
	// TODO: Better tokenizing here
	static public void doxy_breadcrumbs(String hierarchical_kwname, OutputStreamWriter os) throws IOException {
		String bc_text = "";
		
		bc_text += "<hr>\n";
		bc_text += "<div id=\"nav-path\" class=\"navpath\">\n"; 
		bc_text += " <ul>\n";
		bc_text += "  <li>\\ref keywords</li>\n"; 
		
		String [] split_kw = hierarchical_kwname.split("-");
		String bc_kw = "";
		for (String bc: split_kw) {
			bc_kw += bc;
			bc_text += "  <li>\\ref " + bc_kw + "</li>\n";
			bc_kw += "-";
		}
		bc_text += " </ul>\n";
		bc_text += "</div>\n"; 
		bc_text += "<hr>\n\n";

		os.append(bc_text);
	}
	
}
