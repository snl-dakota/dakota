package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.List;

public class RstDividerPrinter {

	public String print(List<Integer> widths, GenericRow verticalSpanOverflow, boolean headerDivider) {
		StringBuilder sb = new StringBuilder();
		sb.append("+");
		for(int i = 0; i < widths.size(); i++) {
			Integer width = widths.get(i);
			StringBuilder dividerCellLine = new StringBuilder();
			for(int j = 0; j < width; j++) {
				dividerCellLine.append(headerDivider ? "=" : "-");
			}
			
			if(verticalSpanOverflow != null && i < verticalSpanOverflow.getData().size()) {
				GenericCell overflowCell = verticalSpanOverflow.getData().get(i);
				width = CellUtil.getCellWidth(overflowCell, widths, i);
				String result =
					getOverflowLineAndUpdateVerticalSpanOverflow(verticalSpanOverflow, width, i);
				if(result.length() > 0) {
					dividerCellLine = new StringBuilder(result);
					i += (overflowCell.getHorizontalSpan() - 1);
				}
			}
			
			sb.append(dividerCellLine.toString());
			sb.append("+");
		}
		return sb.toString();
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
