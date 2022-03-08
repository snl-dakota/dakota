package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.List;

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
	
	public int getCellWidth(List<Integer> widths, int nativeWidthIndex) {
		if(horizontalSpan == 1) {
			return getCellWidthVerticalSpan(widths, nativeWidthIndex);
		} else {
			return getCellWidthHorizontalSpan(widths, nativeWidthIndex);
		}
	}
	
	private int getCellWidthVerticalSpan(List<Integer> widths, int nativeWidthIndex) {
		int nativeWidth = widths.get(nativeWidthIndex);
		if(nativeWidth == -1) {
			throw new IllegalStateException("For columns with vertical spans, width must be explicitly defined.");
		}
		return widths.get(nativeWidthIndex);
	} 
	
	private int getCellWidthHorizontalSpan(List<Integer> widths, int nativeWidthIndex) {
		int totalWidth = 0;
		int span = horizontalSpan;
		for(int i = nativeWidthIndex; i < nativeWidthIndex + span; i++) {
			if(i < widths.size()) {
				totalWidth += widths.get(i);
			}
		}
		for(int i = nativeWidthIndex; i < nativeWidthIndex + span - 1; i++) {
			if(i < widths.size()) {
				totalWidth ++; // Count dividers between cells.
			}
		}
		return totalWidth;
	}
	
	public CellPayload getCellFormattedContents(int cellWidth) {
		CellFormatter cellFormatter = new CellFormatter(this);
		cellFormatter.format(cellWidth);
		return cellFormatter.getPayload();
	}
	
	@Override
	public String toString() {
		return contents;
	}
}
