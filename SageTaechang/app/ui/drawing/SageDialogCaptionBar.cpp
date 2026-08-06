#include "pch.h"
#include "app/ui/drawing/SageDialogCaptionBar.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageDialogCaptionBar, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(ID_TAECHANG_DLG_CLOSE, &CSageDialogCaptionBar::OnCloseClicked)
END_MESSAGE_MAP()

CSageDialogCaptionBar::CSageDialogCaptionBar()
	: m_nCloseCommandId(0) {
}

BOOL CSageDialogCaptionBar::Create(CWnd* pParent, LPCWSTR pszTitle, UINT nCloseCommandId) {
	m_strTitle = pszTitle;
	m_nCloseCommandId = nCloseCommandId;

	CRect rectEmpty(0, 0, 0, 0);
	if (CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		rectEmpty, pParent, 0) == FALSE)
		return FALSE;

	m_wndCloseBtn.Create(L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_DLG_CLOSE);
	m_wndCloseBtn.SetVariant(SAGE_BUTTON_GHOST);
	m_wndCloseBtn.SetIcon(SAGE_BUTTON_ICON_CLOSE);
	m_wndCloseBtn.SetTooltip(TAECHANG_UI_TIP_CLOSE);
	return TRUE;
}

void CSageDialogCaptionBar::Layout(int nWidth) {
	MoveWindow(0, 0, nWidth, TAECHANG_DLG_CAPTION_HEIGHT);

	int nBtnTop = (TAECHANG_DLG_CAPTION_HEIGHT - TAECHANG_DLG_CAPTION_BTN_SIZE) / 2;
	m_wndCloseBtn.MoveWindow(nWidth - TAECHANG_DLG_CAPTION_BTN_PAD - TAECHANG_DLG_CAPTION_BTN_SIZE,
		nBtnTop, TAECHANG_DLG_CAPTION_BTN_SIZE, TAECHANG_DLG_CAPTION_BTN_SIZE);
}

BOOL CSageDialogCaptionBar::OnEraseBkgnd(CDC* pDC) {
	return TRUE;
}

void CSageDialogCaptionBar::OnPaint() {
	CPaintDC dc(this);

	CRect rectClient;
	GetClientRect(&rectClient);
	dc.FillSolidRect(rectClient, TAECHANG_COLOR_LIST_HEADER);
	dc.FillSolidRect(rectClient.left, rectClient.bottom - TAECHANG_BORDER_THICKNESS,
		rectClient.Width(), TAECHANG_BORDER_THICKNESS, TAECHANG_COLOR_BORDER);

	CRect rectText = rectClient;
	rectText.left += TAECHANG_DLG_CAPTION_PAD;
	rectText.right -= TAECHANG_DLG_CAPTION_BTN_PAD + TAECHANG_DLG_CAPTION_BTN_SIZE;

	CFont* pOldFont = dc.SelectObject(SageUiResources::GetFont(SAGE_FONT_CONTENT_SEMIBOLD));
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(TAECHANG_COLOR_TEXT);
	dc.DrawText(m_strTitle, rectText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	if (pOldFont)
		dc.SelectObject(pOldFont);
}

LRESULT CSageDialogCaptionBar::OnNcHitTest(CPoint point) {
	return HTTRANSPARENT;
}

void CSageDialogCaptionBar::OnCloseClicked() {
	GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(m_nCloseCommandId, BN_CLICKED), 0);
}
