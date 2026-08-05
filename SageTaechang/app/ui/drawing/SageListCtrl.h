#pragma once

class CSageListCtrl : public CListCtrl
{
	DECLARE_MESSAGE_MAP()

public:
	CSageListCtrl();

	void SetAlternateRowColor(BOOL bEnable);
	void SetCenterFirstColumn(BOOL bEnable);
	void SetHighlightColumns(int nFirst, int nCount);
	void SetRowSeparator(BOOL bEnable);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNMCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);

private:
	BOOL IsHighlightColumn(int nSubItem) const;
	COLORREF GetRowBackColor(int nItem) const;
	void DrawCenteredFirstColumn(int nItem, BOOL bSelected, NMLVCUSTOMDRAW* pCD);
	void ApplyFixedRowHeight();
	void DrawRowSeparator(int nItem, NMLVCUSTOMDRAW* pCD);

private:
	BOOL m_bAlternateRow;
	BOOL m_bCenterFirstColumn;
	int m_nHighlightFirst;
	int m_nHighlightCount;
	BOOL m_bRowSeparator;
	CImageList m_imgRowSpacer;
};
