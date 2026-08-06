#include "pch.h"
#include "app/ui/panels/SageResultTablePanel.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"
#include <uxtheme.h>

BEGIN_MESSAGE_MAP(SageResultTablePanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(ID_TAECHANG_RESULT_SEARCH_BTN, &SageResultTablePanel::OnSearch)
	ON_BN_CLICKED(ID_TAECHANG_RESULT_RESET_BTN, &SageResultTablePanel::OnFilterReset)
	ON_CBN_SELCHANGE(ID_TAECHANG_RESULT_FILTER_CRITERIA, &SageResultTablePanel::OnCriteriaChanged)
	ON_BN_CLICKED(ID_TAECHANG_SELECT_ALL, &SageResultTablePanel::OnSelectAll)
	ON_BN_CLICKED(ID_TAECHANG_SELECTION_CLEAR, &SageResultTablePanel::OnSelectionClear)
	ON_BN_CLICKED(ID_TAECHANG_ESTIMATE_ONE_PAGE, &SageResultTablePanel::OnOnePageOption)
	ON_NOTIFY(LVN_ITEMCHANGED, ID_TAECHANG_RESULT_LIST, &SageResultTablePanel::OnListItemChanged)
END_MESSAGE_MAP()

SageResultTablePanel::SageResultTablePanel()
	: m_rectFilterCard(0, 0, 0, 0)
	, m_nCriteria(TAECHANG_FILTER_CRITERIA_NONE)
	, m_bTitleVisible(FALSE)
	, m_bSelectAllVisible(FALSE)
	, m_bOnePageVisible(FALSE)
	, m_bFilterVisible(FALSE)
	, m_bUpdatingChecks(FALSE) {
}

static void AcceptDroppedFiles(CWnd& wnd) {
	HWND hWnd = wnd.GetSafeHwnd();
	if (hWnd == NULL || !::IsWindow(hWnd))
		return;

	::DragAcceptFiles(hWnd, TRUE);
	::ChangeWindowMessageFilterEx(hWnd, WM_DROPFILES, MSGFLT_ALLOW, NULL);
	::ChangeWindowMessageFilterEx(hWnd, WM_COPYDATA, MSGFLT_ALLOW, NULL);
	::ChangeWindowMessageFilterEx(hWnd, WM_TAECHANG_COPYGLOBALDATA, MSGFLT_ALLOW, NULL);
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
		pMsg->hwnd == m_wndFilter.GetSafeHwnd()) {
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
	m_wndTitle.Create(TAECHANG_UI_SECTION_RESULT, WS_CHILD | SS_OWNERDRAW, r, this, ID_TAECHANG_RESULT_SECTION);
	m_wndSelectionBar.Create(L"", WS_CHILD | SS_OWNERDRAW, r, this, ID_TAECHANG_RESULT_SELECTION_BAR);
	m_wndSelectionBar.SetCommands(ID_TAECHANG_SELECT_ALL, ID_TAECHANG_SELECTION_CLEAR);
	m_wndOnePage.Create(TAECHANG_UI_ESTIMATE_ONE_PAGE_CHECK, WS_CHILD | BS_AUTOCHECKBOX, r, this, ID_TAECHANG_ESTIMATE_ONE_PAGE);

	m_wndCriteria.Create(WS_CHILD | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS | WS_VSCROLL, r, this, ID_TAECHANG_RESULT_FILTER_CRITERIA);
	m_wndFilter.Create(WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_TAECHANG_RESULT_FILTER_EDIT);
	m_wndFilter.LimitText(TAECHANG_RESULT_FILTER_MAX_LENGTH);
	m_wndSearchBtn.Create(TAECHANG_UI_RESULT_SEARCH_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_TAECHANG_RESULT_SEARCH_BTN);
	m_wndSearchBtn.SetIcon(SAGE_BUTTON_ICON_SEARCH);
	m_wndSearchBtn.SetTooltip(TAECHANG_UI_TIP_SEARCH);
	m_wndResetBtn.Create(TAECHANG_UI_RESULT_RESET_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_TAECHANG_RESULT_RESET_BTN);
	m_wndResetBtn.SetVariant(SAGE_BUTTON_GHOST);
	m_wndResetBtn.SetIcon(SAGE_BUTTON_ICON_RESET);
	m_wndResetBtn.SetTooltip(TAECHANG_UI_TIP_RESET);

	m_wndSummaryBar.Create(L"", WS_CHILD | SS_OWNERDRAW, r, this, ID_TAECHANG_RESULT_SUMMARY_BAR);
	m_wndTotalBar.Create(L"", WS_CHILD | SS_OWNERDRAW, r, this, ID_TAECHANG_RESULT_TOTAL_BAR);

	m_wndList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL, r, this, ID_TAECHANG_RESULT_LIST);
	m_wndList.SetAlternateRowColor(TRUE);
	CHeaderCtrl* pHeader = m_wndList.GetHeaderCtrl();
	if (pHeader != NULL && pHeader->GetSafeHwnd() != NULL) {
		m_wndHeader.SubclassWindow(pHeader->GetSafeHwnd());
		SetWindowTheme(m_wndHeader.GetSafeHwnd(), L"", L"");
	}
}

void SageResultTablePanel::ApplyControlFonts() {
	m_wndTitle.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndOnePage.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCriteria.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCriteria.SetItemHeight(-1, TAECHANG_RESULT_CRITERIA_ITEM_HEIGHT);
	m_wndCriteria.SetItemHeight(0, TAECHANG_RESULT_CRITERIA_ITEM_HEIGHT);
	m_wndFilter.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSearchBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndResetBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_HEADER));
	m_wndList.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
	if (::IsWindow(m_wndHeader.GetSafeHwnd()))
		m_wndHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
}

int SageResultTablePanel::GetBandHeight() const {
	return TAECHANG_RESULT_FILTER_TOP_LIFT + TAECHANG_RESULT_FILTER_BOX_PAD;
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
	m_wndCriteria.ShowWindow(nCmd);
	m_wndFilter.ShowWindow(nCmd);
	m_wndSearchBtn.ShowWindow(nCmd);
	m_wndResetBtn.ShowWindow(nCmd);
	if (bShow) {
		PopulateCriteria();
		return;
	}
	if (m_rectFilterCard.IsRectEmpty())
		return;
	CRect rectStale = m_rectFilterCard;
	m_rectFilterCard.SetRectEmpty();
	rectStale.InflateRect(TAECHANG_CARD_REPAINT_MARGIN, TAECHANG_CARD_REPAINT_MARGIN);
	InvalidateRect(rectStale, TRUE);
}

void SageResultTablePanel::EnableSelectionControls(BOOL bEnable) {
	m_wndSelectionBar.EnableControls(bEnable);
	m_wndOnePage.EnableWindow(bEnable);
}

BOOL SageResultTablePanel::IsOnePageChecked() const {
	return (m_wndOnePage.GetCheck() == BST_CHECKED) ? TRUE : FALSE;
}

void SageResultTablePanel::SetOnePageChecked(BOOL bChecked) {
	m_wndOnePage.SetCheck(bChecked ? BST_CHECKED : BST_UNCHECKED);
}

void SageResultTablePanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);

	int nBandTop = TAECHANG_RESULT_FILTER_TOP_LIFT + TAECHANG_RESULT_FILTER_BOX_PAD;
	int nWidth = rectPanel.Width();
	int nFilterTotalW = TAECHANG_RESULT_CRITERIA_WIDTH + TAECHANG_ACTION_GAP
		+ TAECHANG_RESULT_FILTER_WIDTH + TAECHANG_ACTION_GAP
		+ TAECHANG_RESULT_SEARCH_WIDTH + TAECHANG_ACTION_GAP + TAECHANG_RESULT_RESET_WIDTH;
	int nFilterLeft = nWidth - nFilterTotalW;
	int nBandRight = m_bFilterVisible ? (nFilterLeft - TAECHANG_ROW_GAP) : nWidth;
	if (nBandRight < 0)
		nBandRight = 0;

	if (m_bSelectAllVisible) {
		m_wndTitle.MoveWindow(0, 0, 0, 0);
		int nBarWidth = m_wndSelectionBar.GetContentWidth();
		if (nBarWidth > nBandRight)
			nBarWidth = nBandRight;
		m_wndSelectionBar.MoveWindow(0, nBandTop - TAECHANG_BUTTON_VERT_ADJUST, nBarWidth, TAECHANG_BUTTON_HEIGHT);
		if (m_bOnePageVisible) {
			int nOnePageLeft = nBarWidth + TAECHANG_SELECTION_BAR_GAP;
			m_wndOnePage.MoveWindow(nOnePageLeft, nBandTop - TAECHANG_BUTTON_VERT_ADJUST,
				TAECHANG_ESTIMATE_ONE_PAGE_WIDTH, TAECHANG_BUTTON_HEIGHT);
		}
	} else {
		m_wndTitle.MoveWindow(0, nBandTop, nBandRight, TAECHANG_RESULT_HEADER_HEIGHT);
	}

	if (m_bFilterVisible) {
		int nFilterTop = nBandTop - TAECHANG_RESULT_FILTER_TOP_LIFT;
		m_wndCriteria.MoveWindow(nFilterLeft, nFilterTop, TAECHANG_RESULT_CRITERIA_WIDTH, TAECHANG_EDIT_HEIGHT * TAECHANG_RESULT_CRITERIA_DROP_ROWS);
		int nFilterEditLeft = nFilterLeft + TAECHANG_RESULT_CRITERIA_WIDTH + TAECHANG_ACTION_GAP;
		m_wndFilter.MoveWindow(nFilterEditLeft, nFilterTop, TAECHANG_RESULT_FILTER_WIDTH, TAECHANG_EDIT_HEIGHT);
		CRect rcFmt;
		m_wndFilter.GetClientRect(&rcFmt);
		rcFmt.top += TAECHANG_EDIT_TEXT_TOP_PAD;
		rcFmt.left += TAECHANG_RESULT_FILTER_TEXT_LEFT_PAD;
		rcFmt.right = TAECHANG_EDIT_FORMAT_MAX_WIDTH;
		m_wndFilter.SendMessage(EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcFmt));
		int nSearchLeft = nFilterEditLeft + TAECHANG_RESULT_FILTER_WIDTH + TAECHANG_ACTION_GAP;
		m_wndSearchBtn.MoveWindow(nSearchLeft, nFilterTop, TAECHANG_RESULT_SEARCH_WIDTH, TAECHANG_BUTTON_HEIGHT);
		int nResetLeft = nSearchLeft + TAECHANG_RESULT_SEARCH_WIDTH + TAECHANG_ACTION_GAP;
		m_wndResetBtn.MoveWindow(nResetLeft, nFilterTop, TAECHANG_RESULT_RESET_WIDTH, TAECHANG_BUTTON_HEIGHT);
		m_rectFilterCard.SetRect(
			nFilterLeft - TAECHANG_RESULT_FILTER_BOX_PAD,
			nFilterTop - TAECHANG_RESULT_FILTER_BOX_PAD,
			nResetLeft + TAECHANG_RESULT_RESET_WIDTH + TAECHANG_RESULT_FILTER_BOX_PAD,
			nFilterTop + TAECHANG_EDIT_HEIGHT + TAECHANG_RESULT_FILTER_BOX_PAD);
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

	int nListTop = GetBandHeight() + TAECHANG_RESULT_HEADER_HEIGHT;
	int nBandBottom = GetBandHeight() - TAECHANG_BUTTON_VERT_ADJUST + TAECHANG_BUTTON_HEIGHT;
	if (m_bSelectAllVisible && nListTop < nBandBottom + TAECHANG_ROW_GAP)
		nListTop = nBandBottom + TAECHANG_ROW_GAP;
	if (m_wndSummaryBar.HasItems()) {
		m_wndSummaryBar.MoveWindow(0, nListTop, nWidth, TAECHANG_SUMMARY_BAR_HEIGHT);
		nListTop += TAECHANG_SUMMARY_BAR_HEIGHT + TAECHANG_ROW_GAP;
	}

	int nTotalBarHeight = m_arrTotalCells.empty() ? 0 : TAECHANG_TOTAL_BAR_HEIGHT;
	int nListHeight = nHeight - nListTop - nTotalBarHeight;
	if (nListHeight < TAECHANG_RESULT_MIN_HEIGHT)
		nListHeight = TAECHANG_RESULT_MIN_HEIGHT;
	m_wndList.MoveWindow(0, nListTop, nWidth, nListHeight);
	if (nTotalBarHeight > 0)
		m_wndTotalBar.MoveWindow(0, nListTop + nListHeight, nWidth, nTotalBarHeight);
	UpdateColumnWidths();
}

BOOL SageResultTablePanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
	if (!m_rectFilterCard.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectFilterCard, TAECHANG_COLOR_PANEL);
		CBrush brFilterBox(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectFilterCard, &brFilterBox);
	}
	DrawEditBorder(pDC, m_wndCriteria);
	DrawEditBorder(pDC, m_wndFilter);
	return TRUE;
}

void SageResultTablePanel::DrawEditBorder(CDC* pDC, CWnd& wnd) {
	if (!::IsWindow(wnd.GetSafeHwnd()) || !wnd.IsWindowVisible())
		return;
	CRect rect;
	wnd.GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.InflateRect(1, 1);
	pDC->FillSolidRect(rect.left, rect.top, rect.Width(), 1, TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.bottom - 1, rect.Width(), 1, TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.top, 1, rect.Height(), TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.right - 1, rect.top, 1, rect.Height(), TAECHANG_COLOR_BORDER);
}

HBRUSH SageResultTablePanel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
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

void SageResultTablePanel::SetColumns(const std::vector<SageWorkflowColumn>& arrColumns, const SageWorkflowResultStyle& style) {
	m_arrColumns = arrColumns;
	m_style = style;
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return;

	m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_wndList.SetCheckboxes(style.bCheckbox);
	m_wndList.SetRowSeparator(style.bGridLines);
	m_wndList.SetHighlightColumns(style.nHighlightStart, style.nHighlightCount);
	m_wndList.SetGroupColumn(style.nGroupColumn);

	m_wndList.DeleteAllItems();
	CHeaderCtrl* pHeader = m_wndList.GetHeaderCtrl();
	int nOldColumnCount = (pHeader != NULL) ? pHeader->GetItemCount() : 0;
	for (int i = nOldColumnCount - 1; i >= 0; --i)
		m_wndList.DeleteColumn(i);

	for (int i = 0; i < static_cast<int>(m_arrColumns.size()); ++i) {
		const SageWorkflowColumn& column = m_arrColumns[i];
		int nFormat = (column.nAlign == SAGE_COLUMN_ALIGN_RIGHT) ? LVCFMT_RIGHT : LVCFMT_LEFT;
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

	int nColumnCount = static_cast<int>(m_arrColumns.size());
	int nFixedWidth = 0;
	int nDefinedWidth = 0;
	BOOL bHasStretchColumn = FALSE;
	for (int i = 0; i < nColumnCount; ++i) {
		nDefinedWidth += m_arrColumns[i].nWidth;
		if (m_arrColumns[i].bStretch)
			bHasStretchColumn = TRUE;
		else
			nFixedWidth += m_arrColumns[i].nWidth;
	}

	if (bHasStretchColumn) {
		for (int i = 0; i < nColumnCount; ++i) {
			int nColumnWidth = m_arrColumns[i].nWidth;
			if (m_arrColumns[i].bStretch && nWidth - nFixedWidth > nColumnWidth)
				nColumnWidth = nWidth - nFixedWidth;
			m_wndList.SetColumnWidth(i, nColumnWidth);
		}
		UpdateTotalBarCells();
		return;
	}

	int nAssignedWidth = 0;
	for (int i = 0; i < nColumnCount; ++i) {
		int nColumnWidth = m_arrColumns[i].nWidth;
		if (nWidth > nDefinedWidth) {
			nColumnWidth = (i == nColumnCount - 1)
				? nWidth - nAssignedWidth
				: ::MulDiv(m_arrColumns[i].nWidth, nWidth, nDefinedWidth);
		}
		nAssignedWidth += nColumnWidth;
		m_wndList.SetColumnWidth(i, nColumnWidth);
	}
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
		barCell.bRightAlign = (m_arrColumns[cell.nColumn].nAlign == SAGE_COLUMN_ALIGN_RIGHT) ? TRUE : FALSE;
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
		return TAECHANG_FILTER_CRITERIA_NONE;
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
	if (!::IsWindow(m_wndCriteria.GetSafeHwnd()))
		return;

	int nEffective = GetEffectiveCriteria();
	m_wndCriteria.ResetContent();
	for (int i = 0; i < static_cast<int>(m_arrCriteria.size()); ++i) {
		int nIndex = m_wndCriteria.AddString(m_arrCriteria[i].pszLabel);
		m_wndCriteria.SetItemData(nIndex, m_arrCriteria[i].nCriteria);
	}

	int nCount = m_wndCriteria.GetCount();
	for (int i = 0; i < nCount; ++i) {
		if (static_cast<int>(m_wndCriteria.GetItemData(i)) == nEffective) {
			m_wndCriteria.SetCurSel(i);
			return;
		}
	}
	if (nCount > 0)
		m_wndCriteria.SetCurSel(0);
}

void SageResultTablePanel::SetRows(const std::vector<TaechangResultRow>& arrRows) {
	m_arrRows = arrRows;
	RefreshRows();
}

void SageResultTablePanel::ClearRows() {
	m_arrRows.clear();
	m_arrVisibleRows.clear();
	if (::IsWindow(m_wndList.GetSafeHwnd()))
		m_wndList.DeleteAllItems();
}

const std::vector<TaechangResultRow>& SageResultTablePanel::GetVisibleRows() const {
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
		arrBarItems.push_back(barItem);
	}

	m_wndSummaryBar.SetItems(arrBarItems);
	m_wndSummaryBar.ShowWindow(arrBarItems.empty() ? SW_HIDE : SW_SHOW);
	LayoutTableArea();
}

void SageResultTablePanel::ClearSummary() {
	m_wndSummaryBar.SetItems(std::vector<SageSummaryBarItem>());
	m_wndSummaryBar.ShowWindow(SW_HIDE);
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

		int nColumnCount = static_cast<int>(m_arrColumns.size());
		if (nColumnCount < 1)
			continue;
		int nCount = m_wndList.GetItemCount();
		int nIndex = m_wndList.InsertItem(nCount, SageWorkflowResultTable::GetRowText(m_arrRows[i], m_arrColumns[0].nField));
		for (int nCol = 1; nCol < nColumnCount; ++nCol)
			m_wndList.SetItemText(nIndex, nCol, SageWorkflowResultTable::GetRowText(m_arrRows[i], m_arrColumns[nCol].nField));
		m_wndList.SetItemData(nIndex, static_cast<DWORD_PTR>(m_arrRows[i].m_nSourceRowIndex));
		m_arrVisibleRows.push_back(m_arrRows[i]);
	}
	m_wndList.SetRedraw(TRUE);
	m_wndList.Invalidate();
	m_bUpdatingChecks = FALSE;
	SyncSelectionBar();
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
		strNum.Format(TAECHANG_UI_ROW_NUM_FORMAT, static_cast<unsigned long>(nSourceRowIndex));
		if (!strCheckedRowNums.IsEmpty())
			strCheckedRowNums += TAECHANG_UI_ROW_NUM_SEPARATOR;
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
		strCurrentNum.Format(TAECHANG_UI_ROW_NUM_FORMAT, static_cast<unsigned long>(nSourceRowIndex));
		CString strRemaining = strCheckedRowNums;
		int nTokenIndex = 0;
		CString strToken = strRemaining.Tokenize(TAECHANG_UI_ROW_NUM_SEPARATOR, nTokenIndex);
		while (!strToken.IsEmpty()) {
			strToken.Trim();
			if (strToken == strCurrentNum) {
				m_wndList.SetCheck(i, TRUE);
				break;
			}
			strToken = strRemaining.Tokenize(TAECHANG_UI_ROW_NUM_SEPARATOR, nTokenIndex);
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
	if (::IsWindow(m_wndFilter.GetSafeHwnd()))
		m_wndFilter.SetWindowTextW(m_strKeyword);
}

void SageResultTablePanel::NotifyStateChanged() {
	CWnd* pParent = GetParent();
	if (pParent == NULL || !::IsWindow(pParent->GetSafeHwnd()))
		return;
	pParent->SendMessage(WM_TAECHANG_RESULT_TABLE_CHANGED, 0, 0);
}

void SageResultTablePanel::SyncSelectionBar() {
	if (!::IsWindow(m_wndSelectionBar.GetSafeHwnd()) || !::IsWindow(m_wndList.GetSafeHwnd()))
		return;
	int nTotalCount = m_wndList.GetItemCount();
	int nSelectedCount = GetCheckedRowCount();
	m_wndSelectionBar.SetCounts(nTotalCount, nSelectedCount);
	m_wndSelectionBar.SetAllChecked((nTotalCount > 0 && nSelectedCount == nTotalCount) ? TRUE : FALSE);
}

void SageResultTablePanel::NotifySelectionChanged() {
	SyncSelectionBar();
	CWnd* pParent = GetParent();
	if (pParent == NULL || !::IsWindow(pParent->GetSafeHwnd()))
		return;
	pParent->SendMessage(WM_TAECHANG_RESULT_SELECTION_CHANGED, 0, 0);
}

void SageResultTablePanel::OnSearch() {
	m_wndFilter.GetWindowTextW(m_strKeyword);
	m_strKeyword.Trim();
	RefreshRows();
	NotifyStateChanged();
}

void SageResultTablePanel::OnFilterReset() {
	m_strKeyword.Empty();
	m_wndFilter.SetWindowTextW(L"");
	RefreshRows();
	NotifyStateChanged();
}

void SageResultTablePanel::OnCriteriaChanged() {
	int nSel = m_wndCriteria.GetCurSel();
	if (nSel == CB_ERR)
		return;

	m_nCriteria = static_cast<int>(m_wndCriteria.GetItemData(nSel));
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
	if (bCheck && IsOnePageChecked() && nCount > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS) {
		m_bUpdatingChecks = TRUE;
		for (int i = 0; i < nCount; ++i)
			m_wndList.SetCheck(i, i < TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS ? TRUE : FALSE);
		m_bUpdatingChecks = FALSE;
		AfxMessageBox(TAECHANG_UI_ESTIMATE_ONE_PAGE_LIMIT, MB_ICONWARNING);
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
	TrimCheckedRowsToOnePage(TRUE);
	NotifySelectionChanged();
}

void SageResultTablePanel::TrimCheckedRowsToOnePage(BOOL bShowMessage) {
	if (!IsOnePageChecked())
		return;

	m_bUpdatingChecks = TRUE;
	int nCheckedCount = 0;
	int nListCount = m_wndList.GetItemCount();
	for (int i = 0; i < nListCount; ++i) {
		if (!m_wndList.GetCheck(i))
			continue;
		++nCheckedCount;
		if (nCheckedCount > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS)
			m_wndList.SetCheck(i, FALSE);
	}
	m_bUpdatingChecks = FALSE;
	if (bShowMessage && nCheckedCount > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS)
		AfxMessageBox(TAECHANG_UI_ESTIMATE_ONE_PAGE_LIMIT, MB_ICONWARNING);
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
		&& GetCheckedRowCount() > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS) {
		m_bUpdatingChecks = TRUE;
		m_wndList.SetCheck(pList->iItem, FALSE);
		m_bUpdatingChecks = FALSE;
		AfxMessageBox(TAECHANG_UI_ESTIMATE_ONE_PAGE_LIMIT, MB_ICONWARNING);
		return;
	}

	NotifySelectionChanged();
}
