#pragma once

#include "SageDefine.h"

struct SageResultRow;

enum SageColumnAlign
{
    SAGE_COLUMN_ALIGN_LEFT,
    SAGE_COLUMN_ALIGN_RIGHT,
    SAGE_COLUMN_ALIGN_CENTER
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

struct SageColumnWidthSpec
{
    SageColumnWidthSpec() {
        nWidth = 0;
        bStretch = FALSE;
    }

    SageColumnWidthSpec(int nColumnWidth, BOOL bStretchColumn) {
        nWidth = nColumnWidth;
        bStretch = bStretchColumn;
    }

    int nWidth;
    BOOL bStretch;
};

struct SageWorkflowFilterCriteria
{
    int nCriteria;
    LPCWSTR pszLabel;
    SageResultField nField;
};

struct SageResultSummaryItem
{
    SageResultSummaryItem() {
        bHighlight = FALSE;
        bBadge = FALSE;
    }

    CString strLabel;
    CString strValue;
    CString strUnit;
    BOOL bHighlight;
    BOOL bBadge;
};

enum SageResultTotalRole
{
    SAGE_RESULT_TOTAL_LABEL,
    SAGE_RESULT_TOTAL_COUNT,
    SAGE_RESULT_TOTAL_AMOUNT,
    SAGE_RESULT_TOTAL_AMOUNT_HIGHLIGHT
};

struct SageResultTotalCell
{
    SageResultTotalCell() {
        nColumn = 0;
        nRole = SAGE_RESULT_TOTAL_LABEL;
    }

    int nColumn;
    CString strText;
    SageResultTotalRole nRole;
};

struct SageWorkflowResultStyle
{
    BOOL bCheckbox;
    BOOL bGridLines;
    int nHighlightStart;
    int nHighlightCount;

    SageWorkflowResultStyle() {
        bCheckbox = FALSE;
        bGridLines = FALSE;
        nHighlightStart = 0;
        nHighlightCount = 0;
    }
};

namespace SageWorkflowResultTable {

int GetGenericColumnCount();
const SageWorkflowColumn& GetGenericColumn(int nColumnIndex);
CString GetRowText(const SageResultRow& row, SageResultField nField);
CString FormatAmountNumber(__int64 nAmount);
void DistributeColumnWidths(
    const std::vector<SageColumnWidthSpec>& arrSpecs,
    int nTotalWidth,
    std::vector<int>& outWidths);

}
