#pragma once

#include "pch.h"
#include "app/ui/panels/SagePriceCalcPanel.h"
#include "app/ui/panels/SagePriceManagePanel.h"
#include "app/ui/panels/SageWorkflowHistoryPanel.h"
#include "app/ui/panels/SageWorkflowInputPanel.h"
#include "app/ui/panels/SageWorkflowResultPanel.h"
#include "app/ui/drawing/SageTabCtrl.h"

class ISageWorkflowHandler;

struct SageWorkspaceVisibility {
    SageWorkspaceVisibility() {
        bInputResetVisible = FALSE;
        bHasLastResult = FALSE;
        bInputTableVisible = FALSE;
        bOnePageVisible = FALSE;
        bFilterVisible = FALSE;
    }

    BOOL bInputResetVisible;
    BOOL bHasLastResult;
    BOOL bInputTableVisible;
    BOOL bOnePageVisible;
    BOOL bFilterVisible;
};

class SageWorkspacePanel : public CWnd {
public:
    SageWorkspacePanel();

public:
    BOOL Create(CWnd* pParent, UINT nId);
    void Layout(const CRect& rectPanel);
    void EnableFileDrop();

public:
    void SetWorkflow(int nWorkflowType, ISageWorkflowHandler* pHandler);
    void UpdateVisibility(const SageWorkspaceVisibility& state);

    int  GetSelectedTab() const;
    void SelectTab(int nSemanticTabIndex);
    BOOL IsInputTabSelected() const;
    BOOL IsResultTab() const;
    BOOL IsDetailTab() const;
    BOOL IsDataManageTab() const;

    SageWorkflowInputPanel& GetInputPanel();
    SageWorkflowResultPanel& GetResultPanel();
    SageWorkflowHistoryPanel& GetHistoryPanel();
    SagePriceManagePanel& GetPriceManagePanel();
    SagePriceCalcPanel& GetPriceCalcPanel();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnTabChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnResultTableChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnResultSelectionChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkflowRunRequested(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkflowInputReset(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

private:
    void LayoutTabRow();
    void LayoutActivePanel();
    CRect GetContentRect() const;
    int  GetTabVisualIndex(int nSemanticTabIndex) const;
    int  GetTabSemanticIndex(int nVisualTabIndex) const;
    BOOL IsPriceWorkflow() const;
    LRESULT ForwardToParent(UINT nMessage, WPARAM wParam, LPARAM lParam);

private:
    CSageTabCtrl m_wndTaskTabs;
    SageWorkflowInputPanel m_panelWorkflowInput;
    SageWorkflowResultPanel m_panelWorkflowResult;
    SageWorkflowHistoryPanel m_panelWorkflowHistory;
    SagePriceManagePanel m_panelPriceManage;
    SagePriceCalcPanel m_panelPriceCalc;

private:
    ISageWorkflowHandler* m_pHandler;
    int m_nCurrentWorkflow;
    int m_nSelectedTaskTab;
};
