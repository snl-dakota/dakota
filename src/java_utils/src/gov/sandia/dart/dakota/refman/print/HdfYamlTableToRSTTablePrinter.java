package gov.sandia.dart.dakota.refman.print;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.StringUtils;
import org.yaml.snakeyaml.Yaml;

import gov.sandia.dart.dakota.refman.print.rst.table.GenericRow;
import gov.sandia.dart.dakota.refman.print.rst.table.GenericTable;
import gov.sandia.dart.dakota.refman.print.rst.table.RstTablePrinter;

public class HdfYamlTableToRSTTablePrinter {

	@SuppressWarnings("unchecked")
	public static void main(String[] args) throws IOException {
		String yamlFileStr = args[0];
		File yamlFile = new File(yamlFileStr);
		
		Yaml yaml = new Yaml();
		String document = FileUtils.readFileToString(yamlFile, StandardCharsets.UTF_8);
		Map<String, ?> map = (Map<String, ?>) yaml.load(document);
		
		GenericRow headerRow = getHeaderFromYamlMap(map);
		List<GenericRow> datasetRows = getDatasetRowsFromYamlMap(map);
		
		GenericTable outerTable = new GenericTable();
		outerTable.setColumnWidth(0, 20);
		outerTable.setColumnWidth(1, 100);
		outerTable.addRow(headerRow);
		for(GenericRow datasetRow : datasetRows) {
			outerTable.addRow(datasetRow);
		}
		
		int innerTableWidth = 60; // some arbitrary default value
		if(map.containsKey("scales")) {
			GenericRow innerTableRow = getScalesInnerTableFromYamlMap(map);
			outerTable.addRow(innerTableRow);
			innerTableWidth = innerTableRow.getData().get(1).getContents().indexOf("\n")+2;
		}
		
		outerTable.setColumnWidth(0, 30);
		outerTable.setColumnWidth(1, innerTableWidth);
		
		RstTablePrinter printer = new RstTablePrinter();
		System.out.println(printer.print(outerTable));
	}

	private static GenericRow getHeaderFromYamlMap(Map<String, ?> map) {
		String value = (String) map.get("name");
		GenericRow row = new GenericRow();
		row.addSpanHoldCell();
		row.addCell(value);
		return row;
	}
	
	@SuppressWarnings("unchecked")
	private static List<GenericRow> getDatasetRowsFromYamlMap(Map<String, ?> map) {
		Map<String, String> valueMap = (Map<String, String>) map.get("dataset");
		
		List<GenericRow> rows = new ArrayList<>();
		for(Entry<String, String> entry : valueMap.entrySet()) {
			GenericRow row = new GenericRow();
			row.addCell(StringUtils.capitalize(entry.getKey()));
			row.addCell(entry.getValue());
			rows.add(row);
		}
		
		return rows;
	}
	
	@SuppressWarnings("unchecked")
	private static GenericRow getScalesInnerTableFromYamlMap(Map<String, ?> map) {
		GenericTable scalesTable = new GenericTable();
		List<?> valueList = (List<?>) map.get("scales");
		
		GenericRow headerRow = getLongestHeadersFromValueMap(valueList);
		scalesTable.addRow(headerRow);
		for(Object element : valueList) {
			Map<String,String> innerMap = (Map<String,String>) element;
			scalesTable.addRow(getScaleRow(innerMap, headerRow.getData().size()));
		}
		
		RstTablePrinter printer = new RstTablePrinter();
		String renderedTable = printer.print(scalesTable);
		
		GenericRow scalesRow = new GenericRow();
		scalesRow.addCell("Scales");
		scalesRow.addCell(renderedTable);
		return scalesRow;
	}

	@SuppressWarnings("unchecked")
	private static GenericRow getLongestHeadersFromValueMap(List<?> valueList) {
		Iterator<Object> iter = (Iterator<Object>) valueList.iterator();
		GenericRow longestRow = new GenericRow();
		
		while(iter.hasNext()) {
			Map<String,String> element = (Map<String,String>) iter.next();
			GenericRow testRow = new GenericRow();
			for(Entry<String,String> entry : element.entrySet()) {
				testRow.addCell(StringUtils.capitalize(entry.getKey()));
			}
			if(testRow.getData().size() > longestRow.getData().size()) {
				longestRow = testRow;
			}
		}
		
		return longestRow;
	}
	
	private static GenericRow getScaleRow(Map<String,String> valueMap, int expectedLength) {
		GenericRow row = new GenericRow();
		int actualLength = 0;
		for(Entry<String,?> entry : valueMap.entrySet()) {
			Object value = entry.getValue();
			if(value instanceof String) {
				row.addCell((String)value);
			} else if(value instanceof Integer) {
				row.addCell(((Integer)value).toString());
			} else if(value instanceof Boolean) {
				row.addCell(((Boolean)value).toString());
			} else {
				row.addCell("");
			}
			actualLength++;
		}
		
		while(actualLength < expectedLength) {
			row.addCell("");
			actualLength++;
		}
		
		return row;
	}
}
