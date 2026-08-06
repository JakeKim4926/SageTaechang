#include "pch.h"
#include "app/core/workflow/handlers/SageDeliveryWorkflowHandler.h"
#include "app/core/workflow/TaechangWorkflowResultPresenter.h"
#include "TaechangDefine.h"

namespace {

const SageWorkflowTab g_tabs[] = {
	{ TAECHANG_TAB_INDEX_INPUT, TAECHANG_UI_TAB_INPUT },
	{ TAECHANG_TAB_INDEX_DOCUMENT_HISTORY, TAECHANG_UI_TAB_HISTORY }
};

constexpr int SAGE_DELIVERY_TAB_COUNT = sizeof(g_tabs) / sizeof(g_tabs[0]);

const SageWorkflowColumn g_inputColumns[] = {
	{ TAECHANG_UI_DELIVERY_COL_ROW, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_ROW_WIDTH, FALSE, SAGE_RESULT_FIELD_FIELD },
	{ TAECHANG_UI_DELIVERY_COL_COMPANY, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_COMPANY_WIDTH, FALSE, SAGE_RESULT_FIELD_COMPANY_NAME },
	{ TAECHANG_UI_DELIVERY_COL_DEPARTMENT, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_DEPARTMENT_WIDTH, FALSE, SAGE_RESULT_FIELD_DEPARTMENT },
	{ TAECHANG_UI_DELIVERY_COL_ORDER_DATE, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_DATE_WIDTH, FALSE, SAGE_RESULT_FIELD_ORDER_DATE },
	{ TAECHANG_UI_DELIVERY_COL_DELIVERY_DATE, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_DATE_WIDTH, FALSE, SAGE_RESULT_FIELD_DELIVERY_DATE },
	{ TAECHANG_UI_DELIVERY_COL_DELIVERY_TIME, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_TIME_WIDTH, FALSE, SAGE_RESULT_FIELD_DELIVERY_TIME },
	{ TAECHANG_UI_DELIVERY_COL_ITEM, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_ITEM_WIDTH, FALSE, SAGE_RESULT_FIELD_ITEM_NAME },
	{ TAECHANG_UI_DELIVERY_COL_PRODUCT_TYPE, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_TYPE_WIDTH, FALSE, SAGE_RESULT_FIELD_PRODUCT_TYPE },
	{ TAECHANG_UI_DELIVERY_COL_COMPANY_COPIES, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_DELIVERY_COPIES_WIDTH, FALSE, SAGE_RESULT_FIELD_COMPANY_COPIES },
	{ TAECHANG_UI_DELIVERY_COL_CORPORATION_COPIES, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_DELIVERY_COPIES_WIDTH, FALSE, SAGE_RESULT_FIELD_CORPORATION_COPIES },
	{ TAECHANG_UI_DELIVERY_COL_TOTAL_COPIES, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_DELIVERY_COPIES_WIDTH, FALSE, SAGE_RESULT_FIELD_TOTAL_COPIES }
};

constexpr int SAGE_DELIVERY_INPUT_COLUMN_COUNT = sizeof(g_inputColumns) / sizeof(g_inputColumns[0]);

const SageWorkflowFilterCriteria g_filterCriteria[] = {
	{ TAECHANG_FILTER_CRITERIA_ITEM, TAECHANG_UI_FILTER_CRITERIA_ITEM, SAGE_RESULT_FIELD_ITEM_NAME },
	{ TAECHANG_FILTER_CRITERIA_COMPANY, TAECHANG_UI_FILTER_CRITERIA_COMPANY, SAGE_RESULT_FIELD_COMPANY_NAME }
};

constexpr int SAGE_DELIVERY_FILTER_CRITERIA_COUNT = sizeof(g_filterCriteria) / sizeof(g_filterCriteria[0]);

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

int SageDeliveryWorkflowHandler::GetResultColumnCount(int nTaskType) const {
	if (!UsesCustomResultTable(nTaskType))
		return SageWorkflowResultTable::GetGenericColumnCount();
	return SAGE_DELIVERY_INPUT_COLUMN_COUNT;
}

const SageWorkflowColumn& SageDeliveryWorkflowHandler::GetResultColumn(int nTaskType, int nColumnIndex) const {
	if (!UsesCustomResultTable(nTaskType))
		return SageWorkflowResultTable::GetGenericColumn(nColumnIndex);
	return g_inputColumns[nColumnIndex];
}

SageWorkflowResultStyle SageDeliveryWorkflowHandler::GetResultStyle(int nTaskType) const {
	SageWorkflowResultStyle style;
	if (!UsesCustomResultTable(nTaskType))
		return style;
	style.bCheckbox = TRUE;
	style.bGridLines = TRUE;
	return style;
}

BOOL SageDeliveryWorkflowHandler::UsesCustomResultTable(int nTaskType) const {
	return (nTaskType == TAECHANG_TASK_LOAD) ? TRUE : FALSE;
}

BOOL SageDeliveryWorkflowHandler::BuildResultSummary(
	int nTaskType,
	const std::vector<TaechangResultRow>& arrVisibleRows,
	const CString& strResponseJson,
	std::vector<SageResultSummaryItem>& outItems) const {
	UNREFERENCED_PARAMETER(nTaskType);
	UNREFERENCED_PARAMETER(arrVisibleRows);
	UNREFERENCED_PARAMETER(strResponseJson);
	outItems.clear();
	return FALSE;
}

BOOL SageDeliveryWorkflowHandler::BuildResultTotals(
	int nTaskType,
	const std::vector<TaechangResultRow>& arrVisibleRows,
	std::vector<SageResultTotalCell>& outCells) const {
	UNREFERENCED_PARAMETER(nTaskType);
	UNREFERENCED_PARAMETER(arrVisibleRows);
	outCells.clear();
	return FALSE;
}

int SageDeliveryWorkflowHandler::GetFilterCriteriaCount() const {
	return SAGE_DELIVERY_FILTER_CRITERIA_COUNT;
}

const SageWorkflowFilterCriteria& SageDeliveryWorkflowHandler::GetFilterCriteria(int nIndex) const {
	return g_filterCriteria[nIndex];
}

LPCWSTR SageDeliveryWorkflowHandler::GetInputDialogTitle() const {
	return TAECHANG_UI_SELECT_DELIVERY_INPUT_TITLE;
}

BOOL SageDeliveryWorkflowHandler::UsesInputTable() const {
	return TRUE;
}

BOOL SageDeliveryWorkflowHandler::UsesOnePageOption() const {
	return FALSE;
}

LPCWSTR SageDeliveryWorkflowHandler::FindGenerateCompletedMessage() const {
	return TAECHANG_UI_DELIVERY_GENERATE_COMPLETED;
}

BOOL SageDeliveryWorkflowHandler::ValidateSelectedRows(int nSelectedCount, BOOL bHasSelectedRowNums, BOOL bOnePage, CString& strError) const {
	UNREFERENCED_PARAMETER(nSelectedCount);
	UNREFERENCED_PARAMETER(bOnePage);
	if (!bHasSelectedRowNums) {
		strError = TAECHANG_UI_DELIVERY_SELECT_ROW_REQUIRED;
		return FALSE;
	}
	return TRUE;
}
