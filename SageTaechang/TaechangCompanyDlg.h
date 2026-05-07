#pragma once

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
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);

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
    CButton m_wndOkBtn;
    CButton m_wndCancelBtn;

    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
