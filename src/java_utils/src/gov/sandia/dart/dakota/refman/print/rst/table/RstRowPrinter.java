package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.List;

public class RstRowPrinter {
	
	private GenericRow storedVerticalSpanOverflow; // This row's vertical span overflow
	
	public String print(GenericRow row, List<Integer> widths) {
		return print(row, null, widths);
	}

	public String print(GenericRow row, GenericRow receivedVerticalSpanOverflow, List<Integer> widths) {
		StringBuilder sb = new StringBuilder();
		GenericRow rowOverflow = new GenericRow();
		rowOverflow.setLastRow(row.isLastRow());
		List<GenericCell> cells = row.getData();
		for(int i = 0; i < cells.size(); i++) {
			GenericCell cell = cells.get(i);
			int verticalSpan = cell.getVerticalSpan();
			if(receivedVerticalSpanOverflow != null && i < receivedVerticalSpanOverflow.getData().size()) {
				verticalSpan = Math.max(cell.getVerticalSpan(), receivedVerticalSpanOverflow.getData().get(i).getVerticalSpan());
			}
			rowOverflow.addCell(
				new GenericCell("", cell.getHorizontalSpan(), verticalSpan));
		}
		
		sb.append("|");
		int spanOffset = 0;
		for(int i = 0; i < cells.size(); i++) {
			GenericCell originalCell = cells.get(i);
			GenericCell cell = cells.get(i);
			if(cell instanceof SpanHoldCell &&
				receivedVerticalSpanOverflow != null &&
				i < receivedVerticalSpanOverflow.getData().size()) {
				
				cell = receivedVerticalSpanOverflow.getData().get(i);
			} 
			int width = cell.getCellWidth(widths, i + spanOffset);
			
			CellPayload result = cell.getCellFormattedContents(width);
			String cellAvailable = result.getThisRowPrint();
			String cellRemainder = result.getRemainderToPrint();
			
			sb.append(cellAvailable);
			if(!cellRemainder.isBlank()) {
				rowOverflow.getData().get(i).setContents(cellRemainder);
			}
			int remainingPadding = width - cellAvailable.length();
			sb.append(CellUtil.pad(remainingPadding));
			
			sb.append("|");
			
			if(originalCell instanceof SpanHoldCell && cell.getHorizontalSpan() > 1) {
				i += (cell.getHorizontalSpan() - 1);
			}
			spanOffset += (cell.getHorizontalSpan() - 1);
		}
		
		if(!rowOverflow.elementsAreBlank()) {
			if(rowOverflow.onlySpanningCellsShouldContinue() && !rowOverflow.isLastRow()) {
				storedVerticalSpanOverflow = rowOverflow;
				if(receivedVerticalSpanOverflow != null) {
					throw new IllegalStateException("Collision between two overlapping cells with vertical spans > 1");
				}
			} else {
				sb.append(RstTablePrinter.NEWLINE);
				sb.append(print(rowOverflow, widths));
			}
		} else if(!rowOverflow.onlySpanningCellsShouldContinue() || rowOverflow.isLastRow()) {
			storedVerticalSpanOverflow = null;
		} else {
			storedVerticalSpanOverflow = rowOverflow;
		}
		return sb.toString();
	}
	
	public GenericRow getVerticalSpanOverflow() {
		return storedVerticalSpanOverflow;
	}
}
