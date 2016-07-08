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

import java.io.IOException;
import java.io.OutputStreamWriter;
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

@SuppressWarnings("serial")
public class NIDROutlineGenerator extends NIDRBaseListener{
	static class Required extends ArrayList<Object> {}
	static class Optional extends ArrayList<Object> {}
	static class Alternatives extends ArrayList<Object> {}
			
	public static void main(String[] args) throws IOException {
		ANTLRFileStream input = new ANTLRFileStream("dakota.input.nspec");
		NIDRLexer lexer = new NIDRLexer(input);
		CommonTokenStream tokens = new CommonTokenStream(lexer);
		NIDRParser parser = new NIDRParser(tokens);
		ParseTree tree = parser.input();
		ParseTreeWalker walker = new ParseTreeWalker();
		NIDROutlineGenerator tool = new NIDROutlineGenerator();
		walker.walk(tool, tree);
		tool.print();		
	}

	private Map<String, Integer> keywordCounts = new HashMap<String, Integer>();
	
	List<Object> toplevels = new ArrayList<Object>();
	Stack<Object> scopes = new Stack<Object>();

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

	@SuppressWarnings("unchecked")
	private void print() throws IOException {
		out = new Indenter(new OutputStreamWriter(System.out));
		try {
			printHeader();
			for (Object o: toplevels) {
				Map<Object, Object> toplevel = (Map<Object, Object>) o;				
				out.printIndented(
						String.format("Keyword_Hierarchy::\t%s",
								toplevel.get("name")));						
				out.printAndIndent(
						String.format("Name::\t%s",
								toplevel.get("name")));						
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

				out.unindentAndPrint("");			
				out.printIndented("");			
			}
			printFooter();
		} finally {
			out.close();
		}
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
				printKeyword(keyword, false);
			} 
		}		
	}
	
	private void print(Optional list) {
		printGroupWithLead(list, "Optional_Keywords::", true);
	}

	@SuppressWarnings("unchecked")
	private void printGroupWithLead(List<Object> list, String tag, boolean optional) {
		if (list.get(0) instanceof Map) {
			String label = optional ? "Optional_Keyword::" : "Required_Keyword::";
			Map<Object, Object> keyword = (Map<Object, Object>) list.remove(0);
			if (list.size() > 0) {
				printTaggedKeyword(label, keyword, true);
				printItems(list);
				out.unindent();
			} else {
				printTaggedKeyword(label, keyword,  false);
			}
		} else {
			out.printAndIndent(tag);
			printItems((List<Object>) list);
			out.unindent();
		}
	}
	
	private void print(Required list) {
		printGroupWithLead(list, "Required_Keywords::", false);
	}
	
	private void printKeyword(Map<Object, Object> keyword, boolean hasSubCommands) {
		boolean hasAliases = keyword.containsKey("alias");
		boolean hasParam = keyword.containsKey("param");

		StringBuilder builder = new StringBuilder(keyword.get("name").toString());
		if (hasAliases)
			builder.append(" ALIAS ").append(keyword.get("alias"));
		if (hasParam)
			builder.append(" ").append(keyword.get("param"));
				
		if (hasSubCommands)
			out.printAndIndent(builder.toString());
		else 
			out.printIndented(builder.toString());	
	}
	
	private void printTaggedKeyword(String tag, Map<Object, Object> keyword, boolean hasSubCommands) {
		Map<Object, Object> taggedKeyword = new HashMap<Object, Object>(keyword);
		taggedKeyword.put("name", tag + "\t" + keyword.get("name"));
		printKeyword(taggedKeyword, hasSubCommands);
	}
	
	private void print(Alternatives list) {
		out.printAndIndent("Choose_One::");
		printItems((List<Object>) list);		
		out.unindent();
	}
	
	private void printHeader() {
	}

	private void printFooter() {
	}



}
