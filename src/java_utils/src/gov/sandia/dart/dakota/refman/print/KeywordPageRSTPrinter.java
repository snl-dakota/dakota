package gov.sandia.dart.dakota.refman.print;

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
import gov.sandia.dart.dakota.refman.print.rst.table.GenericRow;
import gov.sandia.dart.dakota.refman.print.rst.table.GenericTable;
import gov.sandia.dart.dakota.refman.print.rst.table.RstTablePrinter;

public class KeywordPageRSTPrinter implements KeywordPrinter {

	@Override
	public void print(String outputDir, RefManInputSpec spec_data, RefManMetaData meta_data) throws IOException {
		for (Entry<String, InputSpecKeywordMetaData> mdentry : spec_data.getEntries()) {
			String kwname = mdentry.getKey();
			InputSpecKeywordMetaData kwInputSpec = mdentry.getValue();
			String hierarchy = kwInputSpec.getKeywordHierarchy();
			
			try(OutputStreamWriter kw_os = new FileWriter(outputDir + "/" + hierarchy + ".rst")) {
				
				boolean has_metadata = meta_data.containsKey(kwname);
				if(!has_metadata) {
					System.err.println("Warning: Missing metadata entry for keyword " + kwname);
				}
				
				kw_os.append(printRSTReference(kwInputSpec));
				
				kw_os.append(printPageTitle(kwname));
				kw_os.append(printBreadcrumbs(kwname));
				
				if(has_metadata) {
					RefManKeywordMetaData kwMetadata = meta_data.getKeywordMetadata(kwname);
					kw_os.append(printBlurb(kwMetadata));
					kw_os.append(printTopics(kwMetadata));
				}
				
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
	public String printKeyword(InputSpecKeywordMetaData kw, RefManMetaData metadata) {
		StringBuilder sb = new StringBuilder();
		
		String alias = kw.getAlias();
		String argument = kw.getArgument();
		String defaultProperty = kw.getDefaultProperty();
		List<String> children = kw.getChildren();
		
		sb.append(printHiddenChildKeywordTOC(kw.getKeywordHierarchy(), children));
		
		sb.append("\n").append(bold("Specification")).append("\n");

		if(alias.trim().isEmpty()) {
			sb.append("\n- ").append(italic("Alias:")).append(" None\n");
		} else {
			sb.append("\n- ").append(italic("Alias:")).append(" ").append(alias).append("\n");
		}

		if(argument.trim().isEmpty()) {
			sb.append("\n- ").append(italic("Arguments:")).append(" None\n");
		} else {
			sb.append("\n- ").append(italic("Arguments:")).append(" ").append(argument).append("\n");
		}

		if(!defaultProperty.trim().isEmpty()) {
			sb.append("\n- ").append(italic("Default:")).append(" ").append(defaultProperty).append("\n");
		}
		
		sb.append(printChildKeywordTable(kw.getKeywordHierarchy(), children, metadata));

		sb.append("\n");
		return sb.toString();
	}

	@Override
	public String printPageTitle(String hierarchical_kwname) {
		String [] split_kw = hierarchical_kwname.split("-");
		String name = split_kw[split_kw.length - 1];
		
		StringBuilder sb = new StringBuilder();
		sb.append(characterLoop("\"", name.length()));
		sb.append("\n").append(name).append("\n");
		sb.append(characterLoop("\"", name.length()));
		sb.append("\n\n");
		return sb.toString();
	}

	@Override
	public String printBreadcrumbs(String hierarchical_kwname) {
		// No need to print breadcrumbs - RST/Sphinx provides this for us automatically!
		return "";
	}

	@Override
	public String printBlurb(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		sb.append(DoxygenToRSTConverter.convert(mdcontents.getBlurb()));
		sb.append("\n\n");
		return sb.toString();
	}

	@Override
	public String printTopics(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		
		if(!mdcontents.getTopics().isBlank()) {
			sb.append("\n").append(bold("Topics")).append("\n");
			sb.append("\n");
			sb.append(DoxygenToRSTConverter.convert(mdcontents.getTopics()));
			sb.append("\n\n");
		}
		return sb.toString();
	}

	@Override
	public String printSeeAlso(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		
		if(!mdcontents.getSeeAlso().isBlank()) {
			sb.append("\n").append(bold("See Also")).append("\n");
			sb.append("\n");
			
			String[] seeAlsoLinks = mdcontents.getSeeAlso().split(",");
			for(String link : seeAlsoLinks) {
				String lastSegment = link;
				if(lastSegment.contains("-")) {
					String[] tokens = link.split("-");
					lastSegment = tokens[tokens.length-1];
				}
				
				sb.append("- :ref:`").append(lastSegment.trim()).append(" <").append(link.trim()).append(">` \n");
			}
			sb.append("\n\n");
		}
		return sb.toString();
	}

	@Override
	public String printDescription(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		
		sb.append("\n").append(bold("Description")).append("\n");
		sb.append("\n");
		sb.append(DoxygenToRSTConverter.convert(mdcontents.getDescription())).append("\n\n");
		return sb.toString();
	}

	@Override
	public String printExamples(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		
		if(!mdcontents.getExamples().isBlank()) {
			sb.append("\n").append(bold("Examples")).append("\n");
			sb.append("\n");
			sb.append(DoxygenToRSTConverter.convert(mdcontents.getExamples()));
			sb.append("\n\n");
		}
		return sb.toString();
	}

	@Override
	public String printTheory(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		
		if(!mdcontents.getTheory().isBlank()) {
			sb.append("\n").append(bold("Theory")).append("\n");
			sb.append("\n");
			sb.append(DoxygenToRSTConverter.convert(mdcontents.getTheory()));
			sb.append("\n\n");
		}
		return sb.toString();
	}

	@Override
	public String printFAQ(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		
		if(!mdcontents.getFaq().isBlank()) {
			sb.append("\n").append(bold("FAQ")).append("\n");
			sb.append("\n");
			sb.append(DoxygenToRSTConverter.convert(mdcontents.getFaq()));
			sb.append("\n\n");
		}
		return sb.toString();
	}
	
	/////////////
	// PRIVATE //
	/////////////

	private String characterLoop(String character, int times) {
		StringBuilder sb = new StringBuilder();
		for(int i = 0; i < times; i++) {
			sb.append(character);
		}
		return sb.toString();
	}
	
	private String printRSTReference(InputSpecKeywordMetaData kw) {
		StringBuilder sb = new StringBuilder();
		String parentHierarchy = kw.getKeywordHierarchy();
		
		sb.append(".. _").append(parentHierarchy.trim()).append(":\n");
		sb.append("\n");
		return sb.toString();
	}
	
	private String printHiddenChildKeywordTOC(String parentHierarchy, List<String> children) {
		StringBuilder sb = new StringBuilder();
		sb.append(".. toctree::\n");
		sb.append("   :hidden:\n");
		sb.append("   :maxdepth: 1\n\n");
		for(String child : children) {
			if(child.contains(":: ")) {
				String[] childTokens = child.split(":: ");
				sb.append("   ").append(parentHierarchy).append("-").append(childTokens[1]).append("\n");
			} else {
				sb.append("   ").append(parentHierarchy).append("-").append(child.trim()).append("\n");
			}
		}
		sb.append("\n");
		return sb.toString();
	}
	
	private String bold(String original) {
		StringBuilder sb = new StringBuilder();
		sb.append("**").append(original).append("**");
		return sb.toString();
	}
	
	private String italic(String original) {
		StringBuilder sb = new StringBuilder();
		sb.append("*").append(original).append("*");
		return sb.toString();
	}
	
	private String printChildKeywordTable(String parentPrefix, List<String> childKeywords, RefManMetaData metadata) {
		List<TableKeyword> keywords = new ArrayList<>();
		TableKeyword oneOfKeyword = null;
		
		boolean inOptionalChooseOneSection = false;
		boolean inRequiredChooseOneSection = false;
		
		for(String child : childKeywords) {
			String trimChildName = child.trim();
			String tokenChildName = "";
			if(child.contains("::")) {
				String[] tokens = child.split("::");
				if(tokens.length > 1) {
					tokenChildName = child.split("::")[1].trim();
				}
			}
			String description = "";
			if(!tokenChildName.isBlank() && !trimChildName.contains("_Choose_One::")) {
				description = metadata.getBlurb(parentPrefix + "-" + tokenChildName).trim();
			}
			
			if(!tokenChildName.isBlank()) {
				if(child.startsWith("Optional_Keyword:: ")) {
					inOptionalChooseOneSection = false;
					inRequiredChooseOneSection = false;
					if(oneOfKeyword != null) {
						keywords.add(oneOfKeyword);
						oneOfKeyword = null;
					}
					
					TableKeyword optKeyword = new TableKeyword(tokenChildName, description, true, false);
					keywords.add(optKeyword);
				} else if(child.startsWith("Required_Keyword:: ")) {
					inOptionalChooseOneSection = false;
					inRequiredChooseOneSection = false;
					TableKeyword reqKeyword = new TableKeyword(tokenChildName, description, false, true);
					keywords.add(reqKeyword);
				} else if(child.startsWith("Optional_Choose_One:: ")) {
					if(oneOfKeyword != null) {
						keywords.add(oneOfKeyword);
						oneOfKeyword = null;
					}
					inOptionalChooseOneSection = true;
					inRequiredChooseOneSection = false;
					oneOfKeyword = new TableKeyword("", description, true, false);
					oneOfKeyword.setOptionalOrRequiredGroupName(tokenChildName);
				} else if(child.startsWith("Required_Choose_One:: ")) {
					if(oneOfKeyword != null) {
						keywords.add(oneOfKeyword);
						oneOfKeyword = null;
					}
					inOptionalChooseOneSection = false;
					inRequiredChooseOneSection = true;
					oneOfKeyword = new TableKeyword("", description, false, true);
					oneOfKeyword.setOptionalOrRequiredGroupName(tokenChildName);
				} else {
					if(oneOfKeyword != null && (inOptionalChooseOneSection || inRequiredChooseOneSection)) {
						TableKeyword subKeyword = new TableKeyword(tokenChildName, description, false, false);
						oneOfKeyword.getOneOf().add(subKeyword);
					} else {
						System.err.println("ERROR - not sure what to do with keyword " + tokenChildName);
					}
				}
			}
		}
		if(oneOfKeyword != null) {
			keywords.add(oneOfKeyword);
			oneOfKeyword = null;
		}
		
		// Now, print the table.
		GenericTable table = new GenericTable();
		table.setColumnWidth(0, 25);
		
		GenericRow header = new GenericRow();
		header.addCell("Required/Optional");
		header.addCell("Description of Group");
		header.addCell("Dakota Keyword");
		header.addCell("Dakota Keyword Description");
		
		List<GenericRow> rows = new ArrayList<>();
		for(TableKeyword tableKeyword : keywords) {
			if(tableKeyword.getOneOf().isEmpty()) {
				GenericRow simpleRow = new GenericRow();
				if(tableKeyword.getOptional()) {
					simpleRow.addCell("Optional", 2, 1);
					simpleRow.addCell(tableKeyword.getKeyword());
					simpleRow.addCell(tableKeyword.getDescription());					
				} else {
					simpleRow.addCell("Required", 2, 1);
					simpleRow.addCell(tableKeyword.getKeyword());
					simpleRow.addCell(tableKeyword.getDescription());
				}
				rows.add(simpleRow);
			} else {
				if(tableKeyword.getOptional()) {
					GenericRow optionalOneOfRow = new GenericRow();
					int verticalSpan = tableKeyword.getOneOf().size();
					optionalOneOfRow.addCell("Optional (Choose One)", 1, verticalSpan);
					optionalOneOfRow.addCell(tableKeyword.getOptionalOrRequiredGroupName(), 1, verticalSpan);
					
					TableKeyword firstKeyword = tableKeyword.getOneOf().get(0);
					optionalOneOfRow.addCell(firstKeyword.getKeyword());
					optionalOneOfRow.addCell(firstKeyword.getDescription());
					rows.add(optionalOneOfRow);
					for(int i = 1; i < tableKeyword.getOneOf().size(); i++) {
						TableKeyword nextKeyword = tableKeyword.getOneOf().get(i);
						GenericRow optionRow = new GenericRow();
						optionRow.addSpanHoldCell();
						optionRow.addSpanHoldCell();
						optionRow.addCell(nextKeyword.getKeyword());
						optionRow.addCell(nextKeyword.getDescription());
						rows.add(optionRow);
					}
				} else {
					GenericRow requiredOneOfRow = new GenericRow();
					int verticalSpan = tableKeyword.getOneOf().size();
					requiredOneOfRow.addCell("Required (Choose One)", 1, verticalSpan);
					requiredOneOfRow.addCell(tableKeyword.getOptionalOrRequiredGroupName(), 1, verticalSpan);
					
					TableKeyword firstKeyword = tableKeyword.getOneOf().get(0);
					requiredOneOfRow.addCell(firstKeyword.getKeyword());
					requiredOneOfRow.addCell(firstKeyword.getDescription());
					rows.add(requiredOneOfRow);
					for(int i = 1; i < tableKeyword.getOneOf().size(); i++) {
						TableKeyword nextKeyword = tableKeyword.getOneOf().get(i);
						GenericRow optionRow = new GenericRow();
						optionRow.addSpanHoldCell();
						optionRow.addSpanHoldCell();
						optionRow.addCell(nextKeyword.getKeyword());
						optionRow.addCell(nextKeyword.getDescription());
						rows.add(optionRow);
					}
				}
			}
		}
		
		if(!rows.isEmpty()) {
			
			table.addRow(header);
			for(GenericRow row : rows) {
				table.addRow(row);
			}			
			RstTablePrinter printer = new RstTablePrinter();
			// System.out.println("**************" + parentPrefix + "***************");
			
			StringBuilder sb = new StringBuilder();
			sb.append("\n\n**Child Keywords:**\n\n");
			sb.append(printer.print(table)).append("\n\n");
			//sb.append("Keyword link dump here"); //TODO
			// System.out.println(sb.toString());
			return sb.toString();
		}
		return "";
	}
}