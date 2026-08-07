#pragma once

class CSageSearchBox : public CStatic
{
	DECLARE_MESSAGE_MAP()

public:
	CSageSearchBox();

	BOOL CreateBox(CWnd* pParent, UINT nBoxId, UINT nEditId);
	void SetCommand(UINT nCommandId);
	void SetPlaceholder(LPCWSTR pszPlaceholder);
	void SetMaxLength(int nMaxLength);
	CString GetKeyword() const;
	void SetKeyword(const CString& strKeyword);
	BOOL IsEditMessage(const MSG* pMsg) const;

protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	CRect GetIconCellRect(const CRect& rectClient) const;
	int  GetTextLineHeight();
	void LayoutEdit();

private:
	CEdit m_wndEdit;
	UINT m_nCommandId;
};
