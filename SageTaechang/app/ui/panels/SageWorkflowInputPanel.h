#pragma once

#include "pch.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageSectionLabel.h"

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
    void UpdateActionVisibility(BOOL bInputResetVisible, BOOL bHasLastResult);
    void EnableGenerateButton(BOOL bEnable);
    void SetActionStatusText(LPCWSTR pszStatus, BOOL bSuccess);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnSelectInput();
    afx_msg void OnSelectOutput();
    afx_msg void OnLoadWorkflow();
    afx_msg void OnGenerateWorkflow();
    afx_msg void OnInputReset();
    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void ApplyControlFonts();
    void LayoutInputSection(int nWidth);
    void LayoutActionSection();
    int  GetContentWidth() const;
    void ApplyEditTextRect(CEdit& wndEdit);
    void DrawEditBorder(CDC* pDC, CWnd& wnd);
    void UpdateProgressPercent(int nPercent);
    void RequestRun(int nTaskType);

private:
    CSageSectionLabel m_wndInputSection;
    CSageSectionLabel m_wndOutputSection;
    CSageLabel m_wndWorkflowLabel;
    CSageLabel m_wndInputLabel;
    CSageLabel m_wndOutputLabel;
    CEdit m_wndInputPath;
    CEdit m_wndOutputFolder;
    CSageButton m_wndSelectInput;
    CSageButton m_wndSelectOutput;
    CSageButton m_wndLoad;
    CSageButton m_wndGenerate;
    CSageButton m_wndInputReset;
    CProgressCtrl m_wndProgress;
    CSageLabel m_wndProgressText;
    CStatic m_wndActionStatus;

private:
    CString m_strInputDialogTitle;
    BOOL m_bAutoLoadOnInput;
    BOOL m_bRunning;
    BOOL m_bInputResetVisible;
    BOOL m_bLastActionSuccess;
    int m_nProgressPercent;
};
