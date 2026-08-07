
#pragma once

#include "TaechangDefine.h"
#include "app/ui/panels/SageWorkspacePanel.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/ui/drawing/SageSidebarTree.h"

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
    CSageLabel m_wndSidebarTitle;
    CSageSidebarTree m_wndSidebarTree;
    CSageLabel m_wndHeaderTitle;
    CStatic m_wndHeaderStatus;
    CSageLabel m_wndTitle;
    CBrush m_brushListHeader;
    int m_nCurrentWorkflow;
    HTREEITEM m_hLastWorkflowItem;
    COLORREF m_colorHeaderStatus;
    SageBackgroundRole m_nHeaderStatusBgRole;

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
    void LayoutChildControls();
    void UpdateAuthState();
    void SetStatusText(const CString& strStatus);
    int GetSelectedWorkflow() const;
    ISageWorkflowHandler* FindCurrentHandler() const;
    COLORREF ResolveStatusColor(const CString& strStatus) const;
    SageBackgroundRole ResolveStatusBgRole(const CString& strStatus) const;
    void DrawShellBands(CDC* pDC, const CRect& rectClient);
    void InvalidateContentArea();
    void EnableFileDropForWindow(CWnd& wnd);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnWorkflowChanged();
    afx_msg void OnSidebarSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnWorkspaceTabChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkspaceStatus(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWorkspaceStateChanged(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnLogin();
    afx_msg void OnLogout();

    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CSageTaechangDoc* CSageTaechangView::GetDocument() const
   { return reinterpret_cast<CSageTaechangDoc*>(m_pDocument); }
#endif

