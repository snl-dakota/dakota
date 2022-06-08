package gov.sandia.dart.dakota.refman.print;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.commons.io.FileUtils;

import gov.sandia.dart.dakota.refman.RefManMetaData;
import gov.sandia.dart.dakota.refman.metadata.RefManKeywordMetaData;

public class KeywordMetadataMarkdownPageRSTPrinter {
	
	//////////////
	// OVERRIDE //
	//////////////

	public void print(File sourceDir, File outputDir) throws IOException {
		Map<String, RefManKeywordMetaData> metadata = parseOldKeywordMetadataFile(sourceDir);
		
		for(Entry<String, RefManKeywordMetaData> entry : metadata.entrySet()) {
			String outputFilename = entry.getKey();
			RefManKeywordMetaData metadatum = entry.getValue();
			
			File destinationFile = new File(outputDir, outputFilename);
			try(OutputStreamWriter kw_os = new FileWriter(destinationFile)) {
				kw_os.append(printBlurb(metadatum));
				kw_os.append(printDescription(metadatum));
				kw_os.append(printTopics(metadatum));
				kw_os.append(printExamples(metadatum));
				kw_os.append(printTheory(metadatum));
				kw_os.append(printFAQ(metadatum));
				kw_os.append(printSeeAlso(metadatum));		
			}
		}
	}
	
	private Map<String, RefManKeywordMetaData> parseOldKeywordMetadataFile(File sourceDir) throws IOException {
		Map<String, RefManKeywordMetaData> metadata = new HashMap<>();
		for(File childFile : sourceDir.listFiles()) {
			String fileContents = FileUtils.readFileToString(childFile, StandardCharsets.UTF_8);
			String[] lines = fileContents.split("\r\n|\n");
			
			String section = "";
			StringBuilder sb = new StringBuilder();
			RefManKeywordMetaData metadatum = new RefManKeywordMetaData();
			
			for(String line : lines) {
				if(RefManMetaData.lineStartsWithDakotaSectionHeader(line)) {
					if(!section.isBlank()) {
						String converted = DoxygenToRSTConverter.convert(sb.toString()) + "\n";
						
						if(section.equals("Blurb")) {
							metadatum.setBlurb(converted);
						} else if(section.equals("Description")) {
							metadatum.setDescription(converted);
						} else if(section.equals("Topics")) {
							metadatum.setTopics(converted);
						} else if(section.equals("Examples")) {
							metadatum.setExamples(converted);
						} else if(section.equals("Theory")) {
							metadatum.setTheory(converted);
						} else if(section.equals("Faq")) {
							metadatum.setFaq(converted);
						} else if(section.equals("See_Also")) {
							metadatum.setSeeAlso(converted);
						}
					}
					section = line.split("::")[0];
					sb = new StringBuilder();
				} else {
					sb.append(line).append("\n");
				}
			}
			
			metadata.put(childFile.getName(), metadatum);
		}
		return metadata;
	}

	public String printBlurb(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		sb.append("Blurb::\n");
		sb.append(mdcontents.getBlurb());
		return sb.toString();
	}

	public String printTopics(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();	
		sb.append("Topics::\n");
		sb.append(mdcontents.getTopics());
		return sb.toString();
	}

	public String printSeeAlso(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();	
		sb.append("See_Also::\n");
		sb.append(mdcontents.getSeeAlso());
		return sb.toString();
	}

	public String printDescription(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		sb.append("Description::\n");
		sb.append(mdcontents.getDescription());
		return sb.toString();
	}

	public String printExamples(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		sb.append("Examples::").append("\n");
		sb.append(mdcontents.getExamples());
		return sb.toString();
	}

	public String printTheory(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		sb.append("Theory::").append("\n");
		sb.append(mdcontents.getTheory());
		return sb.toString();
	}

	public String printFAQ(RefManKeywordMetaData mdcontents) {
		StringBuilder sb = new StringBuilder();
		sb.append("Faq::").append("\n");
		sb.append(mdcontents.getFaq());
		return sb.toString();
	}
}