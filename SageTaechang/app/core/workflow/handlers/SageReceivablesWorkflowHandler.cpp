#include "pch.h"
#include "app/core/workflow/handlers/SageReceivablesWorkflowHandler.h"
#include "TaechangDefine.h"

int SageReceivablesWorkflowHandler::GetWorkflowType() const {
	return TAECHANG_WORKFLOW_RECEIVABLES;
}

LPCWSTR SageReceivablesWorkflowHandler::GetHeaderTitle() const {
	return TAECHANG_UI_RECEIVABLES_NAME;
}

LPCWSTR SageReceivablesWorkflowHandler::GetInputSectionLabel() const {
	return TAECHANG_UI_SECTION_INPUT;
}

LPCWSTR SageReceivablesWorkflowHandler::GetActionButtonLabel() const {
	return TAECHANG_UI_RECEIVABLES_GENERATE_BUTTON;
}

LPCWSTR SageReceivablesWorkflowHandler::GetDetailSectionLabel() const {
	return TAECHANG_UI_SECTION_HISTORY;
}
