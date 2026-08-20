#pragma once

enum SageEditState
{
	SAGE_EDIT_NORMAL,
	SAGE_EDIT_ERROR
};

class CSageEdit : public CEdit
{
	DECLARE_DYNAMIC(CSageEdit)
	DECLARE_MESSAGE_MAP()

public:
	CSageEdit();

	void SetState(SageEditState nState);

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

private:
	SageEditState m_nState;
};

BOOL SageHandleEditSelectAll(MSG* pMsg);
