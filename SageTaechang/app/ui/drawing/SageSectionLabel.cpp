#include "pch.h"
#include "app/ui/drawing/SageSectionLabel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "SageDefine.h"

void CSageSectionLabel::SetHintText(LPCWSTR pszHint) {
	m_strHint = pszHint;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageSectionLabel::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect = lpDrawItemStruct->rcItem;
	pDC->FillSolidRect(rect, SAGE_COLOR_LIST_HEADER);
	pDC->FillSolidRect(
		rect.left,
		rect.bottom - SAGE_BORDER_THICKNESS,
		rect.Width(),
		SAGE_BORDER_THICKNESS,
		SAGE_COLOR_BORDER);

	CString strText;
	GetWindowText(strText);

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(SAGE_COLOR_TEXT);
	CFont* pOldFont = pDC->SelectObject(GetFont());
	CRect rcText = rect;
	rcText.left += SAGE_CARD_PADDING;
	pDC->DrawText(strText, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	if (!m_strHint.IsEmpty()) {
		pDC->SetTextColor(SAGE_COLOR_SECONDARY_TEXT);
		pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CAPTION));
		CRect rcHint = rect;
		rcHint.right -= SAGE_CARD_PADDING;
		pDC->DrawText(m_strHint, rcHint, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}

	if (pOldFont)
		pDC->SelectObject(pOldFont);
}
