#include "pch.h"
#include "app/ui/drawing/SageFilterPillBar.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageFilterPillBar, CStatic)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

CSageFilterPillBar::CSageFilterPillBar()
	: m_nSelectedIndex(0)
	, m_nCommandId(0) {
}

void CSageFilterPillBar::SetCommand(UINT nCommandId) {
	m_nCommandId = nCommandId;
}

void CSageFilterPillBar::SetLabels(const std::vector<CString>& arrLabels) {
	m_arrLabels = arrLabels;
	if (m_nSelectedIndex >= static_cast<int>(m_arrLabels.size()))
		m_nSelectedIndex = 0;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageFilterPillBar::SetSelectedIndex(int nIndex) {
	if (nIndex < 0 || nIndex >= static_cast<int>(m_arrLabels.size()))
		return;
	m_nSelectedIndex = nIndex;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

int CSageFilterPillBar::GetSelectedIndex() const {
	return m_nSelectedIndex;
}

void CSageFilterPillBar::BuildPillRects(CDC* pDC, const CRect& rectClient, std::vector<CRect>& outRects) const {
	outRects.clear();
	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CAPTION));

	int nLeft = rectClient.left;
	int nTop = rectClient.top + (rectClient.Height() - TAECHANG_PILL_HEIGHT) / 2;
	for (int i = 0; i < static_cast<int>(m_arrLabels.size()); ++i) {
		CSize sizeText = pDC->GetTextExtent(m_arrLabels[i]);
		int nWidth = sizeText.cx + TAECHANG_PILL_PAD_X * 2;
		outRects.push_back(CRect(nLeft, nTop, nLeft + nWidth, nTop + TAECHANG_PILL_HEIGHT));
		nLeft += nWidth + TAECHANG_PILL_GAP;
	}

	pDC->SelectObject(pOldFont);
}

int CSageFilterPillBar::FindPillAt(CPoint point) const {
	if (!::IsWindow(GetSafeHwnd()))
		return TAECHANG_LIST_NO_ITEM;

	CRect rectClient;
	GetClientRect(&rectClient);
	CClientDC dc(const_cast<CSageFilterPillBar*>(this));

	std::vector<CRect> arrRects;
	BuildPillRects(&dc, rectClient, arrRects);
	for (int i = 0; i < static_cast<int>(arrRects.size()); ++i) {
		if (arrRects[i].PtInRect(point))
			return i;
	}
	return TAECHANG_LIST_NO_ITEM;
}

void CSageFilterPillBar::OnLButtonDown(UINT nFlags, CPoint point) {
	CStatic::OnLButtonDown(nFlags, point);

	int nIndex = FindPillAt(point);
	if (nIndex == TAECHANG_LIST_NO_ITEM || nIndex == m_nSelectedIndex)
		return;

	m_nSelectedIndex = nIndex;
	Invalidate();
	if (m_nCommandId == 0)
		return;
	GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(m_nCommandId, BN_CLICKED), 0);
}

void CSageFilterPillBar::DrawPill(CDC* pDC, const CRect& rectPill, const CString& strLabel, BOOL bSelected) {
	COLORREF clrFill = bSelected ? TAECHANG_COLOR_ACCENT_SURFACE : TAECHANG_COLOR_PANEL;
	COLORREF clrBorder = bSelected ? TAECHANG_COLOR_PRIMARY : TAECHANG_COLOR_BORDER;
	COLORREF clrText = bSelected ? TAECHANG_COLOR_PRIMARY : TAECHANG_COLOR_TEXT_MUTED;

	CBrush brushFill(clrFill);
	CPen penBorder(PS_SOLID, TAECHANG_BORDER_THICKNESS, clrBorder);
	CBrush* pOldBrush = pDC->SelectObject(&brushFill);
	CPen* pOldPen = pDC->SelectObject(&penBorder);
	pDC->RoundRect(rectPill, CPoint(TAECHANG_PILL_RADIUS * 2, TAECHANG_PILL_RADIUS * 2));
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	pDC->SetTextColor(clrText);
	pDC->DrawText(strLabel, const_cast<CRect*>(&rectPill), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CSageFilterPillBar::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);
	pDC->FillSolidRect(rectClient, SageUiResources::GetBackgroundColor(SAGE_BG_APP));
	if (m_arrLabels.empty())
		return;

	std::vector<CRect> arrRects;
	BuildPillRects(pDC, rectClient, arrRects);

	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CAPTION));
	for (int i = 0; i < static_cast<int>(arrRects.size()); ++i)
		DrawPill(pDC, arrRects[i], m_arrLabels[i], (i == m_nSelectedIndex) ? TRUE : FALSE);
}
