package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.ArrayList;
import java.util.List;

public class GenericCell {

	private String contents;
	private int horizontalSpan;
	private int verticalSpan;
	private final int leftPadding;
	private final int rightPadding;
	
	public GenericCell(String contents) {
		this(contents, 1, 1);
	}
	
	public GenericCell(String contents, int horizontalSpan, int verticalSpan) {
		this(contents, horizontalSpan, verticalSpan, 1, 1);
	}
	
	public GenericCell(String contents, int horizontalSpan, int verticalSpan, int leftPadding, int rightPadding) {
		this.contents = contents;
		this.horizontalSpan = horizontalSpan;
		this.verticalSpan = verticalSpan;
		this.leftPadding = leftPadding;
		this.rightPadding = rightPadding;
	}
	
	public String getContents() {
		return contents;
	}
	
	public void setContents(String contents) {
		this.contents = contents;
	}
	
	public void setHorizontalSpan(int horizontalSpan) {
		this.horizontalSpan = horizontalSpan;
	}
	
	public void setVerticalSpan(int verticalSpan) {
		this.verticalSpan = verticalSpan;
	}
	
	public int getHorizontalSpan() {
		return horizontalSpan;
	}
	
	public int getVerticalSpan() {
		return verticalSpan;
	}

	public int getLeftPadding() {
		return leftPadding;
	}
	
	public int getRightPadding() {
		return rightPadding;
	}
	
	public int getCellWidth(List<Integer> widths, int nativeWidthIndex) {
		if(horizontalSpan == 1) { // Includes vertical spanning logic
			int nativeWidth = widths.get(nativeWidthIndex);
			if(nativeWidth == -1) {
				throw new IllegalStateException("For columns with vertical spans, width must be explicitly defined.");
			}
			return widths.get(nativeWidthIndex);
		} else { // Horizontal spanning logic
			int totalWidth = 0;
			int span = horizontalSpan;
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
	
	public CellPayload getCellFormattedContents(int cellWidth) {
		if(contents.isBlank()) {
			return new CellPayload("", "");
		}
		
		String[] words = contents.split(" ");
		
		if(cellWidth <= leftPadding + rightPadding) {
			throw new IllegalStateException("Padding excludes contents from being added to cell. Make the prescribed cell width wider.");
		}
		
		List<String> thisLineWords = new ArrayList<>();
		List<String> nextLineWords = new ArrayList<>();
		
		int wordIndex = 0;
		String nextWord = words[wordIndex];
		boolean firstWord = true;
		boolean lastWord = words.length == 1;
		
		while(nextWordFits(thisLineWords, nextWord, cellWidth, rightPadding) && wordIndex < words.length) {
			if(wordIndex < words.length) {
				
				nextWord = words[wordIndex];
				firstWord = wordIndex == 0;
				if(firstWord) nextWord = CellUtil.pad(leftPadding) + nextWord;
				
				lastWord = (wordIndex == words.length - 1);
				if(lastWord) nextWord = nextWord + CellUtil.pad(rightPadding);
				
				thisLineWords.add(nextWord);
				
				wordIndex++;
				if(wordIndex < words.length) {
					nextWord = words[wordIndex];
				}
			}
		}
		
		if(words.length == 1 && !nextWord.startsWith(CellUtil.pad(leftPadding)) && !nextWord.startsWith(CellUtil.pad(rightPadding))) {
			nextWord = CellUtil.pad(leftPadding) + nextWord + CellUtil.pad(rightPadding);
		}
		
		if(nextWord.length() > cellWidth) {
			splitWord(nextWord, cellWidth, thisLineWords, nextLineWords, rightPadding);
			wordIndex++;
		}
		for(int remainder = wordIndex; remainder < words.length; remainder++) {
			nextLineWords.add(words[remainder]);
		}
		
		return new CellPayload(String.join(" ", thisLineWords), String.join(" ", nextLineWords));
	}
	
	private boolean nextWordFits(List<String> words, String nextWord, int cellWidth, int rightPadding) {
		String proposedString = "";
		if(words.isEmpty()) {
			proposedString = nextWord;
		} else {
			proposedString = String.join(" ", words) + " " + nextWord;
		}
		return proposedString.length() <= (cellWidth - rightPadding);
	}
	
	private void splitWord(String nextWord, int width, List<String> thisLineWords, List<String> nextLineWords, int rightPadding) {
		int paddedWidth = width - rightPadding;
		String thisLine = nextWord.substring(0, paddedWidth);
		String nextLine = nextWord.substring(paddedWidth, nextWord.length());
		thisLineWords.add(thisLine);
		nextLineWords.add(nextLine);
	}
	
	@Override
	public String toString() {
		return contents;
	}
}
