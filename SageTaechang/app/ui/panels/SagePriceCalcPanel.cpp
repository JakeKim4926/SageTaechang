#include "pch.h"
#include "app/ui/panels/SagePriceCalcPanel.h"
#include "app/ui/dialogs/SageMessageBoxDlg.h"
#include "app/common/SageNumberFormat.h"
#include "app/core/price/TaechangPriceService.h"
#include "app/infra/db/SageDBMgr.h"
#include "app/infra/file/TaechangFileUtils.h"
#include "app/ui/dialogs/TaechangCalcCompanyPickerDlg.h"
#include "app/ui/dialogs/TaechangCalcEstimateDlg.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"
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
	m_wndCompanyLabel.Create(TAECHANG_UI_CALC_COMPANY_LABEL, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCompanyCombo.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL, r, this, ID_CALC_COMPANY_COMBO);
	m_wndCompanyPickBtn.Create(TAECHANG_UI_CALC_COMPANY_PICK_LABEL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_CALC_COMPANY_PICK_BTN);
	m_wndCompanyPickBtn.SetIcon(SAGE_BUTTON_ICON_SEARCH);
	m_wndCompanyPickBtn.SetTooltip(TAECHANG_UI_TIP_PICK_COMPANY);
	m_wndCopiesLabel.Create(TAECHANG_UI_CALC_COPIES_LABEL, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCopiesEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_NUMBER | ES_RIGHT | ES_AUTOHSCROLL, r, this, ID_CALC_COPIES_EDIT);
	m_wndPagesLabel.Create(TAECHANG_UI_CALC_PAGES_LABEL, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndPagesEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_NUMBER | ES_RIGHT | ES_AUTOHSCROLL, r, this, ID_CALC_PAGES_EDIT);
	m_wndCalcBtn.Create(L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_CALC_BTN);
	m_wndCalcBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndCalcBtn.SetIcon(SAGE_BUTTON_ICON_CALCULATE);
	m_wndCalcResetBtn.Create(L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_CALC_RESET_BTN);
	m_wndCalcResetBtn.SetVariant(SAGE_BUTTON_GHOST);
	m_wndCalcResetBtn.SetIcon(SAGE_BUTTON_ICON_RESET);
	m_wndCalcResetBtn.SetTooltip(TAECHANG_UI_TIP_RESET);

	m_wndPrintLabel.Create(TAECHANG_UI_CALC_PRINT_LABEL, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndPrintValue.Create(TAECHANG_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCoverLabel.Create(TAECHANG_UI_CALC_COVER_LABEL, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCoverValue.Create(TAECHANG_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndSubtotalLabel.Create(TAECHANG_UI_CALC_SUBTOTAL_LABEL, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndSubtotalValue.Create(TAECHANG_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndFreightLabel.Create(TAECHANG_UI_CALC_FREIGHT_LABEL, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndFreightEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_RIGHT | ES_AUTOHSCROLL, r, this, ID_CALC_FREIGHT_EDIT);
	m_wndFreightUnitLabel.Create(TAECHANG_UI_CALC_WON_UNIT, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndDivider.Create(L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, r, this);
	m_wndTotalDivider.Create(L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, r, this);
	m_wndTotalLabel.Create(TAECHANG_UI_CALC_TOTAL_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndTotalValue.Create(TAECHANG_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndRangeHint.Create(L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);

	m_wndHistorySection.Create(TAECHANG_UI_CALC_SECTION_HISTORY, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this, ID_CALC_HISTORY_SECTION);
	m_wndHistoryList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL, r, this, ID_CALC_HISTORY_LIST);
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
	m_wndHistoryList.InsertColumn(0, TAECHANG_UI_CALC_HIST_COL_COMPANY, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_COMPANY_W);
	m_wndHistoryList.InsertColumn(1, TAECHANG_UI_CALC_HIST_COL_ITEM, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_ITEM_W);
	m_wndHistoryList.InsertColumn(2, TAECHANG_UI_CALC_HIST_COL_DATE, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_DATE_W);
	m_wndHistoryList.InsertColumn(3, TAECHANG_UI_CALC_HIST_COL_COPIES, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_COPIES_W);
	m_wndHistoryList.InsertColumn(4, TAECHANG_UI_CALC_HIST_COL_PAGES, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_PAGES_W);
	m_wndHistoryList.InsertColumn(5, TAECHANG_UI_CALC_HIST_COL_PRINT, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_PRINT_W);
	m_wndHistoryList.InsertColumn(6, TAECHANG_UI_CALC_HIST_COL_COVER, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_COVER_W);
	m_wndHistoryList.InsertColumn(7, TAECHANG_UI_CALC_HIST_COL_FREIGHT, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_FREIGHT_W);
	m_wndHistoryList.InsertColumn(8, TAECHANG_UI_CALC_HIST_COL_TOTAL, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_TOTAL_W);
	m_wndHistoryList.InsertColumn(9, TAECHANG_UI_CALC_HIST_COL_TIME, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_TIME_W);

	m_wndCopiesEdit.SetLimitText(TAECHANG_CALC_COPIES_INPUT_MAX_LEN);
	m_wndPagesEdit.SetLimitText(TAECHANG_CALC_COPIES_INPUT_MAX_LEN);
	m_wndFreightEdit.SetLimitText(TAECHANG_CALC_FREIGHT_INPUT_MAX_LEN);
	m_wndCompanyCombo.LimitText(TAECHANG_PRICE_COMPANY_MAX_LEN_EN);
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
	m_wndHistorySection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
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

	m_wndCompanyLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCompanyLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCopiesLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCopiesLabel.SetFontRole(SAGE_FONT_CONTENT);

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

	m_wndFreightUnitLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndFreightUnitLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndDivider.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndTotalDivider.SetBackgroundRole(SAGE_BG_PANEL);
}

void SagePriceCalcPanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	LayoutChildControls(rectPanel.Width(), rectPanel.Height());
}

void SagePriceCalcPanel::LayoutChildControls(int nWidth, int nHeight) {
	int nPad = TAECHANG_CALC_PANEL_PADDING;
	int nLabelW = TAECHANG_CALC_RESULT_LABEL_WIDTH;
	int nValW = TAECHANG_CALC_RESULT_VALUE_WIDTH;
	int nInputLabelW = TAECHANG_CALC_INPUT_LABEL_WIDTH;
	int nInputEditW = TAECHANG_CALC_COPIES_EDIT_SHORT_W;
	int nX = TAECHANG_MARGIN;
	int nY = TAECHANG_MARGIN;
	int nW = nWidth - TAECHANG_MARGIN * 2;
	int nInputContentW = nInputLabelW + TAECHANG_LABEL_EDIT_GAP + nInputEditW
		+ TAECHANG_ROW_GAP + nInputLabelW + TAECHANG_LABEL_EDIT_GAP + nInputEditW;
	int nInputPanelW = nInputContentW + nPad * 2;
	if (nInputPanelW > nW)
		nInputPanelW = nW;
	auto ApplyCalcEditTextRect = [](CEdit& edit) {
		CRect rc;
		edit.GetClientRect(&rc);
		rc.left += TAECHANG_FORM_EDIT_TEXT_SIDE_PAD;
		rc.top += TAECHANG_FORM_EDIT_TEXT_TOP_PAD;
		rc.right -= TAECHANG_FORM_EDIT_TEXT_SIDE_PAD;
		rc.bottom -= TAECHANG_FORM_EDIT_TEXT_SIDE_PAD;
		edit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
	};

	int nInputPanelH = nPad + TAECHANG_EDIT_HEIGHT + TAECHANG_ROW_GAP + TAECHANG_EDIT_HEIGHT + nPad;
	m_rectInputCard = CRect(nX, nY, nX + nInputPanelW, nY + nInputPanelH);

	int nCX = nX + nPad;
	int nCY = nY + nPad;
	int nPickBtnGap = TAECHANG_LABEL_EDIT_GAP;
	int nPickBtnW = TAECHANG_CALC_COMPANY_PICK_BTN_W;
	int nComboW = min(TAECHANG_CALC_COMBO_WIDTH,
		nInputContentW - nInputLabelW - TAECHANG_LABEL_EDIT_GAP - nPickBtnW - nPickBtnGap);

	m_wndCompanyLabel.MoveWindow(nCX, nCY, nInputLabelW, TAECHANG_EDIT_HEIGHT);
	int nComboX = nCX + nInputLabelW + TAECHANG_LABEL_EDIT_GAP;
	m_wndCompanyCombo.MoveWindow(nComboX, nCY, nComboW, TAECHANG_EDIT_HEIGHT * TAECHANG_CALC_COMBO_DROP_ROWS);
	m_wndCompanyPickBtn.MoveWindow(nComboX + nComboW + nPickBtnGap, nCY - TAECHANG_BUTTON_VERT_ADJUST, nPickBtnW, TAECHANG_BUTTON_HEIGHT);
	COMBOBOXINFO cbiCompany = {};
	cbiCompany.cbSize = sizeof(COMBOBOXINFO);
	if (m_wndCompanyCombo.GetComboBoxInfo(&cbiCompany) && ::IsWindow(cbiCompany.hwndItem)) {
		CRect rcComboEdit;
		::GetClientRect(cbiCompany.hwndItem, &rcComboEdit);
		rcComboEdit.left += TAECHANG_FORM_EDIT_TEXT_SIDE_PAD;
		rcComboEdit.top += TAECHANG_FORM_EDIT_TEXT_TOP_PAD;
		rcComboEdit.right -= TAECHANG_FORM_EDIT_TEXT_SIDE_PAD;
		rcComboEdit.bottom -= TAECHANG_FORM_EDIT_TEXT_SIDE_PAD;
		::SendMessage(cbiCompany.hwndItem, EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rcComboEdit));
	}
	nCY += TAECHANG_EDIT_HEIGHT + TAECHANG_ROW_GAP;

	m_wndCopiesLabel.MoveWindow(nCX, nCY, nInputLabelW, TAECHANG_EDIT_HEIGHT);
	int nCopiesEditX = nCX + nInputLabelW + TAECHANG_LABEL_EDIT_GAP;
	m_wndCopiesEdit.MoveWindow(nCopiesEditX, nCY, nInputEditW, TAECHANG_EDIT_HEIGHT);
	ApplyCalcEditTextRect(m_wndCopiesEdit);
	int nPagesLabelX = nCopiesEditX + nInputEditW + TAECHANG_ROW_GAP;
	m_wndPagesLabel.MoveWindow(nPagesLabelX, nCY, nInputLabelW, TAECHANG_EDIT_HEIGHT);
	int nPagesEditX = nPagesLabelX + nInputLabelW + TAECHANG_LABEL_EDIT_GAP;
	m_wndPagesEdit.MoveWindow(nPagesEditX, nCY, nInputEditW, TAECHANG_EDIT_HEIGHT);
	ApplyCalcEditTextRect(m_wndPagesEdit);
	int nIconBtnW = TAECHANG_CALC_ICON_BTN_W;
	int nIconBtnH = TAECHANG_CALC_ICON_BTN_H;
	int nIconBtnGap = TAECHANG_CALC_ICON_BTN_GAP;
	int nIconBtnTopPad = (nInputPanelH - nIconBtnH * 2 - nIconBtnGap) / 2;
	int nBtnX = m_rectInputCard.right + TAECHANG_ROW_GAP;
	if (nBtnX + nIconBtnW > nX + nW)
		nBtnX = nX + nW - nIconBtnW;
	m_wndCalcBtn.MoveWindow(nBtnX, nY + nIconBtnTopPad, nIconBtnW, nIconBtnH);
	m_wndCalcResetBtn.MoveWindow(nBtnX, nY + nIconBtnTopPad + nIconBtnH + nIconBtnGap, nIconBtnW, nIconBtnH);

	nY += nInputPanelH + TAECHANG_CALC_SECTION_GAP;

	int nRowH = TAECHANG_EDIT_HEIGHT + TAECHANG_CALC_RESULT_ROW_GAP;
	int nDivH = TAECHANG_CALC_DIVIDER_HEIGHT;
	int nResultPanelH = nPad + nRowH + nRowH + nDivH + TAECHANG_CALC_RESULT_ROW_GAP
		+ nRowH + nRowH + nDivH + TAECHANG_CALC_RESULT_ROW_GAP
		+ TAECHANG_CALC_TOTAL_BAND_HEIGHT + TAECHANG_CALC_RESULT_ROW_GAP
		+ TAECHANG_CALC_RANGE_HINT_HEIGHT + nPad;

	int nRX = nX + nPad;
	int nRY = nY + nPad;
	int nValX = nRX + nLabelW + TAECHANG_LABEL_EDIT_GAP;
	int nResultPanelW = nInputPanelW;
	if (nResultPanelW > nW)
		nResultPanelW = nW;
	m_rectResultCard = CRect(nX, nY, nX + nResultPanelW, nY + nResultPanelH);
	int nContentW = nResultPanelW - nPad * 2;
	if (nContentW < nLabelW + TAECHANG_LABEL_EDIT_GAP + nValW)
		nContentW = nLabelW + TAECHANG_LABEL_EDIT_GAP + nValW;
	int nFreightUnitW = TAECHANG_CALC_FREIGHT_UNIT_WIDTH;
	int nFreightEditW = nValW - nFreightUnitW - TAECHANG_LABEL_EDIT_GAP;
	if (nFreightEditW < TAECHANG_CALC_FREIGHT_EDIT_MIN_W)
		nFreightEditW = TAECHANG_CALC_FREIGHT_EDIT_MIN_W;
	int nUnitRightX = nValX + nValW;

	m_wndPrintLabel.MoveWindow(nRX, nRY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndPrintValue.MoveWindow(nValX, nRY, nValW, TAECHANG_EDIT_HEIGHT);
	nRY += nRowH;

	m_wndCoverLabel.MoveWindow(nRX, nRY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndCoverValue.MoveWindow(nValX, nRY, nValW, TAECHANG_EDIT_HEIGHT);
	nRY += nRowH;

	m_wndDivider.MoveWindow(nRX, nRY, nContentW, nDivH);
	nRY += nDivH + TAECHANG_CALC_RESULT_ROW_GAP;

	m_wndSubtotalLabel.MoveWindow(nRX, nRY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndSubtotalValue.MoveWindow(nValX, nRY, nValW, TAECHANG_EDIT_HEIGHT);
	nRY += nRowH;

	m_wndFreightLabel.MoveWindow(nRX, nRY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndFreightUnitLabel.MoveWindow(nUnitRightX - nFreightUnitW, nRY, nFreightUnitW, TAECHANG_EDIT_HEIGHT);
	int nFreightEditGap = TAECHANG_CALC_FREIGHT_EDIT_GAP;
	m_wndFreightEdit.MoveWindow(nUnitRightX - nFreightUnitW - nFreightEditGap - nFreightEditW,
								nRY, nFreightEditW, TAECHANG_EDIT_HEIGHT);
	ApplyCalcEditTextRect(m_wndFreightEdit);
	nRY += nRowH;

	m_wndTotalDivider.MoveWindow(nRX, nRY, nContentW, nDivH);
	nRY += nDivH + TAECHANG_CALC_RESULT_ROW_GAP;

	m_rectTotalBand = CRect(nRX, nRY, nRX + nContentW, nRY + TAECHANG_CALC_TOTAL_BAND_HEIGHT);
	int nBandTextY = nRY + (TAECHANG_CALC_TOTAL_BAND_HEIGHT - TAECHANG_EDIT_HEIGHT) / 2;
	m_wndTotalLabel.MoveWindow(nRX + TAECHANG_CALC_TOTAL_BAND_PAD, nBandTextY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndTotalValue.MoveWindow(nValX, nBandTextY, nValW - TAECHANG_CALC_TOTAL_BAND_PAD, TAECHANG_EDIT_HEIGHT);
	nRY += TAECHANG_CALC_TOTAL_BAND_HEIGHT + TAECHANG_CALC_RESULT_ROW_GAP;

	m_wndRangeHint.MoveWindow(nRX, nRY, nContentW, TAECHANG_CALC_RANGE_HINT_HEIGHT);

	nY += nResultPanelH + TAECHANG_CALC_SECTION_GAP;

	int nHistoryW = TAECHANG_CALC_HIST_COL_COMPANY_W + TAECHANG_CALC_HIST_COL_ITEM_W
		+ TAECHANG_CALC_HIST_COL_DATE_W
		+ TAECHANG_CALC_HIST_COL_COPIES_W + TAECHANG_CALC_HIST_COL_PAGES_W
		+ TAECHANG_CALC_HIST_COL_PRINT_W + TAECHANG_CALC_HIST_COL_COVER_W
		+ TAECHANG_CALC_HIST_COL_FREIGHT_W + TAECHANG_CALC_HIST_COL_TOTAL_W
		+ TAECHANG_CALC_HIST_COL_TIME_W + ::GetSystemMetrics(SM_CXVSCROLL) + TAECHANG_CALC_HIST_WIDTH_PAD;
	if (nHistoryW > nW)
		nHistoryW = nW;
	m_wndHistorySection.MoveWindow(nX, nY, nHistoryW, TAECHANG_SECTION_TITLE_HEIGHT);
	nY += TAECHANG_SECTION_TITLE_HEIGHT + TAECHANG_PANEL_GAP;

	int nListH = nHeight - TAECHANG_MARGIN - nY;
	if (nListH < TAECHANG_RESULT_MIN_HEIGHT)
		nListH = TAECHANG_RESULT_MIN_HEIGHT;
	m_wndHistoryList.MoveWindow(nX, nY, nHistoryW, nListH);
	m_wndHistoryList.SetColumnWidth(0, TAECHANG_CALC_HIST_COL_COMPANY_W);
	m_wndHistoryList.SetColumnWidth(1, TAECHANG_CALC_HIST_COL_ITEM_W);
	m_wndHistoryList.SetColumnWidth(2, TAECHANG_CALC_HIST_COL_DATE_W);
	m_wndHistoryList.SetColumnWidth(3, TAECHANG_CALC_HIST_COL_COPIES_W);
	m_wndHistoryList.SetColumnWidth(4, TAECHANG_CALC_HIST_COL_PAGES_W);
	m_wndHistoryList.SetColumnWidth(5, TAECHANG_CALC_HIST_COL_PRINT_W);
	m_wndHistoryList.SetColumnWidth(6, TAECHANG_CALC_HIST_COL_COVER_W);
	m_wndHistoryList.SetColumnWidth(7, TAECHANG_CALC_HIST_COL_FREIGHT_W);
	m_wndHistoryList.SetColumnWidth(8, TAECHANG_CALC_HIST_COL_TOTAL_W);
	m_wndHistoryList.SetColumnWidth(9, TAECHANG_CALC_HIST_COL_TIME_W);
	int nHistoryCount = static_cast<int>(m_arrHistory.GetSize());
	TrimHistoryToVisibleCapacity();
	if (m_arrHistory.GetSize() != nHistoryCount)
		RefreshHistoryList();
}

BOOL SagePriceCalcPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
	if (!m_rectInputCard.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectInputCard, TAECHANG_COLOR_PANEL);
		CBrush brBorder(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectInputCard, &brBorder);
	}
	if (!m_rectResultCard.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectResultCard, TAECHANG_COLOR_PANEL);
		CBrush brBorder(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectResultCard, &brBorder);
		CRect rectTotalDiv;
		m_wndTotalDivider.GetWindowRect(&rectTotalDiv);
		ScreenToClient(&rectTotalDiv);
		pDC->FillSolidRect(rectTotalDiv.left, rectTotalDiv.top - TAECHANG_CALC_DIVIDER_HEIGHT,
			rectTotalDiv.Width(), TAECHANG_CALC_DIVIDER_HEIGHT, TAECHANG_COLOR_BORDER);
	}
	if (!m_rectTotalBand.IsRectEmpty())
		pDC->FillSolidRect(m_rectTotalBand, TAECHANG_COLOR_ACCENT_SURFACE);
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
	pDC->FillSolidRect(rect.left, rect.top, rect.Width(), 1, TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.bottom - 1, rect.Width(), 1, TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.top, 1, rect.Height(), TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.right - 1, rect.top, 1, rect.Height(), TAECHANG_COLOR_BORDER);
}

HBRUSH SagePriceCalcPanel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hBrush = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
		return hBrush;
	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
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
	if (sageDBMgr.GetTaechangPriceService()->LoadAllCompanyNames(arrNames, strError) == FALSE)
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
	m_wndPrintValue.SetWindowTextW(TAECHANG_UI_PRICE_SUMMARY_EMPTY);
	m_wndCoverValue.SetWindowTextW(TAECHANG_UI_PRICE_SUMMARY_EMPTY);
	m_wndSubtotalValue.SetWindowTextW(TAECHANG_UI_PRICE_SUMMARY_EMPTY);
	m_wndTotalValue.SetWindowTextW(TAECHANG_UI_PRICE_SUMMARY_EMPTY);
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
		strHint.Format(TAECHANG_UI_CALC_RANGE_FMT, m_calcResult.nRangeMinCopies,
			m_calcResult.nRangeMaxCopies, strUnit.GetString(), strCover.GetString());
	else
		strHint.Format(TAECHANG_UI_CALC_RANGE_OPEN_FMT, m_calcResult.nRangeMinCopies,
			strUnit.GetString(), strCover.GetString());
	m_wndRangeHint.SetWindowTextW(strHint);
}

BOOL SagePriceCalcPanel::UpdatePreview(BOOL bShowMessage) {
	int nSel = m_wndCompanyCombo.GetCurSel();
	if (nSel == CB_ERR) {
		ClearResult();
		if (bShowMessage)
			ShowSageMessageBox(TAECHANG_UI_CALC_SELECT_COMPANY, MB_ICONWARNING);
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
			ShowSageMessageBox(TAECHANG_UI_CALC_COPIES_REQUIRED, MB_ICONWARNING);
		return FALSE;
	}

	SagePriceCalcService calcService(sageDBMgr.GetTaechangPriceService());
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
			ShowSageMessageBox(TAECHANG_UI_CALC_PAGES_REQUIRED, MB_ICONWARNING);
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
	strPrint.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(m_calcResult.nPrintPrice).GetString());
	strCover.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(m_calcResult.nCoverPrice).GetString());
	strSub.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(m_calcResult.nSubtotal).GetString());
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
		ShowSageMessageBox(TAECHANG_UI_CALC_COPIES_INVALID, MB_ICONWARNING);
		return;
	case SAGE_PRICE_CALC_COPIES_ABOVE_MAX:
		ShowSageMessageBox(TAECHANG_UI_PRICE_COPIES_OUT_OF_RANGE, MB_ICONWARNING);
		return;
	case SAGE_PRICE_CALC_PAGES_OUT_OF_RANGE:
		ShowSageMessageBox(TAECHANG_UI_CALC_PAGES_INVALID, MB_ICONWARNING);
		return;
	case SAGE_PRICE_CALC_NO_DATA:
		ShowSageMessageBox(TAECHANG_UI_CALC_NO_DATA, MB_ICONWARNING);
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

	SagePriceCalcService calcService(sageDBMgr.GetTaechangPriceService());
	calcService.ApplyFreight(PriceTextToInt(strFreight), m_calcResult);

	CString strTotal;
	strTotal.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(m_calcResult.nTotal).GetString());
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
		ShowSageMessageBox(TAECHANG_UI_CALC_ESTIMATE_SCRIPT_MISSING, MB_ICONERROR);
		return;
	}
	CString strTemplatePath = CombinePath(strPluginDir, TAECHANG_ESTIMATE_TEMPLATE_REL_PATH);
	CString strScriptPath   = CombinePath(strPluginDir, TAECHANG_CALC_ESTIMATE_SCRIPT_REL_PATH);

	if (!FileExists(strTemplatePath)) {
		ShowSageMessageBox(TAECHANG_UI_CALC_ESTIMATE_TEMPLATE_MISSING, MB_ICONERROR);
		return;
	}
	if (!FileExists(strScriptPath)) {
		ShowSageMessageBox(TAECHANG_UI_CALC_ESTIMATE_SCRIPT_MISSING, MB_ICONERROR);
		return;
	}

	TaechangCalcEstimateDlg dlg(strCompany, nCopies, nPages,
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

	TaechangCalcCompanyPickerDlg dlg(arrNames, strCurrent, this);
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
		return TAECHANG_CALC_MAX_HISTORY;

	int nCapacity = m_wndHistoryList.GetCountPerPage();
	if (nCapacity <= 0)
		return TAECHANG_CALC_MAX_HISTORY;
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
		strCount.Format(TAECHANG_UI_CALC_HIST_COUNT_FMT, nCount);
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
		strCopies.Format(TAECHANG_UI_CALC_HIST_COPIES_FMT, e.nCopies);
		m_wndHistoryList.SetItemText(i, 3, strCopies);

		CString strPages;
		strPages.Format(TAECHANG_UI_CALC_HIST_PAGES_FMT, e.nPages);
		m_wndHistoryList.SetItemText(i, 4, strPages);

		CString strPrint, strCover, strFreight, strTotal;
		strPrint.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(e.nPrintPrice).GetString());
		strCover.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(e.nCoverPrice).GetString());
		strFreight.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(e.nFreight).GetString());
		strTotal.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(e.nTotal).GetString());
		m_wndHistoryList.SetItemText(i, 5, strPrint);
		m_wndHistoryList.SetItemText(i, 6, strCover);
		m_wndHistoryList.SetItemText(i, 7, strFreight);
		m_wndHistoryList.SetItemText(i, 8, strTotal);

		CString strTime = e.timeCalc.Format(TAECHANG_UI_CALC_HIST_TIME_FMT);
		m_wndHistoryList.SetItemText(i, 9, strTime);
	}
	m_wndHistoryList.SetRedraw(TRUE);
	m_wndHistoryList.Invalidate();
}
