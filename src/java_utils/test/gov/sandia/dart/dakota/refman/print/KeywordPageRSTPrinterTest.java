package gov.sandia.dart.dakota.refman.print;

import static org.junit.Assert.assertEquals;

import org.junit.Before;
import org.junit.Test;

public class KeywordPageRSTPrinterTest {
	
	private KeywordPageRSTPrinter printer;
	
	@Before
	public void setup() {
		printer = new KeywordPageRSTPrinter();
	}

	@Test
	public void testConvertDoxygenCrud() {
		assertEquals("This is Dakota documentation.", printer.doxygenConvert("This is %Dakota documentation."));
		assertEquals("100% of my peers agree with me", printer.doxygenConvert("100% of my peers agree with me"));
		assertEquals("*Header*", printer.doxygenConvert("<b> Header </b>"));
	}
	
	@Test
	public void testConvertDoxygenCrud_Lists() {
		String original =
				"My Lists\n"
				+ "\\li List Item 1\n"
				+ "\n"
				+ "\\li List Item 2\n"
				+ "This item unfortunately has a line wrap\n"
				+ "\n"
				+ "End of list";
		String expected =
				"My Lists\n \n"
				+ "- List Item 1\n"
				+ "- List Item 2 This item unfortunately has a line wrap\n"
				+ "End of list";
		
		assertEquals(expected, printer.doxygenConvert(original));
	}
	
	@Test
	public void testConvertDoxygenCrud_Monospace() {
		assertEquals("This has a ``monospace`` section", printer.doxygenConvert("This has a \\c monospace section"));
		assertEquals("This has a \n ``monospace`` section with a line break", printer.doxygenConvert("This has a \\c\n monospace section with a line break"));
		assertEquals("This has a ( ``monospace`` ) section with parentheses", printer.doxygenConvert("This has a (\\c monospace ) section with parentheses"));
		assertEquals("method ( ``top_method_pointer`` ).  The output-related keywords address", printer.doxygenConvert("method (\\c top_method_pointer ).  The output-related keywords address"));
	}
	
	@Test
	public void testConvertDoxygenCrud_ListsAndMonospace() {
		String original =
			"Dakota exports tabular data in one of three formats:\n"
			+ "\\li \\c annotated (default)\n"
			+ "\\li \\c custom_annotated\n"
			+ "\\li \\c freeform";
		
		String expected =
			"Dakota exports tabular data in one of three formats:\n \n"
			+ "- ``annotated`` (default)\n"
			+ "- ``custom_annotated``\n"
			+ "- ``freeform``";
		
		assertEquals(expected, printer.doxygenConvert(original));
	}
}
