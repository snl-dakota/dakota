package gov.sandia.dart.dakota.refman.print.rst.table;

public	class CellPayload {
	private String thisRowPrint;
	private String remainderToPrint;
	
	public CellPayload(String thisRowPrint, String remainderToPrint) {
		this.thisRowPrint = thisRowPrint;
		this.remainderToPrint = remainderToPrint;
	}
	
	public String getThisRowPrint() {
		return thisRowPrint;
	}
	
	public String getRemainderToPrint() {
		return remainderToPrint;
	}
}