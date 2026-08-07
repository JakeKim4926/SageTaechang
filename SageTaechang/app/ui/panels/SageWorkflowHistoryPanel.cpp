#include "pch.h"
#include "app/ui/panels/SageWorkflowHistoryPanel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/core/workflow/SageWorkflowResultTable.h"
#include "app/common/TaechangJson.h"
#include "TaechangDefine.h"

namespace {

struct SageHistoryColumn
{
	LPCWSTR pszLabel;
	int nWidth;
	BOOL bStretch;
};

const SageHistoryColumn g_historyColumns[] = {
	{ TAECHANG_UI_HISTORY_COL_TIME,   TAECHANG_HISTORY_TIME_WIDTH,   FALSE },
	{ TAECHANG_UI_HISTORY_COL_RESULT, TAECHANG_HISTORY_RESULT_WIDTH, FALSE },
	{ TAECHANG_UI_HISTORY_COL_INPUT,  TAECHANG_HISTORY_INPUT_WIDTH,  TRUE },
	{ TAECHANG_UI_HISTORY_COL_OUTPUT, TAECHANG_HISTORY_OUTPUT_WIDTH, TRUE },
	{ TAECHANG_UI_HISTORY_COL_REASON, TAECHANG_HISTORY_REASON_WIDTH, TRUE }
};

constexpr int SAGE_HISTORY_COLUMN_COUNT = sizeof(g_historyColumns) / sizeof(g_historyColumns[0]);

constexpr int SAGE_HISTORY_COLUMN_TIME = 0;
constexpr int SAGE_HISTORY_COLUMN_RESULT = 1;
constexpr int SAGE_HISTORY_COLUMN_INPUT = 2;
constexpr int SAGE_HISTORY_COLUMN_OUTPUT = 3;
constexpr int SAGE_HISTORY_COLUMN_REASON = 4;

}

BEGIN_MESSAGE_MAP(SageWorkflowHistoryPanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL SageWorkflowHistoryPanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD | WS_CLIPCHILDREN, rectEmpty, pParent, nId);
}

int SageWorkflowHistoryPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect r(0, 0, 0, 0);
	m_wndList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
		r, this, ID_TAECHANG_DETAIL_LIST);
	m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_wndList.SetAlternateRowColor(TRUE);
	m_wndList.SetRowSeparator(TRUE);
	m_wndList.SetBadgeColumn(SAGE_HISTORY_COLUMN_RESULT);
	m_wndList.SetMutedText(TAECHANG_UI_HISTORY_NO_OUTPUT, TAECHANG_COLOR_TEXT_PLACEHOLDER);
	m_wndList.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));

	CHeaderCtrl* pHeader = m_wndList.GetHeaderCtrl();
	if (pHeader != NULL && pHeader->GetSafeHwnd() != NULL) {
		m_wndHeader.SubclassWindow(pHeader->GetSafeHwnd());
		SetWindowTheme(m_wndHeader.GetSafeHwnd(), L"", L"");
	}

	m_wndEmpty.Create(L"", WS_CHILD | SS_OWNERDRAW, r, this, ID_TAECHANG_DETAIL_EMPTY);
	m_wndEmpty.SetContent(TAECHANG_UI_HISTORY_EMPTY_TITLE, TAECHANG_UI_HISTORY_EMPTY_DESC);

	CreateColumns();
	ApplyRowStyles();
	UpdateEmptyState();
	return 0;
}

void SageWorkflowHistoryPanel::CreateColumns() {
	for (int i = 0; i < SAGE_HISTORY_COLUMN_COUNT; ++i)
		m_wndList.InsertColumn(i, g_historyColumns[i].pszLabel, LVCFMT_LEFT, g_historyColumns[i].nWidth);
}

void SageWorkflowHistoryPanel::ApplyRowStyles() {
	SageListRowStyle styleSuccess;
	styleSuccess.clrBadgeBackground = TAECHANG_COLOR_BADGE_BG_SUCCESS;
	styleSuccess.clrBadgeText = TAECHANG_COLOR_STATUS_CARD_TEXT_SUCCESS;
	m_wndList.SetRowStyle(TAECHANG_HISTORY_STATE_SUCCESS, styleSuccess);

	SageListRowStyle styleFailed;
	styleFailed.clrRowBackground = TAECHANG_COLOR_STATUS_CARD_BG_ERROR;
	styleFailed.clrBadgeBackground = TAECHANG_COLOR_STATUS_BG_ERROR;
	styleFailed.clrBadgeText = TAECHANG_COLOR_INLINE_ERROR_TEXT;
	m_wndList.SetRowStyle(TAECHANG_HISTORY_STATE_FAILED, styleFailed);
}

void SageWorkflowHistoryPanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	if (rectClient.IsRectEmpty())
		return;

	m_wndList.MoveWindow(0, 0, rectClient.Width(), rectClient.Height());
	m_wndEmpty.MoveWindow(0, 0, rectClient.Width(), rectClient.Height());
	UpdateColumnWidths();
}

void SageWorkflowHistoryPanel::UpdateColumnWidths() {
	CRect rectList;
	m_wndList.GetClientRect(&rectList);
	if (rectList.Width() <= 0)
		return;

	std::vector<SageColumnWidthSpec> arrSpecs;
	for (int i = 0; i < SAGE_HISTORY_COLUMN_COUNT; ++i)
		arrSpecs.push_back(SageColumnWidthSpec(g_historyColumns[i].nWidth, g_historyColumns[i].bStretch));

	std::vector<int> arrWidths;
	SageWorkflowResultTable::DistributeColumnWidths(arrSpecs, rectList.Width(), arrWidths);
	for (int i = 0; i < static_cast<int>(arrWidths.size()); ++i)
		m_wndList.SetColumnWidth(i, arrWidths[i]);
}

void SageWorkflowHistoryPanel::UpdateEmptyState() {
	BOOL bHasRows = m_arrRows.empty() ? FALSE : TRUE;
	m_wndList.ShowWindow(bHasRows ? SW_SHOW : SW_HIDE);
	m_wndEmpty.ShowWindow(bHasRows ? SW_HIDE : SW_SHOW);
}

SageHistoryRow SageWorkflowHistoryPanel::BuildRow(
	const CString& strInputPath, const CString& strResponseJson, BOOL bSuccess) const {
	SageHistoryRow row;
	row.bSuccess = bSuccess;
	row.strTime = CTime::GetCurrentTime().Format(TAECHANG_UI_HISTORY_TIME_FORMAT);
	row.strInputPath = strInputPath.IsEmpty() ? CString(TAECHANG_UI_AMOUNT_EMPTY_MARK) : strInputPath;

	if (bSuccess) {
		row.strOutputPath = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_FILE_PATH);
		if (row.strOutputPath.IsEmpty())
			row.strOutputPath = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_OUTPUT_FOLDER);
		if (row.strOutputPath.IsEmpty())
			row.strOutputPath = TAECHANG_UI_HISTORY_NO_OUTPUT;
		row.strReason = TAECHANG_UI_AMOUNT_EMPTY_MARK;
		return row;
	}

	row.strOutputPath = TAECHANG_UI_AMOUNT_EMPTY_MARK;
	row.strReason = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_MESSAGE);
	if (row.strReason.IsEmpty())
		row.strReason = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_CODE);
	if (row.strReason.IsEmpty())
		row.strReason = TAECHANG_UI_AMOUNT_EMPTY_MARK;
	return row;
}

void SageWorkflowHistoryPanel::InsertRow(int nItem, const SageHistoryRow& row) {
	m_wndList.InsertItem(nItem, row.strTime);
	m_wndList.SetItemText(nItem, SAGE_HISTORY_COLUMN_RESULT,
		row.bSuccess ? TAECHANG_UI_HISTORY_SUCCESS : TAECHANG_UI_HISTORY_FAILED);
	m_wndList.SetItemText(nItem, SAGE_HISTORY_COLUMN_INPUT, row.strInputPath);
	m_wndList.SetItemText(nItem, SAGE_HISTORY_COLUMN_OUTPUT, row.strOutputPath);
	m_wndList.SetItemText(nItem, SAGE_HISTORY_COLUMN_REASON, row.strReason);
	m_wndList.SetItemData(nItem,
		row.bSuccess ? TAECHANG_HISTORY_STATE_SUCCESS : TAECHANG_HISTORY_STATE_FAILED);
}

void SageWorkflowHistoryPanel::AppendEntry(
	const CString& strInputPath, const CString& strResponseJson, BOOL bSuccess) {
	SageHistoryRow row = BuildRow(strInputPath, strResponseJson, bSuccess);
	m_arrRows.insert(m_arrRows.begin(), row);
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return;

	InsertRow(0, row);
	UpdateEmptyState();
	UpdateColumnWidths();
}

BOOL SageWorkflowHistoryPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
	return TRUE;
}
