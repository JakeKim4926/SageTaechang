#include "pch.h"
#include "app/ui/drawing/SageListCtrl.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageListCtrl, CListCtrl)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CSageListCtrl::OnNMCustomDraw)
	ON_NOTIFY_REFLECT_EX(LVN_ITEMCHANGED, &CSageListCtrl::OnSelectionChanged)
END_MESSAGE_MAP()

BOOL CSageListCtrl::OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	NMLISTVIEW* pNM = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	*pResult = 0;
	if ((pNM->uChanged & LVIF_STATE) != 0
		&& ((pNM->uOldState ^ pNM->uNewState) & LVIS_SELECTED) != 0)
		InvalidateItemRow(pNM->iItem);
	return FALSE;
}

void CSageListCtrl::InvalidateItemRow(int nItem) {
	if (nItem == TAECHANG_LIST_NO_ITEM) {
		Invalidate();
		return;
	}
	CRect rcItem;
	if (!GetItemRect(nItem, rcItem, LVIR_BOUNDS))
		return;
	InvalidateRect(rcItem);
}

int CSageListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	ApplyFixedRowHeight();
	return 0;
}

CSageListCtrl::CSageListCtrl()
	: m_bAlternateRow(FALSE)
	, m_nFirstColumnAlign(SAGE_LIST_FIRST_COLUMN_DEFAULT)
	, m_nHighlightFirst(0)
	, m_nHighlightCount(0)
	, m_bRowSeparator(FALSE) {
}

void CSageListCtrl::ApplyFixedRowHeight() {
	if (m_imgRowSpacer.GetSafeHandle() != NULL)
		return;
	if (!m_imgRowSpacer.Create(TAECHANG_LIST_ROW_SPACER_WIDTH, TAECHANG_LIST_ROW_HEIGHT, ILC_COLOR32, 1, 1))
		return;
	SetImageList(&m_imgRowSpacer, LVSIL_SMALL);
}

void CSageListCtrl::SetRowSeparator(BOOL bEnable) {
	m_bRowSeparator = bEnable;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageListCtrl::DrawRowSeparator(int nItem, NMLVCUSTOMDRAW* pCD) {
	CRect rcItem;
	if (!GetItemRect(nItem, rcItem, LVIR_BOUNDS))
		return;
	CDC* pDC = CDC::FromHandle(pCD->nmcd.hdc);
	pDC->FillSolidRect(rcItem.left, rcItem.bottom - TAECHANG_LIST_GRID_THICKNESS,
		rcItem.Width(), TAECHANG_LIST_GRID_THICKNESS, TAECHANG_COLOR_LIST_GRID);
}

void CSageListCtrl::DrawSelectionAccent(int nItem, NMLVCUSTOMDRAW* pCD) {
	CRect rcItem;
	if (!GetItemRect(nItem, rcItem, LVIR_BOUNDS))
		return;
	CDC* pDC = CDC::FromHandle(pCD->nmcd.hdc);
	pDC->FillSolidRect(rcItem.left, rcItem.top,
		TAECHANG_SELECTION_ACCENT_WIDTH, rcItem.Height(), TAECHANG_COLOR_PRIMARY);
}

void CSageListCtrl::SetAlternateRowColor(BOOL bEnable) {
	m_bAlternateRow = bEnable;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageListCtrl::SetFirstColumnAlign(SageListFirstColumnAlign nAlign) {
	m_nFirstColumnAlign = nAlign;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageListCtrl::SetHighlightColumns(int nFirst, int nCount) {
	m_nHighlightFirst = nFirst;
	m_nHighlightCount = nCount;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

BOOL CSageListCtrl::IsHighlightColumn(int nSubItem) const {
	if (m_nHighlightCount <= 0)
		return FALSE;
	return (nSubItem >= m_nHighlightFirst && nSubItem < m_nHighlightFirst + m_nHighlightCount) ? TRUE : FALSE;
}

COLORREF CSageListCtrl::GetRowBackColor(int nItem) const {
	return (nItem % 2 == 1) ? TAECHANG_COLOR_LIST_ROW_ALT : TAECHANG_COLOR_PANEL;
}

void CSageListCtrl::DrawFirstColumn(int nItem, BOOL bSelected, NMLVCUSTOMDRAW* pCD) {
	CDC* pDC = CDC::FromHandle(pCD->nmcd.hdc);
	COLORREF clrBk = bSelected ? TAECHANG_COLOR_LIST_ROW_SELECTED : GetRowBackColor(nItem);

	CRect rcItem;
	GetSubItemRect(nItem, 0, LVIR_LABEL, rcItem);
	pDC->FillSolidRect(&rcItem, clrBk);

	UINT nAlignFormat = DT_CENTER;
	if (m_nFirstColumnAlign == SAGE_LIST_FIRST_COLUMN_RIGHT) {
		nAlignFormat = DT_RIGHT;
		rcItem.right -= TAECHANG_LIST_CELL_RIGHT_PAD;
	}

	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(GetItemText(nItem, 0), &rcItem,
		nAlignFormat | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void CSageListCtrl::OnNMCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) {
	NMLVCUSTOMDRAW* pCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;

	switch (pCD->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;

		case CDDS_ITEMPREPAINT:
		{
			int nItem = static_cast<int>(pCD->nmcd.dwItemSpec);
			BOOL bSelected = (GetItemState(nItem, LVIS_SELECTED) & LVIS_SELECTED) != 0;
			if (m_bRowSeparator || bSelected)
				*pResult |= CDRF_NOTIFYPOSTPAINT;
			if (bSelected) {
				pCD->nmcd.uItemState &= ~(CDIS_SELECTED | CDIS_FOCUS);
				pCD->clrTextBk = TAECHANG_COLOR_LIST_ROW_SELECTED;
				pCD->clrText = TAECHANG_COLOR_TEXT;
				*pResult |= CDRF_NEWFONT;
			} else if (m_bAlternateRow) {
				pCD->clrTextBk = GetRowBackColor(nItem);
				pCD->clrText = TAECHANG_COLOR_TEXT;
				*pResult |= CDRF_NEWFONT;
			}
			if (m_nHighlightCount > 0 || m_nFirstColumnAlign != SAGE_LIST_FIRST_COLUMN_DEFAULT)
				*pResult |= CDRF_NOTIFYSUBITEMDRAW;
			break;
		}

		case CDDS_ITEMPOSTPAINT:
		{
			int nItem = static_cast<int>(pCD->nmcd.dwItemSpec);
			if (m_bRowSeparator)
				DrawRowSeparator(nItem, pCD);
			if ((GetItemState(nItem, LVIS_SELECTED) & LVIS_SELECTED) != 0)
				DrawSelectionAccent(nItem, pCD);
			break;
		}

		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		{
			int nItem = static_cast<int>(pCD->nmcd.dwItemSpec);
			int nSubItem = pCD->iSubItem;
			BOOL bSelected = (GetItemState(nItem, LVIS_SELECTED) & LVIS_SELECTED) != 0;
			if (m_nHighlightCount > 0) {
				BOOL bHighlight = IsHighlightColumn(nSubItem);
				pCD->clrText = bHighlight ? TAECHANG_COLOR_PRIMARY : TAECHANG_COLOR_TEXT;
				CDC* pDC = CDC::FromHandle(pCD->nmcd.hdc);
				pDC->SelectObject(SageUiResources::GetFont(
					bHighlight ? SAGE_FONT_LIST_BOLD : SAGE_FONT_LIST));
				*pResult = CDRF_NEWFONT;
			}
			if (m_nFirstColumnAlign != SAGE_LIST_FIRST_COLUMN_DEFAULT && nSubItem == 0) {
				DrawFirstColumn(nItem, bSelected, pCD);
				*pResult = CDRF_SKIPDEFAULT;
			}
			break;
		}
	}
}
