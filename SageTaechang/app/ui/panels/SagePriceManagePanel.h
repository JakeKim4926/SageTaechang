#pragma once

#include "pch.h"
#include "app/core/price/SagePriceDto.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageComboBox.h"
#include "app/ui/drawing/SageHeaderCtrl.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageOptionCheck.h"
#include "app/ui/drawing/SageEmptyState.h"
#include "app/ui/drawing/SageListCtrl.h"

class SagePriceManagePanel : public CWnd {
public:
    SagePriceManagePanel();

public:
    BOOL Create(CWnd* pParent, UINT nId);
    void Layout(const CRect& rectPanel);
    void RefreshCompanyList(const CString& strFilter = CString());
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnCompanySelChanged();
    afx_msg void OnCompanyEditChanged();
    afx_msg void OnAddCompany();
    afx_msg void OnRenameCompany();
    afx_msg void OnDeleteCompany();
    afx_msg void OnCopiesSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNoMaxCheck();
    afx_msg void OnSingleCheck();
    afx_msg void OnPrintChanged();
    afx_msg void OnCoverChanged();
    afx_msg void OnAdd();
    afx_msg void OnModify();
    afx_msg void OnDelete();
    afx_msg void OnCancel();
    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void ApplyControlFonts();
    void ApplyLabelRoles();
    void LayoutChildControls(int nWidth, int nHeight);
    void DrawEditBorder(CDC* pDC, CWnd& wnd);
    void ApplyRightPanel();
    void ApplyPriceEditTextRect(CEdit& edit);
    void UpdateDetailContext();
    void RefreshCopiesList(const CString& strCompanyName);
    void UpdateEmptyState();
    void UpdateSummaryCard();
    CString GetSelectedCompanyName() const;
    void LoadSelectedCopiesRowToForm();
    void ClearForm();
    BOOL ReadFormToDto(SagePriceDto& dto, CString& strError);

private:
    CSageLabel m_wndCompanyLabel;
    CSageComboBox m_wndCompanyCombo;
    CSageButton m_wndAddCompanyBtn;
    CSageButton m_wndRenameCompanyBtn;
    CSageButton m_wndDeleteCompanyBtn;
    CSageHeaderCtrl m_wndCopiesHeader;
    CSageListCtrl m_wndCopiesList;
    CSageEmptyState m_wndCopiesEmpty;
    CSageLabel m_wndMinCopiesLabel;
    CEdit m_wndMinCopiesEdit;
    CSageLabel m_wndMinCopiesUnit;
    CSageOptionCheck m_wndSingleCheck;
    CSageLabel m_wndMaxCopiesLabel;
    CEdit m_wndMaxCopiesEdit;
    CSageLabel m_wndMaxCopiesUnit;
    CSageOptionCheck m_wndNoMaxCheck;
    CSageLabel m_wndPrintLabel;
    CEdit m_wndPrintEdit;
    CSageLabel m_wndPrintUnit;
    CSageLabel m_wndCoverLabel;
    CEdit m_wndCoverEdit;
    CSageLabel m_wndCoverUnit;
    CSageButton m_wndAddBtn;
    CSageButton m_wndModifyBtn;
    CSageButton m_wndDeleteBtn;
    CSageButton m_wndCancelBtn;
    CSageLabel m_wndDetailHeader;
    CSageLabel m_wndDetailDivider;
    CSageLabel m_wndDetailPriceDivider;
    CSageLabel m_wndDetailContext;
    CSageLabel m_wndDetailState;

private:
    CRect m_rectSummaryCard;
    int m_nPanelState;
    BOOL m_bFormattingPrint;
    BOOL m_bFormattingCover;
};
