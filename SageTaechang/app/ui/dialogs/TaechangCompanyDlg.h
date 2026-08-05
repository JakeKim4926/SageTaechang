#pragma once

#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageEdit.h"
#include "app/ui/drawing/SageInlineError.h"
#include "app/ui/dialogs/SageDialogSizer.h"

#include "pch.h"

class TaechangCompanyDlg : public CDialog {
public:
    TaechangCompanyDlg(CWnd* pParent = NULL);
    ~TaechangCompanyDlg();

public:
    virtual INT_PTR DoModal();
    CString GetCompanyName() const;

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnOK();
    virtual void OnCancel();

    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    int LayoutControls();
    void ApplyFont();
    void ShowInputError(const CString& strMessage);
    BYTE* BuildDialogTemplate();

private:
    CWnd* m_pDlgParent;
    CString m_strCompanyName;

    CStatic m_wndLabel;
    CSageEdit m_wndCompanyEdit;
    CSageInlineError m_wndError;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;

    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
