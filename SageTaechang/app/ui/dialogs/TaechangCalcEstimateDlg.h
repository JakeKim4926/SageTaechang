#pragma once

#include "app/ui/drawing/SageButton.h"

#include "pch.h"

class TaechangCalcEstimateDlg : public CDialog {
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
    void LayoutControls();
    void ApplyFont();
    void ApplyEditTextRect(CEdit& edit);
    BOOL ValidateInputs();
    BOOL SelectOutputFolder(CString& strFolder);
    BOOL RunGenerate(const CString& strOutputFolder);
    BYTE* BuildDialogTemplate();

private:
    CWnd*   m_pDlgParent;
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
    CEdit   m_wndYearEdit;
    CStatic m_wndDateSep1;
    CEdit   m_wndMonthEdit;
    CStatic m_wndDateSep2;
    CEdit   m_wndDayEdit;
    CStatic m_wndDateDivider;
    CStatic m_wndItemLabel;
    CEdit   m_wndItemEdit;
    CStatic m_wndItemDivider;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;

    CFont   m_font;
    CBrush  m_brushBackground;
    CBrush  m_brushPanel;
    CBrush  m_brushDivider;
};
