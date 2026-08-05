#pragma once

enum SageColumnAlign
{
    SAGE_COLUMN_ALIGN_LEFT,
    SAGE_COLUMN_ALIGN_RIGHT
};

struct SageWorkflowColumn
{
    LPCWSTR pszLabel;
    SageColumnAlign nAlign;
    int nWidth;
    BOOL bStretch;
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

}
