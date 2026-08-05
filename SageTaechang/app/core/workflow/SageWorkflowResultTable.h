#pragma once

#include "TaechangDefine.h"

struct TaechangResultRow;

enum SageColumnAlign
{
    SAGE_COLUMN_ALIGN_LEFT,
    SAGE_COLUMN_ALIGN_RIGHT
};

enum SageResultField
{
    SAGE_RESULT_FIELD_FIELD,
    SAGE_RESULT_FIELD_VALUE,
    SAGE_RESULT_FIELD_STATUS,
    SAGE_RESULT_FIELD_REASON,
    SAGE_RESULT_FIELD_COMPANY_NAME,
    SAGE_RESULT_FIELD_DEPARTMENT,
    SAGE_RESULT_FIELD_ORDER_DATE,
    SAGE_RESULT_FIELD_DELIVERY_DATE,
    SAGE_RESULT_FIELD_DELIVERY_TIME,
    SAGE_RESULT_FIELD_MANAGER,
    SAGE_RESULT_FIELD_ISSUE_DATE,
    SAGE_RESULT_FIELD_ITEM_NAME,
    SAGE_RESULT_FIELD_PRODUCT_TYPE,
    SAGE_RESULT_FIELD_COMPANY_COPIES,
    SAGE_RESULT_FIELD_CORPORATION_COPIES,
    SAGE_RESULT_FIELD_TOTAL_COPIES,
    SAGE_RESULT_FIELD_ISSUE_TYPE,
    SAGE_RESULT_FIELD_TOTAL_AMOUNT,
    SAGE_RESULT_FIELD_DEPOSIT_AMOUNT,
    SAGE_RESULT_FIELD_RECEIVABLE_AMOUNT,
    SAGE_RESULT_FIELD_BANK_NAME,
    SAGE_RESULT_FIELD_NOTE
};

struct SageWorkflowColumn
{
    LPCWSTR pszLabel;
    SageColumnAlign nAlign;
    int nWidth;
    BOOL bStretch;
    SageResultField nField;
};

struct SageWorkflowFilterCriteria
{
    int nCriteria;
    LPCWSTR pszLabel;
};

struct SageWorkflowResultStyle
{
    BOOL bCheckbox;
    BOOL bGridLines;
    int nHighlightStart;
    int nHighlightCount;
    int nGroupColumn;

    SageWorkflowResultStyle() {
        bCheckbox = FALSE;
        bGridLines = FALSE;
        nHighlightStart = 0;
        nHighlightCount = 0;
        nGroupColumn = TAECHANG_LIST_NO_GROUP_COLUMN;
    }
};

namespace SageWorkflowResultTable {

int GetGenericColumnCount();
const SageWorkflowColumn& GetGenericColumn(int nColumnIndex);
CString GetRowText(const TaechangResultRow& row, SageResultField nField);

}
