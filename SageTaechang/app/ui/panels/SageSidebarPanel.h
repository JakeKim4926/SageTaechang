#pragma once

#include "pch.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageSidebarTree.h"

class SageSidebarPanel : public CWnd {
public:
    SageSidebarPanel();

public:
    BOOL Create(CWnd* pParent, UINT nId);
    void Layout(const CRect& rectPanel);
    void BuildTree();
    int  GetSelectedWorkflow() const;
    CString GetSelectedCategory() const;

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    BOOL IsLoginRequired(DWORD_PTR nItemData) const;
    void RestoreLastSelection();
    void NotifyParent(UINT nMessage, WPARAM wParam);

private:
    CSageLabel m_wndTitle;
    CSageSidebarTree m_wndTree;

private:
    HTREEITEM m_hLastWorkflowItem;
    int m_nSelectedWorkflow;
};
