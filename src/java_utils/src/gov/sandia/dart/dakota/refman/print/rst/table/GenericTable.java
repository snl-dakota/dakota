package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.ArrayList;
import java.util.List;

public class GenericTable {

	private List<GenericRow> rows = new ArrayList<>();
	
	private List<Integer> definedColumnWidths = new ArrayList<>();
	private int cellLeftPadding = 1;
	private int cellRightPadding = 1;
	
	public GenericRow getHeaderRow() {
		if(!rows.isEmpty()) {
			return rows.get(0);
		} else {
			return null;
		}
	}
	
	public List<GenericRow> getRows() {
		return rows;
	}
	
	public int getLeftPadding() {
		return cellLeftPadding;
	}
	
	public int getRightPadding() {
		return cellRightPadding;
	}
	
	public int getColumnWidth(int column) {
		if(!definedColumnWidths.isEmpty() && column < definedColumnWidths.size()) {
			Integer value = definedColumnWidths.get(column);
			if(value != null) {
				return value;
			}
		}
		
		int widestWidthSoFar = 0;
		for(GenericRow row : rows) {
			if(column < row.getData().size()) {
				int adjustedIndex = CellUtil.getAdjustedHorizontalSpanIndex(row, column);
				GenericCell cell = row.getData().get(adjustedIndex);
				// Is it a regular cell we can measure for determining width?
				if(cell.getHorizontalSpan() == 1 && cell.getVerticalSpan() == 1) { 
					int newWidth = cellLeftPadding + cell.getContents().length() + cellRightPadding;
					if(newWidth > widestWidthSoFar) {
						widestWidthSoFar = newWidth;
					}
				}
			}
		}
		return widestWidthSoFar;
	}
	
	public List<Integer> getColumnWidths() {
		List<Integer> values = new ArrayList<>();
		GenericRow headerRow = rows.get(0);
		for(int i = 0; i < headerRow.getData().size(); i++) {
			values.add(getColumnWidth(i));
		}
		return values;
	}
	
	public void setColumnWidth(int column, int width) {
		while(column >= definedColumnWidths.size()) {
			definedColumnWidths.add(-1);
		}
		definedColumnWidths.set(column, width);
	}
	
	public void addRow(GenericRow row) {
		for(GenericRow oldRow : rows) {
			oldRow.setLastRow(false);
		}
		row.setLastRow(true);
		rows.add(row);
	}
}
