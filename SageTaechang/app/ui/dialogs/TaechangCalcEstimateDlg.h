#pragma once

#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageEdit.h"
#include "app/ui/drawing/SageInlineError.h"
#include "app/ui/dialogs/SageDialogSizer.h"
#include "app/ui/dialogs/SageFramelessDialog.h"

#include "pch.h"

class TaechangCalcEstimateDlg : public SageFramelessDialog {
public:
    TaechangCalcEstimateDlg(
        const CString& strCompany,
        int nCopies, int nPages,
        int nUnitPrice, int nCoverPrice, int nFreight,
        const CString& strTemplatePath,
        const CString& strScriptPath,
        CWnd* pParent = NULL);
    ~TaechangCalcEstimateDlg();

public:
    virtual INT_PTR DoModal();
    CString GetItemName() const;
    CString GetDate() const;

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnOK();
    virtual void OnCancel();

    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnYearChanged();
    afx_msg void OnMonthChanged();

    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    int LayoutControls();
    void ApplyFont();
    void ApplyEditTextRect(CEdit& edit);
    BOOL ValidateInputs();
    void ShowInputError(CSageEdit& edit, const CString& strMessage);
    BOOL SelectOutputFolder(CString& strFolder);
    BOOL RunGenerate(const CString& strOutputFolder);

private:
    CString m_strCompany;
    int     m_nCopies;
    int     m_nPages;
    int     m_nUnitPrice;
    int     m_nCoverPrice;
    int     m_nFreight;
    CString m_strTemplatePath;
    CString m_strScriptPath;

    CString m_strDate;
    CString m_strItemName;

    CStatic m_wndDateLabel;
    CSageEdit m_wndYearEdit;
    CStatic m_wndDateSep1;
    CSageEdit m_wndMonthEdit;
    CStatic m_wndDateSep2;
    CSageEdit m_wndDayEdit;
    CStatic m_wndDateDivider;
    CStatic m_wndItemLabel;
    CSageEdit m_wndItemEdit;
    CStatic m_wndItemDivider;
    CSageInlineError m_wndError;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;

    CFont   m_font;
    CBrush  m_brushBackground;
    CBrush  m_brushPanel;
    CBrush  m_brushDivider;
};
