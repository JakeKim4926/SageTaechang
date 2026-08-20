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
	SAGE_BUTTON_ICON_RESET,
	SAGE_BUTTON_ICON_ADD,
	SAGE_BUTTON_ICON_CLOSE,
	SAGE_BUTTON_ICON_MOVE_UP,
	SAGE_BUTTON_ICON_MOVE_DOWN
};

class CSageButton : public CButton
{
public:
	CSageButton();

	void SetVariant(SageButtonVariant nVariant);
	void SetIcon(SageButtonIcon nIcon);
	void SetTooltip(const CString& strTooltip);
	void SetSurfaceColor(COLORREF clrSurface);
	void SetFocusRing(BOOL bVisible);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	int GetIconSize() const;
	COLORREF GetFocusRingColor() const;
	void DrawIconAt(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon);
	void DrawSearchIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon);
	void DrawResetIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon);
	void DrawAddIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon);
	void DrawCloseIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon);
	void DrawArrowIcon(CDC& dc, const CPoint& ptCenter, COLORREF clrIcon, BOOL bUp);

private:
	SageButtonVariant m_nVariant;
	SageButtonIcon m_nIcon;
	COLORREF m_clrSurface;
	BOOL m_bFocusRing;
	CToolTipCtrl m_toolTip;
};
