#include "pch.h"
#include "app/ui/drawing/SageListBox.h"
#include "app/ui/drawing/SageUiResources.h"
#include "SageDefine.h"

#include <uxtheme.h>

BEGIN_MESSAGE_MAP(CSageListBox, CListBox)
	ON_WM_CREATE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
END_MESSAGE_MAP()

CSageListBox::CSageListBox() {
}

int CSageListBox::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetWindowTheme(GetSafeHwnd(), L"", L"");
	ModifyStyle(WS_BORDER, 0, SWP_FRAMECHANGED);
	SetItemHeight(0, SAGE_LIST_BOX_ROW_HEIGHT);
	return 0;
}

void CSageListBox::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp) {
	CListBox::OnNcCalcSize(bCalcValidRects, lpncsp);
	::InflateRect(&lpncsp->rgrc[0],
		-SAGE_BORDER_THICKNESS, -SAGE_BORDER_THICKNESS);
}

void CSageListBox::OnNcPaint() {
	Default();

	CRect rectFrame;
	GetWindowRect(&rectFrame);
	rectFrame.OffsetRect(-rectFrame.left, -rectFrame.top);

	CWindowDC dc(this);
	CBrush brushFrame(SAGE_COLOR_BORDER);
	dc.FrameRect(rectFrame, &brushFrame);
}

void CSageListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) {
	lpMeasureItemStruct->itemHeight = SAGE_LIST_BOX_ROW_HEIGHT;
}

COLORREF CSageListBox::GetRowColor(int nIndex, BOOL bSelected) const {
	if (bSelected)
		return SAGE_COLOR_LIST_ROW_SELECTED;
	return (nIndex % 2 == 0) ? SAGE_COLOR_PANEL : SAGE_COLOR_LIST_ROW_ALT;
}

void CSageListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	if (lpDrawItemStruct->itemID == (UINT)LB_ERR)
		return;

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectItem(lpDrawItemStruct->rcItem);
	int nIndex = (int)lpDrawItemStruct->itemID;
	BOOL bSelected = (lpDrawItemStruct->itemState & ODS_SELECTED) ? TRUE : FALSE;

	pDC->FillSolidRect(rectItem, GetRowColor(nIndex, bSelected));
	pDC->FillSolidRect(rectItem.left, rectItem.bottom - SAGE_LIST_GRID_THICKNESS,
		rectItem.Width(), SAGE_LIST_GRID_THICKNESS, SAGE_COLOR_LIST_GRID);

	if (bSelected)
		pDC->FillSolidRect(rectItem.left, rectItem.top,
			SAGE_LIST_SELECTION_ACCENT_WIDTH, rectItem.Height(), SAGE_COLOR_PRIMARY);

	CString strText;
	GetText(nIndex, strText);

	CRect rectText(rectItem);
	rectText.left += SAGE_LIST_BOX_TEXT_PAD_X;
	rectText.right -= SAGE_LIST_BOX_TEXT_PAD_X;

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(bSelected ? SAGE_COLOR_PRIMARY_PRESS : SAGE_COLOR_TEXT);
	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(
		bSelected ? SAGE_FONT_CONTENT_SEMIBOLD : SAGE_FONT_CONTENT));
	pDC->DrawText(strText, &rectText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	pDC->SelectObject(pOldFont);
}
