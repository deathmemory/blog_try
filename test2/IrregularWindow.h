#ifndef _IRREGULARWINDOW_H_
#define _IRREGULARWINDOW_H_
/**
  * 不规则窗体类
*/

#include <windows.h>
#include <string>
#include <tchar.h>
#include <assert.h>

#ifndef __String
	#ifdef UNICODE
		#define __String wstring
	#else
		#define __String string
	#endif
#endif


using namespace std;

const DWORD c_dwTransColor = 0x00ff00ff;

class CIrregularWindow
{
public:

	CIrregularWindow(const TCHAR* pBackImgFullPath, TCHAR* sourceType = NULL, DWORD dwAttachWndTransColor = c_dwTransColor);

	//附加窗体
	bool AttachWindow(HWND hWnd);

	//背景图片(全路径 png)
	void SetBackground(const TCHAR* pBackImgFullPath);

	//一般情况 是不用调这个函数的 但是如果出现 
	//被附加的窗体的子窗体 显示不完全的情况 则需要设置这个值
	void SetAttachWndTransColor(DWORD dwColor);

	HWND GetHandle(){return m_hWnd;}

	void CenterWindow();

protected:
	Image* GetImage();

	virtual bool RegisterWindowClass();

	virtual bool Create();

public:

	static void InitGDIplus();

	static void UnInitGDIplus();

private:

	static LRESULT CALLBACK  __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK __HookProc(int nCode,WPARAM wParam,LPARAM lParam);

	static ULONG_PTR gdiplusToken;

	static HHOOK hhk;

protected:

	HWND m_hWnd;

	//被附加的窗体的句柄
	HWND m_hAttachWnd;

	DWORD m_dwAttachWndTransColor;

	//资源类型，如果指定了资源类型则表示从资源中获取图片
	TCHAR* m_sourceType;

	DWORD m_dwResID;
	//背景图片(png)
	__String m_strBackImg;
};

#endif