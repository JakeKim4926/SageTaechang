#pragma once

#include "pch.h"
#include "app/ui/panels/SageResultTablePanel.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageSectionLabel.h"
#include "app/ui/drawing/SageStatusCard.h"

class SageWorkflowInputPanel : public CWnd {
public:
    SageWorkflowInputPanel();

public:
    BOOL Create(CWnd* pParent, UINT nId);
    void Layout(const CRect& rectPanel);
    void EnableFileDrop();

public:
    void SetSectionLabel(LPCWSTR pszLabel);
    void SetActionButtonLabel(LPCWSTR pszLabel);
    void SetInputDialogTitle(LPCWSTR pszTitle);
    void SetAutoLoadOnInput(BOOL bAutoLoad);

    CString GetInputPath() const;
    CString GetOutputFolder() const;
    void SetInputPath(const CString& strInputPath);
    void SetOutputFolder(const CString& strOutputFolder);

    void SetRunningState(BOOL bRunning);
    void UpdateActionVisibility(BOOL bInputResetVisible);
    void UpdateInputTableVisibility(BOOL bTableVisible, BOOL bOnePageVisible, BOOL bFilterVisible);
    void EnableGenerateButton(BOOL bEnable);
    void ResetStatusCard();
    void SetStatusResult(BOOL bSuccess, const CString& strMessage, const CString& strDetail, BOOL bViewResultEnabled);

    SageResultTablePanel& GetInputTable();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnSelectInput();
    afx_msg void OnSelectOutput();
    afx_msg void OnGenerateWorkflow();
    afx_msg void OnInputReset();
    afx_msg void OnOpenOutputFolder();
    afx_msg void OnViewResultTab();
    afx_msg LRESULT OnResultTableChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnResultSelectionChanged(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void ApplyControlFonts();
    void ApplyLabelRoles();
    void LayoutInputCard(int nWidth);
    void LayoutActionSection();
    void LayoutTableArea();
    int  GetContentWidth() const;
    int  GetInputCardHeight() const;
    int  GetTableAreaTop() const;
    void LayoutFormRow(int nTop, int nWidth, CSageLabel& wndLabel, CEdit& wndEdit, CSageButton& wndButton);
    LRESULT ForwardToParent(UINT nMessage, WPARAM wParam, LPARAM lParam);
    void ApplyEditTextRect(CEdit& wndEdit);
    void DrawEditBorder(CDC* pDC, CWnd& wnd);
    void UpdateProgressPercent(int nPercent);
    void RequestRun(int nTaskType);

private:
    CSageSectionLabel m_wndCardHeader;
    CSageLabel m_wndInputLabel;
    CSageLabel m_wndOutputLabel;
    CEdit m_wndInputPath;
    CEdit m_wndOutputFolder;
    CSageButton m_wndSelectInput;
    CSageButton m_wndSelectOutput;
    CSageButton m_wndGenerate;
    CSageButton m_wndInputReset;
    CSageStatusCard m_wndStatusCard;
    CSageLabel m_wndEmptyStateHint;
    SageResultTablePanel m_panelInputTable;

private:
    CString m_strInputDialogTitle;
    BOOL m_bAutoLoadOnInput;
    BOOL m_bRunning;
    BOOL m_bInputResetVisible;
    BOOL m_bTableVisible;
    int m_nProgressPercent;
};
