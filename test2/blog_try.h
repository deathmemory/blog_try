#pragma once
#include "StdAfx.h"
//#include "resource.h"
#include "MKMgr.h"
#include "IrregularWindow.h"

class CMainFrame :
	public DuiLib::CWindowWnd,public INotifyUI//Ӧ�ó��򴰿���CWindowWnd
{
public:
	CMainFrame(void);
	~CMainFrame(void);
public:
	UINT const GetClassStyle();
	LPCTSTR GetWindowClassName() const;

	void OnPrepare();
	void Notify(TNotifyUI& msg);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	CPaintManagerUI m_pm;//��ؼ����ƺ���Ϣ�����������
	CIrregularWindow *m_pBackWnd;
	CMKMgr	m_MKMgr;
};











