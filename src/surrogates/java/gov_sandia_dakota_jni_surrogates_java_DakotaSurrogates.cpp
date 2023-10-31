/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <jni.h>                // JNI header provided by JDK
#include <iostream>
#include "SurrogatesBase.hpp"

#include "gov_sandia_dakota_jni_surrogates_java_DakotaSurrogates.h"  // Generated

using namespace std;

vector<shared_ptr<dakota::surrogates::Surrogate>> surrogateCache;

shared_ptr<dakota::surrogates::Surrogate> findSurrogate(long inPointer) {
   for(int i = 0; i < surrogateCache.size(); i++) {
      long pointerAsLong = reinterpret_cast<unsigned long long>(surrogateCache.at(i).get());
      if(pointerAsLong == inPointer) {
         return surrogateCache.at(i);
      }
   }
   return NULL;
}

// Implementation of the native method loadSurrogate()
JNIEXPORT jlong JNICALL Java_gov_sandia_dakota_jni_surrogates_java_DakotaSurrogates_loadSurrogate(JNIEnv *env, jobject thisObj, jstring inJNIStr, jboolean inJNIBoolean) {

   // Convert the JNI String (jstring) into C-string (char*), then into C++ std::string
   jboolean isCopy;
   const char* inCStr = env->GetStringUTFChars(inJNIStr, &isCopy);
   if(NULL == inCStr) return 0;
   string absFilePath(inCStr);

   shared_ptr<dakota::surrogates::Surrogate> loadedSurrogate =
      dakota::surrogates::Surrogate::load(absFilePath, inJNIBoolean);
   
   surrogateCache.push_back(loadedSurrogate);
   
   return reinterpret_cast<jlong>(loadedSurrogate.get());
}

// Implementation of the native method getVariables()
JNIEXPORT jobjectArray JNICALL Java_gov_sandia_dakota_jni_surrogates_java_DakotaSurrogates_getVariables(JNIEnv *env, jobject thisObj, jlong inPointer) {

    // Step 1. Get the original surrogate shared_ptr.
    shared_ptr<dakota::surrogates::Surrogate> loadedSurrogate = findSurrogate(inPointer);

    // Step 2. Get the variable labels out of the surrogate and stuff them into the jobjectArray.
    if(loadedSurrogate != NULL) {
       vector<string> variableLabels = loadedSurrogate->variable_labels();
       jobjectArray outJNIArray =
          env->NewObjectArray(variableLabels.size(), env->FindClass("java/lang/String"), env->NewStringUTF(""));
    
       for(int i = 0; i < variableLabels.size(); i++) {
          const char* variableLabelCharArr = variableLabels.at(i).c_str();
          jobject objVariableLabel = env->NewStringUTF(variableLabelCharArr);
          env->SetObjectArrayElement(outJNIArray, i, objVariableLabel);
       }
    
       return outJNIArray;
    } else {
       return NULL;
    }
}

// Implementation of the native method getResponses()
JNIEXPORT jobjectArray JNICALL Java_gov_sandia_dakota_jni_surrogates_java_DakotaSurrogates_getResponses(JNIEnv *env, jobject thisObj,  jlong inPointer) {

    // Step 1. Get the original surrogate shared_ptr.
    shared_ptr<dakota::surrogates::Surrogate> loadedSurrogate = findSurrogate(inPointer);

    // Step 2. Get the response labels out of the surrogate and stuff them into the jobjectArray.
    if(loadedSurrogate != NULL) {
       vector<string> responseLabels = loadedSurrogate->response_labels();
       jobjectArray outJNIArray =
          env->NewObjectArray(responseLabels.size(), env->FindClass("java/lang/String"), env->NewStringUTF(""));
    
       for(int i = 0; i < responseLabels.size(); i++) {
          const char* responseLabelCharArr = responseLabels.at(i).c_str();
          jobject objResponseLabel = env->NewStringUTF(responseLabelCharArr);
          env->SetObjectArrayElement(outJNIArray, i, objResponseLabel);
       }
    
       return outJNIArray;
    } else {
       return NULL;
    }
}

// Implementation of the native method value()
JNIEXPORT jdouble JNICALL Java_gov_sandia_dakota_jni_surrogates_java_DakotaSurrogates_value(JNIEnv * env, jobject thisObj, jlong inPointer, jdoubleArray inJNIArr) {

    jboolean isCopy;
    jdouble *inDoubleArray = env->GetDoubleArrayElements(inJNIArr, &isCopy);
    
    if(NULL == inDoubleArray) return 0;
    jsize length = env->GetArrayLength(inJNIArr);

    shared_ptr<dakota::surrogates::Surrogate> loadedSurrogate = findSurrogate(inPointer);
    Eigen::MatrixXd evalPoints(1, length);
    
    for(int i = 0; i < length; i++) {
        evalPoints(0,i) = inDoubleArray[i];
    }

    Eigen::VectorXd result = loadedSurrogate->value(evalPoints);
    return result(0);
}

// Implementation of the native method nativeDeleteSurrogate()
JNIEXPORT void JNICALL Java_gov_sandia_dakota_jni_surrogates_java_DakotaSurrogates_nativeDeleteSurrogate(JNIEnv * env, jobject thisObj, jlong inPointer) {

    shared_ptr<dakota::surrogates::Surrogate> loadedSurrogate = findSurrogate(inPointer);
    if(loadedSurrogate != NULL) {
       loadedSurrogate.reset();
       surrogateCache.erase(remove(surrogateCache.begin(), surrogateCache.end(), loadedSurrogate), surrogateCache.end());
    }
}
