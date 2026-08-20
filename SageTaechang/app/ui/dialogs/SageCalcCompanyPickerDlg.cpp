#include "pch.h"
#include "app/ui/dialogs/SageCalcCompanyPickerDlg.h"
#include "SageDefine.h"
#include "app/ui/drawing/SageUiResources.h"

BEGIN_MESSAGE_MAP(SageCalcCompanyPickerDlg, SageFramelessDialog)
    ON_WM_CTLCOLOR()
    ON_EN_CHANGE(ID_PICKER_DLG_SEARCH_EDIT, &SageCalcCompanyPickerDlg::OnSearchChanged)
    ON_BN_CLICKED(ID_PICKER_DLG_SEARCH_BTN, &SageCalcCompanyPickerDlg::OnSearchChanged)
    ON_LBN_DBLCLK(ID_PICKER_DLG_LIST, &SageCalcCompanyPickerDlg::OnListDblClick)
END_MESSAGE_MAP()

SageCalcCompanyPickerDlg::SageCalcCompanyPickerDlg(const CStringArray& arrNames, const CString& strInitialName, CWnd* pParent)
    : SageFramelessDialog(pParent), m_strInitialName(strInitialName) {
    m_arrAllNames.Copy(arrNames);
}

SageCalcCompanyPickerDlg::~SageCalcCompanyPickerDlg() {}

INT_PTR SageCalcCompanyPickerDlg::DoModal() {
    BYTE* pTemplate = BuildFramelessTemplate(SAGE_UI_PICKER_DLG_TITLE,
        SAGE_PICKER_DLG_TEMPLATE_CX, SAGE_PICKER_DLG_TEMPLATE_CY);
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

CString SageCalcCompanyPickerDlg::GetSelectedName() const {
    return m_strSelectedName;
}

BOOL SageCalcCompanyPickerDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    SetWindowText(SAGE_UI_PICKER_DLG_TITLE);

    m_brushBackground.CreateSolidBrush(SAGE_COLOR_PANEL);
    m_brushPanel.CreateSolidBrush(SAGE_COLOR_PANEL);

    CreateCaptionBar(SAGE_UI_PICKER_DLG_TITLE);
    CreateControls();
    ApplyFont();
    SizeFramelessClient(SAGE_PICKER_DLG_WIDTH, SAGE_PICKER_DLG_HEIGHT + GetContentTop());
    LayoutControls();

    FilterList(CString());

    if (!m_strInitialName.IsEmpty()) {
        int nIdx = m_wndNameList.FindStringExact(-1, m_strInitialName);
        if (nIdx != LB_ERR) {
            m_wndNameList.SetCurSel(nIdx);
            m_wndNameList.SetTopIndex(nIdx);
        }
    }

    m_wndSearch.SetEditFocus();

    return FALSE;
}

BOOL SageCalcCompanyPickerDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
        if (m_wndSearch.IsEditMessage(pMsg)) {
            m_wndNameList.SetFocus();
            if (m_wndNameList.GetCurSel() == LB_ERR && m_wndNameList.GetCount() > 0)
                m_wndNameList.SetCurSel(0);
            return TRUE;
        }
        if (pMsg->hwnd == m_wndNameList.GetSafeHwnd()) {
            OnOK();
            return TRUE;
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}

void SageCalcCompanyPickerDlg::CreateControls() {
    CRect rectEmpty(0, 0, 0, 0);

    m_wndSearch.CreateBox(this, ID_PICKER_DLG_SEARCH_BOX, ID_PICKER_DLG_SEARCH_EDIT);
    m_wndSearch.SetCommand(ID_PICKER_DLG_SEARCH_BTN);
    m_wndNameList.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT |
        LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
        rectEmpty, this, ID_PICKER_DLG_LIST);
    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this);
    m_wndError.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndMatchCount.Create(L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndOkBtn.Create(SAGE_UI_PICKER_DLG_OK,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDOK);
    m_wndCancelBtn.Create(SAGE_UI_PICKER_DLG_CANCEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDCANCEL);

    m_wndSearch.SetPlaceholder(SAGE_UI_PICKER_SEARCH_CUE);
}

void SageCalcCompanyPickerDlg::LayoutControls() {
    int nM = SAGE_MARGIN;
    int nEditH = SAGE_EDIT_HEIGHT;
    int nBtnW = SAGE_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = SAGE_BUTTON_HEIGHT;
    int nGap = SAGE_ROW_GAP;
    int nClientW = SAGE_PICKER_DLG_WIDTH;
    int nClientH = SAGE_PICKER_DLG_HEIGHT + GetContentTop();
    int nContentW = nClientW - nM * 2;

    int nSearchTop = GetContentTop() + nM;
    int nListTop = nSearchTop + nEditH + nGap;
    int nBtnTop = nClientH - nM - nBtnH;
    int nErrorTop = nBtnTop - SAGE_INLINE_MSG_HEIGHT;
    int nListH = nErrorTop - nListTop - nGap;
    if (nListH < nEditH * 4)
        nListH = nEditH * 4;

    m_wndSearch.MoveWindow(nM, nSearchTop, nContentW, nEditH);
    m_wndNameList.MoveWindow(nM, nListTop, nContentW, nListH);
    m_wndError.MoveWindow(nM, nErrorTop, nContentW, SAGE_INLINE_MSG_HEIGHT);
    m_wndMatchCount.MoveWindow(nM, nBtnTop, nContentW, nBtnH);

    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);
}

void SageCalcCompanyPickerDlg::ApplyFont() {
    m_font.CreatePointFont(SAGE_CONTENT_FONT_POINT_SIZE, SAGE_CONTROL_FONT_FACE);

    m_wndNameList.SetFont(&m_font);
    m_wndMatchCount.SetFont(SageUiResources::GetFont(SAGE_FONT_CAPTION));
    m_wndMatchCount.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndMatchCount.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}


void SageCalcCompanyPickerDlg::FilterList(const CString& strKeyword) {
    m_wndNameList.SetRedraw(FALSE);
    m_wndNameList.ResetContent();
    for (int i = 0; i < m_arrAllNames.GetSize(); i++) {
        if (strKeyword.IsEmpty() || m_arrAllNames[i].Find(strKeyword) >= 0)
            m_wndNameList.AddString(m_arrAllNames[i]);
    }
    m_wndNameList.SetRedraw(TRUE);
    if (m_wndNameList.GetCount() > 0)
        m_wndNameList.SetCurSel(0);

    CString strCount;
    strCount.Format(SAGE_UI_PICKER_DLG_MATCH_FMT,
        static_cast<int>(m_arrAllNames.GetSize()), m_wndNameList.GetCount());
    m_wndMatchCount.SetWindowTextW(strCount);
}

void SageCalcCompanyPickerDlg::OnSearchChanged() {
    CString strKeyword = m_wndSearch.GetKeyword();
    strKeyword.Trim();
    FilterList(strKeyword);
}

void SageCalcCompanyPickerDlg::OnListDblClick() {
    OnOK();
}

void SageCalcCompanyPickerDlg::OnOK() {
    m_wndError.ClearMessage();

    int nSel = m_wndNameList.GetCurSel();
    if (nSel == LB_ERR) {
        m_wndError.SetMessage(SAGE_UI_PICKER_SELECT_REQUIRED, SAGE_INLINE_ERROR);
        m_wndNameList.SetFocus();
        return;
    }
    m_wndNameList.GetText(nSel, m_strSelectedName);
    CDialog::OnOK();
}

void SageCalcCompanyPickerDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH SageCalcCompanyPickerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
        return hBrush;

    if (nCtlColor == CTLCOLOR_EDIT) {
        pDC->SetTextColor(SAGE_COLOR_TEXT);
        pDC->SetBkColor(SAGE_COLOR_PANEL);
        return m_brushPanel;
    }

    if (nCtlColor == CTLCOLOR_LISTBOX) {
        pDC->SetTextColor(SAGE_COLOR_TEXT);
        pDC->SetBkColor(SAGE_COLOR_PANEL);
        return m_brushPanel;
    }

    pDC->SetBkColor(SAGE_COLOR_PANEL);
    return m_brushBackground;
}
