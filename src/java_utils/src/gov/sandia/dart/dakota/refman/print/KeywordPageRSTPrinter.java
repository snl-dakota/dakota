package gov.sandia.dart.dakota.refman.print;

import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.List;
import java.util.Map.Entry;

import gov.sandia.dart.dakota.refman.RefManInputSpec;
import gov.sandia.dart.dakota.refman.RefManMetaData;
import gov.sandia.dart.dakota.refman.metadata.InputSpecKeywordMetaData;
import gov.sandia.dart.dakota.refman.metadata.RefManKeywordMetaData;

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
	public String printKeyword(InputSpecKeywordMetaData kw, RefManMetaData meta_data) {
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
		sb.append(doxygenConvert(mdcontents.getBlurb()));
		sb.append("\n\n");
		return sb.toString();
	}

	@Override
	public String printTopics(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		
		if(!mdcontents.getTopics().isBlank()) {
			sb.append("\n").append(bold("Topics")).append("\n");
			sb.append("\n");
			sb.append(doxygenConvert(mdcontents.getTopics()));
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
		sb.append(doxygenConvert(mdcontents.getDescription())).append("\n\n");
		return sb.toString();
	}

	@Override
	public String printExamples(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		
		if(!mdcontents.getExamples().isBlank()) {
			sb.append("\n").append(bold("Examples")).append("\n");
			sb.append("\n");
			sb.append(doxygenConvert(mdcontents.getExamples()));
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
			sb.append(doxygenConvert(mdcontents.getTheory()));
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
			sb.append(doxygenConvert(mdcontents.getFaq()));
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
	
	/////////////////////
	// DOXYGEN CLEANUP //
	/////////////////////
	
	protected String doxygenConvert(String original) {
		String converted = original.trim();
		
		converted = converted.replaceAll("\\%(?=\\w)", "");
		converted = converted.replaceAll("\\<b\\>\\s*", "*");
		converted = converted.replaceAll("\\s*\\<\\/b\\>", "*");
		converted = doxygenConvertLists(converted);
		converted = doxygenConvertMonospaceSections(converted);
		converted = doxygenConvertVerbatimBlock(converted);
		return converted;
	}
	
	private String doxygenConvertLists(String original) {
		StringBuilder sb = new StringBuilder();
		String[] lines = original.split("\\n|\\r\\n");
		
		boolean firstListItemFound = false;
		boolean insideListItem = false;
		for(int i = 0; i < lines.length; i++) {
			String line = lines[i];
			if(insideListItem) {
				if(line.isBlank()) {
					insideListItem = false;
					sb.append("\n");
				} else if(line.contains("\\li")) {
					String replacement = line.replace("\\li", "-");
					sb.append("\n").append(replacement).append(" ");
				} else {
					sb.append(line).append(" ");
				}
			} else if(line.contains("\\li")) {
				insideListItem = true;
				if(!firstListItemFound) {
					firstListItemFound = true;
					sb.append("\n"); // Extra spacer for top of list
				}
				String replacement = line.replace("\\li", "-");
				sb.append(replacement).append(" ");
			} else {
				sb.append(line); // Pass through
				if(i < lines.length - 1) {
					sb.append("\n");
				}
			}
		}
		return sb.toString();
	}
	
	private String doxygenConvertMonospaceSections(String original) {
		StringBuilder sb = new StringBuilder();
		String[] lines = original.split("\\n|\\r\\n");
		
		boolean insideMonospaceSection = false;
		for(int i = 0; i < lines.length; i++) {
			String line = lines[i];
			String[] words = line.split("\\s");
		
			String insertedWord = "";
			for(int j = 0; j < words.length; j++) {
				if(words[j].contains("\\c")) {
					insertedWord = words[j].replace("\\c", "");
					insideMonospaceSection = true;
				} else {
					if(insideMonospaceSection && words[j].matches("[A-Za-z0-9_,\\(\\)\\.:]+")) {
						insertedWord = "``" + words[j] + "``";
						insideMonospaceSection = false;
					} else {
						insertedWord = words[j];
					}
				}
				
				sb.append(insertedWord);
				if(words[j].isEmpty() || (!insertedWord.isEmpty() && j < words.length - 1)) {
					sb.append(" ");
				}
			}
			if(i < lines.length - 1) {
				sb.append("\n");
			}
		}
		return sb.toString();
	}
	
	private String doxygenConvertVerbatimBlock(String original) {
		StringBuilder sb = new StringBuilder();
		String[] lines = original.split("\\n|\\r\\n");
		
		boolean inVerbatimSection = false;
		for(int i = 0; i < lines.length; i++) {
			String line = lines[i];
			if(line.startsWith("\\verbatim")) {
				inVerbatimSection = true;
				sb.append("\n.. code-block::\n\n");
			} else if(line.startsWith("\\endverbatim")) {
				inVerbatimSection = false;
				sb.append("\n");
			} else if(inVerbatimSection) {
				sb.append("    ").append(line).append("\n");
			} else {
				sb.append(line); // Pass through
				if(i < lines.length - 1) {
					sb.append("\n");
				}
			}
		}
		return sb.toString();
	}
}