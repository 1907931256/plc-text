#include "string"
#include <vector>
void SetStrCat(std::string &strOut, std::string strIn, int nValue);
void SetValue(std::string &sRet, const char *szKey, const char *szValue, bool bEnd = false);
void SetValue(std::string &sRet, const char *szKey, int nValue, bool bEnd = false);
void SetjsonEnd(std::string &sRet);

//void split(std::string& s, std::string& delim,std::vector< std::string >* ret);