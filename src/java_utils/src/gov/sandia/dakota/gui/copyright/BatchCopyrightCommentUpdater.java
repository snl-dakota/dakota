package gov.sandia.dakota.gui.copyright;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.FilenameUtils;
import org.apache.commons.lang3.ArrayUtils;

public class BatchCopyrightCommentUpdater {
	
	// ***General outline of what this script does***
	// 
	// 1. Get root repository directory with Dakota plugins checked out
	// 2. Dive recursively into each plugin
	// 3. Find all .java files
	// 4. If copyright header does not exist, add it (SAW-IF or Dakota GUI, depending on configuration)
	// 5. If copyright header already exists, update the year
	
	///////////////
	// CONSTANTS //
	///////////////
	
	private static final String DAKOTA_HEADER =
			  "/*******************************************************************************\n"
			+ " * Dakota Graphical User Interface (Dakota GUI)\n"
			+ " * Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC (NTESS).\n"
			+ " * Under the terms of Contract DE-NA0003525 with NTESS, the U.S. Government retains\n"
			+ " * certain rights in this software.\n"
			+ " * \n"
			+ " * This software is distributed under the Eclipse Public License.  For more information,\n"
			+ " * see the files copyright.txt and license.txt included with the software.\n"
			+ " ******************************************************************************/";
	
	private static final String SAW_IF_HEADER = 
			  "/*******************************************************************************\n"
			+ " * Sandia Analysis Workbench Integration Framework (SAW)\n"
			+ " * Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC (NTESS).\n"
			+ " * Under the terms of Contract DE-NA0003525 with NTESS, the U.S. Government retains\n"
			+ " * certain rights in this software.\n"
			+ " * \n"
			+ " * This software is distributed under the Eclipse Public License.  For more\n"
			+ " * information see the files copyright.txt and license.txt included with the software.\n"
			+ " ******************************************************************************/";
	
	private static final String TEST_STRING = "Under the terms of Contract DE-NA0003525 with NTESS,";
	private static final String STRING_2018 = "* Copyright 2018 National Technology & Engineering Solutions of Sandia, LLC (NTESS).";
	private static final String STRING_2019 = "* Copyright 2019 National Technology & Engineering Solutions of Sandia, LLC (NTESS).";
	private static final String STRING_2020 = "* Copyright 2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).";
	private static final String STRING_2021 = "* Copyright 2021 National Technology & Engineering Solutions of Sandia, LLC (NTESS).";
	
	private static final String STRING_CURRENT = "* Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC (NTESS).";
	
	///////////////////
	// CONFIGURATION //
	///////////////////
	
	private static final boolean DRY_RUN            = true;
	private static final String START_DIR          = "C:\\Users\\emridgw\\workspace\\svn\\dart";
	private static final boolean USE_DAKOTA_HEADER = true;
	private static final boolean USE_SAW_HEADER    = true;
	private static final boolean UPDATE_EXISTING_HEADERS = true;
	private static final boolean INSERT_IF_MISSING = true;
	private static final String[] FOLDER_BLACKLIST = new String[] {
		".metadata",
		".sonarlint",
		"apache-ant-1.10.7",
		"BinaryPluginHelper",
		"build",
		"CC-build-info",
		".settings",
		".svn",
		"bin",
		"test",
		"testdata",
		"cubit-bundle-Windows-AMD64",
		"gov.sandia.aleph",
		"gov.sandia.aleph.ui",
		"gov.sandia.aleph.workflow.runtime",
		"gov.sandia.dakota.method.soga",
		"gov.sandia.dart.dakota.tests",
		"gov.sandia.hdf.hdfview",
		"gov.sandia.hdf.hdfview.tests",
		"gov.sandia.modules",
		"gov.sandia.modules.runtime",
		"org.eclipse.graphiti",
		"org.eclipse.graphiti.mm",
		"org.eclipse.graphiti.ui",
		"SAW-Target-Platform",
		"Target-Platform"
	};
	
	public static void main(String args[]) throws Exception {
		updateJavaFiles(new File(START_DIR));
	}
	
	private static void updateJavaFiles(File startDir) throws IOException {
		for(File childFile : startDir.listFiles()) {
			if(childFile.isDirectory() && !ArrayUtils.contains(FOLDER_BLACKLIST, childFile.getName())) {
				updateJavaFiles(childFile);
			} else {
				String extension = FilenameUtils.getExtension(childFile.getName());
				if(extension.equals("java")) {
					updateJavaFile(childFile);
				}
			}
		}
	}
	
	private static void updateJavaFile(File javaFile) throws IOException {
		boolean hasHeader = hasCommentHeader(javaFile);
		if(hasHeader && UPDATE_EXISTING_HEADERS) {
			updateYearOfCommentHeader(javaFile, STRING_2018);
			updateYearOfCommentHeader(javaFile, STRING_2019);
			updateYearOfCommentHeader(javaFile, STRING_2020);
			updateYearOfCommentHeader(javaFile, STRING_2021);
		} else if(!hasHeader && INSERT_IF_MISSING) {
			if(USE_DAKOTA_HEADER) insertHeader(javaFile, DAKOTA_HEADER);
			else if(USE_SAW_HEADER) insertHeader(javaFile, SAW_IF_HEADER);
		}
	}
	
	private static boolean hasCommentHeader(File javaFile) throws IOException {
		String contents = FileUtils.readFileToString(javaFile, StandardCharsets.UTF_8);
		return contents.contains(TEST_STRING);
	}
	
	private static void updateYearOfCommentHeader(File javaFile, String oldYear) throws IOException {
		String contents = FileUtils.readFileToString(javaFile, StandardCharsets.UTF_8);
		if(contents.contains(oldYear)) {
			if(DRY_RUN) {
				System.out.println("Would update " + javaFile.getAbsolutePath() + " with new year.");
			} else {
				contents = contents.replace(oldYear, STRING_CURRENT);
				FileUtils.write(javaFile, contents, StandardCharsets.UTF_8, false);
			}
		}
	}
	
	private static void insertHeader(File javaFile, String header) throws IOException {
		if(DRY_RUN) {
			System.out.println("Would update " + javaFile.getAbsolutePath() + " with missing header.");
		} else {
			String contents = FileUtils.readFileToString(javaFile, StandardCharsets.UTF_8);
			boolean usesCRLF = contents.contains("\r\n");
			String ln = usesCRLF ? "\r\n" : "\n";
			
			String finalFileContents = header + ln + contents;
			FileUtils.write(javaFile, finalFileContents, StandardCharsets.UTF_8, false);
		}
	}
}
