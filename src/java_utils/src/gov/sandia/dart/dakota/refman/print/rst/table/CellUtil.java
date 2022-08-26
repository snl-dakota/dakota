package gov.sandia.dart.dakota.refman.print.rst.table;

public class CellUtil {
	
	public static String pad(int spaces) {
		StringBuilder sb = new StringBuilder();
		for(int i = 0; i < spaces; i++) {
			sb.append(" ");
		}
		return sb.toString();
	}
}
