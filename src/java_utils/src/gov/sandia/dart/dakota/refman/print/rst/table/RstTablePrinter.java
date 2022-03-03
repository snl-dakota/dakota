package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.List;

public class RstTablePrinter {
	
	public static final String NEWLINE = "\n";

	public String print(GenericTable table) {
		
		List<Integer> widths = table.getColumnWidths();
		
		RstDividerPrinter dividerPrinter = new RstDividerPrinter();
		RstRowPrinter rowPrinter = new RstRowPrinter();
		
		StringBuilder sb = new StringBuilder();
		
		sb.append(dividerPrinter.print(widths, null, false)).append(NEWLINE);
		
		GenericRow headerRow = table.getHeaderRow();
		if(headerRow != null) {
			sb.append(rowPrinter.print(headerRow, widths)).append(NEWLINE);
		}
		
		sb.append(dividerPrinter.print(widths, null, true)).append(NEWLINE);
		
		GenericRow verticalSpanOverflow = null;
		int verticalSpanLocationPointer = 0; // To keep track of how far we are into a vertical span cell.
		
		for(int i = 1; i < table.getRows().size(); i++) {
			GenericRow row = table.getRows().get(i);
			sb.append(rowPrinter.print(row, verticalSpanOverflow, widths)).append(NEWLINE);
			
			GenericRow previousVerticalSpanOverflow = verticalSpanOverflow;
			verticalSpanOverflow = rowPrinter.getVerticalSpanOverflow();
			if(previousVerticalSpanOverflow != verticalSpanOverflow) {
				verticalSpanLocationPointer = 0;
			}
			
			sb.append(dividerPrinter.print(widths, verticalSpanOverflow, false)).append(NEWLINE);
			if(verticalSpanOverflow != null) {
				verticalSpanLocationPointer ++;
				if(verticalSpanOverflow.elementsAreBlank() &&
						verticalSpanLocationPointer == verticalSpanOverflow.getLargestVerticalSpan()) {
					verticalSpanOverflow = null;
				}
			}
		}
		
		return sb.toString();
	}
}
