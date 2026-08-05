#pragma once

enum SageButtonVariant {
	SAGE_BUTTON_SECONDARY,
	SAGE_BUTTON_PRIMARY,
	SAGE_BUTTON_GHOST,
	SAGE_BUTTON_DANGER
};

enum SageButtonIcon {
	SAGE_BUTTON_ICON_NONE,
	SAGE_BUTTON_ICON_SEARCH,
	SAGE_BUTTON_ICON_CALCULATE,
	SAGE_BUTTON_ICON_RESET,
	SAGE_BUTTON_ICON_ADD
};

class CSageButton : public CButton
{
public:
	CSageButton();

	void SetVariant(SageButtonVariant nVariant);
	void SetIcon(SageButtonIcon nIcon);
	void SetTooltip(const CString& strTooltip);
	void SetSurfaceColor(COLORREF clrSurface);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	int GetIconSize() const;
	void DrawIconAt(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon);
	void DrawSearchIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon);
	void DrawCalculateIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon);
	void DrawResetIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon);
	void DrawAddIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon);

private:
	SageButtonVariant m_nVariant;
	SageButtonIcon m_nIcon;
	COLORREF m_clrSurface;
	CToolTipCtrl m_toolTip;
};
