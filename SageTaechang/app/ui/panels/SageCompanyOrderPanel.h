#pragma once

#include "pch.h"
#include "app/core/receivable/TaechangReceivableCompanyOrderDto.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageHeaderCtrl.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageListCtrl.h"
#include "app/ui/drawing/SageSectionLabel.h"
#include "app/ui/drawing/SageSearchBox.h"

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
    afx_msg void OnSave();
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
    void ApplyOrderEditTextRect();
    void SetCardRects(const CRect& rectList, const CRect& rectEdit);
    void DrawEditBorder(CDC* pDC, CWnd& wnd);
    void LayoutListCard(const CRect& rectCard);
    int  LayoutEditCard(int nLeft, int nWidth);
    void FillEditFromSelection();
    void AddCompanyOrder(const CString& strCompanyName, const CString& strOrder);
    BOOL FindSelectedDto(TaechangReceivableCompanyOrderDto& outDto) const;

private:
    CSageSectionLabel m_wndCrudSection;
    CSageSectionLabel m_wndListSection;
    CSageButton m_wndAddBtn;
    CSageButton m_wndSaveBtn;
    CSageButton m_wndDeleteBtn;
    CSageButton m_wndCancelBtn;
    CSageSearchBox m_wndSearch;
    CSageLabel m_wndOrderLabel;
    CEdit m_wndOrderEdit;
    CSageLabel m_wndNameLabel;
    CEdit m_wndCompanyEdit;
    CSageLabel m_wndGuide;
    CSageHeaderCtrl m_wndListHeader;
    CSageListCtrl m_wndList;

private:
    CRect m_rectListCard;
    CRect m_rectEditCard;
    int m_nDividerTop;
    int m_nPanelState;
    CString m_strSearchKeyword;
    int m_nSelectedOrderId;
    CArray<TaechangReceivableCompanyOrderDto, TaechangReceivableCompanyOrderDto&> m_arrOrders;
};
