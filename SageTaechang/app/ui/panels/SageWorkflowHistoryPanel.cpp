#include "pch.h"
#include "app/ui/panels/SageWorkflowHistoryPanel.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/common/TaechangJson.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(SageWorkflowHistoryPanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL SageWorkflowHistoryPanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD | WS_CLIPCHILDREN, rectEmpty, pParent, nId);
}

int SageWorkflowHistoryPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect r(0, 0, 0, 0);
	m_wndSection.Create(TAECHANG_UI_SECTION_DETAIL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this, ID_TAECHANG_DETAIL_SECTION);
	m_wndDetail.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, r, this, ID_TAECHANG_DETAIL_EDIT);

	m_wndSection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndDetail.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	return 0;
}

void SageWorkflowHistoryPanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	if (!::IsWindow(m_wndDetail.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	if (rectClient.IsRectEmpty())
		return;

	m_wndSection.MoveWindow(0, 0, rectClient.Width(), TAECHANG_RESULT_HEADER_HEIGHT);
	m_wndDetail.MoveWindow(
		0,
		TAECHANG_RESULT_HEADER_HEIGHT,
		rectClient.Width(),
		rectClient.Height() - TAECHANG_RESULT_HEADER_HEIGHT);
}

BOOL SageWorkflowHistoryPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
	return TRUE;
}

HBRUSH SageWorkflowHistoryPanel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hBrush = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
		return hBrush;
	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
	if (nCtlColor == CTLCOLOR_STATIC) {
		pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
		return SageUiResources::GetBrush(SAGE_BG_APP);
	}
	if (nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX) {
		pDC->SetBkColor(TAECHANG_COLOR_PANEL);
		return SageUiResources::GetBrush(SAGE_BG_PANEL);
	}
	return hBrush;
}

void SageWorkflowHistoryPanel::SetSectionLabel(LPCWSTR pszLabel) {
	m_wndSection.SetWindowTextW(pszLabel);
}

void SageWorkflowHistoryPanel::AppendEntry(const CString& strInputPath, const CString& strResponseJson, BOOL bSuccess) {
	CString strLine = BuildEntryLine(strInputPath, strResponseJson, bSuccess);
	if (strLine.IsEmpty())
		return;

	if (!m_strHistory.IsEmpty())
		m_strHistory += TAECHANG_UI_HISTORY_ENTRY_BREAK;
	m_strHistory += strLine;
	m_wndDetail.SetWindowTextW(m_strHistory);
}

CString SageWorkflowHistoryPanel::BuildEntryLine(const CString& strInputPath, const CString& strResponseJson, BOOL bSuccess) const {
	CTime now = CTime::GetCurrentTime();
	CString strLine = TAECHANG_UI_HISTORY_ENTRY_PREFIX + now.Format(TAECHANG_UI_HISTORY_TIME_FORMAT) +
		TAECHANG_UI_HISTORY_ENTRY_SUFFIX + (bSuccess ? TAECHANG_UI_HISTORY_SUCCESS : TAECHANG_UI_HISTORY_FAILED);

	CString strInput = strInputPath;
	if (strInput.IsEmpty())
		strInput = TAECHANG_UI_HISTORY_EMPTY_VALUE;
	strLine += TAECHANG_UI_HISTORY_LINE_BREAK;
	strLine += TAECHANG_UI_HISTORY_FIELD_INDENT;
	strLine += TAECHANG_UI_HISTORY_INPUT_PREFIX;
	strLine += strInput;

	if (bSuccess) {
		CString strOutputPath = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_FILE_PATH);
		if (strOutputPath.IsEmpty())
			strOutputPath = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_OUTPUT_FOLDER);
		if (strOutputPath.IsEmpty())
			strOutputPath = TAECHANG_UI_HISTORY_EMPTY_VALUE;
		strLine += TAECHANG_UI_HISTORY_LINE_BREAK;
		strLine += TAECHANG_UI_HISTORY_FIELD_INDENT;
		strLine += TAECHANG_UI_HISTORY_OUTPUT_PREFIX;
		strLine += strOutputPath;
		return strLine;
	}

	CString strReason = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_MESSAGE);
	if (strReason.IsEmpty())
		strReason = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_CODE);
	if (strReason.IsEmpty())
		strReason = TAECHANG_UI_HISTORY_EMPTY_VALUE;
	strLine += TAECHANG_UI_HISTORY_LINE_BREAK;
	strLine += TAECHANG_UI_HISTORY_FIELD_INDENT;
	strLine += TAECHANG_UI_HISTORY_REASON_PREFIX;
	strLine += strReason;
	return strLine;
}
