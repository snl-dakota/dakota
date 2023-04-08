package gov.sandia.dakota.gui.version;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.ArrayUtils;

public class FeatureDelta implements Comparable<FeatureDelta> {
	
	////////////
	// FIELDS //
	////////////

	public final String featureFileAbsPath;
	
	public String id = "";
	public String label = "";
	public String provider = "";
	public String oldVersion = "0.0.0";
	public String newVersion = "0.0.0";
	
	public List<PluginDelta> plugins = new ArrayList<>();
	public List<PluginDelta> childFeatures = new ArrayList<>(); // Child features are treated as plugin objects
	public List<FragmentDelta> fragments = new ArrayList<>();
	
	public List<PluginDelta> recordNewPlugins = new ArrayList<>();
	public List<PluginDelta> recordUpdatedPlugins = new ArrayList<>();
	public List<PluginDelta> recordRemovedPlugins = new ArrayList<>();
	
	public List<PluginDelta> recordNewChildFeatures = new ArrayList<>();
	public List<PluginDelta> recordUpdatedChildFeatures = new ArrayList<>();
	public List<PluginDelta>  recordRemovedChildFeatures = new ArrayList<>();
	
	public int level = 0;
	public boolean crlfDetected = false;
	
	public Map<String, Boolean> unpackMap = new HashMap<>();
	
	/////////////////
	// CONSTRUCTOR //
	/////////////////
	
	public FeatureDelta(File featureFile) throws IOException {
		this.featureFileAbsPath = featureFile.getAbsolutePath();
		File featureXmlFile = new File(featureFile, "feature.xml");
		String featureContents = FileUtils.readFileToString(featureXmlFile, StandardCharsets.UTF_8);
		crlfDetected = featureContents.contains("\r\n");
		String[] lines = featureContents.split(BatchPluginVersionUpdater.LINE_SPLIT_REGEX);
		
		boolean inParentFeatureSection = false;
		boolean inPluginSection = false;
		boolean inChildFeatureSection = false;
		String currentId = "";
		
		boolean isFragment = false;
		String fragmentOS = "";
		String fragmentWS = "";
		String fragmentArch = "";
		
		for(String line : lines) {
			if(line.startsWith("<feature")) {
				inParentFeatureSection = true;
			}
			if(line.contains("<plugin")) {
				inPluginSection = true;
			}
			if(line.contains("<includes")) {
				inChildFeatureSection = true;
			}
			
			if(inParentFeatureSection) {
				if(line.contains("id=")) {
					id = line.split("=")[1].replace("\"", "");
				} else if(line.contains("label=")) {
					label = line.split("=")[1].replace("\"", "");
				} else if(line.contains("provider=")) {
					label = provider.split("=")[1].replace("\"", "");
				} else if(line.contains("version=")) {
					String versionWithQualifier = line.split("=")[1];
					oldVersion = versionWithQualifier.split("\\.qualifier")[0].replace("\"", "");
					inParentFeatureSection = false;
				}
			}
			
			if(inPluginSection || inChildFeatureSection) {
				if(line.contains("id=")) {
					currentId = line.split("=")[1].replace("\"", "");
				} else if(line.contains("fragment=")) {
					isFragment = line.contains("true");
				} else if(line.contains("os=")) {
					fragmentOS = line.split("=")[1].replace("\"", "");
				} else if(line.contains("ws=")) {
					fragmentWS = line.split("=")[1].replace("\"", "");
				} else if(line.contains("arch=")) {
					fragmentArch = line.split("=")[1].replace("\"", "");
				} else if(line.contains("unpack=") && !currentId.isBlank()) {
					unpackMap.put(currentId, line.contains("true"));
				} else if(line.contains("version=") && !currentId.isBlank()) {
					String versionWithQualifier = line.split("=")[1];
					String versionNumber = "";
					boolean useQualifier = false;
					if(versionWithQualifier.contains(".qualifier")) {
						versionNumber = versionWithQualifier.split("\\.qualifier")[0].replace("\"", "");
						useQualifier = true;
					} else if(versionWithQualifier.endsWith("/>")) {
						versionNumber = versionWithQualifier.split("/>")[0].replace("\"", "");
					} else {
						versionNumber = versionWithQualifier.replace("\"", "");
					}
					
					PluginDelta originalPlugin = null;
					FragmentDelta originalFragment = null;
					if(isFragment) {
						originalFragment = new FragmentDelta(currentId, versionNumber, versionNumber);
						originalFragment.os = fragmentOS;
						originalFragment.ws = fragmentWS;
						originalFragment.arch = fragmentArch;
						originalFragment.useQualifier = useQualifier;
					} else {
						originalPlugin = new PluginDelta(currentId, versionNumber, versionNumber);
						originalPlugin.useQualifier = useQualifier;
					}
					
					if(inPluginSection) {
						if(isFragment && originalFragment != null) fragments.add(originalFragment);
						else if(originalPlugin != null) plugins.add(originalPlugin);
					} else if(inChildFeatureSection) {
						childFeatures.add(originalPlugin);
					}
				}
			}
			
			if(line.endsWith("/>")) {
				inPluginSection = false;
				inChildFeatureSection = false;
				isFragment = false;
			}
		}
	}
	
	/////////////
	// UTILITY //
	/////////////
	
	public boolean doesHaveUpdatedPlugins(Collection<PluginDelta> updatedPlugins) {
		for(PluginDelta plugin : plugins) {
			for(PluginDelta updatedPlugin : updatedPlugins) {
				if(plugin.name.equals(updatedPlugin.name)) {
					return true;
				}
			}
		}
		return false;
	}
	
	public boolean doesHaveUpdatedFragments(Collection<PluginDelta> updatedPlugins) {
		for(FragmentDelta fragment : fragments) {
			for(PluginDelta updatedPlugin : updatedPlugins) {
				if(updatedPlugin instanceof FragmentDelta && fragment.name.equals(((FragmentDelta)updatedPlugin).name)) {
					return true;
				}
			}
		}
		return false;
	}
	
	public boolean doesHaveUpdatedFeatures(Collection<FeatureDelta> updatedFeatures) {
		for(PluginDelta feature : childFeatures) {
			for(FeatureDelta updatedFeature : updatedFeatures) {
				if(feature.name.equals(updatedFeature.id)) {
					return true;
				}
			}
		}
		return false;
	}
	
	public boolean doesFeatureHaveDeprecatedPlugins(String[] deprecatedNames) {
		for(PluginDelta plugin : plugins) {
			if(ArrayUtils.contains(deprecatedNames, plugin.name)) {
				return true;
			}
		}
		return false;
	}
	
	public boolean doesFeatureHaveDeprecatedFeatures(String[] deprecatedNames) {
		for(PluginDelta feature : childFeatures) {
			if(ArrayUtils.contains(deprecatedNames, feature.name)) {
				return true;
			}
		}
		return false;
	}
	
	public void updatePluginVersionNumbers(Collection<PluginDelta> updatedPlugins, String[] deprecatedNames, String[] ignoreNewNames) {
		// First, remove deprecated plugins.
		for(Iterator<PluginDelta> iter = plugins.iterator(); iter.hasNext();) {
			PluginDelta nextPlugin = iter.next();
			if(ArrayUtils.contains(deprecatedNames, nextPlugin.name)) {
				recordRemovedPlugins.add(nextPlugin);
				iter.remove();
			}
		}
		
		// Now update old plugins.
		for(PluginDelta updatedPlugin : updatedPlugins) {
			for(PluginDelta featurePlugin : plugins) {
				if(featurePlugin.name.equals(updatedPlugin.name) &&
				   !featurePlugin.newVersion.equals(updatedPlugin.newVersion)) {
					featurePlugin.newVersion = updatedPlugin.newVersion;
					recordUpdatedPlugins.add(featurePlugin);
					break;
				}
			}
		}
		
		// Record new plugins.
		for(PluginDelta plugin : plugins) {
			if(plugin.newVersion.equals("0.0.0") && !ArrayUtils.contains(ignoreNewNames, plugin.name)) {
				recordNewPlugins.add(plugin);
			}
		}
	}
	
	public void updateChildFeatureVersionNumbers(Collection<FeatureDelta> updatedFeatures, String[] deprecatedNames) {
		// First, remove deprecated features.
		for(Iterator<PluginDelta> iter = childFeatures.iterator(); iter.hasNext();) {
			PluginDelta nextPlugin = iter.next();
			if(ArrayUtils.contains(deprecatedNames, nextPlugin.name)) {
				recordRemovedChildFeatures.add(nextPlugin);
				iter.remove();
			}
		}
		// Now update old features.
		for(FeatureDelta updatedFeature : updatedFeatures) {
			for(PluginDelta childFeature : childFeatures) {
				if(childFeature.name.equals(updatedFeature.id) &&
				   !childFeature.newVersion.equals(updatedFeature.newVersion)) {
					recordUpdatedChildFeatures.add(childFeature);
					childFeature.newVersion = updatedFeature.newVersion;
					break;
				}
			}
		}
		// Record new features.
		for(PluginDelta feature : childFeatures) {
			if(feature.newVersion.equals("0.0.0")) {
				recordNewChildFeatures.add(feature);
			}
		}
	}
	
	public void updateChildFeatureVersionNumbers(List<FeatureDelta> updatedChildFeatures) {
		for(FeatureDelta updatedFeature : updatedChildFeatures) {
			for(PluginDelta childFeature : childFeatures) {
				if(childFeature.name.equals(updatedFeature.id) &&
				   !childFeature.newVersion.equals(updatedFeature.newVersion)) {
					childFeature.newVersion = updatedFeature.newVersion;
				}
			}
		}
	}
	
	public int getLevel(List<FeatureDelta> allFeatures, int depthSoFar) {
		if(level != 0) {
			return level;
		} else if(childFeatures.isEmpty()) {
			return depthSoFar; // Base case
		} else {
			int maxDepth = depthSoFar;
			for(PluginDelta childFeature : childFeatures) {
				FeatureDelta feature = getFeatureForChildFeatureReference(allFeatures, childFeature);
				if(feature != null) {
					int featureDepth = depthSoFar + feature.getLevel(allFeatures, depthSoFar);
					maxDepth = Math.max(maxDepth, featureDepth);
				}
			}
			return maxDepth;
		}
	}
	
	private FeatureDelta getFeatureForChildFeatureReference(List<FeatureDelta> allFeatures, PluginDelta childFeature) {
		for(FeatureDelta feature : allFeatures) {
			if(feature.id.equals(childFeature.name)) {
				return feature;
			}
		}
		return null;
	}
	
	public String printFeatureFile() {
		final String INDENT = "   ";
		final String ln = crlfDetected ? "\r\n" : "\n";
		StringBuilder sb = new StringBuilder();
		
		sb.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>").append(ln);
		sb.append("<feature").append(ln);
		sb.append(INDENT).append(INDENT).append("id=\"").append(id).append("\"").append(ln);
		sb.append(INDENT).append(INDENT).append("label=\"").append(label).append("\"").append(ln);
		sb.append(INDENT).append(INDENT).append("version=\"").append(newVersion).append(".qualifier\"").append(ln);
		sb.append(INDENT).append(INDENT).append("provider-name=\"Sandia National Laboratories\">").append(ln);
		sb.append(ln).append(ln);
		
		sb.append(INDENT).append("<description url=\"http://www.example.com/description\">").append(ln);
		sb.append(INDENT).append(INDENT).append("[Enter Feature Description here.]").append(ln);
		sb.append(INDENT).append("</description>").append(ln);
		sb.append(ln);
		
		sb.append(INDENT).append("<copyright url=\"http://www.example.com/copyright\">").append(ln);
		sb.append(INDENT).append(INDENT).append("[Enter Copyright Description here.]").append(ln);
		sb.append(INDENT).append("</copyright>").append(ln);
		sb.append(ln);
		
		sb.append(INDENT).append("<license url=\"http://www.example.com/license\">").append(ln);
		sb.append(INDENT).append(INDENT).append("[Enter License Description here.]").append(ln);
		sb.append(INDENT).append("</license>").append(ln);
		sb.append(ln);
		
		for(PluginDelta childFeature : childFeatures) {
			sb.append(INDENT).append("<includes").append(ln);
			sb.append(INDENT).append(INDENT).append(INDENT).append("id=\"").append(childFeature.name).append("\"").append(ln);
			sb.append(INDENT).append(INDENT).append(INDENT).append("version=\"").append(childFeature.newVersion);
			sb.append(childFeature.useQualifier ? ".qualifier\"/>" : "\"/>");
			sb.append(ln).append(ln);
		}
		
		for(PluginDelta plugin : plugins) {
			sb.append(INDENT).append("<plugin").append(ln);
			sb.append(INDENT).append(INDENT).append(INDENT).append("id=\"").append(plugin.name).append("\"").append(ln);
			sb.append(INDENT).append(INDENT).append(INDENT).append("download-size=\"0\"").append(ln);
			sb.append(INDENT).append(INDENT).append(INDENT).append("install-size=\"0\"").append(ln);
			
			if(plugin instanceof FragmentDelta) {
				FragmentDelta fragment = (FragmentDelta) plugin;
				sb.append(INDENT).append(INDENT).append(INDENT).append("fragment=\"true\"").append(ln);
				sb.append(INDENT).append(INDENT).append(INDENT).append("os=\"").append(fragment.os).append("\"").append(ln);
				sb.append(INDENT).append(INDENT).append(INDENT).append("ws=\"").append(fragment.ws).append("\"").append(ln);
				sb.append(INDENT).append(INDENT).append(INDENT).append("arch=\"").append(fragment.arch).append("\"").append(ln);
			}
			
			sb.append(INDENT).append(INDENT).append(INDENT).append("version=\"").append(plugin.newVersion);
			sb.append(plugin.useQualifier ? ".qualifier\"" : "\"");
			if(unpackMap.containsKey(plugin.name)) {
				sb.append(ln);
				sb.append(INDENT).append(INDENT).append(INDENT).append("unpack=\"").append(Boolean.toString(unpackMap.get(plugin.name))).append("\"");
			}
			sb.append("/>").append(ln).append(ln);
		}
		
		sb.append("</feature>").append(ln);
		return sb.toString();
	}
	
	//////////////
	// OVERRIDE //
	//////////////

	@Override
	public int compareTo(FeatureDelta f) {
		return Integer.compare(level, f.level);
	}
	
	@Override
	public boolean equals(Object other) {
		if(other instanceof FeatureDelta) {
			FeatureDelta otherFeature = (FeatureDelta) other;
			boolean equals = id.equals(otherFeature.id);
			equals = equals && label.equals(otherFeature.label);
			equals = equals && provider.equals(otherFeature.provider);
			equals = equals && oldVersion.equals(otherFeature.oldVersion);
			equals = equals && newVersion.equals(otherFeature.newVersion);
			equals = equals && level == otherFeature.level;
			equals = equals && plugins.equals(otherFeature.plugins);
			equals = equals && childFeatures.equals(otherFeature.childFeatures);
			return equals;
		}
		return false;
	}
	
	@Override
	public int hashCode() {
		 int hash = 7;
		 hash = 31 * hash + (id == null ? 0 : id.hashCode());
		 hash = 31 * hash + (label == null ? 0 : label.hashCode());
		 hash = 31 * hash + (provider == null ? 0 : provider.hashCode());
		 hash = 31 * hash + (oldVersion == null ? 0 : oldVersion.hashCode());
		 hash = 31 * hash + (newVersion == null ? 0 : newVersion.hashCode());
		 hash = 31 * hash + Integer.hashCode(level);
		 hash = 31 * hash + (plugins == null ? 0 : plugins.hashCode());
		 hash = 31 * hash + (childFeatures == null ? 0 : childFeatures.hashCode());
		 return hash;
	}
}
