
#pragma once

#include "TaechangDefine.h"
#include "app/core/receivable/TaechangReceivableCompanyOrderDto.h"
#include "app/ui/panels/SagePriceCalcPanel.h"
#include "app/ui/panels/SagePriceManagePanel.h"
#include "app/ui/panels/SageResultTablePanel.h"
#include "app/ui/drawing/SageHeaderCtrl.h"
#include "app/ui/drawing/SageTabCtrl.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageSectionLabel.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageListCtrl.h"
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
    CSageTabCtrl m_wndTaskTabs;
    CSageSectionLabel m_wndInputSection;
    CSageSectionLabel m_wndOutputSection;
    CSageSectionLabel m_wndDetailSection;
    CSageLabel m_wndTitle;
    CSageLabel m_wndWorkflowLabel;
    CSageLabel m_wndInputLabel;
    CSageLabel m_wndOutputLabel;
    CEdit m_wndInputPath;
    CEdit m_wndOutputFolder;
    CSageButton m_wndSelectInput;
    CSageButton m_wndSelectOutput;
    CSageButton m_wndLoad;
    CSageButton m_wndGenerate;
    CSageButton m_wndInputReset;
    CProgressCtrl m_wndProgress;
    CSageLabel m_wndProgressText;
    CEdit m_wndDetail;
    CSageLabel m_wndEmptyStateHint;
    CStatic m_wndActionStatus;
    CBrush m_brushListHeader;
    BOOL m_bRunning;
    int m_nProgressPercent;
    int m_nSelectedTaskTab;
    int m_nLastWorkflowType;
    int m_nLastTaskType;
    int m_nCurrentWorkflow;
    HTREEITEM m_hLastWorkflowItem;
    COLORREF m_colorHeaderStatus;
    SageBackgroundRole m_nHeaderStatusBgRole;
    BOOL m_bLastTaskSuccess;
    CString m_strLastResponseJson;
    CString m_strExecutionHistory;
    CString m_strRunningInputPath;
    TaechangWorkflowUiState m_stateReceivables;
    TaechangWorkflowUiState m_stateDelivery;
    TaechangWorkflowUiState m_stateEstimate;

    CSageButton m_wndLoginBtn;
    CSageButton m_wndLogoutBtn;
    CSageLabel m_wndUserLabel;
    int m_nAuthDividerX;

    SagePriceManagePanel m_panelPriceManage;
    SagePriceCalcPanel m_panelPriceCalc;
    SageResultTablePanel m_panelInputTable;
    SageResultTablePanel m_panelResultTable;

    // ── 법인 순서 데이터 관리 패널 ───────────────────────────────────────────
    CSageButton         m_wndCoAddBtn;
    CSageButton         m_wndCoModifyBtn;
    CSageButton         m_wndCoDeleteBtn;
    CSageButton         m_wndCoCancelBtn;
    CSageLabel          m_wndCoSearchLabel;
    CEdit               m_wndCoSearchEdit;
    CSageButton         m_wndCoSearchBtn;
    CSageLabel          m_wndCoOrderLabel;
    CEdit               m_wndCoOrderEdit;
    CSageLabel          m_wndCoNameLabel;
    CEdit               m_wndCoCompanyEdit;
    CSageSectionLabel      m_wndCoCrudSection;
    CSageSectionLabel      m_wndCoListSection;
    CRect               m_rectCoCard;
    CSageHeaderCtrl m_wndCoListHeader;
    CSageListCtrl       m_wndCoList;
    int                 m_nCoPanelState;
    CString             m_strCoSearchKeyword;
    int                 m_nCoSelectedOrderId;
    CArray<TaechangReceivableCompanyOrderDto, TaechangReceivableCompanyOrderDto&> m_arrCoOrders;

protected:
    void CreateChildControls();
    void BuildSidebarTree();
    void ApplyControlFonts();
    void ApplyLabelRoles();
    void ApplyWorkflowTabs();
    void UpdateTaskTabVisibility();
    void LayoutChildControls();
    void LayoutInputSection(int nLeft, int nTop, int nWidth, BOOL bShowOutput);
    void LayoutActionSection(int nLeft, int nTop, int nWidth);
    void LayoutResultSection(int nLeft, int nTop, int nWidth, int nHeight);
    void SetRunningState(BOOL bRunning);
    void UpdateAuthState();
    void UpdateProgressPercent(int nPercent);
    void SetStatusText(const CString& strStatus);
    int GetSelectedWorkflow() const;
    ISageWorkflowHandler* FindCurrentHandler() const;
    void UpdateWorkflowLabels();
    BOOL IsInputTabSelected() const;
    BOOL IsResultTab() const;
    BOOL IsDetailTab() const;
    BOOL IsActionTabVisible() const;
    int GetTaskTabVisualIndex(int nSemanticTabIndex) const;
    int GetTaskTabSemanticIndex(int nVisualTabIndex) const;
    BOOL IsInputTableVisible() const;
    BOOL IsOnePageOptionVisible() const;
    BOOL IsInputResetVisible() const;
    BOOL IsDocumentResultFilterVisible() const;
    TaechangWorkflowUiState& GetWorkflowUiState(int nWorkflowType);
    void SaveWorkflowUiState(int nWorkflowType);
    void RestoreWorkflowUiState(int nWorkflowType);
    void RebuildCurrentWorkflowResultList();
    SageResultTablePanel* FindResultTablePanel(ISageWorkflowHandler* pHandler);
    SageResultTablePanel* FindVisibleResultTablePanel();
    void ApplyResultTableSchema();
    void SetResultTableRows(const std::vector<TaechangResultRow>& arrRows);
    void RefreshResultTableRows();
    COLORREF ResolveStatusColor(const CString& strStatus) const;
    SageBackgroundRole ResolveStatusBgRole(const CString& strStatus) const;
    void DrawEditBorder(CDC* pDC, CWnd& wnd);
    void SetCardRect(CRect& rectCard, const CRect& rectNew);
    void InvalidateContentArea();
    BOOL ValidateInputPath(CString& strInputPath);
    BOOL ValidateOutputFolder(CString& strOutputFolder);
    void EnableFileDropForWindow(CWnd& wnd);
    void ApplyDroppedInputPaths(const CString& strPaths);
    void RunWorkflowTask(int nTaskType);
    void DisplayResponse(int nWorkflowType, int nTaskType, const CString& strResponseJson);
    void AppendExecutionHistory(int nWorkflowType, int nTaskType, const CString& strResponseJson, BOOL bSuccess);
    CString BuildExecutionHistoryLine(int nWorkflowType, int nTaskType, const CString& strResponseJson, BOOL bSuccess) const;

    // ── 법인 순서 데이터 관리 패널 ───────────────────────────────────────────
    void CreateCompanyOrderPanel();
    void LayoutCompanyOrderPanel(int nLeft, int nTop, int nWidth, int nHeight);
    void ShowCompanyOrderPanel(BOOL bShow);
    BOOL IsDataManageTab() const;
    void RefreshCompanyOrderList();
    void UpdateCoListColumns();
    void UpdateCoPanelState();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnWorkflowChanged();
    afx_msg void OnSidebarSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnTaskTabChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelectInput();
    afx_msg void OnSelectOutput();
    afx_msg void OnLoadWorkflow();
    afx_msg void OnGenerateWorkflow();
    afx_msg void OnInputReset();
    afx_msg LRESULT OnWorkflowComplete(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnResultTableChanged(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnLogin();
    afx_msg void OnLogout();

    // ── 법인 순서 데이터 관리 이벤트 ─────────────────────────────────────────
    afx_msg void OnCoAdd();
    afx_msg void OnCoModify();
    afx_msg void OnCoDelete();
    afx_msg void OnCoCancel();
    afx_msg void OnCoSearch();
    afx_msg void OnCoListSelChanged(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CSageTaechangDoc* CSageTaechangView::GetDocument() const
   { return reinterpret_cast<CSageTaechangDoc*>(m_pDocument); }
#endif

