#include "pch.h"
#include "app/ui/drawing/SageSidebarTree.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageSidebarTree, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CSageSidebarTree::OnNMCustomDraw)
END_MESSAGE_MAP()

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
			BOOL bIsGroupHeader = (GetParentItem(hItem) == NULL);
			BOOL bIsSelected = (pCD->nmcd.uItemState & CDIS_SELECTED) != 0;
			if (bIsGroupHeader) {
				pCD->clrText = TAECHANG_COLOR_SIDEBAR_CATEGORY;
				pCD->clrTextBk = TAECHANG_COLOR_SIDEBAR;
				*pResult = CDRF_NEWFONT;
			} else {
				pCD->clrText = TAECHANG_COLOR_SIDEBAR_TEXT;
				pCD->clrTextBk = bIsSelected ? TAECHANG_COLOR_SIDEBAR_SELECTED : TAECHANG_COLOR_SIDEBAR;
				*pResult = CDRF_NEWFONT;
				if (bIsSelected)
					*pResult |= CDRF_NOTIFYPOSTPAINT;
			}
			break;
		}
		case CDDS_ITEMPOSTPAINT:
		{
			if (pCD->nmcd.uItemState & CDIS_SELECTED) {
				CDC* pItemDC = CDC::FromHandle(pCD->nmcd.hdc);
				CRect rcItem(pCD->nmcd.rc);
				pItemDC->FillSolidRect(rcItem.left, rcItem.top, TAECHANG_SIDEBAR_ACCENT_WIDTH, rcItem.Height(), TAECHANG_COLOR_PRIMARY);
			}
			break;
		}
	}
}
