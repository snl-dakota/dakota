package gov.sandia.dart.dakota.refman.print;

import static org.junit.Assert.assertEquals;

import org.junit.Test;

public class DoxygenToRSTConverterTest {

	@Test
	public void testConvert() {
		assertEquals("This is Dakota documentation.", DoxygenToRSTConverter.convert("This is %Dakota documentation."));
		assertEquals("100% of my peers agree with me", DoxygenToRSTConverter.convert("100% of my peers agree with me"));
		assertEquals("*Header*", DoxygenToRSTConverter.convert("<b> Header </b>"));
	}
	
	@Test
	public void testConvert_Lists() {
		String original =
				"My Lists\n"
				+ "\\li List Item 1\n"
				+ "\n"
				+ "\\li List Item 2\n"
				+ "This item unfortunately has a line wrap\n"
				+ "\n"
				+ "End of list";
		String expected =
				"My Lists\n\n"
				+ "- List Item 1\n"
				+ "- List Item 2 This item unfortunately has a line wrap\n"
				+ "\n"
				+ "End of list";
		
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_Monospace() {
		assertEquals("This has a ``monospace`` section",
				DoxygenToRSTConverter.convert("This has a \\c monospace section"));
		
		assertEquals("This has a\n ``monospace`` section with a line break",
				DoxygenToRSTConverter.convert("This has a \\c\n monospace section with a line break"));
		
		assertEquals("This has a ( ``monospace`` ) section with parentheses",
				DoxygenToRSTConverter.convert("This has a (\\c monospace ) section with parentheses"));
		
		assertEquals("method ( ``top_method_pointer`` ).  The output-related keywords address",
				DoxygenToRSTConverter.convert("method (\\c top_method_pointer ).  The output-related keywords address"));
		
		assertEquals("*Caution regarding ``dot_frcg``.* In DOT",
				DoxygenToRSTConverter.convert("<em>Caution regarding \\c dot_frcg.</em> In DOT"));
	}
	
	@Test
	public void testConvert_MonospaceAcrossLines() {
		String original = "The \\c max_iterations, \\c\n"
				+ "convergence_tolerance, and \\c output settings are method independent\n"
				+ "controls";
		String expected = "The ``max_iterations``,\n"
				+ "``convergence_tolerance``, and ``output`` settings are method independent\n"
				+ "controls";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_Ref() {
		assertEquals("This has a :ref:`cool<cool>` link.", DoxygenToRSTConverter.convert("This has a \\ref cool link."));
		assertEquals("This has a :ref:`reference-to-something<reference-to-something>` link.", DoxygenToRSTConverter.convert("This has a \\ref reference-to-something link."));
		assertEquals("This has a\n :ref:`line-break-interruption<line-break-interruption>` link.", DoxygenToRSTConverter.convert("This has a \\ref \n line-break-interruption link."));
	}
	
	@Test
	public void testConvert_ListsAndMonospace() {
		String original =
			"Dakota exports tabular data in one of three formats:\n"
			+ "\\li \\c annotated (default)\n"
			+ "\\li \\c custom_annotated\n"
			+ "\\li \\c freeform";
		
		String expected =
			"Dakota exports tabular data in one of three formats:\n\n"
			+ "- ``annotated`` (default)\n"
			+ "- ``custom_annotated``\n"
			+ "- ``freeform``";
		
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_RemoveComments() {
		String original =
			"Here is regular text I want to keep.\n"
			+ "<!-- This is the start of my comment block\n"
			+ "This is the end of my comment block -->\n"
			+ "Here is more text I want to keep.";
		
		String expected =
			"Here is regular text I want to keep.\n\n"
			+ "Here is more text I want to keep.";
		
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_FullLineMath() {
		String original = "\\f[p = ln\\left(\\frac{QoI_3 - QoI_2}{QoI_2 - QoI_1}\\right)/ln(r)\\f]";
		String expected = "\n.. math:: p = ln\\left(\\frac{QoI_3 - QoI_2}{QoI_2 - QoI_1}\\right)/ln(r)\n";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_InlineMath() {
		String original = "where \\f$r\\f$ is the uniform refinement rate specified by";
		String expected = "where  :math:`r`  is the uniform refinement rate specified by";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_ListOfCode() {
		String original = "- \\c /interfaces/truth/truth_m/";
		String expected = "- ``/interfaces/truth/truth_m/``";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_ListOfCodeWithMath() {
		String original = "- \\c /models/simulation/truth_m/sources/truth \\f$ \\rightarrow \\f$ \\c /interfaces/truth/truth_m/";
		String expected = "- ``/models/simulation/truth_m/sources/truth``  :math:`\\rightarrow`  ``/interfaces/truth/truth_m/``";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_MultilineListEntries() {
		String original = "- In the fork case, Dakota will treat the simulation as a black-box\r\n"
				        + "  and communication between Dakota and the simulation occurs through\r\n"
				        + "  parameter and result files. This is the most common case.";
		String expected = "- In the fork case, Dakota will treat the simulation as a black-box"
		                + "  and communication between Dakota and the simulation occurs through"
		                + "  parameter and result files. This is the most common case.";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
}
