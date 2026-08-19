#include "pch.h"
#include "app/ui/drawing/SageSearchBox.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/ui/drawing/SageUiStyle.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageSearchBox, CStatic)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

CSageSearchBox::CSageSearchBox()
	: m_nCommandId(0), m_nEditId(0), m_nCriteriaId(0) {
}

BOOL CSageSearchBox::CreateBox(CWnd* pParent, UINT nBoxId, UINT nEditId) {
	CRect rectEmpty(0, 0, 0, 0);
	if (!CStatic::Create(L"", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW | SS_NOTIFY, rectEmpty, pParent, nBoxId))
		return FALSE;

	if (!m_wndEdit.Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, rectEmpty, this, nEditId))
		return FALSE;

	m_nEditId = nEditId;

	m_wndEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	return TRUE;
}

BOOL CSageSearchBox::CreateCriteriaCell(UINT nCriteriaId, int nDropRows) {
	CRect rectEmpty(0, 0, 0, 0);
	m_wndCriteria.SetFieldHeight(TAECHANG_EDIT_HEIGHT
		- TAECHANG_EDIT_BORDER_WIDTH * 2 - TAECHANG_COMBO_FIELD_INSET);
	if (!m_wndCriteria.Create(
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS | WS_VSCROLL,
		rectEmpty, this, nCriteriaId))
		return FALSE;

	m_nCriteriaId = nCriteriaId;
	m_wndCriteria.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCriteria.SetFieldColor(TAECHANG_COLOR_APP_BACKGROUND);
	m_wndCriteria.MoveWindow(0, 0, TAECHANG_SEARCH_CRITERIA_CELL_WIDTH, TAECHANG_EDIT_HEIGHT * nDropRows);
	LayoutCriteria();
	LayoutEdit();
	return TRUE;
}

void CSageSearchBox::ClearCriteriaItems() {
	if (::IsWindow(m_wndCriteria.GetSafeHwnd()))
		m_wndCriteria.ResetContent();
}

void CSageSearchBox::AddCriteriaItem(LPCWSTR pszLabel, int nItemData) {
	if (!::IsWindow(m_wndCriteria.GetSafeHwnd()))
		return;
	int nIndex = m_wndCriteria.AddString(pszLabel);
	if (nIndex >= 0)
		m_wndCriteria.SetItemData(nIndex, nItemData);
}

int CSageSearchBox::FindCriteriaIndex(int nItemData) const {
	if (!::IsWindow(m_wndCriteria.GetSafeHwnd()))
		return CB_ERR;
	int nCount = m_wndCriteria.GetCount();
	for (int nIndex = 0; nIndex < nCount; ++nIndex) {
		if (static_cast<int>(m_wndCriteria.GetItemData(nIndex)) == nItemData)
			return nIndex;
	}
	return CB_ERR;
}

void CSageSearchBox::SetCriteriaIndex(int nIndex) {
	if (::IsWindow(m_wndCriteria.GetSafeHwnd()))
		m_wndCriteria.SetCurSel(nIndex);
}

int CSageSearchBox::GetSelectedCriteria() const {
	if (!::IsWindow(m_wndCriteria.GetSafeHwnd()))
		return CB_ERR;
	int nSel = m_wndCriteria.GetCurSel();
	if (nSel == CB_ERR)
		return CB_ERR;
	return static_cast<int>(m_wndCriteria.GetItemData(nSel));
}

BOOL CSageSearchBox::OnCommand(WPARAM wParam, LPARAM lParam) {
	UINT nChildId = LOWORD(wParam);
	if ((m_nCriteriaId != 0 && nChildId == m_nCriteriaId)
		|| (m_nEditId != 0 && nChildId == m_nEditId)) {
		GetParent()->SendMessage(WM_COMMAND, wParam, lParam);
		return TRUE;
	}
	return CStatic::OnCommand(wParam, lParam);
}

void CSageSearchBox::SetCommand(UINT nCommandId) {
	m_nCommandId = nCommandId;
}

void CSageSearchBox::SetPlaceholder(LPCWSTR pszPlaceholder) {
	if (!::IsWindow(m_wndEdit.GetSafeHwnd()))
		return;
	m_wndEdit.SendMessage(EM_SETCUEBANNER, TRUE, reinterpret_cast<LPARAM>(pszPlaceholder));
}

void CSageSearchBox::SetMaxLength(int nMaxLength) {
	if (::IsWindow(m_wndEdit.GetSafeHwnd()))
		m_wndEdit.LimitText(nMaxLength);
}

CString CSageSearchBox::GetKeyword() const {
	CString strKeyword;
	if (::IsWindow(m_wndEdit.GetSafeHwnd()))
		m_wndEdit.GetWindowTextW(strKeyword);
	return strKeyword;
}

void CSageSearchBox::SetKeyword(const CString& strKeyword) {
	if (::IsWindow(m_wndEdit.GetSafeHwnd()))
		m_wndEdit.SetWindowTextW(strKeyword);
}

void CSageSearchBox::SetEditFocus() {
	if (::IsWindow(m_wndEdit.GetSafeHwnd()))
		m_wndEdit.SetFocus();
}

BOOL CSageSearchBox::IsEditMessage(const MSG* pMsg) const {
	if (pMsg == NULL)
		return FALSE;
	return (pMsg->hwnd == m_wndEdit.GetSafeHwnd()) ? TRUE : FALSE;
}

int CSageSearchBox::GetCriteriaCellWidth() const {
	return ::IsWindow(m_wndCriteria.GetSafeHwnd()) ? TAECHANG_SEARCH_CRITERIA_CELL_WIDTH : 0;
}

CRect CSageSearchBox::GetCriteriaCellRect(const CRect& rectClient) const {
	CRect rectCell(rectClient);
	rectCell.right = rectCell.left + GetCriteriaCellWidth();
	rectCell.DeflateRect(0, TAECHANG_EDIT_BORDER_WIDTH);
	rectCell.left += TAECHANG_EDIT_BORDER_WIDTH;
	return rectCell;
}

void CSageSearchBox::LayoutCriteria() {
	if (!::IsWindow(m_wndCriteria.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	CRect rectCell = GetCriteriaCellRect(rectClient);
	m_wndCriteria.SetWindowPos(NULL, rectCell.left, rectCell.top, rectCell.Width(), rectCell.Height(),
		SWP_NOZORDER | SWP_NOACTIVATE);
}

CRect CSageSearchBox::GetIconCellRect(const CRect& rectClient) const {
	CRect rectCell(rectClient);
	rectCell.left = rectCell.right - TAECHANG_SEARCH_ICON_CELL_WIDTH;
	rectCell.DeflateRect(0, TAECHANG_EDIT_BORDER_WIDTH);
	rectCell.right -= TAECHANG_EDIT_BORDER_WIDTH;
	return rectCell;
}

int CSageSearchBox::GetTextLineHeight() {
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	TEXTMETRIC tm = {};
	dc.GetTextMetrics(&tm);
	dc.SelectObject(pOldFont);
	return tm.tmHeight;
}

void CSageSearchBox::LayoutEdit() {
	if (!::IsWindow(m_wndEdit.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	int nEditLeft = rectClient.left + GetCriteriaCellWidth()
		+ TAECHANG_EDIT_BORDER_WIDTH + TAECHANG_EDIT_TEXT_LEFT_PAD;
	int nEditRight = rectClient.right - TAECHANG_SEARCH_ICON_CELL_WIDTH - TAECHANG_EDIT_TEXT_LEFT_PAD;
	if (nEditRight < nEditLeft)
		nEditRight = nEditLeft;

	int nEditHeight = GetTextLineHeight();
	m_wndEdit.MoveWindow(
		nEditLeft,
		rectClient.top + (rectClient.Height() - nEditHeight) / 2,
		nEditRight - nEditLeft,
		nEditHeight);
}

void CSageSearchBox::OnSize(UINT nType, int cx, int cy) {
	CStatic::OnSize(nType, cx, cy);
	LayoutCriteria();
	LayoutEdit();
}

void CSageSearchBox::OnLButtonDown(UINT nFlags, CPoint point) {
	CStatic::OnLButtonDown(nFlags, point);

	CRect rectClient;
	GetClientRect(&rectClient);
	if (!GetIconCellRect(rectClient).PtInRect(point)) {
		m_wndEdit.SetFocus();
		return;
	}
	if (m_nCommandId == 0)
		return;
	GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(m_nCommandId, BN_CLICKED), 0);
}

HBRUSH CSageSearchBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	UNREFERENCED_PARAMETER(pWnd);
	if (nCtlColor == CTLCOLOR_EDIT) {
		pDC->SetTextColor(TAECHANG_COLOR_TEXT);
		pDC->SetBkColor(TAECHANG_COLOR_PANEL);
		return SageUiResources::GetBrush(SAGE_BG_PANEL);
	}
	return CStatic::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CSageSearchBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);

	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_PANEL);

	if (GetCriteriaCellWidth() > 0) {
		CRect rectCriteria = GetCriteriaCellRect(rectClient);
		pDC->FillSolidRect(rectCriteria, TAECHANG_COLOR_APP_BACKGROUND);
		pDC->FillSolidRect(
			rectCriteria.right, rectCriteria.top, TAECHANG_EDIT_BORDER_WIDTH, rectCriteria.Height(),
			TAECHANG_COLOR_LIST_HEADER_BORDER);
	}

	CRect rectCell = GetIconCellRect(rectClient);
	pDC->FillSolidRect(rectCell, TAECHANG_COLOR_APP_BACKGROUND);
	pDC->FillSolidRect(
		rectCell.left, rectCell.top, TAECHANG_EDIT_BORDER_WIDTH, rectCell.Height(),
		TAECHANG_COLOR_LIST_HEADER_BORDER);
	SageUiStyle::DrawSearchIcon(*pDC, rectCell.CenterPoint(), TAECHANG_COLOR_TEXT_MUTED);

	CBrush brushBorder(TAECHANG_COLOR_BUTTON_BORDER);
	pDC->FrameRect(rectClient, &brushBorder);
}
