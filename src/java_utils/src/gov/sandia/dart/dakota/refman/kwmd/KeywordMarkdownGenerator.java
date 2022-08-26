package gov.sandia.dart.dakota.refman.kwmd;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import gov.sandia.dart.dakota.refman.print.KeywordMetadataMarkdownPageRSTPrinter;

public class KeywordMarkdownGenerator {
	
	////////////
	// FIELDS //
	////////////
	
	private static final String USAGE_MSG = 
		"Usage: tool.java metadata_dir output_dir";
	
	private static final int EXPECTED_ARG_COUNT = 2;
	
	////////////
	// PUBLIC //
	////////////
	
	public void run(String[] args) throws IOException  {
		if (args.length < EXPECTED_ARG_COUNT || args.length > EXPECTED_ARG_COUNT) {
			System.err.println(USAGE_MSG);
			System.exit(1);
		}
		String kw_md_dir = args[0];
		String output_dir = args[1];
				
		verifyFile(kw_md_dir);
		verifyFile(output_dir);
		
		KeywordMetadataMarkdownPageRSTPrinter printer = new KeywordMetadataMarkdownPageRSTPrinter();
		printer.print(new File(kw_md_dir), new File(output_dir));
	}
	
	/////////////
	// UTILITY //
	/////////////
	
	private void verifyFile(String filepath) throws FileNotFoundException {
		File file = new File(filepath);
		if(!file.exists()) {
			throw new FileNotFoundException(filepath + " does not exist");
		}
	}
	
	//////////
	// MAIN //
	//////////
	
	public static void main(String[] args) throws Exception {
		KeywordMarkdownGenerator app = new KeywordMarkdownGenerator();
		app.run(args);
	}
}
