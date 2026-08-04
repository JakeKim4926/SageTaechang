#include "pch.h"
#include "app/core/workflow/handlers/SageHwpCompareWorkflowHandler.h"
#include "TaechangDefine.h"

namespace {

const SageWorkflowTab g_tabs[] = {
	{ TAECHANG_TAB_INDEX_INPUT, TAECHANG_UI_TAB_FILES },
	{ TAECHANG_TAB_INDEX_PREVIEW, TAECHANG_UI_TAB_INSPECTION },
	{ TAECHANG_TAB_INDEX_RESULT, TAECHANG_UI_TAB_DETAIL },
	{ TAECHANG_TAB_INDEX_DETAIL, TAECHANG_UI_TAB_EXPORT }
};

constexpr int SAGE_HWP_COMPARE_TAB_COUNT = sizeof(g_tabs) / sizeof(g_tabs[0]);

}

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

int SageHwpCompareWorkflowHandler::GetTabCount() const {
	return SAGE_HWP_COMPARE_TAB_COUNT;
}

const SageWorkflowTab& SageHwpCompareWorkflowHandler::GetTab(int nVisualTabIndex) const {
	return g_tabs[nVisualTabIndex];
}
