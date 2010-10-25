#ifndef _PHXCPPAPI_H_
#define _PHXCPPAPI_H_

extern "C" 
#ifndef unix
__declspec(dllexport)
#endif
void mc_load_model(int& nRetCode, const int& iPrint, int& pMCint, const char* modelFile);

extern "C"
#ifndef unix
__declspec(dllexport)
#endif
void mc_get_var_type(int& nRetCode, const int& iPrint, const int& pMCint, int& iType, const char* inName);

extern "C"
#ifndef unix
__declspec(dllexport)
#endif
void mc_get_enum_len(int& nRetCode, const int& iPrint, const int& pMCint, int& enumLen, const char* inName);

extern "C"
#ifndef unix
__declspec(dllexport)
#endif
void mc_get_enum_dbl(int& nRetCode, const int& iPrint, const int& pMCint, double* enumVal, int& enumLen, const char* inName);

extern "C"
#ifndef unix
__declspec(dllexport)
#endif
void mc_get_enum_int(int& nRetCode, const int& iPrint, const int& pMCint, int* enumVal, int& enumLen, const char* inName);

extern "C" 
#ifndef unix
__declspec(dllexport)
#endif
void mc_set_value(int& nRetCode, const int& iPrint, const int& pMCint, const double& inVal, const char* inName);

extern "C" 
#ifndef unix
__declspec(dllexport)
#endif
void mc_set_value_dbl(int& nRetCode, const int& iPrint, const int& pMCint, const double& inVal, const char* inName);

extern "C"
#ifndef unix
__declspec(dllexport)
#endif
void mc_set_value_int(int& nRetCode, const int& iPrint, const int& pMCint, const int& inVal, const char* inName);

extern "C" 
#ifndef unix
__declspec(dllexport)
#endif
void mc_get_value(int& nRetCode, const int& iPrint, const int& pMCint, double& f, const char* outName);

extern "C"
#ifndef unix
__declspec(dllexport)
#endif
void mc_get_value_int(int& nRetCode, const int& iPrint, const int& pMCint, int& f, const char* outName);

extern "C" 
#ifndef unix
__declspec(dllexport)
#endif
void mc_release(int& nRetCode, const int& iPrint, const int& pMCint);

extern "C" 
#ifndef unix
__declspec(dllexport)
#endif
void mc_release_com(int& nRetCode, const int& iPrint, const int& pCOMint);

extern "C" 
#ifndef unix
__declspec(dllexport)
#endif
void mc_store_current_design_point(int& nRetCode, const int& iPrint, const int& pDCint);

#endif  // _PHXCPPAPI_H_
