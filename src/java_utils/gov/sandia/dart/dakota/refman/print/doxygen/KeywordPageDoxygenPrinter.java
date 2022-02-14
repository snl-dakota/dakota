package gov.sandia.dart.dakota.refman.print.doxygen;

import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.Map.Entry;

import gov.sandia.dart.dakota.refman.RefManInputSpec;
import gov.sandia.dart.dakota.refman.RefManMetaData;
import gov.sandia.dart.dakota.refman.metadata.InputSpecKeywordMetaData;
import gov.sandia.dart.dakota.refman.metadata.RefManKeywordMetaData;

public class KeywordPageDoxygenPrinter implements KeywordPrinter {

	@Override
	public void print(String outputDir, RefManInputSpec spec_data, RefManMetaData meta_data) throws IOException {
		// Iterate based on the spec, not the data files.  Then appeal to the data files for information
		try(OutputStreamWriter kw_os = new FileWriter(outputDir + "/DakotaKeywords.dox")) {
		
			// print Doxygen for all keywords, managing the sequencing of output
			for (Entry<String, InputSpecKeywordMetaData> mdentry : spec_data.getEntries()) {
				
				// the data in the spec
				String kwname = mdentry.getKey();
				InputSpecKeywordMetaData kwInputSpec = mdentry.getValue();
				
				// for each, check whether meta data exists, if not, warn
				boolean has_metadata = meta_data.containsKey(kwname);
				if (!has_metadata) {
					throw new IllegalStateException("Warning: Missing metadata entry for keyword " + kwname);
				}
				
				kw_os.append(printPageTitle(kwname));
				kw_os.append(printBreadcrumbs(kwname));
				
				if(has_metadata) {
					RefManKeywordMetaData kwMetadata = meta_data.getKeywordMetadata(kwname);
					kw_os.append(printBlurb(kwMetadata));
					kw_os.append(printTopics(kwMetadata));
				}
				
				// need meta data for the blurb
				kw_os.append(printKeyword(kwInputSpec, meta_data));
				
				if(has_metadata) {
					RefManKeywordMetaData kwMetadata = meta_data.getKeywordMetadata(kwname);
					kw_os.append(printDescription(kwMetadata));
					kw_os.append(printExamples(kwMetadata));
					kw_os.append(printTheory(kwMetadata));
					kw_os.append(printSeeAlso(kwMetadata));		
					kw_os.append(printFAQ(kwMetadata));
				}
			}
		}
	}
	
	@Override
	public String printPageTitle(String hierarchical_kwname) {		
		String [] split_kw = hierarchical_kwname.split("-");
		String name = split_kw[split_kw.length - 1];
		return "\\page " + hierarchical_kwname + " " + name + "\n";
	}
	
	@Override
	public String printBreadcrumbs(String hierarchical_kwname) {
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

		return bc_text;
	}
	
	@Override
	public String printKeyword(InputSpecKeywordMetaData kw, RefManMetaData meta_data) {
		StringBuilder sb = new StringBuilder();
		
		String name = kw.getName();
		String alias = kw.getAlias();
		String argument = kw.getArgument();
		String defaultProperty = kw.getDefaultProperty();
		List<String> children = kw.getChildren();
		
		sb.append("<h2>Specification</h2>\n");

		if(alias.trim().isEmpty()) {
			sb.append("\n<p><b>Alias: </b>none/p>\n");
		} else {
			sb.append("\n<p><b>Alias: </b>").append(alias).append("</p>\n");
		}

		if(argument.trim().isEmpty()) {
			sb.append("<p><b>Argument(s): </b>none</p>\n");
		} else {
			sb.append("<p><b>Argument(s): </b>").append(argument).append("</p>\n");
		}

		if(!defaultProperty.trim().isEmpty()) {
			sb.append("<p><b>Default: </b>").append(defaultProperty).append("</p>\n");
		}

		if(!children.isEmpty()) {
			sb.append("<p><b>Child Keywords:</b></p>\n");

			// save the indices of each new context (required/optional keyword or group)
			
			// TODO: store these in a better data structure
			int index = 0;
			ArrayList<String> category = new ArrayList<>();
			ArrayList<Integer> start_inds = new ArrayList<>();
			ArrayList<Integer> end_inds = new ArrayList<>();
			// first find the indices of the relevant rows (beginning
			// Choose_One, ending with Req/Opt or Choose)
			boolean in_group = false;
			for (String child : children) {
				if (child.contains("_Choose_One")
						|| child.startsWith("Required_Keyword")
						|| child.startsWith("Optional_Keyword")) {
					// Save optional or required in category
					// Leave the whole line for now as potentially includes group label

					category.add(child);
					start_inds.add(index);
					if (in_group)
						end_inds.add(index - 1);
					in_group = true;
				}
				++index;
			}
			if (in_group)
				end_inds.add(index - 1);

			// iterate over the children (some choose groups, some single keyword)
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
			sb.append(header);

			int group_num = 0;
			for (int ci = 0; ci < num_children; ++ci) {

				int start_row = start_inds.get(ci);
				int end_row = end_inds.get(ci);
				// number of sub keywords, not counting the *_Choose_One label
				int num_sub_kw = end_inds.get(ci) - start_inds.get(ci);
				String group_type = category.get(ci);

				if (group_type.contains("Choose_One")) {
					if (num_sub_kw < 1)
						System.err.println("Warning (doxy_print): found choose group with no items.");
					++group_num;
					String sg = doxy_format_subgroup(group_type, children.toArray(new String[children.size()]),
							start_row, end_row, name, meta_data, group_num);
					sb.append(sg);
				} else if (group_type.contains("Keyword")) {
					if (num_sub_kw != 0)
						System.err.println("Warning (doxy_print): found bare keyword with more than one row.");
					String skw = doxy_format_subkw(group_type,
							children.get(start_row), name, meta_data);
					sb.append(skw);
				} else {
					System.err.println("Warning (doxy_print): unexpected keyword subgroup type "
									+ group_type + ".");
				}
			}

			// footer for the spec table
			String footer = "</table>\n";
			sb.append(footer);
		}

		return sb.toString();
	}

	@Override
	public String printBlurb(RefManKeywordMetaData mdcontents) {
		String blurb_text = "";
		if(!mdcontents.getBlurb().isBlank()) {
			blurb_text = mdcontents.getBlurb().trim();
			if (blurb_text.isEmpty())
				blurb_text = "TODO: Need a Blurb\n";
		} else {
			blurb_text = "TODO: Need a Blurb\n";
		}
		return blurb_text + "\n\n";
	}
	
	@Override
	public String printTopics(RefManKeywordMetaData mdcontents) {
		StringBuilder topic_text = new StringBuilder();
		if (!mdcontents.getTopics().isEmpty()) {
			String topic_list = mdcontents.getTopics().trim();
			if (!topic_list.isEmpty()) {
				topic_text.append("<h2>" + RefManKeywordMetaData.TOPICS + "</h2>\n");
				topic_text.append("This keyword is related to the topics:\n");		
			
				String [] split_topics = topic_list.split(",");
				for (String topic: split_topics) {
					topic_text.append("- \\ref topic-" + topic.trim() + "\n");
				}
				return topic_text + "\n\n";
			}
		}
		return "";
	}

	@Override
	public String printSeeAlso(RefManKeywordMetaData mdcontents) {
		StringBuilder sa_text = new StringBuilder();
		if(!mdcontents.getSeeAlso().isBlank()) {
			String sa_list = mdcontents.getSeeAlso().trim();
			if (!sa_list.isEmpty()) {
				sa_text.append("<h2>" + RefManKeywordMetaData.SEE_ALSO + "</h2>\n");
				sa_text.append("These keywords may also be of interest:\n");		
				String [] split_sa = sa_list.split(",");
				for (String sa: split_sa) {
					sa_text.append("- \\ref " + sa.trim() + "\n");
				}
				return sa_text + "\n\n";
			}
		}
		return "";
	}
	
	
	@Override
	public String printDescription(RefManKeywordMetaData mdcontents) {
		return doxyPrint(RefManKeywordMetaData.DESCRIPTION, mdcontents.getDescription());
	}
	
	@Override
	public String printExamples(RefManKeywordMetaData mdcontents) {
		return doxyPrint(RefManKeywordMetaData.EXAMPLES, mdcontents.getExamples());
	}

	@Override
	public String printTheory(RefManKeywordMetaData mdcontents) {
		return doxyPrint(RefManKeywordMetaData.THEORY, mdcontents.getTheory());
	}
	
	@Override
	public String printFAQ(RefManKeywordMetaData mdcontents) {
		return doxyPrint(RefManKeywordMetaData.FAQ, mdcontents.getFaq());
	}
	
	
	/////////////
	// PRIVATE //
	/////////////
	
	private String doxyPrint(String header, String content) {
		if(!content.isBlank()) {
			String md_text = content.trim();
			if (!md_text.isEmpty()) {
				StringBuilder verbatim_text= new StringBuilder("<h2>" + header + "</h2>\n");
				verbatim_text.append(md_text);
				return verbatim_text + "\n\n";
			}
		}
		return "";
	}
	
	/**
	 * print a formatted table entry for a single required or optional keyword
	 * (*_Keyword followed by keyword name)
	 * @param group_type
	 * @param line
	 * @param kwname
	 * @param meta_data
	 * @return
	 */
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

		String subkw_blurb = meta_data.getBlurb(kwname + "-" + subkw);
		String subkw_ref = "\\subpage " + kwname + "-" + subkw;

		// print the table entries for this sub kw
		String table_subgroup = new String();
		table_subgroup += " <tr>\n";
		table_subgroup += "  <th class=\"border-heavy-right\"></th>\n";
		table_subgroup += "  <td class=\"border-light-right\" colspan=\"2\"><strong>" + reqopt + "</strong></td>\n";
		table_subgroup += "  <td class=\"border-light-right\"> " + subkw_ref + "</td>\n";
		table_subgroup += "  <td> " + subkw_blurb + "</td>\n";
		table_subgroup += " </tr>\n";

		return table_subgroup;
	}

	/**
	 * Print a formatted table entry for a choose group (leading line followed by subkeywords)
	 * @param group_type
	 * @param lines
	 * @param start
	 * @param end
	 * @param kwname
	 * @param meta_data
	 * @param group_num
	 * @return
	 */
	private String doxy_format_subgroup(String group_type, String[] lines,
			int start, int end, String kwname, RefManMetaData meta_data,
			int group_num) {

		String table_subgroup = "";

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
			String subkw_blurb = meta_data.getBlurb(kwname + "-" + subkw);
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
		
}
