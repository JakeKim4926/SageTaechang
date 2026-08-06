#include "pch.h"
#include "app/ui/drawing/SageTabCtrl.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageTabCtrl, CTabCtrl)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CSageTabCtrl::ApplyTabHeight() {
	if (GetItemCount() == 0)
		return;

	CRect rcItem;
	GetItemRect(0, &rcItem);
	if (rcItem.Height() >= TAECHANG_TAB_HEIGHT)
		return;
	SetItemSize(CSize(rcItem.Width(), TAECHANG_TAB_HEIGHT));
}

void CSageTabCtrl::OnPaint() {
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);
	dc.FillSolidRect(rect, TAECHANG_COLOR_PANEL);
	dc.FillSolidRect(
		rect.left,
		rect.bottom - TAECHANG_BORDER_THICKNESS,
		rect.Width(),
		TAECHANG_BORDER_THICKNESS,
		TAECHANG_COLOR_BORDER);

	dc.SetBkMode(TRANSPARENT);

	int nCount = GetItemCount();
	int nCurSel = GetCurSel();
	for (int i = 0; i < nCount; ++i) {
		CRect rcItem;
		GetItemRect(i, &rcItem);
		BOOL bSelected = (i == nCurSel);

		if (bSelected) {
			CRect rcLine = rcItem;
			rcLine.top = rect.bottom - TAECHANG_TAB_INDICATOR_HEIGHT;
			rcLine.bottom = rect.bottom;
			dc.FillSolidRect(rcLine, TAECHANG_COLOR_PRIMARY);
		}

		TCITEM tcItem;
		wchar_t szText[64] = {};
		tcItem.mask = TCIF_TEXT;
		tcItem.pszText = szText;
		tcItem.cchTextMax = 63;
		GetItem(i, &tcItem);

		CRect rcText(rcItem.left, rect.top, rcItem.right, rect.bottom - TAECHANG_TAB_INDICATOR_HEIGHT);
		CFont* pOldFont = dc.SelectObject(SageUiResources::GetFont(
			bSelected ? SAGE_FONT_CONTENT_SEMIBOLD : SAGE_FONT_CONTENT));
		dc.SetTextColor(bSelected ? TAECHANG_COLOR_TEXT : TAECHANG_COLOR_SECONDARY_TEXT);
		dc.DrawText(szText, rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		dc.SelectObject(pOldFont);
	}
}
