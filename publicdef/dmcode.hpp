//#pragma once
#ifndef __DMCODE_HPP
#define __DMCODE_HPP

#include <windows.h>
#include <assert.h>
#include <Tlhelp32.h> 
#include <atltrace.h>

typedef struct tagPageInfo
{
	void* addr;
	int size;
}PAGEINFO, *LPPAGEINFO;

namespace dmcode {
#define CHAR2BYTE(x) ((x)<=0x39?(x-0x30):((x)<0x61?(x)-0x37:(x)-0x57))
#define HEX2BYTE(h,l) (CHAR2BYTE(h)<<4|CHAR2BYTE(l))
#define DIMFLAG				'?'

	static void write(unsigned int location, unsigned char* data, unsigned int length){
		DWORD oldProtect;
		unsigned char* code = (unsigned char*)location;
		VirtualProtect(code, length, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(code, data, length);
		VirtualProtect(code, length, oldProtect, &oldProtect);
	}

	template<class T> static void write(unsigned int location, T data){
		write(location, (unsigned char*)&data, sizeof(T));
	}

	static void read(unsigned int location, unsigned char* buffer, unsigned int length){
		DWORD oldProtect;
		unsigned char* code = (unsigned char*)location;
		VirtualProtect(code, length, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(buffer, code, length);
		VirtualProtect(code, length, oldProtect, &oldProtect);
	}

	template<class T> static T read(unsigned int location){
		T tmp;
		read(location, (unsigned char*)&tmp, sizeof(T));
		return tmp;
	}

	template <class T> static void jmpHook(unsigned int location, T cfunc, unsigned int nops = 0){
		DWORD oldProtect;
		unsigned char* code = (unsigned char*)location;
		VirtualProtect(code, 5 + nops, PAGE_EXECUTE_READWRITE, &oldProtect);
		code[0] = 0xE9;
		*reinterpret_cast<int*>(code+1) = (int)cfunc - ((int)code + 5);
		if(nops > 0) memset(code + 5, 0x90, nops);
		VirtualProtect(code, 5 + nops, oldProtect, &oldProtect);
	}

	template <class T> static unsigned int callRedirect(unsigned int location, T cfunc){
		int calldiff = read<int>(location + 1);
		unsigned int prevfunc = calldiff + location + 5;
		unsigned int newfunc = (unsigned int)cfunc;
		calldiff = newfunc - (location + 5);
		write<int>(location + 1, calldiff);
		return prevfunc;
	}

	static bool isHexChar(char chr){
		if(chr>=0x30 && chr<=0x39) return true;
		if(chr>=0x41 && chr<=0x46) return true;
		if(chr>=0x61 && chr<=0x66) return true;
		return false;
	}

	static char getDimChar(char dimchar){
		if (isHexChar(dimchar)){
			return dimchar;
		}else if (DIMFLAG == dimchar){
			return '0';
		}else
			return 0;
	}

	static unsigned int decodeHex(const char* pData,char* outbuf,const int buflen){
		unsigned int decodesize = 0;
		int inlen = strlen(pData);
		char* nspbuf = new char[inlen];
		memset(nspbuf, 0, inlen);
		for (int i = 0, j = 0; i < inlen; ++ i){	// no space
			if ( ' ' == pData[i] ){
				++ j; continue;
			}
			nspbuf[i-j] = pData[i];
		}
		char chr1 = 0, chr2 = 0;
		char *pBinPtr = const_cast<char*>(nspbuf);
		char *end = pBinPtr+inlen;
		for (pBinPtr; pBinPtr < end && decodesize < buflen; pBinPtr += 2, ++ decodesize){
			chr1 = getDimChar(*pBinPtr);
			chr2 = getDimChar(*(pBinPtr+1));
			if(!chr1 || !chr2) break;
			*(outbuf++) = HEX2BYTE(chr1,chr2);
		}
		if(nspbuf)	delete nspbuf;
		return decodesize;
	}

	static int trim(const char* pData,char* outbuf,const __out int buflen){
		int inlen = strlen(pData);
		int j = 0;
		for (int i = 0; i < inlen; ++ i){	// no space
			if ( ' ' == pData[i] ){
				++ j; continue;
			}
			if ( i - j < buflen )
				outbuf[i-j] = (pData[i] <= 0x39)|| DIMFLAG == pData[i] ? pData[i] : pData[i] & 0xDF;	//0xff-0x20
			else
				break;
		}
		return (inlen - j) < buflen ? (inlen - j) : buflen;
	}

	static int FindModule(const char* module, int* location = NULL, int* size = NULL){ 
		HMODULE hModule = GetModuleHandleA(module);
		int imagesize = 0;
		if (hModule){
			__asm{
				push eax;
				push ebx;
				mov ebx,hModule;
				mov eax,[ebx+0x3c];			//pe header
				lea eax,[ebx+eax+0x50];		//image size ptr
				mov eax,[eax]				//get size value
				mov imagesize,eax;
				pop ebx;
				pop eax;
			};
			if (location)	*location	= (int)hModule;
			if (size)		*size		= imagesize;
		}
		return imagesize;
	}
#define PATCHCHAR(srcbuff)						(srcbuff > 9 ? srcbuff - 9 + 0x40 : srcbuff + 0x30)
bool inline __PATCHBUFF(char srcbuff, char* dimbuff){
	bool bret = (DIMFLAG == *(char*)dimbuff || PATCHCHAR(srcbuff) == *(char*)dimbuff);
	return	bret;
}
#define PATCHBUFF(srcbuff, dimbuff)				( __PATCHBUFF((srcbuff & 0xf0)>>4, (char*)dimbuff) && __PATCHBUFF(srcbuff & 0x0f, ((char*)dimbuff)+1))
	static char* dimFindCode(const char* code,const char* startaddr,const unsigned int rangesize){
		char* res = NULL;
		char buff[MAX_PATH] = {0};
		int decsize = trim(code, buff, MAX_PATH);
		DWORD oldprot = 0;
		MEMORY_BASIC_INFORMATION mbi;
		char* pbinptr = (char*)startaddr;
		int tregsize = 0;
		__try{
			while (!res && (pbinptr - startaddr < rangesize) && VirtualQuery(pbinptr, &mbi, sizeof(mbi)) ){
				tregsize = mbi.RegionSize - (pbinptr - mbi.BaseAddress);
				if (tregsize > rangesize)	tregsize = rangesize;
				if (MEM_COMMIT != mbi.State || 0 == mbi.Protect || (PAGE_GUARD & mbi.Protect) != 0 || (PAGE_NOACCESS & mbi.Protect) != 0){
					pbinptr += tregsize;
					continue;
				}
				for (int i = 0; !res && i < tregsize - decsize; ++ i){
					if ( PATCHBUFF(*(pbinptr + i), buff) ){
						for (int j = 2; j < decsize; j += 2){
							if ( ! PATCHBUFF(*(pbinptr + i + j/2), buff + j) ){
								break;
							}else if (j + 2 == decsize){
								res = (char*)(pbinptr + i);
								break;
							}
						}
					}
				}
				pbinptr += tregsize;
			}
		}__except(NULL, EXCEPTION_EXECUTE_HANDLER){
			assert(!"find code exception");
			return NULL;
		}
		return res;
	}

	static char* dimFindCode(const char* code,const char* modename = NULL){
		char* startaddr = NULL;
		int modsize = FindModule(modename, (int*)&startaddr);
		if (modsize > 0){
			return dimFindCode(code, startaddr, modsize);
		}
		return 0;
	}

	char* procDimFindCode(HANDLE pHandle, DWORD PAGE_what, const char* code, std::vector<PAGEINFO>& pageInfoVct){
		MEMORY_BASIC_INFORMATION mbInfo={NULL};
		BYTE* MemBaseStart = NULL;
		PAGEINFO pageinfo;
		DWORD result = 0;
		if (pageInfoVct.size() == 0){
			while (VirtualQueryEx(pHandle, MemBaseStart, &mbInfo, sizeof(MEMORY_BASIC_INFORMATION))){
				result = mbInfo.AllocationProtect & PAGE_what;				
				if(result){
					/*if(!(mbInfo.Type & SEC_IMAGE))*/{
						pageinfo.addr = mbInfo.BaseAddress;
						pageinfo.size = mbInfo.RegionSize;
						pageInfoVct.push_back(pageinfo);
					}
				}
				if ((DWORD)mbInfo.BaseAddress == 0xce0000){
					__asm nop
				}
				MemBaseStart =(BYTE*)((DWORD)mbInfo.BaseAddress + mbInfo.RegionSize);
			}
		}
		__try{
			for (int i = 0, count = pageInfoVct.size(); i < count; ++i){
				void* addr = pageInfoVct[i].addr;
				int size = pageInfoVct[i].size;
				DWORD readbytes = 0;
				char* buff = new char[size];
				if (buff && ReadProcessMemory(pHandle, addr, buff, size, &readbytes)){
					result = (DWORD)dimFindCode(code, buff, readbytes);
					delete buff;	// 需要的是 buff 的数值, 所以释放空间没有影响
					if (result != 0){
						result -= (DWORD)buff;
						return (char*)((DWORD)addr + result);
					}
				}
			}
		}__except(NULL, EXCEPTION_EXECUTE_HANDLER){
			assert("dimFindeCode except 1");
			CloseHandle(pHandle);
			return NULL;
		}
		return NULL;
	}

	char* procDimFindCode(DWORD pid, DWORD PAGE_what, const char* code, std::vector<PAGEINFO>& pageInfoVct)
	{
		char* result = NULL;
		HANDLE pHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);	// 严格控制权限, 避免无意中调用了内存写入等操作
		if(INVALID_HANDLE_VALUE == pHandle)
			return NULL;
		result = procDimFindCode(pHandle, PAGE_what, code, pageInfoVct);
		CloseHandle(pHandle);
		return result;
	}

	static char* procDimFindCode(TCHAR* procName, DWORD PAGE_what, const char* code, std::vector<PAGEINFO>& pageInfoVct, __out DWORD* pdwPid = NULL){
		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
		PROCESSENTRY32 processInfo = {0};
		processInfo.dwSize = sizeof(PROCESSENTRY32);
		DWORD pid = -1;
		while(Process32Next(hSnapShot, &processInfo)!=FALSE){
			if(_tcsicmp(processInfo.szExeFile, procName) == 0){
				pid = processInfo.th32ProcessID;
				break;
			}
		}
		CloseHandle(hSnapShot);
		if (-1 == pid){
			return 0;
		}else{
			if (pdwPid)	*pdwPid = pid;
			return procDimFindCode(pid, PAGE_what, code, pageInfoVct);
		}
	}

#define PTR_ARG '@'
#define FLG_BEGIN '['
#define FLG_END ']'
#define OP_ADD '+'
#define OP_SUB '-'
#define OP_MUL '*'
#define OP_DIV '/'
	DWORD addrArithmetic(HANDLE hproc, DWORD addr, char* operation){		//[[@]+0x40]+0x10 - @为 addr的变量
		char buff[MAX_PATH] = {0};
		int decsize = trim(operation, buff, MAX_PATH);
		char* cell = strtok(buff, "]");
		int tmp = 0;
		__try{
			while (cell){
				int celllen = strlen(cell);
				BOOL patchedOP = TRUE;
				for (int i = 0; i < celllen; ++i){	// 当前地址运算
					if ( cell[i] == FLG_BEGIN || cell[i] == PTR_ARG ){
						continue;
					}
					switch(cell[i]){
					case OP_ADD:{
						tmp = strtol(cell + i + 1, NULL, 16);
						addr += tmp;
								}break;
					case OP_SUB:{
						tmp = strtol(cell + i + 1, NULL, 16);
						addr -= tmp;
								}break;
					case OP_MUL:{
						tmp = strtol(cell + i + 1, NULL, 16);
						addr *= tmp;
								}break;
					case OP_DIV:{
						tmp = strtol(cell + i + 1, NULL, 16);
						addr /= tmp;
								}break;
					default:
						patchedOP = FALSE;
					}
					if (patchedOP)
						break;
				}
				// 读取当前地址的内存数值
				if ( ! ReadProcessMemory(hproc, LPVOID(addr), &addr, sizeof(DWORD), NULL) )
					return 0;
				cell = strtok(NULL, "]");
			}
		}__except(NULL, EXCEPTION_EXECUTE_HANDLER){
			assert("addrArithmetic except");
			return 0;
		}
		return addr;
	}
};
#endif //__DMCODE_HPP