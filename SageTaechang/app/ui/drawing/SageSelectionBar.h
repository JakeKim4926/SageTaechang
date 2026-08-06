#pragma once

#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageUiResources.h"

class CSageSelectionBar : public CStatic
{
	DECLARE_MESSAGE_MAP()

public:
	CSageSelectionBar();

	void SetCommands(UINT nSelectAllCommandId, UINT nClearCommandId);
	void SetCounts(int nTotalCount, int nSelectedCount);
	void SetAllChecked(BOOL bChecked);
	void EnableControls(BOOL bEnable);
	int  GetContentWidth() const;

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelectAllClicked();
	afx_msg void OnClearClicked();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

private:
	void LayoutChildren();
	int  MeasureTextWidth(CDC* pDC, const CString& strText, SageFontRole nRole) const;
	int  GetCheckWidth(CDC* pDC) const;
	int  GetCountWidth(CDC* pDC) const;
	int  GetClearWidth(CDC* pDC) const;
	int  DrawTextSegment(CDC* pDC, int nLeft, const CRect& rectClient, const CString& strText, SageFontRole nRole, COLORREF color);

private:
	CButton m_wndSelectAll;
	CSageButton m_wndClearBtn;
	UINT m_nSelectAllCommandId;
	UINT m_nClearCommandId;
	int m_nTotalCount;
	int m_nSelectedCount;
};
