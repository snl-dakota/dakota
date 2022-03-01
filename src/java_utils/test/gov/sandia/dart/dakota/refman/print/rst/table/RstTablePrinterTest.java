package gov.sandia.dart.dakota.refman.print.rst.table;

import static org.junit.Assert.assertEquals;

import org.junit.Test;

public class RstTablePrinterTest {

	@Test
	public void testPrintHeader() {
		GenericTable table = new GenericTable();
		GenericRow row = new GenericRow();
		row.addCell("Module");
		row.addCell("Learning Goals");
		row.addCell("Approx. Time (minutes)");
		row.addCell("Video/Slides/Exercises");
		
		table.addRow(row);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+--------+----------------+------------------------+------------------------+\n"
				        + "| Module | Learning Goals | Approx. Time (minutes) | Video/Slides/Exercises |\n"
				        + "+========+================+========================+========================+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintTableWithOneRow() {
		GenericTable table = new GenericTable();
		GenericRow header = new GenericRow();
		header.addCell("Module");
		header.addCell("Learning Goals");
		header.addCell("Approx. Time (minutes)");
		header.addCell("Video/Slides/Exercises");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Overview");
		row1.addCell("- What is Dakota?");
		row1.addCell("45");
		row1.addCell("Slides link here");
		
		table.addRow(header);
		table.addRow(row1);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+----------+-------------------+------------------------+------------------------+\n"
	                	+ "| Module   | Learning Goals    | Approx. Time (minutes) | Video/Slides/Exercises |\n"
		                + "+==========+===================+========================+========================+\n"
		                + "| Overview | - What is Dakota? | 45                     | Slides link here       |\n"
		                + "+----------+-------------------+------------------------+------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintTableWithTwoRows() {
		GenericTable table = new GenericTable();
		GenericRow header = new GenericRow();
		header.addCell("Module");
		header.addCell("Learning Goals");
		header.addCell("Approx. Time (minutes)");
		header.addCell("Video/Slides/Exercises");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Overview");
		row1.addCell("- What is Dakota?");
		row1.addCell("45");
		row1.addCell("Slides link here");
		
		GenericRow row2 = new GenericRow();
		row2.addCell("This row has some really long");
		row2.addCell("content in it. This table would");
		row2.addCell("be really obnoxious to read and");
		row2.addCell("it would run off the edge of most user's screens.");
		
		table.addRow(header);
		table.addRow(row1);
		table.addRow(row2);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+-------------------------------+---------------------------------+---------------------------------+---------------------------------------------------+\n"
				        + "| Module                        | Learning Goals                  | Approx. Time (minutes)          | Video/Slides/Exercises                            |\n"
				        + "+===============================+=================================+=================================+===================================================+\n"
				        + "| Overview                      | - What is Dakota?               | 45                              | Slides link here                                  |\n"
				        + "+-------------------------------+---------------------------------+---------------------------------+---------------------------------------------------+\n"
				        + "| This row has some really long | content in it. This table would | be really obnoxious to read and | it would run off the edge of most user's screens. |\n"
				        + "+-------------------------------+---------------------------------+---------------------------------+---------------------------------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintTableWithTwoRowsAndTextWrapping() {
		GenericTable table = new GenericTable();
		GenericRow header = new GenericRow();
		header.addCell("Module");
		header.addCell("Learning Goals");
		header.addCell("Approx. Time (minutes)");
		header.addCell("Video/Slides/Exercises");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Overview");
		row1.addCell("- What is Dakota?");
		row1.addCell("45");
		row1.addCell("Slides link here");
		
		GenericRow row2 = new GenericRow();
		row2.addCell("This row has some really long");
		row2.addCell("content in it. This table would");
		row2.addCell("be really obnoxious to read and");
		row2.addCell("it would run off the edge of most user's screens.");
		
		table.addRow(header);
		table.addRow(row1);
		table.addRow(row2);
		table.setColumnWidth(0, 20);
		table.setColumnWidth(1, 10);
		table.setColumnWidth(2, 15);
		table.setColumnWidth(3, 25);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+--------------------+----------+---------------+-------------------------+\n"
				        + "| Module             | Learning | Approx. Time  | Video/Slides/Exercises  |\n"
				        + "|                    | Goals    | (minutes)     |                         |\n"
				        + "+====================+==========+===============+=========================+\n"
				        + "| Overview           | - What   | 45            | Slides link here        |\n"
				        + "|                    | is       |               |                         |\n"
				        + "|                    | Dakota?  |               |                         |\n"
				        + "+--------------------+----------+---------------+-------------------------+\n"
				        + "| This row has some  | content  | be really     | it would run off the    |\n"
				        + "| really long        | in it.   | obnoxious to  | edge of most user's     |\n"
				        + "|                    | This     | read and      | screens.                |\n"
				        + "|                    | table    |               |                         |\n"
				        + "|                    | would    |               |                         |\n"
				        + "+--------------------+----------+---------------+-------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintSplitLongWords() {
		GenericTable table = new GenericTable();
		GenericRow header = new GenericRow();
		header.addCell("Module");
		header.addCell("Learning Goals");
		header.addCell("Approx. Time (minutes)");
		header.addCell("Video/Slides/Exercises");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Antidisestablishmentarianism");
		row1.addCell("Supercalifragilisticexpialidocious");
		row1.addCell("45");
		row1.addCell("IDK here are some more words");
		
		table.addRow(header);
		table.addRow(row1);
		table.setColumnWidth(0, 20);
		table.setColumnWidth(1, 10);
		table.setColumnWidth(2, 15);
		table.setColumnWidth(3, 25);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+--------------------+----------+---------------+-------------------------+\n"
				        + "| Module             | Learning | Approx. Time  | Video/Slides/Exercises  |\n"
				        + "|                    | Goals    | (minutes)     |                         |\n"
				        + "+====================+==========+===============+=========================+\n"
				        + "| Antidisestablishme | Supercal | 45            | IDK here are some more  |\n"
				        + "| ntarianism         | ifragili |               | words                   |\n"
				        + "|                    | sticexpi |               |                         |\n"
				        + "|                    | alidocio |               |                         |\n"
				        + "|                    | us       |               |                         |\n"
				        + "+--------------------+----------+---------------+-------------------------+\n";
		assertEquals(expected, actual);
	}	
}
