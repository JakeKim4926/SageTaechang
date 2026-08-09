#pragma once

enum SageListFirstColumnAlign
{
	SAGE_LIST_FIRST_COLUMN_DEFAULT,
	SAGE_LIST_FIRST_COLUMN_CENTER,
	SAGE_LIST_FIRST_COLUMN_RIGHT
};

struct SageListRowStyle
{
	SageListRowStyle() {
		clrRowBackground = CLR_NONE;
		clrBadgeBackground = CLR_NONE;
		clrBadgeText = CLR_NONE;
	}

	COLORREF clrRowBackground;
	COLORREF clrBadgeBackground;
	COLORREF clrBadgeText;
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
	void SetCheckboxes(BOOL bEnable);
	void SetMutedText(LPCWSTR pszText, COLORREF clrText);
	void SetBadgeColumn(int nColumn);
	void SetRowStyle(int nState, const SageListRowStyle& style);

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
	BOOL HasCheckboxes() const;
	BOOL FindColumnRect(int nItem, int nColumn, CRect& rectColumn) const;
	BOOL FindCheckImageRect(int nColumn, const CRect& rcColumn, CRect& rectCheckImage) const;
	BOOL BuildCheckStateImages(CImageList& imgState);
	void DrawCheckBox(CDC* pDC, const CRect& rectImage, BOOL bChecked);
	void DrawRowSeparator(int nItem, NMLVCUSTOMDRAW* pCD);
	void DrawSelectionAccent(int nItem, NMLVCUSTOMDRAW* pCD);
	void InvalidateItemRow(int nItem);
	const SageListRowStyle* FindRowStyle(int nItem) const;
	void DrawBadgeColumn(int nItem, BOOL bSelected, NMLVCUSTOMDRAW* pCD);

private:
	BOOL m_bAlternateRow;
	SageListFirstColumnAlign m_nFirstColumnAlign;
	int m_nHighlightFirst;
	int m_nHighlightCount;
	BOOL m_bRowSeparator;
	int m_nBadgeColumn;
	CString m_strMutedText;
	COLORREF m_clrMutedText;
	CImageList m_imgRowSpacer;
	std::vector<SageListRowStyle> m_arrRowStyles;
};
