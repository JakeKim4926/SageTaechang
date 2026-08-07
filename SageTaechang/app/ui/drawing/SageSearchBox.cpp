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
	: m_nCommandId(0) {
}

BOOL CSageSearchBox::CreateBox(CWnd* pParent, UINT nBoxId, UINT nEditId) {
	CRect rectEmpty(0, 0, 0, 0);
	if (!CStatic::Create(L"", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW | SS_NOTIFY, rectEmpty, pParent, nBoxId))
		return FALSE;

	if (!m_wndEdit.Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, rectEmpty, this, nEditId))
		return FALSE;

	m_wndEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	return TRUE;
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

BOOL CSageSearchBox::IsEditMessage(const MSG* pMsg) const {
	if (pMsg == NULL)
		return FALSE;
	return (pMsg->hwnd == m_wndEdit.GetSafeHwnd()) ? TRUE : FALSE;
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
	int nEditLeft = rectClient.left + TAECHANG_EDIT_BORDER_WIDTH + TAECHANG_EDIT_TEXT_LEFT_PAD;
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

	CRect rectCell = GetIconCellRect(rectClient);
	pDC->FillSolidRect(rectCell, TAECHANG_COLOR_APP_BACKGROUND);
	pDC->FillSolidRect(
		rectCell.left, rectCell.top, TAECHANG_EDIT_BORDER_WIDTH, rectCell.Height(),
		TAECHANG_COLOR_LIST_HEADER_BORDER);
	SageUiStyle::DrawSearchIcon(*pDC, rectCell.CenterPoint(), TAECHANG_COLOR_TEXT_MUTED);

	CBrush brushBorder(TAECHANG_COLOR_BUTTON_BORDER);
	pDC->FrameRect(rectClient, &brushBorder);
}
