#include "pch.h"
#include "app/ui/drawing/SageBadge.h"
#include "app/ui/drawing/SageUiResources.h"
#include "SageDefine.h"

BEGIN_MESSAGE_MAP(CSageBadge, CStatic)
END_MESSAGE_MAP()

CSageBadge::CSageBadge()
	: m_clrBackground(SAGE_COLOR_LIST_HEADER)
	, m_clrBorder(SAGE_COLOR_LIST_HEADER_BORDER)
	, m_clrText(SAGE_COLOR_PRIMARY)
	, m_clrSurface(SAGE_COLOR_PANEL)
	, m_nCornerRadius(SAGE_BADGE_HEIGHT) {
}

void CSageBadge::SetCornerRadius(int nRadius) {
	m_nCornerRadius = nRadius;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageBadge::SetSurfaceColor(COLORREF clrSurface) {
	m_clrSurface = clrSurface;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
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
	return sizeText.cx + SAGE_BADGE_PAD_X * 2;
}

void CSageBadge::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);

	pDC->FillSolidRect(rectClient, m_clrSurface);
	if (m_strText.IsEmpty())
		return;

	CRect rectBadge(rectClient);
	rectBadge.top += (rectClient.Height() - SAGE_BADGE_HEIGHT) / 2;
	rectBadge.bottom = rectBadge.top + SAGE_BADGE_HEIGHT;

	CBrush brushFill(m_clrBackground);
	CPen penBorder(PS_SOLID, SAGE_BORDER_THICKNESS, m_clrBorder);
	CBrush* pOldBrush = pDC->SelectObject(&brushFill);
	CPen* pOldPen = pDC->SelectObject(&penBorder);
	pDC->RoundRect(rectBadge, CPoint(m_nCornerRadius, m_nCornerRadius));
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
