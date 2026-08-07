#pragma once

#include "pch.h"
#include "app/ui/panels/SageCompanyOrderPanel.h"
#include "app/ui/panels/SagePriceCalcPanel.h"
#include "app/ui/panels/SagePriceManagePanel.h"
#include "app/ui/panels/SageWorkflowHistoryPanel.h"
#include "app/ui/panels/SageWorkflowInputPanel.h"
#include "app/ui/panels/SageWorkflowResultPanel.h"
#include "app/ui/drawing/SageTabCtrl.h"
#include "app/ui/workflow/SageWorkflowController.h"

class ISageWorkflowHandler;
struct TaechangResultRow;

struct SageWorkflowUiState {
    SageWorkflowUiState()
        : nSelectedTaskTab(TAECHANG_TAB_INDEX_INPUT)
        , nResultFilterCriteria(TAECHANG_FILTER_CRITERIA_NONE)
        , bEstimateOnePage(FALSE) {}

    int nSelectedTaskTab;
    SageWorkflowResultState result;
    CString strResultFilterKeyword;
    int nResultFilterCriteria;
    CString strInputPath;
    CString strOutputFolder;
    CString strCheckedRowNums;
    BOOL bEstimateOnePage;
};

struct SageWorkspaceVisibility {
    SageWorkspaceVisibility() {
        bInputResetVisible = FALSE;
        bInputTableVisible = FALSE;
        bOnePageVisible = FALSE;
        bFilterVisible = FALSE;
    }

    BOOL bInputResetVisible;
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
    void ApplyWorkflowLabels(ISageWorkflowHandler* pHandler);
    void RefreshVisibility();

public:
    BOOL IsRunning() const;
    void RequestRun(int nTaskType);
    void ResetInput();
    void ApplyDroppedInputPaths(const CString& strPaths);
    void SaveWorkflowState(int nWorkflowType);
    void RestoreWorkflowState(int nWorkflowType);
    void RebuildResultTable();

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
    SageCompanyOrderPanel& GetCompanyOrderPanel();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnTabChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnResultTableChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnResultSelectionChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkflowRunRequested(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkflowInputReset(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkflowComplete(WPARAM wParam, LPARAM lParam);
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
    void UpdateVisibility(const SageWorkspaceVisibility& state);
    SageResultTablePanel* FindResultTable();
    void ApplyResultTableSchema();
    void SetResultTableRows(const std::vector<TaechangResultRow>& arrRows);
    void UpdateResultSummary();
    void UpdateActionButtonState();
    void ApplyActionButtonState(int nSelectedCount);
    void SetRunningState(BOOL bRunning);
    void DisplayResponse(int nWorkflowType, int nTaskType, const CString& strResponseJson);
    void ApplyStatusCardResult(
        ISageWorkflowHandler* pHandler,
        int nTaskType,
        const CString& strResponseJson,
        BOOL bSuccess,
        int nResultCount);
    BOOL IsInputTableVisible() const;
    BOOL IsOnePageOptionVisible() const;
    BOOL IsInputResetVisible() const;
    BOOL IsResultFilterVisible() const;
    BOOL ValidateInputPath(CString& strInputPath) const;
    BOOL ValidateOutputFolder(CString& strOutputFolder) const;
    BOOL BuildSelectedRowNums(int nTaskType, CString& strRowNums, BOOL& bOnePage);
    void NotifyStatus(LPCWSTR pszStatus);
    SageWorkflowUiState& GetWorkflowState(int nWorkflowType);

private:
    CSageTabCtrl m_wndTaskTabs;
    SageWorkflowInputPanel m_panelWorkflowInput;
    SageWorkflowResultPanel m_panelWorkflowResult;
    SageWorkflowHistoryPanel m_panelWorkflowHistory;
    SagePriceManagePanel m_panelPriceManage;
    SagePriceCalcPanel m_panelPriceCalc;
    SageCompanyOrderPanel m_panelCompanyOrder;

private:
    SageWorkflowController m_controller;
    SageWorkflowUiState m_stateReceivables;
    SageWorkflowUiState m_stateDelivery;
    SageWorkflowUiState m_stateEstimate;
    ISageWorkflowHandler* m_pHandler;
    int m_nCurrentWorkflow;
    int m_nSelectedTaskTab;
};
