#pragma once

#include "app/ui/drawing/SageFilterComboBox.h"

class CSageSearchBox : public CStatic
{
	DECLARE_MESSAGE_MAP()

public:
	CSageSearchBox();

	BOOL CreateBox(CWnd* pParent, UINT nBoxId, UINT nEditId);
	BOOL CreateCriteriaCell(UINT nCriteriaId, int nDropRows);
	void SetCommand(UINT nCommandId);
	void ClearCriteriaItems();
	void AddCriteriaItem(LPCWSTR pszLabel, int nItemData);
	int  FindCriteriaIndex(int nItemData) const;
	void SetCriteriaIndex(int nIndex);
	int  GetSelectedCriteria() const;
	void SetPlaceholder(LPCWSTR pszPlaceholder);
	void SetMaxLength(int nMaxLength);
	CString GetKeyword() const;
	void SetKeyword(const CString& strKeyword);
	BOOL IsEditMessage(const MSG* pMsg) const;

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	CRect GetIconCellRect(const CRect& rectClient) const;
	CRect GetCriteriaCellRect(const CRect& rectClient) const;
	int  GetCriteriaCellWidth() const;
	int  GetTextLineHeight();
	void LayoutEdit();
	void LayoutCriteria();

private:
	CEdit m_wndEdit;
	CSageFilterComboBox m_wndCriteria;
	UINT m_nCommandId;
	UINT m_nCriteriaId;
};
