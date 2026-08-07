#pragma once

#include "pch.h"
#include "app/ui/drawing/SageListCtrl.h"
#include "app/ui/drawing/SageEmptyState.h"
#include "app/ui/drawing/SageHeaderCtrl.h"
#include "app/ui/drawing/SageFilterPillBar.h"

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
    afx_msg void OnFilterChanged();
    DECLARE_MESSAGE_MAP()

private:
    void LayoutChildren();
    void CreateColumns();
    void ApplyRowStyles();
    void InsertRow(int nItem, const SageHistoryRow& row);
    void UpdateColumnWidths();
    void UpdateEmptyState();
    void UpdateFilterLabels();
    void RebuildVisibleRows();
    BOOL IsRowVisible(const SageHistoryRow& row) const;
    SageHistoryRow BuildRow(const CString& strInputPath, const CString& strResponseJson, BOOL bSuccess) const;

private:
    CSageFilterPillBar m_wndFilterPills;
    CSageListCtrl m_wndList;
    CSageHeaderCtrl m_wndHeader;
    CSageEmptyState m_wndEmpty;

private:
    std::vector<SageHistoryRow> m_arrRows;
};
