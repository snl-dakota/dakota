package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.List;

public class RstTablePrinter {
	
	public static final String NEWLINE = "\n";

	public String print(GenericTable table) {
		
		List<Integer> widths = table.getColumnWidths();
		RstRowPrinter rowPrinter = new RstRowPrinter(table.getLeftPadding(), table.getRightPadding());
		
		StringBuilder sb = new StringBuilder();
		sb.append(printDivider(widths, false)).append(NEWLINE);
		GenericRow headerRow = table.getHeaderRow();
		if(headerRow != null) {
			sb.append(rowPrinter.printRow(headerRow.getDataStrings(), widths)).append(NEWLINE);
		}
		sb.append(printDivider(widths, true)).append(NEWLINE);
		
		for(int i = 1; i < table.getRows().size(); i++) {
			GenericRow row = table.getRows().get(i);
			sb.append(rowPrinter.printRow(row.getDataStrings(), widths)).append(NEWLINE);
			sb.append(printDivider(widths, false)).append(NEWLINE);
		}
		
		return sb.toString();
	}
	
	private String printDivider(List<Integer> widths, boolean headerDivider) {
		StringBuilder sb = new StringBuilder();
		sb.append("+");
		for(int i = 0; i < widths.size(); i++) {
			Integer width = widths.get(i);
			for(int j = 0; j < width; j++) {
				sb.append(headerDivider ? "=" : "-");
			}
			sb.append("+");
		}
		return sb.toString();
	}
}
