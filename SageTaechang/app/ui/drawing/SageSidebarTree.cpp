#include "pch.h"
#include "app/ui/drawing/SageSidebarTree.h"
#include "app/ui/drawing/SageUiResources.h"
#include "SageDefine.h"

BEGIN_MESSAGE_MAP(CSageSidebarTree, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CSageSidebarTree::OnNMCustomDraw)
END_MESSAGE_MAP()

void CSageSidebarTree::DrawTreeItem(HTREEITEM hItem, BOOL bSelected, NMTVCUSTOMDRAW* pCD) {
	CDC* pDC = CDC::FromHandle(pCD->nmcd.hdc);
	BOOL bGroupHeader = (GetParentItem(hItem) == NULL) ? TRUE : FALSE;

	CRect rcRow;
	if (!GetItemRect(hItem, &rcRow, FALSE))
		return;

	CRect rcClient;
	GetClientRect(&rcClient);
	rcRow.left = rcClient.left;
	rcRow.right = rcClient.right;

	BOOL bHighlight = (!bGroupHeader && bSelected) ? TRUE : FALSE;
	pDC->FillSolidRect(rcRow,
		bHighlight ? SAGE_COLOR_SIDEBAR_SELECTED : SAGE_COLOR_SIDEBAR);
	if (bHighlight) {
		pDC->FillSolidRect(
			rcRow.left, rcRow.top, SAGE_SELECTION_ACCENT_WIDTH, rcRow.Height(),
			SAGE_COLOR_PRIMARY);
	}

	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(
		bGroupHeader ? SAGE_FONT_CAPTION
		: (bSelected ? SAGE_FONT_CONTENT_SEMIBOLD : SAGE_FONT_CONTROL)));
	int nOldBkMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF clrOldText = pDC->SetTextColor(bGroupHeader
		? SAGE_COLOR_SIDEBAR_CATEGORY
		: (bSelected ? SAGE_COLOR_SIDEBAR_SELECTED_TEXT : SAGE_COLOR_SIDEBAR_TEXT));
	int nOldCharExtra = pDC->SetTextCharacterExtra(
		bGroupHeader ? SAGE_SIDEBAR_CATEGORY_CHAR_EXTRA : 0);

	CRect rcText(rcRow);
	rcText.left += SAGE_SIDEBAR_PAD_X;
	pDC->DrawText(GetItemText(hItem), &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	pDC->SetTextCharacterExtra(nOldCharExtra);
	pDC->SetTextColor(clrOldText);
	pDC->SetBkMode(nOldBkMode);
	if (pOldFont)
		pDC->SelectObject(pOldFont);
}

void CSageSidebarTree::OnNMCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) {
	NMTVCUSTOMDRAW* pCD = reinterpret_cast<NMTVCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;
	switch (pCD->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
		{
			HTREEITEM hItem = reinterpret_cast<HTREEITEM>(pCD->nmcd.dwItemSpec);
			DrawTreeItem(hItem, (pCD->nmcd.uItemState & CDIS_SELECTED) != 0, pCD);
			*pResult = CDRF_SKIPDEFAULT;
			break;
		}
	}
}
