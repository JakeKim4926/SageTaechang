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
	SAGE_BUTTON_ICON_RESET
};

class CSageButton : public CButton
{
public:
	CSageButton();

	void SetVariant(SageButtonVariant nVariant);
	void SetIcon(SageButtonIcon nIcon);
	void SetSurfaceColor(COLORREF clrSurface);

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	void DrawSearchIcon(CDC& dc, const CRect& rect, COLORREF clrIcon);
	void DrawCalculateIcon(CDC& dc, const CRect& rect, COLORREF clrIcon);
	void DrawResetIcon(CDC& dc, const CRect& rect, COLORREF clrIcon);

private:
	SageButtonVariant m_nVariant;
	SageButtonIcon m_nIcon;
	COLORREF m_clrSurface;
};
