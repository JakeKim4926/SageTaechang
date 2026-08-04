#include "pch.h"
#include "app/core/workflow/handlers/SageDeliveryWorkflowHandler.h"
#include "TaechangDefine.h"

int SageDeliveryWorkflowHandler::GetWorkflowType() const {
	return TAECHANG_WORKFLOW_DELIVERY;
}

LPCWSTR SageDeliveryWorkflowHandler::GetHeaderTitle() const {
	return TAECHANG_UI_DELIVERY_NAME;
}

LPCWSTR SageDeliveryWorkflowHandler::GetInputSectionLabel() const {
	return TAECHANG_UI_SECTION_INPUT;
}

LPCWSTR SageDeliveryWorkflowHandler::GetActionButtonLabel() const {
	return TAECHANG_UI_DELIVERY_GENERATE_BUTTON;
}

LPCWSTR SageDeliveryWorkflowHandler::GetDetailSectionLabel() const {
	return TAECHANG_UI_SECTION_HISTORY;
}
