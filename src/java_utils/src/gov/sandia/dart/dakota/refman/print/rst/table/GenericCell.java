package gov.sandia.dart.dakota.refman.print.rst.table;

public class GenericCell {

	private final String contents;
	private final int horizontalSpan;
	private final int verticalSpan;
	
	public GenericCell(String contents) {
		this(contents, 1, 1);
	}
	
	public GenericCell(String contents, int horizontalSpan, int verticalSpan) {
		this.contents = contents;
		this.horizontalSpan = horizontalSpan;
		this.verticalSpan = verticalSpan;
	}
	
	public String getContents() {
		return contents;
	}
	
	public int getHorizontalSpan() {
		return horizontalSpan;
	}
	
	public int getVerticalSpan() {
		return verticalSpan;
	}
}
