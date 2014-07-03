package gov.sandia.dart.dakota;

import gov.sandia.dart.dakota.nidr.antlr.NIDRBaseListener;
import gov.sandia.dart.dakota.nidr.antlr.NIDRLexer;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser.CommandContext;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser.GOptionalGroupContext;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser.GRequiredGroupContext;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser.OptionalGroupContext;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser.RequiredGroupContext;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser.ToplevelContext;

import java.io.IOException;
import java.io.FileWriter;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Stack;

import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.misc.NotNull;
import org.antlr.v4.runtime.tree.ParseTree;
import org.antlr.v4.runtime.tree.ParseTreeWalker;
import org.antlr.v4.runtime.tree.TerminalNode;

@SuppressWarnings("serial")
public class RefManNIDRSpec extends NIDRBaseListener{
	static class Required extends ArrayList<Object> {}
	static class Optional extends ArrayList<Object> {}
	static class Alternatives extends ArrayList<Object> {}
	// keep this insertion ordered with Linked for printing purposes
	static class KeywordMetaData extends LinkedHashMap<String, String> {}
	
	public RefManNIDRSpec(String input_spec) throws IOException {
		ANTLRFileStream input = new ANTLRFileStream(input_spec);
		NIDRLexer lexer = new NIDRLexer(input);
		CommonTokenStream tokens = new CommonTokenStream(lexer);
		NIDRParser parser = new NIDRParser(tokens);
		ParseTree tree = parser.input();
		ParseTreeWalker walker = new ParseTreeWalker();
		//RefManNIDRSpec tool = new RefManNIDRSpec();
		walker.walk(this, tree);
		// parse the spec, generating input spec data structure
		parse();
	}
	
	private Map<String, Integer> keywordCounts = new HashMap<String, Integer>();
	
	List<Object> toplevels = new ArrayList<Object>();
	Stack<Object> scopes = new Stack<Object>();

	@Override
	public void enterToplevel(ToplevelContext ctx) {
		Map<String, Object> keyword = new HashMap<String, Object>();		
		String name = ctx.SYMBOL().getText();
		keyword.put("tag", ctx.KEYWORD().getText());
		keyword.put("name", name);
		keyword.put("id", uniquify(name));
		keyword.put("macro", ctx.MACRO().getText());
		keyword.put("contents", new ArrayList<Object>());
		scopes.push(keyword);
	}

	@Override
	public void exitToplevel(ToplevelContext ctx) {
		toplevels.add(scopes.pop());
	}
	
	@Override public void enterRequiredGroup(RequiredGroupContext ctx) {
		Required req = new Required();
		scopes.push(req);		
	};
	
	@Override public void exitRequiredGroup(RequiredGroupContext ctx) {
		popAndRecordContext();
	}

	private void popAndRecordContext() {
		Object object = scopes.pop();
		addToContext(object, scopes.peek());
	};
	
	@Override public void enterGRequiredGroup(GRequiredGroupContext ctx) {
		Required req = new Required();
		scopes.push(req);		
	};
	
	@Override public void exitGRequiredGroup(GRequiredGroupContext ctx) {
		popAndRecordContext();
	};

	@Override public void enterOptionalGroup(OptionalGroupContext ctx) {
		Optional opt = new Optional();
		scopes.push(opt);		
	};
	
	@Override public void exitOptionalGroup(OptionalGroupContext ctx) {
		popAndRecordContext();
	};
	
	@Override public void enterGOptionalGroup(GOptionalGroupContext ctx) {
		Optional opt = new Optional();
		scopes.push(opt);		
	};
	
	@Override public void exitGOptionalGroup(GOptionalGroupContext ctx) {
		popAndRecordContext();
	};
	
	@SuppressWarnings("unchecked")
	private void addToContext(Object data, Object scope) {
		if (scope instanceof List)
			((List<Object>) scope).add(data);
		else if (scope instanceof Map)
			((List<Object>) ((Map<Object, Object>) scope).get("contents")).add(data);
	}
	
	@Override
	public void enterCommand(CommandContext ctx) {
		Map<String, Object> keyword = new HashMap<String, Object>();		
		String name = ctx.SYMBOL().get(0).getText();
		keyword.put("name", name);
		keyword.put("id", uniquify(name));
		keyword.put("macro", ctx.MACRO().getText());
		int paramName = 1;
		
		if (ctx.ALIAS().size() > 0) {
			List<String> aliases = new ArrayList<String>();
			for (int i=0; i<ctx.ALIAS().size(); ++i) {
				aliases.add(ctx.SYMBOL(i+1).getText());
			}
			keyword.put("alias", aliases);		
			paramName += aliases.size();
		}

		if (ctx.SYMBOL().size() > paramName) {
			keyword.put("param", ctx.SYMBOL().get(paramName));
			if (ctx.OP() != null) {
				keyword.put("op", ctx.OP().getText());
				keyword.put("number", ctx.NUMBER().getText());
			}  else if (ctx.LENSPEC() != null) {
				keyword.put("lenspec", ctx.LENSPEC().getText());
				keyword.put("length", ctx.SYMBOL(ctx.SYMBOL().size()-1).getText());
			}
		}
		addToContext(keyword, scopes.peek());
	};
		
	private String uniquify(String name) {
		Integer count = keywordCounts.get(name);
		if  (count == null) {
			keywordCounts.put(name, 0);
			return name;
		} else {
			keywordCounts.put(name,  ++count);
			return name + count;
		}
	}

	@Override public void visitTerminal(@NotNull TerminalNode node) {
		if (node.getText().equals("|"))
			addToContext("|", scopes.peek());
	}

	@Override public void enterOrGroup(gov.sandia.dart.dakota.nidr.antlr.NIDRParser.OrGroupContext ctx) {
		Alternatives alt = new Alternatives();
		scopes.push(alt);	
	}
	
	@Override public void exitOrGroup(gov.sandia.dart.dakota.nidr.antlr.NIDRParser.OrGroupContext ctx) {
		Alternatives opt = (Alternatives) scopes.pop();
		addToContext(opt, scopes.peek());
	}

	
	// the current hierarchical keyword context such as [strategy, tabular_graphics_data, tabular_graphics_file]
	List<String> context = new ArrayList<String>();
	
	// get the context as a string
	private String context_string() {
		String hierarchy_string = new String(context.get(0));
		for (String s: context.subList(1,context.size())) {
			hierarchy_string += "-" + s;
		}
		return hierarchy_string;
	}
	
	// container to store final data: a map from keyword (hierarchical) to contents)
	// for now used linked to keep in parsed order for testing (not strictly needed)
	// KeywordMetaData stores Field, Value pairs
	Map<String, KeywordMetaData> spec_data = new LinkedHashMap<String, KeywordMetaData>();
	
	
	public Set<Map.Entry<String, KeywordMetaData>> get_entries() {
		return spec_data.entrySet();
	}
	
	public boolean entry_exists(String kwname) {
		return spec_data.containsKey(kwname);
	}
	
	// Append data to the "Children" field for immediate children of this keyword
	private void append_children(String value) {
		
		// the hierarchical keyword context
		String kw_hier = context_string();
		String field = "Children";

		if (spec_data.containsKey(kw_hier)) {
			KeywordMetaData kw_md = spec_data.get(kw_hier);
			if (kw_md.containsKey(field)) {
				// append to existing child data
				String curr_children = kw_md.get(field);
				curr_children += "\n" + value;
				kw_md.put(field, curr_children);
			}
			else {
				kw_md.put(field, value);
			}
		}
		else {
			// create the key and insert
			KeywordMetaData kw_md = new KeywordMetaData();
			kw_md.put(field, value);
			spec_data.put(kw_hier, kw_md);
		}	
	}
	
	
	private void add_data(String field, String value) {
		
		// the hierarchical keyword context
		String kw_hier = context_string();
		
		// all fields should be single entry except Children
		if (spec_data.containsKey(kw_hier)) {
			KeywordMetaData kw_md = spec_data.get(kw_hier);
			if (kw_md.containsKey(field)) {
				System.out.println("Warning: spec_data multiple insertion of field " + field + " for keyword " + kw_hier);
			}
			else {
				kw_md.put(field, value);
			}
		}
		else {
			// create the key and insert
			KeywordMetaData kw_md = new KeywordMetaData();
			kw_md.put(field, value);
			spec_data.put(kw_hier,  kw_md);
		}
	}
	
	public void print_debug(String output_file) throws IOException {
		OutputStreamWriter os = new FileWriter(output_file);
		for (KeywordMetaData kw_md: spec_data.values()) {
			for (Map.Entry<String, String> entry: kw_md.entrySet()) {
				if (entry.getKey().equals("Children"))
					os.append(entry.getValue() + "\n");
				else
					os.append(entry.getKey() + ":: " + entry.getValue() + "\n");
			}
			os.append("\n");
		}
		os.close();	
	}
	
	// print the doxy spec for this KW, including alias, arguments, and spec table
	public void doxy_print(String kwname, OutputStreamWriter kw_os, RefManMetaData meta_data) throws IOException {
		kw_os.append("<h2>Specification</h2>\n");
	
		String alias = "none";
		if (spec_data.containsKey(kwname) && spec_data.get(kwname).containsKey("Alias")) {
			alias = spec_data.get(kwname).get("Alias");
			if (alias.trim().isEmpty())
				alias = "none";
		}
		
		String argument = "none";
		if (spec_data.containsKey(kwname) && spec_data.get(kwname).containsKey("Argument")) {
			argument = spec_data.get(kwname).get("Argument");
			if (argument.trim().isEmpty())
				argument = "none";
		}
		
		kw_os.append("\n<p><b>Alias: </b>" + alias + "</p>\n");
		kw_os.append("<p><b>Argument(s): </b>" + argument + "</p>\n");
		
		if (spec_data.containsKey(kwname) && spec_data.get(kwname).containsKey("Children")) {

			String children = spec_data.get(kwname).get("Children");
			String[] lines = children.split("\\n");

			// save the indices of each new context (required/optional keyword or group)
			// TODO: store these in a better data structure
			int index = 0;
			ArrayList<String> category = new ArrayList<String>();
			ArrayList<Integer> start_inds = new ArrayList<Integer>(), end_inds = new ArrayList<Integer>();
			// first find the indices of the relevant rows (beginning Choose_One, ending with Req/Opt or Choose)
			boolean in_group = false;
			for (String line: lines) {
				if ( line.contains("_Choose_One") || 
					 line.startsWith("Required_Keyword") || line.startsWith("Optional_Keyword")) {
					// Save optional or required in category
					category.add(line.split(":")[0]);
					start_inds.add(index);
					if (in_group)
						end_inds.add(index-1);
					in_group = true;
				}
				++index;	
			}
			if (in_group)
				end_inds.add(index-1);
		
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
			kw_os.append(header);
			
			int group_num = 0;
			for (int ci = 0; ci<num_children; ++ci) {
				
				int start_row = start_inds.get(ci);
				int end_row = end_inds.get(ci);
				// number of sub keywords, not counting the *_Choose_One label
				int num_sub_kw = end_inds.get(ci) - start_inds.get(ci);
				String group_type = category.get(ci);
							
				if (group_type.contains("Choose_One")) {
					if (num_sub_kw < 1)
						System.err.println("Warning (doxy_print): found choose group with no items.");
					++group_num;
					String sg = doxy_format_subgroup(group_type, lines, start_row, end_row, kwname, meta_data, group_num);
					kw_os.append(sg);
				}
				else if (group_type.contains("Keyword")) {
					if (num_sub_kw != 0)
						System.err.println("Warning (doxy_print): found bare keyword with more than one row.");
					String skw = doxy_format_subkw(group_type, lines[start_row], kwname, meta_data);
					kw_os.append(skw);
				} 
				else {
					System.err.println("Warning (doxy_print): unexpected keyword subgroup type " + group_type + ".");
				}
			}
			
			// footer for the spec table
			String footer = "</table>\n";
			kw_os.append(footer);
			
		}
		
	}
	
	
	// print a formatted table entry for a single required or optional keyword (*_Keyword followed by keyword name)
	private String doxy_format_subkw(String group_type, String line, String kwname, RefManMetaData meta_data) {
	
		// Required or Optional
		String reqopt = group_type.split("_", 2)[0];
				
		String [] splitline = line.split(":{2}?", 2);
		// data may be absent or on next line; not trimming here to preserve user formatting
		String subkw = "";
		if (splitline.length > 1)
			subkw = splitline[1].trim();

		String subkw_blurb = meta_data.get_blurb(kwname + "-" + subkw); 
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
	
	// print a formatted table entry for a choose group (leading line followed by subkeywords)
	private String doxy_format_subgroup(String group_type, String [] lines, int start, int end, String kwname, 
			RefManMetaData meta_data, int group_num) {
		
		String table_subgroup = new String();
		
		// Required or Optional
		String reqopt = group_type.split("_", 2)[0];
		
		int num_subkw = end-start;
		
		for(int index = 1; index <= num_subkw; ++index) {
		
			// skip the *_Choose line
			String subkw = lines[start + index].trim();
			String subkw_blurb = meta_data.get_blurb(kwname + "-" + subkw);
			String subkw_ref = "\\subpage " + kwname + "-" + subkw; 
			
			if (index == 1) {
				// first row
				table_subgroup += " <tr>\n";
				table_subgroup += "  <th class=\"border-heavy-right\" rowspan=\""+ num_subkw +"\" > </th>\n";
				table_subgroup += "  <td class=\"border-light-right vert-align-mid\" rowspan=\"" + num_subkw + "\"><strong>" + reqopt + "</strong><div><em>(Choose One)</em></div></td>\n";
				table_subgroup += "  <td class=\"border-light-right vert-align-mid\" rowspan=\"" + num_subkw + "\"><strong>Group " + group_num + "</strong></td>\n";
				table_subgroup += "  <td class=\"border-light-right border-light-bottom\">" + subkw_ref + "</td>\n";
				table_subgroup += "  <td class=\"border-light-bottom\">" + subkw_blurb + "</td>\n";
				table_subgroup += " </tr>\n";
			}
			else if (index == num_subkw) {
				// last row
				table_subgroup += " <tr>\n";
				table_subgroup += "  <td class=\"border-light-right\">" + subkw_ref + "</td>\n";
				table_subgroup += "  <td>" + subkw_blurb + "</td>\n";
				table_subgroup += " </tr>\n";
			}
			else {
				// middle row
				table_subgroup += " <tr>\n";
				table_subgroup += "  <td class=\"border-light-right border-light-bottom\">" + subkw_ref + "</td>\n";
				table_subgroup += "  <td class=\"border-light-bottom\">" + subkw_blurb + "</td>\n";
				table_subgroup += " </tr>\n";
			}
		}
	
		return table_subgroup;
	}

	
	// deprecated function
	private void doxy_print_kws(String kwname, OutputStreamWriter kw_os, RefManMetaData meta_data, String reqopt) throws IOException {
		
		if (spec_data.containsKey(kwname) && spec_data.get(kwname).containsKey("Children")) {

			String children = spec_data.get(kwname).get("Children");

			// print all required KW together
			boolean found_required = false;
			int index = 0;
			for (String line: children.split("\\n")) {
				if (line.startsWith(reqopt + "_Keyword")) {
					++index;
					if (!found_required) {
						found_required = true;
						kw_os.append("<table class=\"keyword\">\n");
						kw_os.append(" <tr>\n");
						kw_os.append("  <th> </th>\n");
						kw_os.append("  <th class=\"kwtype\" colspan=\"2\">" + reqopt + " Keywords</th>\n");
						kw_os.append(" </tr>\n");					
					}
					String [] splitline = line.split(":{2}?", 2);
					//field = splitline[0].trim();
					// data may be absent or on next line; not trimming here to preserve user formatting
					String subkw = "";
					if (splitline.length > 1)
						subkw = splitline[1].trim();

					String subkw_blurb = meta_data.get_blurb(kwname + "-" + subkw); 
					kw_os.append("<tr>\n");
					kw_os.append("  <td>" + index + "</td>\n");
					kw_os.append("  <td class=\"kwname\">\\subpage " + kwname + "-" + subkw + "</td>\n");
					kw_os.append("  <td class=\"emph-italic\">" + subkw_blurb + "</td>\n");
					kw_os.append(" </tr>\n");
				}
			}
			if (found_required)
				kw_os.append("</table>\n");
		}
	}
	
	// deprecated function
	private void doxy_print_choose(String kwname, OutputStreamWriter kw_os, RefManMetaData meta_data) throws IOException {

		if (spec_data.containsKey(kwname) && spec_data.get(kwname).containsKey("Children")) {

			String children = spec_data.get(kwname).get("Children");
			String[] lines = children.split("\\n");
			
			int index = 0;
			ArrayList<Integer> start_inds = new ArrayList<Integer>(), end_inds = new ArrayList<Integer>();
			// first find the indices of the relevant rows (beginning Choose_One, ending with Req/Opt or Choose)
			boolean in_choose = false;
			for (String line: lines) {
				if (line.contains("_Choose_One")) {
					start_inds.add(index);
					if (in_choose)
						end_inds.add(index-1);
					in_choose = true;
				}
				if (in_choose && (line.startsWith("Required_Keyword") || line.startsWith("Optional_Keyword"))) {
					end_inds.add(index-1);
					in_choose = false;
				}
				++index;	
			}
			if (in_choose)
				end_inds.add(index-1);
						
			// no group names for now
			String group_name = "";
			
			int num_choose = start_inds.size();
			for (int ci = 0; ci<num_choose; ++ci) {
				int row = start_inds.get(ci);
				
				// print header with Choose One
				String [] splitline = lines[row].split(":{2}?", 2);
				String group_type = splitline[0].trim().replace("_", " ");
				int num_sub_kw = end_inds.get(ci) - start_inds.get(ci);
				
				kw_os.append("<table class=\"keyword\">\n");
				kw_os.append(" <tr>\n");
				kw_os.append("  <th> </th>\n");
				kw_os.append("  <th class=\"kwtype\" colspan=\"2\">" + group_type + "</th>\n");
				kw_os.append(" </tr>\n");
				 
				// print first sub kw
				++row;
				String subkw1 = lines[row].trim();
				String subkw1_blurb = meta_data.get_blurb(kwname + "-" + subkw1);
				
				kw_os.append(" <tr>");
				kw_os.append("  <th class=\"kwtype\" rowspan=" + num_sub_kw + ">" + group_name + "</th>\n");
				kw_os.append("  <td class=\"kwname\">\\subpage " + kwname + "-" + subkw1 + "</td>\n");
				kw_os.append("  <td class=\"emph-italic\">" + subkw1_blurb + "</td>\n");
				kw_os.append(" </tr>\n");

				// print rest kw, going until 
				++row;
				for ( ; row <= end_inds.get(ci); ++row) {
					String subkw = lines[row].trim();
					String subkw_blurb = meta_data.get_blurb(kwname + "-" + subkw);
					kw_os.append("<tr>\n");
					kw_os.append("<td class=\"kwname\">\\subpage " + kwname + "-" + subkw + "</td>\n");
					kw_os.append("<td class=\"emph-italic\">" + subkw_blurb + "</td>\n");
					kw_os.append("</tr>\n");
				}
				kw_os.append("</table>\n\n");
				
				
			}
		}
	}
	
	
	@SuppressWarnings("unchecked")
	private void parse() throws IOException {
		try {
			printHeader();
			for (Object o: toplevels) {
				Map<Object, Object> toplevel = (Map<Object, Object>) o;				

				// context for the top level contents (needed by this print)
				context.add((String) toplevel.get("name"));
				
				// print the header for the top-level keyword
				printKeywordHeader(toplevel);
				
				Object contents = toplevel.get("contents");

				// print the immediate children of this keyword
				boolean immediate_children = true;
				printTopLevelItem(contents, immediate_children);
				
				// recurse down the tree
				immediate_children = false;
				printTopLevelItem(contents, immediate_children);
								
				// pop a context for the next top-level KW
				context.remove(context.size()-1);
						
			}
			printFooter();
		} finally {
			;
		}
	}

	
	// for immediate children mode, we print bare keywords, alternates, (KW or alternate) anchors of groups
	// omitting additional items in subgroups
		
	@SuppressWarnings("unchecked")
	private void printTopLevelItem(Object o, boolean immediate_children) {
		if (o instanceof Required) {
			printRequiredGroup((Required) o, "Required_Keyword::", immediate_children);
		}
		else if (o instanceof Optional) {
			printOptionalGroup((Optional) o, "Required_Keyword::", immediate_children);
		}	
		else if (o instanceof Alternatives) {
			printAlternatives((Alternatives) o, "Required_Keyword::", immediate_children);
		}
		else {
			printItems((List<Object>) o, "Required_Keyword::", immediate_children);
		}
	}
	
	// so perhaps recurse on alternates and groups, with that in mind
	@SuppressWarnings("unchecked")
	private void printItem(Object o, String option_str, boolean immediate_children) {
		// Required and Optional override the optional boolean
		if (o instanceof Required) {
			printRequiredGroup((Required) o, option_str, immediate_children);
		}
		else if (o instanceof Optional) {
			printOptionalGroup((Optional) o, option_str, immediate_children);
		}	
		else if (o instanceof Alternatives) {
			printAlternatives((Alternatives) o, option_str, immediate_children);
		}
		// don't want to print bare keywords when in recursive mode; results in duplicates
		else if (o instanceof Map) {
			Map<Object, Object> keyword = (Map<Object, Object>) o;
			// if the alternation is marked with optional or required (Choose), just need the KW here
			if (immediate_children) {
				String kw_optional_str = "";
				if (!option_str.contains("Choose"))
					kw_optional_str = "Required_Keyword::";
				printTaggedKeyword(kw_optional_str, keyword);
			}
			else {
				context.add((String) keyword.get("name"));
				printKeywordHeader(keyword);
				context.remove(context.size()-1);
			}
		}
	}

	// print an item of one of the fundamental types
	private void printItems(List<Object> list, String option_str, boolean immediate_children) {
		for (Object o: list) {
			printItem(o, option_str, immediate_children);
		}
	}
	
	// TODO: these may need to forward Choose status. morphing to req/opt
	private void printRequiredGroup(Required list, String option_str, boolean immediate_children) {
		String kw_optional_str = "Required_Keyword::";
		if (immediate_children) {
			// must propagate Choose, but the keyword itself is required within a potentially optimal Choose
			if (option_str.contains("Choose"))
				kw_optional_str = "Required_Choose_One::";
			printGroupLeader(list, kw_optional_str, immediate_children);
		}	
		else 
			printGroup(list, kw_optional_str, immediate_children);
	}
	
	private void printOptionalGroup(Optional list, String option_str, boolean immediate_children) {
		String kw_optional_str = "Optional_Keyword::";
		if (immediate_children) {
			// must propagate Choose, but the keyword itself is required within a potentially optimal Choose
			if (option_str.contains("Choose"))
				kw_optional_str = "Optional_Choose_One::";
			printGroupLeader(list, kw_optional_str, immediate_children);
		}
		else 
			printGroup(list, kw_optional_str, immediate_children);
	}
		
	// print leader in group (could be required or optional, with 1 or more entries
	@SuppressWarnings("unchecked")
	private void printGroupLeader(List<Object> list, String option_str, boolean immediate_children) {
		
		Object leading_item = list.get(0);
		// group leader is a keyword
		if (leading_item instanceof Map) {
			Map<Object, Object> keyword = (Map<Object, Object>) leading_item;
		
			// when in choose mode, use empty string as the alternation is already marked Opt/Req
			String kw_optional_str = "";
			if (!option_str.contains("Choose")) {
				kw_optional_str = option_str.contains("Optional") ? "Optional_Keyword::" : "Required_Keyword::";
			}
			
			printTaggedKeyword(kw_optional_str, keyword);
		}
		// TODO: do we need special treatment of alternatives?
		// for example, group leader is another group or an alternation
		else {
			printItem(leading_item, option_str, immediate_children);
		}
			
	}
	
	
	private void printGroup(List<Object> list, String option_str, boolean immediate_children) {
		
		// A group can have in it: ReqGroup, OptGroup, Alternatives, or KW
		
		Object first_entry = list.get(0);
		List<Object> sublist = list.subList(1, list.size());
		
		// A leading KW can't be optional
		if (first_entry instanceof Optional) {
			System.out.println("Warning: In context " + context.toString() + ", group leader is optional.");
		}

		// print leader to add its entries, regardless of its type (KW, ALT, GROUP)
		// whether the leader is required is dictated by the calling context
		printItem(first_entry, option_str, false);
		
		// iterate immediate leads, print sublist for each and recursing
		ArrayList<String> immed_leads = find_immediate_leaders(first_entry);
		
		// TODO: verify this is right when this is an optional group itself
		// if there was a keyword leader, following keywords, groups, alternations are required unless again marked optional
		String sublist_optional = "Required_Keyword::";
		
		for (String s: immed_leads) {
			// print the sublist with each possible context from the leader
			context.add(s);
			if (sublist.size() > 0) {
				// print immediate children
				printItems(sublist, sublist_optional, true);
				// now recurse on items, printing their header and recursing
				printItems(sublist, sublist_optional, false);
			}
			context.remove(context.size()-1);
		}
			// TODO!!! IF we have alternation of groups followed by other spec, they are missed in this scheme
//			// Any alternation of required components appearing first in the group can serve as the leading KW.
//			// hack to try to collect up all the leading keywords in this first item's groups that may give context
//				
//			// This will print the subitems at the current level, with the right header, unrolling ALTs
//			// Probably a little aggressive...
//			// TODO: Are we properly recursing to print all keywords?
//			
//			// TODO: this might mess up the printing of immediate children; they'll get inserted in the middle of parents
	}
	
	
	private void printAlternatives(Alternatives list, String option_str, boolean immediate_children) {
		
		if (immediate_children) {
		
			// if doesn't already have Choose, set the tag to Choose
			// if already has, don't want multiple since this doesn't parse nested KW
			String header_str = option_str;
			if (!option_str.contains("Choose") && option_str.contains("Optional")) {
				header_str = "Optional_Choose_One::";
				append_children(header_str);
			}
			else if (!option_str.contains("Choose") && option_str.contains("Required")) {
				header_str = "Required_Choose_One::";
				append_children(header_str);
			}		
			// else already has a choose header; don't append another in immediate children mode
			
			printItems(list, header_str, immediate_children);
		}
		else {
			// iterate the alternatives, printing a header for them and their subkeywords
			printItems(list, option_str, immediate_children);
			
		}
	}
	
	// unroll alternates and groups to find any keywords that can appear as peers at this level
	// this is similar to printing leaders, but we don't want to emit output, so reimplement
	// recursive search returns appended list
	// TODO: should we append or prepend as we find?
	@SuppressWarnings("unchecked")
	private ArrayList<String> find_immediate_leaders(Object o) {
		ArrayList<String> immed_leads = new ArrayList<String>();
		if (o instanceof Required) {
			Required req = (Required) o;
			Object group_leader = req.get(0);
			immed_leads.addAll(find_immediate_leaders(group_leader));
		}
		else if (o instanceof Optional) {
			// optional can't be a leader for anchor purposes
		}	
		else if (o instanceof Alternatives) {
			Alternatives alts = (Alternatives) o;
			for (Object a: alts) {
				immed_leads.addAll(find_immediate_leaders(a));
			}
		}
		// don't want to print bare keywords when in recursive mode; results in duplicates
		else if (o instanceof Map) {
			Map<Object, Object> keyword = (Map<Object, Object>) o;
			String name = (String)keyword.get("name");
			immed_leads.add(name);
		}
		return immed_leads;
	}
	
	// print the tag followed by keyword "name" field
	private void printTaggedKeyword(String tag, Map<Object, Object> keyword) {
		append_children(tag + " " + keyword.get("name").toString());
	}
		
	@SuppressWarnings({ "unchecked" })
	private void printKeywordHeader(Map<Object, Object> keyword) {
		
		String hierarchy_string = context_string();
		
		add_data("Keyword_Hierarchy", hierarchy_string);
		add_data("Name", keyword.get("name").toString());
		
		// TODO: multiple aliases aren't supported, but we print them for now
		StringBuilder alias_builder = new StringBuilder();
		if (keyword.containsKey("alias")) {
			List<String> aliases = (List<String>) keyword.get("alias");
			for (String alias: aliases)
				alias_builder.append(alias).append(" ");			
		}
		add_data("Alias", alias_builder.toString());
		
		StringBuilder param_builder = new StringBuilder();
		if (keyword.containsKey("param"))
			param_builder.append(keyword.get("param"));
		add_data("Argument", param_builder.toString());
		
	}
	
	
	private void printHeader() {
	}

	private void printFooter() {
	}



}
