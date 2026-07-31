#pragma once

#include "app/ui/drawing/SageButton.h"

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
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);

    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void LayoutControls();
    void ApplyFont();
    void ApplyEditTextRect(CEdit& edit);
    BYTE* BuildDialogTemplate();

private:
    CWnd* m_pDlgParent;

    CStatic m_wndIdLabel;
    CStatic m_wndPwLabel;
    CEdit m_wndIdEdit;
    CEdit m_wndPwEdit;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;

    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
