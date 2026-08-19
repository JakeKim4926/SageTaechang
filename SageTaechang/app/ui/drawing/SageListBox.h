#pragma once

class CSageListBox : public CListBox
{
	DECLARE_MESSAGE_MAP()

public:
	CSageListBox();

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnNcPaint();

private:
	COLORREF GetRowColor(int nIndex, BOOL bSelected) const;
};
