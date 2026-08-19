#include "pch.h"
#include "app/ui/panels/SagePriceManagePanel.h"
#include "app/ui/dialogs/SageMessageBoxDlg.h"
#include "app/common/SageNumberFormat.h"
#include "app/core/price/TaechangPriceService.h"
#include "app/infra/db/SageDBMgr.h"
#include "app/ui/dialogs/TaechangCompanyDlg.h"
#include "app/ui/dialogs/TaechangPriceRangeDlg.h"
#include "app/ui/dialogs/TaechangPriceSimpleDlg.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"
#include <uxtheme.h>

BEGIN_MESSAGE_MAP(SagePriceManagePanel, CWnd)
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_CBN_SELCHANGE(ID_PRICE_COMPANY_EDIT, &SagePriceManagePanel::OnCompanySelChanged)
	ON_CBN_EDITCHANGE(ID_PRICE_COMPANY_EDIT, &SagePriceManagePanel::OnCompanyEditChanged)
	ON_BN_CLICKED(ID_PRICE_ADD_COMPANY_BTN, &SagePriceManagePanel::OnAddCompany)
	ON_BN_CLICKED(ID_PRICE_RENAME_COMPANY_BTN, &SagePriceManagePanel::OnRenameCompany)
	ON_BN_CLICKED(ID_PRICE_DELETE_COMPANY_BTN, &SagePriceManagePanel::OnDeleteCompany)
	ON_NOTIFY(LVN_ITEMCHANGED, ID_PRICE_COPIES_LIST, &SagePriceManagePanel::OnCopiesSelChanged)
	ON_BN_CLICKED(ID_PRICE_NO_MAX_CHECK, &SagePriceManagePanel::OnNoMaxCheck)
	ON_BN_CLICKED(ID_PRICE_SINGLE_CHECK, &SagePriceManagePanel::OnSingleCheck)
	ON_EN_CHANGE(ID_PRICE_PRINT_EDIT, &SagePriceManagePanel::OnPrintChanged)
	ON_EN_CHANGE(ID_PRICE_COVER_EDIT, &SagePriceManagePanel::OnCoverChanged)
	ON_BN_CLICKED(ID_PRICE_ADD_BTN, &SagePriceManagePanel::OnAdd)
	ON_BN_CLICKED(ID_PRICE_MODIFY_BTN, &SagePriceManagePanel::OnModify)
	ON_BN_CLICKED(ID_PRICE_DELETE_BTN, &SagePriceManagePanel::OnDelete)
	ON_BN_CLICKED(ID_PRICE_CANCEL_BTN, &SagePriceManagePanel::OnCancel)
END_MESSAGE_MAP()

SagePriceManagePanel::SagePriceManagePanel()
	: m_rectSummaryCard(0, 0, 0, 0)
	, m_nPanelState(TAECHANG_PRICE_PANEL_SUMMARY)
	, m_bFormattingPrint(FALSE)
	, m_bFormattingCover(FALSE) {
}

BOOL SagePriceManagePanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD, rectEmpty, pParent, nId);
}

int SagePriceManagePanel::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CreateControls();
	ApplyControlFonts();
	ApplyLabelRoles();
	return 0;
}

void SagePriceManagePanel::CreateControls() {
	CRect r(0, 0, 0, 0);
	m_wndCompanyLabel.Create(TAECHANG_UI_PRICE_COMPANY_LABEL, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCompanyCombo.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL, r, this, ID_PRICE_COMPANY_EDIT);
	m_wndAddCompanyBtn.Create(TAECHANG_UI_PRICE_ADD_COMPANY_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_PRICE_ADD_COMPANY_BTN);
	m_wndAddCompanyBtn.SetIcon(SAGE_BUTTON_ICON_ADD);
	m_wndRenameCompanyBtn.Create(TAECHANG_UI_PRICE_RENAME_COMPANY_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_PRICE_RENAME_COMPANY_BTN);
	m_wndDeleteCompanyBtn.Create(TAECHANG_UI_PRICE_DELETE_COMPANY_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_PRICE_DELETE_COMPANY_BTN);
	m_wndDeleteCompanyBtn.SetVariant(SAGE_BUTTON_DANGER);

	m_wndCopiesList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, r, this, ID_PRICE_COPIES_LIST);
	m_wndCopiesList.SetAlternateRowColor(TRUE);
	m_wndCopiesList.SetFirstColumnAlign(SAGE_LIST_FIRST_COLUMN_CENTER);
	m_wndCopiesList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_wndCopiesList.SetRowSeparator(TRUE);
	m_wndCopiesList.SetMutedText(TAECHANG_UI_PRICE_MAX_COPIES_NONE, TAECHANG_COLOR_SECONDARY_TEXT);
	m_wndCopiesList.InsertColumn(0, TAECHANG_UI_PRICE_COL_MIN_COPIES, LVCFMT_CENTER, TAECHANG_PRICE_COL_MIN_WIDTH);
	m_wndCopiesList.InsertColumn(1, TAECHANG_UI_PRICE_COL_MAX_COPIES, LVCFMT_CENTER, TAECHANG_PRICE_COL_MAX_WIDTH);
	m_wndCopiesList.InsertColumn(2, TAECHANG_UI_PRICE_COL_PRINT_PRICE, LVCFMT_CENTER, TAECHANG_PRICE_COL_PRINT_WIDTH);
	m_wndCopiesList.InsertColumn(3, TAECHANG_UI_PRICE_COL_COVER_PRICE, LVCFMT_CENTER, TAECHANG_PRICE_COL_COVER_WIDTH);
	if (CHeaderCtrl* pHeader = m_wndCopiesList.GetHeaderCtrl()) {
		m_wndCopiesHeader.SubclassWindow(pHeader->GetSafeHwnd());
		SetWindowTheme(m_wndCopiesHeader.GetSafeHwnd(), L"", L"");
		HDITEM hdi = {};
		hdi.mask = HDI_FORMAT;
		m_wndCopiesHeader.GetItem(0, &hdi);
		hdi.fmt = (hdi.fmt & ~HDF_JUSTIFYMASK) | HDF_CENTER;
		m_wndCopiesHeader.SetItem(0, &hdi);
	}

	m_wndCopiesEmpty.Create(NULL, WS_CHILD | SS_OWNERDRAW, r, this);
	m_wndCopiesEmpty.SetContent(TAECHANG_UI_PRICE_EMPTY_TITLE, TAECHANG_UI_PRICE_EMPTY_DESC);
	m_wndCopiesEmpty.SetAction(TAECHANG_UI_PRICE_ADD_BTN, ID_PRICE_ADD_BTN);

	m_wndMinCopiesLabel.Create(TAECHANG_UI_PRICE_MIN_COPIES_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndMinCopiesEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER | ES_AUTOHSCROLL, r, this, ID_PRICE_MIN_COPIES_EDIT);
	m_wndSingleCheck.Create(TAECHANG_UI_PRICE_SINGLE_LABEL, WS_CHILD | BS_AUTOCHECKBOX, r, this, ID_PRICE_SINGLE_CHECK);
	m_wndMaxCopiesLabel.Create(TAECHANG_UI_PRICE_MAX_COPIES_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndMaxCopiesEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER | ES_AUTOHSCROLL, r, this, ID_PRICE_MAX_COPIES_EDIT);
	m_wndNoMaxCheck.Create(TAECHANG_UI_PRICE_NO_MAX_LABEL, WS_CHILD | BS_AUTOCHECKBOX, r, this, ID_PRICE_NO_MAX_CHECK);

	m_wndPrintLabel.Create(TAECHANG_UI_PRICE_PRINT_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndPrintEdit.Create(WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_PRICE_PRINT_EDIT);
	m_wndCoverLabel.Create(TAECHANG_UI_PRICE_COVER_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCoverEdit.Create(WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_PRICE_COVER_EDIT);

	m_wndAddBtn.Create(TAECHANG_UI_PRICE_ADD_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_PRICE_ADD_BTN);
	m_wndAddBtn.SetIcon(SAGE_BUTTON_ICON_ADD);
	m_wndAddBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndModifyBtn.Create(TAECHANG_UI_PRICE_SAVE_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_MODIFY_BTN);
	m_wndDeleteBtn.Create(TAECHANG_UI_PRICE_REMOVE_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_DELETE_BTN);
	m_wndDeleteBtn.SetVariant(SAGE_BUTTON_DANGER);
	m_wndCancelBtn.Create(TAECHANG_UI_PRICE_CANCEL_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_CANCEL_BTN);

	m_wndDetailHeader.Create(TAECHANG_UI_PRICE_DETAIL_HEADER, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndDetailDivider.Create(L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, r, this);
	SetWindowTheme(m_wndSingleCheck.GetSafeHwnd(), L"", L"");
	SetWindowTheme(m_wndNoMaxCheck.GetSafeHwnd(), L"", L"");
	m_wndSummaryTitle.Create(TAECHANG_UI_PRICE_SUMMARY_GUIDE, WS_CHILD | SS_LEFT, r, this);
	m_wndSummaryCount.Create(L"", WS_CHILD | SS_LEFT, r, this);
	m_wndSummaryRange.Create(L"", WS_CHILD | SS_LEFT, r, this);

	m_wndCompanyCombo.LimitText(TAECHANG_PRICE_COMPANY_MAX_LEN_EN);
	m_wndMinCopiesEdit.SetLimitText(TAECHANG_PRICE_COPIES_INPUT_MAX_LEN);
	m_wndMaxCopiesEdit.SetLimitText(TAECHANG_PRICE_COPIES_INPUT_MAX_LEN);
	m_wndPrintEdit.SetLimitText(TAECHANG_PRICE_AMOUNT_INPUT_MAX_LEN);
	m_wndCoverEdit.SetLimitText(TAECHANG_PRICE_AMOUNT_INPUT_MAX_LEN);
}

void SagePriceManagePanel::ApplyControlFonts() {
	m_wndCompanyCombo.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndAddCompanyBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndRenameCompanyBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndDeleteCompanyBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCopiesList.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
	if (::IsWindow(m_wndCopiesHeader.GetSafeHwnd()))
		m_wndCopiesHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
	m_wndMinCopiesEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSingleCheck.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndMaxCopiesEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndNoMaxCheck.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPrintEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCoverEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndAddBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndModifyBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndDeleteBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCancelBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
}

void SagePriceManagePanel::ApplyLabelRoles() {
	m_wndSummaryTitle.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndSummaryTitle.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndSummaryTitle.SetFontRole(SAGE_FONT_CONTENT);

	m_wndSummaryCount.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndSummaryCount.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndSummaryCount.SetFontRole(SAGE_FONT_CONTENT);

	m_wndSummaryRange.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndSummaryRange.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndSummaryRange.SetFontRole(SAGE_FONT_CONTENT);

	m_wndMinCopiesLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndMinCopiesLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndMinCopiesLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndMaxCopiesLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndMaxCopiesLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndMaxCopiesLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndPrintLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndPrintLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPrintLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCoverLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndCoverLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCoverLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndDetailHeader.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndDetailHeader.SetFontRole(SAGE_FONT_HEADER);
	m_wndDetailDivider.SetBackgroundRole(SAGE_BG_PANEL);

	m_wndCompanyLabel.SetFontRole(SAGE_FONT_CONTENT);
}

void SagePriceManagePanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	LayoutChildControls(rectPanel.Width(), rectPanel.Height());
}

void SagePriceManagePanel::LayoutChildControls(int nWidth, int nHeight) {
	int nLabelW = TAECHANG_PRICE_FORM_LABEL_WIDTH;
	int nCardW = TAECHANG_PRICE_SUMMARY_CARD_WIDTH;
	int nCardGap = TAECHANG_PRICE_SUMMARY_CARD_GAP;
	int nCardPad = TAECHANG_PRICE_SUMMARY_CARD_PADDING;

	int nInnerLeft = TAECHANG_MARGIN;
	int nInnerW = nWidth - TAECHANG_MARGIN * 2;

	int nLeftW = nInnerW - nCardW - nCardGap;
	int nRightX = nInnerLeft + nLeftW + nCardGap;

	int nY = TAECHANG_MARGIN;

	int nActionButtonCount = TAECHANG_PRICE_ACTION_BUTTON_COUNT;
	int nCompanyComboW = min(TAECHANG_PRICE_COMPANY_COMBO_WIDTH,
							 nLeftW - nLabelW - TAECHANG_LABEL_EDIT_GAP - TAECHANG_BUTTON_WIDTH * nActionButtonCount - TAECHANG_ROW_GAP * nActionButtonCount);
	if (nCompanyComboW < TAECHANG_PRICE_COMPANY_COMBO_MIN_WIDTH)
		nCompanyComboW = TAECHANG_PRICE_COMPANY_COMBO_MIN_WIDTH;
	m_wndCompanyLabel.MoveWindow(nInnerLeft, nY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndCompanyCombo.MoveWindow(nInnerLeft + nLabelW + TAECHANG_LABEL_EDIT_GAP, nY, nCompanyComboW,
		TAECHANG_EDIT_HEIGHT * TAECHANG_PRICE_COMBO_DROP_ROWS);
	int nBtnX = nInnerLeft + nLabelW + TAECHANG_LABEL_EDIT_GAP + nCompanyComboW + TAECHANG_ROW_GAP;
	m_wndAddCompanyBtn.MoveWindow(nBtnX, nY, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nBtnX += TAECHANG_BUTTON_WIDTH + TAECHANG_ROW_GAP;
	m_wndAddBtn.MoveWindow(nBtnX, nY, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nBtnX += TAECHANG_BUTTON_WIDTH + TAECHANG_ROW_GAP;
	m_wndRenameCompanyBtn.MoveWindow(nBtnX, nY, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nBtnX += TAECHANG_BUTTON_WIDTH + TAECHANG_ROW_GAP;
	m_wndDeleteCompanyBtn.MoveWindow(nBtnX, nY, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nY += TAECHANG_BUTTON_HEIGHT + TAECHANG_ROW_GAP;

	int nListH = nHeight - nY - TAECHANG_MARGIN;
	if (nListH < TAECHANG_RESULT_HEADER_HEIGHT + TAECHANG_EDIT_HEIGHT * 4)
		nListH = TAECHANG_RESULT_HEADER_HEIGHT + TAECHANG_EDIT_HEIGHT * 4;

	m_wndCopiesList.MoveWindow(nInnerLeft, nY, nLeftW, nListH);
	m_wndCopiesEmpty.MoveWindow(nInnerLeft, nY, nLeftW, nListH);
	int nColMinMax = TAECHANG_PRICE_COL_MINMAX_WIDTH;
	int nColPrice = (nLeftW - nColMinMax * 2) / 2;
	m_wndCopiesList.SetColumnWidth(0, nColMinMax);
	m_wndCopiesList.SetColumnWidth(1, nColMinMax);
	m_wndCopiesList.SetColumnWidth(2, nColPrice);
	m_wndCopiesList.SetColumnWidth(3, nLeftW - nColMinMax * 2 - nColPrice);

	int nCardTop = nY;
	int nCardInnerX = nRightX + nCardPad;
	int nCardInnerW = nCardW - nCardPad * 2;

	int nPanelY = nCardTop + nCardPad;

	m_wndDetailHeader.MoveWindow(nCardInnerX, nPanelY, nCardInnerW, TAECHANG_PRICE_PANEL_LABEL_HEIGHT);
	int nContentY = nPanelY + TAECHANG_PRICE_PANEL_LABEL_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP;
	m_wndDetailDivider.MoveWindow(nCardInnerX, nContentY, nCardInnerW, TAECHANG_PRICE_DETAIL_DIVIDER_HEIGHT);
	nContentY += TAECHANG_PRICE_DETAIL_DIVIDER_HEIGHT + TAECHANG_ROW_GAP;

	int nSummaryY = nContentY;
	m_wndSummaryTitle.MoveWindow(nCardInnerX, nSummaryY, nCardInnerW, TAECHANG_PRICE_SUMMARY_TITLE_HEIGHT);
	nSummaryY += TAECHANG_PRICE_SUMMARY_TITLE_HEIGHT + TAECHANG_PRICE_SUMMARY_ROW_GAP * 2;
	m_wndSummaryCount.MoveWindow(nCardInnerX, nSummaryY, nCardInnerW, TAECHANG_PRICE_SUMMARY_ROW_HEIGHT);
	nSummaryY += TAECHANG_PRICE_SUMMARY_ROW_HEIGHT + TAECHANG_PRICE_SUMMARY_ROW_GAP;
	m_wndSummaryRange.MoveWindow(nCardInnerX, nSummaryY, nCardInnerW, TAECHANG_PRICE_SUMMARY_ROW_HEIGHT);

	int nFormY = nContentY;
	int nFormLabelW = TAECHANG_PRICE_FORM_LABEL_WIDTH;
	int nInlineEditW = nCardInnerW - nFormLabelW - TAECHANG_LABEL_EDIT_GAP;
	int nEditX = nCardInnerX + nFormLabelW + TAECHANG_LABEL_EDIT_GAP;
	auto ApplyPriceEditTextRect = [](CEdit& edit) {
		CRect rc;
		edit.GetClientRect(&rc);
		rc.left += TAECHANG_FORM_EDIT_TEXT_SIDE_PAD;
		rc.top += TAECHANG_FORM_EDIT_TEXT_TOP_PAD;
		rc.right -= TAECHANG_FORM_EDIT_TEXT_SIDE_PAD;
		rc.bottom -= TAECHANG_FORM_EDIT_TEXT_SIDE_PAD;
		edit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
	};

	m_wndMinCopiesLabel.MoveWindow(nCardInnerX, nFormY, nFormLabelW, TAECHANG_PRICE_EDIT_HEIGHT);
	m_wndMinCopiesEdit.MoveWindow(nEditX, nFormY, nInlineEditW, TAECHANG_PRICE_EDIT_HEIGHT);
	ApplyPriceEditTextRect(m_wndMinCopiesEdit);
	m_wndSingleCheck.MoveWindow(nEditX, nFormY + TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP, nInlineEditW, TAECHANG_PRICE_PANEL_LABEL_HEIGHT);
	nFormY += TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP + TAECHANG_PRICE_PANEL_LABEL_HEIGHT + TAECHANG_ROW_GAP;

	m_wndMaxCopiesLabel.MoveWindow(nCardInnerX, nFormY, nFormLabelW, TAECHANG_PRICE_EDIT_HEIGHT);
	m_wndMaxCopiesEdit.MoveWindow(nEditX, nFormY, nInlineEditW, TAECHANG_PRICE_EDIT_HEIGHT);
	ApplyPriceEditTextRect(m_wndMaxCopiesEdit);
	m_wndNoMaxCheck.MoveWindow(nEditX, nFormY + TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP, nInlineEditW, TAECHANG_PRICE_PANEL_LABEL_HEIGHT);
	nFormY += TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP + TAECHANG_PRICE_PANEL_LABEL_HEIGHT + TAECHANG_ROW_GAP;

	m_wndPrintLabel.MoveWindow(nCardInnerX, nFormY, nFormLabelW, TAECHANG_PRICE_EDIT_HEIGHT);
	m_wndPrintEdit.MoveWindow(nEditX, nFormY, nInlineEditW, TAECHANG_PRICE_EDIT_HEIGHT);
	ApplyPriceEditTextRect(m_wndPrintEdit);
	nFormY += TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_ROW_GAP;

	m_wndCoverLabel.MoveWindow(nCardInnerX, nFormY, nFormLabelW, TAECHANG_PRICE_EDIT_HEIGHT);
	m_wndCoverEdit.MoveWindow(nEditX, nFormY, nInlineEditW, TAECHANG_PRICE_EDIT_HEIGHT);
	ApplyPriceEditTextRect(m_wndCoverEdit);
	nFormY += TAECHANG_PRICE_EDIT_HEIGHT;
	m_rectSummaryCard = CRect(nRightX, nCardTop, nRightX + nCardW, nFormY + nCardPad);

	int nHalfBtnW = (nCardInnerW - TAECHANG_ACTION_GAP) / 2;
	int nButtonY = m_rectSummaryCard.bottom + TAECHANG_ROW_GAP;
	m_wndModifyBtn.MoveWindow(nCardInnerX, nButtonY, nHalfBtnW, TAECHANG_BUTTON_HEIGHT);
	m_wndCancelBtn.MoveWindow(nCardInnerX + nHalfBtnW + TAECHANG_ACTION_GAP, nButtonY, nHalfBtnW, TAECHANG_BUTTON_HEIGHT);
	nButtonY += TAECHANG_BUTTON_HEIGHT + TAECHANG_ROW_GAP;
	m_wndDeleteBtn.MoveWindow(nCardInnerX, nButtonY, nCardInnerW, TAECHANG_BUTTON_HEIGHT);
}

void SagePriceManagePanel::OnShowWindow(BOOL bShow, UINT nStatus) {
	CWnd::OnShowWindow(bShow, nStatus);
	if (bShow) {
		ApplyRightPanel();
		return;
	}
	m_rectSummaryCard.SetRectEmpty();
	m_nPanelState = TAECHANG_PRICE_PANEL_SUMMARY;
}

BOOL SagePriceManagePanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
	if (!m_rectSummaryCard.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectSummaryCard, TAECHANG_COLOR_PANEL);
		CBrush brCardBorder(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectSummaryCard, &brCardBorder);
	}
	DrawEditBorder(pDC, m_wndCompanyCombo);
	DrawEditBorder(pDC, m_wndMinCopiesEdit);
	DrawEditBorder(pDC, m_wndMaxCopiesEdit);
	DrawEditBorder(pDC, m_wndPrintEdit);
	DrawEditBorder(pDC, m_wndCoverEdit);
	return TRUE;
}

void SagePriceManagePanel::DrawEditBorder(CDC* pDC, CWnd& wnd) {
	if (!::IsWindow(wnd.GetSafeHwnd()) || !wnd.IsWindowVisible())
		return;
	CRect rect;
	wnd.GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.InflateRect(1, 1);
	pDC->FillSolidRect(rect.left, rect.top, rect.Width(), 1, TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.bottom - 1, rect.Width(), 1, TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.top, 1, rect.Height(), TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.right - 1, rect.top, 1, rect.Height(), TAECHANG_COLOR_BORDER);
}

HBRUSH SagePriceManagePanel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hBrush = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
		return hBrush;
	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
	if (pWnd->GetSafeHwnd() == m_wndNoMaxCheck.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndSingleCheck.GetSafeHwnd()) {
		pDC->SetBkColor(TAECHANG_COLOR_PANEL);
		return SageUiResources::GetBrush(SAGE_BG_PANEL);
	}
	if (nCtlColor == CTLCOLOR_STATIC) {
		pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
		return SageUiResources::GetBrush(SAGE_BG_APP);
	}
	if (nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX) {
		pDC->SetBkColor(TAECHANG_COLOR_PANEL);
		return SageUiResources::GetBrush(SAGE_BG_PANEL);
	}
	return hBrush;
}

BOOL SagePriceManagePanel::PreTranslateMessage(MSG* pMsg) {
	if (pMsg && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB && GetKeyState(VK_SHIFT) >= 0) {
		if (pMsg->hwnd == m_wndMinCopiesEdit.GetSafeHwnd()) {
			if (m_wndMaxCopiesEdit.IsWindowEnabled())
				m_wndMaxCopiesEdit.SetFocus();
			else
				m_wndPrintEdit.SetFocus();
			return TRUE;
		}
		if (pMsg->hwnd == m_wndMaxCopiesEdit.GetSafeHwnd()) {
			m_wndPrintEdit.SetFocus();
			return TRUE;
		}
		if (pMsg->hwnd == m_wndPrintEdit.GetSafeHwnd()) {
			m_wndCoverEdit.SetFocus();
			return TRUE;
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}

void SagePriceManagePanel::ApplyRightPanel() {
	BOOL bSummary = (m_nPanelState == TAECHANG_PRICE_PANEL_SUMMARY);
	BOOL bEditModify = (m_nPanelState == TAECHANG_PRICE_PANEL_EDIT_MODIFY);
	int nSummaryCmd = bSummary ? SW_SHOW : SW_HIDE;
	int nEditCmd = bSummary ? SW_HIDE : SW_SHOW;

	m_wndSummaryTitle.ShowWindow(nSummaryCmd);
	m_wndSummaryCount.ShowWindow(nSummaryCmd);
	m_wndSummaryRange.ShowWindow(nSummaryCmd);

	m_wndMinCopiesLabel.ShowWindow(nEditCmd);
	m_wndMinCopiesEdit.ShowWindow(nEditCmd);
	m_wndSingleCheck.ShowWindow(nEditCmd);
	m_wndMaxCopiesLabel.ShowWindow(nEditCmd);
	m_wndMaxCopiesEdit.ShowWindow(nEditCmd);
	m_wndNoMaxCheck.ShowWindow(nEditCmd);
	m_wndPrintLabel.ShowWindow(nEditCmd);
	m_wndPrintEdit.ShowWindow(nEditCmd);
	m_wndCoverLabel.ShowWindow(nEditCmd);
	m_wndCoverEdit.ShowWindow(nEditCmd);
	m_wndModifyBtn.ShowWindow(nEditCmd);
	m_wndCancelBtn.ShowWindow(nEditCmd);
	m_wndDeleteBtn.ShowWindow(bEditModify ? SW_SHOW : SW_HIDE);

	Invalidate(FALSE);
}

void SagePriceManagePanel::RefreshCompanyList(const CString& strFilter) {
	m_wndCompanyCombo.ResetContent();
	m_wndCopiesList.DeleteAllItems();
	CStringArray arrNames;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->LoadAllCompanyNames(arrNames, strError) == FALSE) {
		UpdateEmptyState();
		return;
	}
	CString strTarget = strFilter;
	strTarget.Trim();
	CString strNeedle = strTarget;
	strNeedle.MakeLower();
	int nExactIndex = TAECHANG_LIST_NO_ITEM;
	for (int i = 0; i < arrNames.GetSize(); ++i) {
		CString strName = arrNames[i];
		CString strHaystack = strName;
		strHaystack.MakeLower();
		int nIndex = m_wndCompanyCombo.AddString(strName);
		if (!strNeedle.IsEmpty() && strHaystack == strNeedle)
			nExactIndex = nIndex;
	}
	if (nExactIndex != TAECHANG_LIST_NO_ITEM) {
		m_wndCompanyCombo.SetCurSel(nExactIndex);
		RefreshCopiesList(strTarget);
		return;
	}
	if (!strTarget.IsEmpty()) {
		m_wndCompanyCombo.SetWindowTextW(strTarget);
		UpdateSummaryCard();
		UpdateEmptyState();
		return;
	}
	UpdateSummaryCard();
	UpdateEmptyState();
}

void SagePriceManagePanel::RefreshCopiesList(const CString& strCompanyName) {
	m_wndCopiesList.DeleteAllItems();
	if (strCompanyName.IsEmpty()) {
		UpdateSummaryCard();
		UpdateEmptyState();
		return;
	}

	CArray<TaechangPriceDto, TaechangPriceDto&> arrPrice;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->LoadByCompany(strCompanyName, arrPrice, strError) == FALSE) {
		UpdateSummaryCard();
		UpdateEmptyState();
		return;
	}

	m_wndCopiesList.SetRedraw(FALSE);
	for (int i = 0; i < arrPrice.GetSize(); ++i) {
		const TaechangPriceDto& dto = arrPrice[i];
		CString strMin;
		strMin.Format(TAECHANG_UI_COPIES_FORMAT, dto.nMinCopies);
		int nIndex = m_wndCopiesList.InsertItem(i, strMin);
		m_wndCopiesList.SetItemData(nIndex, static_cast<DWORD_PTR>(dto.nPriceId));

		CString strMax;
		if (dto.bHasMaxCopies)
			strMax.Format(TAECHANG_UI_COPIES_FORMAT, dto.nMaxCopies);
		else
			strMax = TAECHANG_UI_PRICE_MAX_COPIES_NONE;
		m_wndCopiesList.SetItemText(nIndex, 1, strMax);

		CString strPrint, strCover;
		strPrint = FormatPrice(dto.nPrintPrice);
		strCover = FormatPrice(dto.nCoverPrice);
		m_wndCopiesList.SetItemText(nIndex, 2, strPrint);
		m_wndCopiesList.SetItemText(nIndex, 3, strCover);
	}
	UpdateSummaryCard();
	UpdateEmptyState();
	m_wndCopiesList.SetRedraw(TRUE);
	m_wndCopiesList.Invalidate();
}

void SagePriceManagePanel::UpdateEmptyState() {
	if (!::IsWindow(m_wndCopiesEmpty.GetSafeHwnd()))
		return;
	BOOL bEmpty = (m_wndCopiesList.GetItemCount() == 0) ? TRUE : FALSE;
	m_wndCopiesList.ShowWindow(bEmpty ? SW_HIDE : SW_SHOW);
	m_wndCopiesEmpty.ShowWindow(bEmpty ? SW_SHOW : SW_HIDE);
}

void SagePriceManagePanel::UpdateSummaryCard() {
	if (!::IsWindow(m_wndSummaryTitle.GetSafeHwnd()))
		return;
	m_wndSummaryTitle.SetWindowTextW(TAECHANG_UI_PRICE_SUMMARY_GUIDE);

	int nCount = m_wndCopiesList.GetItemCount();
	if (nCount == 0) {
		m_wndSummaryCount.SetWindowTextW(L"");
		m_wndSummaryRange.SetWindowTextW(L"");
		return;
	}

	CString strCount;
	strCount.Format(TAECHANG_UI_PRICE_SUMMARY_COUNT_FMT, nCount);
	m_wndSummaryCount.SetWindowTextW(strCount);

	CString strMin = m_wndCopiesList.GetItemText(0, 0);
	CString strMax = m_wndCopiesList.GetItemText(nCount - 1, 1);
	CString strRange;
	if (strMax == TAECHANG_UI_PRICE_MAX_COPIES_NONE)
		strRange.Format(TAECHANG_UI_PRICE_SUMMARY_RANGE_OPEN_FMT, static_cast<LPCWSTR>(strMin));
	else
		strRange.Format(TAECHANG_UI_PRICE_SUMMARY_RANGE_FMT, static_cast<LPCWSTR>(strMin), static_cast<LPCWSTR>(strMax));
	m_wndSummaryRange.SetWindowTextW(strRange);
}

CString SagePriceManagePanel::GetSelectedCompanyName() const {
	CString strCompany;
	m_wndCompanyCombo.GetWindowTextW(strCompany);
	strCompany.Trim();
	return strCompany;
}

void SagePriceManagePanel::LoadSelectedCopiesRowToForm() {
	POSITION pos = m_wndCopiesList.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;
	int nItem = m_wndCopiesList.GetNextSelectedItem(pos);

	CString strMin = m_wndCopiesList.GetItemText(nItem, 0);
	CString strMax = m_wndCopiesList.GetItemText(nItem, 1);
	CString strPrint = m_wndCopiesList.GetItemText(nItem, 2);
	CString strCover = m_wndCopiesList.GetItemText(nItem, 3);

	BOOL bNoMax = (strMax == TAECHANG_UI_PRICE_MAX_COPIES_NONE);
	BOOL bSingle = (!bNoMax && strMin == strMax);
	m_wndMinCopiesEdit.SetWindowTextW(strMin);
	m_wndSingleCheck.SetCheck(bSingle ? BST_CHECKED : BST_UNCHECKED);
	m_wndNoMaxCheck.SetCheck(bNoMax ? BST_CHECKED : BST_UNCHECKED);
	m_wndMaxCopiesEdit.SetWindowTextW((bNoMax || bSingle) ? CString() : strMax);
	m_wndMaxCopiesEdit.EnableWindow(!bNoMax && !bSingle);
	m_wndPrintEdit.SetWindowTextW(strPrint);
	m_wndCoverEdit.SetWindowTextW(strCover);
}

void SagePriceManagePanel::ClearForm() {
	m_wndMinCopiesEdit.SetWindowTextW(L"");
	m_wndSingleCheck.SetCheck(BST_UNCHECKED);
	m_wndMaxCopiesEdit.SetWindowTextW(L"");
	m_wndMaxCopiesEdit.EnableWindow(TRUE);
	m_wndNoMaxCheck.SetCheck(BST_UNCHECKED);
	m_wndPrintEdit.SetWindowTextW(L"");
	m_wndCoverEdit.SetWindowTextW(L"");
}

BOOL SagePriceManagePanel::ReadFormToDto(TaechangPriceDto& dto, CString& strError) {
	CString strMin, strMax, strPrint, strCover;
	m_wndMinCopiesEdit.GetWindowTextW(strMin);
	m_wndMaxCopiesEdit.GetWindowTextW(strMax);
	m_wndPrintEdit.GetWindowTextW(strPrint);
	m_wndCoverEdit.GetWindowTextW(strCover);
	strMin.Trim(); strMax.Trim(); strPrint.Trim(); strCover.Trim();

	dto.nReportType = REPORT_TYPE_AUDIT_REPORT;
	dto.nMinCopies = strMin.IsEmpty() ? 0 : _wtoi(strMin);
	if (dto.nMinCopies < 1 || dto.nMinCopies > TAECHANG_PRICE_COPIES_MAX) {
		strError = TAECHANG_UI_PRICE_COPIES_OUT_OF_RANGE;
		return FALSE;
	}

	BOOL bSingle = (m_wndSingleCheck.GetCheck() == BST_CHECKED);
	dto.bHasMaxCopies = (m_wndNoMaxCheck.GetCheck() == BST_CHECKED) ? FALSE : TRUE;
	if (bSingle) {
		dto.nMaxCopies = dto.nMinCopies;
	} else {
		dto.nMaxCopies = (dto.bHasMaxCopies && !strMax.IsEmpty()) ? _wtoi(strMax) : 0;
		if (dto.bHasMaxCopies) {
			if (dto.nMaxCopies < 1 || dto.nMaxCopies > TAECHANG_PRICE_COPIES_MAX) {
				strError = TAECHANG_UI_PRICE_COPIES_OUT_OF_RANGE;
				return FALSE;
			}
			if (dto.nMaxCopies < dto.nMinCopies) {
				strError = TAECHANG_UI_PRICE_MAX_LESS_THAN_MIN;
				return FALSE;
			}
		}
	}

	dto.nPrintPrice = PriceTextToInt(strPrint);
	if (dto.nPrintPrice < 0 || dto.nPrintPrice > TAECHANG_PRICE_AMOUNT_MAX) {
		strError = TAECHANG_UI_PRICE_AMOUNT_OUT_OF_RANGE;
		return FALSE;
	}

	dto.nCoverPrice = PriceTextToInt(strCover);
	if (dto.nCoverPrice < 0 || dto.nCoverPrice > TAECHANG_PRICE_AMOUNT_MAX) {
		strError = TAECHANG_UI_PRICE_AMOUNT_OUT_OF_RANGE;
		return FALSE;
	}

	return TRUE;
}

void SagePriceManagePanel::OnAddCompany() {
	TaechangCompanyDlg dlg(this);
	if (dlg.DoModal() != IDOK)
		return;

	CString strName = dlg.GetCompanyName();
	CStringArray arrNames;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->LoadAllCompanyNames(arrNames, strError) == FALSE) {
		ShowSageMessageBox(strError, MB_ICONERROR);
		return;
	}

	for (int i = 0; i < arrNames.GetSize(); ++i) {
		CString strItem = arrNames[i];
		strItem.Trim();
		if (strItem.CompareNoCase(strName) != 0)
			continue;

		ShowSageMessageBox(TAECHANG_UI_PRICE_COMPANY_EXISTS, MB_ICONINFORMATION);
		RefreshCompanyList(strItem);
		m_nPanelState = TAECHANG_PRICE_PANEL_SUMMARY;
		ClearForm();
		ApplyRightPanel();
		return;
	}

	RefreshCompanyList(strName);
	int nIndex = m_wndCompanyCombo.AddString(strName);
	m_wndCompanyCombo.SetCurSel(nIndex);
	m_wndCompanyCombo.SetWindowTextW(strName);
	m_wndCopiesList.DeleteAllItems();
	ClearForm();
	m_nPanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	ApplyRightPanel();
}

void SagePriceManagePanel::OnRenameCompany() {
	CString strCompany = GetSelectedCompanyName();
	int nIndex = m_wndCompanyCombo.FindStringExact(-1, strCompany);
	if (strCompany.IsEmpty() || nIndex == CB_ERR) {
		ShowSageMessageBox(TAECHANG_UI_PRICE_SELECT_COMPANY, MB_ICONWARNING);
		return;
	}

	TaechangCompanyRenameDlg dlg(this);
	dlg.SetCompanyContext(strCompany, m_wndCopiesList.GetItemCount());
	if (dlg.DoModal() != IDOK)
		return;

	CString strNewName = dlg.GetCompanyName();
	strNewName.Trim();
	if (strNewName.CompareNoCase(strCompany) == 0)
		return;

	CStringArray arrNames;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->LoadAllCompanyNames(arrNames, strError) == FALSE) {
		ShowSageMessageBox(strError, MB_ICONERROR);
		return;
	}

	for (int i = 0; i < arrNames.GetSize(); ++i) {
		CString strItem = arrNames[i];
		strItem.Trim();
		if (strItem.CompareNoCase(strCompany) != 0 && strItem.CompareNoCase(strNewName) == 0) {
			ShowSageMessageBox(TAECHANG_UI_PRICE_COMPANY_EXISTS, MB_ICONINFORMATION);
			return;
		}
	}

	int nAffectedCount = 0;
	if (sageDBMgr.GetTaechangPriceService()->RenameCompany(strCompany, strNewName, nAffectedCount, strError) == FALSE) {
		ShowSageMessageBox(strError, MB_ICONERROR);
		return;
	}

	m_nPanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshCompanyList(strNewName);
	ClearForm();
	ApplyRightPanel();
}

void SagePriceManagePanel::OnDeleteCompany() {
	CString strCompany = GetSelectedCompanyName();
	int nIndex = m_wndCompanyCombo.FindStringExact(-1, strCompany);
	if (strCompany.IsEmpty() || nIndex == CB_ERR) {
		ShowSageMessageBox(TAECHANG_UI_PRICE_SELECT_COMPANY, MB_ICONWARNING);
		return;
	}

	CString strConfirm;
	strConfirm.Format(TAECHANG_UI_PRICE_DELETE_COMPANY_CONFIRM_FORMAT, strCompany.GetString());
	if (ShowSageMessageBox(strConfirm, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) != IDYES) {
		return;
	}

	int nAffectedCount = 0;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->RemoveCompany(
		strCompany,
		nAffectedCount,
		strError) == FALSE) {
		ShowSageMessageBox(strError, MB_ICONERROR);
		return;
	}

	m_nPanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshCompanyList();
	m_wndCopiesList.DeleteAllItems();
	ClearForm();
	ApplyRightPanel();
}

void SagePriceManagePanel::OnCompanySelChanged() {
	int nSel = m_wndCompanyCombo.GetCurSel();
	if (nSel == CB_ERR)
		return;
	CString strCompany;
	m_wndCompanyCombo.GetLBText(nSel, strCompany);
	m_nPanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshCopiesList(strCompany);
	ClearForm();
	ApplyRightPanel();
}

void SagePriceManagePanel::OnCompanyEditChanged() {
	CString strCompany = GetSelectedCompanyName();
	if (strCompany.IsEmpty())
		return;
	int nIndex = m_wndCompanyCombo.FindString(-1, strCompany);
	if (nIndex == CB_ERR)
		return;
	CString strMatch;
	m_wndCompanyCombo.GetLBText(nIndex, strMatch);
	m_wndCompanyCombo.SetCurSel(nIndex);
	m_wndCompanyCombo.SetEditSel(strCompany.GetLength(), -1);
	m_nPanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshCopiesList(strMatch);
	ClearForm();
	ApplyRightPanel();
}

void SagePriceManagePanel::OnCopiesSelChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	NMLISTVIEW* pNM = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	*pResult = 0;
	if (!(pNM->uChanged & LVIF_STATE) || !(pNM->uNewState & LVIS_SELECTED))
		return;
	m_nPanelState = TAECHANG_PRICE_PANEL_EDIT_MODIFY;
	LoadSelectedCopiesRowToForm();
	ApplyRightPanel();
}

void SagePriceManagePanel::OnNoMaxCheck() {
	BOOL bNoMax = (m_wndNoMaxCheck.GetCheck() == BST_CHECKED);
	if (bNoMax && m_wndSingleCheck.GetCheck() == BST_CHECKED) {
		ShowSageMessageBox(TAECHANG_UI_PRICE_SINGLE_AND_NO_MAX_CONFLICT, MB_ICONWARNING);
		m_wndNoMaxCheck.SetCheck(BST_UNCHECKED);
		return;
	}
	m_wndMaxCopiesEdit.EnableWindow(!bNoMax);
	if (bNoMax)
		m_wndMaxCopiesEdit.SetWindowTextW(L"");
}

void SagePriceManagePanel::OnSingleCheck() {
	BOOL bSingle = (m_wndSingleCheck.GetCheck() == BST_CHECKED);
	if (bSingle && m_wndNoMaxCheck.GetCheck() == BST_CHECKED) {
		ShowSageMessageBox(TAECHANG_UI_PRICE_SINGLE_AND_NO_MAX_CONFLICT, MB_ICONWARNING);
		m_wndSingleCheck.SetCheck(BST_UNCHECKED);
		return;
	}
	m_wndMaxCopiesEdit.EnableWindow(!bSingle);
	if (bSingle)
		m_wndMaxCopiesEdit.SetWindowTextW(L"");
}

void SagePriceManagePanel::OnPrintChanged() {
	FormatPriceEditText(m_wndPrintEdit, m_bFormattingPrint);
}

void SagePriceManagePanel::OnCoverChanged() {
	FormatPriceEditText(m_wndCoverEdit, m_bFormattingCover);
}

void SagePriceManagePanel::OnAdd() {
	int nSel = m_wndCompanyCombo.GetCurSel();
	if (nSel == CB_ERR) {
		ShowSageMessageBox(TAECHANG_UI_PRICE_SELECT_COMPANY, MB_ICONWARNING);
		return;
	}

	CString strCompany;
	m_wndCompanyCombo.GetLBText(nSel, strCompany);
	strCompany.Trim();
	if (strCompany.IsEmpty()) {
		ShowSageMessageBox(TAECHANG_UI_PRICE_SELECT_COMPANY, MB_ICONWARNING);
		return;
	}

	TaechangPriceRangeDlg dlg(this);
	for (int i = 0; i < m_wndCopiesList.GetItemCount(); ++i) {
		int nExistingMin = _wtoi(m_wndCopiesList.GetItemText(i, 0));
		CString strExistingMax = m_wndCopiesList.GetItemText(i, 1);
		BOOL bExistingHasMax = (strExistingMax == TAECHANG_UI_PRICE_MAX_COPIES_NONE) ? FALSE : TRUE;
		int nExistingMax = bExistingHasMax ? _wtoi(strExistingMax) : 0;
		dlg.AddExistingRange(nExistingMin, bExistingHasMax, nExistingMax);
	}
	if (dlg.DoModal() != IDOK)
		return;

	TaechangPriceDto dto;
	dto.strCompanyName = strCompany;
	dto.nReportType = REPORT_TYPE_AUDIT_REPORT;
	dto.nMinCopies = dlg.GetMinCopies();
	dto.bHasMaxCopies = dlg.HasMaxCopies();
	dto.nMaxCopies = dlg.GetMaxCopies();
	dto.nPrintPrice = dlg.GetPrintPrice();
	dto.nCoverPrice = dlg.GetCoverPrice();

	int nNewId = 0;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->AddPrice(dto, nNewId, strError) == FALSE) {
		ShowSageMessageBox(strError, MB_ICONERROR);
		return;
	}

	m_nPanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshCompanyList(strCompany);
	ClearForm();
	ApplyRightPanel();
}

void SagePriceManagePanel::OnModify() {
	CString strCompany = GetSelectedCompanyName();
	TaechangPriceDto dto;
	CString strError;
	if (ReadFormToDto(dto, strError) == FALSE) {
		ShowSageMessageBox(strError, MB_ICONWARNING);
		return;
	}
	dto.strCompanyName = strCompany;

	if (m_nPanelState == TAECHANG_PRICE_PANEL_EDIT_ADD) {
		int nNewId;
		if (sageDBMgr.GetTaechangPriceService()->AddPrice(dto, nNewId, strError) == FALSE) {
			ShowSageMessageBox(strError, MB_ICONERROR);
			return;
		}
	} else {
		POSITION pos = m_wndCopiesList.GetFirstSelectedItemPosition();
		if (pos == NULL) {
			ShowSageMessageBox(TAECHANG_UI_PRICE_SELECT_COPIES_ROW, MB_ICONWARNING);
			return;
		}
		int nItem = m_wndCopiesList.GetNextSelectedItem(pos);
		dto.nPriceId = static_cast<int>(m_wndCopiesList.GetItemData(nItem));
		if (sageDBMgr.GetTaechangPriceService()->ModifyPriceById(dto, strError) == FALSE) {
			ShowSageMessageBox(strError, MB_ICONERROR);
			return;
		}
	}

	m_nPanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshCopiesList(strCompany);
	ClearForm();
	ApplyRightPanel();
}

void SagePriceManagePanel::OnDelete() {
	POSITION pos = m_wndCopiesList.GetFirstSelectedItemPosition();
	if (pos == NULL) {
		ShowSageMessageBox(TAECHANG_UI_PRICE_SELECT_COPIES_ROW, MB_ICONWARNING);
		return;
	}
	int nItem = m_wndCopiesList.GetNextSelectedItem(pos);
	int nPriceId = static_cast<int>(m_wndCopiesList.GetItemData(nItem));
	if (nPriceId <= 0) {
		ShowSageMessageBox(TAECHANG_UI_PRICE_SELECT_COPIES_ROW, MB_ICONWARNING);
		return;
	}

	if (ShowSageMessageBox(TAECHANG_UI_PRICE_DELETE_CONFIRM, MB_YESNO | MB_ICONWARNING) != IDYES)
		return;

	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->RemovePrice(nPriceId, strError) == FALSE) {
		ShowSageMessageBox(strError, MB_ICONERROR);
		return;
	}

	CString strCompany = GetSelectedCompanyName();
	m_nPanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshCopiesList(strCompany);
	ClearForm();
	ApplyRightPanel();
	if (m_wndCopiesList.GetItemCount() == 0)
		RefreshCompanyList();
}

void SagePriceManagePanel::OnCancel() {
	m_nPanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	ClearForm();
	m_wndCopiesList.SetItemState(-1, 0, LVIS_SELECTED);
	ApplyRightPanel();
}
