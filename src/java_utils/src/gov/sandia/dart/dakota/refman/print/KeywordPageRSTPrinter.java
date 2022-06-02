package gov.sandia.dart.dakota.refman.print;

import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.List;
import java.util.Map.Entry;

import org.apache.commons.lang3.StringUtils;

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
		
		KeywordPageRSTTablePrinter tablePrinter = new KeywordPageRSTTablePrinter(kw.getKeywordHierarchy(), children, metadata);
		sb.append(tablePrinter.print());

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
			
			String[] seeAlsoLinks = mdcontents.getSeeAlso().split("[,\n]");
			for(String link : seeAlsoLinks) {
				if(!link.isBlank()) {
					String lastSegment = link;
					if(lastSegment.contains("-")) {
						String[] tokens = link.split("-");
						lastSegment = tokens[tokens.length-1];
					}
					
					sb.append("- :ref:`").append(lastSegment.trim()).append(" <").append(link.trim()).append(">` \n");
				}
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
				String remainder = childTokens[1].trim();
				
				// This if-statement ensure that headers for one-of sections do not get treated
				// as native Dakota keywords.
				if(StringUtils.isNotBlank(remainder) &&
				   !remainder.chars().anyMatch(Character::isUpperCase) &&
				   !remainder.contains(" ")) {
					sb.append("   ").append(parentHierarchy).append("-").append(remainder).append("\n");
				}
			} else if(!child.contains("::")) {
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
}