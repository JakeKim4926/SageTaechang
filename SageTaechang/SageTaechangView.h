
#pragma once

#include "TaechangDefine.h"

struct TaechangResultRow;

struct CalcHistoryEntry {
    CString strCompanyName;
    int nCopies;
    int nPages;
    LONGLONG nPrintPrice;
    int nCoverPrice;
    int nFreight;
    LONGLONG nTotal;
    CTime timeCalc;

    CalcHistoryEntry() : nCopies(0), nPages(0), nPrintPrice(0), nCoverPrice(0), nFreight(0), nTotal(0) {}
};

class CTaechangHeaderCtrl : public CHeaderCtrl
{
    DECLARE_MESSAGE_MAP()
protected:
    afx_msg void OnPaint();
};

class CTaechangTabCtrl : public CTabCtrl
{
    DECLARE_MESSAGE_MAP()
protected:
    afx_msg void OnPaint();
};

class CTaechangComboBox : public CComboBox
{
    DECLARE_MESSAGE_MAP()
protected:
    afx_msg void OnPaint();
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
    CTreeCtrl m_wndSidebarTree;
    CStatic m_wndHeaderTitle;
    CStatic m_wndHeaderStatus;
    CTaechangTabCtrl m_wndTaskTabs;
    CStatic m_wndInputSection;
    CStatic m_wndOutputSection;
    CStatic m_wndResultSection;
    CStatic m_wndDetailSection;
    CStatic m_wndTitle;
    CStatic m_wndWorkflowLabel;
    CStatic m_wndInputLabel;
    CStatic m_wndOutputLabel;
    CEdit m_wndInputPath;
    CEdit m_wndOutputFolder;
    CButton m_wndSelectInput;
    CButton m_wndSelectOutput;
    CButton m_wndLoad;
    CButton m_wndGenerate;
    CButton m_wndExportCsv;
    CButton m_wndSelectAll;
    CProgressCtrl m_wndProgress;
    CStatic m_wndProgressText;
    CTaechangHeaderCtrl m_wndResultHeader;
    CListCtrl m_wndResultList;
    CEdit m_wndReceivablesFilter;
    CButton m_wndReceivablesSearchBtn;
    CButton m_wndReceivablesResetBtn;
    CEdit m_wndDetail;
    CStatic m_wndEmptyStateHint;
    CStatic m_wndActionStatus;
    CFont m_fontTitle;
    CFont m_fontHeader;
    CFont m_fontControl;
    CFont m_fontContent;
    CBrush m_brushAppBackground;
    CBrush m_brushPanel;
    CBrush m_brushSidebar;
    CBrush m_brushListHeader;
    CBrush m_brushHeaderStatus;
    BOOL m_bRunning;
    int m_nProgressPercent;
    int m_nSelectedTaskTab;
    int m_nLastWorkflowType;
    int m_nLastTaskType;
    int m_nCurrentWorkflow;
    HTREEITEM m_hLastWorkflowItem;
    COLORREF m_colorHeaderStatus;
    COLORREF m_colorHeaderStatusBg;
    BOOL m_bLastTaskSuccess;
    CString m_strLastResponseJson;
    CString m_strExecutionHistory;
    CString m_strRunningInputPath;
    CString m_strReceivablesFilterKeyword;

    CButton m_wndLoginBtn;
    CButton m_wndLogoutBtn;
    CStatic m_wndUserLabel;

    // ── 가격 데이터 관리 패널 ────────────────────────────────────────────────
    CStatic             m_wndPriceCompanyLabel;
    CTaechangComboBox   m_wndPriceCompanyCombo;
    CButton             m_wndPriceAddCompanyBtn;
    CButton             m_wndPriceRenameCompanyBtn;
    CButton             m_wndPriceDeleteCompanyBtn;
    CTaechangHeaderCtrl m_wndPriceCopiesHeader;
    CListCtrl           m_wndPriceCopiesList;
    CStatic             m_wndPriceMinCopiesLabel;
    CEdit               m_wndPriceMinCopiesEdit;
    CStatic             m_wndPriceMaxCopiesLabel;
    CEdit               m_wndPriceMaxCopiesEdit;
    CButton             m_wndPriceNoMaxCheck;
    CStatic             m_wndPricePrintLabel;
    CEdit               m_wndPricePrintEdit;
    CStatic             m_wndPriceCoverLabel;
    CEdit               m_wndPriceCoverEdit;
    CButton             m_wndPriceAddBtn;
    CButton             m_wndPriceModifyBtn;
    CButton             m_wndPriceDeleteBtn;
    CButton             m_wndPriceCancelBtn;
    CStatic             m_wndPriceSummaryTitle;
    CStatic             m_wndPriceSummaryCount;
    CStatic             m_wndPriceSummaryRange;
    CRect               m_rectPriceSummaryCard;

    // ── 부수 계산 패널 ───────────────────────────────────────────────────────
    CStatic              m_wndCalcCompanyLabel;
    CTaechangComboBox    m_wndCalcCompanyCombo;
    CStatic              m_wndCalcCopiesLabel;
    CEdit                m_wndCalcCopiesEdit;
    CStatic              m_wndCalcPagesLabel;
    CEdit                m_wndCalcPagesEdit;
    CButton              m_wndCalcBtn;
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
    CStatic              m_wndCalcHistorySection;
    CTaechangHeaderCtrl  m_wndCalcHistoryHeader;
    CListCtrl            m_wndCalcHistoryList;
    CRect                m_rectCalcInputPanel;
    CRect                m_rectCalcResultPanel;
    CArray<CalcHistoryEntry, CalcHistoryEntry&> m_arrCalcHistory;

    // ── 가격 관리 내부 상태 ─────────────────────────────────────────────────
    LONGLONG m_nCalcPrintPrice;
    int  m_nCalcCoverPrice;
    int  m_nPricePanelState;

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
    COLORREF ResolveStatusColor(const CString& strStatus) const;
    COLORREF ResolveStatusBgColor(const CString& strStatus) const;
    void DrawSectionLabel(LPDRAWITEMSTRUCT lpDrawItemStruct);
    void DrawEditBorder(CDC* pDC, CWnd& wnd);
    BOOL ValidateInputPath(CString& strInputPath);
    BOOL ValidateOutputFolder(CString& strOutputFolder);
    void EnableFileDropForWindow(CWnd& wnd);
    void ApplyDroppedInputPaths(const CString& strPaths);
    void RunWorkflowTask(int nTaskType);
    void DisplayResponse(int nWorkflowType, int nTaskType, const CString& strResponseJson);
    void InsertResultRow(const TaechangResultRow& row);
    void RefreshReceivablesResultFilter();
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

    // ── 부수 계산 패널 ───────────────────────────────────────────────────────
    void CreatePriceCalcPanel();
    void LayoutPriceCalcPanel(int nLeft, int nTop, int nWidth, int nHeight);
    void ShowPriceCalcPanel(BOOL bShow);
    void RefreshCalcCompanyCombo();
    void UpdateCalcTotal();
    void AddCalcHistory(const CString& strCompany, int nCopies, int nPages, LONGLONG nPrintPrice, int nCoverPrice, int nFreight, LONGLONG nTotal);
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
    afx_msg LRESULT OnWorkflowComplete(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    afx_msg void OnSidebarTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnListCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
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
    afx_msg void OnPriceAdd();
    afx_msg void OnPriceModify();
    afx_msg void OnPriceDelete();
    afx_msg void OnPriceCancel();

    // ── 부수 계산 이벤트 ────────────────────────────────────────────────────
    afx_msg void OnCalc();
    afx_msg void OnCalcFreightChanged();
    afx_msg void OnReceivablesSearch();
    afx_msg void OnReceivablesFilterReset();

    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CSageTaechangDoc* CSageTaechangView::GetDocument() const
   { return reinterpret_cast<CSageTaechangDoc*>(m_pDocument); }
#endif

