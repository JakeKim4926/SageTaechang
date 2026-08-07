#pragma once

class CSageFilterPillBar : public CStatic
{
	DECLARE_MESSAGE_MAP()

public:
	CSageFilterPillBar();

	void SetCommand(UINT nCommandId);
	void SetLabels(const std::vector<CString>& arrLabels);
	void SetSelectedIndex(int nIndex);
	int  GetSelectedIndex() const;

protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	void BuildPillRects(CDC* pDC, const CRect& rectClient, std::vector<CRect>& outRects) const;
	int  FindPillAt(CPoint point) const;
	void DrawPill(CDC* pDC, const CRect& rectPill, const CString& strLabel, BOOL bSelected);

private:
	std::vector<CString> m_arrLabels;
	int m_nSelectedIndex;
	UINT m_nCommandId;
};
