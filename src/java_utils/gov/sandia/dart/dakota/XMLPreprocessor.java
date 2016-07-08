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

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

public class XMLPreprocessor {
	private static int gen = 1;

	public static void main(String[] args) throws Exception {
		Document document = readDocument("dakota.macros.xml");

		// Strip macro definitions, save in table
		Map<String, Element> macros = new HashMap<String, Element>();
		List<Element> macroList= XMLToNIDRTranslator.asElementList(document.getElementsByTagName("defmacro"));
		for (Element macro: macroList) {
			macro.getParentNode().removeChild(macro);
			macros.put(macro.getAttribute("name"), macro);		
		}
		
		// Expand macros as needed
		List<Element> calls =  XMLToNIDRTranslator.asElementList(document.getElementsByTagName("call"));
		for (Element call: calls) {
			Map<String, String> replacements = getReplacements(call); 
			Node parentNode = call.getParentNode();
			String name = call.getAttribute("name");
			Element macro = macros.get(name);
			NodeList mChildren = macro.getChildNodes();
			for (int j=0; j<mChildren.getLength(); ++j) {
				Node copy = mChildren.item(j).cloneNode(true);
				applyReplacements(copy, replacements);
				parentNode.insertBefore(copy, call);
			}
			parentNode.removeChild(call);			
		}
		
		writeDocument(document, "dakota.gen.post.xml");
	}

	private static String genId(String attr) {
		return attr + gen++;
	}
	private static void applyReplacements(Node copy,
			Map<String, String> replacements) {
		NamedNodeMap attributes = copy.getAttributes();
		if (attributes == null)
			return;
		for (int i=0; i<attributes.getLength(); ++i) {
			Attr attr = (Attr) attributes.item(i);
			if (attr.getName().equals("id")) {
				attr.setValue(genId(attr.getValue()));
			} else {
				String value = attr.getValue();
				for (Map.Entry<String, String> entry: replacements.entrySet()) {
					value = value.replaceAll(entry.getKey(), entry.getValue());
				}
				attr.setValue(value);
			}
		}
		
		NodeList children = copy.getChildNodes();
		for (int i=0; i<children.getLength(); ++i) {
			applyReplacements(children.item(i), replacements);
		}		
	}

	private static Map<String, String> getReplacements(Element call) {
		NodeList args = call.getElementsByTagName("arg");
		Map<String, String> replacements = new HashMap<String, String>();
		for (int i=0; i<args.getLength(); ++i) {
			Element arg = (Element) args.item(i);
			replacements.put(arg.getAttribute("name"), arg.getTextContent());
		}
		return replacements;
	}

	private static Document readDocument(String filename) throws FileNotFoundException,
			Exception {
		InputSource inputSource = new InputSource(new FileReader(filename));
		Document document = XMLToNIDRTranslator.parseDocument(inputSource);
		return document;
	}

	private static void writeDocument(Document document, String filename)
			throws TransformerFactoryConfigurationError,
			TransformerConfigurationException, IOException,
			TransformerException {
		TransformerFactory tFactory = TransformerFactory.newInstance();
		Transformer transformer =  tFactory.newTransformer();
		transformer.setOutputProperty(OutputKeys.INDENT, "yes");
		DOMSource source = new DOMSource(document);
		StreamResult result = new StreamResult(new FileWriter(filename));
		transformer.transform(source, result);
	}

}
