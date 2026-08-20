#include "pch.h"
#include "app/ui/panels/SageResultTablePanel.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "SageDefine.h"
#include <uxtheme.h>

BEGIN_MESSAGE_MAP(SageResultTablePanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(ID_SAGE_RESULT_SEARCH_BTN, &SageResultTablePanel::OnSearch)
	ON_BN_CLICKED(ID_SAGE_RESULT_RESET_BTN, &SageResultTablePanel::OnFilterReset)
	ON_CBN_SELCHANGE(ID_SAGE_RESULT_FILTER_CRITERIA, &SageResultTablePanel::OnCriteriaChanged)
	ON_BN_CLICKED(ID_SAGE_SELECT_ALL, &SageResultTablePanel::OnSelectAll)
	ON_BN_CLICKED(ID_SAGE_SELECTION_CLEAR, &SageResultTablePanel::OnSelectionClear)
	ON_BN_CLICKED(ID_SAGE_ESTIMATE_ONE_PAGE, &SageResultTablePanel::OnOnePageOption)
	ON_NOTIFY(LVN_ITEMCHANGED, ID_SAGE_RESULT_LIST, &SageResultTablePanel::OnListItemChanged)
	ON_NOTIFY(LVN_GETDISPINFO, ID_SAGE_RESULT_LIST, &SageResultTablePanel::OnListGetDispInfo)
END_MESSAGE_MAP()

SageResultTablePanel::SageResultTablePanel()
	: m_nCriteria(SAGE_FILTER_CRITERIA_NONE)
	, m_bTitleVisible(FALSE)
	, m_bSelectAllVisible(FALSE)
	, m_bOnePageVisible(FALSE)
	, m_bFilterVisible(FALSE)
	, m_bUpdatingChecks(FALSE)
	, m_bBatchUpdate(FALSE)
	, m_nSelectedRowCount(0) {
}

static void AcceptDroppedFiles(CWnd& wnd) {
	HWND hWnd = wnd.GetSafeHwnd();
	if (hWnd == NULL || !::IsWindow(hWnd))
		return;

	::DragAcceptFiles(hWnd, TRUE);
	::ChangeWindowMessageFilterEx(hWnd, WM_DROPFILES, MSGFLT_ALLOW, NULL);
	::ChangeWindowMessageFilterEx(hWnd, WM_COPYDATA, MSGFLT_ALLOW, NULL);
	::ChangeWindowMessageFilterEx(hWnd, WM_SAGE_COPYGLOBALDATA, MSGFLT_ALLOW, NULL);
}

static SageTotalBarCellStyle ToTotalBarCellStyle(SageResultTotalRole nRole) {
	switch (nRole) {
		case SAGE_RESULT_TOTAL_COUNT:            return SAGE_TOTAL_BAR_COUNT;
		case SAGE_RESULT_TOTAL_AMOUNT:           return SAGE_TOTAL_BAR_AMOUNT;
		case SAGE_RESULT_TOTAL_AMOUNT_HIGHLIGHT: return SAGE_TOTAL_BAR_AMOUNT_HIGHLIGHT;
		default:                                 return SAGE_TOTAL_BAR_LABEL;
	}
}

BOOL SageResultTablePanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD | WS_CLIPCHILDREN, rectEmpty, pParent, nId);
}

BOOL SageResultTablePanel::PreTranslateMessage(MSG* pMsg) {
	if (pMsg != NULL && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
		m_wndSearch.IsEditMessage(pMsg)) {
		OnSearch();
		return TRUE;
	}
	return CWnd::PreTranslateMessage(pMsg);
}

void SageResultTablePanel::EnableFileDrop() {
	AcceptDroppedFiles(*this);
	AcceptDroppedFiles(m_wndTitle);
	AcceptDroppedFiles(m_wndList);
}

int SageResultTablePanel::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CreateControls();
	ApplyControlFonts();
	return 0;
}

void SageResultTablePanel::CreateControls() {
	CRect r(0, 0, 0, 0);
	m_wndTitle.Create(SAGE_UI_SECTION_RESULT, WS_CHILD | SS_OWNERDRAW, r, this, ID_SAGE_RESULT_SECTION);
	m_wndSelectionBar.Create(L"", WS_CHILD | SS_OWNERDRAW, r, this, ID_SAGE_RESULT_SELECTION_BAR);
	m_wndSelectionBar.SetCommands(ID_SAGE_SELECT_ALL, ID_SAGE_SELECTION_CLEAR);
	m_wndOnePage.Create(SAGE_UI_ESTIMATE_ONE_PAGE_CHECK, WS_CHILD | BS_OWNERDRAW, r, this, ID_SAGE_ESTIMATE_ONE_PAGE);
	m_wndOnePage.SetHint(SAGE_UI_ESTIMATE_ONE_PAGE_HINT);

	m_wndSearch.CreateBox(this, ID_SAGE_RESULT_FILTER_BOX, ID_SAGE_RESULT_FILTER_EDIT);
	m_wndSearch.CreateCriteriaCell(ID_SAGE_RESULT_FILTER_CRITERIA, SAGE_RESULT_CRITERIA_DROP_ROWS);
	m_wndSearch.SetCommand(ID_SAGE_RESULT_SEARCH_BTN);
	m_wndSearch.SetMaxLength(SAGE_RESULT_FILTER_MAX_LENGTH);
	m_wndSearch.SetPlaceholder(SAGE_UI_RESULT_FILTER_PLACEHOLDER);
	m_wndResetBtn.Create(SAGE_UI_RESULT_RESET_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_SAGE_RESULT_RESET_BTN);
	m_wndResetBtn.SetVariant(SAGE_BUTTON_GHOST);
	m_wndResetBtn.SetIcon(SAGE_BUTTON_ICON_RESET);
	m_wndResetBtn.SetSurfaceColor(SAGE_COLOR_APP_BACKGROUND);

	m_wndSummaryBar.Create(L"", WS_CHILD | SS_OWNERDRAW, r, this, ID_SAGE_RESULT_SUMMARY_BAR);
	m_wndTotalBar.Create(L"", WS_CHILD | SS_OWNERDRAW, r, this, ID_SAGE_RESULT_TOTAL_BAR);

	m_wndList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL, r, this, ID_SAGE_RESULT_LIST);
	m_wndList.SetAlternateRowColor(TRUE);
	m_wndList.SetFirstColumnAlign(SAGE_LIST_FIRST_COLUMN_CENTER);
	CHeaderCtrl* pHeader = m_wndList.GetHeaderCtrl();
	if (pHeader != NULL && pHeader->GetSafeHwnd() != NULL) {
		m_wndHeader.SubclassWindow(pHeader->GetSafeHwnd());
		SetWindowTheme(m_wndHeader.GetSafeHwnd(), L"", L"");
	}
}

void SageResultTablePanel::ApplyControlFonts() {
	m_wndTitle.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndOnePage.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndResetBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndList.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
	if (::IsWindow(m_wndHeader.GetSafeHwnd()))
		m_wndHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
}

int SageResultTablePanel::GetBandHeight() const {
	return SAGE_RESULT_FILTER_TOP_LIFT + SAGE_RESULT_FILTER_BOX_PAD;
}

int SageResultTablePanel::GetFilterTotalWidth() const {
	return SAGE_SEARCH_CRITERIA_CELL_WIDTH + SAGE_RESULT_FILTER_WIDTH
		+ SAGE_SEARCH_ICON_CELL_WIDTH + SAGE_ACTION_GAP + SAGE_RESULT_RESET_WIDTH;
}

int SageResultTablePanel::GetBandRight() const {
	CRect rectClient;
	GetClientRect(&rectClient);
	int nWidth = rectClient.Width();
	if (!m_bFilterVisible)
		return nWidth;
	int nBandRight = nWidth - GetFilterTotalWidth() - SAGE_ROW_GAP;
	return (nBandRight < 0) ? 0 : nBandRight;
}

void SageResultTablePanel::LayoutSelectionRow() {
	if (!m_bSelectAllVisible || !::IsWindow(m_wndSelectionBar.GetSafeHwnd()))
		return;

	int nRowTop = GetBandHeight() - SAGE_BUTTON_VERT_ADJUST;
	int nBandRight = GetBandRight();
	int nBarWidth = m_wndSelectionBar.GetContentWidth();
	if (nBarWidth > nBandRight)
		nBarWidth = nBandRight;
	m_wndSelectionBar.MoveWindow(0, nRowTop, nBarWidth, SAGE_BUTTON_HEIGHT);
	if (!m_bOnePageVisible)
		return;

	int nOnePageLeft = nBarWidth + SAGE_SELECTION_BAR_GAP;
	m_wndOnePage.MoveWindow(nOnePageLeft, nRowTop, m_wndOnePage.GetContentWidth(), SAGE_BUTTON_HEIGHT);
}

void SageResultTablePanel::SetTitle(LPCWSTR pszTitle) {
	m_bTitleVisible = (pszTitle != NULL) ? TRUE : FALSE;
	if (m_bTitleVisible)
		m_wndTitle.SetWindowTextW(pszTitle);
	m_wndTitle.ShowWindow(m_bTitleVisible ? SW_SHOW : SW_HIDE);
}

void SageResultTablePanel::ShowSelectAll(BOOL bShow) {
	m_bSelectAllVisible = bShow;
	m_wndSelectionBar.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	if (bShow)
		SyncSelectionBar();
}

void SageResultTablePanel::ShowOnePageOption(BOOL bShow) {
	m_bOnePageVisible = bShow;
	m_wndOnePage.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
}

void SageResultTablePanel::ShowFilter(BOOL bShow) {
	m_bFilterVisible = bShow;
	int nCmd = bShow ? SW_SHOW : SW_HIDE;
	m_wndSearch.ShowWindow(nCmd);
	m_wndResetBtn.ShowWindow(nCmd);
	if (bShow)
		PopulateCriteria();
}

void SageResultTablePanel::EnableSelectionControls(BOOL bEnable) {
	m_wndSelectionBar.EnableControls(bEnable);
	m_wndOnePage.EnableWindow(bEnable);
}

BOOL SageResultTablePanel::IsOnePageChecked() const {
	return m_wndOnePage.IsChecked();
}

void SageResultTablePanel::SetOnePageChecked(BOOL bChecked) {
	m_wndOnePage.SetChecked(bChecked);
}

void SageResultTablePanel::LayoutBandRow() {
	int nBandTop = GetBandHeight();
	int nBandRight = GetBandRight();

	if (m_bSelectAllVisible) {
		m_wndTitle.MoveWindow(0, 0, 0, 0);
		m_wndSummaryBar.MoveWindow(0, 0, 0, 0);
		LayoutSelectionRow();
		return;
	}
	if (m_wndSummaryBar.HasItems()) {
		m_wndTitle.MoveWindow(0, 0, 0, 0);
		m_wndSummaryBar.MoveWindow(
			0,
			nBandTop - SAGE_BUTTON_VERT_ADJUST,
			nBandRight,
			SAGE_SUMMARY_BAR_HEIGHT);
		return;
	}
	m_wndSummaryBar.MoveWindow(0, 0, 0, 0);
	m_wndTitle.MoveWindow(0, nBandTop, nBandRight, SAGE_RESULT_HEADER_HEIGHT);
}

void SageResultTablePanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);

	int nBandTop = SAGE_RESULT_FILTER_TOP_LIFT + SAGE_RESULT_FILTER_BOX_PAD;
	int nWidth = rectPanel.Width();
	int nFilterLeft = nWidth - GetFilterTotalWidth();

	LayoutBandRow();

	if (m_bFilterVisible) {
		int nFilterTop = nBandTop - SAGE_BUTTON_VERT_ADJUST;
		int nBoxWidth = SAGE_SEARCH_CRITERIA_CELL_WIDTH + SAGE_RESULT_FILTER_WIDTH
			+ SAGE_SEARCH_ICON_CELL_WIDTH;
		m_wndResetBtn.MoveWindow(nFilterLeft, nFilterTop, SAGE_RESULT_RESET_WIDTH, SAGE_BUTTON_HEIGHT);
		int nBoxLeft = nFilterLeft + SAGE_RESULT_RESET_WIDTH + SAGE_ACTION_GAP;
		m_wndSearch.MoveWindow(nBoxLeft, nFilterTop, nBoxWidth, SAGE_EDIT_HEIGHT);
	}

	LayoutTableArea();
}

void SageResultTablePanel::LayoutTableArea() {
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	int nWidth = rectClient.Width();
	int nHeight = rectClient.Height();
	if (nWidth <= 0)
		return;

	int nListTop = GetBandHeight() + SAGE_RESULT_HEADER_HEIGHT;
	int nBandBottom = 0;
	if (m_bSelectAllVisible)
		nBandBottom = GetBandHeight() - SAGE_BUTTON_VERT_ADJUST + SAGE_BUTTON_HEIGHT;
	else if (m_wndSummaryBar.HasItems())
		nBandBottom = GetBandHeight() - SAGE_BUTTON_VERT_ADJUST + SAGE_SUMMARY_BAR_HEIGHT;
	if (nListTop < nBandBottom + SAGE_ROW_GAP)
		nListTop = nBandBottom + SAGE_ROW_GAP;

	int nTotalBarHeight = m_arrTotalCells.empty() ? 0 : SAGE_TOTAL_BAR_HEIGHT;
	int nListHeight = nHeight - nListTop - nTotalBarHeight;
	if (nListHeight < SAGE_RESULT_MIN_HEIGHT)
		nListHeight = SAGE_RESULT_MIN_HEIGHT;
	m_wndList.MoveWindow(0, nListTop, nWidth, nListHeight);
	if (nTotalBarHeight > 0)
		m_wndTotalBar.MoveWindow(0, nListTop + nListHeight, nWidth, nTotalBarHeight);
	UpdateColumnWidths();
}

BOOL SageResultTablePanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, SAGE_COLOR_APP_BACKGROUND);
	return TRUE;
}


HBRUSH SageResultTablePanel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hBrush = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
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

void SageResultTablePanel::SetColumns(const std::vector<SageWorkflowColumn>& arrColumns, const SageWorkflowResultStyle& style) {
	m_arrColumns = arrColumns;
	m_style = style;
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return;

	m_wndList.SetExtendedStyle(m_wndList.GetExtendedStyle()
		| LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_wndList.SetCheckboxes(style.bCheckbox);
	m_wndList.SetRowSeparator(style.bGridLines);
	m_wndList.SetHighlightColumns(style.nHighlightStart, style.nHighlightCount);

	m_wndList.DeleteAllItems();
	CHeaderCtrl* pHeader = m_wndList.GetHeaderCtrl();
	int nOldColumnCount = (pHeader != NULL) ? pHeader->GetItemCount() : 0;
	for (int i = nOldColumnCount - 1; i >= 0; --i)
		m_wndList.DeleteColumn(i);

	for (int i = 0; i < static_cast<int>(m_arrColumns.size()); ++i) {
		const SageWorkflowColumn& column = m_arrColumns[i];
		int nFormat = LVCFMT_LEFT;
		if (column.nAlign == SAGE_COLUMN_ALIGN_RIGHT)
			nFormat = LVCFMT_RIGHT;
		else if (column.nAlign == SAGE_COLUMN_ALIGN_CENTER)
			nFormat = LVCFMT_CENTER;
		m_wndList.InsertColumn(i, column.pszLabel, nFormat, column.nWidth);
	}
	UpdateColumnWidths();
}

void SageResultTablePanel::UpdateColumnWidths() {
	if (!::IsWindow(m_wndList.GetSafeHwnd()) || m_arrColumns.empty())
		return;

	CRect rectList;
	m_wndList.GetClientRect(&rectList);
	int nWidth = rectList.Width();
	if (nWidth <= 0)
		return;

	std::vector<SageColumnWidthSpec> arrSpecs;
	for (int i = 0; i < static_cast<int>(m_arrColumns.size()); ++i)
		arrSpecs.push_back(SageColumnWidthSpec(m_arrColumns[i].nWidth, m_arrColumns[i].bStretch));

	std::vector<int> arrWidths;
	SageWorkflowResultTable::DistributeColumnWidths(arrSpecs, nWidth, arrWidths);
	for (int i = 0; i < static_cast<int>(arrWidths.size()); ++i)
		m_wndList.SetColumnWidth(i, arrWidths[i]);
	UpdateTotalBarCells();
}

int SageResultTablePanel::GetColumnLeft(int nColumn) const {
	int nLeft = 0;
	for (int i = 0; i < nColumn; ++i)
		nLeft += m_wndList.GetColumnWidth(i);
	return nLeft;
}

void SageResultTablePanel::UpdateTotalBarCells() {
	if (!::IsWindow(m_wndTotalBar.GetSafeHwnd()))
		return;

	CRect rectListClient;
	m_wndList.GetClientRect(&rectListClient);
	m_wndList.ClientToScreen(&rectListClient);
	m_wndTotalBar.ScreenToClient(&rectListClient);

	std::vector<SageTableTotalBarCell> arrBarCells;
	for (int i = 0; i < static_cast<int>(m_arrTotalCells.size()); ++i) {
		const SageResultTotalCell& cell = m_arrTotalCells[i];
		if (cell.nColumn < 0 || cell.nColumn >= static_cast<int>(m_arrColumns.size()))
			continue;

		SageTableTotalBarCell barCell;
		barCell.strText = cell.strText;
		barCell.nLeft = rectListClient.left + GetColumnLeft(cell.nColumn);
		barCell.nWidth = m_wndList.GetColumnWidth(cell.nColumn);
		barCell.nAlign = m_arrColumns[cell.nColumn].nAlign;
		barCell.nStyle = ToTotalBarCellStyle(cell.nRole);
		arrBarCells.push_back(barCell);
	}
	m_wndTotalBar.SetCells(arrBarCells);
}

void SageResultTablePanel::SetFilterCriteria(const std::vector<SageWorkflowFilterCriteria>& arrCriteria) {
	m_arrCriteria = arrCriteria;
	if (m_bFilterVisible)
		PopulateCriteria();
}

int SageResultTablePanel::GetDefaultCriteria() const {
	if (m_arrCriteria.empty())
		return SAGE_FILTER_CRITERIA_NONE;
	return m_arrCriteria[0].nCriteria;
}

int SageResultTablePanel::GetEffectiveCriteria() const {
	for (int i = 0; i < static_cast<int>(m_arrCriteria.size()); ++i) {
		if (m_arrCriteria[i].nCriteria == m_nCriteria)
			return m_nCriteria;
	}
	return GetDefaultCriteria();
}

void SageResultTablePanel::PopulateCriteria() {
	int nEffective = GetEffectiveCriteria();
	m_wndSearch.ClearCriteriaItems();
	for (int i = 0; i < static_cast<int>(m_arrCriteria.size()); ++i)
		m_wndSearch.AddCriteriaItem(m_arrCriteria[i].pszLabel, m_arrCriteria[i].nCriteria);

	int nIndex = m_wndSearch.FindCriteriaIndex(nEffective);
	m_wndSearch.SetCriteriaIndex(nIndex == CB_ERR ? 0 : nIndex);
}

void SageResultTablePanel::SetRows(const std::vector<SageResultRow>& arrRows) {
	m_arrRows = arrRows;
	RefreshRows();
}

void SageResultTablePanel::ClearRows() {
	m_arrRows.clear();
	m_arrVisibleRows.clear();
	if (::IsWindow(m_wndList.GetSafeHwnd()))
		m_wndList.DeleteAllItems();
}

const std::vector<SageResultRow>& SageResultTablePanel::GetVisibleRows() const {
	return m_arrVisibleRows;
}

void SageResultTablePanel::SetSummaryItems(const std::vector<SageResultSummaryItem>& arrItems) {
	std::vector<SageSummaryBarItem> arrBarItems;
	for (int i = 0; i < static_cast<int>(arrItems.size()); ++i) {
		SageSummaryBarItem barItem;
		barItem.strLabel = arrItems[i].strLabel;
		barItem.strValue = arrItems[i].strValue;
		barItem.strUnit = arrItems[i].strUnit;
		barItem.bHighlight = arrItems[i].bHighlight;
		if (arrItems[i].bBadge) {
			barItem.badge.clrBackground = SAGE_COLOR_INLINE_WARN_BG;
			barItem.badge.clrBorder = SAGE_COLOR_INLINE_WARN_BORDER;
			barItem.badge.clrText = SAGE_COLOR_WARNING;
		}
		arrBarItems.push_back(barItem);
	}

	m_wndSummaryBar.SetItems(arrBarItems);
	m_wndSummaryBar.ShowWindow(arrBarItems.empty() ? SW_HIDE : SW_SHOW);
	LayoutBandRow();
	LayoutTableArea();
}

void SageResultTablePanel::ClearSummary() {
	m_wndSummaryBar.SetItems(std::vector<SageSummaryBarItem>());
	m_wndSummaryBar.ShowWindow(SW_HIDE);
	LayoutBandRow();
	LayoutTableArea();
}

void SageResultTablePanel::SetTotalCells(const std::vector<SageResultTotalCell>& arrCells) {
	m_arrTotalCells = arrCells;
	m_wndTotalBar.ShowWindow(m_arrTotalCells.empty() ? SW_HIDE : SW_SHOW);
	LayoutTableArea();
}

void SageResultTablePanel::ClearTotals() {
	m_arrTotalCells.clear();
	m_wndTotalBar.ShowWindow(SW_HIDE);
	LayoutTableArea();
}

void SageResultTablePanel::RefreshRows() {
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return;

	if (m_bFilterVisible)
		PopulateCriteria();

	CString strKeywordLower = m_strKeyword;
	strKeywordLower.Trim();
	strKeywordLower.MakeLower();

	SageResultField nFilterField = SAGE_RESULT_FIELD_COMPANY_NAME;
	int nEffective = GetEffectiveCriteria();
	for (int i = 0; i < static_cast<int>(m_arrCriteria.size()); ++i) {
		if (m_arrCriteria[i].nCriteria == nEffective) {
			nFilterField = m_arrCriteria[i].nField;
			break;
		}
	}

	m_bUpdatingChecks = TRUE;
	if (!m_bBatchUpdate)
		m_wndList.SetRedraw(FALSE);
	m_wndList.DeleteAllItems();
	m_arrVisibleRows.clear();
	for (int i = 0; i < static_cast<int>(m_arrRows.size()); ++i) {
		if (!strKeywordLower.IsEmpty()) {
			CString strTargetLower = SageWorkflowResultTable::GetRowText(m_arrRows[i], nFilterField);
			strTargetLower.MakeLower();
			if (strTargetLower.Find(strKeywordLower) < 0)
				continue;
		}

		if (m_arrColumns.empty())
			continue;
		int nIndex = static_cast<int>(m_arrVisibleRows.size());
		m_arrVisibleRows.push_back(m_arrRows[i]);
		m_wndList.InsertItem(LVIF_TEXT | LVIF_PARAM, nIndex, LPSTR_TEXTCALLBACK, 0, 0, 0,
			static_cast<LPARAM>(m_arrRows[i].m_nSourceRowIndex));
	}
	if (!m_bBatchUpdate) {
		m_wndList.SetRedraw(TRUE);
		m_wndList.Invalidate();
	}
	m_bUpdatingChecks = FALSE;
	SyncSelectionBar();
}

void SageResultTablePanel::BeginBatchUpdate() {
	if (m_bBatchUpdate || !::IsWindow(m_wndList.GetSafeHwnd()))
		return;
	m_bBatchUpdate = TRUE;
	m_wndList.SetRedraw(FALSE);
}

void SageResultTablePanel::EndBatchUpdate() {
	if (!m_bBatchUpdate)
		return;
	m_bBatchUpdate = FALSE;
	m_wndList.SetRedraw(TRUE);
	m_wndList.Invalidate();
}

int SageResultTablePanel::GetRowCount() const {
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return 0;
	return m_wndList.GetItemCount();
}

int SageResultTablePanel::GetCheckedRowCount() const {
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return 0;
	int nCheckedCount = 0;
	int nCount = m_wndList.GetItemCount();
	for (int i = 0; i < nCount; ++i) {
		if (m_wndList.GetCheck(i))
			++nCheckedCount;
	}
	return nCheckedCount;
}

BOOL SageResultTablePanel::IsRowChecked(int nRow) const {
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return FALSE;
	return m_wndList.GetCheck(nRow);
}

void SageResultTablePanel::SetRowChecked(int nRow, BOOL bChecked) {
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return;
	m_wndList.SetCheck(nRow, bChecked);
}

DWORD_PTR SageResultTablePanel::GetRowData(int nRow) const {
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return 0;
	return m_wndList.GetItemData(nRow);
}

CString SageResultTablePanel::GetCheckedRowNums() const {
	CString strCheckedRowNums;
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return strCheckedRowNums;

	int nListCount = m_wndList.GetItemCount();
	for (int i = 0; i < nListCount; ++i) {
		if (!m_wndList.GetCheck(i))
			continue;
		DWORD_PTR nSourceRowIndex = m_wndList.GetItemData(i);
		if (nSourceRowIndex == 0)
			continue;
		CString strNum;
		strNum.Format(SAGE_UI_ROW_NUM_FORMAT, static_cast<unsigned long>(nSourceRowIndex));
		if (!strCheckedRowNums.IsEmpty())
			strCheckedRowNums += SAGE_UI_ROW_NUM_SEPARATOR;
		strCheckedRowNums += strNum;
	}
	return strCheckedRowNums;
}

void SageResultTablePanel::RestoreCheckedRowNums(const CString& strCheckedRowNums) {
	if (strCheckedRowNums.IsEmpty() || !::IsWindow(m_wndList.GetSafeHwnd()))
		return;

	m_bUpdatingChecks = TRUE;
	int nListCount = m_wndList.GetItemCount();
	for (int i = 0; i < nListCount; ++i) {
		DWORD_PTR nSourceRowIndex = m_wndList.GetItemData(i);
		if (nSourceRowIndex == 0)
			continue;
		CString strCurrentNum;
		strCurrentNum.Format(SAGE_UI_ROW_NUM_FORMAT, static_cast<unsigned long>(nSourceRowIndex));
		CString strRemaining = strCheckedRowNums;
		int nTokenIndex = 0;
		CString strToken = strRemaining.Tokenize(SAGE_UI_ROW_NUM_SEPARATOR, nTokenIndex);
		while (!strToken.IsEmpty()) {
			strToken.Trim();
			if (strToken == strCurrentNum) {
				m_wndList.SetCheck(i, TRUE);
				break;
			}
			strToken = strRemaining.Tokenize(SAGE_UI_ROW_NUM_SEPARATOR, nTokenIndex);
		}
	}
	m_bUpdatingChecks = FALSE;
	NotifySelectionChanged();
}

CString SageResultTablePanel::GetFilterKeyword() const {
	return m_strKeyword;
}

int SageResultTablePanel::GetFilterCriteria() const {
	return m_nCriteria;
}

void SageResultTablePanel::RestoreFilter(const CString& strKeyword, int nCriteria) {
	m_strKeyword = strKeyword;
	m_nCriteria = nCriteria;
	m_wndSearch.SetKeyword(m_strKeyword);
}

void SageResultTablePanel::NotifyStateChanged() {
	CWnd* pParent = GetParent();
	if (pParent == NULL || !::IsWindow(pParent->GetSafeHwnd()))
		return;
	pParent->SendMessage(WM_SAGE_RESULT_TABLE_CHANGED, 0, 0);
}

void SageResultTablePanel::SyncSelectionBar() {
	if (!::IsWindow(m_wndSelectionBar.GetSafeHwnd()) || !::IsWindow(m_wndList.GetSafeHwnd()))
		return;
	int nTotalCount = m_wndList.GetItemCount();
	int nSelectedCount = GetCheckedRowCount();
	m_nSelectedRowCount = nSelectedCount;
	m_wndSelectionBar.SetCounts(nTotalCount, nSelectedCount);
	m_wndSelectionBar.SetAllChecked((nTotalCount > 0 && nSelectedCount == nTotalCount) ? TRUE : FALSE);
	LayoutSelectionRow();
}

void SageResultTablePanel::NotifySelectionChanged() {
	SyncSelectionBar();
	CWnd* pParent = GetParent();
	if (pParent == NULL || !::IsWindow(pParent->GetSafeHwnd()))
		return;
	pParent->SendMessage(WM_SAGE_RESULT_SELECTION_CHANGED,
		static_cast<WPARAM>(m_nSelectedRowCount), 0);
}

void SageResultTablePanel::OnSearch() {
	m_strKeyword = m_wndSearch.GetKeyword();
	m_strKeyword.Trim();
	RefreshRows();
	NotifyStateChanged();
}

void SageResultTablePanel::OnFilterReset() {
	m_strKeyword.Empty();
	m_wndSearch.SetKeyword(L"");
	RefreshRows();
	NotifyStateChanged();
}

void SageResultTablePanel::OnCriteriaChanged() {
	int nCriteria = m_wndSearch.GetSelectedCriteria();
	if (nCriteria == CB_ERR)
		return;

	m_nCriteria = nCriteria;
	RefreshRows();
	NotifyStateChanged();
}

void SageResultTablePanel::SetAllRowsChecked(BOOL bChecked) {
	m_bUpdatingChecks = TRUE;
	int nCount = m_wndList.GetItemCount();
	for (int i = 0; i < nCount; ++i)
		m_wndList.SetCheck(i, bChecked);
	m_bUpdatingChecks = FALSE;
}

void SageResultTablePanel::OnSelectAll() {
	int nCount = m_wndList.GetItemCount();
	BOOL bCheck = (GetCheckedRowCount() == nCount && nCount > 0) ? FALSE : TRUE;
	if (bCheck && IsOnePageChecked() && nCount > SAGE_ESTIMATE_ONE_PAGE_MAX_ROWS) {
		m_bUpdatingChecks = TRUE;
		for (int i = 0; i < nCount; ++i)
			m_wndList.SetCheck(i, i < SAGE_ESTIMATE_ONE_PAGE_MAX_ROWS ? TRUE : FALSE);
		m_bUpdatingChecks = FALSE;
		NotifySelectionChanged();
		return;
	}
	SetAllRowsChecked(bCheck);
	NotifySelectionChanged();
}

void SageResultTablePanel::OnSelectionClear() {
	SetAllRowsChecked(FALSE);
	NotifySelectionChanged();
}

void SageResultTablePanel::OnOnePageOption() {
	TrimCheckedRowsToOnePage();
	NotifySelectionChanged();
}

void SageResultTablePanel::TrimCheckedRowsToOnePage() {
	if (!IsOnePageChecked())
		return;

	m_bUpdatingChecks = TRUE;
	int nCheckedCount = 0;
	int nListCount = m_wndList.GetItemCount();
	for (int i = 0; i < nListCount; ++i) {
		if (!m_wndList.GetCheck(i))
			continue;
		++nCheckedCount;
		if (nCheckedCount > SAGE_ESTIMATE_ONE_PAGE_MAX_ROWS)
			m_wndList.SetCheck(i, FALSE);
	}
	m_bUpdatingChecks = FALSE;
}

void SageResultTablePanel::OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	*pResult = 0;
	if (m_bUpdatingChecks)
		return;

	NM_LISTVIEW* pList = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	if ((pList->uChanged & LVIF_STATE) == 0 || pList->iItem < 0)
		return;

	UINT uOldCheck = pList->uOldState & LVIS_STATEIMAGEMASK;
	UINT uNewCheck = pList->uNewState & LVIS_STATEIMAGEMASK;
	if (uOldCheck == 0 || uNewCheck == 0 || uOldCheck == uNewCheck)
		return;

	if (IsOnePageChecked() && uNewCheck == INDEXTOSTATEIMAGEMASK(2)
		&& GetCheckedRowCount() > SAGE_ESTIMATE_ONE_PAGE_MAX_ROWS) {
		m_bUpdatingChecks = TRUE;
		m_wndList.SetCheck(pList->iItem, FALSE);
		m_bUpdatingChecks = FALSE;
		return;
	}

	NotifySelectionChanged();
}

void SageResultTablePanel::OnListGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) {
	*pResult = 0;

	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if ((pDispInfo->item.mask & LVIF_TEXT) == 0 || pDispInfo->item.pszText == NULL
		|| pDispInfo->item.cchTextMax < 1)
		return;

	pDispInfo->item.pszText[0] = L'\0';

	int nRow = pDispInfo->item.iItem;
	int nColumn = pDispInfo->item.iSubItem;
	if (nRow < 0 || nRow >= static_cast<int>(m_arrVisibleRows.size()))
		return;
	if (nColumn < 0 || nColumn >= static_cast<int>(m_arrColumns.size()))
		return;

	CString strText = SageWorkflowResultTable::GetRowText(m_arrVisibleRows[nRow], m_arrColumns[nColumn].nField);
	::lstrcpynW(pDispInfo->item.pszText, strText, pDispInfo->item.cchTextMax);
}
