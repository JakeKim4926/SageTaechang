#include "pch.h"
#include "app/ui/drawing/SageSectionLabel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

void CSageSectionLabel::SetHintText(LPCWSTR pszHint) {
	m_strHint = pszHint;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageSectionLabel::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect = lpDrawItemStruct->rcItem;
	pDC->FillSolidRect(rect, TAECHANG_COLOR_LIST_HEADER);
	pDC->FillSolidRect(
		rect.left,
		rect.bottom - TAECHANG_BORDER_THICKNESS,
		rect.Width(),
		TAECHANG_BORDER_THICKNESS,
		TAECHANG_COLOR_BORDER);

	CString strText;
	GetWindowText(strText);

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
	CFont* pOldFont = pDC->SelectObject(GetFont());
	CRect rcText = rect;
	rcText.left += TAECHANG_CARD_PADDING;
	pDC->DrawText(strText, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	if (!m_strHint.IsEmpty()) {
		pDC->SetTextColor(TAECHANG_COLOR_SECONDARY_TEXT);
		pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CAPTION));
		CRect rcHint = rect;
		rcHint.right -= TAECHANG_CARD_PADDING;
		pDC->DrawText(m_strHint, rcHint, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}

	if (pOldFont)
		pDC->SelectObject(pOldFont);
}
