#include "pch.h"
#include "app/core/workflow/SageWorkflowResultTable.h"
#include "TaechangDefine.h"

namespace {

const SageWorkflowColumn g_genericColumns[] = {
	{ TAECHANG_UI_RESULT_FIELD, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RESULT_FIELD_WIDTH, FALSE },
	{ TAECHANG_UI_RESULT_VALUE, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RESULT_MIN_VALUE_WIDTH, TRUE },
	{ TAECHANG_UI_RESULT_STATUS, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RESULT_STATUS_WIDTH, FALSE },
	{ TAECHANG_UI_RESULT_REASON, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RESULT_REASON_WIDTH, FALSE }
};

constexpr int SAGE_GENERIC_COLUMN_COUNT = sizeof(g_genericColumns) / sizeof(g_genericColumns[0]);

}

namespace SageWorkflowResultTable {

int GetGenericColumnCount() {
	return SAGE_GENERIC_COLUMN_COUNT;
}

const SageWorkflowColumn& GetGenericColumn(int nColumnIndex) {
	return g_genericColumns[nColumnIndex];
}

}
