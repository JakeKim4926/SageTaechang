#pragma once

enum SageInlineMessageVariant
{
	SAGE_INLINE_ERROR,
	SAGE_INLINE_WARNING
};

class CSageInlineError : public CStatic
{
	DECLARE_MESSAGE_MAP()

public:
	CSageInlineError();

	void SetMessage(const CString& strMessage, SageInlineMessageVariant nVariant);
	void ClearMessage();
	BOOL HasMessage() const;

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	void DrawWarningBox(CDC* pDC, const CRect& rectClient);
	void DrawMessageIcon(CDC* pDC, const CRect& rectIcon);
	COLORREF GetIconColor() const;
	COLORREF GetTextColor() const;

private:
	SageInlineMessageVariant m_nVariant;
	CString m_strMessage;
};
