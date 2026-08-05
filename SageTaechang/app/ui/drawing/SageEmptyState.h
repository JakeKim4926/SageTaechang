#pragma once

#include "app/ui/drawing/SageButton.h"

class CSageEmptyState : public CStatic
{
	DECLARE_MESSAGE_MAP()

public:
	CSageEmptyState();

	void SetContent(const CString& strTitle, const CString& strDescription);
	void SetAction(const CString& strLabel, UINT nCommandId);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnActionClicked();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	int GetTextWidth(const CRect& rectClient) const;
	int MeasureDescriptionHeight(CDC* pDC, int nWidth) const;
	int MeasureBlockHeight(CDC* pDC, int nWidth) const;
	void DrawIconBox(CDC* pDC, const CRect& rectBox);
	void LayoutActionButton();

private:
	CString m_strTitle;
	CString m_strDescription;
	UINT m_nCommandId;
	CSageButton m_wndActionBtn;
};
