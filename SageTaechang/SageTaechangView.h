
// SageTaechangView.h: CSageTaechangView 클래스의 인터페이스
//

#pragma once

#include "TaechangDefine.h"

struct TaechangResultRow;

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

public:
    virtual ~CSageTaechangView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    CStatic m_wndSidebarTitle;
    CListBox m_wndWorkflowMenu;
    CStatic m_wndHeaderTitle;
    CStatic m_wndHeaderStatus;
    CTabCtrl m_wndTaskTabs;
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
    CButton m_wndSettings;
    CProgressCtrl m_wndProgress;
    CListCtrl m_wndResultList;
    CEdit m_wndDetail;
    CFont m_fontTitle;
    CFont m_fontHeader;
    CFont m_fontControl;
    CBrush m_brushAppBackground;
    CBrush m_brushPanel;
    CBrush m_brushSidebar;
    BOOL m_bRunning;
    int m_nSelectedTaskTab;
    int m_nLastWorkflowType;
    int m_nLastTaskType;
    CString m_strLastResponseJson;

protected:
    void CreateChildControls();
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
    BOOL IsSettingsButtonVisible() const;
    BOOL ValidateInputPath(CString& strInputPath);
    BOOL ValidateOutputFolder(CString& strOutputFolder);
    void RunWorkflowTask(int nTaskType);
    void DisplayResponse(int nWorkflowType, int nTaskType, const CString& strResponseJson);
    void InsertResultRow(const TaechangResultRow& row);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnWorkflowChanged();
    afx_msg void OnTaskTabChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelectInput();
    afx_msg void OnSelectOutput();
    afx_msg void OnLoadWorkflow();
    afx_msg void OnGenerateWorkflow();
    afx_msg void OnExportCsv();
    afx_msg void OnSettings();
    afx_msg LRESULT OnWorkflowComplete(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CSageTaechangDoc* CSageTaechangView::GetDocument() const
   { return reinterpret_cast<CSageTaechangDoc*>(m_pDocument); }
#endif

