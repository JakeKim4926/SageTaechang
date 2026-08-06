#pragma once

#include "pch.h"
#include "app/ui/drawing/SageSectionLabel.h"

class SageWorkflowHistoryPanel : public CWnd {
public:
    BOOL Create(CWnd* pParent, UINT nId);
    void Layout(const CRect& rectPanel);

public:
    void SetSectionLabel(LPCWSTR pszLabel);
    void AppendEntry(const CString& strInputPath, const CString& strResponseJson, BOOL bSuccess);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    DECLARE_MESSAGE_MAP()

private:
    CString BuildEntryLine(const CString& strInputPath, const CString& strResponseJson, BOOL bSuccess) const;

private:
    CSageSectionLabel m_wndSection;
    CEdit m_wndDetail;

private:
    CString m_strHistory;
};
