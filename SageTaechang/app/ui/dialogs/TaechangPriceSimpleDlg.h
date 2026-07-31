#pragma once

#include "app/ui/drawing/SageButton.h"

#include "pch.h"

class TaechangCompanyRenameDlg : public CDialog {
public:
    TaechangCompanyRenameDlg(CWnd* pParent = NULL);
    ~TaechangCompanyRenameDlg();

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
    void ApplyEditTextRect();
    BYTE* BuildDialogTemplate();

private:
    CWnd* m_pDlgParent;
    CString m_strCompanyName;
    CStatic m_wndLabel;
    CEdit m_wndEdit;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;
    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};

class TaechangCoverPriceDlg : public CDialog {
public:
    TaechangCoverPriceDlg(CWnd* pParent = NULL);
    ~TaechangCoverPriceDlg();

public:
    virtual INT_PTR DoModal();
    int GetCoverPrice() const;

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnOK();
    virtual void OnCancel();

    afx_msg void OnCoverPriceChanged();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void LayoutControls();
    void ApplyFont();
    void ApplyEditTextRect();
    void FormatPriceEditText();
    BYTE* BuildDialogTemplate();

private:
    CWnd* m_pDlgParent;
    int m_nCoverPrice;
    BOOL m_bFormattingCoverPrice;
    CStatic m_wndLabel;
    CEdit m_wndEdit;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;
    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
