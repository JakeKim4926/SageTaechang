#include "pch.h"
#include "app/ui/drawing/SageUiStyle.h"
#include "TaechangDefine.h"

namespace SageUiStyle {

void DrawComboArrow(CDC& dc, const CRect& rcButton) {
	int cx = (rcButton.left + rcButton.right) / 2;
	int cy = (rcButton.top + rcButton.bottom) / 2;
	POINT pts[3] = {
		{ cx - 4, cy - 2 },
		{ cx + 4, cy - 2 },
		{ cx,     cy + 3 }
	};
	CBrush br(TAECHANG_COLOR_PRIMARY);
	CPen pen(PS_NULL, 0, RGB(0, 0, 0));
	CBrush* pOldBr = dc.SelectObject(&br);
	CPen* pOldPen = dc.SelectObject(&pen);
	dc.Polygon(pts, 3);
	dc.SelectObject(pOldBr);
	dc.SelectObject(pOldPen);
}

void DrawCheckBox(CDC& dc, const CRect& rectBox, BOOL bChecked) {
	if (!bChecked) {
		dc.FillSolidRect(rectBox, TAECHANG_COLOR_PANEL);
		CBrush brushBorder(TAECHANG_COLOR_BUTTON_BORDER);
		dc.FrameRect(rectBox, &brushBorder);
		return;
	}

	dc.FillSolidRect(rectBox, TAECHANG_COLOR_PRIMARY);
	CPen penMark(PS_SOLID, TAECHANG_LIST_CHECK_MARK_THICKNESS, TAECHANG_COLOR_PANEL);
	CPen* pOldPen = dc.SelectObject(&penMark);
	int nInset = TAECHANG_LIST_CHECK_MARK_THICKNESS * 2;
	dc.MoveTo(rectBox.left + nInset, rectBox.top + rectBox.Height() / 2);
	dc.LineTo(rectBox.left + rectBox.Width() / 2, rectBox.bottom - nInset);
	dc.LineTo(rectBox.right - nInset, rectBox.top + nInset);
	dc.SelectObject(pOldPen);
}

void DrawSearchIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon) {
	CPen pen(PS_SOLID, TAECHANG_ICON_STROKE, clrIcon);
	CPen* pOldPen = dc.SelectObject(&pen);
	CBrush* pOldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);

	int nLensCx = ptCenter.x - TAECHANG_ICON_SEARCH_HANDLE / 2;
	int nLensCy = ptCenter.y - TAECHANG_ICON_SEARCH_HANDLE / 2;
	dc.Ellipse(nLensCx - TAECHANG_ICON_SEARCH_RADIUS, nLensCy - TAECHANG_ICON_SEARCH_RADIUS,
		nLensCx + TAECHANG_ICON_SEARCH_RADIUS, nLensCy + TAECHANG_ICON_SEARCH_RADIUS);
	dc.MoveTo(nLensCx + TAECHANG_ICON_SEARCH_RADIUS - 1, nLensCy + TAECHANG_ICON_SEARCH_RADIUS - 1);
	dc.LineTo(nLensCx + TAECHANG_ICON_SEARCH_RADIUS + TAECHANG_ICON_SEARCH_HANDLE,
		nLensCy + TAECHANG_ICON_SEARCH_RADIUS + TAECHANG_ICON_SEARCH_HANDLE);

	if (pOldBrush)
		dc.SelectObject(pOldBrush);
	if (pOldPen)
		dc.SelectObject(pOldPen);
}

}
