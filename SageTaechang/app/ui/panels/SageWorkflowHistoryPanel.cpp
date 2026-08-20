#include "pch.h"
#include "app/ui/panels/SageWorkflowHistoryPanel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/core/workflow/SageWorkflowResultTable.h"
#include "app/common/SageJson.h"
#include "SageDefine.h"

namespace {

struct SageHistoryColumn
{
	LPCWSTR pszLabel;
	int nWidth;
	BOOL bStretch;
};

const SageHistoryColumn g_historyColumns[] = {
	{ SAGE_UI_HISTORY_COL_TIME,   SAGE_HISTORY_TIME_WIDTH,   FALSE },
	{ SAGE_UI_HISTORY_COL_RESULT, SAGE_HISTORY_RESULT_WIDTH, FALSE },
	{ SAGE_UI_HISTORY_COL_INPUT,  SAGE_HISTORY_INPUT_WIDTH,  TRUE },
	{ SAGE_UI_HISTORY_COL_OUTPUT, SAGE_HISTORY_OUTPUT_WIDTH, TRUE },
	{ SAGE_UI_HISTORY_COL_REASON, SAGE_HISTORY_REASON_WIDTH, TRUE }
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
	ON_BN_CLICKED(ID_SAGE_DETAIL_FILTER, &SageWorkflowHistoryPanel::OnFilterChanged)
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
		r, this, ID_SAGE_DETAIL_LIST);
	m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_wndList.SetAlternateRowColor(TRUE);
	m_wndList.SetRowSeparator(TRUE);
	m_wndList.SetFirstColumnAlign(SAGE_LIST_FIRST_COLUMN_CENTER);
	m_wndList.SetBadgeColumn(SAGE_HISTORY_COLUMN_RESULT);
	m_wndList.SetMutedText(SAGE_UI_HISTORY_NO_OUTPUT, SAGE_COLOR_TEXT_PLACEHOLDER);
	m_wndList.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));

	CHeaderCtrl* pHeader = m_wndList.GetHeaderCtrl();
	if (pHeader != NULL && pHeader->GetSafeHwnd() != NULL) {
		m_wndHeader.SubclassWindow(pHeader->GetSafeHwnd());
		SetWindowTheme(m_wndHeader.GetSafeHwnd(), L"", L"");
	}

	m_wndEmpty.Create(L"", WS_CHILD | SS_OWNERDRAW, r, this, ID_SAGE_DETAIL_EMPTY);
	m_wndEmpty.SetContent(SAGE_UI_HISTORY_EMPTY_TITLE, SAGE_UI_HISTORY_EMPTY_DESC);

	m_wndFilterPills.Create(L"", WS_CHILD | SS_OWNERDRAW | SS_NOTIFY, r, this, ID_SAGE_DETAIL_FILTER);
	m_wndFilterPills.SetCommand(ID_SAGE_DETAIL_FILTER);

	CreateColumns();
	ApplyRowStyles();
	UpdateFilterLabels();
	UpdateEmptyState();
	return 0;
}

void SageWorkflowHistoryPanel::UpdateFilterLabels() {
	int nSuccessCount = 0;
	for (int i = 0; i < static_cast<int>(m_arrRows.size()); ++i) {
		if (m_arrRows[i].bSuccess)
			++nSuccessCount;
	}
	int nTotalCount = static_cast<int>(m_arrRows.size());

	std::vector<CString> arrLabels;
	CString strLabel;
	strLabel.Format(SAGE_UI_HISTORY_FILTER_ALL, nTotalCount);
	arrLabels.push_back(strLabel);
	strLabel.Format(SAGE_UI_HISTORY_FILTER_SUCCESS, nSuccessCount);
	arrLabels.push_back(strLabel);
	strLabel.Format(SAGE_UI_HISTORY_FILTER_FAILED, nTotalCount - nSuccessCount);
	arrLabels.push_back(strLabel);
	m_wndFilterPills.SetLabels(arrLabels);
}

BOOL SageWorkflowHistoryPanel::IsRowVisible(const SageHistoryRow& row) const {
	int nFilter = m_wndFilterPills.GetSelectedIndex();
	if (nFilter == SAGE_HISTORY_FILTER_SUCCESS)
		return row.bSuccess;
	if (nFilter == SAGE_HISTORY_FILTER_FAILED)
		return row.bSuccess ? FALSE : TRUE;
	return TRUE;
}

void SageWorkflowHistoryPanel::RebuildVisibleRows() {
	m_wndList.DeleteAllItems();
	int nItem = 0;
	for (int i = 0; i < static_cast<int>(m_arrRows.size()); ++i) {
		if (!IsRowVisible(m_arrRows[i]))
			continue;
		InsertRow(nItem, m_arrRows[i]);
		++nItem;
	}
	UpdateEmptyState();
	LayoutChildren();
}

void SageWorkflowHistoryPanel::OnFilterChanged() {
	RebuildVisibleRows();
}

void SageWorkflowHistoryPanel::CreateColumns() {
	for (int i = 0; i < SAGE_HISTORY_COLUMN_COUNT; ++i)
		m_wndList.InsertColumn(i, g_historyColumns[i].pszLabel, LVCFMT_CENTER, g_historyColumns[i].nWidth);
}

void SageWorkflowHistoryPanel::ApplyRowStyles() {
	SageListRowStyle styleSuccess;
	styleSuccess.clrBadgeBackground = SAGE_COLOR_BADGE_BG_SUCCESS;
	styleSuccess.clrBadgeText = SAGE_COLOR_STATUS_CARD_TEXT_SUCCESS;
	m_wndList.SetRowStyle(SAGE_HISTORY_STATE_SUCCESS, styleSuccess);

	SageListRowStyle styleFailed;
	styleFailed.clrRowBackground = SAGE_COLOR_STATUS_CARD_BG_ERROR;
	styleFailed.clrBadgeBackground = SAGE_COLOR_STATUS_BG_ERROR;
	styleFailed.clrBadgeText = SAGE_COLOR_INLINE_ERROR_TEXT;
	m_wndList.SetRowStyle(SAGE_HISTORY_STATE_FAILED, styleFailed);
}

void SageWorkflowHistoryPanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	LayoutChildren();
}

void SageWorkflowHistoryPanel::LayoutChildren() {
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	if (rectClient.IsRectEmpty())
		return;

	int nTableTop = m_arrRows.empty()
		? 0
		: SAGE_PILL_HEIGHT + SAGE_CARD_ROW_GAP;
	m_wndFilterPills.MoveWindow(0, 0, rectClient.Width(), SAGE_PILL_HEIGHT);
	m_wndList.MoveWindow(0, nTableTop, rectClient.Width(), rectClient.Height() - nTableTop);
	m_wndEmpty.MoveWindow(0, nTableTop, rectClient.Width(), rectClient.Height() - nTableTop);
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
	BOOL bHasHistory = m_arrRows.empty() ? FALSE : TRUE;
	BOOL bHasVisibleRows = (m_wndList.GetItemCount() > 0) ? TRUE : FALSE;

	m_wndFilterPills.ShowWindow(bHasHistory ? SW_SHOW : SW_HIDE);
	m_wndList.ShowWindow(bHasVisibleRows ? SW_SHOW : SW_HIDE);
	m_wndEmpty.ShowWindow(bHasVisibleRows ? SW_HIDE : SW_SHOW);
	if (bHasVisibleRows)
		return;

	m_wndEmpty.SetContent(
		bHasHistory ? SAGE_UI_HISTORY_FILTER_EMPTY_TITLE : SAGE_UI_HISTORY_EMPTY_TITLE,
		bHasHistory ? SAGE_UI_HISTORY_FILTER_EMPTY_DESC : SAGE_UI_HISTORY_EMPTY_DESC);
}

SageHistoryRow SageWorkflowHistoryPanel::BuildRow(
	const CString& strInputPath, const CString& strResponseJson, BOOL bSuccess) const {
	SageHistoryRow row;
	row.bSuccess = bSuccess;
	row.strTime = CTime::GetCurrentTime().Format(SAGE_UI_HISTORY_TIME_FORMAT);
	row.strInputPath = strInputPath.IsEmpty() ? CString(SAGE_UI_AMOUNT_EMPTY_MARK) : strInputPath;

	if (bSuccess) {
		row.strOutputPath = JsonExtractString(strResponseJson, SAGE_JSON_KEY_FILE_PATH);
		if (row.strOutputPath.IsEmpty())
			row.strOutputPath = JsonExtractString(strResponseJson, SAGE_JSON_KEY_OUTPUT_FOLDER);
		if (row.strOutputPath.IsEmpty())
			row.strOutputPath = SAGE_UI_HISTORY_NO_OUTPUT;
		row.strReason = SAGE_UI_AMOUNT_EMPTY_MARK;
		return row;
	}

	row.strOutputPath = SAGE_UI_AMOUNT_EMPTY_MARK;
	row.strReason = JsonExtractString(strResponseJson, SAGE_JSON_KEY_MESSAGE);
	if (row.strReason.IsEmpty())
		row.strReason = JsonExtractString(strResponseJson, SAGE_JSON_KEY_CODE);
	if (row.strReason.IsEmpty())
		row.strReason = SAGE_UI_AMOUNT_EMPTY_MARK;
	return row;
}

void SageWorkflowHistoryPanel::InsertRow(int nItem, const SageHistoryRow& row) {
	m_wndList.InsertItem(nItem, row.strTime);
	m_wndList.SetItemText(nItem, SAGE_HISTORY_COLUMN_RESULT,
		row.bSuccess ? SAGE_UI_HISTORY_SUCCESS : SAGE_UI_HISTORY_FAILED);
	m_wndList.SetItemText(nItem, SAGE_HISTORY_COLUMN_INPUT, row.strInputPath);
	m_wndList.SetItemText(nItem, SAGE_HISTORY_COLUMN_OUTPUT, row.strOutputPath);
	m_wndList.SetItemText(nItem, SAGE_HISTORY_COLUMN_REASON, row.strReason);
	m_wndList.SetItemData(nItem,
		row.bSuccess ? SAGE_HISTORY_STATE_SUCCESS : SAGE_HISTORY_STATE_FAILED);
}

SageHistoryRow SageWorkflowHistoryPanel::BuildFileRow(
	const CString& strInputPath, const CString& strFileJson, BOOL bRunSuccess) const {
	SageHistoryRow row;
	row.strTime = CTime::GetCurrentTime().Format(SAGE_UI_HISTORY_TIME_FORMAT);
	row.strInputPath = strInputPath.IsEmpty() ? CString(SAGE_UI_AMOUNT_EMPTY_MARK) : strInputPath;

	CString strStatus = JsonExtractString(strFileJson, SAGE_JSON_KEY_STATUS);
	row.bSuccess = strStatus.IsEmpty()
		? bRunSuccess
		: (strStatus.CompareNoCase(SAGE_JSON_VALUE_SUCCESS) == 0 ? TRUE : FALSE);

	if (row.bSuccess) {
		row.strOutputPath = JsonExtractString(strFileJson, SAGE_JSON_KEY_FILE_PATH);
		if (row.strOutputPath.IsEmpty())
			row.strOutputPath = SAGE_UI_HISTORY_NO_OUTPUT;
		row.strReason = SAGE_UI_AMOUNT_EMPTY_MARK;
		return row;
	}

	row.strOutputPath = SAGE_UI_AMOUNT_EMPTY_MARK;
	row.strReason = JsonExtractString(strFileJson, SAGE_JSON_KEY_MESSAGE);
	if (row.strReason.IsEmpty())
		row.strReason = SAGE_UI_AMOUNT_EMPTY_MARK;
	return row;
}

void SageWorkflowHistoryPanel::AppendEntry(
	const CString& strInputPath, const CString& strResponseJson, BOOL bSuccess) {
	std::vector<CString> arrFiles;
	SplitJsonObjectArray(JsonExtractArray(strResponseJson, SAGE_JSON_KEY_FILES), arrFiles);

	std::vector<SageHistoryRow> arrNewRows;
	if (arrFiles.empty()) {
		arrNewRows.push_back(BuildRow(strInputPath, strResponseJson, bSuccess));
	}
	else {
		for (size_t nFile = 0; nFile < arrFiles.size(); ++nFile)
			arrNewRows.push_back(BuildFileRow(strInputPath, arrFiles[nFile], bSuccess));
	}

	m_arrRows.insert(m_arrRows.begin(), arrNewRows.begin(), arrNewRows.end());
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return;

	UpdateFilterLabels();
	RebuildVisibleRows();
}

BOOL SageWorkflowHistoryPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, SAGE_COLOR_APP_BACKGROUND);
	return TRUE;
}
