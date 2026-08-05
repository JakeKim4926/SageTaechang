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
	ON_BN_CLICKED(ID_TAECHANG_ESTIMATE_ONE_PAGE, &SageResultTablePanel::OnOnePageOption)
	ON_NOTIFY(LVN_ITEMCHANGED, ID_TAECHANG_RESULT_LIST, &SageResultTablePanel::OnListItemChanged)
END_MESSAGE_MAP()

SageResultTablePanel::SageResultTablePanel()
	: m_rectFilterCard(0, 0, 0, 0)
	, m_nCriteria(TAECHANG_FILTER_CRITERIA_NONE)
	, m_bTitleVisible(FALSE)
	, m_bSelectAllVisible(FALSE)
	, m_bOnePageVisible(FALSE)
	, m_bFilterVisible(FALSE) {
}

BOOL SageResultTablePanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD | WS_CLIPCHILDREN, rectEmpty, pParent, nId);
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
	m_wndSelectAll.Create(TAECHANG_UI_SELECT_ALL_BUTTON, WS_CHILD | BS_OWNERDRAW, r, this, ID_TAECHANG_SELECT_ALL);
	m_wndOnePage.Create(TAECHANG_UI_ESTIMATE_ONE_PAGE_CHECK, WS_CHILD | BS_AUTOCHECKBOX, r, this, ID_TAECHANG_ESTIMATE_ONE_PAGE);
	SetWindowTheme(m_wndOnePage.GetSafeHwnd(), L"", L"");

	m_wndCriteria.Create(WS_CHILD | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS | WS_VSCROLL, r, this, ID_TAECHANG_RESULT_FILTER_CRITERIA);
	m_wndFilter.Create(WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_TAECHANG_RESULT_FILTER_EDIT);
	m_wndSearchBtn.Create(TAECHANG_UI_RESULT_SEARCH_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_TAECHANG_RESULT_SEARCH_BTN);
	m_wndSearchBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndSearchBtn.SetIcon(SAGE_BUTTON_ICON_SEARCH);
	m_wndResetBtn.Create(TAECHANG_UI_RESULT_RESET_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_TAECHANG_RESULT_RESET_BTN);

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
	m_wndSelectAll.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndOnePage.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCriteria.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndFilter.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSearchBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndResetBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndList.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	if (::IsWindow(m_wndHeader.GetSafeHwnd()))
		m_wndHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
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
	m_wndSelectAll.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
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

BOOL SageResultTablePanel::IsOnePageChecked() const {
	if (!m_bOnePageVisible)
		return FALSE;
	return (m_wndOnePage.GetCheck() == BST_CHECKED) ? TRUE : FALSE;
}

void SageResultTablePanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);

	int nBandTop = TAECHANG_RESULT_FILTER_TOP_LIFT + TAECHANG_RESULT_FILTER_BOX_PAD;
	int nWidth = rectPanel.Width();
	int nHeight = rectPanel.Height();
	int nFilterTotalW = TAECHANG_RESULT_CRITERIA_WIDTH + TAECHANG_ACTION_GAP
		+ TAECHANG_RESULT_FILTER_WIDTH + TAECHANG_ACTION_GAP
		+ TAECHANG_RESULT_SEARCH_WIDTH + TAECHANG_ACTION_GAP + TAECHANG_RESULT_RESET_WIDTH;
	int nFilterLeft = nWidth - nFilterTotalW;
	int nBandRight = m_bFilterVisible ? (nFilterLeft - TAECHANG_ROW_GAP) : nWidth;
	if (nBandRight < 0)
		nBandRight = 0;

	if (m_bSelectAllVisible) {
		m_wndTitle.MoveWindow(0, 0, 0, 0);
		m_wndSelectAll.MoveWindow(0, nBandTop - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
		if (m_bOnePageVisible) {
			int nOnePageLeft = TAECHANG_BUTTON_WIDTH + TAECHANG_ACTION_GAP;
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

	int nListTop = nBandTop + TAECHANG_RESULT_HEADER_HEIGHT;
	int nListHeight = nHeight - nListTop;
	if (nListHeight < TAECHANG_RESULT_MIN_HEIGHT)
		nListHeight = TAECHANG_RESULT_MIN_HEIGHT;
	m_wndList.MoveWindow(0, nListTop, nWidth, nListHeight);
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

	DWORD dwExtStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	if (style.bCheckbox)
		dwExtStyle |= LVS_EX_CHECKBOXES;
	if (style.bGridLines)
		dwExtStyle |= LVS_EX_GRIDLINES;
	m_wndList.SetExtendedStyle(dwExtStyle);
	m_wndList.SetHighlightColumns(style.nHighlightStart, style.nHighlightCount);

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
	if (::IsWindow(m_wndList.GetSafeHwnd()))
		m_wndList.DeleteAllItems();
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

	m_wndList.SetRedraw(FALSE);
	m_wndList.DeleteAllItems();
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
	}
	m_wndList.SetRedraw(TRUE);
	m_wndList.Invalidate();
}

int SageResultTablePanel::GetRowCount() const {
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return 0;
	return m_wndList.GetItemCount();
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

void SageResultTablePanel::OnSelectAll() {
	int nCount = m_wndList.GetItemCount();
	BOOL bAllChecked = TRUE;
	for (int i = 0; i < nCount; ++i) {
		if (!m_wndList.GetCheck(i)) {
			bAllChecked = FALSE;
			break;
		}
	}
	BOOL bCheck = bAllChecked ? FALSE : TRUE;
	if (bCheck && IsOnePageChecked() && nCount > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS) {
		for (int i = 0; i < nCount; ++i)
			m_wndList.SetCheck(i, i < TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS ? TRUE : FALSE);
		AfxMessageBox(TAECHANG_UI_ESTIMATE_ONE_PAGE_LIMIT, MB_ICONWARNING);
		return;
	}
	for (int i = 0; i < nCount; ++i)
		m_wndList.SetCheck(i, bCheck);
}

void SageResultTablePanel::OnOnePageOption() {
	TrimCheckedRowsToOnePage(TRUE);
}

void SageResultTablePanel::TrimCheckedRowsToOnePage(BOOL bShowMessage) {
	if (!IsOnePageChecked())
		return;

	int nCheckedCount = 0;
	int nListCount = m_wndList.GetItemCount();
	for (int i = 0; i < nListCount; ++i) {
		if (!m_wndList.GetCheck(i))
			continue;
		++nCheckedCount;
		if (nCheckedCount > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS)
			m_wndList.SetCheck(i, FALSE);
	}
	if (bShowMessage && nCheckedCount > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS)
		AfxMessageBox(TAECHANG_UI_ESTIMATE_ONE_PAGE_LIMIT, MB_ICONWARNING);
}

void SageResultTablePanel::OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	*pResult = 0;
	if (!IsOnePageChecked())
		return;

	NM_LISTVIEW* pList = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	if ((pList->uChanged & LVIF_STATE) == 0 || pList->iItem < 0)
		return;

	UINT uOldCheck = pList->uOldState & LVIS_STATEIMAGEMASK;
	UINT uNewCheck = pList->uNewState & LVIS_STATEIMAGEMASK;
	if (uOldCheck == uNewCheck || uNewCheck != INDEXTOSTATEIMAGEMASK(2))
		return;

	int nCheckedCount = 0;
	int nListCount = m_wndList.GetItemCount();
	for (int i = 0; i < nListCount; ++i) {
		if (m_wndList.GetCheck(i))
			++nCheckedCount;
	}
	if (nCheckedCount <= TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS)
		return;

	m_wndList.SetCheck(pList->iItem, FALSE);
	AfxMessageBox(TAECHANG_UI_ESTIMATE_ONE_PAGE_LIMIT, MB_ICONWARNING);
}
