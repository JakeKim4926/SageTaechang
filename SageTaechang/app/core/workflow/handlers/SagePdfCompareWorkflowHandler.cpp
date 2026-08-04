#include "pch.h"
#include "app/core/workflow/handlers/SagePdfCompareWorkflowHandler.h"
#include "TaechangDefine.h"

namespace {

const SageWorkflowTab g_tabs[] = {
	{ TAECHANG_TAB_INDEX_INPUT, TAECHANG_UI_TAB_FILES },
	{ TAECHANG_TAB_INDEX_PREVIEW, TAECHANG_UI_TAB_INSPECTION },
	{ TAECHANG_TAB_INDEX_RESULT, TAECHANG_UI_TAB_DETAIL },
	{ TAECHANG_TAB_INDEX_DETAIL, TAECHANG_UI_TAB_EXPORT }
};

constexpr int SAGE_PDF_COMPARE_TAB_COUNT = sizeof(g_tabs) / sizeof(g_tabs[0]);

}

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

int SagePdfCompareWorkflowHandler::GetTabCount() const {
	return SAGE_PDF_COMPARE_TAB_COUNT;
}

const SageWorkflowTab& SagePdfCompareWorkflowHandler::GetTab(int nVisualTabIndex) const {
	return g_tabs[nVisualTabIndex];
}
