/*******************************************************************************
 * Sandia Analysis Workbench Integration Framework (SAW)
 * Copyright 2016 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * This software is distributed under the Eclipse Public License.
 * For more information see the files copyright.txt and license.txt
 * included with the software.
 ******************************************************************************/
// briadam modification of XMLtoNIDRTranslator.java (ejfried)
package gov.sandia.dart.dakota;

import gov.sandia.dart.dakota.RefManInputSpec;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import javax.xml.namespace.NamespaceContext;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathException;
import javax.xml.xpath.XPathFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.ErrorHandler;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

// Note: this translator assumes XML validates!
// Parse a Dakota XML file and create a reference manual-friendly
// spec organization structure of type RefManInputSpec
public class XMLToRefManSpec {			
	
	public static void main(String[] args) throws Exception {		
		// default file name for testing only
		String dakota_xml = "dakota.xml";
		XMLToRefManSpec translator = new XMLToRefManSpec(dakota_xml);			
		translator.parse();
		RefManInputSpec test_spec_data = translator.refman_spec_data();
		test_spec_data.printDebug("debug.xml.txt");
	}
	
	// -----
	// Public client API
	// -----
	
	public XMLToRefManSpec(String input_xml) throws Exception {
		InputSource inputSource = new InputSource(new FileReader(input_xml));
		doc = parseDocument(inputSource); 	
		spec_data = new RefManInputSpec();	
	}

	public RefManInputSpec refman_spec_data() {
		return spec_data;
	}
	
	// -----
	// Core data members and convenience functions
	// -----

	// intermediate container to hold the parsed XML
	private Document doc;

	// container to hold the final data after parse
	private RefManInputSpec spec_data;

	// the current hierarchical keyword context such as [strategy, tabular_graphics_data, tabular_graphics_file]
	private List<String> context = new ArrayList<String>();
	
	// get the context as a string
	private String context_string() {
		String hierarchy_string = new String(context.get(0));
		for (String s: context.subList(1,context.size())) {
			hierarchy_string += "-" + s;
		}
		return hierarchy_string;
	}
	
	
	// -----
	// Main reference manual parse machinery
	// -----
	
	// parse the spec, generating input spec data structure
	public void parse() throws XPathException {
    	Element input = (Element) doc.getElementsByTagName("input").item(0);
    	for (Element toplevel: asElementList(input.getChildNodes())) {
 
    		// context for the top level contents (needed by this print)
			context.add(toplevel.getAttribute("name"));
			
			// print the header for the top-level keyword
			addKeywordHeader(toplevel);
			
			List<Element> children = asElementList(toplevel.getChildNodes());
			
			// print the immediate children of this keyword
			boolean immediate_children = true;
			for (Element top_elt: children)
				parseElement(top_elt, "Required_Keyword::", immediate_children);
			
			// recurse down the tree
			immediate_children = false;
			for (Element top_elt: children)
				parseElement(top_elt, "Required_Keyword::", immediate_children);
							
			// pop a context for the next top-level KW
			context.remove(context.size()-1);
			
    	}
	}
   
    private void parseElement(Element element, String option_str, boolean immediate_children) throws XPathException {
    	String tag = element.getTagName();
    	if (tag.equals("keyword"))
    		parseKeyword(element, option_str, immediate_children);
    	else if (tag.equals("oneOf"))
    		parseAlternatives(element, option_str, immediate_children);	
    	else if (tag.equals("required"))
    		parseRequired(element, option_str, immediate_children);
    	else if (tag.equals("optional"))
    		parseOptional(element, option_str, immediate_children);	
    	else if (tag.equals("group")) {
                // for now we ignore labeled groups, don't add
                // an additional context, and parse child nodes
                for (Element child: asElementList(element.getChildNodes())) {
		    parseElement(child, option_str, immediate_children);
		}
	}
    	else if (tag.equals("alias") || tag.equals("param"))
    		; // Covered elsewhere
    	else
    		System.out.println("### ERROR: UNKNOWN ELEMENT " + tag);
    }

	private void parseKeyword(Element keyword, String option_str, boolean immediate_children) throws XPathException {
		// TODO Parentheses
		boolean isOptional = keyword.hasAttribute("minOccurs") && keyword.getAttribute("minOccurs").equals("0");
//		boolean hasChildren = hasChildren(keyword);
//		String extras ="";// = getKeywordExtras(keyword); 
//		String output = String.format("%s%s %s%s%s",
//				openingCharacter(isOptional, hasChildren),
//				keyword.getAttribute("name"), 
//				extras,
//				keyword.getAttribute("code"),
//				isOptional && !hasChildren ? " ]" : "");
//			if (hasChildren) 
//				for (Element child: asElementList(keyword.getChildNodes()))
//					parseElement(child);
		
		// if the alternation is marked with optional or required (Choose), just need the KW here
		if (immediate_children) {
			String kw_optional_str = "";
			if (!option_str.contains("Choose"))
				// BMA TODO: Is this correct deviation from the NIDR one?
				if(isOptional)
					kw_optional_str = "Optional_Keyword::";
				else
					kw_optional_str = "Required_Keyword::";
			addTaggedChild(kw_optional_str, keyword);
		}
		else {
			// hierarchical context for the top level contents 
			context.add(keyword.getAttribute("name"));
			addKeywordHeader(keyword);

			// BMA TODO? unlike in NIDR, where they are always in a group, keywords can have sub-keywords
			// Why do we need to print children here?
			List<Element> sub_keywords = asElementList(keyword.getChildNodes());
			for (Element sk: sub_keywords) {
//				// BMA TODO: should be a simpler way to track entering a required KW
//				String kw_optional_str = option_str;
//				if (!isOptional) {
//					// flip optional to required
//					kw_optional_str.replaceAll("Optional", "Required");
//				}
//				parseElement(sk, kw_optional_str, true);
//				parseElement(sk, kw_optional_str, false);
				parseElement(sk, option_str, true);
				parseElement(sk, option_str, false);

			}
			// pop a context for the next top-level KW
			context.remove(context.size()-1);	
		}
	}
	
	// Get a space-separating string of ALIASes from the element
	// TODO: multiple aliases aren't supported, but we print them for now
	private String getAliases(Element keyword) throws XPathException {
		XPath xPath = getXPathProcessor();
		StringBuilder builder = new StringBuilder();
		List<Element> aliases = asElementList((NodeList) xPath.evaluate("dak:alias", keyword, XPathConstants.NODESET));
		for (Element alias: aliases) {
			builder.append(alias.getAttribute("name"));
			builder.append(" ");
		}
		return builder.toString();
	}
	
	private String getParams(Element keyword) throws XPathException {
		XPath xPath = getXPathProcessor();
		StringBuilder builder = new StringBuilder();
		List<Element> params = asElementList((NodeList) xPath.evaluate("dak:param", keyword, XPathConstants.NODESET));
		for (Element param: params) {
			builder.append(getType(param));
			builder.append(" ");
			// BMA TODO: include the constraints on size, value, etc.
//			if (param.hasAttribute("constraint")) {
//				builder.append(param.getAttribute("constraint"));
//				builder.append(" ");				
//			}
		}
		return builder.toString();
	}

	private String getType(Element param) {
		String xmlType = param.getAttribute("type");
		String result = xmlType;
		if ("OUTPUT_FILE".equals(xmlType) || "INPUT_FILE".equals(xmlType))
			result = "STRING";
		return result;
	}

	// Determine whether a keyword has any non-trivial children -- i.e.,
	// children other than param and alias elements
	private static List<String> trivialTags = Arrays.asList("alias", "param");
	private boolean hasChildren(Element keyword) {		
		for (Element element: asElementList(keyword.getChildNodes()))
			if (!trivialTags.contains(element.getTagName()))
						return true;
		return false;
	}
	
	private void parseAlternatives(Element oneOf, String option_str, boolean immediate_children) throws XPathException {
		List<Element> alternatives = asElementList(oneOf.getChildNodes());
		if (immediate_children) {
			
			String group_label = "";
			if (oneOf.hasAttribute("label"))
				group_label = " " + oneOf.getAttribute("label");
			
			// if doesn't already have Choose, set the tag to Choose
			// if already has, don't want multiple since this doesn't parse nested KW
			String header_str = option_str;
			if (!option_str.contains("Choose") && option_str.contains("Optional")) {
				header_str = "Optional_Choose_One::" + group_label;
				spec_data.appendChild(context_string(), header_str);
			}
			else if (!option_str.contains("Choose") && option_str.contains("Required")) {
				header_str = "Required_Choose_One::" + group_label;
				spec_data.appendChild(context_string(), header_str);
			}		
			// else already has a choose header; don't append another in immediate children mode
			for (Element alt_elt: alternatives)
				parseElement(alt_elt, header_str, immediate_children);
		}
		else {
			// iterate the alternatives, printing a header for them and their subkeywords
			// deviation from NIDR; alternatives are required by default.
			for (Element alt_elt: alternatives)
				parseElement(alt_elt, "Required_Keyword::", immediate_children);
			//parseElement(alt_elt, option_str, immediate_children);
		}
	}

	
	private void parseOptional(Element element, String option_str, boolean immediate_children) throws XPathException {
		// BMA TODO: check if non-trivial children
		List<Element> children = asElementList(element.getChildNodes());
		String kw_optional_str = "Optional_Keyword::";
		if (immediate_children) {
			// must propagate Choose, but the keyword itself is required within a potentially optimal Choose
			if (option_str.contains("Choose"))
				kw_optional_str = "Optional_Choose_One::";
			parseGroupLeader(children, kw_optional_str, immediate_children);
		}	
		else 
			parseGroup(children, kw_optional_str, immediate_children);
	}
		
	private void parseRequired(Element element, String option_str, boolean immediate_children) throws XPathException {
		// BMA TODO: check if non-trivial children
		List<Element> children = asElementList(element.getChildNodes());
		String kw_optional_str = "Required_Keyword::";
		if (immediate_children) {
			// must propagate Choose, but the keyword itself is required within a potentially optimal Choose
			if (option_str.contains("Choose"))
				kw_optional_str = "Required_Choose_One::";
			parseGroupLeader(children, kw_optional_str, immediate_children);
		}	
		else 
			parseGroup(children, kw_optional_str, immediate_children);
	}

	private void parseGroupLeader(List<Element> list, String option_str, boolean immediate_children) throws XPathException {
		
		Element leading_item = list.get(0);
		// group leader is a keyword
		String tag = leading_item.getTagName();
    	if (tag.equals("keyword")) {
			// when in choose mode, use empty string as the alternation is already marked Opt/Req
			String kw_optional_str = "";
			if (!option_str.contains("Choose")) {
				kw_optional_str = option_str.contains("Optional") ? "Optional_Keyword::" : "Required_Keyword::";
			}
			addTaggedChild(kw_optional_str, leading_item);
		}
		// TODO: do we need special treatment of alternatives?
		// for example, group leader is another group or an alternation
		else {
			parseElement(leading_item, option_str, immediate_children);
		}
			
	}
	
	
	private void parseGroup(List<Element> list, String option_str, boolean immediate_children) throws XPathException {
		
		// A group can have in it: ReqGroup, OptGroup, Alternatives, or KW
		
		Element first_entry = list.get(0);
		List<Element> sublist = list.subList(1, list.size());
		
		// A leading KW can't be optional
		String tag = first_entry.getTagName();
    	if (tag == "optional") {
			System.out.println("Warning: In context " + context.toString() + ", group leader is optional.");
		}

		// print leader to add its entries, regardless of its type (KW, ALT, GROUP)
		// whether the leader is required is dictated by the calling context
		parseElement(first_entry, option_str, false);
		
		// iterate immediate leads, print sublist for each and recursing
		ArrayList<String> immed_leads = find_immediate_leaders(first_entry);
		
		// TODO: verify this is right when this is an optional group itself
		// if there was a keyword leader, following keywords, groups, alternations are required unless again marked optional
		String sublist_optional = "Required_Keyword::";
		
		for (String s: immed_leads) {
			// print the sublist with each possible context from the leader
			context.add(s);
			if (sublist.size() > 0) {
				// print immediate children
				for (Element sub_elt: sublist) 
					parseElement(sub_elt, sublist_optional, true);
				// now recurse on items, printing their header and recursing
				for (Element sub_elt: sublist) 
					parseElement(sub_elt, sublist_optional, false);
			}
			context.remove(context.size()-1);
		}
			// TODO!!! IF we have alternation of groups followed by other spec, they are missed in this scheme
//			// Any alternation of required components appearing first in the group can serve as the leading KW.
//			// hack to try to collect up all the leading keywords in this first item's groups that may give context
//				
//			// This will print the subitems at the current level, with the right header, unrolling ALTs
//			// Probably a little aggressive...
//			// TODO: Are we properly recursing to print all keywords?
//			
//			// TODO: this might mess up the printing of immediate children; they'll get inserted in the middle of parents
	}
	
	
	// unroll alternates and groups to find any keywords that can appear as peers at this level
	// this is similar to printing leaders, but we don't want to emit output, so reimplement
	// recursive search returns appended list
	// TODO: should we append or prepend as we find?
	private ArrayList<String> find_immediate_leaders(Element element) {
		ArrayList<String> immed_leads = new ArrayList<String>();
		String tag = element.getTagName();
		if (tag.equals("keyword"))
			// don't want to print bare keywords when in recursive mode; results in duplicates
			immed_leads.add(element.getAttribute("name"));
		else if (tag.equals("oneOf")) {
			List<Element> alternatives = asElementList(element.getChildNodes());
			for (Element alt_elt: alternatives)
				immed_leads.addAll(find_immediate_leaders(alt_elt));
		}	
		else if (tag.equals("required")) {
			List<Element> children = asElementList(element.getChildNodes());
			Element group_leader = children.get(0);
			immed_leads.addAll(find_immediate_leaders(group_leader));
		}
		else if (tag.equals("optional"))
			// optional can't be a leader for anchor purposes
			;	
		else if (tag.equals("alias") || tag.equals("param"))
			// shouldn't happen
			; 
		else
			System.out.println("### ERROR: UNKNOWN ELEMENT " + tag);
		return immed_leads;
	}

	
	// print the tag followed by keyword "name" field
	private void addTaggedChild(String tag, Element keyword) {
		spec_data.appendChild(context_string(), tag + " " + keyword.getAttribute("name"));
	}

	private void addKeywordHeader(Element keyword) throws XPathException {

		String hierarchy_string = context_string();

		spec_data.addData(hierarchy_string, "Keyword_Hierarchy", hierarchy_string);
		spec_data.addData(hierarchy_string, "Name", keyword.getAttribute("name"));

		String space_sep_aliases = getAliases(keyword);
		//if (!space_sep_aliases.isEmpty())
		spec_data.addData(hierarchy_string, "Alias", space_sep_aliases);
		
		// TODO: may need to format parameters
		String space_sep_params = getParams(keyword);
		//if (!space_sep_params.isEmpty())
		//addData("Argument", space_sep_params);
		// for backward compat with debugging output, trim to avoid constraints, pointers:
		spec_data.addData(hierarchy_string, "Argument", space_sep_params.trim());

		if (keyword.hasAttribute("default"))
			spec_data.addData(hierarchy_string, "Default", keyword.getAttribute("default"));

	}

	
	// -----
	// Convenience functions
	// -----
	
	public static List<Element> asElementList(NodeList nodes) {
		List<Element> elements = new ArrayList<Element>();
		for (int i=0; i<nodes.getLength(); ++i) {
			if (nodes.item(i) instanceof Element)
				elements.add((Element) nodes.item(i));
		}
		return elements;
	}

	public static Document parseDocument(InputSource source) throws Exception {
            DocumentBuilderFactory dfactory = DocumentBuilderFactory.newInstance();
            dfactory.setNamespaceAware(true);
            DocumentBuilder docBuilder = dfactory.newDocumentBuilder();
            docBuilder.setErrorHandler(new ThrowingErrorHandler());
            return docBuilder.parse(source);            
    }
    
    private static class ThrowingErrorHandler implements ErrorHandler {
        public void warning(SAXParseException exception) throws SAXException {
            throw exception;
        }
        public void error(SAXParseException exception) throws SAXException {
            throw exception;
        }
        public void fatalError(SAXParseException exception) throws SAXException {
            throw exception;
        }
    }
    
    private static XPath xPathInstance;
    public static XPath getXPathProcessor() {
    	if (xPathInstance == null) {
    		xPathInstance = XPathFactory.newInstance().newXPath();
    		xPathInstance.setNamespaceContext(new NamespaceContext() {

    			@Override
    			public Iterator getPrefixes(String namespaceURI) {
    				throw new UnsupportedOperationException();
    			}

    			@Override
    			public String getPrefix(String namespaceURI) {
    				throw new UnsupportedOperationException();
    			}

    			@Override
    			public String getNamespaceURI(String prefix) {
    				return "http://www.sandia.gov/dakota/1.0";
    			}
    		});
    	}
    	return xPathInstance;
    }
}
