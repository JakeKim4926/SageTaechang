#pragma once

enum SageStatusCardState
{
	SAGE_STATUS_CARD_IDLE,
	SAGE_STATUS_CARD_RUNNING,
	SAGE_STATUS_CARD_COMPLETED,
	SAGE_STATUS_CARD_FAILED
};

class CSageStatusCard : public CStatic
{
	DECLARE_MESSAGE_MAP()

public:
	CSageStatusCard();

	void SetIdle(const CString& strMessage);
	void SetRunning(const CString& strMessage);
	void SetProgressPercent(int nPercent);
	void SetResult(BOOL bSuccess, const CString& strMessage, const CString& strDetail);

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	void DrawCardSurface(CDC* pDC, const CRect& rectClient);
	void DrawPendingContent(CDC* pDC, const CRect& rectClient);
	void DrawResultContent(CDC* pDC, const CRect& rectClient);
	void DrawStatusDot(CDC* pDC, const CRect& rectDot);
	void DrawResultIcon(CDC* pDC, const CRect& rectIcon);
	void DrawProgressBar(CDC* pDC, const CRect& rectBar);
	COLORREF GetSurfaceColor() const;
	COLORREF GetBorderColor() const;
	COLORREF GetAccentColor() const;
	COLORREF GetMessageColor() const;

private:
	SageStatusCardState m_nState;
	CString m_strMessage;
	CString m_strDetail;
	int m_nProgressPercent;
};
