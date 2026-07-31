#include "pch.h"
#include "app/ui/drawing/SageComboBox.h"
#include "app/ui/drawing/SageUiStyle.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageComboBox, CComboBox)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CSageComboBox::OnPaint() {
	CPaintDC dc(this);
	COMBOBOXINFO cbi = {};
	cbi.cbSize = sizeof(COMBOBOXINFO);
	GetComboBoxInfo(&cbi);
	CRect rcButton = cbi.rcButton;
	if (rcButton.IsRectEmpty())
		return;
	dc.FillSolidRect(rcButton, TAECHANG_COLOR_APP_BACKGROUND);
	SageUiStyle::DrawComboArrow(dc, rcButton);
}
