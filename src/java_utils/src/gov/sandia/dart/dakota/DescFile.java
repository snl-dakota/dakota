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
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Parse labels, and nothing else, out of a Dakota "desc" file. Not even sure this is 100% correct.
 */
public class DescFile {
	private Map<String, String> labels = new HashMap<String, String>();
	private Map<String, String> groups = new HashMap<String, String>();
	private Map<String, String> helps = new HashMap<String, String>();
	private Map<String, String> parentLabels = new HashMap<String, String>();
	public DescFile(String path) throws IOException {
		load(new File(path));
	}

	public String getLabel(String tag) {
		return labels.get(tag);
	}

	public String getGroup(String tag) {
		return groups.get(tag);
	}

	public String getHelp(String tag) {
		return helps.get(tag);
	}
	
	public String getParentLabel(String tag) {
		return parentLabels.get(tag);
	}
	
	private void load(File file) throws IOException {
		BufferedReader reader = new BufferedReader(new FileReader(file));
		String line;
		try {
			String tag = null;
			while ((line = reader.readLine()) != null) {
				line = line.trim();
				if (line.startsWith("DESC")) {				
					processDesc(line, tag);
					tag = null;
				}
				else if (line.startsWith("TAG")) {					
					tag = line;					
					processTag(tag);
				} else if (line.trim().equals(""))
					; 
				else
					throw new IOException("Bad line in DESC file: " + line);
			}
			
		} finally {
			reader.close();
		}
	}
	private static final Pattern pTagGroup = Pattern.compile("TAG\\s+\"([^\"]*)\"\\s+GROUP\\s+\"([^\"]*)\"");
	private void processTag(String tag) {
		Matcher m = pTagGroup.matcher(tag);
		if (m.matches()) {
			String tagName = m.group(1);
			String group = m.group(2);	
			groups.put(tagName, group);
		}		
		
	}
	// TODO Aaargh. Labels can contain nested sets of curly braces!
	private static final Pattern pChoose = Pattern.compile("\\[CHOOSE (.*)\\]");
	private static final Pattern pLabel = Pattern.compile(".*\\{([^}]*)\\}.*\\s+([^\\s]+)\"");
	private static final Pattern pTag = Pattern.compile("TAG \"([^\"]*)\".*");
	private void processDesc(String desc, String tag) {
		Matcher m = pLabel.matcher(desc);
		if (m.matches()) {
			String label = m.group(1);
			String help = m.group(2);
			if (tag != null)
				m = pTag.matcher(tag);
				if (m.matches()) {
					labels.put(m.group(1), label);
					helps.put(m.group(1), help);
				}			
		} else {
			m = pChoose.matcher(desc);
			if (m.find()) {
				String parentLabel = m.group(1);			
				if (tag != null) {
					m = pTag.matcher(tag);
					if (m.matches()) {
						parentLabels.put(m.group(1), parentLabel);
					}
				}				
			}
		}
	}
	public static void main(String[] argv) throws IOException {
		DescFile d = new DescFile("dakota.input.desc");
		for (Map.Entry<String, String> entry: d.labels.entrySet()) {
			System.out.println(entry);
		}
	}
}
