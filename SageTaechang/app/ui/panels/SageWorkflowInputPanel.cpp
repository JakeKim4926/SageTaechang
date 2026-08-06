#include "pch.h"
#include "app/ui/panels/SageWorkflowInputPanel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(SageWorkflowInputPanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_TAECHANG_SELECT_INPUT, &SageWorkflowInputPanel::OnSelectInput)
	ON_BN_CLICKED(ID_TAECHANG_SELECT_OUTPUT, &SageWorkflowInputPanel::OnSelectOutput)
	ON_BN_CLICKED(ID_TAECHANG_LOAD_WORKFLOW, &SageWorkflowInputPanel::OnLoadWorkflow)
	ON_BN_CLICKED(ID_TAECHANG_GENERATE_WORKFLOW, &SageWorkflowInputPanel::OnGenerateWorkflow)
	ON_BN_CLICKED(ID_TAECHANG_INPUT_RESET_BTN, &SageWorkflowInputPanel::OnInputReset)
	ON_MESSAGE(WM_TAECHANG_RESULT_TABLE_CHANGED, &SageWorkflowInputPanel::OnResultTableChanged)
	ON_MESSAGE(WM_TAECHANG_RESULT_SELECTION_CHANGED, &SageWorkflowInputPanel::OnResultSelectionChanged)
END_MESSAGE_MAP()

SageWorkflowInputPanel::SageWorkflowInputPanel()
	: m_bAutoLoadOnInput(FALSE)
	, m_bRunning(FALSE)
	, m_bInputResetVisible(FALSE)
	, m_bTableVisible(FALSE)
	, m_bLastActionSuccess(FALSE)
	, m_nProgressPercent(0) {
}

static void AcceptDroppedFiles(CWnd& wnd) {
	HWND hWnd = wnd.GetSafeHwnd();
	if (hWnd == NULL || !::IsWindow(hWnd))
		return;

	::DragAcceptFiles(hWnd, TRUE);
	::ChangeWindowMessageFilterEx(hWnd, WM_DROPFILES, MSGFLT_ALLOW, NULL);
	::ChangeWindowMessageFilterEx(hWnd, WM_COPYDATA, MSGFLT_ALLOW, NULL);
	::ChangeWindowMessageFilterEx(hWnd, WM_TAECHANG_COPYGLOBALDATA, MSGFLT_ALLOW, NULL);
}

BOOL SageWorkflowInputPanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD | WS_CLIPCHILDREN, rectEmpty, pParent, nId);
}

void SageWorkflowInputPanel::EnableFileDrop() {
	AcceptDroppedFiles(*this);
	AcceptDroppedFiles(m_wndInputSection);
	AcceptDroppedFiles(m_wndInputPath);
	AcceptDroppedFiles(m_wndSelectInput);
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
	UpdateProgressPercent(0);
	return 0;
}

void SageWorkflowInputPanel::CreateControls() {
	CRect r(0, 0, 0, 0);
	m_wndInputSection.Create(TAECHANG_UI_SECTION_INPUT, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this, ID_TAECHANG_INPUT_SECTION);
	m_wndOutputSection.Create(TAECHANG_UI_SECTION_OUTPUT, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this, ID_TAECHANG_OUTPUT_SECTION);
	m_wndWorkflowLabel.Create(TAECHANG_UI_WORKFLOW_LABEL, WS_CHILD, r, this);
	m_wndInputLabel.Create(TAECHANG_UI_INPUT_LABEL, WS_CHILD | WS_VISIBLE, r, this);
	m_wndOutputLabel.Create(TAECHANG_UI_OUTPUT_LABEL, WS_CHILD | WS_VISIBLE, r, this);
	m_wndInputPath.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY, r, this, ID_TAECHANG_INPUT_EDIT);
	m_wndOutputFolder.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY, r, this, ID_TAECHANG_OUTPUT_EDIT);
	m_wndSelectInput.Create(TAECHANG_UI_INPUT_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_TAECHANG_SELECT_INPUT);
	m_wndSelectOutput.Create(TAECHANG_UI_OUTPUT_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_TAECHANG_SELECT_OUTPUT);
	m_wndLoad.Create(TAECHANG_UI_LOAD_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_TAECHANG_LOAD_WORKFLOW);
	m_wndGenerate.Create(TAECHANG_UI_RECEIVABLES_GENERATE_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_TAECHANG_GENERATE_WORKFLOW);
	m_wndGenerate.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndInputReset.Create(TAECHANG_UI_INPUT_RESET_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_TAECHANG_INPUT_RESET_BTN);
	m_wndInputReset.SetIcon(SAGE_BUTTON_ICON_RESET);
	m_wndInputReset.SetTooltip(TAECHANG_UI_TIP_RESET);
	m_wndInputReset.SetVariant(SAGE_BUTTON_GHOST);
	m_wndInputReset.SetSurfaceColor(TAECHANG_COLOR_APP_BACKGROUND);
	m_wndProgress.Create(WS_CHILD | WS_VISIBLE | PBS_MARQUEE, r, this, ID_TAECHANG_PROGRESS);
	m_wndProgress.SetMarquee(FALSE, 0);
	m_wndProgress.SetRange(0, TAECHANG_PROGRESS_COMPLETE);
	m_wndProgressText.Create(L"", WS_CHILD | WS_VISIBLE | SS_RIGHT, r, this);
	m_wndActionStatus.Create(L"", WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndEmptyStateHint.Create(TAECHANG_UI_EMPTY_STATE_HINT, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, r, this);
	m_wndEmptyStateHint.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndEmptyStateHint.SetFontRole(SAGE_FONT_CONTENT);

	m_panelInputTable.Create(this, ID_TAECHANG_INPUT_TABLE_PANEL);
	m_panelInputTable.SetTitle(NULL);
}

void SageWorkflowInputPanel::ApplyControlFonts() {
	m_wndInputSection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndOutputSection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndInputPath.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndOutputFolder.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSelectInput.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSelectOutput.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndLoad.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndGenerate.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndInputReset.SetFont(SageUiResources::GetFont(SAGE_FONT_HEADER));
	m_wndActionStatus.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
}

int SageWorkflowInputPanel::GetContentWidth() const {
	CRect rectClient;
	GetClientRect(&rectClient);
	return rectClient.Width() - TAECHANG_EDIT_BORDER_WIDTH;
}

int SageWorkflowInputPanel::GetTableAreaTop() const {
	return TAECHANG_INPUT_PANEL_HEIGHT + TAECHANG_PANEL_GAP + TAECHANG_BUTTON_HEIGHT + TAECHANG_PANEL_GAP;
}

void SageWorkflowInputPanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	LayoutInputSection(GetContentWidth());
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
	int nBodyHeight = max(TAECHANG_RESULT_MIN_HEIGHT, rectClient.Height() - nTop - TAECHANG_RESULT_HEADER_HEIGHT);
	m_panelInputTable.Layout(CRect(
		0,
		nTop - m_panelInputTable.GetBandHeight(),
		nWidth,
		nTop + TAECHANG_RESULT_HEADER_HEIGHT + nBodyHeight));
	m_wndEmptyStateHint.MoveWindow(0, nTop, nWidth, nBodyHeight);
}

void SageWorkflowInputPanel::LayoutInputSection(int nWidth) {
	int nTop = 0;
	int nPathWidth = nWidth - TAECHANG_BUTTON_WIDTH - TAECHANG_ROW_GAP;
	int nEditLeft = TAECHANG_BUTTON_WIDTH + TAECHANG_ROW_GAP;

	m_wndInputSection.MoveWindow(0, nTop, nWidth, TAECHANG_SECTION_TITLE_HEIGHT);
	nTop += TAECHANG_SECTION_TITLE_HEIGHT + TAECHANG_ROW_GAP;
	m_wndSelectInput.MoveWindow(0, nTop - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	m_wndInputPath.MoveWindow(nEditLeft, nTop, nPathWidth, TAECHANG_EDIT_HEIGHT);
	ApplyEditTextRect(m_wndInputPath);

	nTop += TAECHANG_BUTTON_HEIGHT + TAECHANG_ROW_GAP;
	m_wndOutputSection.MoveWindow(0, nTop, nWidth, TAECHANG_SECTION_TITLE_HEIGHT);
	nTop += TAECHANG_SECTION_TITLE_HEIGHT + TAECHANG_ROW_GAP;
	m_wndSelectOutput.MoveWindow(0, nTop - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	m_wndOutputFolder.MoveWindow(nEditLeft, nTop, nPathWidth, TAECHANG_EDIT_HEIGHT);
	ApplyEditTextRect(m_wndOutputFolder);
}

void SageWorkflowInputPanel::LayoutActionSection() {
	if (!::IsWindow(m_wndGenerate.GetSafeHwnd()))
		return;

	int nWidth = GetContentWidth();
	if (nWidth <= 0)
		return;

	int nTop = TAECHANG_INPUT_PANEL_HEIGHT + TAECHANG_PANEL_GAP;
	int nX = 0;
	m_wndGenerate.MoveWindow(nX, nTop, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nX += TAECHANG_BUTTON_WIDTH + TAECHANG_ACTION_GAP;
	if (m_bInputResetVisible) {
		m_wndInputReset.MoveWindow(nX, nTop, TAECHANG_INPUT_RESET_WIDTH, TAECHANG_BUTTON_HEIGHT);
		nX += TAECHANG_INPUT_RESET_WIDTH + TAECHANG_ACTION_GAP;
	}

	int nProgressWidth = nWidth - nX - TAECHANG_PROGRESS_TEXT_WIDTH - TAECHANG_ACTION_GAP;
	if (nProgressWidth < 0)
		nProgressWidth = 0;
	int nStatusWidth = nProgressWidth + TAECHANG_ACTION_GAP + TAECHANG_PROGRESS_TEXT_WIDTH;
	m_wndProgress.MoveWindow(nX, nTop + TAECHANG_PROGRESS_VERT_OFFSET, nProgressWidth, TAECHANG_PROGRESS_HEIGHT);
	m_wndProgressText.MoveWindow(nX + nProgressWidth + TAECHANG_ACTION_GAP, nTop + TAECHANG_PROGRESS_TEXT_VERT_OFFSET, TAECHANG_PROGRESS_TEXT_WIDTH, TAECHANG_EDIT_HEIGHT);
	m_wndActionStatus.MoveWindow(nX, nTop + TAECHANG_LABEL_VERT_OFFSET, nStatusWidth, TAECHANG_EDIT_HEIGHT);
}

void SageWorkflowInputPanel::ApplyEditTextRect(CEdit& wndEdit) {
	CRect rcFmt;
	wndEdit.GetClientRect(&rcFmt);
	rcFmt.top += TAECHANG_EDIT_TEXT_TOP_PAD;
	rcFmt.right = TAECHANG_EDIT_FORMAT_MAX_WIDTH;
	wndEdit.SendMessage(EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcFmt));
}

BOOL SageWorkflowInputPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
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
	rect.InflateRect(TAECHANG_EDIT_BORDER_WIDTH, TAECHANG_EDIT_BORDER_WIDTH);
	pDC->FillSolidRect(rect.left, rect.top, rect.Width(), TAECHANG_EDIT_BORDER_WIDTH, TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.bottom - TAECHANG_EDIT_BORDER_WIDTH, rect.Width(), TAECHANG_EDIT_BORDER_WIDTH, TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.top, TAECHANG_EDIT_BORDER_WIDTH, rect.Height(), TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.right - TAECHANG_EDIT_BORDER_WIDTH, rect.top, TAECHANG_EDIT_BORDER_WIDTH, rect.Height(), TAECHANG_COLOR_BORDER);
}

HBRUSH SageWorkflowInputPanel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hBrush = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
		return hBrush;
	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
	if (pWnd->GetSafeHwnd() == m_wndActionStatus.GetSafeHwnd()) {
		pDC->SetTextColor(m_bLastActionSuccess ? TAECHANG_COLOR_SUCCESS : TAECHANG_COLOR_ERROR);
		pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
		return SageUiResources::GetBrush(SAGE_BG_APP);
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

void SageWorkflowInputPanel::SetSectionLabel(LPCWSTR pszLabel) {
	m_wndInputSection.SetWindowTextW(pszLabel);
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
	m_wndLoad.EnableWindow(!bRunning);
	m_wndInputReset.EnableWindow(!bRunning);
	m_panelInputTable.EnableSelectionControls(!bRunning);
	if (bRunning) {
		UpdateProgressPercent(0);
		SetTimer(ID_TAECHANG_PROGRESS_TIMER, TAECHANG_PROGRESS_TIMER_MS, NULL);
		return;
	}
	KillTimer(ID_TAECHANG_PROGRESS_TIMER);
	UpdateProgressPercent(TAECHANG_PROGRESS_COMPLETE);
}

void SageWorkflowInputPanel::UpdateActionVisibility(BOOL bInputResetVisible, BOOL bHasLastResult) {
	m_bInputResetVisible = bInputResetVisible;
	m_wndInputLabel.ShowWindow(SW_HIDE);
	m_wndOutputLabel.ShowWindow(SW_HIDE);
	m_wndLoad.ShowWindow(SW_HIDE);
	m_wndInputReset.ShowWindow(bInputResetVisible ? SW_SHOW : SW_HIDE);

	BOOL bShowActionStatus = (!m_bRunning && bHasLastResult && !bInputResetVisible) ? TRUE : FALSE;
	m_wndProgress.ShowWindow(m_bRunning ? SW_SHOW : SW_HIDE);
	m_wndProgressText.ShowWindow(m_bRunning ? SW_SHOW : SW_HIDE);
	m_wndActionStatus.ShowWindow(bShowActionStatus ? SW_SHOW : SW_HIDE);
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
	return ForwardToParent(WM_TAECHANG_RESULT_TABLE_CHANGED, wParam, lParam);
}

LRESULT SageWorkflowInputPanel::OnResultSelectionChanged(WPARAM wParam, LPARAM lParam) {
	return ForwardToParent(WM_TAECHANG_RESULT_SELECTION_CHANGED, wParam, lParam);
}

void SageWorkflowInputPanel::EnableGenerateButton(BOOL bEnable) {
	m_wndGenerate.EnableWindow(bEnable);
}

void SageWorkflowInputPanel::SetActionStatusText(LPCWSTR pszStatus, BOOL bSuccess) {
	m_bLastActionSuccess = bSuccess;
	m_wndActionStatus.SetWindowTextW(pszStatus);
	m_wndActionStatus.Invalidate();
}

void SageWorkflowInputPanel::UpdateProgressPercent(int nPercent) {
	m_nProgressPercent = nPercent;
	m_wndProgress.SetPos(m_nProgressPercent);
	CString strProgress;
	strProgress.Format(TAECHANG_UI_PROGRESS_FORMAT, m_nProgressPercent);
	m_wndProgressText.SetWindowTextW(strProgress);
}

void SageWorkflowInputPanel::OnTimer(UINT_PTR nIDEvent) {
	if (nIDEvent == ID_TAECHANG_PROGRESS_TIMER) {
		if (m_bRunning && m_nProgressPercent < TAECHANG_PROGRESS_RUNNING_MAX) {
			int nNextPercent = m_nProgressPercent + TAECHANG_PROGRESS_STEP;
			if (nNextPercent > TAECHANG_PROGRESS_RUNNING_MAX)
				nNextPercent = TAECHANG_PROGRESS_RUNNING_MAX;
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
	pParent->SendMessage(WM_TAECHANG_WORKFLOW_RUN_REQUESTED, static_cast<WPARAM>(nTaskType), 0);
}

void SageWorkflowInputPanel::OnSelectInput() {
	CFileDialog dlg(TRUE, TAECHANG_UI_EXCEL_DEFAULT_EXT, NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, TAECHANG_UI_EXCEL_FILTER, this);
	dlg.m_ofn.lpstrTitle = m_strInputDialogTitle;
	if (dlg.DoModal() != IDOK)
		return;

	m_wndInputPath.SetWindowTextW(dlg.GetPathName());
	if (m_bAutoLoadOnInput)
		RequestRun(TAECHANG_TASK_LOAD);
}

void SageWorkflowInputPanel::OnSelectOutput() {
	CFolderPickerDialog dlg(NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, this, 0);
	dlg.m_ofn.lpstrTitle = TAECHANG_UI_SELECT_OUTPUT_TITLE;
	if (dlg.DoModal() == IDOK)
		m_wndOutputFolder.SetWindowTextW(dlg.GetPathName());
}

void SageWorkflowInputPanel::OnLoadWorkflow() {
	RequestRun(TAECHANG_TASK_LOAD);
}

void SageWorkflowInputPanel::OnGenerateWorkflow() {
	RequestRun(TAECHANG_TASK_GENERATE);
}

void SageWorkflowInputPanel::OnInputReset() {
	CWnd* pParent = GetParent();
	if (pParent == NULL || !::IsWindow(pParent->GetSafeHwnd()))
		return;
	pParent->SendMessage(WM_TAECHANG_WORKFLOW_INPUT_RESET, 0, 0);
}
