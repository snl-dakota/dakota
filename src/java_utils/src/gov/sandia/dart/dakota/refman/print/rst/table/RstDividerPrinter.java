package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.List;

public class RstDividerPrinter {
	
	private final boolean headerDivider;
	private int dividerPointer = 0;
	
	public RstDividerPrinter(boolean headerDivider) {
		this.headerDivider = headerDivider;
	}
	
	public String print(List<Integer> widths) {
		return print(widths, null, -1);
	}
	
	public String print(List<Integer> widths, GenericTable table, int rowIndex) {
		StringBuilder sb = new StringBuilder();
		
		String leftPortion = "+";
		
		sb.append(leftPortion);
		for(dividerPointer = 0; dividerPointer < widths.size(); dividerPointer++) {
			Integer width = widths.get(dividerPointer);
			boolean lastEntry = dividerPointer == widths.size() - 1;
			
			sb.append(appendCenterPortion(widths, width, rowIndex, dividerPointer, null));
			sb.append(appendRightPortion(table, rowIndex, lastEntry));
		}
		
		dividerPointer = 0;
		return sb.toString();
	}
	
	public String printWithOverflow(List<Integer> widths, GenericTable table, int rowIndex, GenericRow verticalSpanOverflow) {
		StringBuilder sb = new StringBuilder();
		
		String leftPortion = (segmentIsBlankOverflowFromPreviousRow(verticalSpanOverflow) ? "|" : "+");
		
		sb.append(leftPortion);
		for(dividerPointer = 0; dividerPointer < widths.size(); dividerPointer++) {
			Integer width = widths.get(dividerPointer);
			boolean lastEntry = dividerPointer == widths.size() - 1;
			
			sb.append(appendCenterPortion(widths, width, rowIndex, dividerPointer, verticalSpanOverflow));
			sb.append(appendRightPortionIfOverflow(table, rowIndex, verticalSpanOverflow, lastEntry));
		}
		
		dividerPointer = 0;
		return sb.toString();
	}
	
	private String appendCenterPortion(
			List<Integer> widths, Integer width, int rowIndex, int dividerPointer, GenericRow verticalSpanOverflow) {
		
		String centerPortion = "";
		for(int j = 0; j < width; j++) {
			if(segmentIsBlankOverflowFromPreviousRow(verticalSpanOverflow)) {
				centerPortion += " ";
			} else if(headerDivider) {
				centerPortion += "=";
			} else {
				centerPortion += "-";
			}
		}
		
		String centerOverflowPortion = appendCenterPortionIfOverflow(widths, verticalSpanOverflow);
		if(!centerOverflowPortion.isBlank()) {
			centerPortion = centerOverflowPortion;
		}
		
		return centerPortion;
	}
	
	private boolean segmentIsInterruptedOnNextRow(GenericTable table, int rowIndex) {
		int nextRowIndex = rowIndex + 1;
		if(table != null && nextRowIndex < table.getRows().size()) {
			GenericRow thisRow = table.getRows().get(rowIndex);
			GenericRow nextRow = table.getRows().get(nextRowIndex);
			
			int thisAdjustedStartCellIndex = thisRow.getAdjustedHorizontalSpanIndex(dividerPointer);
			GenericCell thisCell = nextRow.getData().get(thisAdjustedStartCellIndex);
			
			int nextAdjustedStartCellIndex = nextRow.getAdjustedHorizontalSpanIndex(dividerPointer);
			GenericCell nextCell = nextRow.getData().get(nextAdjustedStartCellIndex);
			
			int thisRowCellSpan = thisCell.getHorizontalSpan();
			int nextRowCellSpan = nextCell.getHorizontalSpan();
			return thisRowCellSpan == nextRowCellSpan;
		}
		return false;
	}
	
	private String appendCenterPortionIfOverflow(List<Integer> widths, GenericRow verticalSpanOverflow) {
		if(verticalSpanOverflow != null && dividerPointer < verticalSpanOverflow.getData().size()) {
			GenericCell overflowCell = verticalSpanOverflow.getData().get(dividerPointer);
			int width = overflowCell.getCellWidth(widths, dividerPointer);
			String result =
				getOverflowLineAndUpdateVerticalSpanOverflow(verticalSpanOverflow, width);
			if(result.length() > 0) {
				dividerPointer += (overflowCell.getHorizontalSpan() - 1);
				return result;
			}
		}
		return "";
	}
	
	private String appendRightPortion(GenericTable table, int rowIndex, boolean lastEntry) {
		String rightPortion = "+";
		if(rowIndex != -1) {
			GenericRow previousRow = table.getRows().get(rowIndex);
			GenericRow nextRow = null;
			if(rowIndex < table.getRows().size() - 1) {
				nextRow = table.getRows().get(rowIndex + 1);
			}
			if(!lastEntry) {
				if(cellWasOvertakenByVerticalSpan(table, rowIndex) ||
				  (isPointerInTheMiddleofSpanningCell(previousRow) &&
				  (isPointerInTheMiddleofSpanningCell(nextRow) || nextRow == null))) {
					rightPortion = "-";
				} else {
					rightPortion = "+";
				}
			}
		}
		return rightPortion;
	}
	
	private boolean cellWasOvertakenByVerticalSpan(GenericTable table, int rowIndex) {
		int index = rowIndex;
		int steps = 0;
		if(table != null) {
			while(index >= 0) {
				GenericRow thisRow = table.getRows().get(index);
				int thisAdjustedStartCellIndex = thisRow.getAdjustedHorizontalSpanIndex(dividerPointer);
				GenericCell thisCell = thisRow.getData().get(thisAdjustedStartCellIndex);
				if(thisCell.getHorizontalSpan() > 1 && thisCell.getVerticalSpan() > 1 && thisCell.getVerticalSpan() >= steps) {
					return true;
				}
				index--;
				steps++;
			}
		}
		return false;
	}
	
	private String appendRightPortionIfOverflow(GenericTable table, int rowIndex, GenericRow verticalSpanOverflow, boolean lastEntry) {
		String rightPortion = "+";
		if(!lastEntry) {
			boolean blankOnBothSides =
				segmentIsBlankOverflowFromPreviousRow(verticalSpanOverflow) &&
			    segmentIsOverflowing(verticalSpanOverflow);
			boolean spanningCell =
				isPointerInTheMiddleofSpanningCell(verticalSpanOverflow) &&
				!segmentIsInterruptedOnNextRow(table, rowIndex);
			
			if(spanningCell) {
				rightPortion = "-";
			} else if(blankOnBothSides) {
				rightPortion = "|"; 
			}
		} else if(segmentIsBlankOverflowFromPreviousRow(verticalSpanOverflow)) {
			rightPortion = "|";
		}
		
		return rightPortion;
	}
	
	private boolean isPointerInTheMiddleofSpanningCell(GenericRow row) {
		if(row != null) {
			int adjustedIndex     = row.getAdjustedHorizontalSpanIndex(dividerPointer);
			int adjustedNextIndex = row.getAdjustedHorizontalSpanIndex(dividerPointer + 1);
			GenericCell cell = row.getData().get(adjustedIndex);
			int span = cell.getHorizontalSpan();
			if(span > 1 && adjustedIndex == adjustedNextIndex) { // Inside a spanning cell
				return true;
			}
		}
		return false;
	}
	
	private boolean segmentIsBlankOverflowFromPreviousRow(GenericRow verticalSpanOverflow) {
		if(verticalSpanOverflow != null && dividerPointer >= 0 && dividerPointer < verticalSpanOverflow.getData().size()) {
			GenericCell overflowCell = verticalSpanOverflow.getData().get(dividerPointer);
			return overflowCell.getContents().isBlank() && overflowCell.getVerticalSpan() > 1;
		}
		return false;
	}
	
	private boolean segmentIsOverflowing(GenericRow verticalSpanOverflow) {
		int nextIndex = dividerPointer + 1;
		if(verticalSpanOverflow != null && nextIndex >= 0 && nextIndex < verticalSpanOverflow.getData().size()) {
			GenericCell overflowCell = verticalSpanOverflow.getData().get(nextIndex);
			return overflowCell.getVerticalSpan() > 1;
		}
		return false;
	}
	
	private String getOverflowLineAndUpdateVerticalSpanOverflow(GenericRow rowOverflow, int width) {
		GenericCell overflowCell = rowOverflow.getData().get(dividerPointer);
		if(!overflowCell.getContents().isBlank()) {
			CellPayload result = overflowCell.getCellFormattedContents(width);
			updateVerticalSpanOverflow(rowOverflow, dividerPointer, result.getRemainderToPrint());
			return padToWidth(result.getThisRowPrint(), width);
		}
		return "";
	}
	
	private String padToWidth(String cellAvailable, int width) {
		String finalString = cellAvailable;
		int remainingPadding = width - cellAvailable.length();
		finalString = finalString + CellUtil.pad(remainingPadding);
		return finalString;
	}
	
	private void updateVerticalSpanOverflow(GenericRow rowOverflow, int index, String cellRemainder) {
		rowOverflow.getData().get(index).setContents(cellRemainder);
	}
}
