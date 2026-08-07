#pragma once

#include "pch.h"

struct SageWorkflowRunRequest {
    SageWorkflowRunRequest() {
        hNotifyWnd = NULL;
        nWorkflowType = 0;
        nTaskType = 0;
        bEstimateOnePage = FALSE;
    }

    HWND hNotifyWnd;
    int nWorkflowType;
    int nTaskType;
    CString strInputPath;
    CString strOutputFolder;
    CString strSelectedRowNums;
    BOOL bEstimateOnePage;
};

struct SageWorkflowResult {
    SageWorkflowResult() {
        m_nWorkflowType = 0;
        m_nTaskType = 0;
    }

    int m_nWorkflowType;
    int m_nTaskType;
    CString m_strResponseJson;
};

struct SageWorkflowResultState {
    SageWorkflowResultState() {
        nWorkflowType = 0;
        nTaskType = 0;
        bSuccess = FALSE;
    }

    int nWorkflowType;
    int nTaskType;
    BOOL bSuccess;
    CString strResponseJson;
    CString strInputPath;
};

class SageWorkflowController {
public:
    SageWorkflowController();

public:
    BOOL IsRunning() const;
    BOOL Start(const SageWorkflowRunRequest& request, CString& strError);
    void Finish(int nWorkflowType, int nTaskType, const CString& strResponseJson, BOOL bSuccess, BOOL bKeepResult);

public:
    int  GetLastWorkflowType() const;
    int  GetLastTaskType() const;
    BOOL IsLastTaskSuccess() const;
    const CString& GetLastResponseJson() const;
    const CString& GetRunningInputPath() const;

    void ClearResult();
    SageWorkflowResultState CaptureResult() const;
    void RestoreResult(const SageWorkflowResultState& state);

private:
    BOOL m_bRunning;
    int m_nLastWorkflowType;
    int m_nLastTaskType;
    BOOL m_bLastTaskSuccess;
    CString m_strLastResponseJson;
    CString m_strRunningInputPath;
};
