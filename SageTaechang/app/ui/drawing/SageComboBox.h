#pragma once

class CSageComboBox : public CComboBox
{
	DECLARE_MESSAGE_MAP()

public:
	void ApplyFieldHeight();

protected:
	afx_msg void OnPaint();
	afx_msg LRESULT OnSetFontMessage(WPARAM wParam, LPARAM lParam);
};
