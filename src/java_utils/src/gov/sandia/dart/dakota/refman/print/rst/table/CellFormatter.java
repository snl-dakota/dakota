package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class CellFormatter {
	
	////////////
	// FIELDS //
	////////////
	
	private final GenericCell cell;
	
	private int wordIndex = 0;
	private String nextWord = "";
	private List<String> thisLineWords = new ArrayList<>();
	private List<String> nextLineWords = new ArrayList<>();
	
	private CellPayload payloadResult;
	
	/////////////////
	// CONSTRUCTOR //
	/////////////////
	
	public CellFormatter(GenericCell cell) {
		this.cell = cell;
		this.payloadResult = new CellPayload("", "");
	}
	
	/////////////
	// GETTERS //
	/////////////
	
	public CellPayload getPayload() {
		return payloadResult;
	}
	
	////////////
	// FORMAT //
	////////////
	
	public void format(int cellWidth) {
		String contents = cell.getContents();
		if(contents.isBlank()) {
			return;
		}
		
		int leftPadding = cell.getLeftPadding();
		int rightPadding = cell.getRightPadding();
		if(cellWidth <= leftPadding + rightPadding) {
			throw new IllegalStateException("Padding excludes contents from being added to cell. Make the prescribed cell width wider.");
		}
		
		String[] words = splitByLineBreaksAndSpaces(contents);
		wordIndex = 0;
		nextWord = words[wordIndex];
		
		insertWordsUntilCellIsFilled(cellWidth, words);
		handleWordsTooLongForCell(cellWidth, words);
		handleRemainderWords(words);
		
		payloadResult = new CellPayload(String.join(" ", thisLineWords), String.join(" ", nextLineWords));
	}
	
	/////////////
	// PRIVATE //
	/////////////
	
	private String[] splitByLineBreaksAndSpaces(String contents) {
		List<String> firstSplits = Arrays.asList(contents.split("\n|\r\n"));
		List<String> finalSplits = new ArrayList<>();
		for(String firstSplit : firstSplits) {
			finalSplits.addAll(Arrays.asList(firstSplit.split(" ")));
		}
		return finalSplits.toArray(new String[finalSplits.size()]);
	}
	
	private void insertWordsUntilCellIsFilled(int cellWidth, String[] words) {
		int leftPadding = cell.getLeftPadding();
		int rightPadding = cell.getRightPadding();
		boolean firstWord = true;
		boolean lastWord = false;
		
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
	
	private void handleWordsTooLongForCell(int cellWidth, String[] words) {
		int leftPadding = cell.getLeftPadding();
		int rightPadding = cell.getRightPadding();
		if(words.length == 1 &&
				!nextWord.startsWith(CellUtil.pad(leftPadding)) &&
				!nextWord.endsWith(CellUtil.pad(rightPadding))) {
			nextWord = CellUtil.pad(leftPadding) + nextWord + CellUtil.pad(rightPadding);
		}
		
		if(nextWord.length() > cellWidth) {
			splitWord(nextWord, cellWidth, thisLineWords, nextLineWords, rightPadding);
			wordIndex++;
		}
	}
	
	private void handleRemainderWords(String[] words) {
		for(int remainder = wordIndex; remainder < words.length; remainder++) {
			nextLineWords.add(words[remainder]);
		}
	}
	
	private void splitWord(String nextWord, int width, List<String> thisLineWords, List<String> nextLineWords, int rightPadding) {
		int paddedWidth = width - rightPadding;
		String thisLine = nextWord.substring(0, paddedWidth);
		String nextLine = nextWord.substring(paddedWidth, nextWord.length());
		
		String proposedString = String.join(" ", thisLineWords) + " " + thisLine;
		if(proposedString.length() > paddedWidth && !thisLineWords.isEmpty()) {
			nextLineWords.add(thisLine);
			nextLineWords.add(nextLine);
		} else {
			thisLineWords.add(thisLine);
			nextLineWords.add(nextLine);
		}
	}
}
