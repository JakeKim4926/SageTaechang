#include "pch.h"
#include "app/ui/drawing/SageButton.h"
#include "TaechangDefine.h"

CSageButton::CSageButton()
	: m_nVariant(SAGE_BUTTON_SECONDARY)
	, m_nIcon(SAGE_BUTTON_ICON_NONE) {
}

void CSageButton::SetVariant(SageButtonVariant nVariant) {
	m_nVariant = nVariant;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageButton::SetIcon(SageButtonIcon nIcon) {
	m_nIcon = nIcon;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect = lpDrawItemStruct->rcItem;
	BOOL bPressed = (lpDrawItemStruct->itemState & ODS_SELECTED) != 0;
	BOOL bDisabled = (lpDrawItemStruct->itemState & ODS_DISABLED) != 0;

	if (m_nVariant == SAGE_BUTTON_PRIMARY) {
		COLORREF clrBg = bDisabled ? TAECHANG_COLOR_BORDER
			: bPressed ? TAECHANG_COLOR_PRIMARY_PRESS : TAECHANG_COLOR_PRIMARY;
		pDC->FillSolidRect(rect, clrBg);
		pDC->SetTextColor(bDisabled ? TAECHANG_COLOR_SECONDARY_TEXT : TAECHANG_COLOR_BUTTON_TEXT);
	} else {
		pDC->FillSolidRect(rect, bDisabled ? TAECHANG_COLOR_APP_BACKGROUND : TAECHANG_COLOR_PANEL);
		CBrush brBorder;
		brBorder.CreateSolidBrush(bDisabled ? TAECHANG_COLOR_BORDER : TAECHANG_COLOR_PRIMARY);
		pDC->FrameRect(rect, &brBorder);
		pDC->SetTextColor(bDisabled ? TAECHANG_COLOR_SECONDARY_TEXT : TAECHANG_COLOR_PRIMARY);
	}

	pDC->SetBkMode(TRANSPARENT);

	if (m_nIcon != SAGE_BUTTON_ICON_NONE) {
		COLORREF clrIcon = pDC->GetTextColor();
		if (m_nIcon == SAGE_BUTTON_ICON_SEARCH)
			DrawSearchIcon(*pDC, rect, clrIcon);
		else if (m_nIcon == SAGE_BUTTON_ICON_CALCULATE)
			DrawCalculateIcon(*pDC, rect, clrIcon);
		else
			DrawResetIcon(*pDC, rect, clrIcon);
		return;
	}

	CString strText;
	GetWindowText(strText);

	CFont* pOldFont = pDC->SelectObject(GetFont());
	rect.OffsetRect(0, TAECHANG_BUTTON_TEXT_TOP_OFFSET);
	pDC->DrawText(strText, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	if (pOldFont)
		pDC->SelectObject(pOldFont);
}

void CSageButton::DrawSearchIcon(CDC& dc, const CRect& rect, COLORREF clrIcon) {
	CPen pen(PS_SOLID, 2, clrIcon);
	CPen* pOldPen = dc.SelectObject(&pen);
	CBrush* pOldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
	int nCx = rect.CenterPoint().x - 2;
	int nCy = rect.CenterPoint().y - 2;
	dc.Ellipse(nCx - 6, nCy - 6, nCx + 7, nCy + 7);
	dc.MoveTo(nCx + 5, nCy + 5);
	dc.LineTo(nCx + 11, nCy + 11);
	if (pOldBrush)
		dc.SelectObject(pOldBrush);
	if (pOldPen)
		dc.SelectObject(pOldPen);
}

void CSageButton::DrawCalculateIcon(CDC& dc, const CRect& rect, COLORREF clrIcon) {
	CPen pen(PS_SOLID, 1, clrIcon);
	CPen* pOldPen = dc.SelectObject(&pen);
	CBrush* pOldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
	int cx = rect.CenterPoint().x;
	int cy = rect.CenterPoint().y;
	int dw = 6, dh = 7, fc = 3;
	POINT pts[] = {
		{cx - dw,      cy - dh},
		{cx + dw - fc, cy - dh},
		{cx + dw,      cy - dh + fc},
		{cx + dw,      cy + dh},
		{cx - dw,      cy + dh},
		{cx - dw,      cy - dh},
	};
	dc.Polyline(pts, 6);
	dc.MoveTo(cx + dw - fc, cy - dh);
	dc.LineTo(cx + dw - fc, cy - dh + fc);
	dc.LineTo(cx + dw,      cy - dh + fc);
	dc.MoveTo(cx - dw + 2, cy - 2); dc.LineTo(cx + dw - 2, cy - 2);
	dc.MoveTo(cx - dw + 2, cy + 1); dc.LineTo(cx + dw - 2, cy + 1);
	dc.MoveTo(cx - dw + 2, cy + 4); dc.LineTo(cx + dw - 4, cy + 4);
	if (pOldBrush) dc.SelectObject(pOldBrush);
	if (pOldPen)   dc.SelectObject(pOldPen);
}

void CSageButton::DrawResetIcon(CDC& dc, const CRect& rect, COLORREF clrIcon) {
	CPen pen(PS_SOLID, 2, clrIcon);
	CPen* pOldPen = dc.SelectObject(&pen);
	CBrush* pOldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
	int cx = rect.CenterPoint().x;
	int cy = rect.CenterPoint().y;
	POINT arcPts[] = {
		{cx + 6, cy + 2},
		{cx + 5, cy + 3},
		{cx + 3, cy + 5},
		{cx,     cy + 6},
		{cx - 3, cy + 5},
		{cx - 5, cy + 3},
		{cx - 6, cy},
		{cx - 5, cy - 3},
		{cx - 3, cy - 5},
		{cx,     cy - 6},
		{cx + 3, cy - 5},
	};
	dc.Polyline(arcPts, 11);
	POINT arrow[] = {
		{cx + 3 - 2, cy - 5 - 3},
		{cx + 3,     cy - 5},
		{cx + 3 + 3, cy - 5 + 2},
	};
	dc.Polyline(arrow, 3);
	if (pOldBrush) dc.SelectObject(pOldBrush);
	if (pOldPen)   dc.SelectObject(pOldPen);
}
