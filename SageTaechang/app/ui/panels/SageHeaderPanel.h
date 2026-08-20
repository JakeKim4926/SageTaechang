#pragma once

#include "pch.h"
#include "app/ui/drawing/SageBadge.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageLabel.h"

class SageHeaderPanel : public CWnd {
public:
    BOOL Create(CWnd* pParent, UINT nId);
    void Layout(const CRect& rectPanel);

public:
    void SetTitle(LPCWSTR pszTitle);
    void SetCategory(const CString& strCategory);
    void UpdateAuthState();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLogin();
    afx_msg void OnLogout();
    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void LayoutChildren();
    int  GetTitleWidth();

private:
    CSageLabel m_wndTitle;
    CSageLabel m_wndCategory;
    CSageLabel m_wndUserLabel;
    CSageBadge m_wndRoleBadge;
    CSageButton m_wndLoginBtn;
    CSageButton m_wndLogoutBtn;
};
