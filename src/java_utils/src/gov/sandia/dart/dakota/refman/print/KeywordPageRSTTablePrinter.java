package gov.sandia.dart.dakota.refman.print;

import java.util.ArrayList;
import java.util.List;

import gov.sandia.dart.dakota.refman.RefManMetaData;
import gov.sandia.dart.dakota.refman.print.rst.table.GenericRow;
import gov.sandia.dart.dakota.refman.print.rst.table.GenericTable;
import gov.sandia.dart.dakota.refman.print.rst.table.RstTablePrinter;

/**
 * 
 * @author Elliott Ridgway
 *
 */
public class KeywordPageRSTTablePrinter {
	
	////////////
	// FIELDS //
	////////////
	
	private final String parentPrefix;
	private final List<String> childKeywords;
	private RefManMetaData metadata;
	private List<String> links;
	
	List<TableKeyword> keywords = new ArrayList<>();
	TableKeyword oneOfKeyword = null;
	boolean inOptionalChooseOneSection = false;
	boolean inRequiredChooseOneSection = false;
	
	/////////////////
	// CONSTRUCTOR //
	/////////////////
	
	public KeywordPageRSTTablePrinter(String parentPrefix, List<String> childKeywords, RefManMetaData metadata) {
		this.parentPrefix = parentPrefix;
		this.childKeywords = new ArrayList<>();
		this.childKeywords.addAll(childKeywords);
		this.metadata = metadata;
		
		this.links = new ArrayList<>();
	}
	
	//////////
	// MAIN //
	//////////

	public String print() {
		gatherTableKeywords();
		return paintKeywordTable(keywords);
	}
	
	////////////////////
	// TABLE KEYWORDS //
	////////////////////
	
	private void gatherTableKeywords() {
		for(String child : childKeywords) { 
			String name = getName(child);
			String description = getDescription(child);
						
			if(child.startsWith("Optional_Keyword::")) {
				keywords.add(createOptionalTableKeyword(name, description));
			} else if(child.startsWith("Required_Keyword::")) {
				keywords.add(createRequiredTableKeyword(name, description));
			} else if(child.startsWith("Optional_Choose_One::")) {
				startOptionalChooseOneKeyword(name, description);
			} else if(child.startsWith("Required_Choose_One::")) {
				startRequiredChooseOneKeyword(name, description);
			} else {
				continueWithNextChooseOneKeyword(name, description);
			}
		}
		resetSectionTrackers();
	}
	
	private String getName(String child) {
		String trimChildName = child.trim();
		String tokenChildName = "";
		if(child.contains("::")) {
			String[] tokens = child.split("::");
			if(tokens.length > 1) {
				tokenChildName = child.split("::")[1].trim();
			}
		} else if(inOptionalChooseOneSection || inRequiredChooseOneSection) {
			tokenChildName = trimChildName;
		}
		return tokenChildName;
	}
	
	private String getDescription(String child) {
		String trimChildName = child.trim();
		String tokenChildName = getName(child);
		String description = "";
		if(!tokenChildName.isBlank() && !trimChildName.contains("_Choose_One::")) {
			description = metadata.getBlurb(parentPrefix + "-" + tokenChildName).trim();
			description = description.replaceAll("\n|\r\n", " ");
			description = description.replace("  ", " ");
		}
		return description;
	}
	
	private void resetSectionTrackers() {
		inOptionalChooseOneSection = false;
		inRequiredChooseOneSection = false;
		if(oneOfKeyword != null) {
			keywords.add(oneOfKeyword);
			oneOfKeyword = null;
		}
	}
	
	private TableKeyword createOptionalTableKeyword(String name, String description) {
		resetSectionTrackers();
		return new TableKeyword(name, description, true, false);
	}
	
	private TableKeyword createRequiredTableKeyword(String name, String description) {
		resetSectionTrackers();
		return  new TableKeyword(name, description, false, true);
	}
	
	private void startOptionalChooseOneKeyword(String name, String description) {
		if(oneOfKeyword != null) {
			keywords.add(oneOfKeyword);
			oneOfKeyword = null;
		}
		inOptionalChooseOneSection = true;
		inRequiredChooseOneSection = false;
		oneOfKeyword = new TableKeyword("", description, true, false);
		if(name.isBlank()) {
			oneOfKeyword.setOptionalOrRequiredGroupName("Optional (Choose One)");
		} else {
			oneOfKeyword.setOptionalOrRequiredGroupName(name);
		}
	}
	
	private void startRequiredChooseOneKeyword(String name, String description) {
		if(oneOfKeyword != null) {
			keywords.add(oneOfKeyword);
			oneOfKeyword = null;
		}
		inOptionalChooseOneSection = false;
		inRequiredChooseOneSection = true;
		oneOfKeyword = new TableKeyword("", description, false, true);
		if(name.isBlank()) {
			oneOfKeyword.setOptionalOrRequiredGroupName("Required (Choose One)");
		} else {
			oneOfKeyword.setOptionalOrRequiredGroupName(name);
		}
	}
	
	private void continueWithNextChooseOneKeyword(String name, String description) {
		if(oneOfKeyword != null && (inOptionalChooseOneSection || inRequiredChooseOneSection)) {
			TableKeyword subKeyword = new TableKeyword(name, description, false, false);
			oneOfKeyword.getOneOf().add(subKeyword);
		} else {
			if(name.isBlank()) {
				System.err.println("ERROR - we have a keyword with no name");
			} else {
				System.err.println("ERROR - not sure what to do with keyword " + name);
			}
		}
	}
	
	////////////////////
	// TABLE PAINTING //
	////////////////////
	
	private String paintKeywordTable(List<TableKeyword> keywords) {	
		List<GenericRow> rows = new ArrayList<>();
		
		GenericRow header = new GenericRow();
		header.addCell("Required/Optional");
		header.addCell("Description of Group");
		header.addCell("Dakota Keyword");
		header.addCell("Dakota Keyword Description");
		rows.add(header);
		
		
		for(TableKeyword tableKeyword : keywords) {
			if(tableKeyword.getOneOf().isEmpty()) {
				if(tableKeyword.getOptional()) {
					rows.add(createOptionalRow(tableKeyword));
				} else {
					rows.add(createRequiredRow(tableKeyword));
				}
			} else {
				if(tableKeyword.getOptional()) {
					rows.addAll(createOptionalOneOfRows(tableKeyword));
				} else {
					rows.addAll(createRequiredOneOfRows(tableKeyword));
				}
			}
		}
		
		int widestKeyword = 20; // Default
		final int linkWrapperLength = 6; // Every keyword is wrapped in RST link markup, like so: `keyword`__
		                                 // That's 4 extra characters, plus 2 for the default left and right cell padding.
		for(TableKeyword tableKeyword : keywords) {
			widestKeyword = Math.max(tableKeyword.getKeyword().length() + linkWrapperLength, widestKeyword);
			if(!tableKeyword.getOneOf().isEmpty()) {
				for(TableKeyword subKeyword : tableKeyword.getOneOf()) {
					widestKeyword = Math.max(subKeyword.getKeyword().length() + linkWrapperLength, widestKeyword);
				}
			}
		}
		
		int widestWordInKeywordDescriptionCell = 45; // Default
		for(GenericRow row : rows) {
			if(row.getData().size() >= 4) {
				String[] words = row.getData().get(3).getContents().split(" ");
				for(String word : words) {
					if(word.length() >= widestWordInKeywordDescriptionCell) {
						widestWordInKeywordDescriptionCell = word.length();
					}
				}
			}
		}
		widestWordInKeywordDescriptionCell += 2; // for cell padding on left and right
		
		if(rows.size() > 1) { // More than just a header row
			GenericTable table = new GenericTable();
			
			// These column widths are heuristic, and can be freely altered by future developers.
			table.setColumnWidth(0, 25);
			table.setColumnWidth(1, 20);
			table.setColumnWidth(2, widestKeyword);
			table.setColumnWidth(3, widestWordInKeywordDescriptionCell);
			
			for(GenericRow row : rows) {
				table.addRow(row);
			}			
			
			StringBuilder sb = new StringBuilder();
			sb.append("\n\n**Child Keywords:**\n\n");
			
			RstTablePrinter printer = new RstTablePrinter();
			sb.append(printer.print(table));
			sb.append("\n");
			
			sb.append(printLinks(links));
			
			return sb.toString();
		}
		return "";
	}
	
	private GenericRow createOptionalRow(TableKeyword tableKeyword) {
		GenericRow simpleRow = new GenericRow();
		simpleRow.addCell("Optional", 2, 1);
		simpleRow.addCell(hyperlink(tableKeyword.getKeyword()));
		simpleRow.addCell(tableKeyword.getDescription());
		links.add(buildChildLink(parentPrefix, tableKeyword.getKeyword()));
		return simpleRow;
	}
	
	private GenericRow createRequiredRow(TableKeyword tableKeyword) {
		GenericRow simpleRow = new GenericRow();
		simpleRow.addCell("Required", 2, 1);
		simpleRow.addCell(hyperlink(tableKeyword.getKeyword()));
		simpleRow.addCell(tableKeyword.getDescription());
		links.add(buildChildLink(parentPrefix, tableKeyword.getKeyword()));
		return simpleRow;
	}
	
	private List<GenericRow> createOptionalOneOfRows(TableKeyword tableKeyword) {
		List<GenericRow> oneOfRows = new ArrayList<>();
		
		GenericRow optionalOneOfRow = new GenericRow();
		int verticalSpan = tableKeyword.getOneOf().size();
		optionalOneOfRow.addCell("Optional (Choose One)", 1, verticalSpan);
		optionalOneOfRow.addCell(tableKeyword.getOptionalOrRequiredGroupName(), 1, verticalSpan);
		
		TableKeyword firstKeyword = tableKeyword.getOneOf().get(0);
		optionalOneOfRow.addCell(hyperlink(firstKeyword.getKeyword()));
		optionalOneOfRow.addCell(firstKeyword.getDescription());
		links.add(buildChildLink(parentPrefix, firstKeyword.getKeyword()));
		oneOfRows.add(optionalOneOfRow);
		for(int i = 1; i < tableKeyword.getOneOf().size(); i++) {
			TableKeyword nextKeyword = tableKeyword.getOneOf().get(i);
			GenericRow optionRow = new GenericRow();
			optionRow.addSpanHoldCell();
			optionRow.addSpanHoldCell();
			optionRow.addCell(hyperlink(nextKeyword.getKeyword()));
			optionRow.addCell(nextKeyword.getDescription());
			links.add(buildChildLink(parentPrefix, nextKeyword.getKeyword()));
			oneOfRows.add(optionRow);
		}
		
		return oneOfRows;
	}
	
	private List<GenericRow> createRequiredOneOfRows(TableKeyword tableKeyword) {
		List<GenericRow> oneOfRows = new ArrayList<>();
		
		GenericRow requiredOneOfRow = new GenericRow();
		int verticalSpan = tableKeyword.getOneOf().size();
		requiredOneOfRow.addCell("Required (Choose One)", 1, verticalSpan);
		requiredOneOfRow.addCell(tableKeyword.getOptionalOrRequiredGroupName(), 1, verticalSpan);
		
		TableKeyword firstKeyword = tableKeyword.getOneOf().get(0);
		requiredOneOfRow.addCell(hyperlink(firstKeyword.getKeyword()));
		requiredOneOfRow.addCell(firstKeyword.getDescription());
		links.add(buildChildLink(parentPrefix, firstKeyword.getKeyword()));
		oneOfRows.add(requiredOneOfRow);
		for(int i = 1; i < tableKeyword.getOneOf().size(); i++) {
			TableKeyword nextKeyword = tableKeyword.getOneOf().get(i);
			GenericRow optionRow = new GenericRow();
			optionRow.addSpanHoldCell();
			optionRow.addSpanHoldCell();
			optionRow.addCell(hyperlink(nextKeyword.getKeyword()));
			optionRow.addCell(nextKeyword.getDescription());
			oneOfRows.add(optionRow);
			links.add(buildChildLink(parentPrefix, nextKeyword.getKeyword()));
		}
		
		return oneOfRows;
	}
	
	private String buildChildLink(String parentPrefix, String keyword) {
		return parentPrefix + "-" + keyword + ".html";
	} 
	
	private String hyperlink(String printedText) {
		StringBuilder sb = new StringBuilder();
		sb.append("`").append(printedText).append("`__");
		return sb.toString();
	}
	
	private String printLinks(List<String> links) {
		StringBuilder sb = new StringBuilder();
		sb.append(".. __: ");
		for(int i = 0; i < links.size(); i++) {
			String link = links.get(i);
			sb.append(link);
			if(i < links.size() - 1) {
				sb.append("\n__ ");
			}
		}
		sb.append("\n\n");
		return sb.toString();
	}
}
