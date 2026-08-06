#include "pch.h"
#include "app/ui/drawing/SageSelectionBar.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageSelectionBar, CStatic)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(ID_TAECHANG_SELECTION_CHECK, &CSageSelectionBar::OnSelectAllClicked)
	ON_BN_CLICKED(ID_TAECHANG_SELECTION_CLEAR, &CSageSelectionBar::OnClearClicked)
END_MESSAGE_MAP()

CSageSelectionBar::CSageSelectionBar()
	: m_nSelectAllCommandId(0)
	, m_nClearCommandId(0)
	, m_nTotalCount(0)
	, m_nSelectedCount(0) {
}

int CSageSelectionBar::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectEmpty(0, 0, 0, 0);
	m_wndSelectAll.Create(TAECHANG_UI_SELECT_ALL_BUTTON, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		rectEmpty, this, ID_TAECHANG_SELECTION_CHECK);
	m_wndSelectAll.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));

	m_wndClearBtn.Create(TAECHANG_UI_SELECTION_CLEAR_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rectEmpty, this, ID_TAECHANG_SELECTION_CLEAR);
	m_wndClearBtn.SetVariant(SAGE_BUTTON_GHOST);
	m_wndClearBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	return 0;
}

void CSageSelectionBar::SetCommands(UINT nSelectAllCommandId, UINT nClearCommandId) {
	m_nSelectAllCommandId = nSelectAllCommandId;
	m_nClearCommandId = nClearCommandId;
}

void CSageSelectionBar::SetCounts(int nTotalCount, int nSelectedCount) {
	m_nTotalCount = nTotalCount;
	m_nSelectedCount = nSelectedCount;
	if (!::IsWindow(GetSafeHwnd()))
		return;
	LayoutChildren();
	Invalidate();
}

void CSageSelectionBar::SetAllChecked(BOOL bChecked) {
	if (::IsWindow(m_wndSelectAll.GetSafeHwnd()))
		m_wndSelectAll.SetCheck(bChecked ? BST_CHECKED : BST_UNCHECKED);
}

void CSageSelectionBar::EnableControls(BOOL bEnable) {
	if (::IsWindow(m_wndSelectAll.GetSafeHwnd()))
		m_wndSelectAll.EnableWindow(bEnable);
	if (::IsWindow(m_wndClearBtn.GetSafeHwnd()))
		m_wndClearBtn.EnableWindow(bEnable);
}

void CSageSelectionBar::OnSelectAllClicked() {
	if (m_nSelectAllCommandId == 0)
		return;
	GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(m_nSelectAllCommandId, BN_CLICKED), 0);
}

void CSageSelectionBar::OnClearClicked() {
	if (m_nClearCommandId == 0)
		return;
	GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(m_nClearCommandId, BN_CLICKED), 0);
}

HBRUSH CSageSelectionBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	CStatic::OnCtlColor(pDC, pWnd, nCtlColor);
	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
	pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
	return SageUiResources::GetBrush(SAGE_BG_APP);
}

void CSageSelectionBar::OnSize(UINT nType, int cx, int cy) {
	CStatic::OnSize(nType, cx, cy);
	LayoutChildren();
}

int CSageSelectionBar::MeasureTextWidth(CDC* pDC, const CString& strText, SageFontRole nRole) const {
	if (strText.IsEmpty())
		return 0;
	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(nRole));
	CSize size = pDC->GetTextExtent(strText);
	pDC->SelectObject(pOldFont);
	return size.cx;
}

int CSageSelectionBar::GetCheckWidth(CDC* pDC) const {
	return TAECHANG_SELECTION_CHECK_GLYPH_WIDTH
		+ MeasureTextWidth(pDC, TAECHANG_UI_SELECT_ALL_BUTTON, SAGE_FONT_CONTENT);
}

int CSageSelectionBar::GetCountWidth(CDC* pDC) const {
	CString strTotal;
	strTotal.Format(TAECHANG_UI_SELECTION_TOTAL_FORMAT, m_nTotalCount);
	CString strSelected;
	strSelected.Format(TAECHANG_UI_SELECTION_SELECTED_FORMAT, m_nSelectedCount);

	return MeasureTextWidth(pDC, strTotal, SAGE_FONT_CONTENT)
		+ TAECHANG_ICON_TEXT_GAP + MeasureTextWidth(pDC, strSelected, SAGE_FONT_CONTENT_SEMIBOLD)
		+ TAECHANG_ICON_TEXT_GAP + MeasureTextWidth(pDC, TAECHANG_UI_SELECTION_SUFFIX, SAGE_FONT_CONTENT);
}

int CSageSelectionBar::GetClearWidth(CDC* pDC) const {
	return MeasureTextWidth(pDC, TAECHANG_UI_SELECTION_CLEAR_BUTTON, SAGE_FONT_CONTENT)
		+ TAECHANG_SELECTION_CLEAR_PAD * 2;
}

int CSageSelectionBar::GetContentWidth() const {
	if (!::IsWindow(GetSafeHwnd()))
		return 0;
	CClientDC dc(const_cast<CSageSelectionBar*>(this));
	return GetCheckWidth(&dc) + TAECHANG_SELECTION_BAR_GAP + GetCountWidth(&dc)
		+ TAECHANG_SELECTION_BAR_GAP + GetClearWidth(&dc);
}

void CSageSelectionBar::LayoutChildren() {
	if (!::IsWindow(m_wndSelectAll.GetSafeHwnd()) || !::IsWindow(m_wndClearBtn.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	if (rectClient.Width() <= 0)
		return;

	CClientDC dc(this);
	int nCheckWidth = GetCheckWidth(&dc);
	m_wndSelectAll.MoveWindow(0, 0, nCheckWidth, rectClient.Height());

	int nClearLeft = nCheckWidth + TAECHANG_SELECTION_BAR_GAP + GetCountWidth(&dc) + TAECHANG_SELECTION_BAR_GAP;
	m_wndClearBtn.MoveWindow(nClearLeft, 0, GetClearWidth(&dc), rectClient.Height());
}

int CSageSelectionBar::DrawTextSegment(
	CDC* pDC, int nLeft, const CRect& rectClient, const CString& strText, SageFontRole nRole, COLORREF color) {
	if (strText.IsEmpty())
		return nLeft;

	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(nRole));
	pDC->SetTextColor(color);
	CSize size = pDC->GetTextExtent(strText);
	CRect rectText(nLeft, rectClient.top, nLeft + size.cx, rectClient.bottom);
	pDC->DrawText(strText, &rectText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pDC->SelectObject(pOldFont);
	return nLeft + size.cx;
}

void CSageSelectionBar::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);

	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
	pDC->SetBkMode(TRANSPARENT);

	CString strTotal;
	strTotal.Format(TAECHANG_UI_SELECTION_TOTAL_FORMAT, m_nTotalCount);
	CString strSelected;
	strSelected.Format(TAECHANG_UI_SELECTION_SELECTED_FORMAT, m_nSelectedCount);

	int nLeft = GetCheckWidth(pDC) + TAECHANG_SELECTION_BAR_GAP;
	nLeft = DrawTextSegment(pDC, nLeft, rectClient, strTotal, SAGE_FONT_CONTENT, TAECHANG_COLOR_SECONDARY_TEXT);
	nLeft += TAECHANG_ICON_TEXT_GAP;
	nLeft = DrawTextSegment(pDC, nLeft, rectClient, strSelected, SAGE_FONT_CONTENT_SEMIBOLD, TAECHANG_COLOR_PRIMARY);
	nLeft += TAECHANG_ICON_TEXT_GAP;
	DrawTextSegment(pDC, nLeft, rectClient, TAECHANG_UI_SELECTION_SUFFIX, SAGE_FONT_CONTENT, TAECHANG_COLOR_SECONDARY_TEXT);
}
