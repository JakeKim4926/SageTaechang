#include "pch.h"
#include "app/ui/drawing/SageListCtrl.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CSageListCtrl::OnNMCustomDraw)
END_MESSAGE_MAP()

CSageListCtrl::CSageListCtrl()
	: m_bAlternateRow(FALSE)
	, m_bCenterFirstColumn(FALSE)
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

void CSageListCtrl::SetAlternateRowColor(BOOL bEnable) {
	m_bAlternateRow = bEnable;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageListCtrl::SetCenterFirstColumn(BOOL bEnable) {
	m_bCenterFirstColumn = bEnable;
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

void CSageListCtrl::DrawCenteredFirstColumn(int nItem, BOOL bSelected, NMLVCUSTOMDRAW* pCD) {
	CDC* pDC = CDC::FromHandle(pCD->nmcd.hdc);
	COLORREF clrBk = bSelected ? ::GetSysColor(COLOR_HIGHLIGHT) : GetRowBackColor(nItem);

	CRect rcItem;
	GetSubItemRect(nItem, 0, LVIR_LABEL, rcItem);
	pDC->FillSolidRect(&rcItem, clrBk);

	wchar_t szText[64] = {};
	LVITEM lvi = {};
	lvi.mask = LVIF_TEXT;
	lvi.iItem = nItem;
	lvi.iSubItem = 0;
	lvi.pszText = szText;
	lvi.cchTextMax = 63;
	GetItem(&lvi);

	pDC->SetTextColor(bSelected ? ::GetSysColor(COLOR_HIGHLIGHTTEXT) : TAECHANG_COLOR_TEXT);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(szText, -1, &rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void CSageListCtrl::OnNMCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) {
	NMLVCUSTOMDRAW* pCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;

	switch (pCD->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			ApplyFixedRowHeight();
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;

		case CDDS_ITEMPREPAINT:
		{
			int nItem = static_cast<int>(pCD->nmcd.dwItemSpec);
			BOOL bSelected = (GetItemState(nItem, LVIS_SELECTED) & LVIS_SELECTED) != 0;
			if (m_bRowSeparator)
				*pResult |= CDRF_NOTIFYPOSTPAINT;
			if (bSelected)
				break;
			if (m_bAlternateRow) {
				pCD->clrTextBk = GetRowBackColor(nItem);
				pCD->clrText = TAECHANG_COLOR_TEXT;
				*pResult |= CDRF_NEWFONT;
			}
			if (m_nHighlightCount > 0 || m_bCenterFirstColumn)
				*pResult |= CDRF_NOTIFYSUBITEMDRAW;
			break;
		}

		case CDDS_ITEMPOSTPAINT:
		{
			if (!m_bRowSeparator)
				break;
			DrawRowSeparator(static_cast<int>(pCD->nmcd.dwItemSpec), pCD);
			break;
		}

		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		{
			int nItem = static_cast<int>(pCD->nmcd.dwItemSpec);
			int nSubItem = pCD->iSubItem;
			BOOL bSelected = (GetItemState(nItem, LVIS_SELECTED) & LVIS_SELECTED) != 0;
			if (bSelected)
				break;
			if (m_nHighlightCount > 0) {
				pCD->clrText = IsHighlightColumn(nSubItem) ? TAECHANG_COLOR_PRIMARY : TAECHANG_COLOR_TEXT;
				*pResult = CDRF_NEWFONT;
			}
			if (m_bCenterFirstColumn && nSubItem == 0) {
				DrawCenteredFirstColumn(nItem, bSelected, pCD);
				*pResult = CDRF_SKIPDEFAULT;
			}
			break;
		}
	}
}
