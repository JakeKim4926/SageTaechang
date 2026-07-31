#include "pch.h"
#include "app/ui/drawing/TaechangHeaderCtrl.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CTaechangHeaderCtrl, CHeaderCtrl)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CTaechangHeaderCtrl::OnPaint() {
	CPaintDC dc(this);
	CRect rectClient;
	GetClientRect(&rectClient);
	dc.FillSolidRect(rectClient, TAECHANG_COLOR_PANEL);

	int nCount = GetItemCount();

	CFont* pFont = GetFont();
	CFont* pOldFont = pFont ? dc.SelectObject(pFont) : NULL;
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(TAECHANG_COLOR_BUTTON_TEXT);

	for (int i = 0; i < nCount; ++i) {
		CRect rcItem;
		GetItemRect(i, &rcItem);

		dc.FillSolidRect(rcItem, TAECHANG_COLOR_LIST_HEADER);

		HDITEM hdItem = {};
		wchar_t szText[256] = {};
		hdItem.mask = HDI_TEXT | HDI_FORMAT;
		hdItem.pszText = szText;
		hdItem.cchTextMax = 255;
		GetItem(i, &hdItem);

		if (i < nCount - 1)
			dc.FillSolidRect(rcItem.right - 1, rcItem.top + 4, 1, rcItem.Height() - 8, TAECHANG_COLOR_LIST_HEADER_DIVIDER);

		UINT uFormat = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
		if (hdItem.fmt & HDF_RIGHT) {
			rcItem.right -= 8;
			uFormat |= DT_RIGHT;
		} else if (hdItem.fmt & HDF_CENTER) {
			uFormat |= DT_CENTER;
		} else {
			rcItem.left += 8;
			uFormat |= DT_LEFT;
		}
		dc.DrawText(szText, rcItem, uFormat);
	}

	if (pOldFont)
		dc.SelectObject(pOldFont);
}
