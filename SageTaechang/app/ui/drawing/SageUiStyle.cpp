#include "pch.h"
#include "app/ui/drawing/SageUiStyle.h"
#include "SageDefine.h"

namespace SageUiStyle {

void DrawComboArrow(CDC& dc, const CRect& rcButton) {
	int cx = (rcButton.left + rcButton.right) / 2;
	int cy = (rcButton.top + rcButton.bottom) / 2;
	POINT pts[3] = {
		{ cx - 4, cy - 2 },
		{ cx + 4, cy - 2 },
		{ cx,     cy + 3 }
	};
	CBrush br(SAGE_COLOR_PRIMARY);
	CPen pen(PS_NULL, 0, RGB(0, 0, 0));
	CBrush* pOldBr = dc.SelectObject(&br);
	CPen* pOldPen = dc.SelectObject(&pen);
	dc.Polygon(pts, 3);
	dc.SelectObject(pOldBr);
	dc.SelectObject(pOldPen);
}

void DrawCheckBox(CDC& dc, const CRect& rectBox, BOOL bChecked) {
	if (!bChecked) {
		dc.FillSolidRect(rectBox, SAGE_COLOR_PANEL);
		CBrush brushBorder(SAGE_COLOR_BUTTON_BORDER);
		dc.FrameRect(rectBox, &brushBorder);
		return;
	}

	dc.FillSolidRect(rectBox, SAGE_COLOR_PRIMARY);
	CPen penMark(PS_SOLID, SAGE_LIST_CHECK_MARK_THICKNESS, SAGE_COLOR_PANEL);
	CPen* pOldPen = dc.SelectObject(&penMark);
	int nInset = SAGE_LIST_CHECK_MARK_THICKNESS * 2;
	dc.MoveTo(rectBox.left + nInset, rectBox.top + rectBox.Height() / 2);
	dc.LineTo(rectBox.left + rectBox.Width() / 2, rectBox.bottom - nInset);
	dc.LineTo(rectBox.right - nInset, rectBox.top + nInset);
	dc.SelectObject(pOldPen);
}

void DrawSearchIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon) {
	CPen pen(PS_SOLID, SAGE_ICON_STROKE, clrIcon);
	CPen* pOldPen = dc.SelectObject(&pen);
	CBrush* pOldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);

	int nLensCx = ptCenter.x - SAGE_ICON_SEARCH_HANDLE / 2;
	int nLensCy = ptCenter.y - SAGE_ICON_SEARCH_HANDLE / 2;
	dc.Ellipse(nLensCx - SAGE_ICON_SEARCH_RADIUS, nLensCy - SAGE_ICON_SEARCH_RADIUS,
		nLensCx + SAGE_ICON_SEARCH_RADIUS, nLensCy + SAGE_ICON_SEARCH_RADIUS);
	dc.MoveTo(nLensCx + SAGE_ICON_SEARCH_RADIUS - 1, nLensCy + SAGE_ICON_SEARCH_RADIUS - 1);
	dc.LineTo(nLensCx + SAGE_ICON_SEARCH_RADIUS + SAGE_ICON_SEARCH_HANDLE,
		nLensCy + SAGE_ICON_SEARCH_RADIUS + SAGE_ICON_SEARCH_HANDLE);

	if (pOldBrush)
		dc.SelectObject(pOldBrush);
	if (pOldPen)
		dc.SelectObject(pOldPen);
}

}
