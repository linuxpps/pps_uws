#ifndef __COMMON_H_
#define __COMMON_H_

#pragma once

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <regex>
#include <string>
#include <unordered_map>

#ifdef _MSC_VER
#define PATH_SPLITER_CHAR		'\\'
#define MAX_PATH_LEN			MAX_PATH
#else
#define PATH_SPLITER_CHAR		'/'
#define MAX_PATH_LEN			4096
#include <dirent.h>
#endif

#define NULL_CHAR				'\0'
#define __PROG_PATH_MAIN__		static char PROG_PATH[MAX_PATH_LEN] = { 0 };

#define __PROG_PATH_INIT__(X)	{								\
	strncpy(PROG_PATH, X, sizeof(PROG_PATH));					\
	char* PROG_PATH_PTR = strrchr(PROG_PATH, PATH_SPLITER_CHAR);\
	if (PROG_PATH_PTR != NULL) {								\
		*(PROG_PATH_PTR + 1) = NULL_CHAR;						\
	}															\
	else {														\
		*(PROG_PATH_PTR) = NULL_CHAR;							\
	}															\
}

//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#include <codecvt>
#endif

//#define AToW_EX(s,l) std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>>(new std::codecvt_byname<wchar_t, char, std::mbstate_t>(l)).from_bytes(s)
//#define WToA_EX(s,l) std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>>(new std::codecvt_byname<wchar_t, char, std::mbstate_t>(l)).to_bytes(s)

#define AToW_EX(s,l) std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>>(new std::codecvt<wchar_t, char, std::mbstate_t>(l)).from_bytes(s)
#define WToA_EX(s,l) std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>>(new std::codecvt<wchar_t, char, std::mbstate_t>(l)).to_bytes(s)
#define AToW(s) AToW_EX(s,"chs")
#define WToA(s) WToA_EX(s,"chs")

#define WToUTF8(s) std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(s)
#define UTF8ToW(s) std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(s)

#if !defined(_UNICODE) && !defined(UNICODE)
#define AToT_EX(s,l)	WToA_EX(AToW_EX(s,l),l)
#define TToA_EX(s,l)	WToA_EX(AToW_EX(s,l),l)
#define WToT_EX(s,l)	WToA_EX(s,l)
#define TToW_EX(s,l)	AToW_EX(s,l)
#define AToT(s)	s
#define TToA(s)	s
#define WToT(s)	WToA(s)
#define TToW(s)	AToW(s)
#define TToUTF8(s)	WToUTF8(AToW(s))
#define AToUTF8(s)	WToUTF8(AToW(s))
#define UTF8ToA(s)	WToA(UTF8ToW(s))
#define UTF8ToT(s)	WToA(UTF8ToW(s))
#else
#define AToT_EX(s,l)	AToW_EX(s,l)
#define TToA_EX(s,l)	WToA_EX(s,l)
#define WToT_EX(s,l)	AToW_EX(WToA_EX(s,l),l)
#define TToW_EX(s,l)	AToW_EX(WToA_EX(s,l),l)
#define AToT(s) AToW(s)
#define TToA(s)	WToA(s)
#define WToT(s)	s
#define TToW(s)	s
#define TToUTF8(s)	WToUTF8(s)
#define AToUTF8(s)	WToUTF8(AToW(s))
#define UTF8ToA(s)	WToA(UTF8ToW(s))
#define UTF8ToT(s)	UTF8ToW(s)
#endif

#include <iconv.h>
__inline static
bool unicode_to_utf8(char* inbuf, size_t* inlen, char* outbuf, size_t* outlen)
{
	/* 目的编码, TRANSLIT：遇到无法转换的字符就找相近字符替换
	 *           IGNORE ：遇到无法转换字符跳过*/
	char* encTo = "UTF-8//IGNORE";
	/* 源编码 */
	char* encFrom = "UNICODE";

	/* 获得转换句柄
	 *@param encTo 目标编码方式
	 *@param encFrom 源编码方式
	 *
	 * */
	iconv_t cd = iconv_open(encTo, encFrom);
	if (cd == (iconv_t)-1)
	{
		perror("iconv_open");
	}

	/* 需要转换的字符串 */
	//printf("inbuf=%s\n", inbuf);

	/* 打印需要转换的字符串的长度 */
	//printf("inlen=%d\n", *inlen);


	/* 由于iconv()函数会修改指针，所以要保存源指针 */
	char* tmpin = inbuf;
	char* tmpout = outbuf;
	size_t insize = *inlen;
	size_t outsize = *outlen;

	/* 进行转换
	 *@param cd iconv_open()产生的句柄
	 *@param srcstart 需要转换的字符串
	 *@param inlen 存放还有多少字符没有转换
	 *@param tempoutbuf 存放转换后的字符串
	 *@param outlen 存放转换后,tempoutbuf剩余的空间
	 *
	 * */
	size_t ret = iconv(cd, &tmpin, inlen, &tmpout, outlen);
	if (ret == -1)
	{
		perror("iconv");
	}

	/* 存放转换后的字符串 */
	//printf("outbuf=%s\n", outbuf);

	//存放转换后outbuf剩余的空间
	//printf("outlen=%d\n", *outlen);

	int i = 0;

	//for (i = 0; i < (outsize - (*outlen)); i++)
	{
		//printf("%2c", outbuf[i]);
		//printf("%x\n", outbuf[i]);
	}

	/* 关闭句柄 */
	iconv_close(cd);

	return 0;
}
__inline static
bool utf8_to_unicode(char* inbuf, size_t* inlen, char* outbuf, size_t* outlen)
{

	/* 目的编码, TRANSLIT：遇到无法转换的字符就找相近字符替换
	 *           IGNORE ：遇到无法转换字符跳过*/
	char* encTo = "UNICODE//IGNORE";
	/* 源编码 */
	char* encFrom = "UTF-8";

	/* 获得转换句柄
	 *@param encTo 目标编码方式
	 *@param encFrom 源编码方式
	 *
	 * */
	iconv_t cd = iconv_open(encTo, encFrom);
	if (cd == (iconv_t)-1)
	{
		perror("iconv_open");
	}

	/* 需要转换的字符串 */
	//printf("inbuf=%s\n", inbuf);

	/* 打印需要转换的字符串的长度 */
	//printf("inlen=%d\n", *inlen);

	/* 由于iconv()函数会修改指针，所以要保存源指针 */
	char* tmpin = inbuf;
	char* tmpout = outbuf;
	size_t insize = *inlen;
	size_t outsize = *outlen;

	/* 进行转换
	 *@param cd iconv_open()产生的句柄
	 *@param srcstart 需要转换的字符串
	 *@param inlen 存放还有多少字符没有转换
	 *@param tempoutbuf 存放转换后的字符串
	 *@param outlen 存放转换后,tempoutbuf剩余的空间
	 *
	 * */
	size_t ret = iconv(cd, &tmpin, inlen, &tmpout, outlen);
	if (ret == -1)
	{
		perror("iconv");
	}

	/* 存放转换后的字符串 */
	//printf("outbuf=%s\n", outbuf);

	//存放转换后outbuf剩余的空间
	//printf("outlen=%d\n", *outlen);

	int i = 0;

	//for (i = 0; i < (outsize - (*outlen)); i++)
	{
		//printf("%2c", outbuf[i]);
		//printf("%x\n", outbuf[i]);
	}

	/* 关闭句柄 */
	iconv_close(cd);

	return 0;
}
__inline static
bool gb2312_to_utf8(char* inbuf, size_t* inlen, char* outbuf, size_t* outlen)
{
	/* 目的编码, TRANSLIT：遇到无法转换的字符就找相近字符替换
	 *           IGNORE ：遇到无法转换字符跳过*/
	char* encTo = "UTF-8//IGNORE";
	/* 源编码 */
	char* encFrom = "GB2312";

	/* 获得转换句柄
	 *@param encTo 目标编码方式
	 *@param encFrom 源编码方式
	 *
	 * */
	iconv_t cd = iconv_open(encTo, encFrom);
	if (cd == (iconv_t)-1)
	{
		perror("iconv_open");
	}

	/* 需要转换的字符串 */
	//printf("inbuf=%s\n", inbuf);

	/* 打印需要转换的字符串的长度 */
	//printf("inlen=%d\n", *inlen);


	/* 由于iconv()函数会修改指针，所以要保存源指针 */
	char* tmpin = inbuf;
	char* tmpout = outbuf;
	size_t insize = *inlen;
	size_t outsize = *outlen;

	/* 进行转换
	 *@param cd iconv_open()产生的句柄
	 *@param srcstart 需要转换的字符串
	 *@param inlen 存放还有多少字符没有转换
	 *@param tempoutbuf 存放转换后的字符串
	 *@param outlen 存放转换后,tempoutbuf剩余的空间
	 *
	 * */
	size_t ret = iconv(cd, &tmpin, inlen, &tmpout, outlen);
	if (ret == -1)
	{
		perror("iconv");
	}

	/* 存放转换后的字符串 */
	//printf("outbuf=%s\n", outbuf);

	//存放转换后outbuf剩余的空间
	//printf("outlen=%d\n", *outlen);

	int i = 0;

	//for (i = 0; i < (outsize - (*outlen)); i++)
	{
		//printf("%2c", outbuf[i]);
		//printf("%x\n", outbuf[i]);
	}

	/* 关闭句柄 */
	iconv_close(cd);

	return 0;
}
__inline static
bool utf8_to_gb2312(char* inbuf, size_t* inlen, char* outbuf, size_t* outlen)
{
	/* 目的编码, TRANSLIT：遇到无法转换的字符就找相近字符替换
	 *           IGNORE ：遇到无法转换字符跳过*/
	char* encTo = "GB2312//IGNORE";
	/* 源编码 */
	char* encFrom = "UTF-8";

	/* 获得转换句柄
	 *@param encTo 目标编码方式
	 *@param encFrom 源编码方式
	 *
	 * */
	iconv_t cd = iconv_open(encTo, encFrom);
	if (cd == (iconv_t)-1)
	{
		perror("iconv_open");
	}

	/* 需要转换的字符串 */
	//printf("inbuf=%s\n", inbuf);

	/* 打印需要转换的字符串的长度 */
	//printf("inlen=%d\n", *inlen);

	/* 由于iconv()函数会修改指针，所以要保存源指针 */
	char* tmpin = inbuf;
	char* tmpout = outbuf;
	size_t insize = *inlen;
	size_t outsize = *outlen;

	/* 进行转换
	 *@param cd iconv_open()产生的句柄
	 *@param srcstart 需要转换的字符串
	 *@param inlen 存放还有多少字符没有转换
	 *@param tempoutbuf 存放转换后的字符串
	 *@param outlen 存放转换后,tempoutbuf剩余的空间
	 *
	 * */
	size_t ret = iconv(cd, &tmpin, inlen, &tmpout, outlen);
	if (ret == -1)
	{
		perror("iconv");
	}

	/* 存放转换后的字符串 */
	//printf("outbuf=%s\n", outbuf);

	//存放转换后outbuf剩余的空间
	//printf("outlen=%d\n", *outlen);

	int i = 0;

	//for (i = 0; i < (outsize - (*outlen)); i++)
	{
		//printf("%2c", outbuf[i]);
		//printf("%x\n", outbuf[i]);
	}

	/* 关闭句柄 */
	iconv_close(cd);

	return 0;
}
//////////////////////////////////////////////////////////////////////////////
typedef struct tm time_data_info;
__inline static
int conv_time(time_data_info& tdi, const time_t& tt)
{
	time_data_info* ptdi = localtime(&tt);
	if (ptdi == nullptr)
	{
		return (-1);
	}
	memcpy(&tdi, ptdi, sizeof(tdi));
	tdi.tm_year += 1900;
	tdi.tm_mon += 1;
	return 0;
}
//////////////////////////////////////////////////////////////////////////////
__inline static
int enum_file(std::unordered_map<std::string, std::string>& file_list, const std::string& path)
{
	struct dirent* pdirent = nullptr;
	DIR * pdir = opendir(path.c_str());
	if (pdir != nullptr)
	{
		while ((pdirent = readdir(pdir)) != nullptr)
		{
			if ((strlen(pdirent->d_name) == 1 && (*pdirent->d_name == '.')) ||
				(strlen(pdirent->d_name) == 2 && (*pdirent->d_name == '.') && (*(pdirent->d_name + 1) == '.')))
			{
				continue;
			}
			std::string newpath(MAX_PATH_LEN, '\0');
			snprintf((char *)newpath.c_str(), newpath.size(), "%s/%s", path.c_str(), pdirent->d_name);
			struct stat st = { 0 };
			stat(newpath.c_str(), &st);
			if (S_ISDIR(st.st_mode))
			{
				enum_file(file_list, newpath);
			}
			else
			{
				file_list.insert(std::unordered_map<std::string,std::string>::value_type(newpath, path));
			}
		}
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////////

__inline static 
size_t file_reader(char** ppdata, size_t& size, const std::string& filename, const std::string& mode = "rb")
{
#define DATA_BASE_SIZE	0x10000

	FILE* pF = 0;
	size_t sizeofread = 0;

	char* pdata = 0;

	pF = fopen(filename.c_str(), mode.c_str());
	if (pF)
	{
		size = 0;
		(*ppdata) = (char*)malloc((size + DATA_BASE_SIZE) * sizeof(char));
		while (!feof(pF))
		{
			sizeofread = fread((void*)((*ppdata) + size), sizeof(char), DATA_BASE_SIZE, pF);
			size += sizeofread;
			if (sizeofread >= DATA_BASE_SIZE)
			{
				break;
			}
			pdata = (*ppdata);
			(*ppdata) = (char*)realloc(pdata, (size + DATA_BASE_SIZE) * sizeof(char));
			if (!(*ppdata) || errno != 0)
			{
				if (pdata)
				{
					free(pdata);
					pdata = 0;
				}
				break;
			}
		}

		fclose(pF);
		pF = 0;
	}

	return size;

#undef DATA_BASE_SIZE
}

__inline static 
size_t file_reader(std::string& data, const std::string& filename, const std::string& mode = "rb")
{
#define DATA_BASE_SIZE	0x10000

	FILE* pF = 0;
	size_t size = 0;

	pF = fopen(filename.c_str(), mode.c_str());
	if (pF)
	{
		while (!feof(pF))
		{
			data.resize(data.size() + DATA_BASE_SIZE);
			size += fread((void*)(data.c_str() + data.size() - DATA_BASE_SIZE), sizeof(char), DATA_BASE_SIZE, pF);
		}
		data.resize(size);
		fclose(pF);
		pF = 0;
	}

	return size;

#undef DATA_BASE_SIZE
}

__inline static 
size_t file_writer(const std::string& data, const std::string& filename, const std::string& mode = "wb")
{
	FILE* pF = 0;
	size_t size = 0;

	pF = fopen(filename.c_str(), mode.c_str());
	if (pF)
	{
		size = fwrite((void*)(data.c_str()), sizeof(char), data.size(), pF);
		fclose(pF);
		pF = 0;
	}

	return size;
}

__inline static 
size_t file_writer(const char* data, size_t size, const std::string& filename, const std::string& mode = "wb")
{
	FILE* pF = 0;
	size_t ssize = 0;

	pF = fopen(filename.c_str(), mode.c_str());
	if (pF)
	{
		ssize = fwrite((void*)(data), sizeof(char), size, pF);
		fclose(pF);
		pF = 0;
	}

	return ssize;
}

//////////////////////////////////////////////////////////////////////////////

__inline static 
bool string_regex_valid(const std::string& data, const std::string& pattern)
{
	return std::regex_match(data, std::regex(pattern));
}
__inline static 
size_t string_regex_replace_all(std::string& result, std::string& data, const std::string& replace, const std::string& pattern, std::regex_constants::match_flag_type matchflagtype = std::regex_constants::match_default)
{
	try
	{
		data = std::regex_replace(data, std::regex(pattern), replace, matchflagtype);
	}
	catch (const std::exception & e)
	{
		result = e.what();
	}
	return data.length();
}
__inline static 
size_t string_regex_find(std::string& result, std::vector<std::vector<std::string>>& svv, const std::string& data, const std::string& pattern)
{
	std::smatch smatch;
	std::string::const_iterator ite = data.end();
	std::string::const_iterator itb = data.begin();
	try
	{
		//found
		while (std::regex_search(itb, ite, smatch, std::regex(pattern)))
		{
			if (smatch.size() > 1)
			{
				for (size_t stidx = svv.size() + 1; stidx < smatch.size(); stidx++)
				{
					svv.push_back(std::vector<std::string>());
				}
				for (size_t stidx = 1; stidx < smatch.size(); stidx++)
				{
					svv.at(stidx - 1).push_back(std::string(smatch[stidx].first, smatch[stidx].second));
					itb = smatch[stidx].second;
				}
			}
		}
	}
	catch (const std::exception & e)
	{
		result = e.what();
	}

	return svv.size();
}
__inline static 
std::string::size_type string_reader(std::string& result, const std::string& strData,
	const std::string& strStart, const std::string& strFinal, std::string::size_type stPos = 0,
	bool bTakeStart = false, bool bTakeFinal = false)
{
	std::string::size_type stRetPos = std::string::npos;
	std::string::size_type stStartPos = stPos;
	std::string::size_type stFinalPos = std::string::npos;

	stStartPos = strData.find(strStart, stStartPos);
	if (stStartPos != std::string::npos)
	{
		stRetPos = stFinalPos = strData.find(strFinal, stStartPos + strStart.length());
		if (stFinalPos != std::string::npos)
		{
			if (!bTakeStart)
			{
				stStartPos += strStart.length();
			}
			if (bTakeFinal)
			{
				stFinalPos += strFinal.length();
			}
			result = strData.substr(stStartPos, stFinalPos - stStartPos);
		}
	}

	return stRetPos;
}
__inline static 
std::string string_reader(const std::string& strData,
	const std::string& strStart, const std::string& strFinal,
	bool bTakeStart = false, bool bTakeFinal = false)
{
	std::string strRet = ("");
	std::string::size_type stStartPos = std::string::npos;
	std::string::size_type stFinalPos = std::string::npos;
	stStartPos = strData.find(strStart);
	if (stStartPos != std::string::npos)
	{
		stFinalPos = strData.find(strFinal, stStartPos + strStart.length());
		if (stFinalPos != std::string::npos)
		{
			if (!bTakeStart)
			{
				stStartPos += strStart.length();
			}
			if (bTakeFinal)
			{
				stFinalPos += strFinal.length();
			}
			strRet = strData.substr(stStartPos, stFinalPos - stStartPos);
		}
	}

	return strRet;
}
__inline static 
std::string string_replace_all(std::string& strData, const std::string& strDst, const std::string& strSrc, std::string::size_type stPos = 0)
{
	while ((stPos = strData.find(strSrc, stPos)) != std::string::npos)
	{
		strData.replace(stPos, strSrc.length(), strDst);
		stPos += strDst.length() - strSrc.length();
	}

	return strData;
}
__inline static 
size_t string_split_to_vector(std::vector<std::string>& sv, const std::string& strData, const std::string& strSplitter, std::string::size_type stPos = 0)
{
	std::string strTmp = ("");
	std::string::size_type stIdx = 0;
	std::string::size_type stSize = strData.length();

	while ((stPos = strData.find(strSplitter, stIdx)) != std::string::npos)
	{
		strTmp = strData.substr(stIdx, stPos - stIdx);
		if (!strTmp.length())
		{
			strTmp = ("");
		}
		sv.push_back(strTmp);

		stIdx = stPos + strSplitter.length();
	}

	if (stIdx < stSize)
	{
		strTmp = strData.substr(stIdx, stSize - stIdx);
		if (!strTmp.length())
		{
			strTmp = ("");
		}
		sv.push_back(strTmp);
	}

	return sv.size();
}
#endif // !__COMMON_H_
