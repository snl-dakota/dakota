package gov.sandia.dakota.gui.version;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.ArrayUtils;
import org.apache.commons.lang3.tuple.ImmutableTriple;
import org.apache.commons.lang3.tuple.Triple;

/**
 * 
 * @author Elliott Ridgway
 *
 */
public class BatchPluginVersionUpdater {

	// ***General outline of what this script does***
	// 
	// PHASE 1
	// 1. For every plugin, find the ReleaseNotes file
	// 2. If it contains a [NEXT] section, search for lines prefixed with Major or Minor.
	//    Use this information to bump the plugin version.
	//    Note: You can manually find all [NEXT] ReleaseNotes files with the following command:
	//    grep -r "\[NEXT\]" gov.*/ReleaseNotes
	// 3. Update the MANIFEST.MF and ReleaseNotes files with the new version number.
	// 4. Remember changed version numbers for the next step.
	//
	// PHASE 2
	// 1. For every feature, look at list of plugins and see what has changed from Phase 1.
	//    Any newly-added plugins must be at version 0.0.0 to be considered "new."
	// 2. Update feature.xml with new plugin version numbers. Features can also update each
	//    other if they include child features with version numbers.
	// 3. If ReleaseNotes file exists, add comment to ReleaseNotes file documenting the change
	// 4. Update the feature.xml with a new feature version number, based on the internal changes.
	//
	// PHASE 3
	// 1. Update DakotaUI.product with new feature numbers.
	// 2. If desired by the one running this script, update the gov.sandia.dart.dakotaui.app
	//    ReleaseNotes file and MANIFEST.MF with a new patch version or minor version, and a
	//    high-level description of what changed. This part should be done manually
	//    by the user.
	
	///////////////
	// CONSTANTS //
	///////////////
	
	private static final String RELEASENOTES_FILE = "ReleaseNotes";
	private static final String MANIFEST_MF_FILE = "META-INF/MANIFEST.MF";
	private static final String FEATURE_XML_FILE = "feature.xml";
	private static final String NEXT_MARKER = "[NEXT]";
	public static final String LINE_SPLIT_REGEX = "\\r\\n|\\n";
	
	///////////////////
	// CONFIGURATION //
	///////////////////
	
	private static final boolean DRY_RUN = false;
	private static final String START_DIR = "C:\\Users\\emridgw\\workspace\\svn\\dart";
	private static final String[] PLUGIN_BLACKLIST = new String[]{
		"com.strikewire.snl.apc.Common",
		"com.strikewire.snl.apc.FileManager",
		"com.strikewire.snl.apc.FileSystemDataModel",
		"com.strikewire.utils",
		"gov.sandia.aleph",
		"gov.sandia.aleph.ui",
		"gov.sandia.aleph.workflow.runtime",
		"gov.sandia.aleph.workflow.ui",
		"gov.sandia.apc.editor.calore",
		"gov.sandia.apc.JobSubmission",
		"gov.sandia.apc.JobSubmission.help",
		"gov.sandia.apc.machines",
		"gov.sandia.apc.terminal",
		"gov.sandia.bb.commons.ui.saw",
		"gov.sandia.dakota.parser.ui.saw.bridge",
		"gov.sandia.dart.apache.commons",
		"gov.sandia.dart.application",
		"gov.sandia.dart.aprepro",
		"gov.sandia.dart.argv",
		"gov.sandia.dart.chart.xyplot.ui",
		"gov.sandia.dart.common.core",
		"gov.sandia.dart.common.preferences",
		"gov.sandia.dart.cubit.jni",
		"gov.sandia.dart.cubit.jni.win64",
		"gov.sandia.dart.dakota.tests",
		"gov.sandia.dart.hdf.exodus.bridge",
		"gov.sandia.dart.jna",
		"gov.sandia.dart.launch",
		"gov.sandia.dart.machine.definitions",
		"gov.sandia.dart.metrics",
		"gov.sandia.dart.packaging.jre",
		"gov.sandia.dart.workflow.editor",
		"gov.sandia.dart.workflow.help",
		"gov.sandia.dart.workflow.phase3",
		"gov.sandia.dart.workflow.phase3.embedded",
		"gov.sandia.modules",
		"gov.sandia.modules.runtime",
		"gov.sandia.packaging.jre",
		"org.eclipse.graphiti",
		"org.eclipse.graphiti.mm",
		"org.eclipse.graphiti.ui",
		"org.eclipse.nebula.visualization.xygraph"
	};
	private static final String[] FEATURE_BLACKLIST = new String[]{
		"gov.sandia.apc.JobSubmission.feature",
		"gov.sandia.dakota.wizards.feature",
		"gov.sandia.dart.aprepro.feature",
		"gov.sandia.dart.dakota.feature",
		"gov.sandia.dart.dfm.feature",
		"gov.sandia.dart.machine.definitions.feature",
		"gov.sandia.dart.metrics.feature",
		"gov.sandia.dart.ngw.core.feature",
		"gov.sandia.saw.dakota.bridge.feature",
		"gov.sandia.snl.aleph.feature",
		"gov.sandia.snl.hpc.feature",
		"gov.sandia.snl.login.feature"
	};
	private static final String[] DEPRECATED_PLUGINS = new String[] {};
	private static final String[] DEPRECATED_FEATURES = new String[] {};
	private static final boolean GET_PLUGIN_LIST = true;
	private static final boolean UPDATE_PLUGIN_RELEASENOTES = true;
	private static final boolean UPDATE_PLUGIN_MANIFEST = true;
	private static final boolean GET_FEATURE_LIST = true;
	private static final boolean UPDATE_FEATURE_XML = true;
	private static final boolean UPDATE_FEATURE_RELEASENOTES = true;
	private static final boolean UPDATE_DAKOTAGUI_PRODUCT = true;
	private static final String GUI_VERSION = "6.17.1";
	private static final String BUILD_DATE = "2023/02/22";
	
	////////////
	// FIELDS //
	////////////
	
	// Plugin name, old version, new version
	private static Set<PluginDelta> updatedPluginList = new HashSet<>();
	private static Set<FeatureDelta> updatedFeatureList = new HashSet<>();
	
	//////////
	// MAIN //
	//////////
	
	public static void main(String[] args) throws Exception {
		if(GET_PLUGIN_LIST) mainUpdatePlugins();
		if(GET_FEATURE_LIST) mainUpdateFeatures();
		if(UPDATE_DAKOTAGUI_PRODUCT) mainUpdateDakotaProduct(); 
	}
	
	private static void mainUpdatePlugins() throws IOException {
		List<String> pluginList = getListOfPlugins(START_DIR, false);
		System.out.println("Plugins:");
		for(String plugin : pluginList) {
			System.out.println(plugin);
		}
		System.out.println("***************");
		
		for(String plugin : pluginList) {
			if(UPDATE_PLUGIN_RELEASENOTES) {
				File releaseNotesFile = getReleaseFile(plugin, RELEASENOTES_FILE);
				if(releaseNotesFile != null) {
					PluginDelta updatedPlugin = writePluginReleaseNotesFile(plugin, releaseNotesFile);
					if(updatedPlugin != null) {
						updatedPluginList.add(updatedPlugin);
						if(UPDATE_PLUGIN_MANIFEST) {
							File manifestFile = getReleaseFile(plugin, MANIFEST_MF_FILE);
							if(manifestFile != null) {
								writeManifestFile(manifestFile, updatedPlugin.newVersion);
							}
						}
					}
				}
			}
		}
		System.out.println("***************");
		
		if(DRY_RUN) {
			List<PluginDelta> sortedList = new ArrayList<>();
			sortedList.addAll(updatedPluginList);
			Collections.sort(sortedList, (PluginDelta o1, PluginDelta o2) -> {
				return o1.name.compareTo(o2.name);
			});
			for(PluginDelta plugin : sortedList) {
				System.out.println(plugin.name + " (" + plugin.oldVersion + " -> " + plugin.newVersion + ")");
			}
			System.out.println("***************");
		}
	}
	
	private static void mainUpdateFeatures() throws IOException {
		List<String> featureNameList = getListOfPlugins(START_DIR, true);
		System.out.println("Features:");
		for(String feature : featureNameList) {
			System.out.println(feature);
		}
		System.out.println("***************");
		
		List<FeatureDelta> features = new ArrayList<>();
		for(String featureName : featureNameList) {
			File featureFile = new File(featureName);
			features.add(new FeatureDelta(featureFile));
		}
		for(FeatureDelta feature : features) {
			int level = feature.getLevel(features, 1);
			feature.level = level;
		}
		Collections.sort(features);
		
		for(FeatureDelta feature : features) {
			if(UPDATE_FEATURE_XML) {
				boolean featureUpdated = updateFeature(feature);
				if(featureUpdated) {
					updatedFeatureList.add(feature);
					if(UPDATE_FEATURE_RELEASENOTES) {
						File featureReleaseNotes = getReleaseFile(feature.featureFileAbsPath, RELEASENOTES_FILE);
						if(featureReleaseNotes != null) {
							writeFeatureReleaseNotes(featureReleaseNotes, feature);
						}
					}
				}
			}
		}
		
		System.out.println("***************");
		if(DRY_RUN) {
			List<FeatureDelta> sortedList = new ArrayList<>();
			sortedList.addAll(updatedFeatureList);
			Collections.sort(sortedList, (FeatureDelta o1, FeatureDelta o2) -> {
				return o1.id.compareTo(o2.id);
			});
			for(FeatureDelta feature : sortedList) {
				System.out.println(feature.id + " (" + feature.oldVersion + " -> " + feature.newVersion + ")");
			}
			System.out.println("***************");
		}
	}
	
	private static void mainUpdateDakotaProduct() throws IOException {
		File topLevelPlugin = getTopLevelPlugin(START_DIR);
		List<PluginDelta> topLevelFeatures = getTopLevelFeatures(topLevelPlugin);
		updateTopLevelFeatures(topLevelFeatures);
		writeProductFile(topLevelPlugin, topLevelFeatures);
		writeProductReleaseNotes(topLevelPlugin);
	}
	
	private static Triple<Integer,Integer,Integer> versionNumberStringIntoTriple(String versionNumber) {
		String[] numbers = versionNumber.split("\\.");
		Integer major = Integer.parseInt(numbers[0]);
		Integer minor = Integer.parseInt(numbers[1]);
		Integer patch = Integer.parseInt(numbers[2]);
		return new ImmutableTriple<>(major,minor,patch);
	}
	
	/////////////////////////////
	// PHASE 1: UPDATE PLUGINS //
	/////////////////////////////
	
	private static List<String> getListOfPlugins(String startDir, boolean getFeatures) {
		List<String> pluginList = new ArrayList<>();
		
		File startDirFile = new File(startDir);
		if(!startDirFile.exists()) {
			throw new IllegalStateException(startDir + " does not exist!");
		}
		
		for(File childDir : startDirFile.listFiles()) {
			String name = childDir.getName();
			boolean isFeature = childDir.isDirectory() && name.contains("feature");
			for(String blacklistFeature : FEATURE_BLACKLIST) {
				isFeature = isFeature && !name.contains(blacklistFeature);
			}
			
			boolean isPlugin  = childDir.isDirectory() && !name.contains("feature") && name.contains("gov.sandia.");
			for(String blacklistPlugin : PLUGIN_BLACKLIST) {
				isPlugin = isPlugin && !name.contains(blacklistPlugin);
			}
			
			if((isFeature && getFeatures) || (isPlugin && !getFeatures)) {
				pluginList.add(childDir.getAbsolutePath());
			}
		}
		
		return pluginList;
	}
	
	private static File getReleaseFile(String parentDir, String releaseFileName) {
		File releaseFile = new File(parentDir, releaseFileName);
		if(releaseFile.exists()) {
			return releaseFile;
		}
		return null;
	}
	
	private static PluginDelta writePluginReleaseNotesFile(
			String pluginPath, File releaseNotesFile) throws IOException {
		
		String contents = FileUtils.readFileToString(releaseNotesFile, StandardCharsets.UTF_8);
		List<String> nextSection = getNextSection(contents);
		if(!nextSection.isEmpty()) {
			Triple<Integer,Integer,Integer> oldVersionNumber = getPreviousVersionNumber(contents);
			
			String oldVersionNumberStr =
					oldVersionNumber.getLeft() + "." + oldVersionNumber.getMiddle() + "." + oldVersionNumber.getRight();
			String newVersionNumber = "";
			if(oldVersionNumberStr.equals("0.0.0")) {
				newVersionNumber = "1.0.0";
			} else {
				newVersionNumber = determineNewVersionNumberFromNotes(nextSection, oldVersionNumber);
			}
			List<String> modifiedSection = updateSectionWithNewVersionNumber(nextSection, newVersionNumber);
			if(DRY_RUN) {
				System.out.println("Would update release notes file " + releaseNotesFile.getAbsolutePath());
			} else {
				replaceReleaseNotesFileContents(releaseNotesFile, contents, modifiedSection);
				System.out.println("Updated release notes file " + releaseNotesFile.getAbsolutePath());
			}
			
			File pluginDir = new File(pluginPath);
			return new PluginDelta(pluginDir.getName(), oldVersionNumberStr, newVersionNumber);
		}
		return null;
	}
	
	private static Triple<Integer,Integer,Integer> getPreviousVersionNumber(String contents) {
		String[] lines = contents.split(LINE_SPLIT_REGEX);
		int linePointer = 0;
		while(linePointer < lines.length) {
			String line = lines[linePointer];
			if(line.matches("\\d{1,2}\\.\\d{1,2}\\.\\d{1,2}\\s.*")) { // The first non-next version number section
				String[] versionNumberTokens = line.split("\\s+");
				String versionNumberStr = versionNumberTokens[0];
				return versionNumberStringIntoTriple(versionNumberStr);
			} else {
				linePointer++;
			}
		}
		return new ImmutableTriple<>(0,0,0); // We didn't find an older version number, so this must be a new plugin.
	}
	
	private static List<String> getNextSection(String contents) {
		String[] lines = contents.split("\n|\\r\n");
		List<String> relevantLines = new ArrayList<>();
		int linePointer = 0;
		if(lines[linePointer].equalsIgnoreCase(NEXT_MARKER)) {
			boolean atEnd = false;
			while(!atEnd && linePointer < lines.length) {
				String line = lines[linePointer];
				if(!line.matches("\\d{1,2}\\.\\d{1,2}\\.\\d{1,2}.*")) { // The next version number section (i.e. "1.0.0")
					relevantLines.add(line);
					linePointer++;
				} else {
					atEnd = true;
				}
			}
		}
		return relevantLines;
	}
	
	private static String determineNewVersionNumberFromNotes(List<String> section, Triple<Integer, Integer, Integer> previous) {
		boolean majorTrigger = false;
		boolean minorTrigger = false;
		for(String line : section) {
			if(line.contains("Major") || line.contains("Non-Passive")) {
				majorTrigger = true;
			} else if(line.contains("Minor")) {
				minorTrigger = true;
			}
		}
		
		int newMajor = previous.getLeft();
		int newMinor = previous.getMiddle();
		int newPatch = previous.getRight();
		
		if(!majorTrigger && !minorTrigger) {
			newPatch++;
		} else if(!majorTrigger) {
			newPatch = 0;
			newMinor++;
		} else {
			newPatch = 0;
			newMinor = 0;
			newMajor++;
		}
		
		return newMajor + "." + newMinor + "." + newPatch;
	}
	
	private static List<String> updateSectionWithNewVersionNumber(List<String> nextSection, String newVersionNumber) {
		List<String> updatedSection = new ArrayList<>();
		for(String line : nextSection) {
			if(line.contains(NEXT_MARKER)) {
				String updatedVersionNumberLine = newVersionNumber + " (" + BUILD_DATE + ")";
				updatedSection.add(updatedVersionNumberLine);
			} else {
				updatedSection.add(line);
			}
		}
		return updatedSection;
	}
	
	private static void replaceReleaseNotesFileContents(
			File targetFile, String originalContentsStr, List<String> modifiedSection) throws IOException {
		String[] originalContents = originalContentsStr.split(LINE_SPLIT_REGEX);
		
		StringBuilder newFileContents = new StringBuilder();
		boolean hasCRLF = originalContentsStr.contains("\r\n");
		for(int i = 0; i < modifiedSection.size(); i++) {
			newFileContents.append(modifiedSection.get(i));
			newFileContents.append(hasCRLF ? "\r\n" : "\n");
		}
		for(int i = modifiedSection.size(); i < originalContents.length; i++) {
			newFileContents.append(originalContents[i]);
			newFileContents.append(hasCRLF ? "\r\n" : "\n");
		}
		FileUtils.write(targetFile, newFileContents.toString(), StandardCharsets.UTF_8, false);
	}
	
	private static void writeManifestFile(File manifestFile, String newVersionNumber) throws IOException {
		if(DRY_RUN) {
			System.out.println("Would update manifest file " + manifestFile.getAbsolutePath());
			return;
		}
		
		String contents = FileUtils.readFileToString(manifestFile, StandardCharsets.UTF_8);
		boolean hasCRLF = contents.contains("\r\n");
		StringBuilder newFileContents = new StringBuilder();
		String[] lines = contents.split(LINE_SPLIT_REGEX);
		for(String line : lines) {
			if(line.startsWith("Bundle-Version:")) {
				String updatedLine = "Bundle-Version: " + newVersionNumber + ".qualifier";
				newFileContents.append(updatedLine);
				newFileContents.append(hasCRLF ? "\r\n" : "\n");
			} else {
				newFileContents.append(line);
				newFileContents.append(hasCRLF ? "\r\n" : "\n");
			}
		}
		
		FileUtils.write(manifestFile, newFileContents.toString(), StandardCharsets.UTF_8, false);
	}
	
	//////////////////////////////
	// PHASE 2: UPDATE FEATURES //
	//////////////////////////////
	
	private static boolean updateFeature(FeatureDelta feature) throws IOException {
		boolean newStuff = haveNewPlugins(feature.plugins);
		newStuff = newStuff || haveNewPlugins(feature.childFeatures);
		boolean containsDeprecatedPlugins = feature.doesFeatureHaveDeprecatedPlugins(DEPRECATED_PLUGINS);
		containsDeprecatedPlugins = containsDeprecatedPlugins || feature.doesFeatureHaveDeprecatedFeatures(DEPRECATED_FEATURES);
		boolean containsUpdatedPlugins = feature.doesHaveUpdatedPlugins(updatedPluginList);
		boolean containsUpdatedFragments = feature.doesHaveUpdatedFragments(updatedPluginList);
		containsUpdatedPlugins = containsUpdatedPlugins || feature.doesHaveUpdatedFeatures(updatedFeatureList);
		
		boolean featureChanged = newStuff || containsDeprecatedPlugins || containsUpdatedPlugins || containsUpdatedFragments;
		
		if(featureChanged) {
			feature.newVersion = determineNewFeatureVersionNumber(
				feature.oldVersion, containsUpdatedPlugins, newStuff, containsDeprecatedPlugins);
			updatedFeatureList.add(feature);
			feature.updatePluginVersionNumbers(updatedPluginList, DEPRECATED_PLUGINS, PLUGIN_BLACKLIST);
			feature.updateChildFeatureVersionNumbers(updatedFeatureList, DEPRECATED_FEATURES);
			String newFeatureFileContents = feature.printFeatureFile();
			File destinationFile = new File(feature.featureFileAbsPath, FEATURE_XML_FILE);
			
			if(DRY_RUN) {
				System.out.println("Would update release notes file " + destinationFile.getAbsolutePath());
				return false;
			} else {
				FileUtils.write(destinationFile, newFeatureFileContents, StandardCharsets.UTF_8, false);
				return true;
			}
		}
		return false;
	}
	
	private static boolean haveNewPlugins(List<PluginDelta> pluginUpdateList) {
		for(PluginDelta plugin : pluginUpdateList) {
			String versionNumber = plugin.newVersion;
			if(versionNumber.equals("0.0.0") && !ArrayUtils.contains(PLUGIN_BLACKLIST, plugin.name)) {
				return true;
			}
		}
		return false;
	}
	
	private static String determineNewFeatureVersionNumber(
			String oldFeatureVersionNumber,
			boolean pluginsUpdated,
			boolean pluginsAdded,
			boolean pluginsDeleted) {
		
		Triple<Integer,Integer,Integer> previous = versionNumberStringIntoTriple(oldFeatureVersionNumber);
		int newMajor = previous.getLeft();
		int newMinor = previous.getMiddle();
		int newPatch = previous.getRight();
		
		if(pluginsUpdated && !pluginsAdded && !pluginsDeleted) {
			newPatch++;
		} else if(pluginsAdded && !pluginsDeleted) {
			newPatch = 0;
			newMinor++;
		} else if(pluginsDeleted) {
			newPatch = 0;
			newMinor = 0;
			newMajor++;
		}
		
		return newMajor + "." + newMinor + "." + newPatch;
	}
	
	private static void writeFeatureReleaseNotes(File featureReleaseNotes, FeatureDelta feature) throws IOException {
		if(DRY_RUN) {
			System.out.println("Would update release notes file " + featureReleaseNotes.getAbsolutePath());
			return;
		}
		
		String contents = FileUtils.readFileToString(featureReleaseNotes, StandardCharsets.UTF_8);
		boolean hasCRLF = contents.contains("\r\n");
		final String ln = hasCRLF ? "\r\n" : "\n";
		final String indent = "   ";
		
		StringBuilder sb = new StringBuilder();
		
		boolean anyChanges = !feature.recordNewPlugins.isEmpty();
		anyChanges = anyChanges || !feature.recordUpdatedPlugins.isEmpty();
		anyChanges = anyChanges || !feature.recordRemovedPlugins.isEmpty();
		anyChanges = anyChanges || !feature.recordNewChildFeatures.isEmpty();
		anyChanges = anyChanges || !feature.recordUpdatedChildFeatures.isEmpty();
		anyChanges = anyChanges || !feature.recordRemovedChildFeatures.isEmpty();
		
		if(anyChanges) {
			sb.append(feature.newVersion).append(" (").append(BUILD_DATE).append(")").append(ln);
	
			if(!feature.recordNewPlugins.isEmpty()) {
				sb.append(indent).append("Plugins added:").append(ln);
				for(PluginDelta plugin : feature.recordNewPlugins) {
					sb.append(indent).append(indent).append(" - ");
					sb.append(plugin.name).append(" (");
					sb.append(plugin.newVersion).append(")");
					sb.append(ln);
				}
			}
			if(!feature.recordUpdatedPlugins.isEmpty()) {
				sb.append(indent).append("Plugin version updates:").append(ln);
				for(PluginDelta plugin : feature.recordUpdatedPlugins) {
					sb.append(indent).append(indent).append(" - ");
					sb.append(plugin.name).append(" (");
					sb.append(plugin.oldVersion).append(" -> ");
					sb.append(plugin.newVersion).append(")");
					sb.append(ln);
				}
			}
			if(!feature.recordRemovedPlugins.isEmpty()) {
				sb.append(indent).append("Plugins removed:").append(ln);
				for(PluginDelta plugin : feature.recordRemovedPlugins) {
					sb.append(indent).append(indent).append(" - ");
					sb.append(plugin.name);
					sb.append(ln);
				}
			}
	
			if(!feature.recordNewChildFeatures.isEmpty()) {
				sb.append(indent).append("Features added:").append(ln);
				for(PluginDelta childFeature : feature.recordNewChildFeatures) {
					sb.append(indent).append(indent).append(" - ");
					sb.append(childFeature.name).append(" (");
					sb.append(childFeature.newVersion).append(")");
					sb.append(ln);
				}
			}
			if(!feature.recordUpdatedChildFeatures.isEmpty()) {
				sb.append(indent).append("Feature version updates:").append(ln);
				for(PluginDelta childFeature : feature.recordUpdatedChildFeatures) {
					sb.append(indent).append(indent).append(" - ");
					sb.append(childFeature.name).append(" (");
					sb.append(childFeature.oldVersion).append(" -> ");
					sb.append(childFeature.newVersion).append(")");
					sb.append(ln);
				}
			}
			if(!feature.recordRemovedChildFeatures.isEmpty()) {
				sb.append(indent).append("Features removed:").append(ln);
				for(PluginDelta childFeature : feature.recordRemovedChildFeatures) {
					sb.append(indent).append(indent).append(" - ");
					sb.append(childFeature.name);
					sb.append(ln);
				}
			}
			sb.append(ln);
			
			System.out.println("Updated release notes file " + featureReleaseNotes.getAbsolutePath());
		}
		String[] lines = contents.split(LINE_SPLIT_REGEX);
		for(String line : lines) {
			sb.append(line).append(ln);
		}
		FileUtils.write(featureReleaseNotes, sb, StandardCharsets.UTF_8, false);
	}
	
	/////////////////////////////
	// PHASE 3: UPDATE PRODUCT //
	/////////////////////////////
	
	private static File getTopLevelPlugin(String startDir) {
		File startDirFile = new File(startDir);
		if(!startDirFile.exists()) {
			throw new IllegalStateException(startDir + " does not exist!");
		}
		File productPlugin = new File(startDirFile, "gov.sandia.dart.dakotaui.app");
		if(!productPlugin.exists()) {
			throw new IllegalStateException(productPlugin + " does not exist!");
		}
		return productPlugin;
	}
	
	private static List<PluginDelta> getTopLevelFeatures(File productPluginFolder) throws IOException {
		List<PluginDelta> features = new ArrayList<>();
		File dakotaUiProductFile = new File(productPluginFolder, "DakotaUI.product");
		String productFileContents = FileUtils.readFileToString(dakotaUiProductFile, StandardCharsets.UTF_8);
		String[] lines = productFileContents.split(LINE_SPLIT_REGEX);
		
		boolean inFeatureSection = false;
		for(String line : lines) {
			if(line.contains("<features>")) {
				inFeatureSection = true;
			}
			if(line.contains("</features>")) {
				inFeatureSection = false;
			}
			
			if(line.contains("<feature ") && inFeatureSection) {
				String pluginId = "";
				String version = "";
				String[] words = line.split("\\s+");
				for(String word : words) {
					if(word.contains("id")) {
						pluginId = word.split("=")[1].replace("\"", "").replace("/>", "");
					}
					if(word.contains("version")) {
						version = word.split("=")[1].replace(".qualifier\"/>", "").replace("\"", "");
					}
				}
				features.add(new PluginDelta(pluginId, version, version));
			}
		}
		return features;
	}
	
	private static void updateTopLevelFeatures(List<PluginDelta> topLevelFeatures) {
		for(PluginDelta feature : topLevelFeatures) {
			for(FeatureDelta updatedFeature : updatedFeatureList) {
				if(updatedFeature.id.equals(feature.name)) {
					feature.newVersion = updatedFeature.newVersion;
					break;
				}
			}
		}
	}
	
	private static void writeProductFile(File productPluginFolder, List<PluginDelta> updatedFeatures) throws IOException {
		StringBuilder sb = new StringBuilder();
		File dakotaUiProductFile = new File(productPluginFolder, "DakotaUI.product");
		String productFileContents = FileUtils.readFileToString(dakotaUiProductFile, StandardCharsets.UTF_8);
		String[] lines = productFileContents.split(LINE_SPLIT_REGEX);
		boolean crlfDetected = productFileContents.contains("\r\n");
		String ln = crlfDetected ? "\r\n" : "\n";
		
		boolean inFeatureSection = false;
		for(String line : lines) {
			if(line.contains("<features>")) {
				inFeatureSection = true;
			}
			if(line.contains("</features>")) {
				inFeatureSection = false;
			}
			
			if(line.contains("<feature ") && inFeatureSection) {
				String[] words = line.split("\\s+");
				boolean found = false;
				for(String word : words) {
					if(word.contains("id")) {
						String pluginId = word.split("=")[1].replace("\"", "");
						for(PluginDelta feature : updatedFeatures) {
							if(feature.name.equals(pluginId)) {
								sb.append("      <feature ");
								sb.append("id=\"").append(pluginId);
								sb.append("\" version=\"").append(feature.newVersion).append(".qualifier\"/>");
								sb.append(ln);
								found = true;
								break;
							}
						}
					}
				}
				if(!found) {
					sb.append(line).append(ln);
				}
			} else {
				sb.append(line).append(ln);
			}
		}
		
		if(DRY_RUN) {
			System.out.println("Would update file " + dakotaUiProductFile.getAbsolutePath());
		} else {
			FileUtils.write(dakotaUiProductFile, sb.toString(), StandardCharsets.UTF_8, false);
			System.out.println("Updated DakotaUI.product. Make sure to manually write top-level release notes for this release.");
		}
	}
	
	private static void writeProductReleaseNotes(File productPluginFolder) throws IOException {
		File releaseNotesFile = new File(productPluginFolder, RELEASENOTES_FILE);
		if(DRY_RUN) {
			System.out.println("Would update file " + releaseNotesFile.getAbsolutePath());
			return;
		}
		
		String contents = FileUtils.readFileToString(releaseNotesFile, StandardCharsets.UTF_8);
		boolean hasCRLF = contents.contains("\r\n");
		final String ln = hasCRLF ? "\r\n" : "\n";
		final String indent = "    ";
		StringBuilder sb = new StringBuilder();
		
		sb.append(GUI_VERSION).append(" (").append(BUILD_DATE).append(")").append(ln);
		sb.append(indent).append("- Feature: !!!TALK ABOUT NEW FEATURES AT A HIGH LEVEL HERE!!!").append(ln);
		sb.append(indent).append("- Bug: !!!TALK ABOUT NEW BUGFIXES AT A HIGH LEVEL HERE!!!").append(ln);
		sb.append(ln);
		
		String[] lines = contents.split(LINE_SPLIT_REGEX);
		for(String line : lines) {
			sb.append(line).append(ln);
		}
		FileUtils.write(releaseNotesFile, sb, StandardCharsets.UTF_8, false);
	}
}
