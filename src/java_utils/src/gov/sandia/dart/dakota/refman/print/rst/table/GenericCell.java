package gov.sandia.dart.dakota.refman.print.rst.table;

public class GenericCell {

	private String contents;
	private int horizontalSpan;
	private int verticalSpan;
	private final int leftPadding;
	private final int rightPadding;
	
	public GenericCell(String contents) {
		this(contents, 1, 1);
	}
	
	public GenericCell(String contents, int horizontalSpan, int verticalSpan) {
		this(contents, horizontalSpan, verticalSpan, 1, 1);
	}
	
	public GenericCell(String contents, int horizontalSpan, int verticalSpan, int leftPadding, int rightPadding) {
		this.contents = contents;
		this.horizontalSpan = horizontalSpan;
		this.verticalSpan = verticalSpan;
		this.leftPadding = leftPadding;
		this.rightPadding = rightPadding;
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

	public int getLeftPadding() {
		return leftPadding;
	}
	
	public int getRightPadding() {
		return rightPadding;
	}
	
	@Override
	public String toString() {
		return contents;
	}
}
