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
	
	@Test
	public void testPrintTableWithTwoMergedColumns() {
		GenericTable table = new GenericTable();
		GenericRow header = new GenericRow();
		header.addCell("Module");
		header.addCell("Learning Goals");
		header.addCell("Approx. Time (minutes)");
		header.addCell("Video/Slides/Exercises");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Overview");
		row1.addCell("This is a merged column cell.", 2, 1);
		row1.addCell("Slides link here");
		
		table.addRow(header);
		table.addRow(row1);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+----------+----------------+------------------------+------------------------+\n"
	                	+ "| Module   | Learning Goals | Approx. Time (minutes) | Video/Slides/Exercises |\n"
		                + "+==========+================+========================+========================+\n"
		                + "| Overview | This is a merged column cell.           | Slides link here       |\n"
		                + "+----------+----------------+------------------------+------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintTableWithThreeMergedColumns() {
		GenericTable table = new GenericTable();
		GenericRow header = new GenericRow();
		header.addCell("Module");
		header.addCell("Learning Goals");
		header.addCell("Approx. Time (minutes)");
		header.addCell("Video/Slides/Exercises");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Overview");
		row1.addCell("This is a hecking big merged column cell that spans 3 columns!", 3, 1);
		
		table.addRow(header);
		table.addRow(row1);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+----------+----------------+------------------------+------------------------+\n"
	                	+ "| Module   | Learning Goals | Approx. Time (minutes) | Video/Slides/Exercises |\n"
		                + "+==========+================+========================+========================+\n"
		                + "| Overview | This is a hecking big merged column cell that spans 3 columns!   |\n"
		                + "+----------+----------------+------------------------+------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintTableWithMergedRows() {
		GenericTable table = new GenericTable();
		GenericRow header = new GenericRow();
		header.addCell("Module");
		header.addCell("Learning Goals");
		header.addCell("Approx. Time (minutes)");
		header.addCell("Video/Slides/Exercises");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Overview");
		row1.addCell("Here is a merged cell that spans rows.", 1, 2);
		row1.addCell("45");
		row1.addCell("Slides link here");
		
		GenericRow row2 = new GenericRow();
		row2.addCell("Interface");
		row2.addSpanHoldCell();
		row2.addCell("90");
		row2.addCell("Slides link here");
		
		table.addRow(header);
		table.addRow(row1);
		table.addRow(row2);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+-----------+----------------+------------------------+------------------------+\n"
	                	+ "| Module    | Learning Goals | Approx. Time (minutes) | Video/Slides/Exercises |\n"
		                + "+===========+================+========================+========================+\n"
		                + "| Overview  | Here is a      | 45                     | Slides link here       |\n"
		                + "+-----------+ merged cell    +------------------------+------------------------+\n"
		                + "| Interface | that spans     | 90                     | Slides link here       |\n"
		                + "|           | rows.          |                        |                        |\n"
		                + "+-----------+----------------+------------------------+------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintTableWithMergedRowsAndColumns() {
		GenericTable table = new GenericTable();
		GenericRow header = new GenericRow();
		header.addCell("Module");
		header.addCell("Learning Goals");
		header.addCell("Approx. Time (minutes)");
		header.addCell("Video/Slides/Exercises");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Overview");
		row1.addCell("Here is a merged cell that spans rows.", 1, 2);
		row1.addCell("Here is a merged cell that spans columns.", 2, 1);
		
		GenericRow row2 = new GenericRow();
		row2.addCell("Interface");
		row2.addSpanHoldCell();
		row2.addCell("90");
		row2.addCell("Slides link here");
		
		table.addRow(header);
		table.addRow(row1);
		table.addRow(row2);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+-----------+----------------+------------------------+------------------------+\n"
	                	+ "| Module    | Learning Goals | Approx. Time (minutes) | Video/Slides/Exercises |\n"
		                + "+===========+================+========================+========================+\n"
		                + "| Overview  | Here is a      | Here is a merged cell that spans columns.       |\n"
		                + "+-----------+ merged cell    +------------------------+------------------------+\n"
		                + "| Interface | that spans     | 90                     | Slides link here       |\n"
		                + "|           | rows.          |                        |                        |\n"
		                + "+-----------+----------------+------------------------+------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintTableWithFreakingBigCell() {
		GenericTable table = new GenericTable();
		GenericRow header = new GenericRow();
		header.addCell("Module");
		header.addCell("Learning Goals");
		header.addCell("Approx. Time (minutes)");
		header.addCell("Video/Slides/Exercises");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Overview");
		row1.addCell("Why do we need such a big cell? Well, a lot of text goes into this cell, you see. There is still a blank row below though.", 3, 2);
		
		GenericRow row2 = new GenericRow();
		row2.addCell("Interface");
		row2.addSpanHoldCell();
		row2.addSpanHoldCell();
		row2.addSpanHoldCell();
		
		table.addRow(header);
		table.addRow(row1);
		table.addRow(row2);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+-----------+----------------+------------------------+------------------------+\n"
	                	+ "| Module    | Learning Goals | Approx. Time (minutes) | Video/Slides/Exercises |\n"
		                + "+===========+================+========================+========================+\n"
		                + "| Overview  | Why do we need such a big cell? Well, a lot of text goes into    |\n"
		                + "+-----------+ this cell, you see. There is still a blank row below though.     +\n"
		                + "| Interface |                                                                  |\n"
		                + "+-----------+----------------+------------------------+------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintTableWithFreakingBigCellAndNoExtraSpace() {
		GenericTable table = new GenericTable();
		GenericRow header = new GenericRow();
		header.addCell("Module");
		header.addCell("Learning Goals");
		header.addCell("Approx. Time (minutes)");
		header.addCell("Video/Slides/Exercises");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Overview");
		row1.addCell("Why do we need such a big cell? Well, a lot of text goes into this cell, you see. There is so much text in this cell, in fact, that there is no space left for blank rows!!", 3, 2);
		
		GenericRow row2 = new GenericRow();
		row2.addCell("Interface");
		row2.addSpanHoldCell();
		row2.addSpanHoldCell();
		row2.addSpanHoldCell();
		
		table.addRow(header);
		table.addRow(row1);
		table.addRow(row2);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+-----------+----------------+------------------------+------------------------+\n"
	                	+ "| Module    | Learning Goals | Approx. Time (minutes) | Video/Slides/Exercises |\n"
		                + "+===========+================+========================+========================+\n"
		                + "| Overview  | Why do we need such a big cell? Well, a lot of text goes into    |\n"
		                + "+-----------+ this cell, you see. There is so much text in this cell, in fact, +\n"
		                + "| Interface | that there is no space left for blank rows!!                     |\n"
		                + "+-----------+----------------+------------------------+------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintDakotaKeywordTable() {
		GenericTable table = new GenericTable();
		GenericRow header = new GenericRow();
		header.addCell("Required/Optional");
		header.addCell("Description of Group");
		header.addCell("Dakota Keyword");
		header.addCell("Dakota Keyword Description");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Optional", 2, 1);
		row1.addCell("id_method");
		row1.addCell("Name the method block; helpful when there are multiple");
		
		GenericRow row2 = new GenericRow();
		row2.addCell("Optional", 2, 1);
		row2.addCell("final_solutions");
		row2.addCell("Number of designs returned as the best solutions");
		
		GenericRow row3 = new GenericRow();
		row3.addCell("Required", 1, 3);
		row3.addCell("Method (Iterative Algorithm) (Group 1)", 1, 3);
		row3.addCell("vector_parameter_study");
		row3.addCell("Samples variables along a user-defined vector");
		
		GenericRow row4 = new GenericRow();
		row4.addSpanHoldCell();
		row4.addSpanHoldCell();
		row4.addCell("list_parameter_study");
		row4.addCell("Samples variables as a specified values");
		
		GenericRow row5 = new GenericRow();
		row5.addSpanHoldCell();
		row5.addSpanHoldCell();
		row5.addCell("centered_parameter_study");
		row5.addCell("Samples variables along points moving out from a center point");
		
		table.addRow(header);
		table.addRow(row1);
		table.addRow(row2);
		table.addRow(row3);
		table.addRow(row4);
		table.addRow(row5);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+-------------------+----------------------+--------------------------+---------------------------------------------------------------+\n"
				        + "| Required/Optional | Description of Group | Dakota Keyword           | Dakota Keyword Description                                    |\n"
				        + "+===================+======================+==========================+===============================================================+\n"
				        + "| Optional                                 | id_method                | Name the method block; helpful when there are multiple        |\n"
				        + "+-------------------+----------------------+--------------------------+---------------------------------------------------------------+\n"
				        + "| Optional                                 | final_solutions          | Number of designs returned as the best solutions              |\n"
				        + "+-------------------+----------------------+--------------------------+---------------------------------------------------------------+\n"
				        + "| Required          | Method (Iterative    | vector_parameter_study   | Samples variables along a user-defined vector                 |\n"
				        + "|                   | Algorithm) (Group 1) +--------------------------+---------------------------------------------------------------+\n"
				        + "|                   |                      | list_parameter_study     | Samples variables as a specified values                       |\n"
				        + "|                   |                      +--------------------------+---------------------------------------------------------------+\n"
				        + "|                   |                      | centered_parameter_study | Samples variables along points moving out from a center point |\n"
				        + "+-------------------+----------------------+--------------------------+---------------------------------------------------------------+\n";
		assertEquals(expected, actual);
	}
}
