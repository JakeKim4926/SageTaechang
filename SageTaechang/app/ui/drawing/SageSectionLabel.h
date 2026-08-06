#pragma once

class CSageSectionLabel : public CStatic
{
public:
	void SetHintText(LPCWSTR pszHint);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	CString m_strHint;
};
