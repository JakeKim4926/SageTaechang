#include "pch.h"
#include "app/core/workflow/handlers/SageEstimateWorkflowHandler.h"
#include "TaechangDefine.h"

namespace {

const SageWorkflowTab g_tabs[] = {
	{ TAECHANG_TAB_INDEX_INPUT, TAECHANG_UI_TAB_INPUT },
	{ TAECHANG_TAB_INDEX_DOCUMENT_HISTORY, TAECHANG_UI_TAB_HISTORY }
};

constexpr int SAGE_ESTIMATE_TAB_COUNT = sizeof(g_tabs) / sizeof(g_tabs[0]);

}

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

int SageEstimateWorkflowHandler::GetTabCount() const {
	return SAGE_ESTIMATE_TAB_COUNT;
}

const SageWorkflowTab& SageEstimateWorkflowHandler::GetTab(int nVisualTabIndex) const {
	return g_tabs[nVisualTabIndex];
}
