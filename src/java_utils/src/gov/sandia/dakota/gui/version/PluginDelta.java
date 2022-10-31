package gov.sandia.dakota.gui.version;

public class PluginDelta {

	public String name = "";
	public String oldVersion = "";
	public String newVersion = "";
	public boolean useQualifier = true;
	
	public PluginDelta(String name, String oldVersion, String newVersion) {
		this.name = name;
		this.oldVersion = oldVersion;
		this.newVersion = newVersion;
	}
	
	@Override
	public boolean equals(Object other) {
		if(other instanceof PluginDelta) {
			PluginDelta plugin = (PluginDelta) other;
			boolean equals = name.equals(plugin.name);
			equals = equals && oldVersion.equals(plugin.oldVersion);
			equals = equals && newVersion.equals(plugin.newVersion);
			return equals;
		}
		return false;
	}
	
	@Override
	public int hashCode() {
		 int hash = 7;
		 hash = 31 * hash + (name == null ? 0 : name.hashCode());
		 hash = 31 * hash + (oldVersion == null ? 0 : oldVersion.hashCode());
		 hash = 31 * hash + (newVersion == null ? 0 : newVersion.hashCode());
		 return hash;
	}
}
