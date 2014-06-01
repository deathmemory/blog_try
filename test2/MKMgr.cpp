#include "stdafx.h"
#include "resource.h"
#include "MKMgr.h"
#include "../publicdef/dmcode.hpp"
#include <Shellapi.h>

CMKMgr::CMKMgr(void)
{
}


CMKMgr::~CMKMgr(void)
{
}

BOOL CMKMgr::patchLauncher(){
	BOOL ret = FALSE;
	CDuiString localDir = CPaintManagerUI::GetInstancePath();
	CDuiString launcherPath = localDir + _T("LauncherTransit.dll");
	CDuiString newLauncherPath = localDir + _T("LauncherTransit.dll.bak");
	CopyFile(launcherPath, newLauncherPath, TRUE);	// 已存在则不复制
	DeleteFile(launcherPath);
	HRSRC hResource = ::FindResource(CPaintManagerUI::GetResourceDll(), MAKEINTRESOURCE(IDR_LAUNCHER), _T("DATA"));
	if( hResource == NULL ) return NULL;
	HGLOBAL hGlobal = ::LoadResource(CPaintManagerUI::GetResourceDll(), hResource);
	if( hGlobal == NULL ) {
		FreeResource(hResource);
		return FALSE;
	}
	LPVOID resBuffer = ::LockResource(hGlobal);
	DWORD sourceSize = ::SizeofResource(CPaintManagerUI::GetResourceDll(), hResource);
	HANDLE hfile = CreateFile(launcherPath, GENERIC_READ|GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != hfile){
		DWORD written = 0;
		WriteFile(hfile, "MZ", 2, &written, NULL);
		WriteFile(hfile, resBuffer, sourceSize, &written, NULL);
		CloseHandle(hfile);
		ret = TRUE;
	}
// 	m_pCallback = pCallback;
// 	if( !m_xml.LoadFromMem((BYTE*)::LockResource(hGlobal), ::SizeofResource(CPaintManagerUI::GetResourceDll(), hResource) )) return NULL;
	::FreeResource(hResource);
	return ret;
}

BOOL CMKMgr::patchLanguage(){
	CDuiString localDir = CPaintManagerUI::GetInstancePath();
	CDuiString languagePath = localDir + _T("Language\\en_us.txt");
	DeleteFile(languagePath);
	HRSRC hResource = ::FindResource(CPaintManagerUI::GetResourceDll(), MAKEINTRESOURCE(IDR_LANGUAGE), _T("DATA"));
	if( hResource == NULL ) return NULL;
	HGLOBAL hGlobal = ::LoadResource(CPaintManagerUI::GetResourceDll(), hResource);
	if( hGlobal == NULL ) {
		FreeResource(hResource);
		return FALSE;
	}
	LPVOID resBuffer = ::LockResource(hGlobal);
	DWORD sourceSize = ::SizeofResource(CPaintManagerUI::GetResourceDll(), hResource);
	HANDLE hfile = CreateFile(languagePath, GENERIC_READ|GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != hfile){
		DWORD written = 0;
		WriteFile(hfile, resBuffer, sourceSize, &written, NULL);
		CloseHandle(hfile);
	}
	::FreeResource(hResource);
	return TRUE;
}

void CMKMgr::patchAndRunMK(){
	this->patchLanguage();
	if ( this->patchLauncher() ){
		CDuiString curDir = CPaintManagerUI::GetInstancePath();
		CDuiString strMKCMD = curDir + _T("mk.exe updater_cancel");
		STARTUPINFO si = {sizeof(STARTUPINFO)};
		PROCESS_INFORMATION pi = {NULL};
		if ( CreateProcess(NULL, (LPTSTR)strMKCMD.GetData(), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi) ){
			/*
			0123A2C5     /0F85 DA090000          jnz     0123ACA5                         ;  直接改成请求失败
			0123A2CB  |. |8D4C24 64              lea     ecx, dword ptr [esp+64]
			0123A2CF  |. |51                     push    ecx
			0123A2D0  |. |E8 7B650100            call    01250850
			0123A2D5  |. |6A 01                  push    1
			0123A2D7  |. |6A 00                  push    0
			0123A2D9  |. |56                     push    esi
			0123A2DA  |. |8D4C24 70              lea     ecx, dword ptr [esp+70]
			0123A2DE  |. |C68424 C4000000 02     mov     byte ptr [esp+C4], 2
			0123A2E6  |. |E8 85790100            call    01251C70
			0123A2EB  |. |56                     push    esi
			0123A2EC  |. |E8 2BA40400            call    0128471C

			0F?????????? 8D?????? 51 E8???????? 6A?? 6A?? 56 8D?????? C6?????????????? E8	v40
			*/
			std::vector<PAGEINFO> pageInfoVct;
			HANDLE hProc = pi.hProcess;
			DWORD addr = (DWORD) dmcode::procDimFindCode(hProc/*6252*/, PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY, 
				"0F?????????? 8D?????? 51 E8???????? 6A?? 6A?? 56 8D?????? C6?????????????? E8", pageInfoVct);
			if (addr){
				if ( WriteProcessMemory(hProc, (LPVOID)(addr + 1), "\x85", 1, NULL) ){
					// successed
				}else{
					MessageBox(NULL, _T("patch 失败了，请反馈给作者"), NULL, MB_OK);
				}
			}
			ResumeThread(pi.hThread);
			ResumeThread(hProc);
			CloseHandle(pi.hThread);
			CloseHandle(hProc);
		}else{
			MessageBox(NULL, _T("运行错误，请保证MK汉化器与MK同目录，并且MK处于关闭状态"), NULL, MB_OK);
		}
	}
}
void CMKMgr::doUpdate(){
	CDuiString localDir = CPaintManagerUI::GetInstancePath();
	CDuiString updatePath = localDir + _T("update.exe");
	if ( INVALID_FILE_ATTRIBUTES == GetFileAttributes(updatePath) )
		updatePath = localDir + _T("backUpdate.exe");
	if ( INVALID_FILE_ATTRIBUTES == GetFileAttributes(updatePath) )
		updatePath = localDir + _T("newUpdate.exe");
	if ( INVALID_FILE_ATTRIBUTES != GetFileAttributes(updatePath) ){
		ShellExecute(NULL, _T("open"), updatePath, NULL, NULL, SW_SHOWNORMAL);
	}else{
		CDuiString errStr; errStr.Format(_T("升级错误！err:%d"), GetLastError());
		MessageBox(NULL, errStr, _T("错误"), MB_OK);
	}
}
void CMKMgr::thanksList(){
	CDuiString str = _T("感谢：\n");
	str +=	_T("平   台 -- NGA\n");
	str +=	_T("人气贴 -- 上古战魂魔腾\n");
	str +=	_T("汉   化 -- 白小痴\n");

	MessageBox(NULL, str, _T("感谢"), MB_OK);
}
void CMKMgr::aboutMe(){
	CDuiString myWeibo = _T("http://weibo.com/zsmfshi");
	ShellExecute(NULL, _T("open"), myWeibo, NULL, NULL, SW_SHOWNORMAL);
}
void CMKMgr::faq(){
	CDuiString myFAQ = _T("http://nga.178.com/read.php?tid=7065154&page=e&topid=131979461#pid131979461Anchor");
	ShellExecute(NULL, _T("open"), myFAQ, NULL, NULL, SW_SHOWNORMAL);
}