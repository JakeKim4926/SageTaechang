#include "pch.h"
#include "app/ui/panels/SageWorkflowInputPanel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "SageDefine.h"

BEGIN_MESSAGE_MAP(SageWorkflowInputPanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_SAGE_SELECT_INPUT, &SageWorkflowInputPanel::OnSelectInput)
	ON_BN_CLICKED(ID_SAGE_SELECT_OUTPUT, &SageWorkflowInputPanel::OnSelectOutput)
	ON_BN_CLICKED(ID_SAGE_GENERATE_WORKFLOW, &SageWorkflowInputPanel::OnGenerateWorkflow)
	ON_BN_CLICKED(ID_SAGE_INPUT_RESET_BTN, &SageWorkflowInputPanel::OnInputReset)
	ON_BN_CLICKED(ID_SAGE_OPEN_OUTPUT_FOLDER, &SageWorkflowInputPanel::OnOpenOutputFolder)
	ON_MESSAGE(WM_SAGE_RESULT_TABLE_CHANGED, &SageWorkflowInputPanel::OnResultTableChanged)
	ON_MESSAGE(WM_SAGE_RESULT_SELECTION_CHANGED, &SageWorkflowInputPanel::OnResultSelectionChanged)
END_MESSAGE_MAP()

SageWorkflowInputPanel::SageWorkflowInputPanel()
	: m_bAutoLoadOnInput(FALSE)
	, m_bRunning(FALSE)
	, m_bInputResetVisible(FALSE)
	, m_bTableVisible(FALSE)
	, m_nProgressPercent(0) {
}

static void AcceptDroppedFiles(CWnd& wnd) {
	HWND hWnd = wnd.GetSafeHwnd();
	if (hWnd == NULL || !::IsWindow(hWnd))
		return;

	::DragAcceptFiles(hWnd, TRUE);
	::ChangeWindowMessageFilterEx(hWnd, WM_DROPFILES, MSGFLT_ALLOW, NULL);
	::ChangeWindowMessageFilterEx(hWnd, WM_COPYDATA, MSGFLT_ALLOW, NULL);
	::ChangeWindowMessageFilterEx(hWnd, WM_SAGE_COPYGLOBALDATA, MSGFLT_ALLOW, NULL);
}

BOOL SageWorkflowInputPanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD | WS_CLIPCHILDREN, rectEmpty, pParent, nId);
}

void SageWorkflowInputPanel::EnableFileDrop() {
	AcceptDroppedFiles(*this);
	AcceptDroppedFiles(m_wndCardHeader);
	AcceptDroppedFiles(m_wndInputPath);
	AcceptDroppedFiles(m_wndSelectInput);
	AcceptDroppedFiles(m_wndStatusCard);
	AcceptDroppedFiles(m_wndEmptyStateHint);
	m_panelInputTable.EnableFileDrop();
}

SageResultTablePanel& SageWorkflowInputPanel::GetInputTable() {
	return m_panelInputTable;
}

int SageWorkflowInputPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CreateControls();
	ApplyControlFonts();
	ApplyLabelRoles();
	return 0;
}

void SageWorkflowInputPanel::CreateControls() {
	CRect r(0, 0, 0, 0);
	m_wndCardHeader.Create(SAGE_UI_INPUT_CARD_TITLE, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this, ID_SAGE_INPUT_SECTION);
	m_wndInputLabel.Create(SAGE_UI_SECTION_INPUT, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndOutputLabel.Create(SAGE_UI_SECTION_OUTPUT, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndInputPath.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY, r, this, ID_SAGE_INPUT_EDIT);
	m_wndOutputFolder.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY, r, this, ID_SAGE_OUTPUT_EDIT);
	m_wndSelectInput.Create(SAGE_UI_INPUT_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_SAGE_SELECT_INPUT);
	m_wndSelectOutput.Create(SAGE_UI_OUTPUT_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_SAGE_SELECT_OUTPUT);
	m_wndGenerate.Create(SAGE_UI_RECEIVABLES_GENERATE_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_SAGE_GENERATE_WORKFLOW);
	m_wndGenerate.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndInputReset.Create(SAGE_UI_INPUT_RESET_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_SAGE_INPUT_RESET_BTN);
	m_wndInputReset.SetVariant(SAGE_BUTTON_GHOST);
	m_wndInputReset.SetSurfaceColor(SAGE_COLOR_PANEL);
	m_wndStatusCard.Create(L"", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this, ID_SAGE_STATUS_CARD);
	m_wndStatusCard.SetOpenFolderCommand(ID_SAGE_OPEN_OUTPUT_FOLDER);
	m_wndStatusCard.SetIdle(SAGE_UI_STATUS_CARD_IDLE);
	m_wndEmptyStateHint.Create(SAGE_UI_EMPTY_STATE_HINT, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, r, this);
	m_wndEmptyStateHint.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndEmptyStateHint.SetFontRole(SAGE_FONT_CONTENT);

	m_panelInputTable.Create(this, ID_SAGE_INPUT_TABLE_PANEL);
	m_panelInputTable.SetTitle(NULL);
}

void SageWorkflowInputPanel::ApplyControlFonts() {
	m_wndCardHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_HEADER));
	m_wndInputPath.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndOutputFolder.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSelectInput.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSelectOutput.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndGenerate.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndInputReset.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
}

void SageWorkflowInputPanel::ApplyLabelRoles() {
	m_wndInputLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndInputLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndInputLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndOutputLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndOutputLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndOutputLabel.SetFontRole(SAGE_FONT_CONTENT);
}

int SageWorkflowInputPanel::GetContentWidth() const {
	CRect rectClient;
	GetClientRect(&rectClient);
	return rectClient.Width() - SAGE_EDIT_BORDER_WIDTH;
}

int SageWorkflowInputPanel::GetInputCardHeight() const {
	return SAGE_CARD_HEADER_HEIGHT
		+ SAGE_CARD_PADDING
		+ SAGE_EDIT_HEIGHT + SAGE_CARD_ROW_GAP
		+ SAGE_EDIT_HEIGHT + SAGE_CARD_ROW_GAP
		+ SAGE_CARD_ACTION_BUTTON_HEIGHT
		+ SAGE_CARD_PADDING;
}

int SageWorkflowInputPanel::GetTableAreaTop() const {
	return GetInputCardHeight() + SAGE_CARD_GAP + SAGE_STATUS_CARD_HEIGHT + SAGE_CARD_GAP;
}

void SageWorkflowInputPanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	LayoutInputCard(GetContentWidth());
	LayoutActionSection();
	LayoutTableArea();
}

void SageWorkflowInputPanel::LayoutTableArea() {
	if (!::IsWindow(m_panelInputTable.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	int nWidth = GetContentWidth();
	if (nWidth <= 0)
		return;

	int nTop = GetTableAreaTop();
	int nBodyHeight = max(SAGE_RESULT_MIN_HEIGHT, rectClient.Height() - nTop - SAGE_RESULT_HEADER_HEIGHT);
	m_panelInputTable.Layout(CRect(
		0,
		nTop - m_panelInputTable.GetBandHeight(),
		nWidth,
		nTop + SAGE_RESULT_HEADER_HEIGHT + nBodyHeight));
	m_wndEmptyStateHint.MoveWindow(0, nTop, nWidth, nBodyHeight);
}

void SageWorkflowInputPanel::LayoutFormRow(int nTop, int nWidth, CSageLabel& wndLabel, CEdit& wndEdit, CSageButton& wndButton) {
	int nLabelLeft = SAGE_CARD_PADDING;
	int nEditLeft = nLabelLeft + SAGE_FORM_LABEL_WIDTH + SAGE_CARD_ROW_GAP;
	int nButtonLeft = nWidth - SAGE_CARD_PADDING - SAGE_BUTTON_WIDTH;
	int nEditWidth = nButtonLeft - SAGE_CARD_ROW_GAP - nEditLeft;
	if (nEditWidth < SAGE_CO_COMPANY_EDIT_MIN_WIDTH)
		nEditWidth = SAGE_CO_COMPANY_EDIT_MIN_WIDTH;

	wndLabel.MoveWindow(nLabelLeft, nTop, SAGE_FORM_LABEL_WIDTH, SAGE_EDIT_HEIGHT);
	wndEdit.MoveWindow(nEditLeft, nTop, nEditWidth, SAGE_EDIT_HEIGHT);
	ApplyEditTextRect(wndEdit);
	wndButton.MoveWindow(nButtonLeft, nTop, SAGE_BUTTON_WIDTH, SAGE_BUTTON_HEIGHT);
}

void SageWorkflowInputPanel::LayoutInputCard(int nWidth) {
	m_wndCardHeader.MoveWindow(0, 0, nWidth, SAGE_CARD_HEADER_HEIGHT);

	int nTop = SAGE_CARD_HEADER_HEIGHT + SAGE_CARD_PADDING;
	LayoutFormRow(nTop, nWidth, m_wndInputLabel, m_wndInputPath, m_wndSelectInput);
	nTop += SAGE_EDIT_HEIGHT + SAGE_CARD_ROW_GAP;
	LayoutFormRow(nTop, nWidth, m_wndOutputLabel, m_wndOutputFolder, m_wndSelectOutput);
}

void SageWorkflowInputPanel::LayoutActionSection() {
	if (!::IsWindow(m_wndGenerate.GetSafeHwnd()))
		return;

	int nWidth = GetContentWidth();
	if (nWidth <= 0)
		return;

	int nActionTop = SAGE_CARD_HEADER_HEIGHT + SAGE_CARD_PADDING
		+ (SAGE_EDIT_HEIGHT + SAGE_CARD_ROW_GAP) * 2;
	int nX = SAGE_CARD_PADDING + SAGE_FORM_LABEL_WIDTH + SAGE_CARD_ROW_GAP;
	m_wndGenerate.MoveWindow(nX, nActionTop, SAGE_BUTTON_WIDTH, SAGE_CARD_ACTION_BUTTON_HEIGHT);
	nX += SAGE_BUTTON_WIDTH + SAGE_ACTION_GAP;
	if (m_bInputResetVisible)
		m_wndInputReset.MoveWindow(nX, nActionTop, SAGE_INPUT_RESET_WIDTH, SAGE_CARD_ACTION_BUTTON_HEIGHT);

	m_wndStatusCard.MoveWindow(
		0,
		GetInputCardHeight() + SAGE_CARD_GAP,
		nWidth,
		SAGE_STATUS_CARD_HEIGHT);
}

void SageWorkflowInputPanel::ApplyEditTextRect(CEdit& wndEdit) {
	CRect rcFmt;
	wndEdit.GetClientRect(&rcFmt);
	rcFmt.left += SAGE_EDIT_TEXT_LEFT_PAD;
	rcFmt.top += SAGE_EDIT_TEXT_TOP_PAD;
	rcFmt.right = SAGE_EDIT_FORMAT_MAX_WIDTH;
	wndEdit.SendMessage(EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcFmt));
}

BOOL SageWorkflowInputPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, SAGE_COLOR_APP_BACKGROUND);

	int nCardWidth = GetContentWidth();
	int nCardHeight = GetInputCardHeight();
	if (nCardWidth > 0) {
		CRect rectCard(0, 0, nCardWidth, nCardHeight);
		pDC->FillSolidRect(rectCard, SAGE_COLOR_PANEL);
		CBrush brushCard(SAGE_COLOR_BORDER);
		pDC->FrameRect(rectCard, &brushCard);
	}

	DrawEditBorder(pDC, m_wndInputPath);
	DrawEditBorder(pDC, m_wndOutputFolder);
	return TRUE;
}

void SageWorkflowInputPanel::DrawEditBorder(CDC* pDC, CWnd& wnd) {
	if (!::IsWindow(wnd.GetSafeHwnd()) || !wnd.IsWindowVisible())
		return;
	CRect rect;
	wnd.GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.InflateRect(SAGE_EDIT_BORDER_WIDTH, SAGE_EDIT_BORDER_WIDTH);
	pDC->FillSolidRect(rect.left, rect.top, rect.Width(), SAGE_EDIT_BORDER_WIDTH, SAGE_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.bottom - SAGE_EDIT_BORDER_WIDTH, rect.Width(), SAGE_EDIT_BORDER_WIDTH, SAGE_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.top, SAGE_EDIT_BORDER_WIDTH, rect.Height(), SAGE_COLOR_BORDER);
	pDC->FillSolidRect(rect.right - SAGE_EDIT_BORDER_WIDTH, rect.top, SAGE_EDIT_BORDER_WIDTH, rect.Height(), SAGE_COLOR_BORDER);
}

HBRUSH SageWorkflowInputPanel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
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

void SageWorkflowInputPanel::SetSectionLabel(LPCWSTR pszLabel) {
	m_wndInputLabel.SetWindowTextW(pszLabel);
}

void SageWorkflowInputPanel::SetActionButtonLabel(LPCWSTR pszLabel) {
	m_wndGenerate.SetWindowTextW(pszLabel);
}

void SageWorkflowInputPanel::SetInputDialogTitle(LPCWSTR pszTitle) {
	m_strInputDialogTitle = pszTitle;
}

void SageWorkflowInputPanel::SetAutoLoadOnInput(BOOL bAutoLoad) {
	m_bAutoLoadOnInput = bAutoLoad;
}

CString SageWorkflowInputPanel::GetInputPath() const {
	CString strInputPath;
	if (::IsWindow(m_wndInputPath.GetSafeHwnd()))
		m_wndInputPath.GetWindowTextW(strInputPath);
	return strInputPath;
}

CString SageWorkflowInputPanel::GetOutputFolder() const {
	CString strOutputFolder;
	if (::IsWindow(m_wndOutputFolder.GetSafeHwnd()))
		m_wndOutputFolder.GetWindowTextW(strOutputFolder);
	return strOutputFolder;
}

void SageWorkflowInputPanel::SetInputPath(const CString& strInputPath) {
	if (::IsWindow(m_wndInputPath.GetSafeHwnd()))
		m_wndInputPath.SetWindowTextW(strInputPath);
}

void SageWorkflowInputPanel::SetOutputFolder(const CString& strOutputFolder) {
	if (::IsWindow(m_wndOutputFolder.GetSafeHwnd()))
		m_wndOutputFolder.SetWindowTextW(strOutputFolder);
}

void SageWorkflowInputPanel::SetRunningState(BOOL bRunning) {
	m_bRunning = bRunning;
	m_wndSelectInput.EnableWindow(!bRunning);
	m_wndSelectOutput.EnableWindow(!bRunning);
	m_wndInputReset.EnableWindow(!bRunning);
	m_panelInputTable.EnableSelectionControls(!bRunning);
	if (bRunning) {
		m_wndStatusCard.SetRunning(SAGE_UI_STATUS_CARD_RUNNING);
		UpdateProgressPercent(0);
		SetTimer(ID_SAGE_PROGRESS_TIMER, SAGE_PROGRESS_TIMER_MS, NULL);
		return;
	}
	KillTimer(ID_SAGE_PROGRESS_TIMER);
}

void SageWorkflowInputPanel::UpdateActionVisibility(BOOL bInputResetVisible) {
	m_bInputResetVisible = bInputResetVisible;
	m_wndInputReset.ShowWindow(bInputResetVisible ? SW_SHOW : SW_HIDE);
	LayoutActionSection();
}

void SageWorkflowInputPanel::UpdateInputTableVisibility(BOOL bTableVisible, BOOL bOnePageVisible, BOOL bFilterVisible) {
	m_bTableVisible = bTableVisible;
	m_panelInputTable.ShowSelectAll(bTableVisible);
	m_panelInputTable.ShowOnePageOption(bTableVisible && bOnePageVisible);
	m_panelInputTable.ShowFilter(bTableVisible && bFilterVisible);
	m_panelInputTable.ShowWindow(bTableVisible ? SW_SHOW : SW_HIDE);

	BOOL bShowHint = (!bTableVisible && !m_bRunning) ? TRUE : FALSE;
	m_wndEmptyStateHint.ShowWindow(bShowHint ? SW_SHOW : SW_HIDE);
	LayoutTableArea();
}

LRESULT SageWorkflowInputPanel::ForwardToParent(UINT nMessage, WPARAM wParam, LPARAM lParam) {
	CWnd* pParent = GetParent();
	if (pParent == NULL || !::IsWindow(pParent->GetSafeHwnd()))
		return 0;
	return pParent->SendMessage(nMessage, wParam, lParam);
}

LRESULT SageWorkflowInputPanel::OnResultTableChanged(WPARAM wParam, LPARAM lParam) {
	return ForwardToParent(WM_SAGE_RESULT_TABLE_CHANGED, wParam, lParam);
}

LRESULT SageWorkflowInputPanel::OnResultSelectionChanged(WPARAM wParam, LPARAM lParam) {
	return ForwardToParent(WM_SAGE_RESULT_SELECTION_CHANGED, wParam, lParam);
}

void SageWorkflowInputPanel::EnableGenerateButton(BOOL bEnable) {
	m_wndGenerate.EnableWindow(bEnable);
}

void SageWorkflowInputPanel::ResetStatusCard() {
	m_wndStatusCard.SetIdle(SAGE_UI_STATUS_CARD_IDLE);
}

void SageWorkflowInputPanel::SetStatusResult(BOOL bSuccess, const CString& strMessage, const CString& strDetail) {
	m_wndStatusCard.SetResult(bSuccess, strMessage, strDetail);
}

void SageWorkflowInputPanel::UpdateProgressPercent(int nPercent) {
	m_nProgressPercent = nPercent;
	m_wndStatusCard.SetProgressPercent(m_nProgressPercent);
}

void SageWorkflowInputPanel::OnTimer(UINT_PTR nIDEvent) {
	if (nIDEvent == ID_SAGE_PROGRESS_TIMER) {
		if (m_bRunning && m_nProgressPercent < SAGE_PROGRESS_RUNNING_MAX) {
			int nNextPercent = m_nProgressPercent + SAGE_PROGRESS_STEP;
			if (nNextPercent > SAGE_PROGRESS_RUNNING_MAX)
				nNextPercent = SAGE_PROGRESS_RUNNING_MAX;
			UpdateProgressPercent(nNextPercent);
		}
		return;
	}
	CWnd::OnTimer(nIDEvent);
}

void SageWorkflowInputPanel::RequestRun(int nTaskType) {
	CWnd* pParent = GetParent();
	if (pParent == NULL || !::IsWindow(pParent->GetSafeHwnd()))
		return;
	pParent->SendMessage(WM_SAGE_WORKFLOW_RUN_REQUESTED, static_cast<WPARAM>(nTaskType), 0);
}

void SageWorkflowInputPanel::OnSelectInput() {
	CFileDialog dlg(TRUE, SAGE_UI_EXCEL_DEFAULT_EXT, NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, SAGE_UI_EXCEL_FILTER, this);
	dlg.m_ofn.lpstrTitle = m_strInputDialogTitle;
	if (dlg.DoModal() != IDOK)
		return;

	m_wndInputPath.SetWindowTextW(dlg.GetPathName());
	if (m_bAutoLoadOnInput)
		RequestRun(SAGE_TASK_LOAD);
}

void SageWorkflowInputPanel::OnSelectOutput() {
	CFolderPickerDialog dlg(NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, this, 0);
	dlg.m_ofn.lpstrTitle = SAGE_UI_SELECT_OUTPUT_TITLE;
	if (dlg.DoModal() == IDOK)
		m_wndOutputFolder.SetWindowTextW(dlg.GetPathName());
}

void SageWorkflowInputPanel::OnGenerateWorkflow() {
	RequestRun(SAGE_TASK_GENERATE);
}

void SageWorkflowInputPanel::OnInputReset() {
	ForwardToParent(WM_SAGE_WORKFLOW_INPUT_RESET, 0, 0);
}

void SageWorkflowInputPanel::OnOpenOutputFolder() {
	ForwardToParent(WM_SAGE_OPEN_OUTPUT_FOLDER, 0, 0);
}
