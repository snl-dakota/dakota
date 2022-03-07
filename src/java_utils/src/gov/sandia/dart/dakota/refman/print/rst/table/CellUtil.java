package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.ArrayList;
import java.util.List;

public class CellUtil {
	
	public static String pad(int spaces) {
		StringBuilder sb = new StringBuilder();
		for(int i = 0; i < spaces; i++) {
			sb.append(" ");
		}
		return sb.toString();
	}
	
	public static int getCellWidth(GenericCell cell, List<Integer> widths, int nativeWidthIndex) {
		if(cell.getHorizontalSpan() == 1) { // Includes vertical spanning logic
			int nativeWidth = widths.get(nativeWidthIndex);
			if(nativeWidth == -1) {
				throw new IllegalStateException("For columns with vertical spans, width must be explicitly defined.");
			}
			return widths.get(nativeWidthIndex);
		} else { // Horizontal spanning logic
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
	
	public static CellPayload getCellFormattedContents(GenericCell cell, int cellWidth) {
		if(cell.getContents().isBlank()) {
			return new CellPayload("", "");
		}
		
		String[] words = cell.getContents().split(" ");
		
		if(cellWidth <= cell.getLeftPadding() + cell.getRightPadding()) {
			throw new IllegalStateException("Padding excludes contents from being added to cell. Make the prescribed cell width wider.");
		}
		
		List<String> thisLineWords = new ArrayList<>();
		List<String> nextLineWords = new ArrayList<>();
		
		int wordIndex = 0;
		String nextWord = words[wordIndex];
		boolean firstWord = true;
		boolean lastWord = words.length == 1;
		
		while(nextWordFits(thisLineWords, nextWord, cellWidth, cell.getRightPadding()) && wordIndex < words.length) {
			if(wordIndex < words.length) {
				
				nextWord = words[wordIndex];
				firstWord = wordIndex == 0;
				if(firstWord) nextWord = CellUtil.pad(cell.getLeftPadding()) + nextWord;
				
				lastWord = (wordIndex == words.length - 1);
				if(lastWord) nextWord = nextWord + CellUtil.pad(cell.getRightPadding());
				
				thisLineWords.add(nextWord);
				
				wordIndex++;
				if(wordIndex < words.length) {
					nextWord = words[wordIndex];
				}
			}
		}
		
		if(words.length == 1 && !nextWord.startsWith(CellUtil.pad(cell.getLeftPadding())) && !nextWord.startsWith(CellUtil.pad(cell.getRightPadding()))) {
			nextWord = CellUtil.pad(cell.getLeftPadding()) + nextWord + CellUtil.pad(cell.getRightPadding());
		}
		
		if(nextWord.length() > cellWidth) {
			splitWord(nextWord, cellWidth, thisLineWords, nextLineWords, cell.getRightPadding());
			wordIndex++;
		}
		for(int remainder = wordIndex; remainder < words.length; remainder++) {
			nextLineWords.add(words[remainder]);
		}
		
		return new CellPayload(String.join(" ", thisLineWords), String.join(" ", nextLineWords));
	}
	
	public static int getAdjustedHorizontalSpanIndex(GenericRow row, int columnIndex) {
		int actualIndex = columnIndex;
		for(int i = 0; i < row.getData().size(); i++) {
			if(i < actualIndex) {
				GenericCell cell = row.getData().get(0);
				if(cell.getHorizontalSpan() > 1) {
					actualIndex -= (cell.getHorizontalSpan() - 1);
				}
			}
		}
		return actualIndex;
	}
	
	private static boolean nextWordFits(List<String> words, String nextWord, int cellWidth, int rightPadding) {
		String proposedString = "";
		if(words.isEmpty()) {
			proposedString = nextWord;
		} else {
			proposedString = String.join(" ", words) + " " + nextWord;
		}
		return proposedString.length() <= (cellWidth - rightPadding);
	}
	
	private static void splitWord(String nextWord, int width, List<String> thisLineWords, List<String> nextLineWords, int rightPadding) {
		int paddedWidth = width - rightPadding;
		String thisLine = nextWord.substring(0, paddedWidth);
		String nextLine = nextWord.substring(paddedWidth, nextWord.length());
		thisLineWords.add(thisLine);
		nextLineWords.add(nextLine);
	}
}
