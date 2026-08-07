#include "pch.h"
#include "app/ui/workflow/SageWorkflowController.h"
#include "app/common/TaechangJson.h"
#include "app/core/workflow/TaechangWorkflowResponse.h"
#include "app/infra/office/TaechangDeliveryExcelService.h"
#include "app/infra/office/TaechangEstimateExcelService.h"
#include "app/infra/office/TaechangReceivablesExcelService.h"
#include "TaechangDefine.h"

namespace {

struct SageWorkflowTask {
	HWND m_hWnd;
	int m_nWorkflowType;
	int m_nTaskType;
	CString m_strInputPath;
	CString m_strOutputFolder;
	CString m_strSelectedRowNums;
	BOOL m_bEstimateOnePage;
};

CString BuildWorkflowPayload(const CString& strInputPath, const CString& strOutputFolder, const CString& strRowNums, BOOL bEstimateOnePage) {
	CString strPayload = L"{\"inputPath\":\"" + JsonEscapeString(strInputPath) + L"\"";
	if (!strOutputFolder.IsEmpty())
		strPayload += L",\"outputFolder\":\"" + JsonEscapeString(strOutputFolder) + L"\"";
	if (!strRowNums.IsEmpty())
		strPayload += L",\"" + CString(TAECHANG_JSON_KEY_ROW_NUMS) + L"\":\"" + JsonEscapeString(strRowNums) + L"\"";
	if (bEstimateOnePage)
		strPayload += L",\"" + CString(TAECHANG_JSON_KEY_ESTIMATE_ONE_PAGE) + L"\":true";
	strPayload += L"}";
	return strPayload;
}

CString GetTaskRequestId(const SageWorkflowTask* pTask) {
	if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) {
		if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
			return TAECHANG_REQUEST_ESTIMATE_LOAD;
		return TAECHANG_REQUEST_ESTIMATE_GENERATE;
	}
	if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_DELIVERY) {
		if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
			return TAECHANG_REQUEST_DELIVERY_LOAD;
		return TAECHANG_REQUEST_DELIVERY_GENERATE;
	}
	if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
		return TAECHANG_REQUEST_RECEIVABLES_LOAD;
	return TAECHANG_REQUEST_RECEIVABLES_GENERATE;
}

UINT RunWorkflowWorker(LPVOID pParam) {
	SageWorkflowTask* pTask = reinterpret_cast<SageWorkflowTask*>(pParam);
	SageWorkflowResult* pResult = new SageWorkflowResult();
	pResult->m_nWorkflowType = pTask->m_nWorkflowType;
	pResult->m_nTaskType = pTask->m_nTaskType;

	try {
		CString strPayload = BuildWorkflowPayload(pTask->m_strInputPath, pTask->m_strOutputFolder, pTask->m_strSelectedRowNums, pTask->m_bEstimateOnePage);
		if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) {
			TaechangEstimateExcelService service;
			if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
				pResult->m_strResponseJson = service.BuildLoadInputDataResponse(TAECHANG_REQUEST_ESTIMATE_LOAD, strPayload);
			else
				pResult->m_strResponseJson = service.BuildGenerateResponse(TAECHANG_REQUEST_ESTIMATE_GENERATE, strPayload);
		} else if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_DELIVERY) {
			TaechangDeliveryExcelService service;
			if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
				pResult->m_strResponseJson = service.BuildLoadInputDataResponse(TAECHANG_REQUEST_DELIVERY_LOAD, strPayload);
			else
				pResult->m_strResponseJson = service.BuildGenerateResponse(TAECHANG_REQUEST_DELIVERY_GENERATE, strPayload);
		} else {
			TaechangReceivablesExcelService service;
			if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
				pResult->m_strResponseJson = service.BuildLoadInputDataResponse(TAECHANG_REQUEST_RECEIVABLES_LOAD, strPayload);
			else
				pResult->m_strResponseJson = service.BuildGenerateResponse(TAECHANG_REQUEST_RECEIVABLES_GENERATE, strPayload);
		}
	} catch (...) {
		pResult->m_strResponseJson = BuildErrorResponse(
			GetTaskRequestId(pTask),
			TAECHANG_ERROR_CODE_WORKFLOW_EXCEPTION,
			TAECHANG_UI_WORKFLOW_EXCEPTION);
	}

	HWND hWnd = pTask->m_hWnd;
	delete pTask;

	if (!::IsWindow(hWnd) ||
		!::PostMessageW(hWnd, WM_TAECHANG_WORKFLOW_COMPLETE, 0, reinterpret_cast<LPARAM>(pResult)))
		delete pResult;

	return 0;
}

}

SageWorkflowController::SageWorkflowController()
	: m_bRunning(FALSE)
	, m_nLastWorkflowType(0)
	, m_nLastTaskType(0)
	, m_bLastTaskSuccess(FALSE) {
}

BOOL SageWorkflowController::IsRunning() const {
	return m_bRunning;
}

BOOL SageWorkflowController::Start(const SageWorkflowRunRequest& request, CString& strError) {
	if (m_bRunning) {
		strError = TAECHANG_UI_WORKFLOW_ALREADY_RUNNING;
		return FALSE;
	}
	if (!::IsWindow(request.hNotifyWnd)) {
		strError = TAECHANG_UI_WORKFLOW_START_FAILED;
		return FALSE;
	}

	SageWorkflowTask* pTask = new SageWorkflowTask();
	pTask->m_hWnd = request.hNotifyWnd;
	pTask->m_nWorkflowType = request.nWorkflowType;
	pTask->m_nTaskType = request.nTaskType;
	pTask->m_strInputPath = request.strInputPath;
	pTask->m_strOutputFolder = request.strOutputFolder;
	pTask->m_strSelectedRowNums = request.strSelectedRowNums;
	pTask->m_bEstimateOnePage = request.bEstimateOnePage;

	if (AfxBeginThread(RunWorkflowWorker, pTask, THREAD_PRIORITY_NORMAL, 0, 0, NULL) == NULL) {
		delete pTask;
		strError = TAECHANG_UI_WORKFLOW_START_FAILED;
		return FALSE;
	}

	m_strRunningInputPath = request.strInputPath;
	m_bRunning = TRUE;
	return TRUE;
}

void SageWorkflowController::Finish(int nWorkflowType, int nTaskType, const CString& strResponseJson, BOOL bSuccess, BOOL bKeepResult) {
	m_bRunning = FALSE;
	m_nLastWorkflowType = nWorkflowType;
	m_bLastTaskSuccess = bSuccess;
	if (bKeepResult)
		return;
	m_nLastTaskType = nTaskType;
	m_strLastResponseJson = strResponseJson;
}

int SageWorkflowController::GetLastWorkflowType() const {
	return m_nLastWorkflowType;
}

int SageWorkflowController::GetLastTaskType() const {
	return m_nLastTaskType;
}

BOOL SageWorkflowController::IsLastTaskSuccess() const {
	return m_bLastTaskSuccess;
}

const CString& SageWorkflowController::GetLastResponseJson() const {
	return m_strLastResponseJson;
}

const CString& SageWorkflowController::GetRunningInputPath() const {
	return m_strRunningInputPath;
}

void SageWorkflowController::ClearResult() {
	m_nLastWorkflowType = 0;
	m_nLastTaskType = 0;
	m_bLastTaskSuccess = FALSE;
	m_strLastResponseJson.Empty();
	m_strRunningInputPath.Empty();
}

SageWorkflowResultState SageWorkflowController::CaptureResult() const {
	SageWorkflowResultState state;
	state.nWorkflowType = m_nLastWorkflowType;
	state.nTaskType = m_nLastTaskType;
	state.bSuccess = m_bLastTaskSuccess;
	state.strResponseJson = m_strLastResponseJson;
	state.strInputPath = m_strRunningInputPath;
	return state;
}

void SageWorkflowController::RestoreResult(const SageWorkflowResultState& state) {
	m_nLastWorkflowType = state.nWorkflowType;
	m_nLastTaskType = state.nTaskType;
	m_bLastTaskSuccess = state.bSuccess;
	m_strLastResponseJson = state.strResponseJson;
	m_strRunningInputPath = state.strInputPath;
}
