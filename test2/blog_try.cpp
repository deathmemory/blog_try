// blog_try.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "resource.h"
#include "blog_try.h"

CMainFrame::CMainFrame(void)
{
	CDuiString strResourcePath = CPaintManagerUI::GetInstancePath();
	CIrregularWindow::InitGDIplus();

	TCHAR* test = MAKEINTRESOURCE(IDB_BACKGROUND);
	m_pBackWnd = new CIrregularWindow(test, _T("PNG"));

	assert(m_pBackWnd != NULL && _T("new CIrregularWindow() 失败!"));

	if(m_pBackWnd)
	{
		HWND hBkWnd = m_pBackWnd->GetHandle();
	}
}

CMainFrame::~CMainFrame(void)
{
}
LPCTSTR CMainFrame::GetWindowClassName() const //重写获取类名函数，也就是指定要新建的窗口类的类名
{ 
	return _T("UIMainFrame"); 
};

UINT const CMainFrame::GetClassStyle()  //重写这个函数，来指定窗口样式
{ 
	return UI_CLASSSTYLE_FRAME | CS_DBLCLKS; 
};

void CMainFrame::Notify(TNotifyUI& msg)//处理窗口通知消息，响应用户的输入
{
	CDuiString name = msg.pSender->GetName();
	if (msg.sType == _T("link")){
		if (name == _T("startmkcn")){
			m_MKMgr.patchAndRunMK();
		}else if (name == _T("update")){
			m_MKMgr.doUpdate();
		}else if (name == _T("thankslist")){
			m_MKMgr.thanksList();
		}else if (name == _T("aboutme")){
			m_MKMgr.aboutMe();
		}else if (name == _T("faq")){
			m_MKMgr.faq();
		}
	}else if (msg.sType == _T("click")){
		if (name == _T("close")){
			SendMessage(WM_DESTROY);
		}
	}
}
LRESULT CMainFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
	m_pm.Init(m_hWnd);//主窗口类与窗口句柄关联
	CDialogBuilder builder;
#ifdef _DEBUG
	CControlUI* pRoot = builder.Create(_T("UISkin.xml"), (UINT)0, NULL, &m_pm);//加载XML并动态创建界面无素，与布局界面元素，核心函数单独分析
#else
	CControlUI* pRoot = builder.Create((STRINGorID)MAKEINTRESOURCE(IDR_SKIN_XML), _T("DATA"), NULL, &m_pm);//加载XML并动态创建界面无素，与布局界面元素，核心函数单独分析
#endif
	//注意：CDialogBuilder 并不是一个对话框类
	ASSERT(pRoot && "Failed to parse XML");
	if (NULL==pRoot)//如果找不到皮肤文件则退出
	{
		MessageBox(NULL,TEXT("Cant not find the skin!"),NULL,MB_ICONHAND);
		return 0;
	}
	m_pm.AttachDialog(pRoot);//附加控件数据到HASH表中……为pRoot作为对话框结点，为其创建控件树	
	m_pm.AddNotifier(this);//增加通知处理

	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	styleValue &= ~WS_THICKFRAME; 
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	
	if(m_pBackWnd)
	{
		m_pBackWnd->AttachWindow(m_hWnd);//必须要让背景与当前要作为背景的窗口绑定起来
	};
	return 0;
}
LRESULT CMainFrame::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch( uMsg ) {
	case WM_CREATE: 
		lRes = OnCreate(uMsg, wParam, lParam, bHandled); 
		break;
	case WM_DESTROY:
		::PostQuitMessage(0L);
		CIrregularWindow::UnInitGDIplus();
		delete m_pBackWnd;
		m_pBackWnd = NULL;
		return 0;
	default:
	bHandled = FALSE;
	}
	if( bHandled ) return lRes;
	if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);//设置程序实例
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin"));//实例句柄与渲染类关联,获得皮肤文件目录（加载皮肤文件在OnCreate之中）

	HRESULT Hr = ::CoInitialize(NULL);//初始化COM库, 为加载COM库提供支持
	if( FAILED(Hr) ) 
		return 0;

	CMainFrame* pMainFrame = new CMainFrame();//创建应用程序窗口类对象
	if( pMainFrame == NULL ) 
		return 0;
	pMainFrame->SetIcon(IDI_ICON1);
	//以背景的句柄为父窗口创建DLG，如果不这样的话，在任务栏会产生两个窗体，不信，你就把下面create的第一个参数改成NULL试试，你就懂了
	pMainFrame->Create(pMainFrame->m_pBackWnd->GetHandle(), _T("AdderCalc"), UI_WNDSTYLE_DIALOG, 0);
	//让背景图片居中
	pMainFrame->m_pBackWnd->CenterWindow();
	
	pMainFrame->ShowWindow(true);//显示窗口
	CPaintManagerUI::MessageLoop();//进入消息循环

	::CoUninitialize();//退出程序并释放COM库
	return 0;
}
