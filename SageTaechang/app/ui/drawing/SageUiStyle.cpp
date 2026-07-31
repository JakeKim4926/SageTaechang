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

}
