/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

package gov.sandia.dart.dakota.tools;

import gov.sandia.dart.dakota.jni.DakotaFunctor;
import gov.sandia.dart.dakota.jni.DoubleMatrix;
import gov.sandia.dart.dakota.jni.DoubleVector;
import gov.sandia.dart.dakota.jni.IntVector;
import gov.sandia.dart.dakota.jni.StringVector;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;


public abstract class AbstractDakotaFunctor extends DakotaFunctor implements Iterable<Map.Entry<String, Number>>{
    private Map<String, Number> variables = new HashMap<String, Number>();
    private List<String> responseLabels = new ArrayList<String>();

    protected double getDouble(String name) throws IllegalArgumentException {
	Number num = variables.get(name);
	if (num == null)
	    throw new IllegalArgumentException(name + ": no such parameter.");
	return num.doubleValue();
    }

    protected int getInt(String name) throws IllegalArgumentException {
	Number num = variables.get(name);
	if (num == null)
	    throw new IllegalArgumentException(name + ": no such parameter.");
	return num.intValue();
    }

    protected Number get(String name) throws IllegalArgumentException {
	Number num = variables.get(name);
	if (num == null)
	    throw new IllegalArgumentException(name + ": no such parameter.");
	return num;
    }

    protected List<String> getResponseLabels() {
        return responseLabels;
    }

    public Iterator<Map.Entry<String, Number>> iterator() {
	return variables.entrySet().iterator();
    }


    @Override public void
    setContinuousRealVariables(DoubleVector arg0, StringVector labels) {
	long size = arg0.size();
	for (int i=0; i<arg0.size(); ++i) {
	    variables.put(labels.get(i), arg0.get(i));
	}
    }

    @Override public void
    setDiscreteIntegerVariables(IntVector arg0, StringVector labels) {
	long size = arg0.size();
	for (int i=0; i<arg0.size(); ++i) {
	    variables.put(labels.get(i), arg0.get(i));
	}
    }

    @Override public void
    setDiscreteRealVariables(DoubleVector arg0, StringVector labels) {
	long size = arg0.size();
	for (int i=0; i<arg0.size(); ++i) {
	    variables.put(labels.get(i), arg0.get(i));
	}
    }

    @Override public void
    setResponseLabels(StringVector arg0) {
        responseLabels.clear();
	for (int i=0; i<arg0.size(); ++i) {
	    responseLabels.add(arg0.get(i));
	}
    }

}