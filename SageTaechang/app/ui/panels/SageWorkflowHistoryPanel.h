#pragma once

#include "pch.h"
#include "app/ui/drawing/SageListCtrl.h"
#include "app/ui/drawing/SageEmptyState.h"
#include "app/ui/drawing/SageHeaderCtrl.h"

struct SageHistoryRow
{
    SageHistoryRow() {
        bSuccess = FALSE;
    }

    CString strTime;
    CString strInputPath;
    CString strOutputPath;
    CString strReason;
    BOOL bSuccess;
};

class SageWorkflowHistoryPanel : public CWnd {
public:
    BOOL Create(CWnd* pParent, UINT nId);
    void Layout(const CRect& rectPanel);

public:
    void AppendEntry(const CString& strInputPath, const CString& strResponseJson, BOOL bSuccess);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    DECLARE_MESSAGE_MAP()

private:
    void CreateColumns();
    void ApplyRowStyles();
    void InsertRow(int nItem, const SageHistoryRow& row);
    void UpdateColumnWidths();
    void UpdateEmptyState();
    SageHistoryRow BuildRow(const CString& strInputPath, const CString& strResponseJson, BOOL bSuccess) const;

private:
    CSageListCtrl m_wndList;
    CSageHeaderCtrl m_wndHeader;
    CSageEmptyState m_wndEmpty;

private:
    std::vector<SageHistoryRow> m_arrRows;
};
