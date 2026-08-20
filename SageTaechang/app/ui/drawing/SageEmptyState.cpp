#include "pch.h"
#include "app/ui/drawing/SageEmptyState.h"
#include "app/ui/drawing/SageUiResources.h"
#include "SageDefine.h"

BEGIN_MESSAGE_MAP(CSageEmptyState, CStatic)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_BN_CLICKED(ID_EMPTY_STATE_ACTION, &CSageEmptyState::OnActionClicked)
END_MESSAGE_MAP()

CSageEmptyState::CSageEmptyState()
	: m_nCommandId(0) {
}

int CSageEmptyState::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectEmpty(0, 0, 0, 0);
	m_wndActionBtn.Create(L"", WS_CHILD | BS_OWNERDRAW, rectEmpty, this, ID_EMPTY_STATE_ACTION);
	m_wndActionBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndActionBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	return 0;
}

void CSageEmptyState::SetContent(const CString& strTitle, const CString& strDescription) {
	m_strTitle = strTitle;
	m_strDescription = strDescription;
	if (!::IsWindow(GetSafeHwnd()))
		return;
	LayoutActionButton();
	Invalidate();
}

void CSageEmptyState::SetAction(const CString& strLabel, UINT nCommandId) {
	m_nCommandId = nCommandId;
	if (!::IsWindow(m_wndActionBtn.GetSafeHwnd()))
		return;
	m_wndActionBtn.SetWindowTextW(strLabel);
	m_wndActionBtn.ShowWindow(strLabel.IsEmpty() ? SW_HIDE : SW_SHOW);
	LayoutActionButton();
}

void CSageEmptyState::OnActionClicked() {
	if (m_nCommandId == 0)
		return;
	GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(m_nCommandId, BN_CLICKED), 0);
}

void CSageEmptyState::OnSize(UINT nType, int cx, int cy) {
	CStatic::OnSize(nType, cx, cy);
	LayoutActionButton();
}

int CSageEmptyState::GetTextWidth(const CRect& rectClient) const {
	return (rectClient.Width() < SAGE_EMPTY_DESC_MAX_WIDTH)
		? rectClient.Width() : SAGE_EMPTY_DESC_MAX_WIDTH;
}

int CSageEmptyState::MeasureDescriptionHeight(CDC* pDC, int nWidth) const {
	if (m_strDescription.IsEmpty())
		return 0;
	CRect rectText(0, 0, nWidth, 0);
	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	pDC->DrawText(m_strDescription, &rectText, DT_CENTER | DT_WORDBREAK | DT_CALCRECT);
	pDC->SelectObject(pOldFont);
	return rectText.Height();
}

int CSageEmptyState::MeasureBlockHeight(CDC* pDC, int nWidth) const {
	int nHeight = SAGE_EMPTY_ICON_BOX_SIZE
		+ SAGE_EMPTY_BLOCK_GAP + SAGE_EMPTY_TITLE_HEIGHT;

	int nDescHeight = MeasureDescriptionHeight(pDC, nWidth);
	if (nDescHeight > 0)
		nHeight += SAGE_EMPTY_BLOCK_GAP + nDescHeight;

	if (m_wndActionBtn.IsWindowVisible())
		nHeight += SAGE_EMPTY_BLOCK_GAP + SAGE_BUTTON_HEIGHT;

	return nHeight;
}

void CSageEmptyState::LayoutActionButton() {
	if (!::IsWindow(m_wndActionBtn.GetSafeHwnd()) || !m_wndActionBtn.IsWindowVisible())
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	CClientDC dc(this);

	int nBlockHeight = MeasureBlockHeight(&dc, GetTextWidth(rectClient));
	int nBlockTop = rectClient.top + (rectClient.Height() - nBlockHeight) / 2;

	m_wndActionBtn.MoveWindow(
		rectClient.CenterPoint().x - SAGE_EMPTY_ACTION_WIDTH / 2,
		nBlockTop + nBlockHeight - SAGE_BUTTON_HEIGHT,
		SAGE_EMPTY_ACTION_WIDTH, SAGE_BUTTON_HEIGHT);
}

void CSageEmptyState::DrawIconBox(CDC* pDC, const CRect& rectBox) {
	CBrush brushBox(SAGE_COLOR_LIST_HEADER);
	CPen penBox(PS_NULL, 0, SAGE_COLOR_LIST_HEADER);
	CBrush* pOldBrush = pDC->SelectObject(&brushBox);
	CPen* pOldPen = pDC->SelectObject(&penBox);
	pDC->RoundRect(rectBox,
		CPoint(SAGE_EMPTY_ICON_BOX_RADIUS * 2, SAGE_EMPTY_ICON_BOX_RADIUS * 2));
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	CRect rectIcon(0, 0, SAGE_EMPTY_ICON_SIZE, SAGE_EMPTY_ICON_SIZE);
	rectIcon.OffsetRect(
		rectBox.left + (rectBox.Width() - SAGE_EMPTY_ICON_SIZE) / 2,
		rectBox.top + (rectBox.Height() - SAGE_EMPTY_ICON_SIZE) / 2);

	CRect rectGrid(
		rectIcon.left + SAGE_EMPTY_ICON_INSET_X,
		rectIcon.top + SAGE_EMPTY_ICON_INSET_Y,
		rectIcon.right - SAGE_EMPTY_ICON_INSET_X,
		rectIcon.bottom - SAGE_EMPTY_ICON_INSET_Y);

	CPen penIcon(PS_SOLID, SAGE_BORDER_THICKNESS, SAGE_COLOR_PRIMARY);
	pOldPen = pDC->SelectObject(&penIcon);
	CGdiObject* pOldIconBrush = pDC->SelectStockObject(NULL_BRUSH);

	pDC->Rectangle(rectGrid);
	int nHeaderY = rectGrid.top + SAGE_EMPTY_ICON_HEADER_OFFSET;
	pDC->MoveTo(rectGrid.left, nHeaderY);
	pDC->LineTo(rectGrid.right, nHeaderY);
	int nDividerX = rectGrid.left + SAGE_EMPTY_ICON_DIVIDER_OFFSET;
	pDC->MoveTo(nDividerX, nHeaderY);
	pDC->LineTo(nDividerX, rectGrid.bottom);

	pDC->SelectObject(pOldIconBrush);
	pDC->SelectObject(pOldPen);
}

void CSageEmptyState::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rectClient(lpDrawItemStruct->rcItem);

	pDC->FillSolidRect(rectClient, SAGE_COLOR_PANEL);
	CBrush brushFrame(SAGE_COLOR_BORDER);
	pDC->FrameRect(rectClient, &brushFrame);

	int nTextWidth = GetTextWidth(rectClient);
	int nTextLeft = rectClient.left + (rectClient.Width() - nTextWidth) / 2;
	int nY = rectClient.top + (rectClient.Height() - MeasureBlockHeight(pDC, nTextWidth)) / 2;

	CRect rectBox(0, 0, SAGE_EMPTY_ICON_BOX_SIZE, SAGE_EMPTY_ICON_BOX_SIZE);
	rectBox.OffsetRect(rectClient.CenterPoint().x - SAGE_EMPTY_ICON_BOX_SIZE / 2, nY);
	DrawIconBox(pDC, rectBox);
	nY = rectBox.bottom + SAGE_EMPTY_BLOCK_GAP;

	pDC->SetBkMode(TRANSPARENT);
	CRect rectTitle(nTextLeft, nY, nTextLeft + nTextWidth, nY + SAGE_EMPTY_TITLE_HEIGHT);
	CFont* pOldFont = pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_HEADER));
	pDC->SetTextColor(SAGE_COLOR_TEXT);
	pDC->DrawText(m_strTitle, &rectTitle, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	nY = rectTitle.bottom;

	if (!m_strDescription.IsEmpty()) {
		nY += SAGE_EMPTY_BLOCK_GAP;
		CRect rectDesc(nTextLeft, nY, nTextLeft + nTextWidth,
			nY + MeasureDescriptionHeight(pDC, nTextWidth));
		pDC->SelectObject(SageUiResources::GetFont(SAGE_FONT_CONTENT));
		pDC->SetTextColor(SAGE_COLOR_SECONDARY_TEXT);
		pDC->DrawText(m_strDescription, &rectDesc, DT_CENTER | DT_WORDBREAK);
	}

	pDC->SelectObject(pOldFont);
}
