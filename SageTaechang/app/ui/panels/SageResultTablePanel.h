#pragma once

#include "pch.h"
#include "app/core/workflow/SageWorkflowResultTable.h"
#include "app/core/workflow/TaechangWorkflowResultPresenter.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageFilterComboBox.h"
#include "app/ui/drawing/SageHeaderCtrl.h"
#include "app/ui/drawing/SageListCtrl.h"
#include "app/ui/drawing/SageSectionLabel.h"

class SageResultTablePanel : public CWnd {
public:
    SageResultTablePanel();

public:
    BOOL Create(CWnd* pParent, UINT nId);
    void Layout(const CRect& rectPanel);
    int  GetBandHeight() const;

public:
    void SetTitle(LPCWSTR pszTitle);
    void ShowSelectAll(BOOL bShow);
    void ShowOnePageOption(BOOL bShow);
    void ShowFilter(BOOL bShow);
    BOOL IsOnePageChecked() const;

    void SetColumns(const std::vector<SageWorkflowColumn>& arrColumns, const SageWorkflowResultStyle& style);
    void SetFilterCriteria(const std::vector<SageWorkflowFilterCriteria>& arrCriteria);
    void SetRows(const std::vector<TaechangResultRow>& arrRows);
    void ClearRows();

    int  GetRowCount() const;
    BOOL IsRowChecked(int nRow) const;
    void SetRowChecked(int nRow, BOOL bChecked);
    DWORD_PTR GetRowData(int nRow) const;
    CString GetCheckedRowNums() const;
    void RestoreCheckedRowNums(const CString& strCheckedRowNums);

    CString GetFilterKeyword() const;
    int  GetFilterCriteria() const;
    void RestoreFilter(const CString& strKeyword, int nCriteria);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnSearch();
    afx_msg void OnFilterReset();
    afx_msg void OnCriteriaChanged();
    afx_msg void OnSelectAll();
    afx_msg void OnOnePageOption();
    afx_msg void OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void ApplyControlFonts();
    void DrawEditBorder(CDC* pDC, CWnd& wnd);
    void RefreshRows();
    void UpdateColumnWidths();
    int  GetEffectiveCriteria() const;
    int  GetDefaultCriteria() const;
    void PopulateCriteria();
    void NotifyStateChanged();
    void TrimCheckedRowsToOnePage(BOOL bShowMessage);

private:
    CSageSectionLabel m_wndTitle;
    CSageButton m_wndSelectAll;
    CButton m_wndOnePage;
    CSageFilterComboBox m_wndCriteria;
    CEdit m_wndFilter;
    CSageButton m_wndSearchBtn;
    CSageButton m_wndResetBtn;
    CSageHeaderCtrl m_wndHeader;
    CSageListCtrl m_wndList;

private:
    std::vector<SageWorkflowColumn> m_arrColumns;
    std::vector<SageWorkflowFilterCriteria> m_arrCriteria;
    std::vector<TaechangResultRow> m_arrRows;
    SageWorkflowResultStyle m_style;
    CRect m_rectFilterCard;
    CString m_strKeyword;
    int m_nCriteria;
    BOOL m_bTitleVisible;
    BOOL m_bSelectAllVisible;
    BOOL m_bOnePageVisible;
    BOOL m_bFilterVisible;
};
