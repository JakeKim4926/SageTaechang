#pragma once

#include "app/ui/drawing/SageButton.h"

class TaechangPasswordChangeDlg : public CDialog {
public:
    TaechangPasswordChangeDlg(CWnd* pParent = NULL);
    ~TaechangPasswordChangeDlg();

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
    BYTE* BuildDialogTemplate();
    void CreateControls();
    void LayoutControls();
    void ApplyFont();
    void ApplyEditTextRect(CEdit& edit);

private:
    CWnd* m_pDlgParent;
    CStatic m_wndCurrentLabel;
    CStatic m_wndNewLabel;
    CStatic m_wndConfirmLabel;
    CEdit m_wndCurrentEdit;
    CEdit m_wndNewEdit;
    CEdit m_wndConfirmEdit;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;
    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
