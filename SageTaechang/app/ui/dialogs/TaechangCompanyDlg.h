#pragma once

#include "app/ui/drawing/SageButton.h"

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
    void LayoutControls();
    void ApplyFont();
    BYTE* BuildDialogTemplate();

private:
    CWnd* m_pDlgParent;
    CString m_strCompanyName;

    CStatic m_wndLabel;
    CEdit m_wndCompanyEdit;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;

    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
