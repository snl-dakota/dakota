package gov.sandia.dart.dakota.refman.print;

import java.io.IOException;

import gov.sandia.dart.dakota.refman.RefManInputSpec;
import gov.sandia.dart.dakota.refman.RefManMetaData;
import gov.sandia.dart.dakota.refman.metadata.InputSpecKeywordMetaData;
import gov.sandia.dart.dakota.refman.metadata.RefManKeywordMetaData;

public interface KeywordPrinter {
	
	public void print(String outputDir, RefManInputSpec spec_data, RefManMetaData meta_data) throws IOException;

	public String printPageTitle(String hierarchical_kwname);
	
	public String printBreadcrumbs(String hierarchical_kwname);
	
	public String printKeyword(InputSpecKeywordMetaData kw, RefManMetaData meta_data);
	
	public String printBlurb(RefManKeywordMetaData mdcontents);
	
	public String printTopics(RefManKeywordMetaData mdcontents);
	
	public String printSeeAlso(RefManKeywordMetaData mdcontents);
	
	public String printDescription(RefManKeywordMetaData mdcontents);
	
	public String printExamples(RefManKeywordMetaData mdcontents);

	public String printTheory(RefManKeywordMetaData mdcontents);
	
	public String printFAQ(RefManKeywordMetaData mdcontents);
}
