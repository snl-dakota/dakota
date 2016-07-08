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
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

/**
 * Store Dakota specification data (from XML or NIDR in reference
 * manual-friendly format). Print subsets of resulting data to Doxygen format.
 * 
 */
public class RefManInputSpec {

	// KeywordMetaData is a map with entries, e.g.,
	// Keyword_Hierarchy (environment-tabular_data-annotated)
	// Name
	// Alias
	// Argument
	// Children (\n separated string)
	// We keep this insertion ordered with Linked for printing purposes
	@SuppressWarnings("serial")
	static class KeywordMetaData extends LinkedHashMap<String, String> {
	}

	// container to store final data: a map from keyword (hierarchical) to
	// contents); for now used linked to keep in parsed order for testing (not
	// strictly needed)
	private Map<String, KeywordMetaData> spec_data = new LinkedHashMap<String, KeywordMetaData>();

	// -----
	// Accessors
	// -----

	public Set<Map.Entry<String, KeywordMetaData>> get_entries() {
		return spec_data.entrySet();
	}

	public boolean entry_exists(String kwname) {
		return spec_data.containsKey(kwname);
	}

	// -----
	// Data add functions
	// -----

	// Append data to the "Children" field for immediate children of this
	// keyword (add a child to the current keyword)
	public void appendChild(String kw_hier, String value) {

		// the hierarchical keyword context
		// String kw_hier = context_string();
		String field = "Children";

		if (spec_data.containsKey(kw_hier)) {
			KeywordMetaData kw_md = spec_data.get(kw_hier);
			if (kw_md.containsKey(field)) {
				// append to existing child data
				String curr_children = kw_md.get(field);
				curr_children += "\n" + value;
				kw_md.put(field, curr_children);
			} else {
				kw_md.put(field, value);
			}
		} else {
			// create the key and insert
			KeywordMetaData kw_md = new KeywordMetaData();
			kw_md.put(field, value);
			spec_data.put(kw_hier, kw_md);
		}
	}

	/**
	 * Add a field, value pair at position kw_hier
	 * 
	 * @param kw_hier
	 *            : the hierarchical keyword context from context_string();
	 * @param field
	 *            : key to add
	 * @param value
	 *            : associated value
	 */
	public void addData(String kw_hier, String field, String value) {
		// all fields should be single entry except Children
		if (spec_data.containsKey(kw_hier)) {
			KeywordMetaData kw_md = spec_data.get(kw_hier);
			if (kw_md.containsKey(field))
				System.out
						.println("Warning: spec_data multiple insertion of field "
								+ field + " for keyword " + kw_hier);
			else
				kw_md.put(field, value);
		} else {
			// create the key and insert
			KeywordMetaData kw_md = new KeywordMetaData();
			kw_md.put(field, value);
			spec_data.put(kw_hier, kw_md);
		}
	}

	// print a hierarhical text file for debugging purposes
	public void printDebug(String output_file) throws IOException {
		OutputStreamWriter os = new FileWriter(output_file);
		for (KeywordMetaData kw_md : spec_data.values()) {
			for (Map.Entry<String, String> entry : kw_md.entrySet()) {
				if (entry.getKey().equals("Children"))
					os.append(entry.getValue() + "\n");
				else
					os.append(entry.getKey() + ":: " + entry.getValue() + "\n");
			}
			os.append("\n");
		}
		os.close();
	}

	// -----
	// Doxygen print functions
	// -----

	// BMA TODO: replace the following 3 lookups with a convenience function
	// private String retrieve_string(String lookup_kwname, String
	// default_string) {
	// }

	// print the doxy spec for this KW, including alias, arguments, and spec
	// table
	public void doxy_print(String kwname, OutputStreamWriter kw_os,
			RefManMetaData meta_data) throws IOException {
		kw_os.append("<h2>Specification</h2>\n");

		String alias = "none";
		if (spec_data.containsKey(kwname)
				&& spec_data.get(kwname).containsKey("Alias")) {
			alias = spec_data.get(kwname).get("Alias");
			if (alias.trim().isEmpty())
				alias = "none";
		}
		kw_os.append("\n<p><b>Alias: </b>" + alias + "</p>\n");

		String argument = "none";
		if (spec_data.containsKey(kwname)
				&& spec_data.get(kwname).containsKey("Argument")) {
			argument = spec_data.get(kwname).get("Argument");
			if (argument.trim().isEmpty())
				argument = "none";
		}
		kw_os.append("<p><b>Argument(s): </b>" + argument + "</p>\n");

		String default_value = "";
		if (spec_data.containsKey(kwname)
				&& spec_data.get(kwname).containsKey("Default")) {
			default_value = spec_data.get(kwname).get("Default");
			if (default_value.trim().isEmpty())
				default_value = "";
		}
		if (!default_value.isEmpty())
			kw_os.append("<p><b>Default: </b>" + default_value + "</p>\n");

		if (spec_data.containsKey(kwname)
				&& spec_data.get(kwname).containsKey("Children")) {

			String children = spec_data.get(kwname).get("Children");
			String[] lines = children.split("\\n");

			// save the indices of each new context (required/optional keyword
			// or group)
			// TODO: store these in a better data structure
			int index = 0;
			ArrayList<String> category = new ArrayList<String>();
			ArrayList<Integer> start_inds = new ArrayList<Integer>(), end_inds = new ArrayList<Integer>();
			// first find the indices of the relevant rows (beginning
			// Choose_One, ending with Req/Opt or Choose)
			boolean in_group = false;
			for (String line : lines) {
				if (line.contains("_Choose_One")
						|| line.startsWith("Required_Keyword")
						|| line.startsWith("Optional_Keyword")) {
					// Save optional or required in category
					// Leave the whole line for now as potentially includes group label
					//category.add(line.split(":")[0]);
					category.add(line);
					start_inds.add(index);
					if (in_group)
						end_inds.add(index - 1);
					in_group = true;
				}
				++index;
			}
			if (in_group)
				end_inds.add(index - 1);

			// iterate over the children (some choose groups, some single
			// keyword)
			int num_children = start_inds.size();

			// print header for the spec table
			String header = "<table class=\"spec-table\">\n";
			header += " <tr>\n";
			header += "  <th width=\"2%\" class=\"border-heavy-right\"> </th>\n";
			header += "  <th width=\"10%\">Required/Optional</th>\n";
			header += "  <th width=\"15%\">Description of Group</th>\n";
			header += "  <th width=\"20%\">Dakota Keyword</th>\n";
			header += "  <th width=\"52%\">Dakota Keyword Description</th>\n";
			header += " </tr>\n";
			kw_os.append(header);

			int group_num = 0;
			for (int ci = 0; ci < num_children; ++ci) {

				int start_row = start_inds.get(ci);
				int end_row = end_inds.get(ci);
				// number of sub keywords, not counting the *_Choose_One label
				int num_sub_kw = end_inds.get(ci) - start_inds.get(ci);
				String group_type = category.get(ci);

				if (group_type.contains("Choose_One")) {
					if (num_sub_kw < 1)
						System.err
								.println("Warning (doxy_print): found choose group with no items.");
					++group_num;
					String sg = doxy_format_subgroup(group_type, lines,
							start_row, end_row, kwname, meta_data, group_num);
					kw_os.append(sg);
				} else if (group_type.contains("Keyword")) {
					if (num_sub_kw != 0)
						System.err
								.println("Warning (doxy_print): found bare keyword with more than one row.");
					String skw = doxy_format_subkw(group_type,
							lines[start_row], kwname, meta_data);
					kw_os.append(skw);
				} else {
					System.err
							.println("Warning (doxy_print): unexpected keyword subgroup type "
									+ group_type + ".");
				}
			}

			// footer for the spec table
			String footer = "</table>\n";
			kw_os.append(footer);

		}

	}

	// print a formatted table entry for a single required or optional keyword
	// (*_Keyword followed by keyword name)
	private String doxy_format_subkw(String group_type, String line,
			String kwname, RefManMetaData meta_data) {

		// Required or Optional
		String reqopt = group_type.split("_", 2)[0];

		String[] splitline = line.split(":{2}?", 2);
		// data may be absent or on next line; not trimming here to preserve
		// user formatting
		String subkw = "";
		if (splitline.length > 1)
			subkw = splitline[1].trim();

		String subkw_blurb = meta_data.get_blurb(kwname + "-" + subkw);
		String subkw_ref = "\\subpage " + kwname + "-" + subkw;

		// print the table entries for this sub kw
		String table_subgroup = new String();
		table_subgroup += " <tr>\n";
		table_subgroup += "  <th class=\"border-heavy-right\"></th>\n";
		table_subgroup += "  <td class=\"border-light-right\" colspan=\"2\"><strong>"
				+ reqopt + "</strong></td>\n";
		table_subgroup += "  <td class=\"border-light-right\"> " + subkw_ref
				+ "</td>\n";
		table_subgroup += "  <td> " + subkw_blurb + "</td>\n";
		table_subgroup += " </tr>\n";

		return table_subgroup;

	}

	// print a formatted table entry for a choose group (leading line followed
	// by subkeywords)
	private String doxy_format_subgroup(String group_type, String[] lines,
			int start, int end, String kwname, RefManMetaData meta_data,
			int group_num) {

		String table_subgroup = new String();

		// Required or Optional
		String reqopt = group_type.split("_", 2)[0];

		// default group descriptor (conservative parsing until we get a better
		// data structure)
		String group_title = "Group " + group_num;
		if (group_type.contains(":: ")) {
			String tokens[] = group_type.split(":: ", 2);
			if (tokens.length > 1) {
				String group_label = tokens[1].trim();
				if (!group_label.isEmpty()) 
					group_title = group_label + " (Group " + group_num + ")";
			}
		}

		int num_subkw = end - start;

		for (int index = 1; index <= num_subkw; ++index) {

			// skip the *_Choose line
			String subkw = lines[start + index].trim();
			String subkw_blurb = meta_data.get_blurb(kwname + "-" + subkw);
			String subkw_ref = "\\subpage " + kwname + "-" + subkw;

			if (index == 1) {
				// first row
				table_subgroup += " <tr>\n";
				table_subgroup += "  <th class=\"border-heavy-right\" rowspan=\""
						+ num_subkw + "\" > </th>\n";
				table_subgroup += "  <td class=\"border-light-right vert-align-mid\" rowspan=\""
						+ num_subkw
						+ "\"><strong>"
						+ reqopt
						+ "</strong><div><em>(Choose One)</em></div></td>\n";
				table_subgroup += "  <td class=\"border-light-right vert-align-mid\" rowspan=\""
						+ num_subkw
						+ "\"><strong>"
						+ group_title
						+ "</strong></td>\n";
				table_subgroup += "  <td class=\"border-light-right border-light-bottom\">"
						+ subkw_ref + "</td>\n";
				table_subgroup += "  <td class=\"border-light-bottom\">"
						+ subkw_blurb + "</td>\n";
				table_subgroup += " </tr>\n";
			} else if (index == num_subkw) {
				// last row
				table_subgroup += " <tr>\n";
				table_subgroup += "  <td class=\"border-light-right\">"
						+ subkw_ref + "</td>\n";
				table_subgroup += "  <td>" + subkw_blurb + "</td>\n";
				table_subgroup += " </tr>\n";
			} else {
				// middle row
				table_subgroup += " <tr>\n";
				table_subgroup += "  <td class=\"border-light-right border-light-bottom\">"
						+ subkw_ref + "</td>\n";
				table_subgroup += "  <td class=\"border-light-bottom\">"
						+ subkw_blurb + "</td>\n";
				table_subgroup += " </tr>\n";
			}
		}

		return table_subgroup;
	}

	// deprecated function
	// private void doxy_print_kws(String kwname, OutputStreamWriter kw_os,
	// RefManMetaData meta_data, String reqopt) throws IOException {
	//
	// if (spec_data.containsKey(kwname) &&
	// spec_data.get(kwname).containsKey("Children")) {
	//
	// String children = spec_data.get(kwname).get("Children");
	//
	// // print all required KW together
	// boolean found_required = false;
	// int index = 0;
	// for (String line: children.split("\\n")) {
	// if (line.startsWith(reqopt + "_Keyword")) {
	// ++index;
	// if (!found_required) {
	// found_required = true;
	// kw_os.append("<table class=\"keyword\">\n");
	// kw_os.append(" <tr>\n");
	// kw_os.append("  <th> </th>\n");
	// kw_os.append("  <th class=\"kwtype\" colspan=\"2\">" + reqopt +
	// " Keywords</th>\n");
	// kw_os.append(" </tr>\n");
	// }
	// String [] splitline = line.split(":{2}?", 2);
	// //field = splitline[0].trim();
	// // data may be absent or on next line; not trimming here to preserve user
	// formatting
	// String subkw = "";
	// if (splitline.length > 1)
	// subkw = splitline[1].trim();
	//
	// String subkw_blurb = meta_data.get_blurb(kwname + "-" + subkw);
	// kw_os.append("<tr>\n");
	// kw_os.append("  <td>" + index + "</td>\n");
	// kw_os.append("  <td class=\"kwname\">\\subpage " + kwname + "-" + subkw +
	// "</td>\n");
	// kw_os.append("  <td class=\"emph-italic\">" + subkw_blurb + "</td>\n");
	// kw_os.append(" </tr>\n");
	// }
	// }
	// if (found_required)
	// kw_os.append("</table>\n");
	// }
	// }

	// deprecated function
	// private void doxy_print_choose(String kwname, OutputStreamWriter kw_os,
	// RefManMetaData meta_data) throws IOException {
	//
	// if (spec_data.containsKey(kwname) &&
	// spec_data.get(kwname).containsKey("Children")) {
	//
	// String children = spec_data.get(kwname).get("Children");
	// String[] lines = children.split("\\n");
	//
	// int index = 0;
	// ArrayList<Integer> start_inds = new ArrayList<Integer>(), end_inds = new
	// ArrayList<Integer>();
	// // first find the indices of the relevant rows (beginning Choose_One,
	// ending with Req/Opt or Choose)
	// boolean in_choose = false;
	// for (String line: lines) {
	// if (line.contains("_Choose_One")) {
	// start_inds.add(index);
	// if (in_choose)
	// end_inds.add(index-1);
	// in_choose = true;
	// }
	// if (in_choose && (line.startsWith("Required_Keyword") ||
	// line.startsWith("Optional_Keyword"))) {
	// end_inds.add(index-1);
	// in_choose = false;
	// }
	// ++index;
	// }
	// if (in_choose)
	// end_inds.add(index-1);
	//
	// // no group names for now
	// String group_name = "";
	//
	// int num_choose = start_inds.size();
	// for (int ci = 0; ci<num_choose; ++ci) {
	// int row = start_inds.get(ci);
	//
	// // print header with Choose One
	// String [] splitline = lines[row].split(":{2}?", 2);
	// String group_type = splitline[0].trim().replace("_", " ");
	// int num_sub_kw = end_inds.get(ci) - start_inds.get(ci);
	//
	// kw_os.append("<table class=\"keyword\">\n");
	// kw_os.append(" <tr>\n");
	// kw_os.append("  <th> </th>\n");
	// kw_os.append("  <th class=\"kwtype\" colspan=\"2\">" + group_type +
	// "</th>\n");
	// kw_os.append(" </tr>\n");
	//
	// // print first sub kw
	// ++row;
	// String subkw1 = lines[row].trim();
	// String subkw1_blurb = meta_data.get_blurb(kwname + "-" + subkw1);
	//
	// kw_os.append(" <tr>");
	// kw_os.append("  <th class=\"kwtype\" rowspan=" + num_sub_kw + ">" +
	// group_name + "</th>\n");
	// kw_os.append("  <td class=\"kwname\">\\subpage " + kwname + "-" + subkw1
	// + "</td>\n");
	// kw_os.append("  <td class=\"emph-italic\">" + subkw1_blurb + "</td>\n");
	// kw_os.append(" </tr>\n");
	//
	// // print rest kw, going until
	// ++row;
	// for ( ; row <= end_inds.get(ci); ++row) {
	// String subkw = lines[row].trim();
	// String subkw_blurb = meta_data.get_blurb(kwname + "-" + subkw);
	// kw_os.append("<tr>\n");
	// kw_os.append("<td class=\"kwname\">\\subpage " + kwname + "-" + subkw +
	// "</td>\n");
	// kw_os.append("<td class=\"emph-italic\">" + subkw_blurb + "</td>\n");
	// kw_os.append("</tr>\n");
	// }
	// kw_os.append("</table>\n\n");
	//
	//
	// }
	// }
	// }

}
