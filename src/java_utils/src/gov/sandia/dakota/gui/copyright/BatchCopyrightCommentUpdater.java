package gov.sandia.dakota.gui.copyright;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.FilenameUtils;

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
			+ " * Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC (NTESS).\n"
			+ " * Under the terms of Contract DE-NA0003525 with NTESS, the U.S. Government retains\n"
			+ " * certain rights in this software.\n"
			+ " * \n"
			+ " * This software is distributed under the Eclipse Public License.  For more information,\n"
			+ " * see the files copyright.txt and license.txt included with the software.\n"
			+ " ******************************************************************************/";
	
	private static final String SAW_IF_HEADER = 
			  "/*******************************************************************************\n"
			+ " * Sandia Analysis Workbench Integration Framework (SAW)\n"
			+ " * Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC (NTESS).\n"
			+ " * Under the terms of Contract DE-NA0003525 with NTESS, the U.S. Government retains\n"
			+ " * certain rights in this software.\n"
			+ " * \n"
			+ " * This software is distributed under the Eclipse Public License.  For more\n"
			+ " * information see the files copyright.txt and license.txt included with the software.\n"
			+ " ******************************************************************************/";
	
	private static final String TEST_STRING = "Under the terms of Contract DE-NA0003525 with NTESS,";
	private static final String STRING_2018 = "* Copyright 2018";
	private static final String STRING_2019 = "* Copyright 2019";
	private static final String STRING_2020 = "* Copyright 2020";
	private static final String STRING_2021 = "* Copyright 2021";
	private static final String STRING_2022 = "* Copyright 2022";
	
	private static final String STRING_CURRENT = "* Copyright 2023";
	
	public static final List<String> DAKOTA_GUI_PLUGINS = List.of(
		"gov.sandia.bb.commons",
		"gov.sandia.bb.commons.ui",
		"gov.sandia.bb.commons.workflow.runtime",
		"gov.sandia.bb.commons.workflow.ui",
		"gov.sandia.blackbox",
		"gov.sandia.blackbox.aprepro",
		"gov.sandia.blackbox.bmf",
		"gov.sandia.blackbox.dakota",
		"gov.sandia.blackbox.dakota.surrogate",
		"gov.sandia.blackbox.im",
		"gov.sandia.blackbox.iwf",
		"gov.sandia.blackbox.struct",
		"gov.sandia.blackbox.workflow.runtime",
		"gov.sandia.blackbox.workflow.ui",
		"gov.sandia.bmf",
		"gov.sandia.bmf.ui",
		"gov.sandia.bmf.units",
		"gov.sandia.chartreuse",
		"gov.sandia.chartreuse.csv.bridge",
		"gov.sandia.chartreuse.distributions",
		"gov.sandia.chartreuse.distributions.dakota.bridge",
		"gov.sandia.chartreuse.impl.plotly",
		"gov.sandia.chartreuse.impl.plotly.linux",
		"gov.sandia.chartreuse.impl.plotly.mac",
		"gov.sandia.chartreuse.impl.plotly.windows",
		"gov.sandia.chartreuse.workflow",
		"gov.sandia.dakota.chartreuse.bridge",
		"gov.sandia.dakota.chartreuse.hdf.bridge",
		"gov.sandia.dakota.chartreuse.workflow",
		"gov.sandia.dakota.common",
		"gov.sandia.dakota.common.ui",
		"gov.sandia.dakota.editor",
		"gov.sandia.dakota.editor.method.ga",
		"gov.sandia.dakota.editor.method.hybrid",
		"gov.sandia.dakota.editor.method.optpp",
		"gov.sandia.dakota.editor.method.paramsweep",
		"gov.sandia.dakota.editor.method.sampling",
		"gov.sandia.dakota.editor.model.surrogates",
		"gov.sandia.dakota.glimmer.bridge",
		"gov.sandia.dakota.help",
		"gov.sandia.dakota.jni",
		"gov.sandia.dakota.mlmf",
		"gov.sandia.dakota.parser",
		"gov.sandia.dakota.parser.ui",
		"gov.sandia.dakota.runner",
		"gov.sandia.dakota.taxonomy",
		"gov.sandia.dakota.wizards",
		"gov.sandia.dakota.workflow.bridge",
		"gov.sandia.dart.dakota.jobsubmission",
		"gov.sandia.dart.dakota.workflow.runtime",
		"gov.sandia.dart.dakota.workflow.ui",
		"gov.sandia.dart.dakotaui.app",
		"gov.sandia.glimmer",
		"gov.sandia.hdf.chartreuse.bridge",
		"gov.sandia.hdf.eclipse",
		"gov.sandia.hdf.workflow.runtime",
		"gov.sandia.hdf.workflow.ui",
		"gov.sandia.highlighter",
		"gov.sandia.highlighter.ui",
		"gov.sandia.qoi",
		"gov.sandia.qoi.ui"
	);
	
	public static final List<String> SAW_IF_PLUGINS = List.of(
		"com.strikewire.snl.apc.Common",
		"com.strikewire.snl.apc.FileManager",
		"com.strikewire.snl.apc.FileSystemDataModel",
		"com.strikewire.utils",
		"gov.sandia.apc.JobSubmission",
		"gov.sandia.apc.JobSubmission.help",
		"gov.sandia.apc.machines",
		"gov.sandia.apc.terminal",
		"gov.sandia.dart.accounts.jobs",
		"gov.sandia.dart.accounts.jobs.localstorage",
		"gov.sandia.dart.accounts.jobs.provider",
		"gov.sandia.dart.apache.commons",
		"gov.sandia.dart.application",
		"gov.sandia.dart.aprepro",
		"gov.sandia.dart.argv",
		"gov.sandia.dart.chart.xyplot.ui",
		"gov.sandia.dart.common.core",
		"gov.sandia.dart.common.preferences",
		"gov.sandia.dart.contrib.xml",
		"gov.sandia.dart.jna",
		"gov.sandia.dart.launch",
		"gov.sandia.dart.login",
		"gov.sandia.dart.login.core",
		"gov.sandia.dart.login.history.view",
		"gov.sandia.dart.login.krb",
		"gov.sandia.dart.login.krb.status",
		"gov.sandia.dart.login.sspi",
		"gov.sandia.dart.login.sspi.status",
		"gov.sandia.dart.metrics",
		"gov.sandia.dart.workflow.editor",
		"gov.sandia.dart.workflow.help",
		"gov.sandia.dart.workflow.phase3",
		"gov.sandia.dart.workflow.phase3.embedded",
		"gov.sandia.dart.workflow.phase3.modules",
		"gov.sandia.modules.runtime"
	);
	
	public static final List<String> NON_DAKOTA_PLUGINS = List.of(
		"gov.sandia.aleph",
		"gov.sandia.aleph.ui",
		"gov.sandia.aleph.workflow.runtime",
		"gov.sandia.bb.commons.ui.saw",
		"gov.sandia.blackbox.xyce",
		"gov.sandia.dakota.method.soga",
		"gov.sandia.dakota.parser.ui.saw.bridge",
		"gov.sandia.dart.dakota.tests",
		"gov.sandia.hdf.hdfview",
		"gov.sandia.hdf.hdfview.tests",
		"gov.sandia.modules",
		"gov.sandia.modules.runtime",
		"org.eclipse.graphiti",
		"org.eclipse.graphiti.mm",
		"org.eclipse.graphiti.ui"
	);
	
	public static final List<String> PLATFORM_PLUGINS = List.of(
		"apache-ant-1.10.7",
		"BinaryPluginHelper",
		"build",
		"CC-build-info",
		"cubit-bundle-Windows-AMD64",
		"SAW-Target-Platform",
		"Target-Platform"
	);
	
	public static final List<String> NON_SRC_FOLDERS = List.of(
		".metadata",
		".sonarlint",
		".settings",
		".svn",
		"bin",
		"test",
		"testdata"
	);
	
	///////////////////
	// CONFIGURATION //
	///////////////////
	
	private static final boolean DRY_RUN           = false;
	private static final String START_DIR          = "C:\\Users\\emridgw\\workspace\\svn\\dart";
	private static final boolean UPDATE_EXISTING_HEADERS = true;
	private static final boolean INSERT_IF_MISSING = true;
	private static final boolean INSERT_DAKOTA_HEADER = true;
	private static final boolean INSERT_SAW_HEADER    = false;
	private static final List<String> FOLDER_WHITELIST = new ArrayList<>();
	
	static {
		// FOLDER_WHITELIST.addAll(SAW_IF_PLUGINS);
		FOLDER_WHITELIST.addAll(DAKOTA_GUI_PLUGINS);
	}
	
	//////////
	// MAIN //
	//////////
	
	public static void main(String args[]) throws Exception {
		updateJavaFiles(new File(START_DIR));
		System.out.println("Done!");
	}
	
	private static void updateJavaFiles(File startDir) throws IOException {
		for(File childFile : startDir.listFiles()) {
			if(childFile.isDirectory() && !NON_SRC_FOLDERS.contains(childFile.getName()) ) {
				if(startDir.getAbsolutePath().equals(START_DIR)) { // More stringent checks for root folders.
					if(FOLDER_WHITELIST.contains(childFile.getName())) {
						System.out.println("Working on " + childFile.getName() + "...");
						updateJavaFiles(childFile);
					}
				} else {
					updateJavaFiles(childFile);
				}
			} else if(!childFile.isDirectory()) {
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
			updateYearOfCommentHeader(javaFile, STRING_2022);
		} else if(!hasHeader && INSERT_IF_MISSING) {
			if(INSERT_DAKOTA_HEADER) insertHeader(javaFile, DAKOTA_HEADER);
			else if(INSERT_SAW_HEADER) insertHeader(javaFile, SAW_IF_HEADER);
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
				System.out.println("Updated " + javaFile.getAbsolutePath() + " with new year.");
				contents = contents.replace(oldYear, STRING_CURRENT);
				FileUtils.write(javaFile, contents, StandardCharsets.UTF_8, false);
			}
		}
	}
	
	private static void insertHeader(File javaFile, String header) throws IOException {
		if(DRY_RUN) {
			System.out.println("Would update " + javaFile.getAbsolutePath() + " with missing header.");
		} else {
			System.out.println("Updated " + javaFile.getAbsolutePath() + " with missing header.");
			String contents = FileUtils.readFileToString(javaFile, StandardCharsets.UTF_8);
			boolean usesCRLF = contents.contains("\r\n");
			String ln = usesCRLF ? "\r\n" : "\n";
			String formattedHeader = usesCRLF ? header.replace("\n", "\r\n") : header;
			
			String finalFileContents = formattedHeader + ln + contents;
			FileUtils.write(javaFile, finalFileContents, StandardCharsets.UTF_8, false);
		}
	}
}
