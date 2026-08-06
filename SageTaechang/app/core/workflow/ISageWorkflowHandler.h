#pragma once

#include "app/core/workflow/SageWorkflowTab.h"
#include "app/core/workflow/SageWorkflowResultTable.h"

class ISageWorkflowHandler
{
public:
    virtual ~ISageWorkflowHandler() {}

    virtual int GetWorkflowType() const = 0;

    virtual LPCWSTR GetHeaderTitle() const = 0;
    virtual LPCWSTR GetInputSectionLabel() const = 0;
    virtual LPCWSTR GetActionButtonLabel() const = 0;
    virtual CString BuildActionButtonLabel(int nSelectedCount) const = 0;
    virtual LPCWSTR GetDetailSectionLabel() const = 0;

    virtual int GetTabCount() const = 0;
    virtual const SageWorkflowTab& GetTab(int nVisualTabIndex) const = 0;

    virtual int GetResultColumnCount(int nTaskType) const = 0;
    virtual const SageWorkflowColumn& GetResultColumn(int nTaskType, int nColumnIndex) const = 0;
    virtual SageWorkflowResultStyle GetResultStyle(int nTaskType) const = 0;
    virtual BOOL UsesCustomResultTable(int nTaskType) const = 0;
    virtual BOOL BuildResultSummary(
        int nTaskType,
        const std::vector<TaechangResultRow>& arrVisibleRows,
        const CString& strResponseJson,
        std::vector<SageResultSummaryItem>& outItems) const = 0;
    virtual BOOL BuildResultTotals(
        int nTaskType,
        const std::vector<TaechangResultRow>& arrVisibleRows,
        std::vector<SageResultTotalCell>& outCells) const = 0;

    virtual int GetFilterCriteriaCount() const = 0;
    virtual const SageWorkflowFilterCriteria& GetFilterCriteria(int nIndex) const = 0;

    virtual LPCWSTR GetInputDialogTitle() const = 0;
    virtual BOOL UsesInputTable() const = 0;
    virtual BOOL UsesOnePageOption() const = 0;
    virtual LPCWSTR FindGenerateCompletedMessage() const = 0;

    virtual BOOL ValidateSelectedRows(int nSelectedCount, BOOL bHasSelectedRowNums, BOOL bOnePage, CString& strError) const = 0;
};
