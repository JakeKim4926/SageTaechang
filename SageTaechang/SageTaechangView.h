
// SageTaechangView.h: CSageTaechangView 클래스의 인터페이스
//

#pragma once

#include "TaechangDefine.h"

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
    CStatic m_wndTitle;
    CStatic m_wndWorkflowLabel;
    CComboBox m_wndWorkflow;
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
    BOOL m_bRunning;
    int m_nLastWorkflowType;
    int m_nLastTaskType;
    CString m_strLastResponseJson;

protected:
    void CreateChildControls();
    void LayoutChildControls();
    void SetRunningState(BOOL bRunning);
    void SetStatusText(const CString& strStatus);
    int GetSelectedWorkflow() const;
    void UpdateWorkflowLabels();
    void UpdateExportButtonState();
    BOOL IsCompareWorkflow(int nWorkflowType) const;
    BOOL ValidateInputPath(CString& strInputPath);
    BOOL ValidateOutputFolder(CString& strOutputFolder);
    void RunWorkflowTask(int nTaskType);
    void DisplayResponse(int nWorkflowType, int nTaskType, const CString& strResponseJson);
    void InsertResultRow(
        const CString& strField,
        const CString& strValue,
        const CString& strStatus,
        const CString& strReason);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnWorkflowChanged();
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

