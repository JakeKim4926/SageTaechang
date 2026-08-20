
#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "SageTaechang.h"
#endif

#include "app/ui/frame/SageTaechangDoc.h"
#include "app/ui/view/SageTaechangView.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/core/workflow/ISageWorkflowHandler.h"
#include "app/core/workflow/SageWorkflowRegistry.h"
#include "app/ui/dialogs/SagePasswordChangeDlg.h"
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static CString BuildDroppedPathList(HDROP hDropInfo) {
	CString strPaths;
	UINT nFileCount = DragQueryFileW(hDropInfo, 0xFFFFFFFF, NULL, 0);
	for (UINT i = 0; i < nFileCount; ++i) {
		UINT nLength = DragQueryFileW(hDropInfo, i, NULL, 0);
		if (nLength == 0)
			continue;
		CString strPath;
		LPWSTR pszPath = strPath.GetBuffer(static_cast<int>(nLength) + 1);
		DragQueryFileW(hDropInfo, i, pszPath, nLength + 1);
		strPath.ReleaseBuffer();
		if (!strPaths.IsEmpty())
			strPaths += L"\r\n";
		strPaths += strPath;
	}
	return strPaths;
}

void CSageTaechangView::EnableFileDropForWindow(CWnd& wnd) {
	HWND hWnd = wnd.GetSafeHwnd();
	if (hWnd == NULL || !::IsWindow(hWnd))
		return;

	::DragAcceptFiles(hWnd, TRUE);
	::ChangeWindowMessageFilterEx(hWnd, WM_DROPFILES, MSGFLT_ALLOW, NULL);
	::ChangeWindowMessageFilterEx(hWnd, WM_COPYDATA, MSGFLT_ALLOW, NULL);
	::ChangeWindowMessageFilterEx(hWnd, 0x0049, MSGFLT_ALLOW, NULL);
}

IMPLEMENT_DYNCREATE(CSageTaechangView, CView)

BEGIN_MESSAGE_MAP(CSageTaechangView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_SAGE_SIDEBAR_WORKFLOW, &CSageTaechangView::OnSidebarWorkflow)
	ON_MESSAGE(WM_SAGE_SIDEBAR_ACTION, &CSageTaechangView::OnSidebarAction)
	ON_MESSAGE(WM_SAGE_WORKSPACE_TAB_CHANGED, &CSageTaechangView::OnWorkspaceTabChanged)
	ON_MESSAGE(WM_SAGE_WORKSPACE_STATUS, &CSageTaechangView::OnWorkspaceStatus)
	ON_MESSAGE(WM_SAGE_WORKSPACE_STATE_CHANGED, &CSageTaechangView::OnWorkspaceStateChanged)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

CSageTaechangView::CSageTaechangView() noexcept
	: m_nCurrentWorkflow(SAGE_WORKFLOW_DELIVERY)
	{
	m_brushListHeader.CreateSolidBrush(SAGE_COLOR_LIST_HEADER);
}

CSageTaechangView::~CSageTaechangView() {}

BOOL CSageTaechangView::PreCreateWindow(CREATESTRUCT& cs) {
	cs.style |= WS_CLIPCHILDREN;
	return CView::PreCreateWindow(cs);
}

BOOL CSageTaechangView::PreTranslateMessage(MSG* pMsg) {
	if (pMsg && pMsg->message == WM_DROPFILES) {
		OnDropFiles(reinterpret_cast<HDROP>(pMsg->wParam));
		return TRUE;
	}
	return CView::PreTranslateMessage(pMsg);
}

int CSageTaechangView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CreateChildControls();
	EnableFileDropForWindow(*this);
	CFrameWnd* pFrame = GetParentFrame();
	if (pFrame != NULL)
		EnableFileDropForWindow(*pFrame);
	SetStatusText(SAGE_UI_READY);
	return 0;
}

void CSageTaechangView::CreateChildControls() {
	m_panelHeader.Create(this, ID_SAGE_HEADER_PANEL);
	m_panelHeader.ShowWindow(SW_SHOW);
	m_panelSidebar.Create(this, ID_SAGE_SIDEBAR_PANEL);
	m_panelSidebar.ShowWindow(SW_SHOW);
	m_panelWorkspace.Create(this, ID_SAGE_WORKSPACE_PANEL);
	m_panelWorkspace.EnableFileDrop();
	m_panelWorkspace.ShowWindow(SW_SHOW);

	OnWorkflowChanged();
	m_panelSidebar.BuildTree();
	m_panelHeader.UpdateAuthState();
}

void CSageTaechangView::OnSize(UINT nType, int cx, int cy) {
	CView::OnSize(nType, cx, cy);
	LayoutChildControls();
}

void CSageTaechangView::LayoutChildControls() {
	if (!::IsWindow(m_panelSidebar.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);

	m_panelSidebar.Layout(CRect(0, 0, SAGE_SIDEBAR_WIDTH, rectClient.Height()));
	m_panelHeader.Layout(CRect(
		SAGE_SIDEBAR_WIDTH + SAGE_BORDER_THICKNESS,
		0,
		rectClient.Width(),
		SAGE_HEADER_HEIGHT + SAGE_BORDER_THICKNESS));

	InvalidateContentArea();

	m_panelWorkspace.Layout(CRect(
		SAGE_SIDEBAR_WIDTH + SAGE_BORDER_THICKNESS,
		SAGE_HEADER_HEIGHT + SAGE_BORDER_THICKNESS,
		rectClient.Width(),
		rectClient.Height()));
	m_panelWorkspace.RefreshVisibility();
}

void CSageTaechangView::OnDraw(CDC* pDC) {
	CSageTaechangDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, SAGE_COLOR_APP_BACKGROUND);
	pDC->FillSolidRect(SAGE_SIDEBAR_WIDTH, 0, 1, rectClient.Height(), SAGE_COLOR_BORDER);
}


void CSageTaechangView::InvalidateContentArea() {
	CRect rectContent;
	GetClientRect(&rectContent);
	rectContent.left += SAGE_SIDEBAR_WIDTH;
	InvalidateRect(rectContent, TRUE);
}


int CSageTaechangView::GetSelectedWorkflow() const {
	return m_nCurrentWorkflow;
}

ISageWorkflowHandler* CSageTaechangView::FindCurrentHandler() const {
	return SageWorkflowRegistry::FindHandler(GetSelectedWorkflow());
}

void CSageTaechangView::OnWorkflowChanged() {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	m_panelWorkspace.SetWorkflow(m_nCurrentWorkflow, pHandler);
	m_panelWorkspace.RestoreWorkflowState(m_nCurrentWorkflow);

	m_panelHeader.SetCategory(m_panelSidebar.GetSelectedCategory());
	if (IsPriceWorkflowType(m_nCurrentWorkflow)) {
		m_panelHeader.SetTitle(
			m_nCurrentWorkflow == SAGE_WORKFLOW_PRICE_MANAGE
			? SAGE_UI_PRICE_MANAGE_NAME
			: SAGE_UI_PRICE_CALC_NAME
		);
		if (m_nCurrentWorkflow == SAGE_WORKFLOW_PRICE_MANAGE)
			m_panelWorkspace.GetPriceManagePanel().RefreshCompanyList();
		else
			m_panelWorkspace.GetPriceCalcPanel().RefreshCompanyCombo();
		LayoutChildControls();
		Invalidate(FALSE);
		SetStatusText(SAGE_UI_READY);
		return;
	}

	if (pHandler != NULL) {
		m_panelHeader.SetTitle(pHandler->GetHeaderTitle());
		m_panelWorkspace.ApplyWorkflowLabels(pHandler);
	}
	m_panelWorkspace.RebuildResultTable();
	LayoutChildControls();
	if (!m_panelWorkspace.IsRunning())
		SetStatusText(SAGE_UI_READY);
}

LRESULT CSageTaechangView::OnSidebarWorkflow(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	m_panelWorkspace.SaveWorkflowState(m_nCurrentWorkflow);
	m_nCurrentWorkflow = static_cast<int>(wParam);
	OnWorkflowChanged();
	return 0;
}

LRESULT CSageTaechangView::OnSidebarAction(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	SagePasswordChangeDlg dlg(this);
	dlg.DoModal();
	return 0;
}

LRESULT CSageTaechangView::OnWorkspaceTabChanged(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	LayoutChildControls();
	Invalidate();
	return 0;
}

LRESULT CSageTaechangView::OnWorkspaceStatus(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	LPCWSTR pszStatus = reinterpret_cast<LPCWSTR>(lParam);
	if (pszStatus != NULL)
		SetStatusText(pszStatus);
	return 0;
}

LRESULT CSageTaechangView::OnWorkspaceStateChanged(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	m_panelWorkspace.SaveWorkflowState(m_nCurrentWorkflow);
	return 0;
}

void CSageTaechangView::OnDropFiles(HDROP hDropInfo) {
	CString strPaths = BuildDroppedPathList(hDropInfo);
	DragFinish(hDropInfo);
	m_panelWorkspace.ApplyDroppedInputPaths(strPaths);
}




BOOL CSageTaechangView::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, SAGE_COLOR_APP_BACKGROUND);
	pDC->FillSolidRect(0, 0, SAGE_SIDEBAR_WIDTH, rectClient.Height(), SAGE_COLOR_SIDEBAR);
	pDC->FillSolidRect(0, SAGE_HEADER_HEIGHT, SAGE_SIDEBAR_WIDTH, SAGE_BORDER_THICKNESS, SAGE_COLOR_SIDEBAR_DIVIDER);
	pDC->FillSolidRect(SAGE_SIDEBAR_WIDTH, 0, 1, rectClient.Height(), SAGE_COLOR_BORDER);
	return TRUE;
}

HBRUSH CSageTaechangView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hBrush = CView::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
		return hBrush;
	pDC->SetTextColor(SAGE_COLOR_TEXT);
	if (nCtlColor == CTLCOLOR_STATIC) {
		pDC->SetBkColor(SAGE_COLOR_APP_BACKGROUND);
		return SageUiResources::GetBrush(SAGE_BG_APP);
	}
	if (nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX) {
		pDC->SetBkColor(SAGE_COLOR_PANEL);
		return SageUiResources::GetBrush(SAGE_BG_PANEL);
	}
	return hBrush;
}

void CSageTaechangView::SetStatusText(const CString& strStatus) {
	CFrameWnd* pFrame = GetParentFrame();
	if (pFrame != NULL)
		pFrame->SetMessageText(strStatus);
}




#ifdef _DEBUG
void CSageTaechangView::AssertValid() const {
	CView::AssertValid();
}

void CSageTaechangView::Dump(CDumpContext& dc) const {
	CView::Dump(dc);
}

CSageTaechangDoc* CSageTaechangView::GetDocument() const {
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSageTaechangDoc)));
	return (CSageTaechangDoc*)m_pDocument;
}
#endif


