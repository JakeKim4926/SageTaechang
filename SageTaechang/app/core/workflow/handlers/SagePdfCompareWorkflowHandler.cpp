#include "pch.h"
#include "app/core/workflow/handlers/SagePdfCompareWorkflowHandler.h"
#include "TaechangDefine.h"

int SagePdfCompareWorkflowHandler::GetWorkflowType() const {
	return TAECHANG_WORKFLOW_PDF_COMPARE;
}

LPCWSTR SagePdfCompareWorkflowHandler::GetHeaderTitle() const {
	return TAECHANG_UI_PDF_COMPARE_NAME;
}

LPCWSTR SagePdfCompareWorkflowHandler::GetInputSectionLabel() const {
	return TAECHANG_UI_SECTION_INSPECTION_INPUT;
}

LPCWSTR SagePdfCompareWorkflowHandler::GetActionButtonLabel() const {
	return TAECHANG_UI_PDF_COMPARE_BUTTON;
}

LPCWSTR SagePdfCompareWorkflowHandler::GetDetailSectionLabel() const {
	return TAECHANG_UI_SECTION_DETAIL;
}
