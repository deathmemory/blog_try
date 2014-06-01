#pragma once
#include <Tlhelp32.h>
#include <tchar.h>

namespace ProcessHelper
{

	static BOOL ScanDllInProcess(DWORD dwPID, LPCTSTR strDllName)
	{
		BOOL bRet = FALSE;
		MODULEENTRY32 pe32;
		// ��ʹ������ṹ֮ǰ�����������Ĵ�С
		pe32.dwSize = sizeof(pe32); 
		// ������������ģ����һ������
		//276Ϊĳ���̵�ID
		HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
		if(hProcessSnap == INVALID_HANDLE_VALUE)
		{       
			//��������ʧ��
			return bRet;  
		}

		// �������̿��գ�������ʾÿ�����̵���Ϣ
		BOOL bMore = Module32First(hProcessSnap, &pe32);
		while(bMore){       
			// 			printf("\n[DLL NAME]\t%s\n",pe32.szModule);
			// 			printf("[DLL PATH]\t%s\n",pe32.szExePath);
			if ( _tcsicmp(strDllName, pe32.szModule) == 0){
				bRet = BOOL(pe32.modBaseAddr);
				break;
			}

			bMore = Module32Next(hProcessSnap, &pe32);
		}
		// ��Ҫ���������snapshot����
		CloseHandle(hProcessSnap);

		return bRet;
	}

	static BOOL ScanDllInProcess(LPCTSTR strProcName, LPCTSTR strDllName)
	{
		BOOL bRet = FALSE;
		DWORD dwPID = 0;
		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
		PROCESSENTRY32* processInfo = new PROCESSENTRY32;
		processInfo->dwSize = sizeof(PROCESSENTRY32);
		while(Process32Next(hSnapShot,processInfo)!=FALSE){
			if(_tcsicmp(strProcName,processInfo->szExeFile) == 0){
				dwPID = processInfo->th32ProcessID;
				break;
			}
		}
		CloseHandle(hSnapShot);
		delete processInfo;

		if (dwPID){
			bRet = ScanDllInProcess(dwPID, strDllName);
		}
		return bRet;
	}

	static DWORD ScanProcess(LPCTSTR strProcName)
	{
		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
		PROCESSENTRY32* processInfo = new PROCESSENTRY32;
		processInfo->dwSize = sizeof(PROCESSENTRY32);
		DWORD dwRet = 0;
		while(Process32Next(hSnapShot,processInfo)!=FALSE){
			if(_tcsicmp(strProcName,processInfo->szExeFile) == 0){
				dwRet = processInfo->th32ProcessID;
				delete processInfo;
				return dwRet;
			}
		}
		CloseHandle(hSnapShot);
		delete processInfo;
		return dwRet;
	}

	static BOOL KillProcByName(LPCTSTR strProcName)
	{
		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
		//���ڻ�������н��̵���Ϣ��
		//����hSnapShot�г�ȡ���ݵ�һ��PROCESSENTRY32�ṹ��
		//����ṹ������һ�����̣���ToolHelp32 API��һ���֡�
		//��ȡ���ݿ�Process32First()��Process32Next()������������
		//�������Process32Next()������ԭ���ǣ�
		//BOOL WINAPI Process32Next(HANDLE hSnapshot,LPPROCESSENTRY32 lppe);
		PROCESSENTRY32* processInfo = new PROCESSENTRY32;
		// ��������PROCESSENTRY32��dwSize��Ա��ֵ ;
		processInfo->dwSize = sizeof(PROCESSENTRY32);
		//���ｫ���վ����PROCESSENTRY32�ṹ����Process32Next()��
		//ִ��֮��PROCESSENTRY32 �ṹ����ý��̵���Ϣ��ѭ��������ֱ����������FALSE��
		//printf("****************��ʼ�оٽ���****************\n");
		while(Process32Next(hSnapShot,processInfo)!=FALSE){
			// 		char szBuf[MAX_PATH];
			// 		wcstombs(szBuf, processInfo->szExeFile, MAX_PATH);
			if(_tcsicmp(strProcName, processInfo->szExeFile) == 0){
				int ID = processInfo->th32ProcessID;
				HANDLE hProcess;
				// ���������ú��� TerminateProcess()��ֹ���̣�
				// ����������PROCESS_ALL_ACCESS
				hProcess = OpenProcess(PROCESS_ALL_ACCESS,TRUE,ID);
				if(hProcess == NULL)
				{
					//qDebug("�򿪽���ʧ�� !\n");
					delete processInfo;
					return FALSE;
				}
				TerminateProcess(hProcess,0);
				CloseHandle(hProcess);
			}
		}
		CloseHandle(hSnapShot);
		delete processInfo;
		return TRUE;
	}

	static BOOL SupendProcess(LPCTSTR lpszProcName, DWORD dwExceptThdId = -1)
	{
		THREADENTRY32 th32;
		th32.dwSize = sizeof(th32);
		BOOL bRet = TRUE;
		DWORD dwPid = ScanProcess(lpszProcName);
		if (0 == dwPid)
			return FALSE;
		HANDLE hThreadSnap=::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
		if( INVALID_HANDLE_VALUE != hThreadSnap )
		{
			if ( Thread32First(hThreadSnap, &th32) )
			{
				do
				{
					if(th32.th32OwnerProcessID == dwPid && th32.th32ThreadID != dwExceptThdId)
					{ 
						HANDLE oth =  OpenThread (THREAD_ALL_ACCESS,FALSE,th32.th32ThreadID);
						if(-1 == (::SuspendThread(oth))){
							bRet = FALSE;
						}
						CloseHandle(oth);
					}
				}while(::Thread32Next(hThreadSnap,&th32));
			}
			else
				bRet = FALSE;
		}
		else
			bRet = FALSE;
		::CloseHandle(hThreadSnap);
		return bRet;
	}

	static BOOL ResumeProcess(LPCTSTR lpszProcName, DWORD dwExceptThdId = -1)
	{
		THREADENTRY32 th32;
		th32.dwSize = sizeof(th32);
		BOOL bRet = TRUE;
		DWORD dwPid = ScanProcess(lpszProcName);
		if (0 == dwPid)
			return FALSE;
		HANDLE hThreadSnap=::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
		if( INVALID_HANDLE_VALUE != hThreadSnap )
		{
			if ( Thread32First(hThreadSnap, &th32) )
			{
				do
				{
					if(th32.th32OwnerProcessID == dwPid && th32.th32ThreadID != dwExceptThdId)
					{ 
						DWORD dwCount = 0;
						HANDLE oth =  OpenThread (THREAD_ALL_ACCESS,FALSE,th32.th32ThreadID);
						while( (dwCount = ::ResumeThread(oth)) > 0);
						
						CloseHandle(oth);
					}
				}while(::Thread32Next(hThreadSnap,&th32));
			}
			else
				bRet = FALSE;
		}
		else
			bRet = FALSE;
		::CloseHandle(hThreadSnap);
		return bRet;
	}

	static BOOL InjectProcess(HANDLE hProcess, LPCTSTR lpszDllPath){
		LPVOID lpBuffer = VirtualAllocEx(hProcess, NULL, (_tcslen(lpszDllPath) + 1) * sizeof(TCHAR), MEM_COMMIT, PAGE_READWRITE);
		if (lpBuffer){
			DWORD dwWritten;
			WriteProcessMemory(hProcess, lpBuffer, lpszDllPath, (_tcslen(lpszDllPath) + 1) * sizeof(TCHAR), &dwWritten);
			DWORD dwThreadId;
			HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, lpBuffer, 0, &dwThreadId);
			if (hThread){
				WaitForSingleObject(hThread, INFINITE);
				CloseHandle(hThread);
			}
			VirtualFreeEx(hProcess, lpBuffer, 0, MEM_DECOMMIT);
			return TRUE;
		}
		return FALSE;
	}

	static BOOL InjectProcess(LPCTSTR procName, LPCTSTR lpszDllPath){
		DWORD pid = ScanProcess(procName);
		if ( 0 != pid ){
			HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
			if (hProc){
				return InjectProcess(hProc, lpszDllPath);
				CloseHandle(hProc);
			}
		}
		return FALSE;
	}
};