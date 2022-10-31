/*******************************************************************************
 * Sandia Analysis Workbench Integration Framework (SAW)
 * Copyright 2016 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * This software is distributed under the Eclipse Public License.
 * For more information see the files copyright.txt and license.txt
 * included with the software.
 ******************************************************************************/
package gov.sandia.dart.dakota;

import java.io.PrintWriter;
import java.io.Writer;

public class Indenter {
	public static final String SPC = "                                                                                                                                                                                  ";
	private int indent = 0;
	private PrintWriter writer;

	public Indenter(Writer w) {
		writer = new PrintWriter(w);
	}
	
	public void close() {
		if (writer != null)
			writer.close();
	}
	public void printAndIndent(String string) {
		writer.print(SPC.substring(0, indent*2));
		writer.println(string);
		++ indent;
	}
	
	public void unindentAndPrint(String string) {
		-- indent;
		writer.print(SPC.substring(0, indent*2));
		writer.println(string);
	}
	
	public void unindent() {
		-- indent;
	}
	
	public void printIndented(String string) {
		writer.print(SPC.substring(0, indent*2));
		writer.println(string);
	}
}
