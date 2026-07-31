
#pragma once

#include "TaechangDefine.h"
#include "app/core/receivable/TaechangReceivableCompanyOrderDto.h"
#include "app/ui/drawing/SageHeaderCtrl.h"
#include "app/ui/drawing/SageTabCtrl.h"
#include "app/ui/drawing/SageComboBox.h"
#include "app/ui/drawing/SageFilterComboBox.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageSectionLabel.h"
#include "app/ui/drawing/SageListCtrl.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/ui/drawing/SageSidebarTree.h"

struct TaechangResultRow;
struct TaechangPriceDto;

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

struct CalcHistoryEntry {
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

    CalcHistoryEntry() : nCopies(0), nPages(0), nPrintPrice(0), nCoverPrice(0), nFreight(0), nTotal(0) {}
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
    CStatic m_wndSidebarTitle;
    CSageSidebarTree m_wndSidebarTree;
    CStatic m_wndHeaderTitle;
    CStatic m_wndHeaderStatus;
    CSageTabCtrl m_wndTaskTabs;
    CSageSectionLabel m_wndInputSection;
    CSageSectionLabel m_wndOutputSection;
    CSageSectionLabel m_wndResultSection;
    CSageSectionLabel m_wndDetailSection;
    CStatic m_wndTitle;
    CStatic m_wndWorkflowLabel;
    CStatic m_wndInputLabel;
    CStatic m_wndOutputLabel;
    CEdit m_wndInputPath;
    CEdit m_wndOutputFolder;
    CSageButton m_wndSelectInput;
    CSageButton m_wndSelectOutput;
    CSageButton m_wndLoad;
    CSageButton m_wndGenerate;
    CSageButton m_wndExportCsv;
    CSageButton m_wndSelectAll;
    CButton m_wndEstimateOnePage;
    CSageButton m_wndInputReset;
    CProgressCtrl m_wndProgress;
    CStatic m_wndProgressText;
    CSageHeaderCtrl m_wndResultHeader;
    CSageListCtrl m_wndResultList;
    CSageFilterComboBox m_wndResultFilterCriteria;
    CEdit m_wndResultFilter;
    CSageButton m_wndResultSearchBtn;
    CSageButton m_wndResultResetBtn;
    CEdit m_wndDetail;
    CStatic m_wndEmptyStateHint;
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
    CString m_strResultFilterKeyword;
    int m_nResultFilterCriteria;
    CRect m_rectResultFilterBox;
    TaechangWorkflowUiState m_stateReceivables;
    TaechangWorkflowUiState m_stateDelivery;
    TaechangWorkflowUiState m_stateEstimate;

    CSageButton m_wndLoginBtn;
    CSageButton m_wndLogoutBtn;
    CStatic m_wndUserLabel;
    int m_nAuthDividerX;

    // ── 가격 데이터 관리 패널 ────────────────────────────────────────────────
    CStatic             m_wndPriceCompanyLabel;
    CSageComboBox   m_wndPriceCompanyCombo;
    CSageButton         m_wndPriceAddCompanyBtn;
    CSageButton         m_wndPriceRenameCompanyBtn;
    CSageButton         m_wndPriceDeleteCompanyBtn;
    CSageHeaderCtrl m_wndPriceCopiesHeader;
    CSageListCtrl       m_wndPriceCopiesList;
    CStatic             m_wndPriceMinCopiesLabel;
    CEdit               m_wndPriceMinCopiesEdit;
    CButton             m_wndPriceSingleCheck;
    CStatic             m_wndPriceMaxCopiesLabel;
    CEdit               m_wndPriceMaxCopiesEdit;
    CButton             m_wndPriceNoMaxCheck;
    CStatic             m_wndPricePrintLabel;
    CEdit               m_wndPricePrintEdit;
    CStatic             m_wndPriceCoverLabel;
    CEdit               m_wndPriceCoverEdit;
    CSageButton         m_wndPriceAddBtn;
    CSageButton         m_wndPriceModifyBtn;
    CSageButton         m_wndPriceDeleteBtn;
    CSageButton         m_wndPriceCancelBtn;
    CStatic             m_wndPriceDetailHeader;
    CStatic             m_wndPriceDetailDivider;
    CStatic             m_wndPriceSummaryTitle;
    CStatic             m_wndPriceSummaryCount;
    CStatic             m_wndPriceSummaryRange;
    CRect               m_rectPriceSummaryCard;

    // ── 부수 계산 패널 ───────────────────────────────────────────────────────
    CStatic              m_wndCalcCompanyLabel;
    CSageComboBox    m_wndCalcCompanyCombo;
    CSageButton          m_wndCalcCompanyPickBtn;
    CStatic              m_wndCalcCopiesLabel;
    CEdit                m_wndCalcCopiesEdit;
    CStatic              m_wndCalcPagesLabel;
    CEdit                m_wndCalcPagesEdit;
    CSageButton          m_wndCalcBtn;
    CSageButton          m_wndCalcResetBtn;
    CStatic              m_wndCalcPrintLabel;
    CStatic              m_wndCalcPrintValue;
    CStatic              m_wndCalcCoverLabel;
    CStatic              m_wndCalcCoverValue;
    CStatic              m_wndCalcSubtotalLabel;
    CStatic              m_wndCalcSubtotalValue;
    CStatic              m_wndCalcFreightLabel;
    CEdit                m_wndCalcFreightEdit;
    CStatic              m_wndCalcFreightUnitLabel;
    CStatic              m_wndCalcDivider;
    CStatic              m_wndCalcTotalDivider;
    CStatic              m_wndCalcTotalLabel;
    CStatic              m_wndCalcTotalValue;
    CSageSectionLabel       m_wndCalcHistorySection;
    CSageHeaderCtrl  m_wndCalcHistoryHeader;
    CSageListCtrl        m_wndCalcHistoryList;
    CRect                m_rectCalcInputPanel;
    CRect                m_rectCalcResultPanel;
    CArray<CalcHistoryEntry, CalcHistoryEntry&> m_arrCalcHistory;

    // ── 가격 관리 내부 상태 ─────────────────────────────────────────────────
    LONGLONG m_nCalcPrintPrice;
    int  m_nCalcCoverPrice;
    int  m_nCalcUnitPrice;
    int  m_nPricePanelState;
    BOOL m_bFormattingCalcFreight;
    BOOL m_bFormattingPricePrint;
    BOOL m_bFormattingPriceCover;

    // ── 법인 순서 데이터 관리 패널 ───────────────────────────────────────────
    CSageButton         m_wndCoAddBtn;
    CSageButton         m_wndCoModifyBtn;
    CSageButton         m_wndCoDeleteBtn;
    CSageButton         m_wndCoCancelBtn;
    CStatic             m_wndCoSearchLabel;
    CEdit               m_wndCoSearchEdit;
    CSageButton         m_wndCoSearchBtn;
    CStatic             m_wndCoOrderLabel;
    CEdit               m_wndCoOrderEdit;
    CStatic             m_wndCoNameLabel;
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
    void ApplyWorkflowTabs();
    void ApplyResultColumns();
    void UpdateTaskTabVisibility();
    void UpdateResultColumns();
    void LayoutChildControls();
    void LayoutInputSection(int nLeft, int nTop, int nWidth, BOOL bShowOutput);
    void LayoutActionSection(int nLeft, int nTop, int nWidth);
    void LayoutResultSection(int nLeft, int nTop, int nWidth, int nHeight);
    void SetRunningState(BOOL bRunning);
    void UpdateAuthState();
    void UpdateProgressPercent(int nPercent);
    void SetStatusText(const CString& strStatus);
    int GetSelectedWorkflow() const;
    void UpdateWorkflowLabels();
    void UpdateExportButtonState();
    BOOL IsCompareWorkflow(int nWorkflowType) const;
    BOOL IsInputTabSelected() const;
    BOOL IsResultTab() const;
    BOOL IsDetailTab() const;
    BOOL IsExportTab() const;
    BOOL IsActionTabVisible() const;
    BOOL HasDocumentResultTab() const;
    int GetTaskTabVisualIndex(int nSemanticTabIndex) const;
    int GetTaskTabSemanticIndex(int nVisualTabIndex) const;
    BOOL IsReceivablesResultTable() const;
    BOOL IsDeliveryInputTable() const;
    BOOL IsEstimateInputTable() const;
    BOOL IsInputResetVisible() const;
    BOOL IsDocumentResultFilterVisible() const;
    BOOL IsDocumentWorkflowStateTarget(int nWorkflowType) const;
    TaechangWorkflowUiState& GetWorkflowUiState(int nWorkflowType);
    void SaveWorkflowUiState(int nWorkflowType);
    void RestoreWorkflowUiState(int nWorkflowType);
    void SaveCheckedRowNums(TaechangWorkflowUiState& state);
    void RestoreCheckedRowNums(const TaechangWorkflowUiState& state);
    void RebuildCurrentWorkflowResultList();
    COLORREF ResolveStatusColor(const CString& strStatus) const;
    SageBackgroundRole ResolveStatusBgRole(const CString& strStatus) const;
    void DrawEditBorder(CDC* pDC, CWnd& wnd);
    BOOL ValidateInputPath(CString& strInputPath);
    BOOL ValidateOutputFolder(CString& strOutputFolder);
    void EnableFileDropForWindow(CWnd& wnd);
    void ApplyDroppedInputPaths(const CString& strPaths);
    void RunWorkflowTask(int nTaskType);
    void DisplayResponse(int nWorkflowType, int nTaskType, const CString& strResponseJson);
    void InsertResultRow(const TaechangResultRow& row);
    void RefreshDocumentResultFilter();
    void PopulateResultFilterCriteria();
    int GetEffectiveFilterCriteria() const;
    int GetDefaultFilterCriteria() const;
    void AppendExecutionHistory(int nWorkflowType, int nTaskType, const CString& strResponseJson, BOOL bSuccess);
    CString BuildExecutionHistoryLine(int nWorkflowType, int nTaskType, const CString& strResponseJson, BOOL bSuccess) const;

    // ── 가격 관리 패널 ───────────────────────────────────────────────────────
    void CreatePriceManagePanel();
    void LayoutPriceManagePanel(int nLeft, int nTop, int nWidth, int nHeight);
    void ShowPriceManagePanel(BOOL bShow);
    void ApplyPriceRightPanel();
    void RefreshPriceCompanyList(const CString& strFilter = CString());
    void RefreshPriceCopiesList(const CString& strCompanyName);
    void UpdatePriceSummaryCard();
    void LoadSelectedCopiesRowToForm();
    void ClearPriceForm();
    BOOL ReadPriceFormToDto(TaechangPriceDto& dto, CString& strError);
    CString GetSelectedCompanyName() const;
    void FormatPriceEditText(CEdit& edit, BOOL& bFormatting);

    // ── 법인 순서 데이터 관리 패널 ───────────────────────────────────────────
    void CreateCompanyOrderPanel();
    void LayoutCompanyOrderPanel(int nLeft, int nTop, int nWidth, int nHeight);
    void ShowCompanyOrderPanel(BOOL bShow);
    BOOL IsDataManageTab() const;
    void RefreshCompanyOrderList();
    void UpdateCoListColumns();
    void UpdateCoPanelState();

    // ── 부수 계산 패널 ───────────────────────────────────────────────────────
    void CreatePriceCalcPanel();
    void LayoutPriceCalcPanel(int nLeft, int nTop, int nWidth, int nHeight);
    void ShowPriceCalcPanel(BOOL bShow);
    void RefreshCalcCompanyCombo();
    void ClearCalcInputAndResult();
    void ClearCalcResult();
    BOOL UpdateCalcPreview(BOOL bShowMessage);
    void UpdateCalcTotal();
    void AddCalcHistory(const CString& strCompany, int nCopies, int nPages, const CString& strItemName, const CString& strDate, LONGLONG nPrintPrice, int nCoverPrice, int nFreight, LONGLONG nTotal);
    void RefreshCalcHistoryList();
    int  GetCalcHistoryVisibleCapacity() const;
    void TrimCalcHistoryToVisibleCapacity();

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
    afx_msg void OnExportCsv();
    afx_msg void OnSelectAll();
    afx_msg void OnEstimateOnePage();
    afx_msg void OnInputReset();
    afx_msg LRESULT OnWorkflowComplete(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnResultListItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLogin();
    afx_msg void OnLogout();

    // ── 가격 데이터 관리 이벤트 ─────────────────────────────────────────────
    afx_msg void OnPriceCompanySelChanged();
    afx_msg void OnPriceCompanyEditChanged();
    afx_msg void OnPriceAddCompany();
    afx_msg void OnPriceRenameCompany();
    afx_msg void OnPriceDeleteCompany();
    afx_msg void OnPriceCopiesSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnPriceNoMaxCheck();
    afx_msg void OnPriceSingleCheck();
    afx_msg void OnPricePrintChanged();
    afx_msg void OnPriceCoverChanged();
    afx_msg void OnPriceAdd();
    afx_msg void OnPriceModify();
    afx_msg void OnPriceDelete();
    afx_msg void OnPriceCancel();

    // ── 법인 순서 데이터 관리 이벤트 ─────────────────────────────────────────
    afx_msg void OnCoAdd();
    afx_msg void OnCoModify();
    afx_msg void OnCoDelete();
    afx_msg void OnCoCancel();
    afx_msg void OnCoSearch();
    afx_msg void OnCoListSelChanged(NMHDR* pNMHDR, LRESULT* pResult);

    // ── 부수 계산 이벤트 ────────────────────────────────────────────────────
    afx_msg void OnCalc();
    afx_msg void OnCalcReset();
    afx_msg void OnCalcCompanyChanged();
    afx_msg void OnCalcInputChanged();
    afx_msg void OnCalcFreightChanged();
    afx_msg void OnCalcCompanyPick();
    afx_msg void OnResultSearch();
    afx_msg void OnResultFilterReset();
    afx_msg void OnResultFilterCriteriaChanged();

    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CSageTaechangDoc* CSageTaechangView::GetDocument() const
   { return reinterpret_cast<CSageTaechangDoc*>(m_pDocument); }
#endif

