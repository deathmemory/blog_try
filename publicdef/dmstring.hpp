/************************************************************************/
/*	content :	Solve the transformation between the string [W2A/A2W]
	HowToUse:	Create a object DMW2A or DMA2W , you can call [obj.getBuffer()]
				to get the result.
				In loop use [obj.reset(src, cp)] to reConvert string.
				The [obj] will release buffer by Destructor.
	author	:	Simon Xie(DeathMemory)
	date	:	2013/03/25              

	example	:	
	(1)
	DMW2A obj(L"string", CP_UTF8);
	CHAR* lpszResult = obj.getBuffer();
	(2)
	DMW2A obj(NULL);
	CHAR* lpszResult = NULL;
	for ( int i = 0; i < 2; ++ i )
	{
		if (i)
			obj.reset(L"string 1");
		else
			obj.reset(L"string 2");
		printf( "%s", obj.getBuffer() );
	}
*/
/************************************************************************/

#pragma once
#include <time.h>
#include <vector>

#ifdef _UNICODE
#define DMT2A			DMW2A
//#else
//...
#endif

template <typename SrcChar, typename TgtChar>
class DMSTRING
{
public:
	TgtChar* m_lpsz;
	DMSTRING() : m_lpsz(NULL)
	{
	}
	virtual ~DMSTRING()
	{
		release();
	}
	TgtChar* getBuffer()
	{
		return m_lpsz;
	}
	void reset(const SrcChar* lpsz, UINT codePage = CP_UTF8)
	{
		release();
		if ( ! lpsz )
			return;

		int method = sizeof(SrcChar);
		int len = 0;
		void* lpcharFuncAddr = NULL;
		if ( 1 != method )	// src char == wchar ; main's wchar to char
			lpcharFuncAddr = (void*)&WideCharToMultiByte;
		else
			lpcharFuncAddr = (void*)&MultiByteToWideChar;
		if( 1 != method )
			__asm{
				push NULL
				push NULL
			}
		__asm
		{
			push 0
			push NULL
			push -1
			push lpsz
			push 0
			push codePage
			call lpcharFuncAddr
			mov len, eax
		}
		//len = lpcharFuncAddr(codePage, 0, lpsz, -1, NULL, 0);
		m_lpsz = new TgtChar[len+1];
		ZeroMemory(m_lpsz, (len+1)*sizeof(TgtChar));
		void* lptsz = (void*)m_lpsz;

		if( 1 != method )
			__asm{
				push NULL
				push NULL
		}
		//lpcharFuncAddr(codePage, 0, lpsz, -1, m_lpsz, len);
		__asm
		{
			push len
			push lptsz
			push -1
			push lpsz
			push 0
			push codePage
			call lpcharFuncAddr
		}
	}
	operator TgtChar*() const   // as a C string
	{
		return m_lpsz;
	}
protected:
	void release()
	{
		if ( m_lpsz )
			delete [] m_lpsz, m_lpsz = NULL;
	}
};

class DMW2A : public DMSTRING<WCHAR, char>
{
public:
	DMW2A(const WCHAR* lpWsz, UINT codePage = CP_UTF8 )
	{
		reset(lpWsz, codePage);
	}
};

class DMA2W : public DMSTRING<char, WCHAR>
{
public:
	DMA2W(const char* lpsz, UINT codePage = CP_UTF8 )
	{
		reset(lpsz, codePage);
	}
};

class DMSTRCVR
{
private:
	std::vector<DMA2W*> m_vctA2W;
	std::vector<DMW2A*> m_vctW2A;
public:
	~DMSTRCVR(){
		for (int i = 0; i < m_vctA2W.size(); ++i){
			delete m_vctA2W[i];
		}
		for (int i = 0; i < m_vctW2A.size(); ++i){
			delete m_vctW2A[i];
		}
	}

	WCHAR* dmA2W(const char* lpsz, UINT codePage = CP_UTF8 ){
		DMA2W* _a2w = new DMA2W(lpsz, codePage);
		m_vctA2W.push_back(_a2w);
		return _a2w->getBuffer();
	}

	char* dmW2A(const WCHAR* lpWsz, UINT codePage = CP_UTF8 ){
		DMW2A* _w2a = new DMW2A(lpWsz, codePage);
		m_vctW2A.push_back(_w2a);
		return _w2a->getBuffer();
	}

	char* dmA2A(const char* lpsz, UINT codePageSrc = CP_ACP, UINT codePageTgt = CP_UTF8  ){
		return dmW2A( dmA2W(lpsz, codePageSrc), codePageTgt );
	}
};

namespace DMSTR_EX
{
	// return len
	static UINT GetRandomStr(LPTSTR lpszOut, DWORD dwBufLen, DWORD dwLowRangeLen = 6, DWORD dwMaxRangeLen = 10)
	{
		if ( ! lpszOut )
			return 0;
		srand( (unsigned)time( NULL ) );
		UINT unLen = rand() % (dwMaxRangeLen + 1);
		if ( unLen < dwLowRangeLen )
			unLen = dwLowRangeLen;
		for (int i = 0; i < dwBufLen && i < unLen; ++i)
		{
			lpszOut[i] = 97 + (rand() % 26);
		}
		return (unLen > dwBufLen) ? dwBufLen : unLen;
	}

	static char* memstr(char* src, char* tag, int srcSize)
	{
		char* _result = NULL;
		for ( int i = 0, len = lstrlenA(tag); i < srcSize - len; ++ i)
			if ( memcmp(src + i, tag, len) == 0 ){
				_result = src + i;
				break;
			}
		return _result;
	}
};

#define USES_DMSTRCOVERT							DMSTRCVR dmStrCvr;
#define DA2W(lpsz, codePage)						dmStrCvr.dmA2W(lpsz, codePage)							
#define DW2A(lpWsz, codePage)						dmStrCvr.dmW2A(lpWsz, codePage)	
#define DA2A(lpsz, codePageSrc, codePageTgt)		dmStrCvr.dmA2A(lpsz, codePageSrc, codePageTgt)
#ifdef _UNICODE
#	define DA2T(lpsz, codePage)						DA2W(lpsz, codePage)
#	define DT2A(lpWsz, codePage)					DW2A(lpWsz, codePage)
#else
#	define DA2T(lpsz, codePage)						lpsz
#	define DT2A(lpsz, codePage)						lpsz
#endif