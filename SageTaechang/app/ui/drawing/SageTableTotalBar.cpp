#include "pch.h"
#include "app/ui/drawing/SageTableTotalBar.h"
#include "SageDefine.h"

void CSageTableTotalBar::SetCells(const std::vector<SageTableTotalBarCell>& arrCells) {
	m_arrCells = arrCells;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

SageFontRole CSageTableTotalBar::GetCellFont(SageTotalBarCellStyle nStyle) const {
	return (nStyle == SAGE_TOTAL_BAR_COUNT) ? SAGE_FONT_LIST_SEMIBOLD : SAGE_FONT_LIST_BOLD;
}

COLORREF CSageTableTotalBar::GetCellColor(SageTotalBarCellStyle nStyle) const {
	switch (nStyle) {
		case SAGE_TOTAL_BAR_LABEL:            return SAGE_COLOR_TEXT_MUTED;
		case SAGE_TOTAL_BAR_COUNT:            return SAGE_COLOR_SECONDARY_TEXT;
		case SAGE_TOTAL_BAR_AMOUNT_HIGHLIGHT: return SAGE_COLOR_PRIMARY;
		default:                              return SAGE_COLOR_TEXT;
	}
}

void CSageTableTotalBar::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);

	pDC->FillSolidRect(rectClient, SAGE_COLOR_LIST_HEADER);
	pDC->FillSolidRect(
		rectClient.left, rectClient.top, rectClient.Width(), SAGE_BORDER_THICKNESS, SAGE_COLOR_BORDER);

	pDC->SetBkMode(TRANSPARENT);
	for (int i = 0; i < static_cast<int>(m_arrCells.size()); ++i) {
		const SageTableTotalBarCell& cell = m_arrCells[i];
		if (cell.strText.IsEmpty() || cell.nWidth <= 0)
			continue;

		CRect rectCell(cell.nLeft, rectClient.top, cell.nLeft + cell.nWidth, rectClient.bottom);
		UINT nFormat = DT_VCENTER | DT_SINGLELINE;
		if (cell.nAlign == SAGE_COLUMN_ALIGN_CENTER) {
			nFormat |= DT_CENTER;
		}
		else if (cell.nAlign == SAGE_COLUMN_ALIGN_RIGHT) {
			rectCell.right -= SAGE_LIST_CELL_RIGHT_PAD;
			nFormat |= DT_RIGHT;
		}
		else {
			rectCell.left += SAGE_LIST_CELL_LEFT_PAD;
			nFormat |= DT_LEFT;
		}

		CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(GetCellFont(cell.nStyle)));
		pDC->SetTextColor(GetCellColor(cell.nStyle));
		pDC->DrawText(cell.strText, &rectCell, nFormat);
		pDC->SelectObject(pOldFont);
	}
}
