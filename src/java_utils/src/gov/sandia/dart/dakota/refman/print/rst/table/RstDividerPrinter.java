package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.List;

public class RstDividerPrinter {

	public String print(List<Integer> widths, GenericRow verticalSpanOverflow, boolean headerDivider) {
		StringBuilder sb = new StringBuilder();
		
		String leftPortion = (segmentIsBlankOverflowFromPreviousRow(verticalSpanOverflow, 0) ? "|" : "+");
		String centerPortion = "";
		String rightPortion = "";
		
		sb.append(leftPortion);
		for(int i = 0; i < widths.size(); i++) {
			Integer width = widths.get(i);
			centerPortion = "";
			for(int j = 0; j < width; j++) {
				if(segmentIsBlankOverflowFromPreviousRow(verticalSpanOverflow, i)) {
					centerPortion += " ";
				} else {
					centerPortion += (headerDivider ? "=" : "-");
				}
			}
			
			if(verticalSpanOverflow != null && i < verticalSpanOverflow.getData().size()) {
				GenericCell overflowCell = verticalSpanOverflow.getData().get(i);
				width = CellUtil.getCellWidth(overflowCell, widths, i);
				String result =
					getOverflowLineAndUpdateVerticalSpanOverflow(verticalSpanOverflow, width, i);
				if(result.length() > 0) {
					centerPortion = result;
					i += (overflowCell.getHorizontalSpan() - 1);
				}
			}
			
			sb.append(centerPortion);
			
			if(i < widths.size() - 1) {
				boolean blankOnBothSides =
					segmentIsBlankOverflowFromPreviousRow(verticalSpanOverflow, i) &&
				    segmentIsOverflowing(verticalSpanOverflow, i+1);
				rightPortion = (blankOnBothSides ? "|" : "+");
			} else {
				rightPortion = (segmentIsBlankOverflowFromPreviousRow(verticalSpanOverflow, i) ? "|" : "+");
			}
			sb.append(rightPortion);
		}
		
		return sb.toString();
	}
	
	private boolean segmentIsBlankOverflowFromPreviousRow(GenericRow verticalSpanOverflow, int index) {
		if(verticalSpanOverflow != null && index >= 0 && index < verticalSpanOverflow.getData().size()) {
			GenericCell overflowCell = verticalSpanOverflow.getData().get(index);
			return overflowCell.getContents().isBlank() && overflowCell.getVerticalSpan() > 1;
		}
		return false;
	}
	
	private boolean segmentIsOverflowing(GenericRow verticalSpanOverflow, int index) {
		if(verticalSpanOverflow != null && index >= 0 && index < verticalSpanOverflow.getData().size()) {
			GenericCell overflowCell = verticalSpanOverflow.getData().get(index);
			return overflowCell.getVerticalSpan() > 1;
		}
		return false;
	}
	
	private String getOverflowLineAndUpdateVerticalSpanOverflow(GenericRow rowOverflow, int width, int index) {
		GenericCell overflowCell = rowOverflow.getData().get(index);
		if(!overflowCell.getContents().isBlank()) {
			CellPayload result = CellUtil.getCellFormattedContents(overflowCell, width);
			String cellAvailable = result.getThisRowPrint();
			String cellRemainder = result.getRemainderToPrint();
			
			rowOverflow.getData().get(index).setContents(cellRemainder);
			
			String finalString = cellAvailable;
			int remainingPadding = width - cellAvailable.length();
			finalString = finalString + CellUtil.pad(remainingPadding);
			return finalString;
		}
		return "";
	}
}
