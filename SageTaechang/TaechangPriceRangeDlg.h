#pragma once

#include "pch.h"

class TaechangPriceRangeDlg : public CDialog {
public:
    TaechangPriceRangeDlg(CWnd* pParent = NULL);
    ~TaechangPriceRangeDlg();

public:
    virtual INT_PTR DoModal();
    int GetMinCopies() const;
    BOOL HasMaxCopies() const;
    int GetMaxCopies() const;
    int GetPrintPrice() const;

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnOK();
    virtual void OnCancel();

    afx_msg void OnNoMaxCheck();
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
    int m_nMinCopies;
    BOOL m_bHasMaxCopies;
    int m_nMaxCopies;
    int m_nPrintPrice;

    CStatic m_wndMinLabel;
    CEdit m_wndMinEdit;
    CStatic m_wndMaxLabel;
    CEdit m_wndMaxEdit;
    CButton m_wndNoMaxCheck;
    CStatic m_wndPrintLabel;
    CEdit m_wndPrintEdit;
    CButton m_wndOkBtn;
    CButton m_wndCancelBtn;

    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
