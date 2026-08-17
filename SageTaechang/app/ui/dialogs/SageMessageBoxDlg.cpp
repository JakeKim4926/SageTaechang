#include "pch.h"
#include "app/ui/dialogs/SageMessageBoxDlg.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(SageMessageBoxDlg, SageFramelessDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDYES, &SageMessageBoxDlg::OnYesClicked)
	ON_BN_CLICKED(IDNO, &SageMessageBoxDlg::OnNoClicked)
END_MESSAGE_MAP()

SageMessageBoxDlg::SageMessageBoxDlg(const CString& strMessage, UINT nType, CWnd* pParent)
	: SageFramelessDialog(pParent), m_strMessage(strMessage), m_nType(nType) {
}

INT_PTR SageMessageBoxDlg::DoModal() {
	BYTE* pTemplate = BuildFramelessTemplate(GetCaptionTitle(),
		TAECHANG_MSGBOX_TEMPLATE_CX, TAECHANG_MSGBOX_TEMPLATE_CY);
	InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
	INT_PTR nResult = CDialog::DoModal();
	delete[] pTemplate;
	return nResult;
}

BOOL SageMessageBoxDlg::IsConfirm() const {
	return ((m_nType & MB_TYPEMASK) == MB_YESNO) ? TRUE : FALSE;
}

BOOL SageMessageBoxDlg::IsDefaultReject() const {
	if (!IsConfirm())
		return FALSE;
	return ((m_nType & MB_DEFBUTTON2) == MB_DEFBUTTON2) ? TRUE : FALSE;
}

SageMessageIcon SageMessageBoxDlg::GetMessageIcon() const {
	UINT nIconType = m_nType & MB_ICONMASK;
	if (nIconType == MB_ICONERROR)
		return SAGE_MESSAGE_ICON_ERROR;
	if (nIconType == MB_ICONWARNING || nIconType == MB_ICONQUESTION)
		return SAGE_MESSAGE_ICON_WARNING;
	if (nIconType == MB_ICONINFORMATION)
		return SAGE_MESSAGE_ICON_INFO;
	return SAGE_MESSAGE_ICON_NONE;
}

LPCWSTR SageMessageBoxDlg::GetCaptionTitle() const {
	if (IsConfirm())
		return TAECHANG_UI_MSGBOX_TITLE_CONFIRM;

	SageMessageIcon nIcon = GetMessageIcon();
	if (nIcon == SAGE_MESSAGE_ICON_ERROR)
		return TAECHANG_UI_MSGBOX_TITLE_ERROR;
	if (nIcon == SAGE_MESSAGE_ICON_WARNING)
		return TAECHANG_UI_MSGBOX_TITLE_WARNING;
	return TAECHANG_UI_MSGBOX_TITLE_INFO;
}

BOOL SageMessageBoxDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	SetWindowText(GetCaptionTitle());
	m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_PANEL);

	CreateCaptionBar(GetCaptionTitle());
	CreateControls();
	ApplyStyle();
	SizeFramelessClient(TAECHANG_MSGBOX_WIDTH, LayoutControls());

	if (IsDefaultReject())
		m_wndRejectBtn.SetFocus();
	else
		m_wndAcceptBtn.SetFocus();

	return FALSE;
}

void SageMessageBoxDlg::CreateControls() {
	CRect rectEmpty(0, 0, 0, 0);

	m_wndBody.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this);

	if (IsConfirm()) {
		m_wndAcceptBtn.Create(TAECHANG_UI_MSGBOX_YES,
			WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDYES);
		m_wndRejectBtn.Create(TAECHANG_UI_MSGBOX_NO,
			WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDNO);
		return;
	}

	m_wndAcceptBtn.Create(TAECHANG_UI_MSGBOX_OK,
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDOK);
}

void SageMessageBoxDlg::ApplyStyle() {
	m_wndBody.SetMessage(m_strMessage, GetMessageIcon());

	m_wndAcceptBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTROL));
	m_wndAcceptBtn.SetVariant(IsConfirm() ? SAGE_BUTTON_DANGER : SAGE_BUTTON_PRIMARY);

	if (!IsConfirm())
		return;

	m_wndRejectBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTROL));
	m_wndRejectBtn.SetVariant(SAGE_BUTTON_SECONDARY);
}

int SageMessageBoxDlg::LayoutControls() {
	int nM = TAECHANG_MARGIN;
	int nGap = TAECHANG_ROW_GAP;
	int nBtnW = TAECHANG_LOGIN_DLG_BTN_WIDTH;
	int nBtnH = TAECHANG_BUTTON_HEIGHT;
	int nClientW = TAECHANG_MSGBOX_WIDTH;
	int nBodyW = nClientW - nM * 2;

	int nBodyTop = GetContentTop() + nM;
	int nBodyHeight = m_wndBody.MeasureHeight(nBodyW);
	m_wndBody.MoveWindow(nM, nBodyTop, nBodyW, nBodyHeight);

	int nBtnTop = nBodyTop + nBodyHeight + nM;
	int nBtnRight = nClientW - nM;

	if (IsConfirm()) {
		m_wndRejectBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
		m_wndAcceptBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);
	}
	else {
		m_wndAcceptBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
	}

	return nBtnTop + nBtnH + nM;
}

void SageMessageBoxDlg::OnYesClicked() {
	EndDialog(IDYES);
}

void SageMessageBoxDlg::OnNoClicked() {
	EndDialog(IDNO);
}

void SageMessageBoxDlg::OnOK() {
	if (IsConfirm()) {
		EndDialog(IsDefaultReject() ? IDNO : IDYES);
		return;
	}

	CDialog::OnOK();
}

void SageMessageBoxDlg::OnCancel() {
	if (IsConfirm()) {
		EndDialog(IDNO);
		return;
	}

	CDialog::OnCancel();
}

HBRUSH SageMessageBoxDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
	pDC->SetBkColor(TAECHANG_COLOR_PANEL);
	return m_brushBackground;
}

int ShowSageMessageBox(LPCWSTR pszText, UINT nType, CWnd* pParent) {
	CWnd* pOwner = (pParent != NULL) ? pParent : AfxGetMainWnd();
	SageMessageBoxDlg dlg(pszText, nType, pOwner);
	return (int)dlg.DoModal();
}
