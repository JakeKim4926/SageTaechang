
#pragma once

#include "TaechangDefine.h"
#include "app/ui/panels/SageResultTablePanel.h"
#include "app/ui/panels/SageWorkspacePanel.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/ui/drawing/SageSidebarTree.h"

struct TaechangResultRow;
struct SageWorkflowColumn;
class ISageWorkflowHandler;

struct TaechangWorkflowUiState {
    int nSelectedTaskTab;
    int nLastWorkflowType;
    int nLastTaskType;
    BOOL bLastTaskSuccess;
    CString strLastResponseJson;
    CString strRunningInputPath;
    CString strResultFilterKeyword;
    int nResultFilterCriteria;
    CString strInputPath;
    CString strOutputFolder;
    CString strCheckedRowNums;
    BOOL bEstimateOnePage;

    TaechangWorkflowUiState()
        : nSelectedTaskTab(TAECHANG_TAB_INDEX_INPUT)
        , nLastWorkflowType(0)
        , nLastTaskType(0)
        , bLastTaskSuccess(FALSE)
        , nResultFilterCriteria(TAECHANG_FILTER_CRITERIA_NONE)
        , bEstimateOnePage(FALSE) {}
};

class CSageTaechangView : public CView
{
protected:
    CSageTaechangView() noexcept;
    DECLARE_DYNCREATE(CSageTaechangView)

public:
    CSageTaechangDoc* GetDocument() const;

public:
    virtual void OnDraw(CDC* pDC);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
    virtual ~CSageTaechangView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    CSageLabel m_wndSidebarTitle;
    CSageSidebarTree m_wndSidebarTree;
    CSageLabel m_wndHeaderTitle;
    CStatic m_wndHeaderStatus;
    CSageLabel m_wndTitle;
    CBrush m_brushListHeader;
    BOOL m_bRunning;
    int m_nLastWorkflowType;
    int m_nLastTaskType;
    int m_nCurrentWorkflow;
    HTREEITEM m_hLastWorkflowItem;
    COLORREF m_colorHeaderStatus;
    SageBackgroundRole m_nHeaderStatusBgRole;
    BOOL m_bLastTaskSuccess;
    CString m_strLastResponseJson;
    CString m_strRunningInputPath;
    TaechangWorkflowUiState m_stateReceivables;
    TaechangWorkflowUiState m_stateDelivery;
    TaechangWorkflowUiState m_stateEstimate;

    CSageButton m_wndLoginBtn;
    CSageButton m_wndLogoutBtn;
    CSageLabel m_wndUserLabel;
    int m_nAuthDividerX;

    SageWorkspacePanel m_panelWorkspace;

protected:
    void CreateChildControls();
    void BuildSidebarTree();
    void ApplyControlFonts();
    void ApplyLabelRoles();
    void UpdateTaskTabVisibility();
    void LayoutChildControls();
    void SetRunningState(BOOL bRunning);
    void UpdateAuthState();
    void SetStatusText(const CString& strStatus);
    int GetSelectedWorkflow() const;
    ISageWorkflowHandler* FindCurrentHandler() const;
    void UpdateWorkflowLabels();
    BOOL IsInputTableVisible() const;
    BOOL IsOnePageOptionVisible() const;
    BOOL IsInputResetVisible() const;
    BOOL IsDocumentResultFilterVisible() const;
    TaechangWorkflowUiState& GetWorkflowUiState(int nWorkflowType);
    void SaveWorkflowUiState(int nWorkflowType);
    void RestoreWorkflowUiState(int nWorkflowType);
    void RebuildCurrentWorkflowResultList();
    SageResultTablePanel* FindResultTablePanel(ISageWorkflowHandler* pHandler);
    void ApplyResultTableSchema();
    void SetResultTableRows(const std::vector<TaechangResultRow>& arrRows);
    void RefreshResultTableRows();
    void UpdateResultSummary();
    void UpdateActionButtonState();
    void ApplyActionButtonState(int nSelectedCount);
    COLORREF ResolveStatusColor(const CString& strStatus) const;
    SageBackgroundRole ResolveStatusBgRole(const CString& strStatus) const;
    void DrawShellBands(CDC* pDC, const CRect& rectClient);
    void InvalidateContentArea();
    BOOL ValidateInputPath(CString& strInputPath);
    BOOL ValidateOutputFolder(CString& strOutputFolder);
    void EnableFileDropForWindow(CWnd& wnd);
    void ApplyDroppedInputPaths(const CString& strPaths);
    void RunWorkflowTask(int nTaskType);
    void DisplayResponse(int nWorkflowType, int nTaskType, const CString& strResponseJson);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnWorkflowChanged();
    afx_msg void OnSidebarSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnWorkspaceTabChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkflowComplete(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkflowRunRequested(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkflowInputReset(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnResultTableChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnResultSelectionChanged(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnLogin();
    afx_msg void OnLogout();

    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CSageTaechangDoc* CSageTaechangView::GetDocument() const
   { return reinterpret_cast<CSageTaechangDoc*>(m_pDocument); }
#endif

