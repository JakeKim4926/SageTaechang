#include "pch.h"
#include "app/ui/drawing/TaechangComboBox.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CTaechangComboBox, CComboBox)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CTaechangComboBox::OnPaint() {
	CPaintDC dc(this);
	COMBOBOXINFO cbi = {};
	cbi.cbSize = sizeof(COMBOBOXINFO);
	GetComboBoxInfo(&cbi);
	CRect rcButton = cbi.rcButton;
	if (rcButton.IsRectEmpty())
		return;
	dc.FillSolidRect(rcButton, TAECHANG_COLOR_APP_BACKGROUND);
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
