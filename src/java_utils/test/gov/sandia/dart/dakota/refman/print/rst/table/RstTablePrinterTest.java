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
		                + "+----------+-----------------------------------------+------------------------+\n";
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
		                + "+----------+------------------------------------------------------------------+\n";
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
		                + "+-----------+------------------------------------------------------------------+\n";
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
		                + "+-----------+------------------------------------------------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintDakotaKeywordTable_Example1() {
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
				        + "+------------------------------------------+--------------------------+---------------------------------------------------------------+\n"
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
	
	@Test
	public void testPrintDakotaKeywordTable_Example2() {
		GenericTable table = new GenericTable();
		table.setColumnWidth(0, 25);
		
		GenericRow header = new GenericRow();
		header.addCell("Required/Optional");
		header.addCell("Description of Group");
		header.addCell("Dakota Keyword");
		header.addCell("Dakota Keyword Description");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Required (Choose One)");
		row1.addCell("Importance Sampling Approach");
		row1.addCell("import");
		row1.addCell("Importance sampling option for probability refinement");
		
		GenericRow row2 = new GenericRow();
		row2.addSpanHoldCell();
		row2.addSpanHoldCell();
		row2.addCell("adapt_import");
		row2.addCell("Importance sampling option for probability refinement");
		
		GenericRow row3 = new GenericRow();
		row3.addSpanHoldCell();
		row3.addSpanHoldCell();
		row3.addCell("mm_adapt_import");
		row3.addCell("Importance sampling option for probability refinement");
		
		GenericRow row4 = new GenericRow();
		row4.addCell("Optional", 2, 1);
		row4.addCell("refinement_samples");
		row4.addCell("Number of samples used to refine a probabilty estimate or sampling design.");
		
		table.addRow(header);
		table.addRow(row1);
		table.addRow(row2);
		table.addRow(row3);
		table.addRow(row4);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+-------------------------+------------------------------+--------------------+----------------------------------------------------------------------------+\n"
				        + "| Required/Optional       | Description of Group         | Dakota Keyword     | Dakota Keyword Description                                                 |\n"
				        + "+=========================+==============================+====================+============================================================================+\n"
				        + "| Required (Choose One)   | Importance Sampling Approach | import             | Importance sampling option for probability refinement                      |\n"
				        + "+-------------------------+------------------------------+--------------------+----------------------------------------------------------------------------+\n"
				        + "|                         |                              | adapt_import       | Importance sampling option for probability refinement                      |\n"
				        + "+-------------------------+------------------------------+--------------------+----------------------------------------------------------------------------+\n"
				        + "|                         |                              | mm_adapt_import    | Importance sampling option for probability refinement                      |\n"
				        + "+-------------------------+------------------------------+--------------------+----------------------------------------------------------------------------+\n"
				        + "| Optional                                               | refinement_samples | Number of samples used to refine a probabilty estimate or sampling design. |\n"
				        + "+--------------------------------------------------------+--------------------+----------------------------------------------------------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintDakotaKeywordTable_Example3() {
		GenericTable table = new GenericTable();
		table.setColumnWidth(0, 25);
		
		GenericRow header = new GenericRow();
		header.addCell("Required/Optional");
		header.addCell("Description of Group");
		header.addCell("Dakota Keyword");
		header.addCell("Dakota Keyword Description");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Required (Choose One)", 1, 3);
		row1.addCell("Statistics to Compute", 1, 3);
		row1.addCell("probabilities");
		row1.addCell("Computes probabilities associated with response levels");
		
		GenericRow row2 = new GenericRow();
		row2.addSpanHoldCell();
		row2.addSpanHoldCell();
		row2.addCell("reliabilities");
		row2.addCell("Computes reliabilities associated with response levels");
		
		GenericRow row3 = new GenericRow();
		row3.addSpanHoldCell();
		row3.addSpanHoldCell();
		row3.addCell("gen_reliabilities");
		row3.addCell("Computes generalized reliabilities associated with response levels");
		
		GenericRow row4 = new GenericRow();
		row4.addCell("Optional", 2, 1);
		row4.addCell("system");
		row4.addCell("Compute system reliability (series or parallel)");
		
		table.addRow(header);
		table.addRow(row1);
		table.addRow(row2);
		table.addRow(row3);
		table.addRow(row4);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+-------------------------+----------------------+-------------------+--------------------------------------------------------------------+\n"
						+ "| Required/Optional       | Description of Group | Dakota Keyword    | Dakota Keyword Description                                         |\n"
						+ "+=========================+======================+===================+====================================================================+\n"
						+ "| Required (Choose One)   | Statistics to        | probabilities     | Computes probabilities associated with response levels             |\n"
						+ "|                         | Compute              +-------------------+--------------------------------------------------------------------+\n"
						+ "|                         |                      | reliabilities     | Computes reliabilities associated with response levels             |\n"
						+ "|                         |                      +-------------------+--------------------------------------------------------------------+\n"
						+ "|                         |                      | gen_reliabilities | Computes generalized reliabilities associated with response levels |\n"
						+ "+-------------------------+----------------------+-------------------+--------------------------------------------------------------------+\n"
						+ "| Optional                                       | system            | Compute system reliability (series or parallel)                    |\n"
						+ "+------------------------------------------------+-------------------+--------------------------------------------------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintDakotaKeywordTable_Example3_WithMoreCellWrapping() {
		GenericTable table = new GenericTable();
		table.setColumnWidth(0, 25);
		table.setColumnWidth(1, 30);
		table.setColumnWidth(2, 20);
		table.setColumnWidth(3, 45);
		
		GenericRow header = new GenericRow();
		header.addCell("Required/Optional");
		header.addCell("Description of Group");
		header.addCell("Dakota Keyword");
		header.addCell("Dakota Keyword Description");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Required (Choose One)", 1, 3);
		row1.addCell("Statistics to Compute", 1, 3);
		row1.addCell("probabilities");
		row1.addCell("Computes probabilities associated with response levels");
		
		GenericRow row2 = new GenericRow();
		row2.addSpanHoldCell();
		row2.addSpanHoldCell();
		row2.addCell("reliabilities");
		row2.addCell("Computes reliabilities associated with response levels");
		
		GenericRow row3 = new GenericRow();
		row3.addSpanHoldCell();
		row3.addSpanHoldCell();
		row3.addCell("gen_reliabilities");
		row3.addCell("Computes generalized reliabilities associated with response levels");
		
		GenericRow row4 = new GenericRow();
		row4.addCell("Optional", 2, 1);
		row4.addCell("system");
		row4.addCell("Compute system reliability (series or parallel)");
		
		table.addRow(header);
		table.addRow(row1);
		table.addRow(row2);
		table.addRow(row3);
		table.addRow(row4);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+-------------------------+------------------------------+--------------------+---------------------------------------------+\n"
						+ "| Required/Optional       | Description of Group         | Dakota Keyword     | Dakota Keyword Description                  |\n"
						+ "+=========================+==============================+====================+=============================================+\n"
						+ "| Required (Choose One)   | Statistics to Compute        | probabilities      | Computes probabilities associated with      |\n"
						+ "|                         |                              |                    | response levels                             |\n"
						+ "|                         |                              +--------------------+---------------------------------------------+\n"
						+ "|                         |                              | reliabilities      | Computes reliabilities associated with      |\n"
						+ "|                         |                              |                    | response levels                             |\n"
						+ "|                         |                              +--------------------+---------------------------------------------+\n"
						+ "|                         |                              | gen_reliabilities  | Computes generalized reliabilities          |\n"
						+ "|                         |                              |                    | associated with response levels             |\n"
						+ "+-------------------------+------------------------------+--------------------+---------------------------------------------+\n"
						+ "| Optional                                               | system             | Compute system reliability (series or       |\n"
						+ "|                                                        |                    | parallel)                                   |\n"
						+ "+--------------------------------------------------------+--------------------+---------------------------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintDakotaKeywordTable_BatchSelection() {
		GenericTable table = new GenericTable();
		table.setColumnWidth(0, 25);
		table.setColumnWidth(1, 20);
		table.setColumnWidth(2, 22);
		table.setColumnWidth(3, 45);
		
		GenericRow header = new GenericRow();
		header.addCell("Required/Optional");
		header.addCell("Description of Group");
		header.addCell("Dakota Keyword");
		header.addCell("Dakota Keyword Description");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("Required (Choose One)", 1, 4);
		row1.addCell("Batch Selection Criterion", 1, 4);
		row1.addCell("`naive`__");
		row1.addCell("Take the highest scoring candidates");
		
		GenericRow row2 = new GenericRow();
		row2.addSpanHoldCell();
		row2.addSpanHoldCell();
		row2.addCell("`distance_penalty`__");
		row2.addCell("Add a penalty to spread out the points in the batch");
		
		GenericRow row3 = new GenericRow();
		row3.addSpanHoldCell();
		row3.addSpanHoldCell();
		row3.addCell("`topology`__");
		row3.addCell("In this selection strategy, we use information about the topology of the space from the Morse-Smale complex to identify next points to select.");
		
		GenericRow row4 = new GenericRow();
		row4.addSpanHoldCell();
		row4.addSpanHoldCell();
		row4.addCell("`constant_liar`__");
		row4.addCell("Use information from the existing surrogate model to predict what the surrogate upgrade will be with new points.");
		
		table.addRow(header);
		table.addRow(row1);
		table.addRow(row2);
		table.addRow(row3);
		table.addRow(row4);
		
		RstTablePrinter printer = new RstTablePrinter();
		String actual = printer.print(table);
		String expected = "+-------------------------+--------------------+----------------------+---------------------------------------------+\n"
						+ "| Required/Optional       | Description of     | Dakota Keyword       | Dakota Keyword Description                  |\n"
						+ "|                         | Group              |                      |                                             |\n"
						+ "+=========================+====================+======================+=============================================+\n"
						+ "| Required (Choose One)   | Batch Selection    | `naive`__            | Take the highest scoring candidates         |\n"
						+ "|                         | Criterion          +----------------------+---------------------------------------------+\n"
						+ "|                         |                    | `distance_penalty`__ | Add a penalty to spread out the points in   |\n"
						+ "|                         |                    |                      | the batch                                   |\n"
						+ "|                         |                    +----------------------+---------------------------------------------+\n"
						+ "|                         |                    | `topology`__         | In this selection strategy, we use          |\n"
						+ "|                         |                    |                      | information about the topology of the space |\n"
						+ "|                         |                    |                      | from the Morse-Smale complex to identify    |\n"
						+ "|                         |                    |                      | next points to select.                      |\n"
						+ "|                         |                    +----------------------+---------------------------------------------+\n"
						+ "|                         |                    | `constant_liar`__    | Use information from the existing surrogate |\n"
						+ "|                         |                    |                      | model to predict what the surrogate upgrade |\n"
						+ "|                         |                    |                      | will be with new points.                    |\n"
						+ "+-------------------------+--------------------+----------------------+---------------------------------------------+\n";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintTableInsideTable() {
		RstTablePrinter printer = new RstTablePrinter();
		
		GenericTable innerTable = new GenericTable();
		innerTable.setColumnWidth(0, 10);
		innerTable.setColumnWidth(1, 15);
		innerTable.setColumnWidth(2, 10);
		
		GenericRow header = new GenericRow();
		header.addCell("Test 1");
		header.addCell("Test 2");
		header.addCell("Test 3");
		
		GenericRow row1 = new GenericRow();
		row1.addCell("A");
		row1.addCell("B");
		row1.addCell("C");
		
		innerTable.addRow(header);
		innerTable.addRow(row1);
		String innerTableRendered = printer.print(innerTable);
		int tableWidth = innerTableRendered.indexOf("\n")+2;
		
		GenericTable outerTable = new GenericTable();
		outerTable.setColumnWidth(0, 25);
		outerTable.setColumnWidth(1, 20);
		outerTable.setColumnWidth(2, 22);
		outerTable.setColumnWidth(3, tableWidth);
		
		GenericRow outerHeader = new GenericRow();
		outerHeader.addCell("Required/Optional");
		outerHeader.addCell("Description of Group");
		outerHeader.addCell("Dakota Keyword");
		outerHeader.addCell("Dakota Keyword Description");
		
		GenericRow row2 = new GenericRow();
		row2.addCell("Dummy text 1");
		row2.addCell("Dummy text 2");
		row2.addCell("dummy_text");
		row2.addCell(innerTableRendered);
		
		GenericRow row3 = new GenericRow();
		row3.addCell("Dummy text 3");
		row3.addCell("Dummy text 4");
		row3.addCell("dummy_text_2");
		row3.addCell("This is some more dummy text to make sure we didn't break text wrapping with the nested table.");
		
		outerTable.addRow(outerHeader);
		outerTable.addRow(row2);
		outerTable.addRow(row3);
		
		String actual = printer.print(outerTable);
		String expected = "+-------------------------+--------------------+----------------------+-----------------------------------------+\n"
						+ "| Required/Optional       | Description of     | Dakota Keyword       | Dakota Keyword Description              |\n"
						+ "|                         | Group              |                      |                                         |\n"
						+ "+=========================+====================+======================+=========================================+\n"
						+ "| Dummy text 1            | Dummy text 2       | dummy_text           | +----------+---------------+----------+ |\n"
						+ "|                         |                    |                      | | Test 1   | Test 2        | Test 3   | |\n"
						+ "|                         |                    |                      | +==========+===============+==========+ |\n"
						+ "|                         |                    |                      | | A        | B             | C        | |\n"
						+ "|                         |                    |                      | +----------+---------------+----------+ |\n"
						+ "+-------------------------+--------------------+----------------------+-----------------------------------------+\n"
						+ "| Dummy text 3            | Dummy text 4       | dummy_text_2         | This is some more dummy text to make    |\n"
						+ "|                         |                    |                      | sure we didn't break text wrapping with |\n"
						+ "|                         |                    |                      | the nested table.                       |\n"
						+ "+-------------------------+--------------------+----------------------+-----------------------------------------+\n"
						+ "";
		assertEquals(expected, actual);
	}
	
	@Test
	public void testPrintTableWithKeywordRunoffInDescription() {
		RstTablePrinter printer = new RstTablePrinter();
				
		GenericTable outerTable = new GenericTable();
		outerTable.setColumnWidth(0, 25);
		outerTable.setColumnWidth(1, 20);
		outerTable.setColumnWidth(2, 32);
		outerTable.setColumnWidth(3, 68);
		
		GenericRow outerHeader = new GenericRow();
		outerHeader.addCell("Required/Optional");
		outerHeader.addCell("Description of Group");
		outerHeader.addCell("Dakota Keyword");
		outerHeader.addCell("Dakota Keyword Description");
		
		GenericRow row2 = new GenericRow();
		row2.addCell("Optional", 2, 1);
		row2.addCell("`dimension_preference`__");
		row2.addCell("A set of weights specifying the realtive importance of each uncertain variable (dimension)");
		
		GenericRow row3 = new GenericRow();
		row3.addCell("Optional", 2, 1);
		row3.addCell("`basis_type`__");
		row3.addCell("Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.");
		
		GenericRow row4 = new GenericRow();
		row4.addCell("Required (Choose One)", 1, 2);
		row4.addCell("Required (Choose One)", 1, 2);
		row4.addCell("`collocation_ratio`__");
		row4.addCell("Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.");
		
		GenericRow row5 = new GenericRow();
		row5.addSpanHoldCell();
		row5.addSpanHoldCell();
		row5.addCell("`expansion_samples_sequence`__");
		row5.addCell("Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the "
				+ "``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion. Current multi-stage "
				+ "expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. "
				+ "If adaptive refinement is active, then this sequence specifies the starting point for each level within either "
				+ "an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., "
				+ ":dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`");
		
		GenericRow row6 = new GenericRow();
		row6.addCell("Optional", 2, 1);
		row6.addCell("`import_build_points_file`__");
		row6.addCell("File containing points you wish to use to build a surrogate.");
		
		outerTable.addRow(outerHeader);
		outerTable.addRow(row2);
		outerTable.addRow(row3);
		outerTable.addRow(row4);
		outerTable.addRow(row5);
		outerTable.addRow(row6);
		
		String actual = printer.print(outerTable);
		String expected =
				    "+-------------------------+--------------------+--------------------------------+--------------------------------------------------------------------+\n"
				  + "| Required/Optional       | Description of     | Dakota Keyword                 | Dakota Keyword Description                                         |\n"
				  + "|                         | Group              |                                |                                                                    |\n"
				  + "+=========================+====================+================================+====================================================================+\n"
				  + "| Optional                                     | `dimension_preference`__       | A set of weights specifying the realtive importance of each        |\n"
				  + "|                                              |                                | uncertain variable (dimension)                                     |\n"
				  + "+----------------------------------------------+--------------------------------+--------------------------------------------------------------------+\n"
				  + "| Optional                                     | `basis_type`__                 | Specify the type of basis truncation to be used for a Polynomial   |\n"
				  + "|                                              |                                | Chaos Expansion.                                                   |\n"
				  + "+-------------------------+--------------------+--------------------------------+--------------------------------------------------------------------+\n"
				  + "| Required (Choose One)   | Required (Choose   | `collocation_ratio`__          | Set the number of points used to build a PCE via regression to be  |\n"
				  + "|                         | One)               |                                | proportional to the number of terms in the expansion.              |\n"
				  + "|                         |                    +--------------------------------+--------------------------------------------------------------------+\n"
				  + "|                         |                    | `expansion_samples_sequence`__ | Sequence of expansion samples used in a multi-stage polynomial     |\n"
				  + "|                         |                    |                                | chaos expansion Each level entry of the                            |\n"
				  + "|                         |                    |                                | ``expansion_samples_sequence`` applies to one expansion within a   |\n"
				  + "|                         |                    |                                | multi-stage expansion. Current multi-stage expansions that support |\n"
				  + "|                         |                    |                                | expansion samples sequences include multilevel and multifidelity   |\n"
				  + "|                         |                    |                                | polynomial chaos. If adaptive refinement is active, then this      |\n"
				  + "|                         |                    |                                | sequence specifies the starting point for each level within either |\n"
				  + "|                         |                    |                                | an individual or integrated refinement approach. A corresponding   |\n"
				  + "|                         |                    |                                | scalar specification is documented at, e.g.,                       |\n"
				  + "|                         |                    |                                | :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples` |\n"
				  + "+-------------------------+--------------------+--------------------------------+--------------------------------------------------------------------+\n"
				  + "| Optional                                     | `import_build_points_file`__   | File containing points you wish to use to build a surrogate.       |\n"
				  + "+----------------------------------------------+--------------------------------+--------------------------------------------------------------------+\n";
		assertEquals(expected, actual);
	}
}
