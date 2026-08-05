#include "pch.h"
#include "app/core/workflow/handlers/SageEstimateWorkflowHandler.h"
#include "TaechangDefine.h"

namespace {

const SageWorkflowTab g_tabs[] = {
	{ TAECHANG_TAB_INDEX_INPUT, TAECHANG_UI_TAB_INPUT },
	{ TAECHANG_TAB_INDEX_DOCUMENT_HISTORY, TAECHANG_UI_TAB_HISTORY }
};

constexpr int SAGE_ESTIMATE_TAB_COUNT = sizeof(g_tabs) / sizeof(g_tabs[0]);

const SageWorkflowColumn g_inputColumns[] = {
	{ TAECHANG_UI_ESTIMATE_COL_ROW, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_ESTIMATE_ROW_WIDTH, FALSE },
	{ TAECHANG_UI_ESTIMATE_COL_COMPANY, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_ESTIMATE_COMPANY_WIDTH, FALSE },
	{ TAECHANG_UI_ESTIMATE_COL_DATE, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_ESTIMATE_DATE_WIDTH, FALSE },
	{ TAECHANG_UI_ESTIMATE_COL_ITEM, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_ESTIMATE_ITEM_WIDTH, FALSE },
	{ TAECHANG_UI_ESTIMATE_COL_COPIES, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_ESTIMATE_COPIES_WIDTH, FALSE },
	{ TAECHANG_UI_ESTIMATE_COL_PAGES, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_ESTIMATE_PAGES_WIDTH, FALSE },
	{ TAECHANG_UI_ESTIMATE_COL_UNIT_PRICE, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_ESTIMATE_UNIT_PRICE_WIDTH, FALSE },
	{ TAECHANG_UI_ESTIMATE_COL_COVER, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_ESTIMATE_COVER_WIDTH, FALSE },
	{ TAECHANG_UI_ESTIMATE_COL_FREIGHT, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_ESTIMATE_FREIGHT_WIDTH, FALSE }
};

constexpr int SAGE_ESTIMATE_INPUT_COLUMN_COUNT = sizeof(g_inputColumns) / sizeof(g_inputColumns[0]);

const SageWorkflowFilterCriteria g_filterCriteria[] = {
	{ TAECHANG_FILTER_CRITERIA_ITEM, TAECHANG_UI_FILTER_CRITERIA_ITEM },
	{ TAECHANG_FILTER_CRITERIA_COMPANY, TAECHANG_UI_FILTER_CRITERIA_COMPANY }
};

constexpr int SAGE_ESTIMATE_FILTER_CRITERIA_COUNT = sizeof(g_filterCriteria) / sizeof(g_filterCriteria[0]);

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

int SageEstimateWorkflowHandler::GetResultColumnCount(int nTaskType) const {
	if (!UsesCustomResultTable(nTaskType))
		return SageWorkflowResultTable::GetGenericColumnCount();
	return SAGE_ESTIMATE_INPUT_COLUMN_COUNT;
}

const SageWorkflowColumn& SageEstimateWorkflowHandler::GetResultColumn(int nTaskType, int nColumnIndex) const {
	if (!UsesCustomResultTable(nTaskType))
		return SageWorkflowResultTable::GetGenericColumn(nColumnIndex);
	return g_inputColumns[nColumnIndex];
}

SageWorkflowResultStyle SageEstimateWorkflowHandler::GetResultStyle(int nTaskType) const {
	SageWorkflowResultStyle style;
	if (!UsesCustomResultTable(nTaskType))
		return style;
	style.bCheckbox = TRUE;
	style.bGridLines = TRUE;
	return style;
}

BOOL SageEstimateWorkflowHandler::UsesCustomResultTable(int nTaskType) const {
	return (nTaskType == TAECHANG_TASK_LOAD) ? TRUE : FALSE;
}

int SageEstimateWorkflowHandler::GetFilterCriteriaCount() const {
	return SAGE_ESTIMATE_FILTER_CRITERIA_COUNT;
}

const SageWorkflowFilterCriteria& SageEstimateWorkflowHandler::GetFilterCriteria(int nIndex) const {
	return g_filterCriteria[nIndex];
}

LPCWSTR SageEstimateWorkflowHandler::GetInputDialogTitle() const {
	return TAECHANG_UI_SELECT_ESTIMATE_INPUT_TITLE;
}

BOOL SageEstimateWorkflowHandler::UsesInputTable() const {
	return TRUE;
}

BOOL SageEstimateWorkflowHandler::ValidateSelectedRows(int nSelectedCount, BOOL bHasSelectedRowNums, BOOL bOnePage, CString& strError) const {
	if (!bHasSelectedRowNums) {
		strError = TAECHANG_UI_ESTIMATE_SELECT_ROW_REQUIRED;
		return FALSE;
	}
	if (bOnePage && nSelectedCount > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS) {
		strError = TAECHANG_UI_ESTIMATE_ONE_PAGE_LIMIT;
		return FALSE;
	}
	return TRUE;
}
