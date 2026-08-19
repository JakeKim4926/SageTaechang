#include "pch.h"
#include "app/ui/drawing/SageComboBox.h"
#include "app/ui/drawing/SageUiStyle.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageComboBox, CComboBox)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_MESSAGE(WM_SETFONT, &CSageComboBox::OnSetFontMessage)
END_MESSAGE_MAP()

CSageComboBox::CSageComboBox()
	: m_bFittingField(FALSE) {
}

void CSageComboBox::ApplyFieldHeight() {
	if (!::IsWindow(GetSafeHwnd()) || m_bFittingField)
		return;

	m_bFittingField = TRUE;

	for (int nPass = 0; nPass < TAECHANG_COMBO_FIT_MAX_PASS; ++nPass) {
		COMBOBOXINFO info = {};
		info.cbSize = sizeof(COMBOBOXINFO);
		if (!GetComboBoxInfo(&info))
			break;

		int nFieldHeight = info.rcItem.bottom - info.rcItem.top;
		if (nFieldHeight <= 0)
			break;

		int nClosedHeight = info.rcItem.bottom + info.rcItem.top;
		int nDelta = nClosedHeight - TAECHANG_EDIT_HEIGHT;
		if (nDelta == 0)
			break;

		int nItemHeight = GetItemHeight(-1) - nDelta;
		if (nItemHeight <= 0)
			break;

		SetItemHeight(-1, nItemHeight);
	}

	m_bFittingField = FALSE;
}

void CSageComboBox::OnSize(UINT nType, int cx, int cy) {
	CComboBox::OnSize(nType, cx, cy);
	ApplyFieldHeight();
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
