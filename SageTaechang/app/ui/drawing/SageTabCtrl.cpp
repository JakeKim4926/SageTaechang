#include "pch.h"
#include "app/ui/drawing/SageTabCtrl.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageTabCtrl, CTabCtrl)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CSageTabCtrl::OnPaint() {
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);
	dc.FillSolidRect(rect, TAECHANG_COLOR_APP_BACKGROUND);

	CFont* pFont = GetFont();
	CFont* pOldFont = pFont ? dc.SelectObject(pFont) : NULL;
	dc.SetBkMode(TRANSPARENT);

	int nCount = GetItemCount();
	int nCurSel = GetCurSel();
	for (int i = 0; i < nCount; ++i) {
		CRect rcItem;
		GetItemRect(i, &rcItem);
		BOOL bSelected = (i == nCurSel);

		dc.FillSolidRect(rcItem, bSelected ? TAECHANG_COLOR_PANEL : TAECHANG_COLOR_APP_BACKGROUND);

		if (bSelected) {
			CRect rcLine = rcItem;
			rcLine.top = rcLine.bottom - TAECHANG_TAB_INDICATOR_HEIGHT;
			dc.FillSolidRect(rcLine, TAECHANG_COLOR_PRIMARY);
		}

		TCITEM tcItem;
		wchar_t szText[64] = {};
		tcItem.mask = TCIF_TEXT;
		tcItem.pszText = szText;
		tcItem.cchTextMax = 63;
		GetItem(i, &tcItem);

		dc.SetTextColor(bSelected ? TAECHANG_COLOR_TEXT : TAECHANG_COLOR_SECONDARY_TEXT);
		dc.DrawText(szText, rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	if (pOldFont)
		dc.SelectObject(pOldFont);
}
