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

import gov.sandia.dart.dakota.nidr.antlr.NIDRBaseListener;
import gov.sandia.dart.dakota.nidr.antlr.NIDRLexer;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser.CommandContext;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser.GOptionalGroupContext;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser.GRequiredGroupContext;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser.OptionalGroupContext;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser.RequiredGroupContext;
import gov.sandia.dart.dakota.nidr.antlr.NIDRParser.ToplevelContext;

import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
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

@SuppressWarnings("serial")
public class NIDRToXMLTranslator extends NIDRBaseListener{
	static class Required extends ArrayList<Object> {}
	static class Optional extends ArrayList<Object> {}
	static class Alternatives extends ArrayList<Object> {}
			
	public NIDRToXMLTranslator(DescFile dfile) {
		this.dfile = dfile;
	}

	public static void main(String[] args) throws IOException {
		DescFile dfile = new DescFile("dakota.input.desc");
		ANTLRFileStream input = new ANTLRFileStream("dakota.input.nspec");
		NIDRLexer lexer = new NIDRLexer(input);
		CommonTokenStream tokens = new CommonTokenStream(lexer);
		NIDRParser parser = new NIDRParser(tokens);
		ParseTree tree = parser.input();
		ParseTreeWalker walker = new ParseTreeWalker();
		NIDRToXMLTranslator tool = new NIDRToXMLTranslator(dfile);
		walker.walk(tool, tree);
		tool.print();		
	}

	Map<String, String> taglists = new HashMap<String, String>() {{
		put("id_method", "method");		
		put("id_model", "model");
		put("id_variables", "variables");		
		put("id_interface", "interface");
		put("id_responses", "responses");
	}};
	
	private Map<String, Integer> keywordCounts = new HashMap<String, Integer>();
	private DescFile dfile;	
	List<Object> toplevels = new ArrayList<Object>();
	Stack<Object> scopes = new Stack<Object>();
	private Stack<String> path = new Stack<String>();

	private Indenter out;

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
		
		// Do things link here?
		if (taglists.containsKey(name))
			keyword.put("taglist", taglists.get(name));
		
		// Is our parameter a link to something else?
		if (name.endsWith("_pointer")) {
			String tag = getDestinationName(name);
			if (taglists.containsValue(tag))
				keyword.put("in_taglist", tag);
		}
		
		addToContext(keyword, scopes.peek());
	};
		
	private String getDestinationName(String name) {
		for (String tag: taglists.values()) {
			if (name.contains(tag + "_"))
				return tag;
		}
		return null;
	}

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

	@SuppressWarnings("unchecked")
	private void print() throws IOException {
		out = new Indenter(new FileWriter("dakota.gen.xml"));
		try {
			printHeader();
			for (Object o: toplevels) {
				Map<Object, Object> toplevel = (Map<Object, Object>) o;
				String minOccurs, maxOccurs;
				String tag = toplevel.get("tag").toString();
				if (tag.equals("KEYWORD01")) {
					minOccurs = "0";
					maxOccurs = "1";
				} else if (tag.equals("KEYWORD12")) {
					minOccurs = "1";
					maxOccurs = "unbounded";
				} else {
					minOccurs="0";
					maxOccurs="unbounded";
				}
				
				out.printIndented("<!-- **** TOPLEVEL *** -->");
				String name = (String) toplevel.get("name");
				path.push(name);
				String label = getLabel(name, getPath(path));
				String help = getHelp(getPath(path));
				out.printAndIndent(
						String.format("<keyword id=\"%s\" name=\"%s\" minOccurs=\"%s\" maxOccurs=\"%s\" code=\"%s\" label=\"%s\" help=\"%s\">",
								toplevel.get("id"),
								name,
								minOccurs, 
								maxOccurs,
								toplevel.get("macro"), 
								label, 
								help));
				Object contents = toplevel.get("contents");

				if (contents instanceof Required) {
					print((Required) contents);
				}
				else if (contents instanceof Optional) {
					print((Optional) contents);
				}	
				else if (contents instanceof Alternatives) {
					print((Alternatives) contents);
				}
				else {
					printItems((List<Object>) contents);
				}				
				path.pop();
				out.unindentAndPrint("</keyword>");
				out.printIndented("");			
			}
			printFooter();
		} finally {
			out.close();
		}
	}

	private String getLabel(String name, String path) {
		String label = dfile.getLabel(path);
		return label != null ? label : name;
	}
	
	private String getHelp(String path) {
		String help = dfile.getHelp(path);
		return help != null ? help : "";
	}
	
	private String getParentLabel(String path) {
		String help = dfile.getParentLabel(path);
		return help != null ? help : "";
	}

	@SuppressWarnings("unchecked")
	private void printItems(List<Object> list) {
		for (Object o: list) {
			if (o instanceof Required) {
				print((Required) o);
			}
			else if (o instanceof Optional) {
				print((Optional) o);
			}	
			else if (o instanceof Alternatives) {
				print((Alternatives) o);
			}
			else if (o instanceof Map) {
				Map<Object, Object> keyword = (Map<Object, Object>) o;
				printKeyword(keyword, "", false);
			} 
		}		
	}
	
	private void print(Optional list) {
		printGroupWithLead(list, "optional", "minOccurs=\"0\"");
	}

	@SuppressWarnings("unchecked")
	private void printGroupWithLead(List<Object> list, String tag, String attribute) {
		if (list.get(0) instanceof Map) {
			Map<Object, Object> keyword = (Map<Object, Object>) list.remove(0);
			if (list.size() > 0) {
				printKeyword(keyword, attribute, true);
				printItems(list);
				path.pop();
				out.unindentAndPrint("</keyword>");
			} else {
				printKeyword(keyword, attribute, false);
			}
		} else {
			out.printAndIndent("<" + tag + ">");
			printItems((List<Object>) list);
			out.unindentAndPrint("</" + tag + ">");
		}
	}
	
	private String getPath(Stack<String> components) {
		StringBuilder builder = new StringBuilder();
		for (int i=0; i<components.size(); ++i) {
			builder.append(components.get(i));
			if (i < components.size() - 1)
				builder.append("/");
		}
		return builder.toString();
	}
	
	private void print(Required list) {
		printGroupWithLead(list, "required", "minOccurs=\"1\"");
	}
	
	@SuppressWarnings("unchecked")
	private void printKeyword(Map<Object, Object> keyword, String extraAttributes, boolean hasSubCommands) {
		boolean hasAliases = keyword.containsKey("alias");
		boolean hasParam = keyword.containsKey("param");
		boolean hasContent = hasSubCommands | hasAliases | hasParam;

		String name = (String) keyword.get("name");
		path.push((String) keyword.get("name"));
		final String stringPath = getPath(path);
		String label = getLabel(name, stringPath);
		String group = dfile.getGroup(stringPath);
		String help = getHelp(stringPath);
		
		if (group != null) {
			StringBuilder b = new StringBuilder(extraAttributes);
			b.append(" group=\"");
			b.append(group);
			b.append("\"");
			extraAttributes = b.toString();
		}
		
		String header = String.format("<keyword  id=\"%s\" name=\"%s\" code=\"%s\" label=\"%s\" help=\"%s\" %s %s>",
				keyword.get("id"),
				name,
				keyword.get("macro"), 
				label,
				help,
				extraAttributes,
				hasContent ? "" : "/");
		
		if (hasContent)
			out.printAndIndent(header);
		else {
			out.printIndented(header);
			path.pop();
		}
		
		if (hasAliases) {
			List<String> aliases = (List<String>) keyword.get("alias");
			for (String alias: aliases) {
				out.printIndented(String.format("<alias name=\"%s\"/>", alias));
			}
		}
		if (hasParam) {
			// Compute constraint attribute
			StringBuilder constraint = new StringBuilder();
			if (keyword.containsKey("op")) {
				constraint.append(keyword.get("op")).append(" ").append(keyword.get("number"));
			} else if (keyword.containsKey("lenspec")) {
				constraint.append(keyword.get("lenspec")).append(" ").append(keyword.get("length"));
			}

			// Assemble all attributes. Technically can only have one of these.
			StringBuilder attributes = new StringBuilder();
			if (constraint.length() > 0)
				attributes.append("constraint=\"").append(constraint.toString()).append("\" ");	
			if (keyword.containsKey("in_taglist"))
				attributes.append("in_taglist=\"").append(keyword.get("in_taglist")).append("\" ");	
			if (keyword.containsKey("taglist"))
				attributes.append("taglist=\"").append(keyword.get("taglist")).append("\" ");		
			
			String type = getParamType(keyword);		
			
			out.printIndented(String.format("<param type=\"%s\" %s/>", type, attributes.toString()));
		}
		
		if (hasContent && !hasSubCommands) {
			path.pop();
			out.unindentAndPrint("</keyword>");
		}
	}

	private static final List<String> OUTPUT_FILES = Arrays.asList(
		"tabular_data_file", "results_output_file", "history_file", "log_file",
		"export_points_file", "export_expansion_file", "export_model_file",
		"results_file", "restart_file", "output_file", "error_file"		
	);
	
	private static final List<String> INPUT_FILES = Arrays.asList(
		"import_points_file", "import_expansion_file" 
	);
	
	private String getParamType(Map<Object, Object> keyword) {		
		String nidrType = keyword.get("param").toString();
		String result = nidrType;
		String name = (String) keyword.get("name");

		// Some hard-coded "fixups"
		if (OUTPUT_FILES.contains(name))
			result = "OUTPUT_FILE";
		else if (INPUT_FILES.contains(name))
			result = "INPUT_FILE";				
		return result;
	}

	/**
	 * Ugly hack here.  The "desc" file has oneOf labels attached to one randomly-selected keyword it contains.
	 * We just have to search for it. We're assuming we'll find labels only on directly-contained keywords, although I'm
	 * not sure that's always the case.
	 * @param list
	 */
	private void print(Alternatives list) {
		String label = null;
		for (Object obj: list) {
			if (obj instanceof Map) {
				Map<Object, Object> keyword =  (Map<Object, Object>) obj;
				String name = (String) keyword.get("name");
				path.push(name);
				try {
					String parentLabel = getParentLabel(getPath(path));
					if (parentLabel != null && !parentLabel.trim().isEmpty()) {
						label = parentLabel;
						break;
					} 
				} finally {
					path.pop();
				}					
			}
		}
		if (label == null)
			out.printAndIndent("<oneOf>");
		else
			out.printAndIndent(String.format("<oneOf label=\"%s\">", label));
		printItems((List<Object>) list);		
		out.unindentAndPrint("</oneOf>");
	}
	
	private void printHeader() {
		out.printIndented("<?xml version='1.0' encoding='UTF-8'?>");
		out.printIndented("<!-- Generated code: DO NOT EDIT -->");
		out.printIndented("<document xmlns='http://www.sandia.gov/dakota/1.0' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'");
		out.printAndIndent("                  xsi:schemaLocation='http://www.sandia.gov/dakota/1.0 dakota.xsd'>");
		out.printAndIndent("<input>");
		
	}

	private void printFooter() {
		out.unindentAndPrint("</input>");
		out.unindentAndPrint("</document>");		
	}



}
