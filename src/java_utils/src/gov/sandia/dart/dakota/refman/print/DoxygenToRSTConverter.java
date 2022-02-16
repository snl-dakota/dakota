package gov.sandia.dart.dakota.refman.print;

public class DoxygenToRSTConverter {
	
	public static String convert(String original) {
		String converted = original.trim();
		
		converted = converted.replaceAll("\\%(?=\\w)", "");
		converted = converted.replaceAll("\\<b\\>\\s*", "*");
		converted = converted.replaceAll("\\<em\\>\\s*", "*");
		converted = converted.replaceAll("\\s*\\<\\/b\\>", "*");
		converted = converted.replaceAll("\\s*\\<\\/em\\>", "*");
		
		converted = removeCommentBlocks(converted);
		converted = convertLists(converted);
		converted = convertMarkupFlag(converted, "\\c", "``", "``", "");
		converted = convertMarkupFlag(converted, "\\ref", ":ref:`", "<", ">`");
		converted = convertVerbatimBlock(converted);
		
		converted = converted.replaceAll("\\\\f\\[", "\n\\.\\.\smath:: ");
		converted = converted.replaceAll("\\\\f\\]", "\n");
		return converted;
	}
	
	private static String convertLists(String original) {
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
				} else if(line.contains("\\li")) {
					String replacement = line.replace("\\li", "-");
					sb.append("\n").append(replacement).append(" ");
				} else {
					sb.append(line).append(" ");
				}
			} else if(line.contains("\\li")) {
				insideListItem = true;
				if(!firstListItemFound) {
					firstListItemFound = true;
					sb.append("\n"); // Extra spacer for top of list
				}
				String replacement = line.replace("\\li", "-");
				sb.append(replacement).append(" ");
			} else {
				sb.append(line); // Pass through
				if(i < lines.length - 1) {
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
				if(words[j].contains(key)) {
					wordWithMarkup = new StringBuilder(words[j].replace(key, ""));
					insideMonospaceSection = true;
				} else {
					if(insideMonospaceSection) {
						if(!words[j].isEmpty()) {
							wordWithMarkup = new StringBuilder(newStart);
							for(int k = 0; k < words[j].length(); k++) {
								String nextChar = "" + (words[j].charAt(k));
								if(nextChar.matches("[A-Za-z0-9_\\-\\n]+")) {
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
					if(words[j].equals("<!–") || words[j].equals("<!--")) {
						eat = true;
					} else {
						sb.append(words[j]);
						if(j < words.length - 1) {
							sb.append(" ");
						}
					}
				} else {
					if(words[j].equals("–>") || words[j].equals("-->")) {
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
}
