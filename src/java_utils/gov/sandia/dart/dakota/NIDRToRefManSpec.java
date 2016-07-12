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

import gov.sandia.dart.dakota.RefManInputSpec;
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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.misc.NotNull;
import org.antlr.v4.runtime.tree.ParseTree;
import org.antlr.v4.runtime.tree.ParseTreeWalker;
import org.antlr.v4.runtime.tree.TerminalNode;


/**
 * Parse dakota.input.nspec into a NIDRToRefManSpec object for reference manual generation
 */
@SuppressWarnings("serial")
public class NIDRToRefManSpec extends NIDRBaseListener {
	static class Required extends ArrayList<Object> {}
	static class Optional extends ArrayList<Object> {}
	static class Alternatives extends ArrayList<Object> {}
	
	public static void main(String[] args) throws Exception {		
		// default file name for testing only
		String dakota_nspec = "dakota.input.nspec";
		NIDRToRefManSpec translator = new NIDRToRefManSpec(dakota_nspec);			
		translator.parse();
		RefManInputSpec test_spec_data = translator.refman_spec_data();
		test_spec_data.printDebug("debug.nspec.txt");
	}
	
	// -----
	// Public client API
	// -----
	
	public NIDRToRefManSpec(String input_nspec) throws IOException {
		ANTLRFileStream input = new ANTLRFileStream(input_nspec);
		NIDRLexer lexer = new NIDRLexer(input);
		CommonTokenStream tokens = new CommonTokenStream(lexer);
		NIDRParser parser = new NIDRParser(tokens);
		ParseTree tree = parser.input();
		ParseTreeWalker walker = new ParseTreeWalker();
		//NIDRToRefManSpec tool = new NIDRToRefManSpec();
		walker.walk(this, tree);
		spec_data = new RefManInputSpec();
	}
	
	public RefManInputSpec refman_spec_data() {
		return spec_data;
	}
	
	// -----
	// Core data members and convenience functions
	// -----
	
	// container to hold the final data after parse
	private RefManInputSpec spec_data;

	// the current hierarchical keyword context such as [strategy, tabular_graphics_data, tabular_graphics_file]
	private List<String> context = new ArrayList<String>();
	
	// get the context as a string
	private String context_string() {
		String hierarchy_string = new String(context.get(0));
		for (String s: context.subList(1,context.size())) {
			hierarchy_string += "-" + s;
		}
		return hierarchy_string;
	}

	
	// -----
	// NIDR-related parse machinery
	// -----
	
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

	
	// -----
	// Main reference manual parse machinery
	// -----

	// parse the spec, generating input spec data structure
	@SuppressWarnings("unchecked")
	public void parse() throws IOException {
		try {
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
				spec_data.appendChild(context_string(), header_str);
			}
			else if (!option_str.contains("Choose") && option_str.contains("Required")) {
				header_str = "Required_Choose_One::";
				spec_data.appendChild(context_string(), header_str);
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
		spec_data.appendChild(context_string(), tag + " " + keyword.get("name").toString());
	}
		
	@SuppressWarnings({ "unchecked" })
	private void printKeywordHeader(Map<Object, Object> keyword) {
		
		String hierarchy_string = context_string();
		
		spec_data.addData(hierarchy_string, "Keyword_Hierarchy", hierarchy_string);
		spec_data.addData(hierarchy_string, "Name", keyword.get("name").toString());
		
		// TODO: multiple aliases aren't supported, but we print them for now
		StringBuilder alias_builder = new StringBuilder();
		if (keyword.containsKey("alias")) {
			List<String> aliases = (List<String>) keyword.get("alias");
			for (String alias: aliases)
				alias_builder.append(alias).append(" ");			
		}
		spec_data.addData(hierarchy_string, "Alias", alias_builder.toString());
		
		StringBuilder param_builder = new StringBuilder();
		if (keyword.containsKey("param"))
			param_builder.append(keyword.get("param"));
		spec_data.addData(hierarchy_string, "Argument", param_builder.toString());
		
	}
	
}
