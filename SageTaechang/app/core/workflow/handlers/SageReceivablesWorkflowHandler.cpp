#include "pch.h"
#include "app/core/workflow/handlers/SageReceivablesWorkflowHandler.h"
#include "TaechangDefine.h"

namespace {

const SageWorkflowTab g_tabs[] = {
	{ TAECHANG_TAB_INDEX_INPUT, TAECHANG_UI_TAB_INPUT },
	{ TAECHANG_TAB_INDEX_DOCUMENT_RESULT, TAECHANG_UI_TAB_RESULT },
	{ TAECHANG_TAB_INDEX_DOCUMENT_HISTORY, TAECHANG_UI_TAB_HISTORY },
	{ TAECHANG_TAB_INDEX_DOCUMENT_DATA_MANAGE, TAECHANG_UI_TAB_DATA_MANAGE }
};

constexpr int SAGE_RECEIVABLES_TAB_COUNT = sizeof(g_tabs) / sizeof(g_tabs[0]);

}

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

int SageReceivablesWorkflowHandler::GetTabCount() const {
	return SAGE_RECEIVABLES_TAB_COUNT;
}

const SageWorkflowTab& SageReceivablesWorkflowHandler::GetTab(int nVisualTabIndex) const {
	return g_tabs[nVisualTabIndex];
}
