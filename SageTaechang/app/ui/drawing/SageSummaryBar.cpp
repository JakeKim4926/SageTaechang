#include "pch.h"
#include "app/ui/drawing/SageSummaryBar.h"
#include "SageDefine.h"

BEGIN_MESSAGE_MAP(CSageSummaryBar, CStatic)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CSageSummaryBar::SetItems(const std::vector<SageSummaryBarItem>& arrItems) {
	m_arrItems = arrItems;
	if (!::IsWindow(GetSafeHwnd()))
		return;

	ApplyBadgeItem();
	Invalidate();
}

BOOL CSageSummaryBar::HasItems() const {
	return m_arrItems.empty() ? FALSE : TRUE;
}

int CSageSummaryBar::FindBadgeItemIndex() const {
	for (int i = 0; i < static_cast<int>(m_arrItems.size()); ++i) {
		if (m_arrItems[i].badge.IsVisible())
			return i;
	}
	return SAGE_SUMMARY_NO_BADGE_ITEM;
}

CString CSageSummaryBar::BuildBadgeText(const SageSummaryBarItem& item) const {
	CString strText;
	strText.Format(SAGE_UI_SUMMARY_BADGE_FORMAT,
		static_cast<LPCWSTR>(item.strLabel),
		static_cast<LPCWSTR>(item.strValue),
		static_cast<LPCWSTR>(item.strUnit));
	return strText;
}

void CSageSummaryBar::ApplyBadgeItem() {
	int nIndex = FindBadgeItemIndex();
	if (nIndex == SAGE_SUMMARY_NO_BADGE_ITEM) {
		if (::IsWindow(m_wndBadge.GetSafeHwnd()))
			m_wndBadge.ShowWindow(SW_HIDE);
		return;
	}

	if (!::IsWindow(m_wndBadge.GetSafeHwnd())) {
		ModifyStyle(0, WS_CLIPCHILDREN);
		m_wndBadge.Create(L"", WS_CHILD | SS_OWNERDRAW, CRect(0, 0, 0, 0), this, ID_SAGE_SUMMARY_BADGE);
		m_wndBadge.SetCornerRadius(SAGE_BADGE_RADIUS);
		m_wndBadge.SetSurfaceColor(SAGE_COLOR_APP_BACKGROUND);
	}

	const SageSummaryBarItem& item = m_arrItems[nIndex];
	m_wndBadge.SetBadge(BuildBadgeText(item),
		item.badge.clrBackground, item.badge.clrBorder, item.badge.clrText);
	m_wndBadge.ShowWindow(SW_SHOW);
	LayoutBadge();
}

void CSageSummaryBar::LayoutBadge() {
	int nIndex = FindBadgeItemIndex();
	if (nIndex == SAGE_SUMMARY_NO_BADGE_ITEM || !::IsWindow(m_wndBadge.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);

	CClientDC dc(this);
	int nLeft = rectClient.left;
	for (int i = 0; i < nIndex; ++i) {
		if (i > 0)
			nLeft += SAGE_SUMMARY_ITEM_GAP + SAGE_BORDER_THICKNESS + SAGE_SUMMARY_ITEM_GAP;
		nLeft += MeasureItemWidth(&dc, m_arrItems[i]);
	}
	if (nIndex > 0)
		nLeft += SAGE_SUMMARY_ITEM_GAP + SAGE_BORDER_THICKNESS + SAGE_SUMMARY_ITEM_GAP;

	int nWidth = MeasureItemWidth(&dc, m_arrItems[nIndex]);
	m_wndBadge.MoveWindow(nLeft, rectClient.top, nWidth, rectClient.Height());
}

void CSageSummaryBar::OnSize(UINT nType, int cx, int cy) {
	CStatic::OnSize(nType, cx, cy);
	LayoutBadge();
}

int CSageSummaryBar::MeasureTextWidth(CDC* pDC, const CString& strText, SageFontRole nRole) const {
	if (strText.IsEmpty())
		return 0;
	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(nRole));
	CSize size = pDC->GetTextExtent(strText);
	pDC->SelectObject(pOldFont);
	return size.cx;
}

int CSageSummaryBar::MeasureItemWidth(CDC* pDC, const SageSummaryBarItem& item) const {
	if (item.badge.IsVisible())
		return MeasureTextWidth(pDC, BuildBadgeText(item), SAGE_FONT_CAPTION) + SAGE_BADGE_PAD_X * 2;

	int nWidth = MeasureTextWidth(pDC, item.strLabel, SAGE_FONT_CAPTION);
	if (nWidth > 0)
		nWidth += SAGE_SUMMARY_TEXT_GAP;
	nWidth += MeasureTextWidth(pDC, item.strValue, SAGE_FONT_SUMMARY);
	int nUnitWidth = MeasureTextWidth(pDC, item.strUnit, SAGE_FONT_CAPTION);
	if (nUnitWidth > 0)
		nWidth += SAGE_SUMMARY_TEXT_GAP + nUnitWidth;
	return nWidth;
}

int CSageSummaryBar::DrawTextSegment(CDC* pDC, int nLeft, const CRect& rectItem, const CString& strText, SageFontRole nRole, COLORREF color) {
	if (strText.IsEmpty())
		return nLeft;

	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(nRole));
	pDC->SetTextColor(color);
	CSize size = pDC->GetTextExtent(strText);
	CRect rectText(nLeft, rectItem.top, nLeft + size.cx, rectItem.bottom);
	pDC->DrawText(strText, &rectText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pDC->SelectObject(pOldFont);
	return nLeft + size.cx;
}

void CSageSummaryBar::DrawDivider(CDC* pDC, int nLeft, const CRect& rectClient) {
	int nTop = rectClient.top + (rectClient.Height() - SAGE_SUMMARY_DIVIDER_HEIGHT) / 2;
	pDC->FillSolidRect(nLeft, nTop, SAGE_BORDER_THICKNESS, SAGE_SUMMARY_DIVIDER_HEIGHT, SAGE_COLOR_BORDER);
}

void CSageSummaryBar::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);

	pDC->FillSolidRect(rectClient, SAGE_COLOR_APP_BACKGROUND);

	pDC->SetBkMode(TRANSPARENT);
	int nLeft = rectClient.left;
	for (int i = 0; i < static_cast<int>(m_arrItems.size()); ++i) {
		if (i > 0) {
			nLeft += SAGE_SUMMARY_ITEM_GAP;
			DrawDivider(pDC, nLeft, rectClient);
			nLeft += SAGE_BORDER_THICKNESS + SAGE_SUMMARY_ITEM_GAP;
		}

		const SageSummaryBarItem& item = m_arrItems[i];
		int nItemWidth = MeasureItemWidth(pDC, item);
		if (nLeft + nItemWidth > rectClient.right)
			return;

		if (item.badge.IsVisible()) {
			nLeft += nItemWidth;
			continue;
		}

		nLeft = DrawTextSegment(pDC, nLeft, rectClient, item.strLabel, SAGE_FONT_CAPTION, SAGE_COLOR_SECONDARY_TEXT);
		if (!item.strLabel.IsEmpty())
			nLeft += SAGE_SUMMARY_TEXT_GAP;

		COLORREF colorValue = item.bHighlight ? SAGE_COLOR_PRIMARY : SAGE_COLOR_TEXT;
		nLeft = DrawTextSegment(pDC, nLeft, rectClient, item.strValue, SAGE_FONT_SUMMARY, colorValue);
		if (item.strUnit.IsEmpty())
			continue;

		nLeft += SAGE_SUMMARY_TEXT_GAP;
		nLeft = DrawTextSegment(pDC, nLeft, rectClient, item.strUnit, SAGE_FONT_CAPTION, SAGE_COLOR_SECONDARY_TEXT);
	}
}
