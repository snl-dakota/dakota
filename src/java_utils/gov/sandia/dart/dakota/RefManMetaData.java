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
package gov.sandia.dart.dakota;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

/**
 * @author briadam
 *
 */
public class RefManMetaData {

	public
	
	RefManMetaData() {
		metadataStore = new LinkedHashMap<String, KeywordMetaData>();
		topicTree = null;
	}
	
	// parse metadata from provided directory
	RefManMetaData(String dirname, RefManInputSpec spec_data) throws IOException {
		metadataStore = new LinkedHashMap<String, KeywordMetaData>();
		topicTree = null;
		parse_metadata_files(dirname, spec_data);
	}
	
	// parse metadata from provided directory
	RefManMetaData(String dirname, RefManTopicTree topic_tree, RefManInputSpec spec_data) throws IOException {
		metadataStore = new LinkedHashMap<String, KeywordMetaData>();
		topicTree = topic_tree;
		parse_metadata_files(dirname, spec_data);
	}
	
	String get_blurb(String hier_kw) {
		if (metadataStore.containsKey(hier_kw))
			if (metadataStore.get(hier_kw).containsKey("Blurb"))
				return metadataStore.get(hier_kw).get("Blurb");
		System.err.println("Warning: Metadata for keyword " + hier_kw + " has no blurb");
		return "";	
	}
	
	public Set<Map.Entry<String, KeywordMetaData>> get_entries() {
		return metadataStore.entrySet();
	}
	
	
	public boolean entry_exists(String kwname) {
		return metadataStore.containsKey(kwname);
	}
	
//	private KeywordMetaData get_mdcontents(String kwname) {
//		return metadataStore.get(kwname);
//	}

		
	
	// write a doxygen page for each keyword
//	void print_doxygen(NIDRToRefManSpec spec_data) {
//		for (Map.Entry<String, KeywordMetaData> mdentry: metadataStore.entrySet()) {
//			String kwname = mdentry.getKey();
//			KeywordMetaData mdcontents = mdentry.getValue();
//		
////			doxy_page_title(kwname);
////			doxy_breadcrumbs(kwname);
////			doxy_blurb(mdcontents);
////			doxy_topics(mdcontents);
////			doxy_spec(mdcontents, spec_data);
////			doxy_verbatim(mdcontents, "Description", "Description");
////			doxy_verbatim(mdcontents, "Examples", "Examples");
////			doxy_verbatim(mdcontents, "Theory", "Theory");
////			doxy_see_also(mdcontents);			
////			doxy_verbatim(mdcontents, "Faq", "FAQ");	
//		}
	//}
	
	
	private

	// Read data consisting of either 
	//   DUPLICATE-* (in which case we defer to the DUPLICATE-* file)
	// OR
	//   Field:: Single line string
	//   Field:: Multi-line
	//     string
	//   NextField::
	void parse_metadata_files(String metadata_dir, RefManInputSpec spec_data) throws IOException {
		
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
			boolean filename_in_nspec = spec_data.entry_exists(mdfile);
			if (!filename_in_nspec) {
				System.err.println("Warning: keyword metadata file " + mdfile + " exists, but no such keyword in .nspec. Skipping.");
				continue;
			}
			
			// get the first line and check for a DUPLICATE tag, then delegate
			BufferedReader mdin = new BufferedReader(new InputStreamReader(new FileInputStream(metadata_dir + "/" + mdfile))); 
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
						BufferedReader dupin = new BufferedReader(new InputStreamReader(new FileInputStream(duplicate_file)));
						read_fields(mdfile, dupin);
						dupin.close();
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
			mdin.close();
		}
	}

	
	// process data for a single keyword
	private void read_fields(String mdfile, BufferedReader mdin) throws IOException {

		// a map from KW name to it's contents map
		KeywordMetaData mdcontents = new KeywordMetaData();
		
		//System.out.println("Processing metadata file: " + metadata_dir + "/" + f);
		String line, field = "", data = "";
		while ((line = mdin.readLine()) != null) {
			if (line.contains("::")) {
				// encountered a new field; print previous data, then parse
				if (!field.isEmpty()) {
					mdcontents.put(field, data);
					// associate this keyword with its topics
					if (field.equals("Topics"))
						append_topics(mdfile, data);
				}	
				// tokenize on :: (double colon) into field + data
				String [] splitline = line.split(":{2}?", 2);
				field = splitline[0].trim();
				// data may be absent or on next line; not trimming here to preserve user formatting
				if (splitline.length > 1)
					data = splitline[1] + "\n";
				else
					data = "\n";
			}
			else
				data += line + "\n";
		}
		// the last field found won't be printed in the loop above
		if (!field.isEmpty()) {
			mdcontents.put(field, data);
			// associate this keyword with its topics
			if (field.equals("Topics"))
				append_topics(mdfile, data);
		}	
		metadataStore.put(mdfile, mdcontents);
	}
	
	private void append_topics(String kw, String topic_list) {
		// must guard against topicTree pointer; doesn't have to be present
		// tokenize on comma or whitespace including newline and trim to get keyword
		if (!topic_list.isEmpty() && topicTree != null) {
			String [] split_topics = topic_list.split("[,\\s]");
			for (String topic: split_topics)
				if (!topic.trim().isEmpty())
					topicTree.add_subkeyword(topic.trim(), kw);
		}
	}
	
	public void doxy_blurb(String kw_name, OutputStreamWriter os) throws IOException {
		KeywordMetaData mdcontents = metadataStore.get(kw_name);
		String blurb_text = "";
		if (mdcontents.containsKey("Blurb")) {
			blurb_text = mdcontents.get("Blurb").trim();
			if (blurb_text.isEmpty())
				blurb_text = "TODO: Need a Blurb\n";
		}
		else
			blurb_text = "TODO: Need a Blurb\n";
		os.append(blurb_text + "\n\n");
	}
	
	// print topics if present
	public void doxy_topics(String kw_name, OutputStreamWriter os) throws IOException {
		KeywordMetaData mdcontents = metadataStore.get(kw_name);
		StringBuilder topic_text = new StringBuilder();
		if (mdcontents.containsKey("Topics")) {
			String topic_list = mdcontents.get("Topics").trim();
			if (!topic_list.isEmpty()) {
				topic_text.append("<h2>Topics</h2>\n");
				topic_text.append("This keyword is related to the topics:\n");		
			
				String [] split_topics = topic_list.split(",");
				for (String topic: split_topics) {
					topic_text.append("- \\ref topic-" + topic.trim() + "\n");
				}
				os.append(topic_text + "\n\n");
			}

		}
	}

	// print see also if present
	public void doxy_see_also(String kw_name, OutputStreamWriter os) throws IOException {
		KeywordMetaData mdcontents = metadataStore.get(kw_name);
		StringBuilder sa_text = new StringBuilder();
		if (mdcontents.containsKey("See_Also")) {
			String sa_list = mdcontents.get("See_Also").trim();
			if (!sa_list.isEmpty()) {
				sa_text.append("<h2>See Also</h2>\n");
				sa_text.append("These keywords may also be of interest:\n");		
				String [] split_sa = sa_list.split(",");
				for (String sa: split_sa) {
					sa_text.append("- \\ref " + sa.trim() + "\n");
				}
				os.append(sa_text + "\n\n");
			}

		}
	}
	
	// handler for verbatim paste-in for description, examples, theory, FAQ
	public void doxy_verbatim(String kw_name, String key, String div_name, OutputStreamWriter os)  throws IOException {
		KeywordMetaData mdcontents = metadataStore.get(kw_name);
		if (mdcontents.containsKey(key)) {
			String md_text = mdcontents.get(key).trim();
			if (!md_text.isEmpty()) {
				StringBuilder verbatim_text= new StringBuilder("<h2>" + div_name + "</h2>\n");
				verbatim_text.append(md_text);
				os.append(verbatim_text + "\n\n");
			}
		}
	}


	
	// the developer written metadata for a keyword; keys are fields like Alias, Blurb, Theory
	// values are associated text
	@SuppressWarnings("serial")
	static class KeywordMetaData extends LinkedHashMap<String, String> {}
	
	// map from hierarchial keyword name to 
	// TODO: not clear why we need Linked; ordering shouldn't matter
	Map<String, KeywordMetaData> metadataStore;
	
	// topic tree for storing keywords associated with a topic
	RefManTopicTree topicTree;
	
	
}
