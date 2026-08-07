#include "pch.h"
#include "app/ui/drawing/SageListCtrl.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/ui/drawing/SageUiStyle.h"
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
	, m_nGroupColumn(TAECHANG_LIST_NO_GROUP_COLUMN)
	, m_nHighlightFirst(0)
	, m_nHighlightCount(0)
	, m_bRowSeparator(FALSE)
	, m_nBadgeColumn(TAECHANG_LIST_NO_BADGE_COLUMN)
	, m_clrMutedText(TAECHANG_COLOR_SECONDARY_TEXT) {
}

void CSageListCtrl::SetBadgeColumn(int nColumn) {
	m_nBadgeColumn = nColumn;
}

void CSageListCtrl::SetRowStyle(int nState, const SageListRowStyle& style) {
	if (nState < 0)
		return;
	if (static_cast<int>(m_arrRowStyles.size()) <= nState)
		m_arrRowStyles.resize(nState + 1);
	m_arrRowStyles[nState] = style;
}

const SageListRowStyle* CSageListCtrl::FindRowStyle(int nItem) const {
	if (m_arrRowStyles.empty())
		return NULL;
	int nState = static_cast<int>(GetItemData(nItem));
	if (nState < 0 || nState >= static_cast<int>(m_arrRowStyles.size()))
		return NULL;
	return &m_arrRowStyles[nState];
}

void CSageListCtrl::ApplyFixedRowHeight() {
	if (m_imgRowSpacer.GetSafeHandle() != NULL)
		return;
	if (!m_imgRowSpacer.Create(TAECHANG_LIST_ROW_SPACER_WIDTH, TAECHANG_LIST_ROW_HEIGHT, ILC_COLOR32, 1, 1))
		return;
	SetImageList(&m_imgRowSpacer, LVSIL_SMALL);
}

void CSageListCtrl::DrawCheckBox(CDC* pDC, const CRect& rectImage, BOOL bChecked) {
	CRect rectBox(0, 0, TAECHANG_LIST_CHECK_BOX_SIZE, TAECHANG_LIST_CHECK_BOX_SIZE);
	rectBox.OffsetRect(
		(rectImage.Width() - TAECHANG_LIST_CHECK_BOX_SIZE) / 2,
		(rectImage.Height() - TAECHANG_LIST_CHECK_BOX_SIZE) / 2);
	SageUiStyle::DrawCheckBox(*pDC, rectBox, bChecked);
}

BOOL CSageListCtrl::BuildCheckStateImages(CImageList& imgState) {
	if (!imgState.Create(TAECHANG_LIST_CHECK_IMAGE_WIDTH, TAECHANG_LIST_ROW_HEIGHT,
			ILC_COLOR32 | ILC_MASK, TAECHANG_LIST_CHECK_STATE_COUNT, 1))
		return FALSE;

	CRect rectImage(0, 0, TAECHANG_LIST_CHECK_IMAGE_WIDTH, TAECHANG_LIST_ROW_HEIGHT);
	CClientDC dcScreen(this);
	for (int i = 0; i < TAECHANG_LIST_CHECK_STATE_COUNT; ++i) {
		CDC dcMem;
		CBitmap bmpState;
		if (!dcMem.CreateCompatibleDC(&dcScreen))
			return FALSE;
		if (!bmpState.CreateCompatibleBitmap(&dcScreen, rectImage.Width(), rectImage.Height()))
			return FALSE;

		CBitmap* pOldBitmap = dcMem.SelectObject(&bmpState);
		dcMem.FillSolidRect(rectImage, TAECHANG_COLOR_IMAGE_MASK);
		DrawCheckBox(&dcMem, rectImage, (i == TAECHANG_LIST_CHECK_STATE_CHECKED) ? TRUE : FALSE);
		dcMem.SelectObject(pOldBitmap);
		imgState.Add(&bmpState, TAECHANG_COLOR_IMAGE_MASK);
	}
	return TRUE;
}

void CSageListCtrl::SetCheckboxes(BOOL bEnable) {
	if (!::IsWindow(GetSafeHwnd()))
		return;

	DWORD dwStyle = GetExtendedStyle();
	if (!bEnable) {
		SetExtendedStyle(dwStyle & ~LVS_EX_CHECKBOXES);
		return;
	}

	SetExtendedStyle(dwStyle | LVS_EX_CHECKBOXES);

	CImageList imgState;
	if (!BuildCheckStateImages(imgState))
		return;
	SetImageList(&imgState, LVSIL_STATE);
	imgState.Detach();
	Invalidate();
}

void CSageListCtrl::SetRowSeparator(BOOL bEnable) {
	m_bRowSeparator = bEnable;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageListCtrl::SetMutedText(LPCWSTR pszText, COLORREF clrText) {
	m_strMutedText = pszText;
	m_clrMutedText = clrText;
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

void CSageListCtrl::SetGroupColumn(int nColumn) {
	m_nGroupColumn = nColumn;
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
	const SageListRowStyle* pStyle = FindRowStyle(nItem);
	if (pStyle != NULL && pStyle->clrRowBackground != CLR_NONE)
		return pStyle->clrRowBackground;
	return (nItem % 2 == 1) ? TAECHANG_COLOR_LIST_ROW_ALT : TAECHANG_COLOR_PANEL;
}

void CSageListCtrl::DrawBadgeColumn(int nItem, BOOL bSelected, NMLVCUSTOMDRAW* pCD) {
	CDC* pDC = CDC::FromHandle(pCD->nmcd.hdc);
	CRect rcItem;
	GetSubItemRect(nItem, m_nBadgeColumn, LVIR_LABEL, rcItem);
	pDC->FillSolidRect(&rcItem, bSelected ? TAECHANG_COLOR_LIST_ROW_SELECTED : GetRowBackColor(nItem));

	const SageListRowStyle* pStyle = FindRowStyle(nItem);
	if (pStyle == NULL || pStyle->clrBadgeBackground == CLR_NONE)
		return;

	CString strText = GetItemText(nItem, m_nBadgeColumn);
	if (strText.IsEmpty())
		return;

	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CAPTION));
	CSize sizeText = pDC->GetTextExtent(strText);

	CRect rectBadge(rcItem);
	rectBadge.left += TAECHANG_LIST_CELL_LEFT_PAD;
	rectBadge.right = rectBadge.left + sizeText.cx + TAECHANG_LIST_BADGE_PAD_X * 2;
	rectBadge.top += (rcItem.Height() - TAECHANG_LIST_BADGE_HEIGHT) / 2;
	rectBadge.bottom = rectBadge.top + TAECHANG_LIST_BADGE_HEIGHT;

	CBrush brushBadge(pStyle->clrBadgeBackground);
	CPen penBadge(PS_SOLID, TAECHANG_BORDER_THICKNESS, pStyle->clrBadgeBackground);
	CBrush* pOldBrush = pDC->SelectObject(&brushBadge);
	CPen* pOldPen = pDC->SelectObject(&penBadge);
	pDC->RoundRect(rectBadge,
		CPoint(TAECHANG_LIST_BADGE_RADIUS * 2, TAECHANG_LIST_BADGE_RADIUS * 2));
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	int nOldBkMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF clrOldText = pDC->SetTextColor(pStyle->clrBadgeText);
	pDC->DrawText(strText, &rectBadge, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->SetTextColor(clrOldText);
	pDC->SetBkMode(nOldBkMode);
	pDC->SelectObject(pOldFont);
}

COLORREF CSageListCtrl::ResolveSubItemTextColor(int nItem, int nSubItem, BOOL bHighlight) const {
	CString strText = GetItemText(nItem, nSubItem);
	if (strText == TAECHANG_UI_AMOUNT_EMPTY_MARK)
		return TAECHANG_COLOR_TEXT_PLACEHOLDER;
	if (!m_strMutedText.IsEmpty() && strText == m_strMutedText)
		return m_clrMutedText;
	return bHighlight ? TAECHANG_COLOR_PRIMARY : TAECHANG_COLOR_TEXT;
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

BOOL CSageListCtrl::IsGroupStartRow(int nItem) const {
	if (nItem <= 0)
		return TRUE;
	CString strText = GetItemText(nItem, m_nGroupColumn);
	if (strText == TAECHANG_UI_SEPARATOR_MARK)
		return TRUE;
	return (strText != GetItemText(nItem - 1, m_nGroupColumn)) ? TRUE : FALSE;
}

void CSageListCtrl::DrawGroupColumn(int nItem, BOOL bSelected, NMLVCUSTOMDRAW* pCD) {
	CDC* pDC = CDC::FromHandle(pCD->nmcd.hdc);
	COLORREF clrBk = bSelected ? TAECHANG_COLOR_LIST_ROW_SELECTED : GetRowBackColor(nItem);

	CRect rcItem;
	GetSubItemRect(nItem, m_nGroupColumn, LVIR_LABEL, rcItem);
	pDC->FillSolidRect(&rcItem, clrBk);
	rcItem.left += TAECHANG_LIST_CELL_LEFT_PAD;

	BOOL bGroupStart = IsGroupStartRow(nItem);
	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(
		bGroupStart ? SAGE_FONT_LIST_SEMIBOLD : SAGE_FONT_LIST));
	COLORREF clrOldText = pDC->SetTextColor(
		bGroupStart ? TAECHANG_COLOR_TEXT : TAECHANG_COLOR_TEXT_PLACEHOLDER);
	int nOldBkMode = pDC->SetBkMode(TRANSPARENT);

	pDC->DrawText(bGroupStart ? GetItemText(nItem, m_nGroupColumn) : CString(TAECHANG_UI_REPEAT_MARK),
		&rcItem, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	pDC->SetBkMode(nOldBkMode);
	pDC->SetTextColor(clrOldText);
	pDC->SelectObject(pOldFont);
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
			pCD->nmcd.uItemState &= ~CDIS_FOCUS;
			if (bSelected) {
				pCD->nmcd.uItemState &= ~CDIS_SELECTED;
				pCD->clrTextBk = TAECHANG_COLOR_LIST_ROW_SELECTED;
				pCD->clrText = TAECHANG_COLOR_TEXT;
				*pResult |= CDRF_NEWFONT;
			} else if (m_bAlternateRow) {
				pCD->clrTextBk = GetRowBackColor(nItem);
				pCD->clrText = TAECHANG_COLOR_TEXT;
				*pResult |= CDRF_NEWFONT;
			}
			if (m_nHighlightCount > 0 || m_nFirstColumnAlign != SAGE_LIST_FIRST_COLUMN_DEFAULT
				|| m_nGroupColumn != TAECHANG_LIST_NO_GROUP_COLUMN || !m_strMutedText.IsEmpty()
				|| m_nBadgeColumn != TAECHANG_LIST_NO_BADGE_COLUMN)
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
				pCD->clrText = ResolveSubItemTextColor(nItem, nSubItem, bHighlight);
				CDC* pDC = CDC::FromHandle(pCD->nmcd.hdc);
				pDC->SelectObject(SageUiResources::GetFont(
					bHighlight ? SAGE_FONT_LIST_BOLD : SAGE_FONT_LIST));
				*pResult = CDRF_NEWFONT;
			} else if (!m_strMutedText.IsEmpty()) {
				pCD->clrText = ResolveSubItemTextColor(nItem, nSubItem, FALSE);
				*pResult = CDRF_NEWFONT;
			}
			if (m_nFirstColumnAlign != SAGE_LIST_FIRST_COLUMN_DEFAULT && nSubItem == 0) {
				DrawFirstColumn(nItem, bSelected, pCD);
				*pResult = CDRF_SKIPDEFAULT;
			}
			if (nSubItem == m_nGroupColumn) {
				DrawGroupColumn(nItem, bSelected, pCD);
				*pResult = CDRF_SKIPDEFAULT;
			}
			if (nSubItem == m_nBadgeColumn) {
				DrawBadgeColumn(nItem, bSelected, pCD);
				*pResult = CDRF_SKIPDEFAULT;
			}
			break;
		}
	}
}
