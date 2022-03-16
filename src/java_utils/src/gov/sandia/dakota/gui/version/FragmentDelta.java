package gov.sandia.dakota.gui.version;

public class FragmentDelta extends PluginDelta {

	public String os = "";
	public String ws = "";
	public String arch = "";
	
	public FragmentDelta(String name, String oldVersion, String newVersion) {
		super(name, oldVersion, newVersion);
	}
	
	@Override
	public boolean equals(Object other) {
		if(other instanceof FragmentDelta) {
			FragmentDelta plugin = (FragmentDelta) other;
			boolean equals = super.equals(other); 
			equals = equals && os.equals(plugin.os);
			equals = equals && ws.equals(plugin.ws);
			equals = equals && arch.equals(plugin.arch);
			return equals;
		}
		return false;
	}
	
	@Override
	public int hashCode() {
		 int hash = super.hashCode();
		 hash = 31 * hash + (os == null ? 0 : os.hashCode());
		 hash = 31 * hash + (ws == null ? 0 : ws.hashCode());
		 hash = 31 * hash + (arch == null ? 0 : arch.hashCode());
		 return hash;
	}
}
