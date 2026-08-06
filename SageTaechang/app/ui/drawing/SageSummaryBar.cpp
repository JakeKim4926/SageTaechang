#include "pch.h"
#include "app/ui/drawing/SageSummaryBar.h"
#include "TaechangDefine.h"

void CSageSummaryBar::SetItems(const std::vector<SageSummaryBarItem>& arrItems) {
	m_arrItems = arrItems;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

BOOL CSageSummaryBar::HasItems() const {
	return m_arrItems.empty() ? FALSE : TRUE;
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
	int nWidth = MeasureTextWidth(pDC, item.strLabel, SAGE_FONT_CAPTION);
	if (nWidth > 0)
		nWidth += TAECHANG_SUMMARY_TEXT_GAP;
	nWidth += MeasureTextWidth(pDC, item.strValue, SAGE_FONT_SUMMARY);
	int nUnitWidth = MeasureTextWidth(pDC, item.strUnit, SAGE_FONT_CAPTION);
	if (nUnitWidth > 0)
		nWidth += TAECHANG_SUMMARY_TEXT_GAP + nUnitWidth;
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
	int nTop = rectClient.top + (rectClient.Height() - TAECHANG_SUMMARY_DIVIDER_HEIGHT) / 2;
	pDC->FillSolidRect(nLeft, nTop, TAECHANG_BORDER_THICKNESS, TAECHANG_SUMMARY_DIVIDER_HEIGHT, TAECHANG_COLOR_BORDER);
}

void CSageSummaryBar::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);

	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);

	pDC->SetBkMode(TRANSPARENT);
	int nLeft = rectClient.left;
	for (int i = 0; i < static_cast<int>(m_arrItems.size()); ++i) {
		if (i > 0) {
			nLeft += TAECHANG_SUMMARY_ITEM_GAP;
			DrawDivider(pDC, nLeft, rectClient);
			nLeft += TAECHANG_BORDER_THICKNESS + TAECHANG_SUMMARY_ITEM_GAP;
		}

		const SageSummaryBarItem& item = m_arrItems[i];
		if (nLeft + MeasureItemWidth(pDC, item) > rectClient.right)
			return;

		nLeft = DrawTextSegment(pDC, nLeft, rectClient, item.strLabel, SAGE_FONT_CAPTION, TAECHANG_COLOR_SECONDARY_TEXT);
		if (!item.strLabel.IsEmpty())
			nLeft += TAECHANG_SUMMARY_TEXT_GAP;

		COLORREF colorValue = item.bHighlight ? TAECHANG_COLOR_PRIMARY : TAECHANG_COLOR_TEXT;
		nLeft = DrawTextSegment(pDC, nLeft, rectClient, item.strValue, SAGE_FONT_SUMMARY, colorValue);
		if (item.strUnit.IsEmpty())
			continue;

		nLeft += TAECHANG_SUMMARY_TEXT_GAP;
		nLeft = DrawTextSegment(pDC, nLeft, rectClient, item.strUnit, SAGE_FONT_CAPTION, TAECHANG_COLOR_SECONDARY_TEXT);
	}
}
