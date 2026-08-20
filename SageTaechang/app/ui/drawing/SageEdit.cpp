#include "pch.h"
#include "app/ui/drawing/SageEdit.h"
#include "SageDefine.h"

#include <uxtheme.h>

BEGIN_MESSAGE_MAP(CSageEdit, CEdit)
	ON_WM_CREATE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
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

void CSageEdit::OnNcPaint() {
	CRect rectFrame;
	GetWindowRect(&rectFrame);
	rectFrame.OffsetRect(-rectFrame.left, -rectFrame.top);

	CWindowDC dc(this);
	CBrush brushFrame(m_nState == SAGE_EDIT_ERROR
		? SAGE_COLOR_ERROR : SAGE_COLOR_BORDER);
	dc.FrameRect(rectFrame, &brushFrame);
}
