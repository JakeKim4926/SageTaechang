#include "pch.h"
#include "app/ui/drawing/SageInlineError.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageInlineError, CStatic)
END_MESSAGE_MAP()

CSageInlineError::CSageInlineError()
	: m_nVariant(SAGE_INLINE_ERROR), m_nBackgroundRole(SAGE_BG_APP) {
}

void CSageInlineError::SetBackgroundRole(SageBackgroundRole nRole) {
	m_nBackgroundRole = nRole;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageInlineError::SetMessage(const CString& strMessage, SageInlineMessageVariant nVariant) {
	m_strMessage = strMessage;
	m_nVariant = nVariant;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageInlineError::ClearMessage() {
	if (m_strMessage.IsEmpty())
		return;
	m_strMessage.Empty();
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

BOOL CSageInlineError::HasMessage() const {
	return m_strMessage.IsEmpty() ? FALSE : TRUE;
}

COLORREF CSageInlineError::GetIconColor() const {
	return (m_nVariant == SAGE_INLINE_WARNING)
		? TAECHANG_COLOR_WARNING : TAECHANG_COLOR_ERROR;
}

COLORREF CSageInlineError::GetTextColor() const {
	return (m_nVariant == SAGE_INLINE_WARNING)
		? TAECHANG_COLOR_INLINE_WARN_TEXT : TAECHANG_COLOR_INLINE_ERROR_TEXT;
}

void CSageInlineError::DrawWarningBox(CDC* pDC, const CRect& rectClient) {
	CBrush brushFill(TAECHANG_COLOR_INLINE_WARN_BG);
	CPen penBorder(PS_SOLID, TAECHANG_BORDER_THICKNESS, TAECHANG_COLOR_INLINE_WARN_BORDER);
	CBrush* pOldBrush = pDC->SelectObject(&brushFill);
	CPen* pOldPen = pDC->SelectObject(&penBorder);

	pDC->RoundRect(rectClient,
		CPoint(TAECHANG_INLINE_MSG_BOX_RADIUS * 2, TAECHANG_INLINE_MSG_BOX_RADIUS * 2));

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}

void CSageInlineError::DrawMessageIcon(CDC* pDC, const CRect& rectIcon) {
	COLORREF clrIcon = GetIconColor();
	CPen penIcon(PS_SOLID, TAECHANG_BORDER_THICKNESS, clrIcon);
	CPen* pOldPen = pDC->SelectObject(&penIcon);
	CGdiObject* pOldBrush = pDC->SelectStockObject(NULL_BRUSH);

	CPoint ptCenter = rectIcon.CenterPoint();
	pDC->Ellipse(ptCenter.x - TAECHANG_INLINE_ICON_RADIUS, ptCenter.y - TAECHANG_INLINE_ICON_RADIUS,
		ptCenter.x + TAECHANG_INLINE_ICON_RADIUS, ptCenter.y + TAECHANG_INLINE_ICON_RADIUS);
	pDC->MoveTo(ptCenter.x, rectIcon.top + TAECHANG_INLINE_ICON_STEM_TOP);
	pDC->LineTo(ptCenter.x, rectIcon.top + TAECHANG_INLINE_ICON_STEM_BOTTOM);
	pDC->FillSolidRect(ptCenter.x, rectIcon.top + TAECHANG_INLINE_ICON_DOT_TOP,
		TAECHANG_INLINE_ICON_DOT_SIZE, TAECHANG_INLINE_ICON_DOT_SIZE, clrIcon);

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

void CSageInlineError::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);

	pDC->FillSolidRect(rectClient,
		SageUiResources::GetBackgroundColor(m_nBackgroundRole));

	if (m_strMessage.IsEmpty())
		return;

	if (m_nVariant == SAGE_INLINE_WARNING)
		DrawWarningBox(pDC, rectClient);

	CRect rectIcon(rectClient);
	if (m_nVariant == SAGE_INLINE_WARNING)
		rectIcon.left += TAECHANG_INLINE_MSG_BOX_PAD_X;
	rectIcon.top += (rectClient.Height() - TAECHANG_INLINE_MSG_ICON_SIZE) / 2;
	rectIcon.right = rectIcon.left + TAECHANG_INLINE_MSG_ICON_SIZE;
	rectIcon.bottom = rectIcon.top + TAECHANG_INLINE_MSG_ICON_SIZE;
	DrawMessageIcon(pDC, rectIcon);

	CRect rectText(rectClient);
	rectText.left = rectIcon.right + TAECHANG_INLINE_MSG_ICON_GAP;
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(GetTextColor());
	pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CAPTION));
	pDC->DrawText(m_strMessage, &rectText,
		DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}
