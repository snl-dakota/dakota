package gov.sandia.dart.dakota.refman.print;

import java.util.ArrayList;
import java.util.List;

public class TableKeyword {

	private final String keyword;
	private final boolean optional;
	private final boolean required;
	private String groupName;
	private String description;
	private List<TableKeyword> oneOf;
	
	public TableKeyword(String keyword, String description, boolean optional, boolean required) {
		this.keyword = keyword;
		this.description = description;
		this.optional = optional;
		this.required = required;
		this.oneOf = new ArrayList<>();
	}
	
	public String getKeyword() {
		return keyword;
	}
	
	public String getDescription() {
		return description;
	}
	
	public boolean getOptional() {
		return optional;
	}
	
	public boolean getRequired() {
		return required;
	}
	
	public String getOptionalOrRequiredGroupName() {
		return groupName;
	}
	
	public void setOptionalOrRequiredGroupName(String groupName) {
		this.groupName = groupName;
	}
	
	public List<TableKeyword> getOneOf() {
		return oneOf;
	}
}
