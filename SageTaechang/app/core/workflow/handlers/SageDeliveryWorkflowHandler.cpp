#include "pch.h"
#include "app/core/workflow/handlers/SageDeliveryWorkflowHandler.h"
#include "TaechangDefine.h"

namespace {

const SageWorkflowTab g_tabs[] = {
	{ TAECHANG_TAB_INDEX_INPUT, TAECHANG_UI_TAB_INPUT },
	{ TAECHANG_TAB_INDEX_DOCUMENT_HISTORY, TAECHANG_UI_TAB_HISTORY }
};

constexpr int SAGE_DELIVERY_TAB_COUNT = sizeof(g_tabs) / sizeof(g_tabs[0]);

}

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

int SageDeliveryWorkflowHandler::GetTabCount() const {
	return SAGE_DELIVERY_TAB_COUNT;
}

const SageWorkflowTab& SageDeliveryWorkflowHandler::GetTab(int nVisualTabIndex) const {
	return g_tabs[nVisualTabIndex];
}
