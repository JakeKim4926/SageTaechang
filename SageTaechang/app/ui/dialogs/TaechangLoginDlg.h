#pragma once

#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageEdit.h"
#include "app/ui/drawing/SageInlineError.h"
#include "app/ui/dialogs/SageDialogSizer.h"

#include "pch.h"

class TaechangLoginDlg : public CDialog {
public:
    TaechangLoginDlg(CWnd* pParent = NULL);
    ~TaechangLoginDlg();

public:
    virtual INT_PTR DoModal();

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
    void ApplyEditTextRect(CEdit& edit);
    void ShowInputError(CSageEdit& edit, const CString& strMessage);
    BYTE* BuildDialogTemplate();

private:
    CWnd* m_pDlgParent;

    CStatic m_wndIdLabel;
    CStatic m_wndPwLabel;
    CSageEdit m_wndIdEdit;
    CSageEdit m_wndPwEdit;
    CSageInlineError m_wndError;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;

    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
