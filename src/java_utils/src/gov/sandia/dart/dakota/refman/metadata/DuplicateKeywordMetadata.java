package gov.sandia.dart.dakota.refman.metadata;

public class DuplicateKeywordMetadata extends RefManKeywordMetaData {

	private final String duplicateFileContents;
	
	public DuplicateKeywordMetadata(String duplicateFileContents) {
		this.duplicateFileContents = duplicateFileContents;
	}
	
	public String getDuplicateFileContents() {
		return duplicateFileContents;
	}
}
