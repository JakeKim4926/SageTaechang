#pragma once

#include "app/ui/drawing/SageButton.h"

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
    int GetCoverPrice() const;
    void AddExistingRange(int nMinCopies, BOOL bHasMaxCopies, int nMaxCopies);

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnOK();
    virtual void OnCancel();

    afx_msg void OnNoMaxCheck();
    afx_msg void OnSingleCheck();
    afx_msg void OnPrintPriceChanged();
    afx_msg void OnCoverPriceChanged();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);

    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void LayoutControls();
    void ApplyFont();
    void ApplyEditTextRect(CEdit& edit);
    void FormatPriceEditText(CEdit& edit, BOOL& bFormatting);
    BOOL IsCopiesRangeOverlap(int nMinA, BOOL bHasMaxA, int nMaxA, int nMinB, BOOL bHasMaxB, int nMaxB) const;
    BOOL IsOverlappingExistingRange(int nMinCopies, BOOL bHasMaxCopies, int nMaxCopies) const;
    BYTE* BuildDialogTemplate();

private:
    CWnd* m_pDlgParent;
    int m_nMinCopies;
    BOOL m_bHasMaxCopies;
    int m_nMaxCopies;
    int m_nPrintPrice;
    int m_nCoverPrice;
    BOOL m_bFormattingPrintPrice;
    BOOL m_bFormattingCoverPrice;
    CArray<int, int> m_arrExistingMinCopies;
    CArray<int, int> m_arrExistingHasMaxCopies;
    CArray<int, int> m_arrExistingMaxCopies;

    CStatic m_wndMinLabel;
    CEdit m_wndMinEdit;
    CButton m_wndSingleCheck;
    CStatic m_wndMaxLabel;
    CEdit m_wndMaxEdit;
    CButton m_wndNoMaxCheck;
    CStatic m_wndPrintLabel;
    CEdit m_wndPrintEdit;
    CStatic m_wndCoverLabel;
    CEdit m_wndCoverEdit;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;

    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
