#include "stdafx.h"
#include "resource.h"
#include "MKMgr.h"
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
		WriteFile(hfile, resBuffer, sourceSize, &written, NULL);
		CloseHandle(hfile);
		ret = TRUE;
	}
// 	m_pCallback = pCallback;
// 	if( !m_xml.LoadFromMem((BYTE*)::LockResource(hGlobal), ::SizeofResource(CPaintManagerUI::GetResourceDll(), hResource) )) return NULL;
	::FreeResource(hResource);
	return ret;
}

void CMKMgr::patchAndRunMK(){
	if ( this->patchLauncher() ){
		CDuiString strMKPath = _T("mk.exe");
		if ((DWORD)ShellExecute(NULL, _T("open"), strMKPath, NULL, NULL, SW_SHOWNORMAL) <= 32){
			MessageBox(NULL, _T("运行错误"), NULL, MB_OK);
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
