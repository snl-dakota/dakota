/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

%include "std_string.i"
%include "std_vector.i"
%include "std_list.i"
%include "carrays.i"
%include "cpointer.i"


/* This tells SWIG to treat char ** as a special case when used as a parameter
   in a function call */
%typemap(in) char ** (jint size) {
    int i = 0;
    size = jenv->GetArrayLength($input);
    $1 = (char **) new char*[(size+1)*sizeof(char *)];
    /* make a copy of each string */
    for (i = 0; i<size; i++) {
      jstring j_string = (jstring)jenv->GetObjectArrayElement($input, i);
      const char * c_string = jenv->GetStringUTFChars(j_string, 0);
      $1[i] = new char[(strlen(c_string)+1)*sizeof(char)];
      strcpy($1[i], c_string);
      jenv->ReleaseStringUTFChars(j_string, c_string);
      jenv->DeleteLocalRef(j_string);
    }
    $1[i] = 0;
}

/* This cleans up the memory we new'd before the function call */
%typemap(freearg) char ** {
    int i;
    for (i=0; i<size$argnum-1; i++)
      delete[]($1[i]);
    delete[]($1);
}

/* This allows a C function to return a char ** as a Java String array */
%typemap(out) char ** {
    int i;
    int len=0;
    jstring temp_string;
    const jclass clazz = jenv->FindClass("java/lang/String");

    while ($1[len]) len++;    
    jresult = jenv->NewObjectArray(len, clazz, NULL);
    /* exception checking omitted */

    for (i=0; i<len; i++) {
      temp_string =jenv->NewStringUTF(*result++);
      jenv->SetObjectArrayElement(jresult, i, temp_string);
      jenv->DeleteLocalRef(temp_string);
    }
}

/* These 3 typemaps tell SWIG what JNI and Java types to use */
%typemap(jni) char ** "jobjectArray"
%typemap(jtype) char ** "String[]"
%typemap(jstype) char ** "String[]"

/* These 2 typemaps handle the conversion of the jtype to jstype typemap type
   and vice versa */
%typemap(javain) char ** "$javainput"
%typemap(javaout) char ** {
    return $jnicall;
  }

%exception {
  try {
    $action
  } catch (std::runtime_error &ex) {
    jclass clazz = jenv->FindClass("java/lang/RuntimeException");
    jenv->ThrowNew(clazz, ex.what());
    return $null;
  } 
}


%module(directors="1") DakotaWrapper

#pragma SWIG nowarn=473,514
%ignore operator=;
%ignore operator==;
%ignore operator<<;
%ignore operator!=;
%ignore operator>>;
%ignore derived_interface;
%ignore nearby;
%ignore hash_value;
%ignore shared_approximation;
%ignore approximation_data;
%ignore discrepancy_correction;
%ignore discrete_string_variables;
%ignore continuous_variable_types;
%ignore discrete_int_variable_types;
%ignore discrete_real_variable_types;
%ignore string_variable_types;
%ignore discrete_string_variable_types;
%ignore continuous_variable_labels;
%ignore discrete_int_variable_labels;
%ignore discrete_int_variable_labels;
%ignore discrete_real_variable_labels;
%ignore discrete_real_variable_labels;
%ignore string_variable_labels;
%ignore discrete_string_variable_labels;
%ignore discrete_string_variables_view;
%ignore all_discrete_string_variable_labels;
%ignore all_discrete_string_variables;
%ignore inactive_discrete_string_variable_labels;
%ignore inactive_discrete_string_variables;
%ignore inactive_discrete_string_variable_types;
%ignore continuous_variable_ids;
%ignore inactive_continuous_variable_labels;
%ignore inactive_discrete_int_variable_labels;
%ignore inactive_discrete_real_variable_labels;
%ignore inactive_string_variable_labels;
%ignore inactive_discrete_string_variable_labels;
%ignore inactive_continuous_variable_types;
%ignore inactive_discrete_int_variable_types;
%ignore inactive_discrete_real_variable_types;
%ignore inactive_continuous_variable_ids;
%ignore inactive_string_variable_ids;
%ignore inactive_discrete_string_variable_ids;
%ignore all_continuous_variable_labels;
%ignore all_discrete_int_variable_labels;
%ignore all_discrete_real_variable_labels;
%ignore all_continuous_variable_types;
%ignore all_string_variable_types;
%ignore all_discrete_string_variable_types;
%ignore all_discrete_int_variable_types;
%ignore all_discrete_real_variable_types;
%ignore all_continuous_variable_ids;
%ignore all_string_variable_ids;
%ignore all_discrete_string_variable_ids;

%template(StringVector) std::vector< std::string >;
%template(DoubleVector) std::vector< double  >;
%template(IntVector) std::vector< int  >;
%template(DoubleMatrix) std::vector< std::vector<double>  >;
%feature("director");
%{
  #include <stdexcept>
  #include "DakotaIterator.hpp"
  #include "OutputManager.hpp"
  #include "ProgramOptions.hpp"
  #include "DataMethod.hpp"
  #include "DataInterface.hpp"
  #include "DataVariables.hpp"
  #include "DataResponses.hpp"
  #include "ProblemDescDB.hpp"
  #include "DakotaVariables.hpp"
  #include "DakotaResponse.hpp"
  #include "DakotaEnvironment.hpp"
  #include "LibraryEnvironment.hpp"
  #include "DakotaUtils.hpp"
  #include "DakotaExtras.hpp"
  using namespace Dakota;
  extern "C" void nidr_set_input_string(const char *);
%}
#define StringArray std::vector<std::string>
#define String std::string
#define Real double
#define BOOST_SERIALIZATION_SPLIT_MEMBER() 
#define BOOST_CLASS_IMPLEMENTATION(X, Y)
#define BOOST_CLASS_TRACKING(X, Y)
%include "OutputManager.hpp"
%include "DakotaVariables.hpp"
%include "DakotaResponse.hpp"
%include "ProblemDescDB.hpp"
%include "ProgramOptions.hpp"
%include "DataVariables.hpp"
%include "DataResponses.hpp"
%include "DakotaEnvironment.hpp"
%include "LibraryEnvironment.hpp"
%include "DataMethod.hpp"
%include "DataInterface.hpp"
#undef String
#undef StringArray
#undef Real
%include "DakotaUtils.hpp"
%include "DakotaExtras.hpp"

%array_functions(int, pInt);
%array_functions(float, pFloat);
%array_functions(double, pDouble);
%array_functions(char*, ppChar);
%array_functions(std::string, pString);
%array_functions(bool, pBoolean);
%pointer_class(ProblemDescDB, pProblemDescDB);

extern "C" void nidr_set_input_string(const char *);
