#pragma once

#include "app/ui/drawing/SageButton.h"

class CSageDialogCaptionBar : public CWnd
{
	DECLARE_MESSAGE_MAP()

public:
	CSageDialogCaptionBar();

	BOOL Create(CWnd* pParent, LPCWSTR pszTitle, UINT nCloseCommandId);
	void Layout(int nWidth);

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnNcHitTest(CPoint point);

private:
	CString m_strTitle;
	CSageButton m_wndCloseBtn;
};
