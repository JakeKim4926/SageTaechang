#include "pch.h"
#include "app/core/workflow/handlers/SageDeliveryWorkflowHandler.h"
#include "TaechangDefine.h"

namespace {

const SageWorkflowTab g_tabs[] = {
	{ TAECHANG_TAB_INDEX_INPUT, TAECHANG_UI_TAB_INPUT },
	{ TAECHANG_TAB_INDEX_DOCUMENT_HISTORY, TAECHANG_UI_TAB_HISTORY }
};

constexpr int SAGE_DELIVERY_TAB_COUNT = sizeof(g_tabs) / sizeof(g_tabs[0]);

const SageWorkflowColumn g_inputColumns[] = {
	{ TAECHANG_UI_DELIVERY_COL_ROW, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_ROW_WIDTH, FALSE },
	{ TAECHANG_UI_DELIVERY_COL_COMPANY, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_COMPANY_WIDTH, FALSE },
	{ TAECHANG_UI_DELIVERY_COL_DEPARTMENT, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_DEPARTMENT_WIDTH, FALSE },
	{ TAECHANG_UI_DELIVERY_COL_ORDER_DATE, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_DATE_WIDTH, FALSE },
	{ TAECHANG_UI_DELIVERY_COL_DELIVERY_DATE, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_DATE_WIDTH, FALSE },
	{ TAECHANG_UI_DELIVERY_COL_DELIVERY_TIME, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_TIME_WIDTH, FALSE },
	{ TAECHANG_UI_DELIVERY_COL_ITEM, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_ITEM_WIDTH, FALSE },
	{ TAECHANG_UI_DELIVERY_COL_PRODUCT_TYPE, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_DELIVERY_TYPE_WIDTH, FALSE },
	{ TAECHANG_UI_DELIVERY_COL_COMPANY_COPIES, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_DELIVERY_COPIES_WIDTH, FALSE },
	{ TAECHANG_UI_DELIVERY_COL_CORPORATION_COPIES, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_DELIVERY_COPIES_WIDTH, FALSE },
	{ TAECHANG_UI_DELIVERY_COL_TOTAL_COPIES, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_DELIVERY_COPIES_WIDTH, FALSE }
};

constexpr int SAGE_DELIVERY_INPUT_COLUMN_COUNT = sizeof(g_inputColumns) / sizeof(g_inputColumns[0]);

BOOL HasDeliveryInputTable(int nTaskType) {
	return (nTaskType == TAECHANG_TASK_LOAD) ? TRUE : FALSE;
}

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
	if (!HasDeliveryInputTable(nTaskType))
		return SageWorkflowResultTable::GetGenericColumnCount();
	return SAGE_DELIVERY_INPUT_COLUMN_COUNT;
}

const SageWorkflowColumn& SageDeliveryWorkflowHandler::GetResultColumn(int nTaskType, int nColumnIndex) const {
	if (!HasDeliveryInputTable(nTaskType))
		return SageWorkflowResultTable::GetGenericColumn(nColumnIndex);
	return g_inputColumns[nColumnIndex];
}

SageWorkflowResultStyle SageDeliveryWorkflowHandler::GetResultStyle(int nTaskType) const {
	SageWorkflowResultStyle style;
	if (!HasDeliveryInputTable(nTaskType))
		return style;
	style.bCheckbox = TRUE;
	style.bGridLines = TRUE;
	return style;
}
