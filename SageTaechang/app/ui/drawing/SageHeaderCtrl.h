#pragma once

class CSageHeaderCtrl : public CHeaderCtrl
{
	DECLARE_MESSAGE_MAP()
protected:
	afx_msg void OnPaint();
	afx_msg LRESULT OnHeaderLayout(WPARAM wParam, LPARAM lParam);
};
