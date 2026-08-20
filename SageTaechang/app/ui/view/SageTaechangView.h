
#pragma once

#include "SageDefine.h"
#include "app/ui/panels/SageWorkspacePanel.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/ui/panels/SageHeaderPanel.h"
#include "app/ui/panels/SageSidebarPanel.h"

class ISageWorkflowHandler;

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
    CBrush m_brushListHeader;
    int m_nCurrentWorkflow;

    SageHeaderPanel m_panelHeader;
    SageSidebarPanel m_panelSidebar;
    SageWorkspacePanel m_panelWorkspace;

protected:
    void CreateChildControls();
    void LayoutChildControls();
    void SetStatusText(const CString& strStatus);
    int GetSelectedWorkflow() const;
    ISageWorkflowHandler* FindCurrentHandler() const;
    void InvalidateContentArea();
    void EnableFileDropForWindow(CWnd& wnd);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnWorkflowChanged();
    afx_msg LRESULT OnSidebarWorkflow(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSidebarAction(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkspaceTabChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkspaceStatus(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkspaceStateChanged(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDropFiles(HDROP hDropInfo);

    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CSageTaechangDoc* CSageTaechangView::GetDocument() const
   { return reinterpret_cast<CSageTaechangDoc*>(m_pDocument); }
#endif

