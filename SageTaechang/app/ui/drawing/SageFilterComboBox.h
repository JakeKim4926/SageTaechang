#pragma once

class CSageFilterComboBox : public CComboBox
{
	DECLARE_MESSAGE_MAP()
public:
	CSageFilterComboBox();

	void SetFieldColor(COLORREF clrField);
	void SetFieldHeight(int nFieldHeight);

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
protected:
	afx_msg void OnPaint();

private:
	COLORREF m_clrField;
	int m_nFieldHeight;
};
