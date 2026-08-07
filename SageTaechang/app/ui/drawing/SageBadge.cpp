#include "pch.h"
#include "app/ui/drawing/SageBadge.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageBadge, CStatic)
END_MESSAGE_MAP()

CSageBadge::CSageBadge()
	: m_clrBackground(TAECHANG_COLOR_LIST_HEADER)
	, m_clrBorder(TAECHANG_COLOR_LIST_HEADER_BORDER)
	, m_clrText(TAECHANG_COLOR_PRIMARY) {
}

void CSageBadge::SetBadge(
	const CString& strText, COLORREF clrBackground, COLORREF clrBorder, COLORREF clrText) {
	m_strText = strText;
	m_clrBackground = clrBackground;
	m_clrBorder = clrBorder;
	m_clrText = clrText;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

int CSageBadge::GetContentWidth() const {
	if (!::IsWindow(GetSafeHwnd()) || m_strText.IsEmpty())
		return 0;

	CClientDC dc(const_cast<CSageBadge*>(this));
	CFont* pOldFont = dc.SelectObject(SageUiResources::GetFont(SAGE_FONT_CAPTION));
	CSize sizeText = dc.GetTextExtent(m_strText);
	dc.SelectObject(pOldFont);
	return sizeText.cx + TAECHANG_BADGE_PAD_X * 2;
}

void CSageBadge::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);

	pDC->FillSolidRect(rectClient, SageUiResources::GetBackgroundColor(SAGE_BG_PANEL));
	if (m_strText.IsEmpty())
		return;

	CRect rectBadge(rectClient);
	rectBadge.top += (rectClient.Height() - TAECHANG_BADGE_HEIGHT) / 2;
	rectBadge.bottom = rectBadge.top + TAECHANG_BADGE_HEIGHT;

	CBrush brushFill(m_clrBackground);
	CPen penBorder(PS_SOLID, TAECHANG_BORDER_THICKNESS, m_clrBorder);
	CBrush* pOldBrush = pDC->SelectObject(&brushFill);
	CPen* pOldPen = pDC->SelectObject(&penBorder);
	pDC->RoundRect(rectBadge, CPoint(TAECHANG_BADGE_HEIGHT, TAECHANG_BADGE_HEIGHT));
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CAPTION));
	int nOldBkMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF clrOldText = pDC->SetTextColor(m_clrText);
	pDC->DrawText(m_strText, &rectBadge, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->SetTextColor(clrOldText);
	pDC->SetBkMode(nOldBkMode);
	if (pOldFont)
		pDC->SelectObject(pOldFont);
}
