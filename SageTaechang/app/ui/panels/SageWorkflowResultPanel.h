#pragma once

#include "pch.h"
#include "app/ui/panels/SageResultTablePanel.h"

class SageWorkflowResultPanel : public CWnd {
public:
    BOOL Create(CWnd* pParent, UINT nId);
    void Layout(const CRect& rectPanel);
    void EnableFileDrop();
    int  GetBandHeight() const;

public:
    void UpdateResultTableVisibility(BOOL bFilterVisible);
    SageResultTablePanel& GetResultTable();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg LRESULT OnResultTableChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnResultSelectionChanged(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

private:
    void LayoutResultTable();
    LRESULT ForwardToParent(UINT nMessage, WPARAM wParam, LPARAM lParam);

private:
    SageResultTablePanel m_panelResultTable;
};
