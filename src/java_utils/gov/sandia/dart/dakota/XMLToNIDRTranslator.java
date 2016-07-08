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
package gov.sandia.dart.dakota;

import java.io.FileReader;
import java.io.FileWriter;
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
public class XMLToNIDRTranslator {			

	public static void main(String[] args) throws Exception {		
		
		// default file names for backward compatibility only
		// can be removed or changed once DWB doesn't need them for testing
		String dakota_xml = "dakota.gen.xml";
		String dakota_nspec = "dakota.input.gen.nspec";
		
		if (args.length !=0 && args.length != 2) {
			System.err.println("Usage: XMLToNIDRTranslator [ dakota.xml dakota.input.nspec ]");
			System.exit(1);
		}
		if (args.length > 0) {
			dakota_xml = args[0];
			dakota_nspec = args[1];
		}
		
		InputSource inputSource = new InputSource(new FileReader(dakota_xml));
		XMLToNIDRTranslator translator = new XMLToNIDRTranslator(inputSource, new FileWriter(dakota_nspec));			
		try {
			translator.writeNIDRFile();
		} finally {
			translator.close();
		}
	}
	
	private Indenter out;
	private Document doc;
	
	public XMLToNIDRTranslator(InputSource inputSource, FileWriter fileWriter) throws Exception {
		doc = parseDocument(inputSource); 
		out = new Indenter(fileWriter);
	}

    private void writeNIDRFile() throws XPathException {
    	// TODO Indentation!
    	Element input = (Element) doc.getElementsByTagName("input").item(0);
    	for (Element toplevel: asElementList(input.getChildNodes())) {
    		printToplevel(toplevel);
    	}
	}
    
	private  void printToplevel(Element toplevel) throws XPathException {
		out.printAndIndent(String.format("%s %s %s", chooseKeyword(toplevel), toplevel.getAttribute("name"), toplevel.getAttribute("code")));	
		for (Element child: asElementList(toplevel.getChildNodes())) {
			printElement(child);
		}
		out.unindentAndPrint("");
	}

	private Object chooseKeyword(Element toplevel) {
		if ("0".equals(toplevel.getAttribute("minOccurs")) && "1".equals(toplevel.getAttribute("maxOccurs")))
				return "KEYWORD01";
		else if ("1".equals(toplevel.getAttribute("minOccurs")) && "unbounded".equals(toplevel.getAttribute("maxOccurs")))
			return "KEYWORD12";		
		else
			return "KEYWORD";
	}

	private void printKeyword(Element keyword) throws XPathException {
		// TODO Parentheses
		boolean isOptional = keyword.hasAttribute("minOccurs") && keyword.getAttribute("minOccurs").equals("0");
		boolean hasChildren = hasChildren(keyword);
		String extras = getKeywordExtras(keyword); 
		String output = String.format("%s%s %s%s%s",
				openingCharacter(isOptional, hasChildren),
				keyword.getAttribute("name"), 
				extras,
				keyword.getAttribute("code"),
				isOptional && !hasChildren ? " ]" : "");
		
		if (!hasChildren)
			out.printIndented(output);
		else {
			out.printAndIndent(output);
			for (Element child: asElementList(keyword.getChildNodes())) {
				printElement(child);
			}
			out.unindentAndPrint(closingCharacter(isOptional, hasChildren));
		}
	}

	private String closingCharacter(boolean isOptional, boolean hasChildren) {
		if (isOptional)
			return " ]";
		else if (hasChildren)
			return " )";
		else
			return "";		
	}

	private String openingCharacter(boolean isOptional, boolean hasChildren) {
		if (isOptional)
			return "[ ";
		else if (hasChildren)
			return "( ";
		else
			return "";				
	}

	private String getKeywordExtras(Element keyword) throws XPathException {
		XPath xPath = getXPathProcessor();
		StringBuilder builder = new StringBuilder();
		List<Element> aliases = asElementList((NodeList) xPath.evaluate("dak:alias", keyword, XPathConstants.NODESET));
		for (Element alias: aliases) {
			builder.append("ALIAS ");
			builder.append(alias.getAttribute("name"));
			builder.append(" ");
		}
		
		List<Element> params = asElementList((NodeList) xPath.evaluate("dak:param", keyword, XPathConstants.NODESET));
		for (Element param: params) {
			builder.append(getType(param));
			builder.append(" ");
			if (param.hasAttribute("constraint")) {
				builder.append(param.getAttribute("constraint"));
				builder.append(" ");				
			}
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
	// Do these need to be grouped?
	private void printAlternatives(Element oneOf) throws XPathException {
		List<Element> elements = asElementList(oneOf.getChildNodes());
		printElement(elements.get(0));
		for (int i=1; i<elements.size(); ++i) {
			out.printIndented("|");
			printElement(elements.get(i));			
		}
	}

	private void printElement(Element element) throws XPathException {
		// TODO Required, Optional
		String tag = element.getTagName();
		if (tag.equals("keyword"))
			printKeyword(element);
		else if (tag.equals("oneOf"))
			printAlternatives(element);	
		else if (tag.equals("required"))
			printRequired(element);
		else if (tag.equals("optional"))
			printOptional(element);	
		else if (tag.equals("group"))
  		        printGroup(element);
		else if (tag.equals("alias") || tag.equals("param"))
			; // Covered elsewhere
		else {
			out.printIndented("### ERROR: UNKNOWN ELEMENT " + tag);
			System.out.println("### ERROR: UNKNOWN ELEMENT " + tag);
		}
	}

	private void printOptional(Element element) throws XPathException {
		out.printAndIndent("[ ");
		for (Element child: asElementList(element.getChildNodes())) {
			printElement(child);
		}
		out.unindentAndPrint(" ]");	}

	private void printRequired(Element element) throws XPathException {
		out.printAndIndent("( ");
		for (Element child: asElementList(element.getChildNodes())) {
			printElement(child);
		}
		out.unindentAndPrint(" )");
	}

        // For a labeled group, don't add context, just parse contents
	private void printGroup(Element element) throws XPathException {
	        // NIDR doesn't know about labeled groups; we just skip them
		for (Element child: asElementList(element.getChildNodes())) {
			printElement(child);
		}
	}

	public static List<Element> asElementList(NodeList nodes) {
		List<Element> elements = new ArrayList<Element>();
		for (int i=0; i<nodes.getLength(); ++i) {
			if (nodes.item(i) instanceof Element)
				elements.add((Element) nodes.item(i));
		}
		return elements;
	}

	private void close() {
		out.close();
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
