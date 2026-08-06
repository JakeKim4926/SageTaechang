#pragma once

class CSageTabCtrl : public CTabCtrl
{
	DECLARE_MESSAGE_MAP()
public:
	void ApplyTabHeight();

protected:
	afx_msg void OnPaint();
};
