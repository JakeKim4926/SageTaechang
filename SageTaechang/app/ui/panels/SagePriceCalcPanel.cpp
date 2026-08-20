#include "pch.h"
#include "app/ui/panels/SagePriceCalcPanel.h"
#include "app/ui/dialogs/SageMessageBoxDlg.h"
#include "app/common/SageNumberFormat.h"
#include "app/core/price/SagePriceService.h"
#include "app/infra/db/SageDBMgr.h"
#include "app/infra/file/SageFileUtils.h"
#include "app/ui/dialogs/SageCalcCompanyPickerDlg.h"
#include "app/ui/dialogs/SageCalcEstimateDlg.h"
#include "app/ui/drawing/SageUiResources.h"
#include "SageDefine.h"
#include <uxtheme.h>

BEGIN_MESSAGE_MAP(SagePriceCalcPanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_CBN_SELCHANGE(ID_CALC_COMPANY_COMBO, &SagePriceCalcPanel::OnCompanyChanged)
	ON_CBN_EDITCHANGE(ID_CALC_COMPANY_COMBO, &SagePriceCalcPanel::OnCompanyChanged)
	ON_BN_CLICKED(ID_CALC_BTN, &SagePriceCalcPanel::OnCalc)
	ON_BN_CLICKED(ID_CALC_RESET_BTN, &SagePriceCalcPanel::OnCalcReset)
	ON_EN_CHANGE(ID_CALC_COPIES_EDIT, &SagePriceCalcPanel::OnInputChanged)
	ON_EN_CHANGE(ID_CALC_PAGES_EDIT, &SagePriceCalcPanel::OnInputChanged)
	ON_EN_CHANGE(ID_CALC_FREIGHT_EDIT, &SagePriceCalcPanel::OnFreightChanged)
	ON_BN_CLICKED(ID_CALC_COMPANY_PICK_BTN, &SagePriceCalcPanel::OnCompanyPick)
END_MESSAGE_MAP()

SagePriceCalcPanel::SagePriceCalcPanel()
	: m_rectInputCard(0, 0, 0, 0)
	, m_rectResultCard(0, 0, 0, 0)
	, m_rectHistoryCard(0, 0, 0, 0)
	, m_rectResultRows(0, 0, 0, 0)
	, m_rectTotalBand(0, 0, 0, 0)
	, m_bFormattingFreight(FALSE) {
}

BOOL SagePriceCalcPanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD, rectEmpty, pParent, nId);
}

int SagePriceCalcPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CreateControls();
	ApplyControlFonts();
	ApplyLabelRoles();
	return 0;
}

void SagePriceCalcPanel::CreateControls() {
	CRect r(0, 0, 0, 0);
	m_wndInputSection.Create(SAGE_UI_CALC_SECTION_INPUT, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this, ID_CALC_INPUT_SECTION);
	m_wndCompanyLabel.Create(SAGE_UI_CALC_COMPANY_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCompanyCombo.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL, r, this, ID_CALC_COMPANY_COMBO);
	m_wndCompanyPickBtn.Create(SAGE_UI_CALC_COMPANY_PICK_LABEL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_CALC_COMPANY_PICK_BTN);
	m_wndCompanyPickBtn.SetIcon(SAGE_BUTTON_ICON_SEARCH);
	m_wndCompanyPickBtn.SetTooltip(SAGE_UI_TIP_PICK_COMPANY);
	m_wndCopiesLabel.Create(SAGE_UI_CALC_COPIES_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCopiesEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_NUMBER | ES_RIGHT | ES_AUTOHSCROLL, r, this, ID_CALC_COPIES_EDIT);
	m_wndCopiesUnitLabel.Create(SAGE_UI_CALC_COPIES_UNIT, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndPagesLabel.Create(SAGE_UI_CALC_PAGES_LABEL, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndPagesEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_NUMBER | ES_RIGHT | ES_AUTOHSCROLL, r, this, ID_CALC_PAGES_EDIT);
	m_wndFreightLabel.Create(SAGE_UI_CALC_FREIGHT_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndFreightEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_RIGHT | ES_AUTOHSCROLL, r, this, ID_CALC_FREIGHT_EDIT);
	m_wndFreightUnitLabel.Create(SAGE_UI_CALC_WON_UNIT, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCalcBtn.Create(SAGE_UI_CALC_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_CALC_BTN);
	m_wndCalcBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndCalcResetBtn.Create(SAGE_UI_CALC_RESET_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_CALC_RESET_BTN);
	m_wndCalcResetBtn.SetVariant(SAGE_BUTTON_GHOST);

	m_wndResultSection.Create(SAGE_UI_CALC_SECTION_RESULT, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this, ID_CALC_RESULT_SECTION);
	m_wndPrintLabel.Create(SAGE_UI_CALC_PRINT_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndPrintValue.Create(SAGE_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCoverLabel.Create(SAGE_UI_CALC_COVER_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCoverValue.Create(SAGE_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndSubtotalLabel.Create(SAGE_UI_CALC_SUBTOTAL_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndSubtotalValue.Create(SAGE_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndFreightResultLabel.Create(SAGE_UI_CALC_FREIGHT_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndFreightValue.Create(SAGE_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndTotalLabel.Create(SAGE_UI_CALC_TOTAL_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndTotalValue.Create(SAGE_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndRangeHint.Create(L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);

	m_wndHistorySection.Create(SAGE_UI_CALC_SECTION_HISTORY, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this, ID_CALC_HISTORY_SECTION);
	m_wndHistoryList.Create(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL, r, this, ID_CALC_HISTORY_LIST);
	m_wndHistoryList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_wndHistoryList.SetRowSeparator(TRUE);
	{
		CHeaderCtrl* pHeader = m_wndHistoryList.GetHeaderCtrl();
		if (pHeader && pHeader->GetSafeHwnd()) {
			m_wndHistoryHeader.SubclassWindow(pHeader->GetSafeHwnd());
			SetWindowTheme(m_wndHistoryHeader.GetSafeHwnd(), L"", L"");
		}
	}
	m_wndHistoryList.SetFirstColumnAlign(SAGE_LIST_FIRST_COLUMN_CENTER);
	m_wndHistoryList.InsertColumn(0, SAGE_UI_CALC_HIST_COL_COMPANY, LVCFMT_CENTER, SAGE_CALC_HIST_COL_COMPANY_W);
	m_wndHistoryList.InsertColumn(1, SAGE_UI_CALC_HIST_COL_ITEM, LVCFMT_CENTER, SAGE_CALC_HIST_COL_ITEM_W);
	m_wndHistoryList.InsertColumn(2, SAGE_UI_CALC_HIST_COL_DATE, LVCFMT_CENTER, SAGE_CALC_HIST_COL_DATE_W);
	m_wndHistoryList.InsertColumn(3, SAGE_UI_CALC_HIST_COL_COPIES, LVCFMT_CENTER, SAGE_CALC_HIST_COL_COPIES_W);
	m_wndHistoryList.InsertColumn(4, SAGE_UI_CALC_HIST_COL_PAGES, LVCFMT_CENTER, SAGE_CALC_HIST_COL_PAGES_W);
	m_wndHistoryList.InsertColumn(5, SAGE_UI_CALC_HIST_COL_PRINT, LVCFMT_CENTER, SAGE_CALC_HIST_COL_PRINT_W);
	m_wndHistoryList.InsertColumn(6, SAGE_UI_CALC_HIST_COL_COVER, LVCFMT_CENTER, SAGE_CALC_HIST_COL_COVER_W);
	m_wndHistoryList.InsertColumn(7, SAGE_UI_CALC_HIST_COL_FREIGHT, LVCFMT_CENTER, SAGE_CALC_HIST_COL_FREIGHT_W);
	m_wndHistoryList.InsertColumn(8, SAGE_UI_CALC_HIST_COL_TOTAL, LVCFMT_CENTER, SAGE_CALC_HIST_COL_TOTAL_W);
	m_wndHistoryList.InsertColumn(9, SAGE_UI_CALC_HIST_COL_TIME, LVCFMT_CENTER, SAGE_CALC_HIST_COL_TIME_W);

	m_wndCopiesEdit.SetLimitText(SAGE_CALC_COPIES_INPUT_MAX_LEN);
	m_wndPagesEdit.SetLimitText(SAGE_CALC_COPIES_INPUT_MAX_LEN);
	m_wndFreightEdit.SetLimitText(SAGE_CALC_FREIGHT_INPUT_MAX_LEN);
	m_wndCompanyCombo.LimitText(SAGE_PRICE_COMPANY_MAX_LEN_EN);
	if (CHeaderCtrl* pHeader = m_wndHistoryList.GetHeaderCtrl()) {
		for (int i = 0; i < pHeader->GetItemCount(); ++i) {
			HDITEM hdi = {};
			hdi.mask = HDI_FORMAT;
			pHeader->GetItem(i, &hdi);
			hdi.fmt = (hdi.fmt & ~HDF_JUSTIFYMASK) | HDF_CENTER;
			pHeader->SetItem(i, &hdi);
		}
	}
}

void SagePriceCalcPanel::ApplyControlFonts() {
	m_wndCompanyCombo.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCompanyPickBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCopiesEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPagesEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCalcBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCalcResetBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndFreightEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndInputSection.SetFont(SageUiResources::GetFont(SAGE_FONT_HEADER));
	m_wndResultSection.SetFont(SageUiResources::GetFont(SAGE_FONT_HEADER));
	m_wndHistorySection.SetFont(SageUiResources::GetFont(SAGE_FONT_HEADER));
	m_wndHistoryList.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
	if (::IsWindow(m_wndHistoryHeader.GetSafeHwnd()))
		m_wndHistoryHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
}

void SagePriceCalcPanel::ApplyLabelRoles() {
	m_wndTotalLabel.SetTextColorRole(SAGE_TEXT_DEFAULT);
	m_wndTotalLabel.SetBackgroundRole(SAGE_BG_ACCENT_SURFACE);
	m_wndTotalLabel.SetFontRole(SAGE_FONT_HEADER);

	m_wndTotalValue.SetTextColorRole(SAGE_TEXT_PRIMARY);
	m_wndTotalValue.SetBackgroundRole(SAGE_BG_ACCENT_SURFACE);
	m_wndTotalValue.SetFontRole(SAGE_FONT_SUMMARY);

	m_wndRangeHint.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndRangeHint.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndRangeHint.SetFontRole(SAGE_FONT_CAPTION);

	m_wndCompanyLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndCompanyLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCompanyLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCopiesLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndCopiesLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCopiesLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCopiesUnitLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndCopiesUnitLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCopiesUnitLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndPagesLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndPagesLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPagesLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndPrintLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndPrintLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPrintLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndPrintValue.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPrintValue.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCoverLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndCoverLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCoverLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCoverValue.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCoverValue.SetFontRole(SAGE_FONT_CONTENT);

	m_wndSubtotalLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndSubtotalLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndSubtotalLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndSubtotalValue.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndSubtotalValue.SetFontRole(SAGE_FONT_CONTENT);

	m_wndFreightLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndFreightLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndFreightLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndFreightUnitLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndFreightUnitLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndFreightUnitLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndFreightResultLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndFreightResultLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndFreightResultLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndFreightValue.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndFreightValue.SetFontRole(SAGE_FONT_CONTENT);
}

void SagePriceCalcPanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	LayoutChildControls(rectPanel.Width(), rectPanel.Height());
}

void SagePriceCalcPanel::LayoutChildControls(int nWidth, int nHeight) {
	int nInputCardWidth = SAGE_CALC_INPUT_CARD_WIDTH;
	if (nInputCardWidth > nWidth)
		nInputCardWidth = nWidth;
	int nResultCardLeft = nInputCardWidth + SAGE_CALC_CARD_GAP;
	int nResultCardWidth = nWidth - nResultCardLeft;
	if (nResultCardWidth < SAGE_CALC_RESULT_CARD_MIN_WIDTH)
		nResultCardWidth = SAGE_CALC_RESULT_CARD_MIN_WIDTH;

	m_rectInputCard = CRect(0, 0, nInputCardWidth, GetInputCardHeight());
	m_rectResultCard = CRect(nResultCardLeft, 0,
		nResultCardLeft + nResultCardWidth, GetResultCardHeight());
	LayoutInputCard(m_rectInputCard);
	LayoutResultCard(m_rectResultCard);

	int nHistoryTop = max(m_rectInputCard.bottom, m_rectResultCard.bottom) + SAGE_CARD_GAP;
	int nHistoryBottom = nHeight;
	int nHistoryMinBottom = nHistoryTop + SAGE_BORDER_THICKNESS * 2
		+ SAGE_CARD_HEADER_HEIGHT + SAGE_RESULT_MIN_HEIGHT;
	if (nHistoryBottom < nHistoryMinBottom)
		nHistoryBottom = nHistoryMinBottom;
	m_rectHistoryCard = CRect(0, nHistoryTop, nWidth, nHistoryBottom);
	LayoutHistoryCard(m_rectHistoryCard);

	int nHistoryCount = static_cast<int>(m_arrHistory.GetSize());
	TrimHistoryToVisibleCapacity();
	if (m_arrHistory.GetSize() != nHistoryCount)
		RefreshHistoryList();
}

int SagePriceCalcPanel::GetInputCardHeight() const {
	return SAGE_BORDER_THICKNESS
		+ SAGE_CARD_HEADER_HEIGHT
		+ SAGE_CARD_PADDING
		+ SAGE_EDIT_HEIGHT + SAGE_CARD_ROW_GAP
		+ SAGE_EDIT_HEIGHT + SAGE_CARD_ROW_GAP
		+ SAGE_EDIT_HEIGHT + SAGE_CARD_ROW_GAP
		+ SAGE_CARD_ACTION_BUTTON_HEIGHT
		+ SAGE_CARD_PADDING
		+ SAGE_BORDER_THICKNESS;
}

int SagePriceCalcPanel::GetResultCardHeight() const {
	return SAGE_BORDER_THICKNESS
		+ SAGE_CARD_HEADER_HEIGHT
		+ SAGE_CARD_PADDING
		+ SAGE_CALC_RESULT_ROW_HEIGHT * SAGE_CALC_RESULT_ROW_COUNT
		+ SAGE_CALC_TOTAL_BAND_GAP
		+ SAGE_CALC_TOTAL_BAND_HEIGHT
		+ SAGE_CARD_ROW_GAP
		+ SAGE_CALC_RANGE_HINT_HEIGHT
		+ SAGE_CARD_PADDING
		+ SAGE_BORDER_THICKNESS;
}

void SagePriceCalcPanel::LayoutCardHeader(CSageSectionLabel& wndSection, const CRect& rectCard) {
	wndSection.MoveWindow(
		rectCard.left + SAGE_BORDER_THICKNESS,
		rectCard.top + SAGE_BORDER_THICKNESS,
		rectCard.Width() - SAGE_BORDER_THICKNESS * 2,
		SAGE_CARD_HEADER_HEIGHT);
}

int SagePriceCalcPanel::GetCardContentTop(const CRect& rectCard) const {
	return rectCard.top + SAGE_BORDER_THICKNESS + SAGE_CARD_HEADER_HEIGHT;
}

void SagePriceCalcPanel::LayoutInputCard(const CRect& rectCard) {
	LayoutCardHeader(m_wndInputSection, rectCard);

	int nContentLeft = rectCard.left + SAGE_CARD_PADDING;
	int nContentRight = rectCard.right - SAGE_CARD_PADDING;
	int nFieldLeft = nContentLeft + SAGE_FORM_LABEL_WIDTH + SAGE_LABEL_EDIT_GAP;
	int nTop = GetCardContentTop(rectCard) + SAGE_CARD_PADDING;

	int nPickBtnLeft = nContentRight - SAGE_CALC_COMPANY_PICK_BTN_W;
	m_wndCompanyLabel.MoveWindow(nContentLeft, nTop, SAGE_FORM_LABEL_WIDTH, SAGE_EDIT_HEIGHT);
	m_wndCompanyCombo.MoveWindow(nFieldLeft, nTop,
		nPickBtnLeft - SAGE_LABEL_EDIT_GAP - nFieldLeft,
		SAGE_EDIT_HEIGHT * SAGE_CALC_COMBO_DROP_ROWS);
	m_wndCompanyPickBtn.MoveWindow(nPickBtnLeft, nTop,
		SAGE_CALC_COMPANY_PICK_BTN_W, SAGE_ICON_BUTTON_SIZE);
	nTop += SAGE_EDIT_HEIGHT + SAGE_CARD_ROW_GAP;

	int nUnitLeft = nFieldLeft + SAGE_CALC_COPIES_EDIT_WIDTH + SAGE_LABEL_EDIT_GAP;
	m_wndCopiesLabel.MoveWindow(nContentLeft, nTop, SAGE_FORM_LABEL_WIDTH, SAGE_EDIT_HEIGHT);
	m_wndCopiesEdit.MoveWindow(nFieldLeft, nTop, SAGE_CALC_COPIES_EDIT_WIDTH, SAGE_EDIT_HEIGHT);
	ApplyEditTextRect(m_wndCopiesEdit);
	m_wndCopiesUnitLabel.MoveWindow(nUnitLeft, nTop, SAGE_CALC_UNIT_LABEL_WIDTH, SAGE_EDIT_HEIGHT);

	int nPagesLabelLeft = nUnitLeft + SAGE_CALC_UNIT_LABEL_WIDTH + SAGE_CARD_ROW_GAP;
	m_wndPagesLabel.MoveWindow(nPagesLabelLeft, nTop, SAGE_FORM_LABEL_WIDTH, SAGE_EDIT_HEIGHT);
	m_wndPagesEdit.MoveWindow(nPagesLabelLeft + SAGE_FORM_LABEL_WIDTH + SAGE_LABEL_EDIT_GAP, nTop,
		SAGE_CALC_COPIES_EDIT_WIDTH, SAGE_EDIT_HEIGHT);
	ApplyEditTextRect(m_wndPagesEdit);
	nTop += SAGE_EDIT_HEIGHT + SAGE_CARD_ROW_GAP;

	m_wndFreightLabel.MoveWindow(nContentLeft, nTop, SAGE_FORM_LABEL_WIDTH, SAGE_EDIT_HEIGHT);
	m_wndFreightEdit.MoveWindow(nFieldLeft, nTop, SAGE_CALC_COPIES_EDIT_WIDTH, SAGE_EDIT_HEIGHT);
	ApplyEditTextRect(m_wndFreightEdit);
	m_wndFreightUnitLabel.MoveWindow(nUnitLeft, nTop, SAGE_CALC_UNIT_LABEL_WIDTH, SAGE_EDIT_HEIGHT);
	nTop += SAGE_EDIT_HEIGHT + SAGE_CARD_ROW_GAP;

	int nResetLeft = nContentRight - SAGE_INPUT_RESET_WIDTH;
	m_wndCalcBtn.MoveWindow(nContentLeft, nTop,
		nResetLeft - SAGE_ACTION_GAP - nContentLeft, SAGE_CARD_ACTION_BUTTON_HEIGHT);
	m_wndCalcResetBtn.MoveWindow(nResetLeft, nTop,
		SAGE_INPUT_RESET_WIDTH, SAGE_CARD_ACTION_BUTTON_HEIGHT);
}

void SagePriceCalcPanel::LayoutResultCard(const CRect& rectCard) {
	LayoutCardHeader(m_wndResultSection, rectCard);

	int nContentLeft = rectCard.left + SAGE_CARD_PADDING;
	int nContentRight = rectCard.right - SAGE_CARD_PADDING;
	int nTop = GetCardContentTop(rectCard) + SAGE_CARD_PADDING;

	m_rectResultRows = CRect(nContentLeft, nTop, nContentRight,
		nTop + SAGE_CALC_RESULT_ROW_HEIGHT * SAGE_CALC_RESULT_ROW_COUNT);
	LayoutResultRow(nTop, nContentLeft, nContentRight, m_wndPrintLabel, m_wndPrintValue);
	nTop += SAGE_CALC_RESULT_ROW_HEIGHT;
	LayoutResultRow(nTop, nContentLeft, nContentRight, m_wndCoverLabel, m_wndCoverValue);
	nTop += SAGE_CALC_RESULT_ROW_HEIGHT;
	LayoutResultRow(nTop, nContentLeft, nContentRight, m_wndSubtotalLabel, m_wndSubtotalValue);
	nTop += SAGE_CALC_RESULT_ROW_HEIGHT;
	LayoutResultRow(nTop, nContentLeft, nContentRight, m_wndFreightResultLabel, m_wndFreightValue);
	nTop += SAGE_CALC_RESULT_ROW_HEIGHT + SAGE_CALC_TOTAL_BAND_GAP;

	m_rectTotalBand = CRect(nContentLeft, nTop, nContentRight, nTop + SAGE_CALC_TOTAL_BAND_HEIGHT);
	int nBandTextTop = nTop + (SAGE_CALC_TOTAL_BAND_HEIGHT - SAGE_EDIT_HEIGHT) / 2;
	int nBandLabelLeft = nContentLeft + SAGE_CALC_TOTAL_BAND_PAD;
	int nBandValueLeft = nBandLabelLeft + SAGE_FORM_LABEL_WIDTH + SAGE_LABEL_EDIT_GAP;
	m_wndTotalLabel.MoveWindow(nBandLabelLeft, nBandTextTop, SAGE_FORM_LABEL_WIDTH, SAGE_EDIT_HEIGHT);
	m_wndTotalValue.MoveWindow(nBandValueLeft, nBandTextTop,
		nContentRight - SAGE_CALC_TOTAL_BAND_PAD - nBandValueLeft, SAGE_EDIT_HEIGHT);
	nTop += SAGE_CALC_TOTAL_BAND_HEIGHT + SAGE_CARD_ROW_GAP;

	m_wndRangeHint.MoveWindow(nContentLeft, nTop,
		nContentRight - nContentLeft, SAGE_CALC_RANGE_HINT_HEIGHT);
}

void SagePriceCalcPanel::LayoutResultRow(int nTop, int nLeft, int nRight,
	CSageLabel& wndLabel, CSageLabel& wndValue) {
	int nTextTop = nTop + (SAGE_CALC_RESULT_ROW_HEIGHT - SAGE_EDIT_HEIGHT) / 2;
	int nValueLeft = nLeft + SAGE_FORM_LABEL_WIDTH + SAGE_LABEL_EDIT_GAP;
	wndLabel.MoveWindow(nLeft, nTextTop, SAGE_FORM_LABEL_WIDTH, SAGE_EDIT_HEIGHT);
	wndValue.MoveWindow(nValueLeft, nTextTop, nRight - nValueLeft, SAGE_EDIT_HEIGHT);
}

void SagePriceCalcPanel::LayoutHistoryCard(const CRect& rectCard) {
	LayoutCardHeader(m_wndHistorySection, rectCard);

	int nListTop = GetCardContentTop(rectCard);
	int nListWidth = rectCard.Width() - SAGE_BORDER_THICKNESS * 2;
	m_wndHistoryList.MoveWindow(rectCard.left + SAGE_BORDER_THICKNESS, nListTop,
		nListWidth, rectCard.bottom - nListTop - SAGE_BORDER_THICKNESS);

	int nFixedWidth = SAGE_CALC_HIST_COL_COMPANY_W + SAGE_CALC_HIST_COL_DATE_W
		+ SAGE_CALC_HIST_COL_COPIES_W + SAGE_CALC_HIST_COL_PAGES_W
		+ SAGE_CALC_HIST_COL_PRINT_W + SAGE_CALC_HIST_COL_COVER_W
		+ SAGE_CALC_HIST_COL_FREIGHT_W + SAGE_CALC_HIST_COL_TOTAL_W
		+ SAGE_CALC_HIST_COL_TIME_W + ::GetSystemMetrics(SM_CXVSCROLL);
	int nItemWidth = nListWidth - nFixedWidth;
	if (nItemWidth < SAGE_CALC_HIST_COL_ITEM_W)
		nItemWidth = SAGE_CALC_HIST_COL_ITEM_W;

	m_wndHistoryList.SetColumnWidth(0, SAGE_CALC_HIST_COL_COMPANY_W);
	m_wndHistoryList.SetColumnWidth(1, nItemWidth);
	m_wndHistoryList.SetColumnWidth(2, SAGE_CALC_HIST_COL_DATE_W);
	m_wndHistoryList.SetColumnWidth(3, SAGE_CALC_HIST_COL_COPIES_W);
	m_wndHistoryList.SetColumnWidth(4, SAGE_CALC_HIST_COL_PAGES_W);
	m_wndHistoryList.SetColumnWidth(5, SAGE_CALC_HIST_COL_PRINT_W);
	m_wndHistoryList.SetColumnWidth(6, SAGE_CALC_HIST_COL_COVER_W);
	m_wndHistoryList.SetColumnWidth(7, SAGE_CALC_HIST_COL_FREIGHT_W);
	m_wndHistoryList.SetColumnWidth(8, SAGE_CALC_HIST_COL_TOTAL_W);
	m_wndHistoryList.SetColumnWidth(9, SAGE_CALC_HIST_COL_TIME_W);
}

void SagePriceCalcPanel::ApplyEditTextRect(CEdit& wndEdit) {
	CRect rectText;
	wndEdit.GetClientRect(&rectText);
	rectText.left += SAGE_FORM_EDIT_TEXT_SIDE_PAD;
	rectText.top += SAGE_FORM_EDIT_TEXT_TOP_PAD;
	rectText.right -= SAGE_FORM_EDIT_TEXT_SIDE_PAD;
	rectText.bottom -= SAGE_FORM_EDIT_TEXT_SIDE_PAD;
	wndEdit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rectText));
}

void SagePriceCalcPanel::DrawCard(CDC* pDC, const CRect& rectCard) {
	if (rectCard.IsRectEmpty())
		return;
	pDC->FillSolidRect(rectCard, SAGE_COLOR_PANEL);
	CBrush brushBorder(SAGE_COLOR_BORDER);
	pDC->FrameRect(rectCard, &brushBorder);
}

BOOL SagePriceCalcPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, SAGE_COLOR_APP_BACKGROUND);

	DrawCard(pDC, m_rectInputCard);
	DrawCard(pDC, m_rectResultCard);
	DrawCard(pDC, m_rectHistoryCard);

	if (!m_rectResultRows.IsRectEmpty()) {
		for (int i = 1; i <= SAGE_CALC_RESULT_ROW_COUNT; ++i)
			pDC->FillSolidRect(m_rectResultRows.left,
				m_rectResultRows.top + SAGE_CALC_RESULT_ROW_HEIGHT * i - SAGE_CALC_RESULT_DIVIDER_HEIGHT,
				m_rectResultRows.Width(), SAGE_CALC_RESULT_DIVIDER_HEIGHT, SAGE_COLOR_LIST_GRID);
	}
	if (!m_rectTotalBand.IsRectEmpty())
		pDC->FillSolidRect(m_rectTotalBand, SAGE_COLOR_ACCENT_SURFACE);

	DrawEditBorder(pDC, m_wndCompanyCombo);
	DrawEditBorder(pDC, m_wndCopiesEdit);
	DrawEditBorder(pDC, m_wndPagesEdit);
	DrawEditBorder(pDC, m_wndFreightEdit);
	return TRUE;
}

void SagePriceCalcPanel::DrawEditBorder(CDC* pDC, CWnd& wnd) {
	if (!::IsWindow(wnd.GetSafeHwnd()) || !wnd.IsWindowVisible())
		return;
	CRect rect;
	wnd.GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.InflateRect(1, 1);
	pDC->FillSolidRect(rect.left, rect.top, rect.Width(), 1, SAGE_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.bottom - 1, rect.Width(), 1, SAGE_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.top, 1, rect.Height(), SAGE_COLOR_BORDER);
	pDC->FillSolidRect(rect.right - 1, rect.top, 1, rect.Height(), SAGE_COLOR_BORDER);
}

HBRUSH SagePriceCalcPanel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hBrush = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
		return hBrush;
	pDC->SetTextColor(SAGE_COLOR_TEXT);
	if (nCtlColor == CTLCOLOR_STATIC) {
		pDC->SetBkColor(SAGE_COLOR_APP_BACKGROUND);
		return SageUiResources::GetBrush(SAGE_BG_APP);
	}
	if (nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX) {
		pDC->SetBkColor(SAGE_COLOR_PANEL);
		return SageUiResources::GetBrush(SAGE_BG_PANEL);
	}
	return hBrush;
}

BOOL SagePriceCalcPanel::PreTranslateMessage(MSG* pMsg) {
	if (pMsg && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB && GetKeyState(VK_SHIFT) >= 0) {
		COMBOBOXINFO cbiCompany = {};
		cbiCompany.cbSize = sizeof(COMBOBOXINFO);
		m_wndCompanyCombo.GetComboBoxInfo(&cbiCompany);
		if (pMsg->hwnd == m_wndCompanyCombo.GetSafeHwnd() || pMsg->hwnd == cbiCompany.hwndItem) {
			m_wndCopiesEdit.SetFocus();
			return TRUE;
		}
		if (pMsg->hwnd == m_wndCopiesEdit.GetSafeHwnd()) {
			m_wndPagesEdit.SetFocus();
			return TRUE;
		}
		if (pMsg->hwnd == m_wndPagesEdit.GetSafeHwnd()) {
			m_wndFreightEdit.SetFocus();
			return TRUE;
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}

void SagePriceCalcPanel::RefreshCompanyCombo() {
	m_wndCompanyCombo.ResetContent();
	CStringArray arrNames;
	CString strError;
	if (sageDBMgr.GetSagePriceService()->LoadAllCompanyNames(arrNames, strError) == FALSE)
		return;
	for (int i = 0; i < arrNames.GetSize(); ++i)
		m_wndCompanyCombo.AddString(arrNames[i]);
}

void SagePriceCalcPanel::ClearInputAndResult() {
	m_wndCopiesEdit.SetWindowTextW(L"");
	m_wndPagesEdit.SetWindowTextW(L"");
	m_wndFreightEdit.SetWindowTextW(L"");
	ClearResult();
}

void SagePriceCalcPanel::ClearResult() {
	m_calcResult = SagePriceCalcResult();
	m_wndPrintValue.SetWindowTextW(SAGE_UI_PRICE_SUMMARY_EMPTY);
	m_wndCoverValue.SetWindowTextW(SAGE_UI_PRICE_SUMMARY_EMPTY);
	m_wndSubtotalValue.SetWindowTextW(SAGE_UI_PRICE_SUMMARY_EMPTY);
	m_wndFreightValue.SetWindowTextW(SAGE_UI_PRICE_SUMMARY_EMPTY);
	m_wndTotalValue.SetWindowTextW(SAGE_UI_PRICE_SUMMARY_EMPTY);
	m_wndRangeHint.SetWindowTextW(L"");
}

void SagePriceCalcPanel::UpdateRangeHint() {
	if (m_calcResult.nRangeMinCopies < 1) {
		m_wndRangeHint.SetWindowTextW(L"");
		return;
	}

	CString strUnit = FormatPrice(m_calcResult.nUnitPrice);
	CString strCover = FormatPrice(m_calcResult.nCoverPrice);
	CString strHint;
	if (m_calcResult.bRangeHasMaxCopies)
		strHint.Format(SAGE_UI_CALC_RANGE_FMT, m_calcResult.nRangeMinCopies,
			m_calcResult.nRangeMaxCopies, strUnit.GetString(), strCover.GetString());
	else
		strHint.Format(SAGE_UI_CALC_RANGE_OPEN_FMT, m_calcResult.nRangeMinCopies,
			strUnit.GetString(), strCover.GetString());
	m_wndRangeHint.SetWindowTextW(strHint);
}

BOOL SagePriceCalcPanel::UpdatePreview(BOOL bShowMessage) {
	int nSel = m_wndCompanyCombo.GetCurSel();
	if (nSel == CB_ERR) {
		ClearResult();
		if (bShowMessage)
			ShowSageMessageBox(SAGE_UI_CALC_SELECT_COMPANY, MB_ICONWARNING);
		return FALSE;
	}

	CString strCompany;
	m_wndCompanyCombo.GetLBText(nSel, strCompany);

	CString strCopies;
	m_wndCopiesEdit.GetWindowTextW(strCopies);
	strCopies.Trim();
	if (strCopies.IsEmpty()) {
		ClearResult();
		if (bShowMessage)
			ShowSageMessageBox(SAGE_UI_CALC_COPIES_REQUIRED, MB_ICONWARNING);
		return FALSE;
	}

	SagePriceCalcService calcService(sageDBMgr.GetSagePriceService());
	SagePriceCalcFailure nFailure;
	if (calcService.ValidateCopies(_wtoi(strCopies), nFailure) == FALSE) {
		ClearResult();
		if (bShowMessage)
			ShowFailureMessage(nFailure, CString());
		return FALSE;
	}

	CString strPages;
	m_wndPagesEdit.GetWindowTextW(strPages);
	strPages.Trim();
	if (strPages.IsEmpty()) {
		ClearResult();
		if (bShowMessage)
			ShowSageMessageBox(SAGE_UI_CALC_PAGES_REQUIRED, MB_ICONWARNING);
		return FALSE;
	}

	CString strFreight;
	m_wndFreightEdit.GetWindowTextW(strFreight);
	strFreight.Trim();

	SagePriceCalcResult result;
	CString strError;
	if (calcService.Calculate(strCompany, _wtoi(strCopies), _wtoi(strPages), PriceTextToInt(strFreight),
		result, nFailure, strError) == FALSE) {
		ClearResult();
		if (bShowMessage)
			ShowFailureMessage(nFailure, strError);
		return FALSE;
	}

	m_calcResult = result;

	CString strPrint, strCover, strSub;
	strPrint.Format(SAGE_UI_CALC_WON_FORMAT, FormatPrice(m_calcResult.nPrintPrice).GetString());
	strCover.Format(SAGE_UI_CALC_WON_FORMAT, FormatPrice(m_calcResult.nCoverPrice).GetString());
	strSub.Format(SAGE_UI_CALC_WON_FORMAT, FormatPrice(m_calcResult.nSubtotal).GetString());
	m_wndPrintValue.SetWindowTextW(strPrint);
	m_wndCoverValue.SetWindowTextW(strCover);
	m_wndSubtotalValue.SetWindowTextW(strSub);
	UpdateRangeHint();
	UpdateTotal();
	return TRUE;
}

void SagePriceCalcPanel::ShowFailureMessage(SagePriceCalcFailure nFailure, const CString& strError) const {
	switch (nFailure) {
	case SAGE_PRICE_CALC_COPIES_BELOW_MIN:
		ShowSageMessageBox(SAGE_UI_CALC_COPIES_INVALID, MB_ICONWARNING);
		return;
	case SAGE_PRICE_CALC_COPIES_ABOVE_MAX:
		ShowSageMessageBox(SAGE_UI_PRICE_COPIES_OUT_OF_RANGE, MB_ICONWARNING);
		return;
	case SAGE_PRICE_CALC_PAGES_OUT_OF_RANGE:
		ShowSageMessageBox(SAGE_UI_CALC_PAGES_INVALID, MB_ICONWARNING);
		return;
	case SAGE_PRICE_CALC_NO_DATA:
		ShowSageMessageBox(SAGE_UI_CALC_NO_DATA, MB_ICONWARNING);
		return;
	default:
		ShowSageMessageBox(strError, MB_ICONERROR);
		return;
	}
}

void SagePriceCalcPanel::UpdateTotal() {
	CString strFreight;
	m_wndFreightEdit.GetWindowTextW(strFreight);
	strFreight.Trim();

	SagePriceCalcService calcService(sageDBMgr.GetSagePriceService());
	calcService.ApplyFreight(PriceTextToInt(strFreight), m_calcResult);

	CString strFreightValue;
	strFreightValue.Format(SAGE_UI_CALC_WON_FORMAT, FormatPrice(m_calcResult.nFreight).GetString());
	m_wndFreightValue.SetWindowTextW(strFreightValue);

	CString strTotal;
	strTotal.Format(SAGE_UI_CALC_WON_FORMAT, FormatPrice(m_calcResult.nTotal).GetString());
	m_wndTotalValue.SetWindowTextW(strTotal);
}

void SagePriceCalcPanel::OnCalc() {
	if (UpdatePreview(TRUE) == FALSE)
		return;

	int nSel = m_wndCompanyCombo.GetCurSel();
	CString strCompany;
	m_wndCompanyCombo.GetLBText(nSel, strCompany);

	CString strCopies;
	m_wndCopiesEdit.GetWindowTextW(strCopies);
	strCopies.Trim();
	int nCopies = _wtoi(strCopies);

	CString strPages;
	m_wndPagesEdit.GetWindowTextW(strPages);
	strPages.Trim();
	int nPages = _wtoi(strPages);

	CString strFreight;
	m_wndFreightEdit.GetWindowTextW(strFreight);
	strFreight.Trim();
	int nFreight = PriceTextToInt(strFreight);
	if (nFreight < 0) nFreight = 0;

	CString strPluginDir;
	if (!GetExecutableDirectory(strPluginDir)) {
		ShowSageMessageBox(SAGE_UI_CALC_ESTIMATE_SCRIPT_MISSING, MB_ICONERROR);
		return;
	}
	CString strTemplatePath = CombinePath(strPluginDir, SAGE_ESTIMATE_TEMPLATE_REL_PATH);
	CString strScriptPath   = CombinePath(strPluginDir, SAGE_CALC_ESTIMATE_SCRIPT_REL_PATH);

	if (!FileExists(strTemplatePath)) {
		ShowSageMessageBox(SAGE_UI_CALC_ESTIMATE_TEMPLATE_MISSING, MB_ICONERROR);
		return;
	}
	if (!FileExists(strScriptPath)) {
		ShowSageMessageBox(SAGE_UI_CALC_ESTIMATE_SCRIPT_MISSING, MB_ICONERROR);
		return;
	}

	SageCalcEstimateDlg dlg(strCompany, nCopies, nPages,
		m_calcResult.nUnitPrice, m_calcResult.nCoverPrice, nFreight,
		strTemplatePath, strScriptPath, this);
	if (dlg.DoModal() == IDOK) {
		AddHistory(strCompany, nCopies, nPages, dlg.GetItemName(), dlg.GetDate(),
			m_calcResult.nPrintPrice, m_calcResult.nCoverPrice, nFreight,
			m_calcResult.nPrintPrice + m_calcResult.nCoverPrice + nFreight);
	}
}

void SagePriceCalcPanel::OnCalcReset() {
	m_wndCopiesEdit.SetWindowTextW(L"");
	m_wndPagesEdit.SetWindowTextW(L"");
	m_wndFreightEdit.SetWindowTextW(L"");
	ClearResult();
	m_wndCopiesEdit.SetFocus();
}

void SagePriceCalcPanel::OnCompanyChanged() {
	ClearInputAndResult();
}

void SagePriceCalcPanel::OnInputChanged() {
	UpdatePreview(FALSE);
}

void SagePriceCalcPanel::OnFreightChanged() {
	FormatPriceEditText(m_wndFreightEdit, m_bFormattingFreight);
	UpdateTotal();
}

void SagePriceCalcPanel::OnCompanyPick() {
	int nCount = m_wndCompanyCombo.GetCount();
	CStringArray arrNames;
	for (int i = 0; i < nCount; i++) {
		CString strName;
		m_wndCompanyCombo.GetLBText(i, strName);
		arrNames.Add(strName);
	}

	CString strCurrent;
	int nCurSel = m_wndCompanyCombo.GetCurSel();
	if (nCurSel != CB_ERR)
		m_wndCompanyCombo.GetLBText(nCurSel, strCurrent);

	SageCalcCompanyPickerDlg dlg(arrNames, strCurrent, this);
	if (dlg.DoModal() == IDOK) {
		CString strSelected = dlg.GetSelectedName();
		int nIdx = m_wndCompanyCombo.FindStringExact(-1, strSelected);
		if (nIdx != CB_ERR) {
			m_wndCompanyCombo.SetCurSel(nIdx);
			ClearInputAndResult();
		}
	}
}

void SagePriceCalcPanel::AddHistory(const CString& strCompany, int nCopies, int nPages, const CString& strItemName,
	const CString& strDate, LONGLONG nPrintPrice, int nCoverPrice, int nFreight, LONGLONG nTotal) {
	SageCalcHistoryEntry entry;
	entry.strCompanyName = strCompany;
	entry.strItemName = strItemName;
	entry.strDate = strDate;
	entry.nCopies = nCopies;
	entry.nPages = nPages;
	entry.nPrintPrice = nPrintPrice;
	entry.nCoverPrice = nCoverPrice;
	entry.nFreight = nFreight;
	entry.nTotal = nTotal;
	entry.timeCalc = CTime::GetCurrentTime();

	m_arrHistory.InsertAt(0, entry);
	TrimHistoryToVisibleCapacity();

	RefreshHistoryList();
}

int SagePriceCalcPanel::GetHistoryVisibleCapacity() const {
	if (!::IsWindow(m_wndHistoryList.GetSafeHwnd()))
		return SAGE_CALC_MAX_HISTORY;

	int nCapacity = m_wndHistoryList.GetCountPerPage();
	if (nCapacity <= 0)
		return SAGE_CALC_MAX_HISTORY;
	return nCapacity;
}

void SagePriceCalcPanel::TrimHistoryToVisibleCapacity() {
	int nCapacity = GetHistoryVisibleCapacity();
	if (nCapacity < 1)
		nCapacity = 1;
	while (m_arrHistory.GetSize() > nCapacity)
		m_arrHistory.RemoveAt(nCapacity);
}

void SagePriceCalcPanel::RefreshHistoryList() {
	int nCount = static_cast<int>(m_arrHistory.GetSize());
	if (nCount > 0) {
		CString strCount;
		strCount.Format(SAGE_UI_CALC_HIST_COUNT_FMT, nCount);
		m_wndHistorySection.SetHintText(strCount);
	} else {
		m_wndHistorySection.SetHintText(L"");
	}

	m_wndHistoryList.SetRedraw(FALSE);
	m_wndHistoryList.DeleteAllItems();
	for (int i = 0; i < m_arrHistory.GetSize(); ++i) {
		const SageCalcHistoryEntry& e = m_arrHistory[i];
		m_wndHistoryList.InsertItem(i, e.strCompanyName);
		m_wndHistoryList.SetItemText(i, 1, e.strItemName);
		m_wndHistoryList.SetItemText(i, 2, e.strDate);

		CString strCopies;
		strCopies.Format(SAGE_UI_CALC_HIST_COPIES_FMT, e.nCopies);
		m_wndHistoryList.SetItemText(i, 3, strCopies);

		CString strPages;
		strPages.Format(SAGE_UI_CALC_HIST_PAGES_FMT, e.nPages);
		m_wndHistoryList.SetItemText(i, 4, strPages);

		CString strPrint, strCover, strFreight, strTotal;
		strPrint.Format(SAGE_UI_CALC_WON_FORMAT, FormatPrice(e.nPrintPrice).GetString());
		strCover.Format(SAGE_UI_CALC_WON_FORMAT, FormatPrice(e.nCoverPrice).GetString());
		strFreight.Format(SAGE_UI_CALC_WON_FORMAT, FormatPrice(e.nFreight).GetString());
		strTotal.Format(SAGE_UI_CALC_WON_FORMAT, FormatPrice(e.nTotal).GetString());
		m_wndHistoryList.SetItemText(i, 5, strPrint);
		m_wndHistoryList.SetItemText(i, 6, strCover);
		m_wndHistoryList.SetItemText(i, 7, strFreight);
		m_wndHistoryList.SetItemText(i, 8, strTotal);

		CString strTime = e.timeCalc.Format(SAGE_UI_CALC_HIST_TIME_FMT);
		m_wndHistoryList.SetItemText(i, 9, strTime);
	}
	m_wndHistoryList.SetRedraw(TRUE);
	m_wndHistoryList.Invalidate();
}
