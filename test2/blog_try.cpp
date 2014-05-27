// blog_try.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "blog_try.h"


CMainFrame::CMainFrame(void)
{
	CDuiString strResourcePath = CPaintManagerUI::GetInstancePath();
	CIrregularWindow::InitGDIplus();

	m_pBackWnd = new CIrregularWindow(strResourcePath + _T("bg.png"));

	assert(m_pBackWnd != NULL && _T("new CIrregularWindow() ʧ��!"));

	if(m_pBackWnd)
	{
		HWND hBkWnd = m_pBackWnd->GetHandle();
	}
}

CMainFrame::~CMainFrame(void)
{
}
LPCTSTR CMainFrame::GetWindowClassName() const //��д��ȡ����������Ҳ����ָ��Ҫ�½��Ĵ����������
{ 
	return _T("UIMainFrame"); 
};
UINT const CMainFrame::GetClassStyle()  //��д�����������ָ��������ʽ
{ 
	return UI_CLASSSTYLE_FRAME | CS_DBLCLKS; 
};
void CMainFrame::Notify(TNotifyUI& msg)//��������֪ͨ��Ϣ����Ӧ�û�������
{
	if (msg.sType == _T("link")){
		MessageBox(0, msg.pSender->GetName() + _T("link name"), 0, 0);
	}
}
LRESULT CMainFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
	m_pm.Init(m_hWnd);//���������봰�ھ������
	CDialogBuilder builder;

	CControlUI* pRoot = builder.Create(_T("UISkin.xml"), (UINT)0, NULL, &m_pm);//����XML����̬�����������أ��벼�ֽ���Ԫ�أ����ĺ�����������
	//ע�⣺CDialogBuilder ������һ���Ի�����
	ASSERT(pRoot && "Failed to parse XML");
	if (NULL==pRoot)//����Ҳ���Ƥ���ļ����˳�
	{
		MessageBox(NULL,TEXT("Cant not find the skin!"),NULL,MB_ICONHAND);
		return 0;
	}
	m_pm.AttachDialog(pRoot);//���ӿؼ����ݵ�HASH���С���ΪpRoot��Ϊ�Ի����㣬Ϊ�䴴���ؼ���	
	m_pm.AddNotifier(this);//����֪ͨ����

	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	styleValue &= ~WS_THICKFRAME; 
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	
	if(m_pBackWnd)
	{
		m_pBackWnd->AttachWindow(m_hWnd);//����Ҫ�ñ����뵱ǰҪ��Ϊ�����Ĵ��ڰ�����
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
	CPaintManagerUI::SetInstance(hInstance);//���ó���ʵ��
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() 
		+ _T("skin"));//ʵ���������Ⱦ�����,���Ƥ���ļ�Ŀ¼������Ƥ���ļ���OnCreate֮�У�

	HRESULT Hr = ::CoInitialize(NULL);//��ʼ��COM��, Ϊ����COM���ṩ֧��
	if( FAILED(Hr) ) 
		return 0;

	CMainFrame* pMainFrame = new CMainFrame();//����Ӧ�ó��򴰿������
	if( pMainFrame == NULL ) 
		return 0;
	//�Ա����ľ��Ϊ�����ڴ���DLG������������Ļ�����������������������壬���ţ���Ͱ�����create�ĵ�һ�������ĳ�NULL���ԣ���Ͷ���
	pMainFrame->Create(pMainFrame->m_pBackWnd->GetHandle(), _T("AdderCalc"), UI_WNDSTYLE_DIALOG, 0);
	//�ñ���ͼƬ����
	pMainFrame->m_pBackWnd->CenterWindow();

	pMainFrame->ShowWindow(true);//��ʾ����
	CPaintManagerUI::MessageLoop();//������Ϣѭ��

	::CoUninitialize();//�˳������ͷ�COM��
	return 0;
}