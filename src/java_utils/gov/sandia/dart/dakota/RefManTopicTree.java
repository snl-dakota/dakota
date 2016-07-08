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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.SortedSet;
import java.util.Stack;
import java.util.TreeSet;

/**
 * @author briadam
 * 
 */
public class RefManTopicTree {
	public RefManTopicTree() {
		// don't need to preserve ordering of the topic entries since the root
		// topic knows its subtopics; use HashMap for efficiency
		topicList = new HashMap<String, TopicEntry>();
	}

	public RefManTopicTree(String filename) throws IOException {
		// don't need to preserve ordering of the topic entries since the root
		// topic knows its subtopics; use HashMap for efficiency
		topicList = new HashMap<String, TopicEntry>();
		read_topic_file(filename);
	}

	
	// add an empty topic (no sub-topics)
	void add_topic(String topic) {
		if (!topicList.containsKey(topic)) {
			TopicEntry empty_entry = new TopicEntry();
			topicList.put(topic, empty_entry);
		}
	}

	// add the child topic to the parent, creating parent and/or child if needed
	void add_subtopic(String parent_topic, String child_topic) {
		// associate child with parent
		if (topicList.containsKey(parent_topic)) {
			topicList.get(parent_topic).subTopics.add(child_topic);
		} else {
			TopicEntry new_entry = new TopicEntry();
			new_entry.subTopics.add(child_topic);
			topicList.put(parent_topic, new_entry);
		}
		// add child if needed
		if (!topicList.containsKey(child_topic))
			add_topic(child_topic);
	}

	// associate a keyword with the given topic, warning and not inserting if DNE
	void add_subkeyword(String topic, String keyword) {
		if (topicList.containsKey(topic)) {
			topicList.get(topic).subKeywords.add(keyword);
		} else {
			System.err.println("Warning: Topic " + topic
					+ " missing from TopicTree.  Specfied by keyword " + keyword
					+ ".");
//			TopicEntry new_entry = new TopicEntry();
//			new_entry.subKeywords.add(keyword);
//			topicList.put(topic, new_entry);
		}
	}

	// debugging function to print the topics with their subtopics
	void print_debug(OutputStreamWriter os) throws IOException {
		for (Map.Entry<String, TopicEntry> topic : topicList.entrySet()) {
			String topic_name = topic.getKey();
			TopicEntry entry = topic.getValue();
			os.append("topic-" + topic_name + "\n");
			for (String st : entry.subTopics)
				os.append("  " + st + "\n");
		}
	}

	// print the topic index to Doxygen format
	void print_doxygen(String topic_md_dir, String topic_intro_file, RefManMetaData kw_metadata, OutputStreamWriter os) throws IOException {
		// print topic pages in no particular order
		for (Map.Entry<String, TopicEntry> topic : topicList.entrySet()) {
			String topic_name = topic.getKey();
			TopicEntry entry = topic.getValue();

			if (topic_name.equals("topics")) {
				// first print the header page, with content from the file that introduces the topic
				File topic_md_file = new File(topic_intro_file);
				if (topic_md_file.isFile()) {
					BufferedReader mdin = new BufferedReader(new InputStreamReader(new FileInputStream(topic_md_file))); 
					String line;
					while ((line = mdin.readLine()) != null) {
						os.append(line + "\n");
					}
					mdin.close();
				}
				else
					System.err.println("Warning: Missing topic metadata file " + topic_md_file);
				os.append("\n");			
				
				for (String subtopic : entry.subTopics)
					os.append("- \\subpage topic-" + subtopic + "\n");
				os.append("\n<!-----Topics List---------------------------->\n");
			} 
			else {

				StringBuilder topic_page = new StringBuilder();

				topic_page.append("<!-------------------------------------------->\n");
				topic_page.append("\\page topic-" + topic_name + " " + topic_name + "\n");
				topic_page.append("<h2>Description</h2>\n");
				
				// Read and cat for example: TopicMetadataFile-package_npsol
				File topic_md_file = new File(topic_md_dir + "/topic-" + topic_name);
				if (topic_md_file.isFile()) {
					BufferedReader mdin = new BufferedReader(new InputStreamReader(new FileInputStream(topic_md_file))); 
					String line;
					while ((line = mdin.readLine()) != null) {
						topic_page.append(line + "\n");
					}
					mdin.close();
				}
				else
					System.err.println("Warning: Missing topic metadata file " + topic_md_file);
		
				topic_page.append("\n<h2>Related Topics</h2>\n");
				for (String subtopic : entry.subTopics)
					topic_page.append("- \\subpage topic-" + subtopic + "\n");
				topic_page.append("\n");
				
				topic_page.append("<h2>Related Keywords</h2>\n");
				for (String subkw : entry.subKeywords) {
					String blurb = kw_metadata.get_blurb(subkw);
					topic_page.append("- \\ref " + subkw + " : " + blurb.trim() + "\n\n");
				}
				os.append(topic_page + "\n");
			}
		}
		os.flush();
	}

	private
	
	/// read the space-indented topic file and create the topic tree
	void read_topic_file(String filename) throws IOException {
		
		// create the root node, called "topics"
		int curr_depth = -1;
		Stack<String> context = new Stack<String>();
		context.push("topics");
		add_topic(context.peek());
		
		BufferedReader mdin = new BufferedReader(new InputStreamReader(new FileInputStream(filename))); 
			
		String line;
		while ((line = mdin.readLine()) != null) {

			int leading_spaces = 0;
			for (char c : line.toCharArray())
				if (Character.isWhitespace(c)) 
					++leading_spaces;
				else
					break;
			
			String child = line.trim();
			//System.out.println("topic: " + child + ", indent= " + leading_spaces + ", curr_depth = " + curr_depth);
			
			if (leading_spaces > curr_depth) {
				add_subtopic(context.peek(), child);
				context.push(child);
			} 
			else if (leading_spaces == curr_depth) {
				// remove the peer as it's not the context
				context.pop();
				add_subtopic(context.peek(), child);
				context.push(child);
			}
			else if (leading_spaces < curr_depth) {
				// we might move out more than one level
				for (int i = leading_spaces; i<=curr_depth; ++i) 
					context.pop();
				add_subtopic(context.peek(), child);
				context.push(child);
			}				
			curr_depth = leading_spaces;
		}
		mdin.close();
	}
	
	// map from topics to subtopics: a flat list of topics for fast lookup
	// could use a linked hash map for subtopics to enforce uniqueness
	Map<String, TopicEntry> topicList;

	// a topic entry consists of subtopics and keywords that relate to the topic
	private class TopicEntry {
		public 
		
		TopicEntry() {
			subTopics = new ArrayList<String>();
			subKeywords = new TreeSet<String>();
		}

		private
		
		// sub-topics are ordered by insertion so the print in correct order
		ArrayList<String> subTopics;
		// sub-keywords are ordered alphabetically since they may get introduced
		// in random order during parsing
		SortedSet<String> subKeywords;
	}
}
