#include "pch.h"
#include "app/core/workflow/handlers/SageEstimateWorkflowHandler.h"
#include "TaechangDefine.h"

int SageEstimateWorkflowHandler::GetWorkflowType() const {
	return TAECHANG_WORKFLOW_ESTIMATE;
}

LPCWSTR SageEstimateWorkflowHandler::GetHeaderTitle() const {
	return TAECHANG_UI_ESTIMATE_NAME;
}

LPCWSTR SageEstimateWorkflowHandler::GetInputSectionLabel() const {
	return TAECHANG_UI_SECTION_INPUT;
}

LPCWSTR SageEstimateWorkflowHandler::GetActionButtonLabel() const {
	return TAECHANG_UI_ESTIMATE_GENERATE_BUTTON;
}

LPCWSTR SageEstimateWorkflowHandler::GetDetailSectionLabel() const {
	return TAECHANG_UI_SECTION_HISTORY;
}
