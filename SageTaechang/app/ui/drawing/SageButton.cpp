#include "pch.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageUiStyle.h"
#include "TaechangDefine.h"

CSageButton::CSageButton()
	: m_nVariant(SAGE_BUTTON_SECONDARY)
	, m_nIcon(SAGE_BUTTON_ICON_NONE)
	, m_clrSurface(TAECHANG_COLOR_PANEL)
	, m_bFocusRing(FALSE) {
}

void CSageButton::SetSurfaceColor(COLORREF clrSurface) {
	m_clrSurface = clrSurface;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
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

void CSageButton::SetTooltip(const CString& strTooltip) {
	if (!::IsWindow(GetSafeHwnd()))
		return;

	if (m_toolTip.GetSafeHwnd() == NULL) {
		if (!m_toolTip.Create(this))
			return;
		m_toolTip.AddTool(this, strTooltip);
		m_toolTip.Activate(TRUE);
		return;
	}
	m_toolTip.UpdateTipText(strTooltip, this);
}

BOOL CSageButton::PreTranslateMessage(MSG* pMsg) {
	if (m_toolTip.GetSafeHwnd() != NULL)
		m_toolTip.RelayEvent(pMsg);
	return CButton::PreTranslateMessage(pMsg);
}

void CSageButton::SetFocusRing(BOOL bVisible) {
	m_bFocusRing = bVisible;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

COLORREF CSageButton::GetFocusRingColor() const {
	return (m_nVariant == SAGE_BUTTON_PRIMARY)
		? TAECHANG_COLOR_FOCUS_RING_PRIMARY : TAECHANG_COLOR_FOCUS_RING_NEUTRAL;
}

void CSageButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect = lpDrawItemStruct->rcItem;
	BOOL bPressed = (lpDrawItemStruct->itemState & ODS_SELECTED) != 0;
	BOOL bDisabled = (lpDrawItemStruct->itemState & ODS_DISABLED) != 0;

	if (m_bFocusRing) {
		BOOL bFocused = (lpDrawItemStruct->itemState & ODS_FOCUS) != 0;
		pDC->FillSolidRect(rect, bFocused ? GetFocusRingColor() : m_clrSurface);
		rect.DeflateRect(TAECHANG_FOCUS_RING_WIDTH, TAECHANG_FOCUS_RING_WIDTH);
	}

	if (m_nVariant == SAGE_BUTTON_PRIMARY) {
		COLORREF clrBg = bDisabled ? TAECHANG_COLOR_BORDER
			: bPressed ? TAECHANG_COLOR_PRIMARY_PRESS : TAECHANG_COLOR_PRIMARY;
		pDC->FillSolidRect(rect, clrBg);
		pDC->SetTextColor(bDisabled ? TAECHANG_COLOR_SECONDARY_TEXT : TAECHANG_COLOR_BUTTON_TEXT);
	} else if (m_nVariant == SAGE_BUTTON_GHOST) {
		pDC->FillSolidRect(rect, bPressed ? TAECHANG_COLOR_LIST_HEADER : m_clrSurface);
		pDC->SetTextColor(bDisabled ? TAECHANG_COLOR_BORDER : TAECHANG_COLOR_TEXT_MUTED);
	} else {
		COLORREF clrBorder = (m_nVariant == SAGE_BUTTON_DANGER)
			? TAECHANG_COLOR_DANGER_BORDER : TAECHANG_COLOR_BUTTON_BORDER;
		COLORREF clrLabel = (m_nVariant == SAGE_BUTTON_DANGER)
			? TAECHANG_COLOR_ERROR : TAECHANG_COLOR_TEXT;
		pDC->FillSolidRect(rect, bDisabled || bPressed
			? TAECHANG_COLOR_APP_BACKGROUND : TAECHANG_COLOR_PANEL);
		CBrush brBorder;
		brBorder.CreateSolidBrush(bDisabled ? TAECHANG_COLOR_BORDER : clrBorder);
		pDC->FrameRect(rect, &brBorder);
		pDC->SetTextColor(bDisabled ? TAECHANG_COLOR_SECONDARY_TEXT : clrLabel);
	}

	pDC->SetBkMode(TRANSPARENT);

	COLORREF clrIcon = pDC->GetTextColor();
	CString strText;
	GetWindowText(strText);

	if (m_nIcon != SAGE_BUTTON_ICON_NONE && strText.IsEmpty()) {
		DrawIconAt(*pDC, rect.CenterPoint(), clrIcon);
		return;
	}

	CFont* pOldFont = pDC->SelectObject(GetFont());

	if (m_nIcon == SAGE_BUTTON_ICON_NONE) {
		rect.OffsetRect(0, TAECHANG_BUTTON_TEXT_TOP_OFFSET);
		pDC->DrawText(strText, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		if (pOldFont)
			pDC->SelectObject(pOldFont);
		return;
	}

	int nIconSize = GetIconSize();
	CSize sizeText = pDC->GetTextExtent(strText);
	int nGroupWidth = nIconSize + TAECHANG_ICON_TEXT_GAP + sizeText.cx;
	int nGroupLeft = rect.left + (rect.Width() - nGroupWidth) / 2;

	DrawIconAt(*pDC, CPoint(nGroupLeft + nIconSize / 2, rect.CenterPoint().y), clrIcon);

	CRect rectText(nGroupLeft + nIconSize + TAECHANG_ICON_TEXT_GAP, rect.top,
		rect.right, rect.bottom);
	rectText.OffsetRect(0, TAECHANG_BUTTON_TEXT_TOP_OFFSET);
	pDC->DrawText(strText, rectText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	if (pOldFont)
		pDC->SelectObject(pOldFont);
}

int CSageButton::GetIconSize() const {
	return (m_nIcon == SAGE_BUTTON_ICON_ADD) ? TAECHANG_ICON_ADD_SIZE : TAECHANG_ICON_SIZE;
}

void CSageButton::DrawIconAt(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon) {
	if (m_nIcon == SAGE_BUTTON_ICON_SEARCH)
		DrawSearchIcon(dc, ptCenter, clrIcon);
	else if (m_nIcon == SAGE_BUTTON_ICON_CALCULATE)
		DrawCalculateIcon(dc, ptCenter, clrIcon);
	else if (m_nIcon == SAGE_BUTTON_ICON_ADD)
		DrawAddIcon(dc, ptCenter, clrIcon);
	else if (m_nIcon == SAGE_BUTTON_ICON_CLOSE)
		DrawCloseIcon(dc, ptCenter, clrIcon);
	else if (m_nIcon == SAGE_BUTTON_ICON_MOVE_UP)
		DrawArrowIcon(dc, ptCenter, clrIcon, TRUE);
	else if (m_nIcon == SAGE_BUTTON_ICON_MOVE_DOWN)
		DrawArrowIcon(dc, ptCenter, clrIcon, FALSE);
	else
		DrawResetIcon(dc, ptCenter, clrIcon);
}

void CSageButton::DrawSearchIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon) {
	SageUiStyle::DrawSearchIcon(dc, ptCenter, clrIcon);
}

void CSageButton::DrawAddIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon) {
	int nHalfSpan = TAECHANG_ICON_ADD_SPAN / 2;
	int nHalfStroke = TAECHANG_ICON_STROKE / 2;

	dc.FillSolidRect(ptCenter.x - nHalfSpan, ptCenter.y - nHalfStroke,
		TAECHANG_ICON_ADD_SPAN, TAECHANG_ICON_STROKE, clrIcon);
	dc.FillSolidRect(ptCenter.x - nHalfStroke, ptCenter.y - nHalfSpan,
		TAECHANG_ICON_STROKE, TAECHANG_ICON_ADD_SPAN, clrIcon);
}

void CSageButton::DrawCloseIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon) {
	CPen pen(PS_SOLID, TAECHANG_ICON_STROKE, clrIcon);
	CPen* pOldPen = dc.SelectObject(&pen);

	int nHalfSpan = TAECHANG_ICON_CLOSE_SPAN / 2;
	dc.MoveTo(ptCenter.x - nHalfSpan, ptCenter.y - nHalfSpan);
	dc.LineTo(ptCenter.x + nHalfSpan, ptCenter.y + nHalfSpan);
	dc.MoveTo(ptCenter.x + nHalfSpan, ptCenter.y - nHalfSpan);
	dc.LineTo(ptCenter.x - nHalfSpan, ptCenter.y + nHalfSpan);

	if (pOldPen)
		dc.SelectObject(pOldPen);
}

void CSageButton::DrawArrowIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon, BOOL bUp) {
	CPen pen(PS_SOLID, TAECHANG_ICON_STROKE, clrIcon);
	CPen* pOldPen = dc.SelectObject(&pen);

	int nTipY = bUp
		? ptCenter.y - TAECHANG_ICON_ARROW_HALF_HEIGHT
		: ptCenter.y + TAECHANG_ICON_ARROW_HALF_HEIGHT;
	int nTailY = bUp
		? ptCenter.y + TAECHANG_ICON_ARROW_HALF_HEIGHT
		: ptCenter.y - TAECHANG_ICON_ARROW_HALF_HEIGHT;

	dc.MoveTo(ptCenter.x - TAECHANG_ICON_ARROW_HALF_WIDTH, nTailY);
	dc.LineTo(ptCenter.x, nTipY);
	dc.LineTo(ptCenter.x + TAECHANG_ICON_ARROW_HALF_WIDTH, nTailY);

	if (pOldPen)
		dc.SelectObject(pOldPen);
}

void CSageButton::DrawCalculateIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon) {
	CPen pen(PS_SOLID, 1, clrIcon);
	CPen* pOldPen = dc.SelectObject(&pen);
	CBrush* pOldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
	int cx = ptCenter.x;
	int cy = ptCenter.y;
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

void CSageButton::DrawResetIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon) {
	CPen pen(PS_SOLID, TAECHANG_ICON_STROKE, clrIcon);
	CPen* pOldPen = dc.SelectObject(&pen);
	CBrush* pOldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);

	int nRadius = TAECHANG_ICON_RESET_RADIUS;
	int nArrow = TAECHANG_ICON_RESET_ARROW;
	int cx = ptCenter.x;
	int cy = ptCenter.y;

	CRect rectArc(cx - nRadius, cy - nRadius, cx + nRadius + 1, cy + nRadius + 1);
	dc.Arc(rectArc, CPoint(cx + nArrow, cy - nRadius), CPoint(cx + nRadius, cy + nArrow));

	POINT arrow[] = {
		{cx,              cy - nRadius - nArrow},
		{cx + nArrow,     cy - nRadius},
		{cx + nArrow * 2, cy - nRadius + nArrow},
	};
	dc.Polyline(arrow, 3);
	if (pOldBrush) dc.SelectObject(pOldBrush);
	if (pOldPen)   dc.SelectObject(pOldPen);
}
