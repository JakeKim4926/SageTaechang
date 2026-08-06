#pragma once

#include "pch.h"
#include "app/core/price/SagePriceCalcService.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageComboBox.h"
#include "app/ui/drawing/SageHeaderCtrl.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageListCtrl.h"
#include "app/ui/drawing/SageSectionLabel.h"

struct SageCalcHistoryEntry {
    CString strCompanyName;
    CString strItemName;
    CString strDate;
    int nCopies;
    int nPages;
    LONGLONG nPrintPrice;
    int nCoverPrice;
    int nFreight;
    LONGLONG nTotal;
    CTime timeCalc;

    SageCalcHistoryEntry()
        : nCopies(0)
        , nPages(0)
        , nPrintPrice(0)
        , nCoverPrice(0)
        , nFreight(0)
        , nTotal(0) {}
};

class SagePriceCalcPanel : public CWnd {
public:
    SagePriceCalcPanel();

public:
    BOOL Create(CWnd* pParent, UINT nId);
    void Layout(const CRect& rectPanel);
    void RefreshCompanyCombo();
    void ClearInputAndResult();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnCalc();
    afx_msg void OnCalcReset();
    afx_msg void OnCompanyChanged();
    afx_msg void OnInputChanged();
    afx_msg void OnFreightChanged();
    afx_msg void OnCompanyPick();
    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void ApplyControlFonts();
    void ApplyLabelRoles();
    void LayoutChildControls(int nWidth, int nHeight);
    void DrawEditBorder(CDC* pDC, CWnd& wnd);
    void ClearResult();
    BOOL UpdatePreview(BOOL bShowMessage);
    void ShowFailureMessage(SagePriceCalcFailure nFailure, const CString& strError) const;
    void UpdateTotal();
    void AddHistory(const CString& strCompany, int nCopies, int nPages, const CString& strItemName,
        const CString& strDate, LONGLONG nPrintPrice, int nCoverPrice, int nFreight, LONGLONG nTotal);
    int  GetHistoryVisibleCapacity() const;
    void TrimHistoryToVisibleCapacity();
    void RefreshHistoryList();
    void UpdateRangeHint();

private:
    CSageLabel m_wndCompanyLabel;
    CSageComboBox m_wndCompanyCombo;
    CSageButton m_wndCompanyPickBtn;
    CSageLabel m_wndCopiesLabel;
    CEdit m_wndCopiesEdit;
    CSageLabel m_wndPagesLabel;
    CEdit m_wndPagesEdit;
    CSageButton m_wndCalcBtn;
    CSageButton m_wndCalcResetBtn;
    CSageLabel m_wndPrintLabel;
    CSageLabel m_wndPrintValue;
    CSageLabel m_wndCoverLabel;
    CSageLabel m_wndCoverValue;
    CSageLabel m_wndSubtotalLabel;
    CSageLabel m_wndSubtotalValue;
    CSageLabel m_wndFreightLabel;
    CEdit m_wndFreightEdit;
    CSageLabel m_wndFreightUnitLabel;
    CSageLabel m_wndDivider;
    CSageLabel m_wndTotalDivider;
    CSageLabel m_wndTotalLabel;
    CSageLabel m_wndTotalValue;
    CSageLabel m_wndRangeHint;
    CSageSectionLabel m_wndHistorySection;
    CSageHeaderCtrl m_wndHistoryHeader;
    CSageListCtrl m_wndHistoryList;

private:
    CRect m_rectInputCard;
    CRect m_rectResultCard;
    CRect m_rectTotalBand;
    SagePriceCalcResult m_calcResult;
    BOOL m_bFormattingFreight;
    CArray<SageCalcHistoryEntry, SageCalcHistoryEntry&> m_arrHistory;
};
