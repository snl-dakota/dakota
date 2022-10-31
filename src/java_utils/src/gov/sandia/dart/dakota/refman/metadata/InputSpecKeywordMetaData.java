package gov.sandia.dart.dakota.refman.metadata;

import java.util.ArrayList;
import java.util.List;

/**
 * 
 * @author Elliott Ridgway
 *
 */
public class InputSpecKeywordMetaData {

	public static final String DEFAULT = "Default";
	public static final String KEYWORD_HIERARCHY = "Keyword_Hierarchy";
	public static final String ALIAS = "Alias";
	public static final String ARGUMENT = "Argument";
	public static final String NAME = "Name";
	
	private String defaultProperty = "";
	private String keywordHierarchy = "";
	private String name = "";
	private String alias = "";
	private String argument = "";
	private List<String> children = new ArrayList<>();
	
	public String getKeywordHierarchy() {
		return keywordHierarchy;
	}
	
	public void setKeywordHierarchy(String keywordHierarchy) {
		this.keywordHierarchy = keywordHierarchy;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getAlias() {
		return alias;
	}

	public void setAlias(String alias) {
		this.alias = alias;
	}

	public String getArgument() {
		return argument;
	}

	public void setArgument(String argument) {
		this.argument = argument;
	}

	public List<String> getChildren() {
		return children;
	}

	public String getDefaultProperty() {
		return defaultProperty;
	}

	public void setDefaultProperty(String defaultProperty) {
		this.defaultProperty = defaultProperty;
	}
	
	public void setByField(String field, String value) {
		if(field.equals(ALIAS)) {
			setAlias(value);
		} else if(field.equals(ARGUMENT)) {
			setArgument(value);
		} else if(field.equals(DEFAULT)) {
			setDefaultProperty(value);
		} else if(field.equals(KEYWORD_HIERARCHY)) {
			setKeywordHierarchy(value);
		} else if(field.equals(NAME)) {
			setName(value);
		}
	}
	
	public boolean fieldAlreadySet(String field) {
		if(field.equals(ALIAS)) {
			return !alias.isBlank();
		} else if(field.equals(ARGUMENT)) {
			return !argument.isBlank();
		} else if(field.equals(DEFAULT)) {
			return !defaultProperty.isBlank();
		} else if(field.equals(KEYWORD_HIERARCHY)) {
			return !keywordHierarchy.isBlank();
		} else if(field.equals(NAME)) {
			return !name.isBlank();
		}
		return false;
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append(KEYWORD_HIERARCHY).append(":: ").append(keywordHierarchy).append("\n");
		sb.append(NAME).append(":: ").append(name).append("\n");
		sb.append(ALIAS).append(":: ").append(alias).append("\n");
		sb.append(ARGUMENT).append(":: ").append(argument).append("\n");
		if(!children.isEmpty()) {
			for(String child : children) {
				sb.append(child).append("\n");				
			}
		}
		sb.append("\n");
		return sb.toString();
	}	
}
