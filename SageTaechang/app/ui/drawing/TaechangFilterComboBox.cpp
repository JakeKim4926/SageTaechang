#include "pch.h"
#include "app/ui/drawing/TaechangFilterComboBox.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CTaechangFilterComboBox, CComboBox)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CTaechangFilterComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) {
	lpMeasureItemStruct->itemHeight = TAECHANG_RESULT_CRITERIA_ITEM_HEIGHT;
}

void CTaechangFilterComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	if (static_cast<int>(lpDrawItemStruct->itemID) < 0)
		return;
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rcItem = lpDrawItemStruct->rcItem;
	BOOL bSelected = (lpDrawItemStruct->itemState & ODS_SELECTED) ? TRUE : FALSE;
	pDC->FillSolidRect(rcItem, bSelected ? TAECHANG_COLOR_PRIMARY : TAECHANG_COLOR_PANEL);
	CString strText;
	GetLBText(lpDrawItemStruct->itemID, strText);
	CFont* pFont = GetFont();
	CFont* pOldFont = pFont ? pDC->SelectObject(pFont) : NULL;
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(bSelected ? TAECHANG_COLOR_PANEL : TAECHANG_COLOR_TEXT);
	pDC->DrawText(strText, rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	if (pOldFont)
		pDC->SelectObject(pOldFont);
}

void CTaechangFilterComboBox::OnPaint() {
	CPaintDC dc(this);
	CRect rcClient;
	GetClientRect(&rcClient);
	dc.FillSolidRect(rcClient, TAECHANG_COLOR_PANEL);

	COMBOBOXINFO cbi = {};
	cbi.cbSize = sizeof(COMBOBOXINFO);
	GetComboBoxInfo(&cbi);
	CRect rcButton = cbi.rcButton;
	if (!rcButton.IsRectEmpty()) {
		int cx = (rcButton.left + rcButton.right) / 2;
		int cy = (rcButton.top + rcButton.bottom) / 2;
		POINT pts[3] = {
			{ cx - 4, cy - 2 },
			{ cx + 4, cy - 2 },
			{ cx,     cy + 3 }
		};
		CBrush br(TAECHANG_COLOR_PRIMARY);
		CPen pen(PS_NULL, 0, RGB(0, 0, 0));
		CBrush* pOldBr = dc.SelectObject(&br);
		CPen* pOldPen = dc.SelectObject(&pen);
		dc.Polygon(pts, 3);
		dc.SelectObject(pOldBr);
		dc.SelectObject(pOldPen);
	}

	int nSel = GetCurSel();
	if (nSel != CB_ERR) {
		CString strText;
		GetLBText(nSel, strText);
		CRect rcText = rcClient;
		if (!rcButton.IsRectEmpty())
			rcText.right = rcButton.left;
		CFont* pFont = GetFont();
		CFont* pOldFont = pFont ? dc.SelectObject(pFont) : NULL;
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(TAECHANG_COLOR_TEXT);
		dc.DrawText(strText, rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		if (pOldFont)
			dc.SelectObject(pOldFont);
	}
}
