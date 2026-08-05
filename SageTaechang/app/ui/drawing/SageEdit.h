#pragma once

enum SageEditState
{
	SAGE_EDIT_NORMAL,
	SAGE_EDIT_ERROR
};

class CSageEdit : public CEdit
{
	DECLARE_MESSAGE_MAP()

public:
	CSageEdit();

	void SetState(SageEditState nState);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnNcPaint();

private:
	SageEditState m_nState;
};
