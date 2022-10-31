package gov.sandia.dart.dakota.refman.print.rst.table;

import static org.junit.Assert.assertEquals;

import org.junit.Test;

public class GenericRowTest {

	@Test
	public void testGetAdjustedHorizontalSpanIndex_FirstCell() {
		GenericRow row = new GenericRow();
		row.addCell(new GenericCell("Test"));
		
		assertEquals(0, row.getAdjustedHorizontalSpanIndex(0));
	}
	
	@Test
	public void testGetAdjustedHorizontalSpanIndex_SecondCell() {
		GenericRow row = new GenericRow();
		row.addCell(new GenericCell("Test"));
		row.addCell(new GenericCell("Test2"));
		
		assertEquals(0, row.getAdjustedHorizontalSpanIndex(0));
		assertEquals(1, row.getAdjustedHorizontalSpanIndex(1));
	}
	
	@Test
	public void testGetAdjustedHorizontalSpanIndex_FirstSpanningCell() {
		GenericRow row = new GenericRow();
		row.addCell(new GenericCell("SpanTest", 2, 1));
		
		assertEquals(0, row.getAdjustedHorizontalSpanIndex(0));
		assertEquals(0, row.getAdjustedHorizontalSpanIndex(1));
	}
	
	@Test
	public void testGetAdjustedHorizontalSpanIndex_TwoSpanningCells() {
		GenericRow row = new GenericRow();
		row.addCell(new GenericCell("SpanTest", 2, 1));
		row.addCell(new GenericCell("SpanTest", 2, 1));
		
		assertEquals(0, row.getAdjustedHorizontalSpanIndex(0));
		assertEquals(0, row.getAdjustedHorizontalSpanIndex(1));
		assertEquals(1, row.getAdjustedHorizontalSpanIndex(2));
		assertEquals(1, row.getAdjustedHorizontalSpanIndex(3));
	}
	
	@Test
	public void testGetAdjustedHorizontalSpanIndex_ThreeCellsWithSpans() {
		GenericRow row = new GenericRow();
		row.addCell(new GenericCell("SpanTest", 2, 1));
		row.addCell(new GenericCell("Test", 1, 1));
		row.addCell(new GenericCell("SpanTest2", 2, 1));
		
		assertEquals(0, row.getAdjustedHorizontalSpanIndex(0));
		assertEquals(0, row.getAdjustedHorizontalSpanIndex(1));
		assertEquals(1, row.getAdjustedHorizontalSpanIndex(2));
		assertEquals(2, row.getAdjustedHorizontalSpanIndex(3));
		assertEquals(2, row.getAdjustedHorizontalSpanIndex(4));
	}
	
	@Test
	public void testGetAdjustedHorizontalSpanIndex_SpanningThree() {
		GenericRow row = new GenericRow();
		row.addCell(new GenericCell("Test", 1, 1));
		row.addCell(new GenericCell("SpanTest", 3, 1));
		
		assertEquals(0, row.getAdjustedHorizontalSpanIndex(0));
		assertEquals(1, row.getAdjustedHorizontalSpanIndex(1));
		assertEquals(1, row.getAdjustedHorizontalSpanIndex(2));
		assertEquals(1, row.getAdjustedHorizontalSpanIndex(3));
	}
}
