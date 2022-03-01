package gov.sandia.dart.dakota.refman.print.rst.table;

import java.util.ArrayList;
import java.util.List;

public class GenericRow {

	private List<GenericCell> data = new ArrayList<>();
	
	public List<GenericCell> getData() {
		return data;
	}
	
	public List<String> getDataStrings() {
		List<String> dataStrings = new ArrayList<>();
		for(GenericCell cell : data) {
			dataStrings.add(cell.getContents());
		}
		return dataStrings;
	}
	
	public int getRowHeight() {
		return -1;
	}
	
	public void addCell(String cellContents) {
		GenericCell newCell = new GenericCell(cellContents);
		data.add(newCell);
	}
}
