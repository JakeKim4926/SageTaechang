#include "pch.h"
#include "app/core/workflow/handlers/SageHwpCompareWorkflowHandler.h"
#include "TaechangDefine.h"

int SageHwpCompareWorkflowHandler::GetWorkflowType() const {
	return TAECHANG_WORKFLOW_HWP_COMPARE;
}

LPCWSTR SageHwpCompareWorkflowHandler::GetHeaderTitle() const {
	return TAECHANG_UI_HWP_COMPARE_NAME;
}

LPCWSTR SageHwpCompareWorkflowHandler::GetInputSectionLabel() const {
	return TAECHANG_UI_SECTION_INSPECTION_INPUT;
}

LPCWSTR SageHwpCompareWorkflowHandler::GetActionButtonLabel() const {
	return TAECHANG_UI_HWP_COMPARE_BUTTON;
}

LPCWSTR SageHwpCompareWorkflowHandler::GetDetailSectionLabel() const {
	return TAECHANG_UI_SECTION_DETAIL;
}
