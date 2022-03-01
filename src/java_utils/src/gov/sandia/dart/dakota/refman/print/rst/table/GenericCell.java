package gov.sandia.dart.dakota.refman.print.rst.table;

public class GenericCell {

	private String contents;
	private int horizontalSpan;
	private int verticalSpan;
	
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
	
	public void setContents(String contents) {
		this.contents = contents;
	}
	
	public void setHorizontalSpan(int horizontalSpan) {
		this.horizontalSpan = horizontalSpan;
	}
	
	public void setVerticalSpan(int verticalSpan) {
		this.verticalSpan = verticalSpan;
	}
	
	public int getHorizontalSpan() {
		return horizontalSpan;
	}
	
	public int getVerticalSpan() {
		return verticalSpan;
	}
}
