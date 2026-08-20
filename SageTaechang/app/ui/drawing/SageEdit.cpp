#include "pch.h"
#include "app/ui/drawing/SageEdit.h"
#include "app/ui/drawing/SageUiResources.h"
#include "SageDefine.h"

#include <uxtheme.h>

IMPLEMENT_DYNAMIC(CSageEdit, CEdit)

BEGIN_MESSAGE_MAP(CSageEdit, CEdit)
	ON_WM_CREATE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

CSageEdit::CSageEdit()
	: m_nState(SAGE_EDIT_NORMAL) {
}

int CSageEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
	SetWindowTheme(GetSafeHwnd(), L"", L"");
	ModifyStyle(WS_BORDER, 0, SWP_FRAMECHANGED);
	return 0;
}

void CSageEdit::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp) {
	CEdit::OnNcCalcSize(bCalcValidRects, lpncsp);
	::InflateRect(&lpncsp->rgrc[0],
		-SAGE_BORDER_THICKNESS, -SAGE_BORDER_THICKNESS);
}

void CSageEdit::SetState(SageEditState nState) {
	if (m_nState == nState)
		return;
	m_nState = nState;
	if (::IsWindow(GetSafeHwnd()))
		SetWindowPos(NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

HBRUSH CSageEdit::CtlColor(CDC* pDC, UINT nCtlColor) {
	if (IsWindowEnabled()) {
		pDC->SetTextColor(SAGE_COLOR_TEXT);
		pDC->SetBkColor(SAGE_COLOR_PANEL);
		return SageUiResources::GetBrush(SAGE_BG_PANEL);
	}

	pDC->SetTextColor(SAGE_COLOR_TEXT_PLACEHOLDER);
	pDC->SetBkColor(SAGE_COLOR_LIST_HEADER);
	return SageUiResources::GetBrush(SAGE_BG_LIST_HEADER);
}

void CSageEdit::OnNcPaint() {
	CRect rectFrame;
	GetWindowRect(&rectFrame);
	rectFrame.OffsetRect(-rectFrame.left, -rectFrame.top);

	CWindowDC dc(this);
	CBrush brushFrame(m_nState == SAGE_EDIT_ERROR
		? SAGE_COLOR_ERROR : SAGE_COLOR_BORDER);
	dc.FrameRect(rectFrame, &brushFrame);
}
