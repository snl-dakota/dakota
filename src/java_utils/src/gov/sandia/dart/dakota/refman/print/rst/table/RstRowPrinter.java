package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.ArrayList;
import java.util.List;

public class RstRowPrinter {
	
	private final int leftPadding;
	private final int rightPadding;
	
	public RstRowPrinter(int leftPadding, int rightPadding) {
		this.leftPadding = leftPadding;
		this.rightPadding = rightPadding;
	}
	

	public String printRow(List<GenericCell> cells, List<Integer> widths) {
		StringBuilder sb = new StringBuilder();
		List<GenericCell> rowOverflow = new ArrayList<>();
		for(int i = 0; i < cells.size(); i++) {
			GenericCell cell = cells.get(i);
			rowOverflow.add(new GenericCell("", cell.getHorizontalSpan(), cell.getVerticalSpan()));
		}
		
		sb.append("|");
		for(int i = 0; i < cells.size(); i++) {
			String cell = cells.get(i).getContents();
			int width = getCellWidth(cells.get(i), widths, i);
			
			CellPayload result = getCellFormattedContents(cell, width);
			String cellAvailable = result.getThisRowPrint();
			String cellRemainder = result.getRemainderToPrint();
			
			sb.append(cellAvailable);
			if(!cellRemainder.isBlank()) {
				rowOverflow.get(i).setContents(cellRemainder);
			}
			int remainingPadding = width - cellAvailable.length();
			sb.append(pad(remainingPadding));
			
			sb.append("|");
		}
		
		if(!elementsAreBlank(rowOverflow)) {
			sb.append(RstTablePrinter.NEWLINE);
			sb.append(printRow(rowOverflow, widths));
		}
		return sb.toString();
	}
	
	private int getCellWidth(GenericCell cell, List<Integer> widths, int nativeWidthIndex) {
		if(cell.getHorizontalSpan() == 1) {
			return widths.get(nativeWidthIndex);
		} else {
			int totalWidth = 0;
			int span = cell.getHorizontalSpan();
			for(int i = nativeWidthIndex; i < nativeWidthIndex + span; i++) {
				if(i < widths.size()) {
					totalWidth += widths.get(i);
				}
			}
			for(int i = nativeWidthIndex; i < nativeWidthIndex + span - 1; i++) {
				if(i < widths.size()) {
					totalWidth ++; // Count dividers between cells.
				}
			}
			return totalWidth;
		}
	}

	private CellPayload getCellFormattedContents(String cellContents, int cellWidth) {
		if(cellContents.isBlank()) {
			return new CellPayload("", "");
		}
		
		String[] words = cellContents.split(" ");
		
		if(cellWidth <= leftPadding + rightPadding) {
			throw new IllegalStateException("Padding excludes contents from being added to cell. Make the prescribed cell width wider.");
		}
		
		List<String> thisLineWords = new ArrayList<>();
		List<String> nextLineWords = new ArrayList<>();
		
		int wordIndex = 0;
		String nextWord = words[wordIndex];
		boolean firstWord = true;
		boolean lastWord = words.length == 1;
		
		while(nextWordFits(thisLineWords, nextWord, cellWidth, lastWord) && wordIndex < words.length) {
			if(wordIndex < words.length) {
				
				nextWord = words[wordIndex];
				firstWord = wordIndex == 0;
				if(firstWord) nextWord = pad(leftPadding) + nextWord;
				
				lastWord = (wordIndex == words.length - 1);
				if(lastWord) nextWord = nextWord + pad(rightPadding);
				
				thisLineWords.add(nextWord);
				
				wordIndex++;
				if(wordIndex < words.length) {
					nextWord = words[wordIndex];
				}
			}
		}
		
		if(words.length == 1 && !nextWord.startsWith(pad(leftPadding)) && !nextWord.startsWith(pad(rightPadding))) {
			nextWord = pad(leftPadding) + nextWord + pad(rightPadding);
		}
		
		if(nextWord.length() > cellWidth) {
			splitWord(nextWord, cellWidth, thisLineWords, nextLineWords);
			wordIndex++;
		}
		for(int remainder = wordIndex; remainder < words.length; remainder++) {
			nextLineWords.add(words[remainder]);
		}
		
		return new CellPayload(String.join(" ", thisLineWords), String.join(" ", nextLineWords));
	}
	
	private boolean nextWordFits(List<String> words, String nextWord, int cellWidth, boolean lastWord) {
		String proposedString = "";
		if(words.isEmpty()) {
			proposedString = nextWord;
		} else {
			proposedString = String.join(" ", words) + " " + nextWord;
		}
		return proposedString.length() <= (cellWidth - rightPadding);
	}
	
	private void splitWord(String nextWord, int width, List<String> thisLineWords, List<String> nextLineWords) {
		int paddedWidth = width - rightPadding;
		String thisLine = nextWord.substring(0, paddedWidth);
		String nextLine = nextWord.substring(paddedWidth, nextWord.length());
		thisLineWords.add(thisLine);
		nextLineWords.add(nextLine);
	}
	
	private boolean elementsAreBlank(List<GenericCell> elements) {
		for(GenericCell element : elements) {
			if(!element.getContents().isBlank()) {
				return false;
			}
		}
		return true;
	}
	
	private String pad(int spaces) {
		StringBuilder sb = new StringBuilder();
		for(int i = 0; i < spaces; i++) {
			sb.append(" ");
		}
		return sb.toString();
	}
	
	//////
	
	class CellPayload {
		private String thisRowPrint;
		private String remainderToPrint;
		
		public CellPayload(String thisRowPrint, String remainderToPrint) {
			this.thisRowPrint = thisRowPrint;
			this.remainderToPrint = remainderToPrint;
		}
		
		public String getThisRowPrint() {
			return thisRowPrint;
		}
		
		public String getRemainderToPrint() {
			return remainderToPrint;
		}
	}
}
