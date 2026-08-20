#include "pch.h"
#include "app/ui/panels/SageWorkflowResultPanel.h"
#include "SageDefine.h"

BEGIN_MESSAGE_MAP(SageWorkflowResultPanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_SAGE_RESULT_TABLE_CHANGED, &SageWorkflowResultPanel::OnResultTableChanged)
	ON_MESSAGE(WM_SAGE_RESULT_SELECTION_CHANGED, &SageWorkflowResultPanel::OnResultSelectionChanged)
END_MESSAGE_MAP()

BOOL SageWorkflowResultPanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD | WS_CLIPCHILDREN, rectEmpty, pParent, nId);
}

int SageWorkflowResultPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_panelResultTable.Create(this, ID_SAGE_RESULT_TABLE_PANEL);
	m_panelResultTable.SetTitle(SAGE_UI_SECTION_RESULT);
	m_panelResultTable.ShowWindow(SW_SHOW);
	return 0;
}

BOOL SageWorkflowResultPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, SAGE_COLOR_APP_BACKGROUND);
	return TRUE;
}

void SageWorkflowResultPanel::EnableFileDrop() {
	m_panelResultTable.EnableFileDrop();
}

SageResultTablePanel& SageWorkflowResultPanel::GetResultTable() {
	return m_panelResultTable;
}

int SageWorkflowResultPanel::GetBandHeight() const {
	return m_panelResultTable.GetBandHeight();
}

void SageWorkflowResultPanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	LayoutResultTable();
}

void SageWorkflowResultPanel::LayoutResultTable() {
	if (!::IsWindow(m_panelResultTable.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	if (rectClient.IsRectEmpty())
		return;
	m_panelResultTable.Layout(rectClient);
}

void SageWorkflowResultPanel::UpdateResultTableVisibility(BOOL bFilterVisible) {
	m_panelResultTable.ShowFilter(bFilterVisible);
	LayoutResultTable();
}

LRESULT SageWorkflowResultPanel::ForwardToParent(UINT nMessage, WPARAM wParam, LPARAM lParam) {
	CWnd* pParent = GetParent();
	if (pParent == NULL || !::IsWindow(pParent->GetSafeHwnd()))
		return 0;
	return pParent->SendMessage(nMessage, wParam, lParam);
}

LRESULT SageWorkflowResultPanel::OnResultTableChanged(WPARAM wParam, LPARAM lParam) {
	return ForwardToParent(WM_SAGE_RESULT_TABLE_CHANGED, wParam, lParam);
}

LRESULT SageWorkflowResultPanel::OnResultSelectionChanged(WPARAM wParam, LPARAM lParam) {
	return ForwardToParent(WM_SAGE_RESULT_SELECTION_CHANGED, wParam, lParam);
}
