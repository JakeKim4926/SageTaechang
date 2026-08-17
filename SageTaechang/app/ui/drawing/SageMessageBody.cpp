#include "pch.h"
#include "app/ui/drawing/SageMessageBody.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageMessageBody, CStatic)
END_MESSAGE_MAP()

CSageMessageBody::CSageMessageBody()
	: m_nIcon(SAGE_MESSAGE_ICON_NONE) {
}

void CSageMessageBody::SetMessage(const CString& strMessage, SageMessageIcon nIcon) {
	m_strMessage = strMessage;
	m_nIcon = nIcon;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

int CSageMessageBody::MeasureHeight(int nWidth) {
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(SageUiResources::GetFont(SAGE_FONT_CONTENT));

	CRect rectText(0, 0, nWidth - GetTextLeft(), 0);
	dc.DrawText(m_strMessage, &rectText, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
	dc.SelectObject(pOldFont);

	int nHeight = rectText.Height();
	if (nHeight > TAECHANG_MSGBOX_MAX_TEXT_HEIGHT)
		nHeight = TAECHANG_MSGBOX_MAX_TEXT_HEIGHT;
	if (nHeight < TAECHANG_MSGBOX_ICON_SIZE)
		nHeight = TAECHANG_MSGBOX_ICON_SIZE;

	return nHeight;
}

COLORREF CSageMessageBody::GetIconColor() const {
	if (m_nIcon == SAGE_MESSAGE_ICON_ERROR)
		return TAECHANG_COLOR_ERROR;
	if (m_nIcon == SAGE_MESSAGE_ICON_WARNING)
		return TAECHANG_COLOR_WARNING;
	return TAECHANG_COLOR_PRIMARY;
}

int CSageMessageBody::GetTextLeft() const {
	if (m_nIcon == SAGE_MESSAGE_ICON_NONE)
		return 0;
	return TAECHANG_MSGBOX_ICON_SIZE + TAECHANG_MSGBOX_ICON_TEXT_GAP;
}

void CSageMessageBody::DrawAlertGlyph(CDC* pDC, const CRect& rectIcon, COLORREF clrIcon) {
	CPoint ptCenter = rectIcon.CenterPoint();
	pDC->MoveTo(ptCenter.x, rectIcon.top + TAECHANG_MSGBOX_ALERT_STEM_TOP);
	pDC->LineTo(ptCenter.x, rectIcon.top + TAECHANG_MSGBOX_ALERT_STEM_BOTTOM);
	pDC->FillSolidRect(ptCenter.x - TAECHANG_MSGBOX_ICON_DOT_SIZE / 2,
		rectIcon.top + TAECHANG_MSGBOX_ALERT_DOT_TOP,
		TAECHANG_MSGBOX_ICON_DOT_SIZE, TAECHANG_MSGBOX_ICON_DOT_SIZE, clrIcon);
}

void CSageMessageBody::DrawInfoGlyph(CDC* pDC, const CRect& rectIcon, COLORREF clrIcon) {
	CPoint ptCenter = rectIcon.CenterPoint();
	pDC->FillSolidRect(ptCenter.x - TAECHANG_MSGBOX_ICON_DOT_SIZE / 2,
		rectIcon.top + TAECHANG_MSGBOX_INFO_DOT_TOP,
		TAECHANG_MSGBOX_ICON_DOT_SIZE, TAECHANG_MSGBOX_ICON_DOT_SIZE, clrIcon);
	pDC->MoveTo(ptCenter.x, rectIcon.top + TAECHANG_MSGBOX_INFO_STEM_TOP);
	pDC->LineTo(ptCenter.x, rectIcon.top + TAECHANG_MSGBOX_INFO_STEM_BOTTOM);
}

void CSageMessageBody::DrawMessageIcon(CDC* pDC, const CRect& rectIcon) {
	COLORREF clrIcon = GetIconColor();
	CPen penIcon(PS_SOLID, TAECHANG_ICON_STROKE, clrIcon);
	CPen* pOldPen = pDC->SelectObject(&penIcon);
	CGdiObject* pOldBrush = pDC->SelectStockObject(NULL_BRUSH);

	CPoint ptCenter = rectIcon.CenterPoint();
	pDC->Ellipse(ptCenter.x - TAECHANG_MSGBOX_ICON_RADIUS, ptCenter.y - TAECHANG_MSGBOX_ICON_RADIUS,
		ptCenter.x + TAECHANG_MSGBOX_ICON_RADIUS, ptCenter.y + TAECHANG_MSGBOX_ICON_RADIUS);

	if (m_nIcon == SAGE_MESSAGE_ICON_INFO)
		DrawInfoGlyph(pDC, rectIcon, clrIcon);
	else
		DrawAlertGlyph(pDC, rectIcon, clrIcon);

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

void CSageMessageBody::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);

	pDC->FillSolidRect(rectClient, SageUiResources::GetBackgroundColor(SAGE_BG_PANEL));

	if (m_strMessage.IsEmpty())
		return;

	if (m_nIcon != SAGE_MESSAGE_ICON_NONE) {
		CRect rectIcon(rectClient.left, rectClient.top,
			rectClient.left + TAECHANG_MSGBOX_ICON_SIZE,
			rectClient.top + TAECHANG_MSGBOX_ICON_SIZE);
		DrawMessageIcon(pDC, rectIcon);
	}

	CRect rectText(rectClient);
	rectText.left += GetTextLeft();

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CONTENT));

	CRect rectCalc(rectText);
	rectCalc.bottom = rectCalc.top;
	pDC->DrawText(m_strMessage, &rectCalc, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
	int nTextOffset = (rectText.Height() - rectCalc.Height()) / 2;
	if (nTextOffset > 0)
		rectText.top += nTextOffset;

	pDC->DrawText(m_strMessage, &rectText, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);
	pDC->SelectObject(pOldFont);
}
