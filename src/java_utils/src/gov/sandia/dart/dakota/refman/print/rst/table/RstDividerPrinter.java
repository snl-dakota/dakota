package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.List;

public class RstDividerPrinter {
	
	private final boolean headerDivider;
	private int dividerPointer = 0;
	
	public RstDividerPrinter(boolean headerDivider) {
		this.headerDivider = headerDivider;
	}
	
	public String print(List<Integer> widths, GenericRow verticalSpanOverflow) {
		StringBuilder sb = new StringBuilder();
		
		String leftPortion = (segmentIsBlankOverflowFromPreviousRow(verticalSpanOverflow) ? "|" : "+");
		
		sb.append(leftPortion);
		for(dividerPointer = 0; dividerPointer < widths.size(); dividerPointer++) {
			Integer width = widths.get(dividerPointer);
			boolean lastEntry = dividerPointer == widths.size() - 1;
			
			sb.append(appendCenterPortion(widths, width, verticalSpanOverflow));
			sb.append(appendRightPortion(verticalSpanOverflow, lastEntry));
		}
		
		dividerPointer = 0;
		return sb.toString();
	}
	
	private String appendCenterPortion(List<Integer> widths, Integer width, GenericRow verticalSpanOverflow) {
		String centerPortion = "";
		for(int j = 0; j < width; j++) {
			if(segmentIsBlankOverflowFromPreviousRow(verticalSpanOverflow)) {
				centerPortion += " ";
			} else {
				centerPortion += (headerDivider ? "=" : "-");
			}
		}
		
		String centerOverflowPortion = appendCenterPortionIfOverflow(widths, verticalSpanOverflow);
		if(!centerOverflowPortion.isBlank()) {
			centerPortion = centerOverflowPortion;
		}
		
		return centerPortion;
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
	
	private String appendRightPortion(GenericRow verticalSpanOverflow, boolean lastEntry) {
		String rightPortion = "";
		if(!lastEntry) {
			boolean blankOnBothSides =
				segmentIsBlankOverflowFromPreviousRow(verticalSpanOverflow) &&
			    segmentIsOverflowing(verticalSpanOverflow);
			rightPortion = (blankOnBothSides ? "|" : "+");
		} else {
			rightPortion = (segmentIsBlankOverflowFromPreviousRow(verticalSpanOverflow) ? "|" : "+");
		}
		return rightPortion;
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
