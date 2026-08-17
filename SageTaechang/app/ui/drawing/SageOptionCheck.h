#pragma once

class CSageOptionCheck : public CButton
{
	DECLARE_MESSAGE_MAP()

public:
	CSageOptionCheck();

	void SetHint(LPCWSTR pszHint);
	void SetChecked(BOOL bChecked);
	void SetFrameVisible(BOOL bVisible);
	BOOL IsChecked() const;
	int  GetContentWidth() const;

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
	afx_msg BOOL OnClicked();

private:
	int GetSidePadding() const;

private:
	CString m_strHint;
	BOOL m_bChecked;
	BOOL m_bFrameVisible;
};
