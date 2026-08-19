#pragma once

class CSageComboBox : public CComboBox
{
	DECLARE_MESSAGE_MAP()

public:
	CSageComboBox();

	void ApplyFieldHeight();

protected:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnSetFontMessage(WPARAM wParam, LPARAM lParam);

private:
	BOOL m_bFittingField;
};
