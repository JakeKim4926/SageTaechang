#include "pch.h"
#include "app/ui/drawing/SageComboBox.h"
#include "app/ui/drawing/SageUiStyle.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageComboBox, CComboBox)
	ON_WM_PAINT()
	ON_MESSAGE(WM_SETFONT, &CSageComboBox::OnSetFontMessage)
END_MESSAGE_MAP()

void CSageComboBox::ApplyFieldHeight() {
	if (!::IsWindow(GetSafeHwnd()))
		return;
	SetItemHeight(-1, TAECHANG_EDIT_HEIGHT - TAECHANG_COMBO_FIELD_INSET);
	SetItemHeight(0, TAECHANG_EDIT_HEIGHT);
}

LRESULT CSageComboBox::OnSetFontMessage(WPARAM wParam, LPARAM lParam) {
	LRESULT lResult = Default();
	ApplyFieldHeight();
	return lResult;
}

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
