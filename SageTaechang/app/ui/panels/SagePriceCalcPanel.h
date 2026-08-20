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
    int  GetInputCardHeight() const;
    int  GetResultCardHeight() const;
    void LayoutCardHeader(CSageSectionLabel& wndSection, const CRect& rectCard);
    int  GetCardContentTop(const CRect& rectCard) const;
    void LayoutInputCard(const CRect& rectCard);
    void LayoutResultCard(const CRect& rectCard);
    void LayoutHistoryCard(const CRect& rectCard);
    void LayoutResultRow(int nTop, int nLeft, int nRight, CSageLabel& wndLabel, CSageLabel& wndValue);
    void ApplyEditTextRect(CEdit& wndEdit);
    void DrawCard(CDC* pDC, const CRect& rectCard);
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
    CSageSectionLabel m_wndInputSection;
    CSageLabel m_wndCompanyLabel;
    CSageComboBox m_wndCompanyCombo;
    CSageButton m_wndCompanyPickBtn;
    CSageLabel m_wndCopiesLabel;
    CEdit m_wndCopiesEdit;
    CSageLabel m_wndCopiesUnitLabel;
    CSageLabel m_wndPagesLabel;
    CEdit m_wndPagesEdit;
    CSageLabel m_wndFreightLabel;
    CEdit m_wndFreightEdit;
    CSageLabel m_wndFreightUnitLabel;
    CSageButton m_wndCalcBtn;
    CSageButton m_wndCalcResetBtn;
    CSageSectionLabel m_wndResultSection;
    CSageLabel m_wndPrintLabel;
    CSageLabel m_wndPrintValue;
    CSageLabel m_wndCoverLabel;
    CSageLabel m_wndCoverValue;
    CSageLabel m_wndSubtotalLabel;
    CSageLabel m_wndSubtotalValue;
    CSageLabel m_wndFreightResultLabel;
    CSageLabel m_wndFreightValue;
    CSageLabel m_wndTotalLabel;
    CSageLabel m_wndTotalValue;
    CSageLabel m_wndRangeHint;
    CSageSectionLabel m_wndHistorySection;
    CSageHeaderCtrl m_wndHistoryHeader;
    CSageListCtrl m_wndHistoryList;

private:
    CRect m_rectInputCard;
    CRect m_rectResultCard;
    CRect m_rectHistoryCard;
    CRect m_rectResultRows;
    CRect m_rectTotalBand;
    SagePriceCalcResult m_calcResult;
    BOOL m_bFormattingFreight;
    CArray<SageCalcHistoryEntry, SageCalcHistoryEntry&> m_arrHistory;
};
