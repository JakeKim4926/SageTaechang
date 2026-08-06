#pragma once

class CSageOptionCheck : public CButton
{
	DECLARE_MESSAGE_MAP()

public:
	CSageOptionCheck();

	void SetHint(LPCWSTR pszHint);
	void SetChecked(BOOL bChecked);
	BOOL IsChecked() const;
	int  GetContentWidth() const;

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
	afx_msg BOOL OnClicked();

private:
	CString m_strHint;
	BOOL m_bChecked;
};
