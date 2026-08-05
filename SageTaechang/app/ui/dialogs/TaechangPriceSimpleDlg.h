#pragma once

#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageEdit.h"
#include "app/ui/drawing/SageInlineError.h"
#include "app/ui/dialogs/SageDialogSizer.h"

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
    int LayoutControls();
    void ApplyFont();
    void ApplyEditTextRect();
    void ShowInputError(const CString& strMessage);
    BYTE* BuildDialogTemplate();

private:
    CWnd* m_pDlgParent;
    CString m_strCompanyName;
    CStatic m_wndLabel;
    CSageEdit m_wndEdit;
    CSageInlineError m_wndError;
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
    int LayoutControls();
    void ApplyFont();
    void ApplyEditTextRect();
    void FormatPriceEditText();
    void ShowInputError(const CString& strMessage);
    BYTE* BuildDialogTemplate();

private:
    CWnd* m_pDlgParent;
    int m_nCoverPrice;
    BOOL m_bFormattingCoverPrice;
    CStatic m_wndLabel;
    CSageEdit m_wndEdit;
    CSageInlineError m_wndError;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;
    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
