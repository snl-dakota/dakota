package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.ArrayList;
import java.util.List;

public class GenericRow {

	private List<GenericCell> data = new ArrayList<>();
	private boolean lastRow;
	
	public List<GenericCell> getData() {
		return data;
	}
	
	public boolean isLastRow() {
		return lastRow;
	}
	
	public void setLastRow(boolean lastRow) {
		this.lastRow = lastRow;
	}
	
	public void addCell(GenericCell cell) {
		data.add(cell);
	}
	
	public void addCell(String cellContents) {
		addCell(cellContents, 1, 1);
	}
	
	public void addCell(String cellContents, int horizontalSpan, int verticalSpan) {
		addCell(cellContents, horizontalSpan, verticalSpan, 1, 1);
	}
	
	public void addCell(String cellContents, int horizontalSpan, int verticalSpan, int leftPadding, int rightPadding) {
		GenericCell newCell = new GenericCell(cellContents, horizontalSpan, verticalSpan, leftPadding, rightPadding);
		data.add(newCell);
	}
	
	public void addSpanHoldCell() {
		SpanHoldCell newCell = new SpanHoldCell();
		data.add(newCell);
	}
	
	public boolean elementsAreBlank() {
		for(GenericCell element : data) {
			if(!element.getContents().isBlank()) {
				return false;
			}
		}
		return true;
	}
	
	public boolean onlySpanningCellsShouldContinue() {
		for(GenericCell element : data) {
			if(!element.getContents().isBlank() && element.getVerticalSpan() == 1) {
				return false;
			}
		}
		return true;
	}
	
	public int getLargestVerticalSpan() {
		int verticalSpan = 1;
		for(GenericCell element : data) {
			verticalSpan = Math.max(verticalSpan, element.getVerticalSpan());
		}
		return verticalSpan;
	}
}
