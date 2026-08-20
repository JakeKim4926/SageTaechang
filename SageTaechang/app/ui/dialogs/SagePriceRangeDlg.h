#pragma once

#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageEdit.h"
#include "app/ui/drawing/SageInlineError.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageOptionCheck.h"
#include "app/ui/dialogs/SageDialogSizer.h"
#include "app/ui/dialogs/SageFramelessDialog.h"

#include "pch.h"

class SagePriceRangeDlg : public SageFramelessDialog {
public:
    SagePriceRangeDlg(CWnd* pParent = NULL);
    ~SagePriceRangeDlg();

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

    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    int LayoutControls();
    void ApplyFont();
    void ApplyEditTextRect(CEdit& edit);
    void FormatPriceEditText(CEdit& edit, BOOL& bFormatting);
    void ShowInputError(CSageEdit& edit, const CString& strMessage);
    void ClearInputError();
    BOOL IsCopiesRangeOverlap(int nMinA, BOOL bHasMaxA, int nMaxA, int nMinB, BOOL bHasMaxB, int nMaxB) const;
    BOOL IsOverlappingExistingRange(int nMinCopies, BOOL bHasMaxCopies, int nMaxCopies) const;

private:
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

    CSageLabel m_wndMinLabel;
    CSageEdit m_wndMinEdit;
    CSageLabel m_wndMinUnit;
    CSageOptionCheck m_wndSingleCheck;
    CSageLabel m_wndMaxLabel;
    CSageEdit m_wndMaxEdit;
    CSageLabel m_wndMaxUnit;
    CSageOptionCheck m_wndNoMaxCheck;
    CSageLabel m_wndPrintLabel;
    CSageEdit m_wndPrintEdit;
    CSageLabel m_wndPrintUnit;
    CSageLabel m_wndCoverLabel;
    CSageEdit m_wndCoverEdit;
    CSageLabel m_wndCoverUnit;
    CSageInlineError m_wndError;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;

    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
