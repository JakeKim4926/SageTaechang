#pragma once

enum SageListFirstColumnAlign
{
	SAGE_LIST_FIRST_COLUMN_DEFAULT,
	SAGE_LIST_FIRST_COLUMN_CENTER,
	SAGE_LIST_FIRST_COLUMN_RIGHT
};

class CSageListCtrl : public CListCtrl
{
	DECLARE_MESSAGE_MAP()

public:
	CSageListCtrl();

	void SetAlternateRowColor(BOOL bEnable);
	void SetFirstColumnAlign(SageListFirstColumnAlign nAlign);
	void SetHighlightColumns(int nFirst, int nCount);
	void SetRowSeparator(BOOL bEnable);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNMCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);

private:
	BOOL IsHighlightColumn(int nSubItem) const;
	COLORREF GetRowBackColor(int nItem) const;
	void DrawFirstColumn(int nItem, BOOL bSelected, NMLVCUSTOMDRAW* pCD);
	COLORREF ResolveSubItemTextColor(int nItem, int nSubItem, BOOL bHighlight) const;
	void ApplyFixedRowHeight();
	void DrawRowSeparator(int nItem, NMLVCUSTOMDRAW* pCD);
	void DrawSelectionAccent(int nItem, NMLVCUSTOMDRAW* pCD);
	void InvalidateItemRow(int nItem);

private:
	BOOL m_bAlternateRow;
	SageListFirstColumnAlign m_nFirstColumnAlign;
	int m_nHighlightFirst;
	int m_nHighlightCount;
	BOOL m_bRowSeparator;
	CImageList m_imgRowSpacer;
};
