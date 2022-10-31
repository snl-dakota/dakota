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
	public void testConvert_FullLineMathWithLineBreaks() {
		String original = "Use the traditional total-order index set to construct a polynomial chaos expansion. That is for a given order \\f$p\\f$ keep all terms with a \\f$d\\f$-dimensional multi index \\f$\\mathbf{i}=(i_1,\\ldots,i_d)\\f$ that satisfies\r\n"
						+ "\\f[\r\n"
						+ "\\sum_{k=1}^d i_k \\le p\r\n"
						+ "\\f]";
		String expected = "Use the traditional total-order index set to construct a polynomial chaos expansion. That is for a given order :math:`p`  keep all terms with a :math:`d` -dimensional multi index :math:`\\mathbf{i}=(i_1,\\ldots,i_d)`  that satisfies\n"
						+ "\n"
						+ ".. math:: \n"
						+ "\n"
						+ "   \\sum_{k=1}^d i_k \\le p"
						+ "\n\n";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_InlineMath() {
		String original = "where \\f$r\\f$ is the uniform refinement rate specified by";
		String expected = "where :math:`r`  is the uniform refinement rate specified by";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_ListOfCode() {
		String original = "- \\c /interfaces/truth/truth_m/";
		String expected = "\n- ``/interfaces/truth/truth_m/``";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_ListOfCodeWithMath() {
		String original = "- \\c /models/simulation/truth_m/sources/truth \\f$ \\rightarrow \\f$ \\c /interfaces/truth/truth_m/";
		String expected = "\n- ``/models/simulation/truth_m/sources/truth`` :math:`\\rightarrow`  ``/interfaces/truth/truth_m/``";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_MultilineListEntries() {
		String original = "- In the fork case, Dakota will treat the simulation as a black-box\r\n"
				        + "  and communication between Dakota and the simulation occurs through\r\n"
				        + "  parameter and result files. This is the most common case.";
		String expected = "\n- In the fork case, Dakota will treat the simulation as a black-box"
		                + "  and communication between Dakota and the simulation occurs through"
		                + "  parameter and result files. This is the most common case.";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}

	@Test
	public void testConvert_ListFollowedByBlankLine() {
		String original = "- \\c /interfaces/truth/truth_m/\r\n"
				        + "- \\c /interfaces/APPROX_INTERFACE_1/surr/\r\n"
				        + "\r\n"
				        + "Depending on the \\ref environment-results_output-hdf5-model_selection, the following links \r\n"
				        + "may be added to model \\c sources groups.";
		String expected = "\n- ``/interfaces/truth/truth_m/``\n"
				        + "- ``/interfaces/APPROX_INTERFACE_1/surr/``\n"
				        + "\n"
				        + "Depending on the :ref:`environment-results_output-hdf5-model_selection<environment-results_output-hdf5-model_selection>`, the following links\n"
				        + "may be added to model ``sources`` groups.";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testConvert_HtmlLists() {
		String original = "At a high-level, the adaptive sampling pipeline is a four-step process:\r\n"
						+ "<ul>\r\n"
						+ "<li> Evaluate the expensive simulation (referred to as the true model) at\r\n"
						+ "initial sample point\r\n"
						+ "<ol>\r\n"
						+ " <li> Fit a surrogate model\r\n"
						+ " <li> Create a candidate set and score based on information from surrogate\r\n"
						+ " <li> Select a candidate point to evaluate the true model \r\n"
						+ " <li> Loop until done\r\n"
						+ "</ol>\r\n"
						+ "</ul>\r\n"
						+ " \r\n"
						+ "In terms of the %Dakota implementation, the adaptive sampling method\r\n"
						+ "currently uses Latin Hypercube sampling (LHS) to generate the initial\r\n"
						+ "points in Step 1 above. For Step 2, we use a Gaussian process model.";
		String expected = "At a high-level, the adaptive sampling pipeline is a four-step process:\n"
						+ "\n"
						+ "\n"
						+ "- Evaluate the expensive simulation (referred to as the true model) at initial sample point\n"
						+ "\n"
						+ "\n"
						+ "  1. Fit a surrogate model\n"
						+ "\n"
						+ "  2. Create a candidate set and score based on information from surrogate\n"
						+ "\n"
						+ "  3. Select a candidate point to evaluate the true model\n"
						+ "\n"
						+ "  4. Loop until done\n"
						+ "\n"
						+ "\n"
						+ "\n"
						+ "In terms of the Dakota implementation, the adaptive sampling method\n"
						+ "currently uses Latin Hypercube sampling (LHS) to generate the initial\n"
						+ "points in Step 1 above. For Step 2, we use a Gaussian process model.";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));		
	}
	
	@Test
	public void testListsWithSpaces() {
		String original = "\\li \\c merit_max: based on \\f$ \\ell_\\infty\\f$ norm\r\n"
						+ "\\li \\c merit_max_smooth: based on smoothed \\f$ \\ell_\\infty\\f$ norm\r\n"
						+ "\\li \\c merit1: based on \\f$ \\ell_1\\f$ norm\r\n"
						+ "\\li \\c merit1_smooth: based on smoothed \\f$ \\ell_1\\f$ norm\r\n"
						+ "\\li \\c merit2: based on \\f$ \\ell_2\\f$ norm\r\n"
						+ "\\li \\c merit2_smooth: based on smoothed \\f$ \\ell_2\\f$ norm\r\n"
						+ "\\li \\c merit2_squared: based on \\f$ \\ell_2^2\\f$ norm\r\n"
						+ "\r\n"
						+ "The user can also specify the following to affect the merit functions:\r\n"
						+ "\\li \\c constraint_penalty\r\n"
						+ "\\li \\c smoothing_parameter";
		String expected = "\n- ``merit_max``: based on :math:`\\ell_\\infty`  norm\n"
						+ "- ``merit_max_smooth``: based on smoothed :math:`\\ell_\\infty`  norm\n"
						+ "- ``merit1``: based on :math:`\\ell_1`  norm\n"
						+ "- ``merit1_smooth``: based on smoothed :math:`\\ell_1`  norm\n"
						+ "- ``merit2``: based on :math:`\\ell_2`  norm\n"
						+ "- ``merit2_smooth``: based on smoothed :math:`\\ell_2`  norm\n"
						+ "- ``merit2_squared``: based on :math:`\\ell_2^2`  norm\n\n"
						+ "The user can also specify the following to affect the merit functions:\n\n"
						+ "- ``constraint_penalty``\n"
						+ "- ``smoothing_parameter``";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testUnnecessarilyNestedList() {
		String original = "There are three scale types:\r\n"
						+ "<ol>\r\n"
						+ "  <li> \\c value - characteristic value scaling\r\n"
						+ "\r\n"
						+ "    The target quantity will be scaled (divided) by the specified characteristic value.\r\n"
						+ "  </li>\r\n"
						+ "  <li> \\c auto - automatic scaling\r\n"
						+ "\r\n"
						+ "     First the quantity is scaled by any characteristic value, then\r\n"
						+ "     automatic scaling will be attempted according to the following scheme:\r\n"
						+ "\r\n"
						+ "    <ul> \r\n"
						+ "      <li> two-sided bounds scaled into the interval [0,1]; \r\n"
						+ "      <li> one-sided bound or targets are scaled by the characteristic\r\n"
						+ "         value, moving the bound or target to 1 and changing the sense of\r\n"
						+ "         inequalities where necessary;\r\n"
						+ "      <li> no bounds or targets: no automatic scaling possible, therefore no \r\n"
						+ "           scaling for this component \r\n"
						+ "    </ul> \r\n"
						+ "\r\n"
						+ "     Automatic scaling is not available for objective functions nor calibration \r\n"
						+ "     terms since they lack bound constraints. Futher, when automatically\r\n"
						+ "     scaled, linear constraints are scaled by characteristic values only, not\r\n"
						+ "     affinely scaled into [0,1]. \r\n"
						+ "  <li> \\c log - logarithmic scaling\r\n"
						+ "\r\n"
						+ "     First, any characteristic values from the\r\n"
						+ "     optional \\c *_scales specification are applied. Then logarithm base\r\n"
						+ "     10 scaling is applied.\r\n"
						+ "\r\n"
						+ "     Logarithmic scaling is not available for\r\n"
						+ "     linear constraints.\r\n"
						+ "\r\n"
						+ "     When continuous design variables are log\r\n"
						+ "     scaled, linear constraints are not allowed.\r\n"
						+ "  </li>\r\n"
						+ "</ol>";
		String expected = "There are three scale types:\n"
						+ "\n"
						+ "\n"
						+ "1. ``value`` - characteristic value scaling The target quantity will be scaled (divided) by the specified characteristic value.\n"
						+ "\n"
						+ "\n"
						+ "2. ``auto`` - automatic scaling First the quantity is scaled by any characteristic value, then automatic scaling will be attempted according to the following scheme:\n"
						+ "\n"
						+ "\n"
						+ "\n"
						+ "  - two-sided bounds scaled into the interval [0,1];\n"
						+ "\n"
						+ "  - one-sided bound or targets are scaled by the characteristic value, moving the bound or target to 1 and changing the sense of inequalities where necessary;\n"
						+ "\n"
						+ "  - no bounds or targets: no automatic scaling possible, therefore no scaling for this component Automatic scaling is not available for objective functions nor calibration terms since they lack bound constraints. Futher, when automatically scaled, linear constraints are scaled by characteristic values only, not affinely scaled into [0,1].\n"
						+ "\n"
						+ "3. ``log`` - logarithmic scaling First, any characteristic values from the optional ``*_scales`` specification are applied. Then logarithm base 10 scaling is applied. Logarithmic scaling is not available for linear constraints. When continuous design variables are log scaled, linear constraints are not allowed.";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));		
	}
	
	@Test
	public void testCitations() {
		String original = "Multi-dimensional integration by Stroud cubature rules \r\n"
						+ "\\cite stroud and extensions \r\n"
						+ "\\cite xiu_cubature, as specified with \\c cubature_integrand. ";
		String expected = "Multi-dimensional integration by Stroud cubature rules\n"
						+ ":cite:p:`stroud` and extensions\n"
						+ ":cite:p:`xiu_cubature`, as specified with ``cubature_integrand``.";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
	
	@Test
	public void testItalics() {
		String original = "at most,\r\n"
						+ "two-way interactions. In addition, the random variable set must\r\n"
						+ "be independent and identically distributed (\\e iid), so the use\r\n"
						+ "of \\c askey or \\c wiener transformations may be required to\r\n"
						+ "create \\e iid variable sets in the transformed space (as well as\r\n"
						+ "to allow usage of the higher order cubature rules for normal and\r\n"
						+ "uniform). Note that global sensitivity analysis often assumes\r\n"
						+ "uniform bounded regions, rather than precise probability\r\n"
						+ "distributions, so the \\e iid restriction would not be problematic\r\n"
						+ "in that case.";
		String expected = "at most,\n"
						+ "two-way interactions. In addition, the random variable set must\n"
						+ "be independent and identically distributed ( *iid*), so the use\n"
						+ "of ``askey`` or ``wiener`` transformations may be required to\n"
						+ "create *iid* variable sets in the transformed space (as well as\n"
						+ "to allow usage of the higher order cubature rules for normal and\n"
						+ "uniform). Note that global sensitivity analysis often assumes\n"
						+ "uniform bounded regions, rather than precise probability\n"
						+ "distributions, so the *iid* restriction would not be problematic\n"
						+ "in that case.";
		assertEquals(expected, DoxygenToRSTConverter.convert(original));
	}
}
