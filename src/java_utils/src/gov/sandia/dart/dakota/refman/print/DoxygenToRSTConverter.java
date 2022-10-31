package gov.sandia.dart.dakota.refman.print;

import java.util.ArrayDeque;
import java.util.Deque;

public class DoxygenToRSTConverter {
	
	private enum ListType {
		NONE,
		ORDERED,
		UNORDERED;
	}
	
	public static String convert(String original) {
		String converted = original.trim();
		
		converted = converted.replaceAll("\\%(?=\\w)", "");
		converted = converted.replaceAll("\\<b\\>\\s*", "*");
		converted = converted.replaceAll("\\<em\\>\\s*", "*");
		converted = converted.replaceAll("\\s*\\<\\/b\\>", "*");
		converted = converted.replaceAll("\\s*\\<\\/em\\>", "*");
		
		converted = removeCommentBlocks(converted);
		converted = convertMultilineLists(converted);
		converted = convertHtmlLists(converted);
		converted = convertLists(converted, "\\li");
		converted = convertLists(converted, "-# ");
		converted = convertMarkupFlag(converted, "\\\\e(?![A-Za-z])", "*", "*", "");
		converted = convertMarkupFlag(converted, "\\\\cite", ":cite:p:`", "`", "");
		converted = convertMarkupFlag(converted, "\\\\c", "``", "``", "");
		converted = convertMarkupFlag(converted, "\\\\ref", ":ref:`", "<", ">`");
		converted = convertMarkupBookends(converted, "\\f$", "\\f$", ":math:`", "` ");
		converted = convertVerbatimBlock(converted);
		
		converted = converted.replaceAll("\\\\f\\[(\r\n|\n)+", "\n\\.\\. math:: \n\n   ");
		converted = converted.replaceAll("\\\\f\\[", "\n\\.\\. math:: ");
		converted = converted.replaceAll("\\\\f\\]\\s*", "\n");
		return converted;
	}
	
	private static String convertLists(String original, String itemMarker) {
		StringBuilder sb = new StringBuilder();
		String[] lines = original.split("\\n|\\r\\n");
		
		boolean firstListItemFound = false;
		boolean insideListItem = false;
		for(int i = 0; i < lines.length; i++) {
			String line = lines[i];
			if(insideListItem) {
				if(line.isBlank()) {
					insideListItem = false;
					sb.append("\n");
				} else if(line.startsWith(itemMarker)) {
					String replacement = line.replace(itemMarker, "-");
					sb.append("\n").append(replacement).append(" ");
				} else {
					sb.append(line).append(" ");
				}
			} else if(line.startsWith(itemMarker)) {
				insideListItem = true;
				if(!firstListItemFound) {
					firstListItemFound = true;
					sb.append("\n"); // Extra spacer for top of list
				}
				String replacement = line.replace(itemMarker, "-");
				sb.append(replacement).append(" ");
			} else {
				if(firstListItemFound) {
					sb.append("\n"); // Extra spacer for bottom of list
					firstListItemFound = false;
				}
				
				sb.append(line); // Pass through
				if(i < lines.length) {
					sb.append("\n");
				}
			}
		}
		return sb.toString();
	}
	
	private static String convertMarkupFlag(String original, String key, String newStart, String newEchoStart, String newEchoEnd) {
		StringBuilder sb = new StringBuilder();
		String[] lines = original.split("\\n|\\r\\n");
		
		boolean insideMonospaceSection = false;
		for(int i = 0; i < lines.length; i++) {
			String line = lines[i];
			String[] words = line.split("\\s");
		
			StringBuilder wordWithMarkup = new StringBuilder("");
			StringBuilder insertedWord = new StringBuilder("");
			
			boolean printedCloser = false;
			for(int j = 0; j < words.length; j++) {
				String permissiveKey = ".*" + key + ".*";
				if(words[j].matches(permissiveKey)) {
					wordWithMarkup = new StringBuilder(words[j].replaceAll(key, ""));
					insideMonospaceSection = true;
				} else {
					if(insideMonospaceSection) {
						if(!words[j].isEmpty()) {
							wordWithMarkup = new StringBuilder(newStart);
							for(int k = 0; k < words[j].length(); k++) {
								String nextChar = "" + (words[j].charAt(k));
								if(nextChar.matches("[A-Za-z0-9_\\-\\n\\/\\{\\}\\*]+")) {
									insertedWord.append(nextChar);
								} else {
									if(insideMonospaceSection) {
										String remainder = words[j].substring(k, words[j].length());
										wordWithMarkup.append(insertedWord.toString());
										wordWithMarkup.append(
											buildMarkupEnd(
												insertedWord.toString(), newEchoStart, newEchoEnd)).append(remainder);
										insideMonospaceSection = false;
										printedCloser = true;
										insertedWord = new StringBuilder();
									} else {
										insertedWord.append(nextChar);
									}
								}
							}

							if(!printedCloser) {
								wordWithMarkup.append(insertedWord.toString());
								wordWithMarkup.append(buildMarkupEnd(insertedWord.toString(), newEchoStart, newEchoEnd));
								insertedWord = new StringBuilder();
								insideMonospaceSection = false;
							}
						}
					} else {
						wordWithMarkup = new StringBuilder(words[j]);
					}
				}
				
				sb.append(wordWithMarkup.toString());
				if(words[j].isEmpty() || (wordWithMarkup.length() > 0 && j < words.length - 1)) {
					sb.append(" ");
				}
				wordWithMarkup = new StringBuilder();
				printedCloser = false;
			}
			if(i < lines.length - 1) {
				sb.append("\n");
			}
		}
		return sb.toString();
	}
	
	private static String buildMarkupEnd(String original, String start, String end) {
		if(start.isBlank() || end.isBlank()) {
			return start + end;
		} else {
			return start + original + end;
		}
	}
	
	private static String convertVerbatimBlock(String original) {
		StringBuilder sb = new StringBuilder();
		String[] lines = original.split("\\n|\\r\\n");
		
		boolean inVerbatimSection = false;
		for(int i = 0; i < lines.length; i++) {
			String line = lines[i];
			if(line.startsWith("\\verbatim")) {
				inVerbatimSection = true;
				sb.append("\n.. code-block::\n\n");
			} else if(line.startsWith("\\endverbatim")) {
				inVerbatimSection = false;
				sb.append("\n");
			} else if(inVerbatimSection) {
				sb.append("    ").append(line).append("\n");
			} else {
				sb.append(line); // Pass through
				if(i < lines.length - 1) {
					sb.append("\n");
				}
			}
		}
		return sb.toString();
	}
	
	private static String removeCommentBlocks(String original) {
		StringBuilder sb = new StringBuilder();
		String[] lines = original.split("\\n|\\r\\n");

		boolean eat = false;
		for(int i = 0; i < lines.length; i++) {
			String[] words = lines[i].split("\\s");
			for(int j = 0; j < words.length; j++) {
				if(!eat) {
					if(words[j].equals("<!�") || words[j].equals("<!--") || words[j].equals("<!---")) {
						eat = true;
					} else {
						sb.append(words[j]);
						if(j < words.length - 1) {
							sb.append(" ");
						}
					}
				} else {
					if(words[j].equals("�>") || words[j].equals("-->") || words[j].equals("--->")) {
						eat = false;
					} 
				}
				
			}
			
			if(!eat && i < lines.length - 1) {
				sb.append("\n");
			}
		}
		return sb.toString();
	}
	
	private static String convertMarkupBookends(String original, String startFlag, String endFlag, String newStartFlag, String newEndFlag) {
		StringBuilder sb = new StringBuilder();
		String[] lines = original.split("\\n|\\r\\n");
		
		boolean insideSection = false;
		for(int i = 0; i < lines.length; i++) {
			String line = lines[i];
			String[] words = line.split("\\s");
			for(int j = 0; j < words.length; j++) {
				String newWord = words[j];
				while(newWord.indexOf(startFlag) != -1 || newWord.indexOf(endFlag) != -1) {
					if(!insideSection) {
						int index = newWord.indexOf(startFlag);
						String paddedNewStartFlag = newStartFlag;
						if(index > 0) {
							paddedNewStartFlag = " " + newStartFlag;
						}
						newWord = newWord.substring(0, index) + paddedNewStartFlag + newWord.substring(index+startFlag.length(), newWord.length());
						insideSection = true;
					} else {
						int index = newWord.indexOf(endFlag);
						newWord = newWord.substring(0, index) + newEndFlag + newWord.substring(index+endFlag.length(), newWord.length());
						insideSection = false;
					}
				}
				sb.append(newWord);
				if(j < words.length - 1) {
					sb.append(" ");
				}
			}
			
			if(i < lines.length - 1) {
				sb.append("\n");
			}
		}
		
		String finalTrimmedString = sb.toString();
		finalTrimmedString = finalTrimmedString.replaceAll(newStartFlag + "\\s", newStartFlag);
		finalTrimmedString = finalTrimmedString.replaceAll("\\s" + newEndFlag, newEndFlag);
		
		return finalTrimmedString;
	}
	
	private static String convertMultilineLists(String original) {
		StringBuilder sb = new StringBuilder();
		String[] lines = original.split("\\n|\\r\\n");
		boolean inLineSection = false;
		for(int i = 0; i < lines.length; i++) {
			String line = lines[i];
			if(!line.isBlank()) {
				String firstCharacter = "" + line.charAt(0);
				
				if(firstCharacter.equals("-")) {
					sb.append("\n"); // New list item needs a new line.
					if(!inLineSection) {
						inLineSection = true;
					}
				} else if(!line.matches("\\s{2}[A-Za-z0-9\\\\]+.*") && inLineSection) {
					inLineSection = false;
				}
				
				sb.append(line);
			} else if(inLineSection) {
				sb.append("\n");
				inLineSection = false;
			}
			
			if(!inLineSection) {
				sb.append("\n");
			}
		}
		return sb.toString();
	}
	
	private static String convertHtmlLists(String original) {
		StringBuilder sb = new StringBuilder();
		Deque<ListType> listStack = new ArrayDeque<>();
		ListType currentList = ListType.NONE;
		
		String[] lines = original.split("\\n|\\r\\n");
		int listDepth = 0;
		int listItemCount = 0;
		
		for(int i = 0; i < lines.length; i++) {
			boolean partOfPreviousListItem = false;
			String line = lines[i];
			
			if(line.contains("<ul>")) {
				listStack.push(ListType.UNORDERED);
				listDepth++;
			} else if(line.contains("<ol>")) {
				listStack.push(ListType.ORDERED);
				listItemCount = 0;
				listDepth++;
			} else if(line.contains("</ul>") || line.contains("</ol>")) {
				listStack.pop();
				listDepth--;
			} else if(line.contains("<li>")) {
				sb.append("\n");
				if(currentList == ListType.ORDERED) {
					listItemCount++;
				}
			} else if(currentList == ListType.ORDERED || currentList == ListType.UNORDERED) {
				partOfPreviousListItem = true;
			}
			
			currentList = listStack.peek();
			if(currentList == null) currentList = ListType.NONE;
			
			String scrubbedLine  = line.replace("<ul>", "");
			scrubbedLine = scrubbedLine.replace("</ul>", "");
			scrubbedLine = scrubbedLine.replace("<ol>", "");
			scrubbedLine = scrubbedLine.replace("</ol>", "");
			scrubbedLine = scrubbedLine.replace("<li>", "");
			scrubbedLine = scrubbedLine.replace("</li>", "");
			
			if(currentList == ListType.ORDERED || currentList == ListType.UNORDERED) {
				int depthSpace = listDepth-1;
				for(int j = 0; j < depthSpace; j++) {
					sb.append("  ");
				}
				
				if(!scrubbedLine.isBlank()) {
					if(partOfPreviousListItem) {
						String removePreviousLineBreakString = sb.toString();
						while(removePreviousLineBreakString.endsWith(" ") || removePreviousLineBreakString.endsWith("\n") || removePreviousLineBreakString.endsWith("\r\n")) {
							removePreviousLineBreakString =
								removePreviousLineBreakString.substring(0, removePreviousLineBreakString.length()-1);
						}
						sb = new StringBuilder();
						sb.append(removePreviousLineBreakString).append(" ");
					} else {
						if(currentList == ListType.ORDERED) {
							sb.append(listItemCount).append(". ");
						} else if(currentList == ListType.UNORDERED) {
							sb.append("- ");
						}
					}
				}
				sb.append(scrubbedLine.trim());
			} else {
				sb.append(scrubbedLine);
			}
			sb.append("\n");
		}
		return sb.toString();
	}	
}
