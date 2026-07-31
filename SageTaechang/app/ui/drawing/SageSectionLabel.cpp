#include "pch.h"
#include "app/ui/drawing/SageSectionLabel.h"
#include "TaechangDefine.h"

void CSageSectionLabel::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect = lpDrawItemStruct->rcItem;
	pDC->FillSolidRect(rect, TAECHANG_COLOR_APP_BACKGROUND);
	constexpr int nAccentWidth = 3;
	pDC->FillSolidRect(rect.left, rect.top + 2, nAccentWidth, rect.Height() - 4, TAECHANG_COLOR_PRIMARY);

	CString strText;
	GetWindowText(strText);

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
	CFont* pOldFont = pDC->SelectObject(GetFont());
	CRect rcText = rect;
	rcText.left += nAccentWidth + 8;
	pDC->DrawText(strText, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	if (pOldFont)
		pDC->SelectObject(pOldFont);
}
