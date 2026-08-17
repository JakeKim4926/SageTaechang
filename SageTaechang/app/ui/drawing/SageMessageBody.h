#pragma once

enum SageMessageIcon {
	SAGE_MESSAGE_ICON_NONE,
	SAGE_MESSAGE_ICON_INFO,
	SAGE_MESSAGE_ICON_WARNING,
	SAGE_MESSAGE_ICON_ERROR
};

class CSageMessageBody : public CStatic
{
	DECLARE_MESSAGE_MAP()

public:
	CSageMessageBody();

	void SetMessage(const CString& strMessage, SageMessageIcon nIcon);
	int MeasureHeight(int nWidth);

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	void DrawMessageIcon(CDC* pDC, const CRect& rectIcon);
	void DrawAlertGlyph(CDC* pDC, const CRect& rectIcon, COLORREF clrIcon);
	void DrawInfoGlyph(CDC* pDC, const CRect& rectIcon, COLORREF clrIcon);
	COLORREF GetIconColor() const;
	int GetTextLeft() const;

private:
	SageMessageIcon m_nIcon;
	CString m_strMessage;
};
