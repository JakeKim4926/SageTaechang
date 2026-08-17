#include "pch.h"
#include "app/ui/drawing/SageOptionCheck.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/ui/drawing/SageUiStyle.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageOptionCheck, CButton)
	ON_CONTROL_REFLECT_EX(BN_CLICKED, &CSageOptionCheck::OnClicked)
END_MESSAGE_MAP()

CSageOptionCheck::CSageOptionCheck()
	: m_bChecked(FALSE), m_bFrameVisible(TRUE) {
}

void CSageOptionCheck::SetFrameVisible(BOOL bVisible) {
	m_bFrameVisible = bVisible;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

int CSageOptionCheck::GetSidePadding() const {
	return m_bFrameVisible ? TAECHANG_OPTION_CHECK_PADDING : 0;
}

void CSageOptionCheck::SetHint(LPCWSTR pszHint) {
	m_strHint = pszHint;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageOptionCheck::SetChecked(BOOL bChecked) {
	m_bChecked = bChecked;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

BOOL CSageOptionCheck::IsChecked() const {
	return m_bChecked;
}

int CSageOptionCheck::GetContentWidth() const {
	if (!::IsWindow(GetSafeHwnd()))
		return 0;

	CString strLabel;
	GetWindowTextW(strLabel);
	CClientDC dc(const_cast<CSageOptionCheck*>(this));

	CFont* pOldFont = dc.SelectObject(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	int nWidth = GetSidePadding() + TAECHANG_LIST_CHECK_BOX_SIZE
		+ TAECHANG_ICON_TEXT_GAP + dc.GetTextExtent(strLabel).cx;
	if (!m_strHint.IsEmpty()) {
		dc.SelectObject(SageUiResources::GetFont(SAGE_FONT_CAPTION));
		nWidth += TAECHANG_ICON_TEXT_GAP + dc.GetTextExtent(m_strHint).cx;
	}
	dc.SelectObject(pOldFont);
	return nWidth + GetSidePadding();
}

BOOL CSageOptionCheck::OnClicked() {
	SetChecked(m_bChecked ? FALSE : TRUE);
	return FALSE;
}

void CSageOptionCheck::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);
	BOOL bDisabled = (lpDrawItemStruct->itemState & ODS_DISABLED) ? TRUE : FALSE;

	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_PANEL);
	if (m_bFrameVisible) {
		CBrush brushBorder(TAECHANG_COLOR_BUTTON_BORDER);
		pDC->FrameRect(rectClient, &brushBorder);
	}

	CRect rectBox(0, 0, TAECHANG_LIST_CHECK_BOX_SIZE, TAECHANG_LIST_CHECK_BOX_SIZE);
	rectBox.OffsetRect(
		rectClient.left + GetSidePadding(),
		rectClient.top + (rectClient.Height() - TAECHANG_LIST_CHECK_BOX_SIZE) / 2);
	SageUiStyle::DrawCheckBox(*pDC, rectBox, m_bChecked);

	CString strLabel;
	GetWindowTextW(strLabel);

	pDC->SetBkMode(TRANSPARENT);
	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	pDC->SetTextColor(bDisabled ? TAECHANG_COLOR_TEXT_PLACEHOLDER : TAECHANG_COLOR_TEXT);
	CSize sizeLabel = pDC->GetTextExtent(strLabel);
	CRect rectLabel(
		rectBox.right + TAECHANG_ICON_TEXT_GAP, rectClient.top,
		rectBox.right + TAECHANG_ICON_TEXT_GAP + sizeLabel.cx, rectClient.bottom);
	pDC->DrawText(strLabel, &rectLabel, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pDC->SelectObject(pOldFont);

	if (m_strHint.IsEmpty())
		return;

	pOldFont = pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CAPTION));
	pDC->SetTextColor(TAECHANG_COLOR_SECONDARY_TEXT);
	CRect rectHint(
		rectLabel.right + TAECHANG_ICON_TEXT_GAP, rectClient.top,
		rectClient.right - GetSidePadding(), rectClient.bottom);
	pDC->DrawText(m_strHint, &rectHint, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pDC->SelectObject(pOldFont);
}
