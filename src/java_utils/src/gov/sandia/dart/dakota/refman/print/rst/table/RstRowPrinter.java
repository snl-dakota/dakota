package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.List;

public class RstRowPrinter {
	
	private GenericRow storedVerticalSpanOverflow; // This row's vertical span overflow
	private int rowPointer = 0;
	private int spanOffset = 0;
	
	public String print(GenericRow row, List<Integer> widths) {
		return print(row, null, widths);
	}

	public String print(GenericRow row, GenericRow receivedRowOverflow, List<Integer> widths) {
		StringBuilder sb = new StringBuilder();
		List<GenericCell> cells = row.getData();
		GenericRow rowOverflow = initNewOverflowRow(row, receivedRowOverflow, cells);
		
		sb.append("|");
		
		for(rowPointer = 0; rowPointer < cells.size(); rowPointer++) {
			GenericCell originalCell = cells.get(rowPointer);
			GenericCell cell = getCell(cells, receivedRowOverflow, rowPointer);
			int width = cell.getCellWidth(widths, rowPointer + spanOffset);
			
			CellPayload result = cell.getCellFormattedContents(width);
			String cellAvailable = result.getThisRowPrint();
			String cellRemainder = result.getRemainderToPrint();
			
			sb.append(cellAvailable);
			if(!cellRemainder.isBlank()) {
				rowOverflow.getData().get(rowPointer).setContents(cellRemainder);
			}
			sb.append(printCellPaddingAndRightBorder(width, cellAvailable));
			updateRowPointerForHorizontalSpan(originalCell, cell);
		}
		
		updateVerticalSpanOverflow(rowOverflow, receivedRowOverflow);
		sb.append(printRowOverflow(rowOverflow, widths));
		
		rowPointer = 0;
		spanOffset = 0;
		return sb.toString();
	}
	
	private GenericRow initNewOverflowRow(GenericRow row, GenericRow receivedRowOverflow, List<GenericCell> cells) {
		GenericRow rowOverflow = new GenericRow();
		rowOverflow.setLastRow(row.isLastRow());
		
		for(int i = 0; i < cells.size(); i++) {
			GenericCell cell = cells.get(i);
			int verticalSpan = cell.getVerticalSpan();
			if(receivedRowOverflow != null && i < receivedRowOverflow.getData().size()) {
				verticalSpan = Math.max(cell.getVerticalSpan(), receivedRowOverflow.getData().get(i).getVerticalSpan());
			}
			rowOverflow.addCell(new GenericCell("", cell.getHorizontalSpan(), verticalSpan));
		}
		
		return rowOverflow;
	}
	
	private GenericCell getCell(List<GenericCell> cells, GenericRow receivedRowOverflow, int rowPointer) {
		GenericCell cell = cells.get(rowPointer);
		if(cell instanceof SpanHoldCell &&
			receivedRowOverflow != null &&
			rowPointer < receivedRowOverflow.getData().size()) {
			
			cell = receivedRowOverflow.getData().get(rowPointer);
		}
		return cell;
	}
	
	private String printCellPaddingAndRightBorder(int width, String cellAvailable) {
		int remainingPadding = width - cellAvailable.length();
		StringBuilder sb = new StringBuilder();
		sb.append(CellUtil.pad(remainingPadding));
		sb.append("|");
		return sb.toString();
	}
	
	private void updateRowPointerForHorizontalSpan(GenericCell oldCell, GenericCell newCell) {
		if(oldCell instanceof SpanHoldCell && newCell.getHorizontalSpan() > 1) {
			rowPointer += (newCell.getHorizontalSpan() - 1);
		}
		spanOffset += (newCell.getHorizontalSpan() - 1);
	}
	
	private void updateVerticalSpanOverflow(GenericRow rowOverflow, GenericRow receivedVerticalSpanOverflow) {
		if(!rowOverflow.elementsAreBlank()) {
			if(rowOverflow.onlySpanningCellsShouldContinue() && !rowOverflow.isLastRow()) {
				storedVerticalSpanOverflow = rowOverflow;
				if(receivedVerticalSpanOverflow != null) {
					throw new IllegalStateException("Collision between two overlapping cells with vertical spans > 1");
				}
			}
		} else if(!rowOverflow.onlySpanningCellsShouldContinue() || rowOverflow.isLastRow()) {
			storedVerticalSpanOverflow = null;
		} else {
			storedVerticalSpanOverflow = rowOverflow;
		}
	}
	
	private String printRowOverflow(GenericRow rowOverflow, List<Integer> widths) {
		boolean proceed = !rowOverflow.elementsAreBlank();
		proceed = proceed && (!rowOverflow.onlySpanningCellsShouldContinue() || rowOverflow.isLastRow());
		if(proceed) {
			StringBuilder sb = new StringBuilder();
			sb.append(RstTablePrinter.NEWLINE);
			sb.append(print(rowOverflow, widths));
			return sb.toString();
		}
		return "";
	}
	
	public GenericRow getVerticalSpanOverflow() {
		return storedVerticalSpanOverflow;
	}
}
