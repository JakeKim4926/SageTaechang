#include "pch.h"
#include "app/ui/panels/SageCompanyOrderPanel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/infra/db/SageDBMgr.h"
#include "TaechangDefine.h"
#include <uxtheme.h>

BEGIN_MESSAGE_MAP(SageCompanyOrderPanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(ID_COORDER_ADD_BTN, &SageCompanyOrderPanel::OnAdd)
	ON_BN_CLICKED(ID_COORDER_SAVE_BTN, &SageCompanyOrderPanel::OnSave)
	ON_BN_CLICKED(ID_COORDER_DELETE_BTN, &SageCompanyOrderPanel::OnDelete)
	ON_BN_CLICKED(ID_COORDER_CANCEL_BTN, &SageCompanyOrderPanel::OnCancel)
	ON_BN_CLICKED(ID_COORDER_SEARCH_BTN, &SageCompanyOrderPanel::OnSearch)
	ON_BN_CLICKED(ID_COORDER_MOVE_UP_BTN, &SageCompanyOrderPanel::OnMoveUp)
	ON_BN_CLICKED(ID_COORDER_MOVE_DOWN_BTN, &SageCompanyOrderPanel::OnMoveDown)
	ON_NOTIFY(LVN_ITEMCHANGED, ID_COORDER_LIST, &SageCompanyOrderPanel::OnListSelChanged)
END_MESSAGE_MAP()

SageCompanyOrderPanel::SageCompanyOrderPanel()
	: m_rectListCard(0, 0, 0, 0)
	, m_rectEditCard(0, 0, 0, 0)
	, m_nDividerTop(0)
	, m_nPanelState(TAECHANG_CO_PANEL_IDLE)
	, m_nSelectedOrderId(0) {
}

BOOL SageCompanyOrderPanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD | WS_CLIPCHILDREN, rectEmpty, pParent, nId);
}

int SageCompanyOrderPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CreateControls();
	ApplyControlFonts();
	ApplyLabelRoles();
	return 0;
}

BOOL SageCompanyOrderPanel::PreTranslateMessage(MSG* pMsg) {
	if (pMsg != NULL && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
		m_wndSearch.IsEditMessage(pMsg)) {
		OnSearch();
		return TRUE;
	}
	if (pMsg != NULL && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB) {
		if (pMsg->hwnd == m_wndOrderEdit.GetSafeHwnd()) {
			m_wndCompanyEdit.SetFocus();
			return TRUE;
		}
		if (pMsg->hwnd == m_wndCompanyEdit.GetSafeHwnd()) {
			m_wndOrderEdit.SetFocus();
			return TRUE;
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}

void SageCompanyOrderPanel::CreateControls() {
	CRect r(0, 0, 0, 0);
	m_wndCrudSection.Create(TAECHANG_UI_CO_CRUD_SECTION, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this, ID_COORDER_CRUD_SECTION);
	m_wndListSection.Create(TAECHANG_UI_CO_LIST_SECTION, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this, ID_COORDER_LIST_SECTION);
	m_wndAddBtn.Create(TAECHANG_UI_CO_ADD_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_COORDER_ADD_BTN);
	m_wndAddBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndAddBtn.SetIcon(SAGE_BUTTON_ICON_ADD);
	m_wndSaveBtn.Create(TAECHANG_UI_CO_SAVE_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_COORDER_SAVE_BTN);
	m_wndSaveBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndDeleteBtn.Create(TAECHANG_UI_CO_DELETE_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_COORDER_DELETE_BTN);
	m_wndDeleteBtn.SetVariant(SAGE_BUTTON_DANGER);
	m_wndCancelBtn.Create(TAECHANG_UI_CO_CANCEL_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_COORDER_CANCEL_BTN);
	m_wndMoveUpBtn.Create(L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_COORDER_MOVE_UP_BTN);
	m_wndMoveUpBtn.SetIcon(SAGE_BUTTON_ICON_MOVE_UP);
	m_wndMoveUpBtn.SetTooltip(TAECHANG_UI_TIP_MOVE_UP);
	m_wndMoveDownBtn.Create(L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_COORDER_MOVE_DOWN_BTN);
	m_wndMoveDownBtn.SetIcon(SAGE_BUTTON_ICON_MOVE_DOWN);
	m_wndMoveDownBtn.SetTooltip(TAECHANG_UI_TIP_MOVE_DOWN);
	m_wndSearch.CreateBox(this, ID_COORDER_SEARCH_BOX, ID_COORDER_SEARCH_EDIT);
	m_wndSearch.SetCommand(ID_COORDER_SEARCH_BTN);
	m_wndSearch.SetPlaceholder(TAECHANG_UI_CO_SEARCH_PLACEHOLDER);
	m_wndSearch.SetMaxLength(TAECHANG_CO_COMPANY_NAME_MAX);
	m_wndOrderLabel.Create(TAECHANG_UI_CO_ORDER_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndOrderEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_NUMBER | ES_RIGHT, r, this, ID_COORDER_ORDER_EDIT);
	m_wndOrderEdit.LimitText(TAECHANG_CO_ORDER_TEXT_MAX);
	m_wndNameLabel.Create(TAECHANG_UI_CO_NAME_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCompanyEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_COORDER_COMPANY_EDIT);
	m_wndCompanyEdit.LimitText(TAECHANG_CO_COMPANY_NAME_MAX);
	m_wndGuide.Create(TAECHANG_UI_CO_GUIDE, WS_CHILD | WS_VISIBLE | SS_LEFT, r, this);
	m_wndList.Create(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, r, this, ID_COORDER_LIST);
	m_wndList.SetAlternateRowColor(TRUE);
	m_wndList.SetFirstColumnAlign(SAGE_LIST_FIRST_COLUMN_RIGHT);
	m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_wndList.SetRowSeparator(TRUE);
	m_wndList.InsertColumn(0, TAECHANG_UI_CO_COL_ORDER, LVCFMT_RIGHT, TAECHANG_CO_ORDER_COL_WIDTH);
	m_wndList.InsertColumn(1, TAECHANG_UI_CO_COL_COMPANY, LVCFMT_LEFT, TAECHANG_CO_ORDER_COL_WIDTH);

	CHeaderCtrl* pHeader = m_wndList.GetHeaderCtrl();
	if (pHeader == NULL)
		return;
	m_wndListHeader.SubclassWindow(pHeader->GetSafeHwnd());
	SetWindowTheme(m_wndListHeader.GetSafeHwnd(), L"", L"");
}

void SageCompanyOrderPanel::ApplyControlFonts() {
	m_wndCrudSection.SetFont(SageUiResources::GetFont(SAGE_FONT_HEADER));
	m_wndListSection.SetFont(SageUiResources::GetFont(SAGE_FONT_HEADER));
	m_wndAddBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSaveBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndDeleteBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCancelBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndOrderEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCompanyEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndList.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
	if (::IsWindow(m_wndListHeader.GetSafeHwnd()))
		m_wndListHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
}

void SageCompanyOrderPanel::ApplyLabelRoles() {
	m_wndOrderLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndOrderLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndOrderLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndNameLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndNameLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndNameLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndGuide.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndGuide.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndGuide.SetFontRole(SAGE_FONT_CAPTION);
}

void SageCompanyOrderPanel::ApplyOrderEditTextRect() {
	CRect rcFmt;
	m_wndOrderEdit.GetClientRect(&rcFmt);
	rcFmt.DeflateRect(TAECHANG_EDIT_TEXT_LEFT_PAD, 0);
	rcFmt.top += TAECHANG_EDIT_TEXT_TOP_PAD;
	m_wndOrderEdit.SendMessage(EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcFmt));
}

void SageCompanyOrderPanel::ApplyEditTextRect(CEdit& wndEdit, int nLeftPad) {
	CRect rcFmt;
	wndEdit.GetClientRect(&rcFmt);
	rcFmt.top += TAECHANG_EDIT_TEXT_TOP_PAD;
	rcFmt.left += nLeftPad;
	rcFmt.right = TAECHANG_EDIT_FORMAT_MAX_WIDTH;
	wndEdit.SendMessage(EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcFmt));
}

void SageCompanyOrderPanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	if (rectClient.IsRectEmpty())
		return;

	int nListCardWidth = rectClient.Width() - TAECHANG_CO_CARD_GAP - TAECHANG_CO_EDIT_CARD_WIDTH;
	if (nListCardWidth > TAECHANG_CO_LIST_CARD_WIDTH)
		nListCardWidth = TAECHANG_CO_LIST_CARD_WIDTH;
	int nEditCardLeft = nListCardWidth + TAECHANG_CO_CARD_GAP;

	CRect rectListCard(0, 0, nListCardWidth, rectClient.Height());
	LayoutListCard(rectListCard);

	int nEditCardBottom = LayoutEditCard(nEditCardLeft, TAECHANG_CO_EDIT_CARD_WIDTH);
	SetCardRects(rectListCard,
		CRect(nEditCardLeft, 0, nEditCardLeft + TAECHANG_CO_EDIT_CARD_WIDTH, nEditCardBottom));
}

void SageCompanyOrderPanel::LayoutListCard(const CRect& rectCard) {
	m_wndListSection.MoveWindow(
		rectCard.left, rectCard.top, rectCard.Width(), TAECHANG_CARD_HEADER_HEIGHT);

	int nRowTop = rectCard.top + TAECHANG_CARD_HEADER_HEIGHT;
	int nSearchTop = nRowTop + (TAECHANG_CO_SEARCH_ROW_HEIGHT - TAECHANG_EDIT_HEIGHT) / 2;
	int nContentLeft = rectCard.left + TAECHANG_CARD_PADDING;
	int nContentRight = rectCard.right - TAECHANG_CARD_PADDING;

	int nAddBtnLeft = nContentRight - TAECHANG_CO_ADD_BTN_WIDTH;
	m_wndAddBtn.MoveWindow(nAddBtnLeft, nSearchTop, TAECHANG_CO_ADD_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);

	m_wndSearch.MoveWindow(
		nContentLeft, nSearchTop,
		nAddBtnLeft - TAECHANG_ACTION_GAP - nContentLeft, TAECHANG_EDIT_HEIGHT);

	int nListTop = nRowTop + TAECHANG_CO_SEARCH_ROW_HEIGHT;
	int nListHeight = rectCard.bottom - nListTop - TAECHANG_EDIT_BORDER_WIDTH;
	if (nListHeight < TAECHANG_RESULT_MIN_HEIGHT)
		nListHeight = TAECHANG_RESULT_MIN_HEIGHT;
	m_wndList.MoveWindow(
		rectCard.left + TAECHANG_EDIT_BORDER_WIDTH,
		nListTop,
		rectCard.Width() - TAECHANG_EDIT_BORDER_WIDTH * 2,
		nListHeight);
	UpdateListColumns();
}

int SageCompanyOrderPanel::LayoutEditCard(int nLeft, int nWidth) {
	m_wndCrudSection.MoveWindow(nLeft, 0, nWidth, TAECHANG_CARD_HEADER_HEIGHT);

	int nContentLeft = nLeft + TAECHANG_CARD_PADDING;
	int nContentRight = nLeft + nWidth - TAECHANG_CARD_PADDING;
	int nTop = TAECHANG_CARD_HEADER_HEIGHT + TAECHANG_CARD_PADDING;

	m_wndOrderLabel.MoveWindow(
		nContentLeft, nTop, TAECHANG_CO_FORM_LABEL_WIDTH, TAECHANG_EDIT_HEIGHT);
	m_wndOrderEdit.MoveWindow(
		nContentLeft + TAECHANG_CO_FORM_LABEL_WIDTH + TAECHANG_CARD_ROW_GAP, nTop,
		TAECHANG_CO_ORDER_EDIT_WIDTH, TAECHANG_EDIT_HEIGHT);
	ApplyOrderEditTextRect();

	int nMoveLeft = nContentLeft + TAECHANG_CO_FORM_LABEL_WIDTH + TAECHANG_CARD_ROW_GAP
		+ TAECHANG_CO_ORDER_EDIT_WIDTH + TAECHANG_ACTION_GAP;
	m_wndMoveUpBtn.MoveWindow(nMoveLeft, nTop, TAECHANG_ICON_BUTTON_SIZE, TAECHANG_ICON_BUTTON_SIZE);
	m_wndMoveDownBtn.MoveWindow(
		nMoveLeft + TAECHANG_ICON_BUTTON_SIZE + TAECHANG_ACTION_GAP, nTop,
		TAECHANG_ICON_BUTTON_SIZE, TAECHANG_ICON_BUTTON_SIZE);

	nTop += TAECHANG_EDIT_HEIGHT + TAECHANG_CARD_ROW_GAP;
	m_wndNameLabel.MoveWindow(
		nContentLeft, nTop, TAECHANG_CO_FORM_LABEL_WIDTH, TAECHANG_EDIT_HEIGHT);
	int nNameEditLeft = nContentLeft + TAECHANG_CO_FORM_LABEL_WIDTH + TAECHANG_CARD_ROW_GAP;
	m_wndCompanyEdit.MoveWindow(
		nNameEditLeft, nTop, nContentRight - nNameEditLeft, TAECHANG_EDIT_HEIGHT);
	ApplyEditTextRect(m_wndCompanyEdit, TAECHANG_CO_NAME_TEXT_LEFT_PAD);

	nTop += TAECHANG_EDIT_HEIGHT + TAECHANG_CARD_ROW_GAP;
	m_nDividerTop = nTop;
	nTop += TAECHANG_CO_DIVIDER_HEIGHT + TAECHANG_CARD_ROW_GAP;
	int nHalfWidth = (nContentRight - nContentLeft - TAECHANG_ACTION_GAP) / 2;
	m_wndSaveBtn.MoveWindow(nContentLeft, nTop, nHalfWidth, TAECHANG_BUTTON_HEIGHT);
	m_wndCancelBtn.MoveWindow(
		nContentLeft + nHalfWidth + TAECHANG_ACTION_GAP, nTop, nHalfWidth, TAECHANG_BUTTON_HEIGHT);

	nTop += TAECHANG_BUTTON_HEIGHT + TAECHANG_CARD_ROW_GAP;
	m_wndDeleteBtn.MoveWindow(nContentLeft, nTop, nContentRight - nContentLeft, TAECHANG_BUTTON_HEIGHT);

	nTop += TAECHANG_BUTTON_HEIGHT + TAECHANG_CARD_ROW_GAP;
	m_wndGuide.MoveWindow(nContentLeft, nTop, nContentRight - nContentLeft, TAECHANG_CO_GUIDE_HEIGHT);
	return nTop + TAECHANG_CO_GUIDE_HEIGHT + TAECHANG_CARD_PADDING;
}

void SageCompanyOrderPanel::SetCardRects(const CRect& rectList, const CRect& rectEdit) {
	if (m_rectListCard == rectList && m_rectEditCard == rectEdit)
		return;

	CRect rectStale;
	rectStale.UnionRect(m_rectListCard, m_rectEditCard);
	rectStale.UnionRect(rectStale, rectList);
	rectStale.UnionRect(rectStale, rectEdit);
	m_rectListCard = rectList;
	m_rectEditCard = rectEdit;
	if (rectStale.IsRectEmpty())
		return;

	rectStale.InflateRect(TAECHANG_CARD_REPAINT_MARGIN, TAECHANG_CARD_REPAINT_MARGIN);
	InvalidateRect(rectStale, TRUE);
}

BOOL SageCompanyOrderPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);

	CBrush brushCard(TAECHANG_COLOR_BORDER);
	if (!m_rectListCard.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectListCard, TAECHANG_COLOR_PANEL);
		pDC->FrameRect(m_rectListCard, &brushCard);
	}
	if (!m_rectEditCard.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectEditCard, TAECHANG_COLOR_PANEL);
		pDC->FrameRect(m_rectEditCard, &brushCard);
		pDC->FillSolidRect(
			m_rectEditCard.left + TAECHANG_CARD_PADDING,
			m_nDividerTop,
			m_rectEditCard.Width() - TAECHANG_CARD_PADDING * 2,
			TAECHANG_CO_DIVIDER_HEIGHT,
			TAECHANG_COLOR_LIST_GRID);
	}
	DrawEditBorder(pDC, m_wndOrderEdit);
	DrawEditBorder(pDC, m_wndCompanyEdit);
	return TRUE;
}

void SageCompanyOrderPanel::DrawEditBorder(CDC* pDC, CWnd& wnd) {
	if (!::IsWindow(wnd.GetSafeHwnd()) || !wnd.IsWindowVisible())
		return;
	CRect rect;
	wnd.GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.InflateRect(TAECHANG_EDIT_BORDER_WIDTH, TAECHANG_EDIT_BORDER_WIDTH);
	pDC->FillSolidRect(rect.left, rect.top, rect.Width(), TAECHANG_EDIT_BORDER_WIDTH, TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.bottom - TAECHANG_EDIT_BORDER_WIDTH, rect.Width(), TAECHANG_EDIT_BORDER_WIDTH, TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.top, TAECHANG_EDIT_BORDER_WIDTH, rect.Height(), TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.right - TAECHANG_EDIT_BORDER_WIDTH, rect.top, TAECHANG_EDIT_BORDER_WIDTH, rect.Height(), TAECHANG_COLOR_BORDER);
}

HBRUSH SageCompanyOrderPanel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
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

void SageCompanyOrderPanel::UpdateListColumns() {
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return;

	CRect rectList;
	m_wndList.GetClientRect(&rectList);
	m_wndList.SetColumnWidth(0, TAECHANG_CO_ORDER_COL_WIDTH);
	m_wndList.SetColumnWidth(1, max(0, rectList.Width() - TAECHANG_CO_ORDER_COL_WIDTH));
}

void SageCompanyOrderPanel::UpdatePanelState() {
	BOOL bAdding = (m_nPanelState == TAECHANG_CO_PANEL_ADD) ? TRUE : FALSE;
	BOOL bHasSelection = (m_nSelectedOrderId > 0) ? TRUE : FALSE;

	m_wndAddBtn.EnableWindow(!bAdding ? TRUE : FALSE);
	m_wndSaveBtn.EnableWindow((bAdding || bHasSelection) ? TRUE : FALSE);
	m_wndDeleteBtn.EnableWindow((!bAdding && bHasSelection) ? TRUE : FALSE);
	m_wndCancelBtn.EnableWindow((bAdding || bHasSelection) ? TRUE : FALSE);
	m_wndOrderEdit.EnableWindow((bAdding || bHasSelection) ? TRUE : FALSE);
	m_wndCompanyEdit.EnableWindow((bAdding || bHasSelection) ? TRUE : FALSE);
	m_wndSearch.EnableWindow(!bAdding ? TRUE : FALSE);

	int nSelectedIndex = FindSelectedIndex();
	BOOL bCanMove = (!bAdding && nSelectedIndex != TAECHANG_LIST_NO_ITEM) ? TRUE : FALSE;
	m_wndMoveUpBtn.EnableWindow((bCanMove && nSelectedIndex > 0) ? TRUE : FALSE);
	m_wndMoveDownBtn.EnableWindow(
		(bCanMove && nSelectedIndex < m_arrOrders.GetSize() - 1) ? TRUE : FALSE);

	CString strCount;
	strCount.Format(TAECHANG_UI_CO_COUNT_FORMAT, static_cast<int>(m_arrOrders.GetSize()));
	m_wndListSection.SetHintText(strCount);
}

void SageCompanyOrderPanel::FillEditFromSelection() {
	TaechangReceivableCompanyOrderDto dto;
	if (m_nSelectedOrderId > 0 && FindSelectedDto(dto)) {
		CString strOrder;
		strOrder.Format(TAECHANG_UI_CO_ORDER_FORMAT, dto.nSortOrder);
		m_wndOrderEdit.SetWindowTextW(strOrder);
		m_wndCompanyEdit.SetWindowTextW(dto.strCompanyName);
		return;
	}
	m_wndOrderEdit.SetWindowTextW(CString());
	m_wndCompanyEdit.SetWindowTextW(CString());
}

BOOL SageCompanyOrderPanel::FindSelectedDto(TaechangReceivableCompanyOrderDto& outDto) const {
	for (int i = 0; i < m_arrOrders.GetSize(); ++i) {
		if (m_arrOrders[i].nOrderId == m_nSelectedOrderId) {
			outDto = m_arrOrders[i];
			return TRUE;
		}
	}
	return FALSE;
}

void SageCompanyOrderPanel::RefreshList() {
	if (!::IsWindow(m_wndList.GetSafeHwnd()))
		return;

	CString strError;
	if (sageDBMgr.GetReceivableCompanyOrderService()->LoadAllCompanyOrders(m_arrOrders, strError) == FALSE) {
		AfxMessageBox(strError);
		return;
	}

	m_wndList.SetRedraw(FALSE);
	m_wndList.DeleteAllItems();
	CString strFilterLower = m_strSearchKeyword;
	strFilterLower.MakeLower();
	for (int i = 0; i < m_arrOrders.GetSize(); ++i) {
		const TaechangReceivableCompanyOrderDto& dto = m_arrOrders[i];
		if (!strFilterLower.IsEmpty()) {
			CString strNameLower = dto.strCompanyName;
			strNameLower.MakeLower();
			if (strNameLower.Find(strFilterLower) < 0)
				continue;
		}
		int nItem = m_wndList.InsertItem(m_wndList.GetItemCount(), L"");
		CString strOrder;
		strOrder.Format(TAECHANG_UI_CO_ORDER_FORMAT, dto.nSortOrder);
		m_wndList.SetItemText(nItem, 0, strOrder);
		m_wndList.SetItemText(nItem, 1, dto.strCompanyName);
		m_wndList.SetItemData(nItem, static_cast<DWORD_PTR>(dto.nOrderId));
	}
	if (m_nSelectedOrderId > 0) {
		for (int i = 0; i < m_wndList.GetItemCount(); ++i) {
			if (static_cast<int>(m_wndList.GetItemData(i)) != m_nSelectedOrderId)
				continue;
			m_wndList.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			m_wndList.EnsureVisible(i, FALSE);
			break;
		}
	}
	UpdatePanelState();
	Invalidate(FALSE);
	m_wndList.SetRedraw(TRUE);
	m_wndList.Invalidate();
}

void SageCompanyOrderPanel::OnAdd() {
	m_nPanelState = TAECHANG_CO_PANEL_ADD;
	m_nSelectedOrderId = 0;
	m_wndList.SetItemState(TAECHANG_LIST_NO_ITEM, 0, LVIS_SELECTED);
	m_wndOrderEdit.SetWindowTextW(CString());
	m_wndCompanyEdit.SetWindowTextW(CString());
	UpdatePanelState();
	m_wndCompanyEdit.SetFocus();
	Invalidate(FALSE);
}

void SageCompanyOrderPanel::AddCompanyOrder(const CString& strCompanyName, const CString& strOrder) {
	int nSortOrder = TAECHANG_CO_ORDER_FIRST;
	CString strOrderStr = strOrder;
	strOrderStr.Trim();
	if (strOrderStr.IsEmpty()) {
		for (int i = 0; i < m_arrOrders.GetSize(); ++i) {
			if (m_arrOrders[i].nSortOrder >= nSortOrder)
				nSortOrder = m_arrOrders[i].nSortOrder + 1;
		}
	} else {
		nSortOrder = _wtoi(strOrderStr);
	}

	TaechangReceivableCompanyOrderDto dto;
	dto.strCompanyName = strCompanyName;
	dto.nSortOrder = nSortOrder;
	int nNewOrderId = 0;
	CString strError;
	if (sageDBMgr.GetReceivableCompanyOrderService()->AddCompanyOrder(dto, nNewOrderId, strError) == FALSE) {
		AfxMessageBox(strError);
		return;
	}

	m_nPanelState = TAECHANG_CO_PANEL_IDLE;
	m_nSelectedOrderId = nNewOrderId;
	m_wndList.SetItemState(TAECHANG_LIST_NO_ITEM, 0, LVIS_SELECTED);
	RefreshList();
}

void SageCompanyOrderPanel::OnSave() {
	CString strCompanyName;
	CString strOrderStr;
	m_wndCompanyEdit.GetWindowTextW(strCompanyName);
	m_wndOrderEdit.GetWindowTextW(strOrderStr);
	strCompanyName.Trim();
	if (strCompanyName.IsEmpty()) {
		AfxMessageBox(TAECHANG_UI_CO_COMPANY_REQUIRED);
		m_wndCompanyEdit.SetFocus();
		return;
	}

	if (m_nPanelState == TAECHANG_CO_PANEL_ADD) {
		AddCompanyOrder(strCompanyName, strOrderStr);
		return;
	}

	if (m_nSelectedOrderId <= 0) {
		AfxMessageBox(TAECHANG_UI_CO_SELECT_REQUIRED);
		return;
	}

	TaechangReceivableCompanyOrderDto dto;
	if (!FindSelectedDto(dto))
		return;

	dto.strCompanyName = strCompanyName;
	strOrderStr.Trim();
	if (!strOrderStr.IsEmpty())
		dto.nSortOrder = _wtoi(strOrderStr);

	CString strError;
	if (sageDBMgr.GetReceivableCompanyOrderService()->ChangeCompanyOrder(dto, strError) == FALSE) {
		AfxMessageBox(strError);
		return;
	}
	m_nPanelState = TAECHANG_CO_PANEL_IDLE;
	RefreshList();
}

void SageCompanyOrderPanel::OnDelete() {
	if (m_nSelectedOrderId <= 0) {
		AfxMessageBox(TAECHANG_UI_CO_SELECT_REQUIRED);
		return;
	}

	TaechangReceivableCompanyOrderDto dto;
	FindSelectedDto(dto);

	CString strConfirm;
	strConfirm.Format(TAECHANG_UI_CO_DELETE_CONFIRM_FMT, dto.strCompanyName.GetString());
	if (AfxMessageBox(strConfirm, MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;

	CString strError;
	if (sageDBMgr.GetReceivableCompanyOrderService()->RemoveCompanyOrder(m_nSelectedOrderId, strError) == FALSE) {
		AfxMessageBox(strError);
		return;
	}
	m_nSelectedOrderId = 0;
	m_wndOrderEdit.SetWindowTextW(CString());
	m_wndCompanyEdit.SetWindowTextW(CString());
	RefreshList();
}

void SageCompanyOrderPanel::OnCancel() {
	if (m_nPanelState == TAECHANG_CO_PANEL_ADD) {
		m_nPanelState = TAECHANG_CO_PANEL_IDLE;
		FillEditFromSelection();
		UpdatePanelState();
		Invalidate(FALSE);
		return;
	}

	m_nSelectedOrderId = 0;
	m_wndList.SetItemState(TAECHANG_LIST_NO_ITEM, 0, LVIS_SELECTED);
	FillEditFromSelection();
	UpdatePanelState();
	Invalidate(FALSE);
}

int SageCompanyOrderPanel::FindSelectedIndex() const {
	for (int i = 0; i < m_arrOrders.GetSize(); ++i) {
		if (m_arrOrders[i].nOrderId == m_nSelectedOrderId)
			return i;
	}
	return TAECHANG_LIST_NO_ITEM;
}

void SageCompanyOrderPanel::MoveSelected(int nOffset) {
	int nIndex = FindSelectedIndex();
	if (nIndex == TAECHANG_LIST_NO_ITEM)
		return;

	int nTargetIndex = nIndex + nOffset;
	if (nTargetIndex < 0 || nTargetIndex >= m_arrOrders.GetSize())
		return;

	CString strError;
	if (sageDBMgr.GetReceivableCompanyOrderService()->SwapCompanyOrder(
		m_arrOrders[nIndex], m_arrOrders[nTargetIndex], strError) == FALSE) {
		AfxMessageBox(strError);
		return;
	}
	RefreshList();
	FillEditFromSelection();
}

void SageCompanyOrderPanel::OnMoveUp() {
	MoveSelected(-1);
}

void SageCompanyOrderPanel::OnMoveDown() {
	MoveSelected(1);
}

void SageCompanyOrderPanel::OnSearch() {
	m_strSearchKeyword = m_wndSearch.GetKeyword();
	m_strSearchKeyword.Trim();
	m_nSelectedOrderId = 0;
	m_wndOrderEdit.SetWindowTextW(CString());
	m_wndCompanyEdit.SetWindowTextW(CString());
	RefreshList();
}

void SageCompanyOrderPanel::OnListSelChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	UNREFERENCED_PARAMETER(pNMHDR);
	*pResult = 0;

	int nSel = m_wndList.GetNextItem(TAECHANG_LIST_NO_ITEM, LVNI_SELECTED);
	m_nSelectedOrderId = (nSel >= 0) ? static_cast<int>(m_wndList.GetItemData(nSel)) : 0;
	if (m_nPanelState != TAECHANG_CO_PANEL_IDLE) {
		UpdatePanelState();
		return;
	}

	FillEditFromSelection();
	Invalidate(FALSE);
	UpdatePanelState();
}
