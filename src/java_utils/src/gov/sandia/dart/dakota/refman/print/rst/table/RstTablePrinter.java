package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.List;

public class RstTablePrinter {
	
	public static final String NEWLINE = "\n";
	private GenericRow verticalSpanOverflow = null;
	private int verticalSpanLocationPointer = 0; // To keep track of how far we are into a vertical span cell.

	public String print(GenericTable table) {
		
		List<Integer> widths = table.getColumnWidths();
		
		RstDividerPrinter headerDividerPrinter = new RstDividerPrinter(true);
		RstDividerPrinter dividerPrinter = new RstDividerPrinter(false);
		RstRowPrinter rowPrinter = new RstRowPrinter();
		
		StringBuilder sb = new StringBuilder();
		
		sb.append(dividerPrinter.print(widths)).append(NEWLINE);
		
		GenericRow headerRow = table.getHeaderRow();
		if(headerRow != null) {
			sb.append(rowPrinter.print(headerRow, widths)).append(NEWLINE);
		}
		
		sb.append(headerDividerPrinter.print(widths)).append(NEWLINE);
		
		for(int i = 1; i < table.getRows().size(); i++) {
			GenericRow row = table.getRows().get(i);
			sb.append(rowPrinter.print(row, verticalSpanOverflow, widths)).append(NEWLINE);
			
			updateVerticalSpan(rowPrinter);
			if(verticalSpanOverflow != null) {
				sb.append(dividerPrinter.printWithOverflow(widths, table, i, verticalSpanOverflow)).append(NEWLINE);
			} else {
				sb.append(dividerPrinter.print(widths, table, i)).append(NEWLINE);
			}
		}
		
		return sb.toString();
	}
	
	private void updateVerticalSpan(RstRowPrinter previousRowPrinter) {
		GenericRow previousVerticalSpanOverflow = verticalSpanOverflow;
		verticalSpanOverflow = previousRowPrinter.getVerticalSpanOverflow();
		if(previousVerticalSpanOverflow != verticalSpanOverflow) {
			verticalSpanLocationPointer = 0;
		}
		
		if(verticalSpanOverflow != null) {
			verticalSpanLocationPointer ++;
			if(verticalSpanOverflow.elementsAreBlank() &&
					verticalSpanLocationPointer == verticalSpanOverflow.getLargestVerticalSpan()) {
				verticalSpanOverflow = null;
				verticalSpanLocationPointer = 0;
			}
		}
	}
}
