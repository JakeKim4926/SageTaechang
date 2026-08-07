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
	ON_BN_CLICKED(ID_COORDER_MODIFY_BTN, &SageCompanyOrderPanel::OnModify)
	ON_BN_CLICKED(ID_COORDER_DELETE_BTN, &SageCompanyOrderPanel::OnDelete)
	ON_BN_CLICKED(ID_COORDER_CANCEL_BTN, &SageCompanyOrderPanel::OnCancel)
	ON_BN_CLICKED(ID_COORDER_SEARCH_BTN, &SageCompanyOrderPanel::OnSearch)
	ON_NOTIFY(LVN_ITEMCHANGED, ID_COORDER_LIST, &SageCompanyOrderPanel::OnListSelChanged)
END_MESSAGE_MAP()

SageCompanyOrderPanel::SageCompanyOrderPanel()
	: m_rectCard(0, 0, 0, 0)
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
		pMsg->hwnd == m_wndSearchEdit.GetSafeHwnd()) {
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
	m_wndModifyBtn.Create(TAECHANG_UI_CO_MODIFY_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_COORDER_MODIFY_BTN);
	m_wndDeleteBtn.Create(TAECHANG_UI_CO_DELETE_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_COORDER_DELETE_BTN);
	m_wndDeleteBtn.SetVariant(SAGE_BUTTON_DANGER);
	m_wndCancelBtn.Create(TAECHANG_UI_CO_CANCEL_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_COORDER_CANCEL_BTN);
	m_wndSearchLabel.Create(TAECHANG_UI_CO_SEARCH_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndSearchEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_COORDER_SEARCH_EDIT);
	m_wndSearchBtn.Create(L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_COORDER_SEARCH_BTN);
	m_wndSearchBtn.SetIcon(SAGE_BUTTON_ICON_SEARCH);
	m_wndSearchBtn.SetTooltip(TAECHANG_UI_TIP_SEARCH);
	m_wndOrderLabel.Create(TAECHANG_UI_CO_ORDER_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndOrderEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_NUMBER, r, this, ID_COORDER_ORDER_EDIT);
	m_wndOrderEdit.LimitText(TAECHANG_CO_ORDER_TEXT_MAX);
	m_wndNameLabel.Create(TAECHANG_UI_CO_NAME_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCompanyEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_COORDER_COMPANY_EDIT);
	m_wndCompanyEdit.LimitText(TAECHANG_CO_COMPANY_NAME_MAX);
	m_wndList.Create(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, r, this, ID_COORDER_LIST);
	m_wndList.SetAlternateRowColor(TRUE);
	m_wndList.SetFirstColumnAlign(SAGE_LIST_FIRST_COLUMN_CENTER);
	m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_wndList.SetRowSeparator(TRUE);
	m_wndList.InsertColumn(0, TAECHANG_UI_CO_COL_ORDER, LVCFMT_CENTER, TAECHANG_CO_ORDER_COL_WIDTH);
	m_wndList.InsertColumn(1, TAECHANG_UI_CO_COL_COMPANY, LVCFMT_LEFT, TAECHANG_CO_COMPANY_NAME_WIDTH);

	CHeaderCtrl* pHeader = m_wndList.GetHeaderCtrl();
	if (pHeader == NULL)
		return;
	m_wndListHeader.SubclassWindow(pHeader->GetSafeHwnd());
	SetWindowTheme(m_wndListHeader.GetSafeHwnd(), L"", L"");
	HDITEM hdi = {};
	hdi.mask = HDI_FORMAT;
	m_wndListHeader.GetItem(1, &hdi);
	hdi.fmt = (hdi.fmt & ~HDF_JUSTIFYMASK) | HDF_CENTER;
	m_wndListHeader.SetItem(1, &hdi);
}

void SageCompanyOrderPanel::ApplyControlFonts() {
	m_wndCrudSection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndListSection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndAddBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndModifyBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndDeleteBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCancelBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSearchEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSearchBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndOrderEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCompanyEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndList.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
	if (::IsWindow(m_wndListHeader.GetSafeHwnd()))
		m_wndListHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
}

void SageCompanyOrderPanel::ApplyLabelRoles() {
	m_wndSearchLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndSearchLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndOrderLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndOrderLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndOrderLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndNameLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndNameLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndNameLabel.SetFontRole(SAGE_FONT_CONTENT);
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

	int nPad = TAECHANG_MARGIN;
	m_wndCrudSection.MoveWindow(0, 0, TAECHANG_CO_LIST_WIDTH, TAECHANG_SECTION_TITLE_HEIGHT);

	int nCardTop = TAECHANG_SECTION_TITLE_HEIGHT + TAECHANG_ROW_GAP;
	int nCardContentLeft = nPad;
	int nCardContentRight = TAECHANG_CO_LIST_WIDTH - nPad;

	int nInputTop = nCardTop + nPad;
	int nX = nCardContentLeft;
	m_wndOrderLabel.MoveWindow(nX, nInputTop + TAECHANG_LABEL_VERT_OFFSET, TAECHANG_CO_ORDER_LABEL_W, TAECHANG_EDIT_HEIGHT);
	nX += TAECHANG_CO_ORDER_LABEL_W + TAECHANG_LABEL_EDIT_GAP;
	m_wndOrderEdit.MoveWindow(nX, nInputTop, TAECHANG_CO_ORDER_EDIT_WIDTH, TAECHANG_EDIT_HEIGHT);
	ApplyEditTextRect(m_wndOrderEdit, TAECHANG_CO_ORDER_TEXT_LEFT_PAD);

	nX += TAECHANG_CO_ORDER_EDIT_WIDTH + TAECHANG_ACTION_GAP;
	m_wndNameLabel.MoveWindow(nX, nInputTop + TAECHANG_LABEL_VERT_OFFSET, TAECHANG_CO_NAME_LABEL_W, TAECHANG_EDIT_HEIGHT);
	nX += TAECHANG_CO_NAME_LABEL_W + TAECHANG_LABEL_EDIT_GAP;
	int nCompanyEditWidth = nCardContentRight - nX;
	if (nCompanyEditWidth < TAECHANG_CO_COMPANY_EDIT_MIN_WIDTH)
		nCompanyEditWidth = TAECHANG_CO_COMPANY_EDIT_MIN_WIDTH;
	m_wndCompanyEdit.MoveWindow(nX, nInputTop, nCompanyEditWidth, TAECHANG_EDIT_HEIGHT);
	ApplyEditTextRect(m_wndCompanyEdit, TAECHANG_CO_NAME_TEXT_LEFT_PAD);

	int nBtnTop = nInputTop + TAECHANG_EDIT_HEIGHT + TAECHANG_ROW_GAP;
	nX = nCardContentLeft + (nCardContentRight - nCardContentLeft - TAECHANG_CO_BTN_GROUP_WIDTH) / 2;
	m_wndAddBtn.MoveWindow(nX, nBtnTop, TAECHANG_CO_SMALL_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nX += TAECHANG_CO_SMALL_BTN_WIDTH + TAECHANG_ACTION_GAP;
	m_wndModifyBtn.MoveWindow(nX, nBtnTop, TAECHANG_CO_SMALL_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nX += TAECHANG_CO_SMALL_BTN_WIDTH + TAECHANG_ACTION_GAP;
	m_wndCancelBtn.MoveWindow(nX, nBtnTop, TAECHANG_CO_SMALL_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nX += TAECHANG_CO_SMALL_BTN_WIDTH + TAECHANG_ACTION_GAP;
	m_wndDeleteBtn.MoveWindow(nX, nBtnTop, TAECHANG_CO_SMALL_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);

	int nCardHeight = nPad + TAECHANG_EDIT_HEIGHT + TAECHANG_ROW_GAP + TAECHANG_BUTTON_HEIGHT + nPad;
	SetCardRect(CRect(0, nCardTop, TAECHANG_CO_LIST_WIDTH, nCardTop + nCardHeight));

	int nListSectionTop = nCardTop + nCardHeight + TAECHANG_PANEL_GAP;
	int nListWidth = TAECHANG_CO_LIST_WIDTH - TAECHANG_MARGIN;
	int nSearchTop = nListSectionTop + TAECHANG_RESULT_HEADER_HEIGHT + TAECHANG_ROW_GAP;

	int nSearchBtnLeft = nListWidth - TAECHANG_RESULT_SEARCH_WIDTH;
	int nSearchEditLeft = nSearchBtnLeft - TAECHANG_ACTION_GAP - TAECHANG_RESULT_FILTER_WIDTH;
	int nSearchLabelLeft = nSearchEditLeft - TAECHANG_LABEL_EDIT_GAP - TAECHANG_CO_SEARCH_LABEL_W;
	int nSectionLabelWidth = nSearchLabelLeft - TAECHANG_ACTION_GAP;
	if (nSectionLabelWidth < 0)
		nSectionLabelWidth = 0;

	m_wndListSection.MoveWindow(0, nListSectionTop, nSectionLabelWidth, TAECHANG_RESULT_HEADER_HEIGHT);
	m_wndSearchLabel.MoveWindow(nSearchLabelLeft, nSearchTop + TAECHANG_LABEL_VERT_OFFSET, TAECHANG_CO_SEARCH_LABEL_W, TAECHANG_EDIT_HEIGHT);
	m_wndSearchEdit.MoveWindow(nSearchEditLeft, nSearchTop, TAECHANG_RESULT_FILTER_WIDTH, TAECHANG_EDIT_HEIGHT);
	ApplyEditTextRect(m_wndSearchEdit, TAECHANG_CO_NAME_TEXT_LEFT_PAD);
	m_wndSearchBtn.MoveWindow(nSearchBtnLeft, nSearchTop, TAECHANG_RESULT_SEARCH_WIDTH, TAECHANG_BUTTON_HEIGHT);

	int nListTop = nSearchTop + TAECHANG_EDIT_HEIGHT + TAECHANG_ROW_GAP;
	int nListHeight = rectClient.Height() - nListTop;
	if (nListHeight < TAECHANG_RESULT_MIN_HEIGHT)
		nListHeight = TAECHANG_RESULT_MIN_HEIGHT;
	m_wndList.MoveWindow(0, nListTop, nListWidth, nListHeight);
	UpdateListColumns();
}

void SageCompanyOrderPanel::SetCardRect(const CRect& rectNew) {
	if (m_rectCard == rectNew)
		return;

	CRect rectStale;
	rectStale.UnionRect(m_rectCard, rectNew);
	m_rectCard = rectNew;
	if (rectStale.IsRectEmpty())
		return;

	rectStale.InflateRect(TAECHANG_CARD_REPAINT_MARGIN, TAECHANG_CARD_REPAINT_MARGIN);
	InvalidateRect(rectStale, TRUE);
}

BOOL SageCompanyOrderPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);

	if (!m_rectCard.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectCard, TAECHANG_COLOR_PANEL);
		CBrush brushCard(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectCard, &brushCard);
	}
	DrawEditBorder(pDC, m_wndSearchEdit);
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
	BOOL bEditing = (m_nPanelState == TAECHANG_CO_PANEL_MODIFY) ? TRUE : FALSE;
	BOOL bHasSelection = (m_nSelectedOrderId > 0) ? TRUE : FALSE;

	m_wndAddBtn.EnableWindow(!bEditing ? TRUE : FALSE);
	m_wndModifyBtn.EnableWindow((bEditing || bHasSelection) ? TRUE : FALSE);
	m_wndModifyBtn.SetWindowTextW(bEditing ? TAECHANG_UI_CO_SAVE_BTN : TAECHANG_UI_CO_MODIFY_BTN);
	m_wndDeleteBtn.EnableWindow((!bEditing && bHasSelection) ? TRUE : FALSE);
	m_wndCancelBtn.EnableWindow(bEditing ? TRUE : FALSE);
	m_wndSearchEdit.EnableWindow(!bEditing ? TRUE : FALSE);
	m_wndSearchBtn.EnableWindow(!bEditing ? TRUE : FALSE);
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

	int nSortOrder = TAECHANG_CO_ORDER_FIRST;
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

	m_wndOrderEdit.SetWindowTextW(CString());
	m_wndCompanyEdit.SetWindowTextW(CString());
	m_nSelectedOrderId = nNewOrderId;
	m_wndList.SetItemState(TAECHANG_LIST_NO_ITEM, 0, LVIS_SELECTED);
	RefreshList();
}

void SageCompanyOrderPanel::OnModify() {
	if (m_nPanelState == TAECHANG_CO_PANEL_IDLE) {
		if (m_nSelectedOrderId <= 0) {
			AfxMessageBox(TAECHANG_UI_CO_SELECT_REQUIRED);
			return;
		}
		m_nPanelState = TAECHANG_CO_PANEL_MODIFY;
		UpdatePanelState();
		m_wndCompanyEdit.SetFocus();
		Invalidate(FALSE);
		return;
	}

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
	m_nPanelState = TAECHANG_CO_PANEL_IDLE;
	m_nSelectedOrderId = 0;
	m_wndList.SetItemState(TAECHANG_LIST_NO_ITEM, 0, LVIS_SELECTED);
	m_wndOrderEdit.SetWindowTextW(CString());
	m_wndCompanyEdit.SetWindowTextW(CString());
	UpdatePanelState();
	Invalidate(FALSE);
}

void SageCompanyOrderPanel::OnSearch() {
	m_wndSearchEdit.GetWindowTextW(m_strSearchKeyword);
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

	TaechangReceivableCompanyOrderDto dto;
	if (m_nSelectedOrderId > 0 && FindSelectedDto(dto)) {
		CString strOrder;
		strOrder.Format(TAECHANG_UI_CO_ORDER_FORMAT, dto.nSortOrder);
		m_wndOrderEdit.SetWindowTextW(strOrder);
		m_wndCompanyEdit.SetWindowTextW(dto.strCompanyName);
	} else {
		m_wndOrderEdit.SetWindowTextW(CString());
		m_wndCompanyEdit.SetWindowTextW(CString());
	}
	Invalidate(FALSE);
	UpdatePanelState();
}
