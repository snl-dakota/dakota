package gov.sandia.dart.dakota.refman.print;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

import org.apache.commons.io.FileUtils;

public class TopicDoxygenToRstConverter {

	////////////
	// FIELDS //
	////////////
	
	private static final String USAGE_MSG = 
		"Usage: tool.java topic_dir output_dir";
	
	private static final int EXPECTED_ARG_COUNT = 2;
	
	////////////
	// PUBLIC //
	////////////
	
	public void run(String[] args) throws IOException  {
		if (args.length < EXPECTED_ARG_COUNT || args.length > EXPECTED_ARG_COUNT) {
			System.err.println(USAGE_MSG);
			System.exit(1);
		}
		String topic_dir = args[0];
		String output_dir = args[1];
				
		verifyFile(topic_dir);
		verifyFile(output_dir);
		
		File topicDirFile = new File(topic_dir);
		File outputDirFile = new File(output_dir);
		for(File topicFile : topicDirFile.listFiles()) {
			String topicFileContents = FileUtils.readFileToString(topicFile, StandardCharsets.UTF_8);
			String result = DoxygenToRSTConverter.convert(topicFileContents);
			File outputFile = new File(outputDirFile, topicFile.getName() + ".rst");
			FileUtils.writeStringToFile(outputFile, result, StandardCharsets.UTF_8);
		}
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
		TopicDoxygenToRstConverter app = new TopicDoxygenToRstConverter();
		app.run(args);
	}
}
