#pragma once

#include "pch.h"
#include "app/core/receivable/TaechangReceivableCompanyOrderDto.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageHeaderCtrl.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageListCtrl.h"
#include "app/ui/drawing/SageSectionLabel.h"

class SageCompanyOrderPanel : public CWnd {
public:
    SageCompanyOrderPanel();

public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
    BOOL Create(CWnd* pParent, UINT nId);
    void Layout(const CRect& rectPanel);
    void RefreshList();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnAdd();
    afx_msg void OnModify();
    afx_msg void OnDelete();
    afx_msg void OnCancel();
    afx_msg void OnSearch();
    afx_msg void OnListSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void ApplyControlFonts();
    void ApplyLabelRoles();
    void UpdateListColumns();
    void UpdatePanelState();
    void ApplyEditTextRect(CEdit& wndEdit, int nLeftPad);
    void SetCardRect(const CRect& rectNew);
    void DrawEditBorder(CDC* pDC, CWnd& wnd);
    BOOL FindSelectedDto(TaechangReceivableCompanyOrderDto& outDto) const;

private:
    CSageSectionLabel m_wndCrudSection;
    CSageSectionLabel m_wndListSection;
    CSageButton m_wndAddBtn;
    CSageButton m_wndModifyBtn;
    CSageButton m_wndDeleteBtn;
    CSageButton m_wndCancelBtn;
    CSageLabel m_wndSearchLabel;
    CEdit m_wndSearchEdit;
    CSageButton m_wndSearchBtn;
    CSageLabel m_wndOrderLabel;
    CEdit m_wndOrderEdit;
    CSageLabel m_wndNameLabel;
    CEdit m_wndCompanyEdit;
    CSageHeaderCtrl m_wndListHeader;
    CSageListCtrl m_wndList;

private:
    CRect m_rectCard;
    int m_nPanelState;
    CString m_strSearchKeyword;
    int m_nSelectedOrderId;
    CArray<TaechangReceivableCompanyOrderDto, TaechangReceivableCompanyOrderDto&> m_arrOrders;
};
