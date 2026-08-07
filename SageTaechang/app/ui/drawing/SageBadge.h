#pragma once

class CSageBadge : public CStatic
{
	DECLARE_MESSAGE_MAP()

public:
	CSageBadge();

	void SetBadge(const CString& strText, COLORREF clrBackground, COLORREF clrBorder, COLORREF clrText);
	int  GetContentWidth() const;

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	CString m_strText;
	COLORREF m_clrBackground;
	COLORREF m_clrBorder;
	COLORREF m_clrText;
};
