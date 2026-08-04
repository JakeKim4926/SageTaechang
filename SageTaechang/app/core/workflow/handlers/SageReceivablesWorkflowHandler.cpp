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

const SageWorkflowColumn g_resultColumns[] = {
	{ TAECHANG_UI_RECEIVABLES_COL_COMPANY, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RECEIVABLES_COMPANY_WIDTH, FALSE },
	{ TAECHANG_UI_RECEIVABLES_COL_MANAGER, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RECEIVABLES_MANAGER_WIDTH, FALSE },
	{ TAECHANG_UI_RECEIVABLES_COL_ISSUE_DATE, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RECEIVABLES_DATE_WIDTH, FALSE },
	{ TAECHANG_UI_RECEIVABLES_COL_ITEM, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RECEIVABLES_ITEM_WIDTH, FALSE },
	{ TAECHANG_UI_RECEIVABLES_COL_ISSUE_TYPE, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RECEIVABLES_TYPE_WIDTH, FALSE },
	{ TAECHANG_UI_RECEIVABLES_COL_TOTAL_AMOUNT, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_RECEIVABLES_AMOUNT_WIDTH, FALSE },
	{ TAECHANG_UI_RECEIVABLES_COL_DEPOSIT_AMOUNT, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_RECEIVABLES_AMOUNT_WIDTH, FALSE },
	{ TAECHANG_UI_RECEIVABLES_COL_RECEIVABLE_AMOUNT, SAGE_COLUMN_ALIGN_RIGHT, TAECHANG_RECEIVABLES_AMOUNT_WIDTH, FALSE },
	{ TAECHANG_UI_RECEIVABLES_COL_BANK, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RECEIVABLES_BANK_WIDTH, FALSE },
	{ TAECHANG_UI_RECEIVABLES_COL_NOTE, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RECEIVABLES_NOTE_WIDTH, FALSE }
};

constexpr int SAGE_RECEIVABLES_RESULT_COLUMN_COUNT = sizeof(g_resultColumns) / sizeof(g_resultColumns[0]);

BOOL HasReceivablesResultTable(int nTaskType) {
	if (nTaskType == TAECHANG_TASK_LOAD)
		return TRUE;
	return (nTaskType == TAECHANG_TASK_GENERATE) ? TRUE : FALSE;
}

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

int SageReceivablesWorkflowHandler::GetResultColumnCount(int nTaskType) const {
	if (!HasReceivablesResultTable(nTaskType))
		return SageWorkflowResultTable::GetGenericColumnCount();
	return SAGE_RECEIVABLES_RESULT_COLUMN_COUNT;
}

const SageWorkflowColumn& SageReceivablesWorkflowHandler::GetResultColumn(int nTaskType, int nColumnIndex) const {
	if (!HasReceivablesResultTable(nTaskType))
		return SageWorkflowResultTable::GetGenericColumn(nColumnIndex);
	return g_resultColumns[nColumnIndex];
}

SageWorkflowResultStyle SageReceivablesWorkflowHandler::GetResultStyle(int nTaskType) const {
	SageWorkflowResultStyle style;
	if (!HasReceivablesResultTable(nTaskType))
		return style;
	style.bGridLines = TRUE;
	style.nHighlightStart = TAECHANG_RECEIVABLES_COL_IDX_TOTAL_AMOUNT;
	style.nHighlightCount = TAECHANG_RECEIVABLES_COL_IDX_RECEIVABLE_AMOUNT - TAECHANG_RECEIVABLES_COL_IDX_TOTAL_AMOUNT + 1;
	return style;
}
