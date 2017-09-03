#ifndef __RG_UTILITIES_H_
#define __RG_UTILITIES_H_

#include <stdio.h>
#include <string>
#include <vector>




#ifdef __cplusplus
extern "C" {
#endif

/*
 * general function wrappers: 
 *
 *
 *
 *
 */

  int RG_DirectoryExists(const char * aDir); 


  int RG_FileExists(const char * aFileName);

  int RG_FIFOExists(const char * aFileName);


  void RG_Sleep(unsigned long ams); 

  
  std::vector<std::string> RG_Tokenize(std::string, char );

  unsigned int RG_LargerOf(unsigned int a, 
			   unsigned int b); 


  unsigned int binary_to_gray(unsigned int num);
  unsigned int gray_to_binary(unsigned int num);
  unsigned int ones_count(unsigned int num, unsigned int nbits);
  
  unsigned int parity(unsigned int num, unsigned int nbits);


  int RG_DivertStdStreamToFile(int stream, std::string fname);
  int RG_DivertBackStdStream(int stream, int file);
/*
 * error checking and reporting mechanisms. 
 *
 *
 */

#ifndef DEBUG_INT_STACK
#define DEBUG_INT_STACK 0
#endif
  

#if( DEBUG_INT_STACK )
#define DBG(_x)	((void)(_x))
#else
#define DBG(_x)	((void)0)
#endif




#define RG_DEBUG_MSG(debugstr)   do{fprintf(stdout, "DEBUG: %s (file %s : %d) %s \n",__func__,__FILE__,__LINE__, (debugstr));fflush(stdout);}while(0)


#define RG_CHECK_PTR_RETURN_NULL(ptr) do{if(NULL==(ptr)){fprintf(stderr, "RG_ERROR: NULL pointer encounter in function %s (file %s : %d). This could have been passed ass an argument, or the result of some operation.\n",__func__,__FILE__,__LINE__);return NULL;}}while(0)

#define RG_CHECK_PTR_RETURN_INT(ptr, retval) do{if(NULL==(ptr)){fprintf(stderr, "RG_ERROR: NULL pointer encountered in function %s (file %s : %d). This could have been passed ass an argument, or the result of some operation.\n",__func__,__FILE__,__LINE__);return(retval);}}while(0)

#define RG_CHECK_PTR_RETURN_VOID(ptr) do{if(NULL==(ptr)){fprintf(stderr, "RG_ERROR: NULL pointer encountered in function %s (file %s : %d). This could have been passed ass an argument, or the result of some operation\n",__func__,__FILE__,__LINE__);return;}}while(0)


#define RG_NULL_RETURN(rv) do{if(0!=(rv)){fprintf(stderr, "RG_ERROR: not NULL RV encountered in function %s (file %s : %d).\n",__func__,__FILE__,__LINE__);return rv;}}while(0)

#define RG_NULL_RETURN_MSG(rv, usr_msg) do{if(0!=(rv)){fprintf(stderr, "RG_ERROR: not NULL RV encountered in function %s (file %s : %d). Message = %s.\n",__func__,__FILE__,__LINE__,usr_msg);return rv;}}while(0)



#define RG_CHECK_ALLOCATION_RETURN_NULL(ptr) do{if(NULL==(ptr)){fprintf(stderr, "RG_ERROR: NULL pointer returned from allocation %s (file %s : %d) \n",__func__,__FILE__,__LINE__);return NULL;}}while(0)


#define RG_CHECK_ALLOCATION_RETURN_INT(ptr, retval) do{if(NULL==(ptr)){fprintf(stderr, "RG_ERROR: NULL pointer returned from allocation  %s (file %s : %d) \n",__func__,__FILE__,__LINE__);return(retval);}}while(0)


#define RG_CHECK_ZERO_RETURN(retval, usr_msg) do{if(0!=(retval)){fprintf(stderr, "RG_ERROR: Non-zero return value received (%d). Message = %s.(func %s file %s : %d)\n", retval, usr_msg, __func__,__FILE__,__LINE__);return(retval);}}while(0)





#define RG_CHECK_INDEX_RANGE_RETURN(low, value, high) do{	\
if((value)<(low) || ((value)>(high))){ \
  fprintf(stderr, "RG_ERROR: index out of permitted range: %d <= %d <= %d. Function=%s (%s:%d) \n", low, value, high, __func__, __FILE__, __LINE__); \
  return -1; }}while(0)
  
  

#define RG_CHECK_INDEX_UPPER_BOUND_RETURN(value, high) do{	\
if((value)>(high)){ \
  fprintf(stderr, "RG_ERROR: index greater than upper bound: %d <= %d. Function=%s (%s:%d) \n", value, high, __func__, __FILE__, __LINE__); \
  return -1; }}while(0)
  

#define RG_CHECK_INDEX_LOWER_BOUND_RETURN(value, low) do{	\
if((value)<(low)){							\
  fprintf(stderr, "RG_ERROR: index out of permitted range: %d <= %d. Function=%s (%s:%d) \n", low, value, __func__, __FILE__, __LINE__); \
  return -1; }}while(0)
  






#define RG_REPORT_ERROR_MSG(str) do{fprintf(stderr, "RG_ERROR: File = %s, function = %s, line %d: Message = %s \n", __FILE__,__func__,__LINE__,(str));}while(0)


#define RG_REPORT_ERROR_MSG2(str1, str2) do{fprintf(stderr, "RG_ERROR: File = %s, function = %s, line %d: Message = %s %s \n", __FILE__,__func__,__LINE__,(str1),(str2));}while(0)


#define RG_REPORT_ERROR_MSG3(str1, str2, str3) do{fprintf(stderr, "RG_ERROR: File = %s, function = %s, line %d: Message = %s %s %s \n", __FILE__,__func__,__LINE__,(str1),(str2),(str3));}while(0)


#define RG_REPORT_ERROR_MSG_CODE(str, code) do{fprintf(stderr, "RG_ERROR: File = %s, function = %s, line %d: Message = %s ; Code= %d \n", __FILE__,__func__,__LINE__,(str),(code));}while(0) 


#define RG_REPORT_ERROR_MSG_INT_VALUE(str, code) do{fprintf(stderr, "RG_ERROR: File = %s, function = %s, line %d: Message = %s %d \n", __FILE__,__func__,__LINE__,(str),(code));}while(0) 

#define RG_REPORT_ERROR_CODE(code) do{fprintf(stderr, "RG_ERROR: File = %s, function = %s, line %d : Code = %d \n", __FILE__,__func__,__LINE__,(code));}while(0)



#define RG_REPORT_ERROR_VALUES_DO_NOT_MATCH(msg, val1, val2) do{fprintf(stderr, "RG_ERROR: File = %s, function = %s, line %d : %s   %d != %d \n", __FILE__,__func__,__LINE__,(msg), (val1), (val2));}while(0)




#define RG_REPORT_WARNING_MSG(str) do{fprintf(stderr, "RG_WARN: File = %s, function = %s, line %d: Message = %s \n", __FILE__,__func__,__LINE__,(str));}while(0)
#define RG_REPORT_WARNING_MSG2(str1, str2) do{fprintf(stderr, "RG_WARN: File = %s, function = %s, line %d: Message = %s %s \n", __FILE__,__func__,__LINE__,(str1),(str2));}while(0)






#define RG_REPORT_INFO_MSG(str) do{fprintf(stdout, "RG_INFO: File = %s, function = %s, line %d: Message = %s \n", __FILE__,__func__,__LINE__,(str));fflush(stdout);}while(0)
#define RG_REPORT_INFO_MSG2(str1, str2) do{fprintf(stdout, "RG_INFO: File = %s, function = %s, line %d: Message = %s %s \n", __FILE__,__func__,__LINE__,(str1),(str2));fflush(stdout);}while(0)








  

#ifdef  __cplusplus
}
#endif




#endif 



