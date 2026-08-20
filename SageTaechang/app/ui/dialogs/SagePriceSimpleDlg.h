#pragma once

#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageEdit.h"
#include "app/ui/drawing/SageInlineError.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/dialogs/SageDialogSizer.h"
#include "app/ui/dialogs/SageFramelessDialog.h"

#include "pch.h"

class SageCompanyRenameDlg : public SageFramelessDialog {
public:
    SageCompanyRenameDlg(CWnd* pParent = NULL);
    ~SageCompanyRenameDlg();

public:
    virtual INT_PTR DoModal();
    CString GetCompanyName() const;
    void SetCompanyContext(const CString& strCompanyName, int nPriceCount);

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

private:
    CString m_strCompanyName;
    CString m_strInitialName;
    int m_nPriceCount;
    CSageLabel m_wndLabel;
    CSageLabel m_wndHint;
    CSageEdit m_wndEdit;
    CSageInlineError m_wndError;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;
    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};

class SageCoverPriceDlg : public SageFramelessDialog {
public:
    SageCoverPriceDlg(CWnd* pParent = NULL);
    ~SageCoverPriceDlg();

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

private:
    int m_nCoverPrice;
    BOOL m_bFormattingCoverPrice;
    CSageLabel m_wndLabel;
    CSageEdit m_wndEdit;
    CSageInlineError m_wndError;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;
    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
