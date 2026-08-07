#include "pch.h"
#include "app/ui/drawing/SageStatusCard.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(CSageStatusCard, CStatic)
END_MESSAGE_MAP()

CSageStatusCard::CSageStatusCard()
	: m_nState(SAGE_STATUS_CARD_IDLE)
	, m_nProgressPercent(0) {
}

void CSageStatusCard::SetIdle(const CString& strMessage) {
	m_nState = SAGE_STATUS_CARD_IDLE;
	m_strMessage = strMessage;
	m_strDetail.Empty();
	m_nProgressPercent = 0;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageStatusCard::SetRunning(const CString& strMessage) {
	m_nState = SAGE_STATUS_CARD_RUNNING;
	m_strMessage = strMessage;
	m_strDetail.Empty();
	m_nProgressPercent = 0;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageStatusCard::SetProgressPercent(int nPercent) {
	if (m_nState != SAGE_STATUS_CARD_RUNNING || m_nProgressPercent == nPercent)
		return;
	m_nProgressPercent = nPercent;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageStatusCard::SetResult(BOOL bSuccess, const CString& strMessage, const CString& strDetail) {
	m_nState = bSuccess ? SAGE_STATUS_CARD_COMPLETED : SAGE_STATUS_CARD_FAILED;
	m_strMessage = strMessage;
	m_strDetail = strDetail;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

COLORREF CSageStatusCard::GetSurfaceColor() const {
	if (m_nState == SAGE_STATUS_CARD_COMPLETED)
		return TAECHANG_COLOR_STATUS_CARD_BG_SUCCESS;
	if (m_nState == SAGE_STATUS_CARD_FAILED)
		return TAECHANG_COLOR_STATUS_CARD_BG_ERROR;
	return TAECHANG_COLOR_PANEL;
}

COLORREF CSageStatusCard::GetBorderColor() const {
	if (m_nState == SAGE_STATUS_CARD_COMPLETED)
		return TAECHANG_COLOR_STATUS_CARD_BORDER_SUCCESS;
	if (m_nState == SAGE_STATUS_CARD_FAILED)
		return TAECHANG_COLOR_DANGER_BORDER;
	return TAECHANG_COLOR_BORDER;
}

COLORREF CSageStatusCard::GetAccentColor() const {
	if (m_nState == SAGE_STATUS_CARD_RUNNING)
		return TAECHANG_COLOR_WARNING;
	if (m_nState == SAGE_STATUS_CARD_COMPLETED)
		return TAECHANG_COLOR_SUCCESS;
	if (m_nState == SAGE_STATUS_CARD_FAILED)
		return TAECHANG_COLOR_ERROR;
	return TAECHANG_COLOR_TEXT_PLACEHOLDER;
}

COLORREF CSageStatusCard::GetMessageColor() const {
	if (m_nState == SAGE_STATUS_CARD_IDLE)
		return TAECHANG_COLOR_SECONDARY_TEXT;
	if (m_nState == SAGE_STATUS_CARD_COMPLETED)
		return TAECHANG_COLOR_STATUS_CARD_TEXT_SUCCESS;
	if (m_nState == SAGE_STATUS_CARD_FAILED)
		return TAECHANG_COLOR_INLINE_ERROR_TEXT;
	return TAECHANG_COLOR_TEXT;
}

void CSageStatusCard::DrawCardSurface(CDC* pDC, const CRect& rectClient) {
	pDC->FillSolidRect(rectClient, GetSurfaceColor());
	CBrush brushBorder(GetBorderColor());
	pDC->FrameRect(rectClient, &brushBorder);
}

void CSageStatusCard::DrawStatusDot(CDC* pDC, const CRect& rectDot) {
	COLORREF clrDot = GetAccentColor();
	CBrush brushDot(clrDot);
	CPen penDot(PS_SOLID, TAECHANG_BORDER_THICKNESS, clrDot);
	CBrush* pOldBrush = pDC->SelectObject(&brushDot);
	CPen* pOldPen = pDC->SelectObject(&penDot);

	pDC->Ellipse(rectDot);

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}

void CSageStatusCard::DrawResultIcon(CDC* pDC, const CRect& rectIcon) {
	COLORREF clrIcon = GetAccentColor();
	CPen penIcon(PS_SOLID, TAECHANG_STATUS_CARD_ICON_THICKNESS, clrIcon);
	CPen* pOldPen = pDC->SelectObject(&penIcon);
	CGdiObject* pOldBrush = pDC->SelectStockObject(NULL_BRUSH);

	CPoint ptCenter = rectIcon.CenterPoint();
	pDC->Ellipse(
		ptCenter.x - TAECHANG_STATUS_CARD_ICON_RADIUS,
		ptCenter.y - TAECHANG_STATUS_CARD_ICON_RADIUS,
		ptCenter.x + TAECHANG_STATUS_CARD_ICON_RADIUS,
		ptCenter.y + TAECHANG_STATUS_CARD_ICON_RADIUS);

	if (m_nState == SAGE_STATUS_CARD_COMPLETED) {
		pDC->MoveTo(
			ptCenter.x + TAECHANG_STATUS_CARD_CHECK_START_X,
			ptCenter.y + TAECHANG_STATUS_CARD_CHECK_START_Y);
		pDC->LineTo(
			ptCenter.x + TAECHANG_STATUS_CARD_CHECK_MID_X,
			ptCenter.y + TAECHANG_STATUS_CARD_CHECK_MID_Y);
		pDC->LineTo(
			ptCenter.x + TAECHANG_STATUS_CARD_CHECK_END_X,
			ptCenter.y + TAECHANG_STATUS_CARD_CHECK_END_Y);
	} else {
		pDC->MoveTo(ptCenter.x, ptCenter.y + TAECHANG_STATUS_CARD_ALERT_STEM_TOP);
		pDC->LineTo(ptCenter.x, ptCenter.y + TAECHANG_STATUS_CARD_ALERT_STEM_BOTTOM);
		pDC->FillSolidRect(
			ptCenter.x, ptCenter.y + TAECHANG_STATUS_CARD_ALERT_DOT_TOP,
			TAECHANG_STATUS_CARD_ALERT_DOT_SIZE, TAECHANG_STATUS_CARD_ALERT_DOT_SIZE, clrIcon);
	}

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

void CSageStatusCard::DrawProgressBar(CDC* pDC, const CRect& rectBar) {
	pDC->FillSolidRect(rectBar, TAECHANG_COLOR_LIST_GRID);
	int nFillWidth = MulDiv(rectBar.Width(), m_nProgressPercent, TAECHANG_PROGRESS_COMPLETE);
	if (nFillWidth <= 0)
		return;
	pDC->FillSolidRect(rectBar.left, rectBar.top, nFillWidth, rectBar.Height(), TAECHANG_COLOR_PRIMARY);
}

void CSageStatusCard::DrawPendingContent(CDC* pDC, const CRect& rectClient) {
	CRect rectContent(rectClient);
	rectContent.DeflateRect(TAECHANG_CARD_PADDING, 0);

	int nBlockHeight = TAECHANG_STATUS_CARD_TITLE_LINE_HEIGHT;
	if (m_nState == SAGE_STATUS_CARD_RUNNING)
		nBlockHeight += TAECHANG_CARD_ROW_GAP + TAECHANG_STATUS_CARD_PROGRESS_HEIGHT;

	CRect rectRow(rectContent);
	rectRow.top = rectClient.top + (rectClient.Height() - nBlockHeight) / 2;
	rectRow.bottom = rectRow.top + TAECHANG_STATUS_CARD_TITLE_LINE_HEIGHT;

	CRect rectDot(rectRow);
	rectDot.top += (rectRow.Height() - TAECHANG_STATUS_CARD_DOT_SIZE) / 2;
	rectDot.bottom = rectDot.top + TAECHANG_STATUS_CARD_DOT_SIZE;
	rectDot.right = rectDot.left + TAECHANG_STATUS_CARD_DOT_SIZE;
	DrawStatusDot(pDC, rectDot);

	CRect rectText(rectRow);
	rectText.left = rectDot.right + TAECHANG_STATUS_CARD_DOT_GAP;
	if (m_nState == SAGE_STATUS_CARD_RUNNING)
		rectText.right -= TAECHANG_PROGRESS_TEXT_WIDTH;

	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CONTENT_SEMIBOLD));
	pDC->SetTextColor(GetMessageColor());
	pDC->DrawText(m_strMessage, &rectText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	if (m_nState != SAGE_STATUS_CARD_RUNNING)
		return;

	CString strPercent;
	strPercent.Format(TAECHANG_UI_PROGRESS_FORMAT, m_nProgressPercent);
	CRect rectPercent(rectRow);
	rectPercent.left = rectRow.right - TAECHANG_PROGRESS_TEXT_WIDTH;
	pDC->SetTextColor(TAECHANG_COLOR_PRIMARY);
	pDC->DrawText(strPercent, &rectPercent, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	CRect rectBar(rectContent);
	rectBar.top = rectRow.bottom + TAECHANG_CARD_ROW_GAP;
	rectBar.bottom = rectBar.top + TAECHANG_STATUS_CARD_PROGRESS_HEIGHT;
	DrawProgressBar(pDC, rectBar);
}

void CSageStatusCard::DrawResultContent(CDC* pDC, const CRect& rectClient) {
	CRect rectContent(rectClient);
	rectContent.DeflateRect(TAECHANG_CARD_PADDING, 0);

	CRect rectIcon(rectContent);
	rectIcon.top = rectClient.top + (rectClient.Height() - TAECHANG_STATUS_CARD_ICON_SIZE) / 2;
	rectIcon.bottom = rectIcon.top + TAECHANG_STATUS_CARD_ICON_SIZE;
	rectIcon.right = rectIcon.left + TAECHANG_STATUS_CARD_ICON_SIZE;
	DrawResultIcon(pDC, rectIcon);

	int nBlockHeight = TAECHANG_STATUS_CARD_TITLE_LINE_HEIGHT;
	if (!m_strDetail.IsEmpty())
		nBlockHeight += TAECHANG_STATUS_CARD_DETAIL_LINE_HEIGHT;

	CRect rectTitle(rectContent);
	rectTitle.left = rectIcon.right + TAECHANG_STATUS_CARD_ICON_GAP;
	rectTitle.top = rectClient.top + (rectClient.Height() - nBlockHeight) / 2;
	rectTitle.bottom = rectTitle.top + TAECHANG_STATUS_CARD_TITLE_LINE_HEIGHT;

	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CONTENT_SEMIBOLD));
	pDC->SetTextColor(GetMessageColor());
	pDC->DrawText(m_strMessage, &rectTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	if (m_strDetail.IsEmpty())
		return;

	CRect rectDetail(rectTitle);
	rectDetail.top = rectTitle.bottom;
	rectDetail.bottom = rectDetail.top + TAECHANG_STATUS_CARD_DETAIL_LINE_HEIGHT;
	UINT nDetailFormat = (m_nState == SAGE_STATUS_CARD_COMPLETED)
		? DT_PATH_ELLIPSIS : DT_END_ELLIPSIS;
	pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CAPTION));
	pDC->SetTextColor(TAECHANG_COLOR_TEXT_MUTED);
	pDC->DrawText(m_strDetail, &rectDetail, DT_LEFT | DT_VCENTER | DT_SINGLELINE | nDetailFormat);
}

void CSageStatusCard::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);

	DrawCardSurface(pDC, rectClient);
	if (m_nState == SAGE_STATUS_CARD_COMPLETED || m_nState == SAGE_STATUS_CARD_FAILED) {
		DrawResultContent(pDC, rectClient);
		return;
	}
	DrawPendingContent(pDC, rectClient);
}
