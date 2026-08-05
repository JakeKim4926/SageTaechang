#pragma once

#include "app/core/workflow/ISageWorkflowHandler.h"

class SageDeliveryWorkflowHandler : public ISageWorkflowHandler
{
public:
    virtual int GetWorkflowType() const;

    virtual LPCWSTR GetHeaderTitle() const;
    virtual LPCWSTR GetInputSectionLabel() const;
    virtual LPCWSTR GetActionButtonLabel() const;
    virtual LPCWSTR GetDetailSectionLabel() const;

    virtual int GetTabCount() const;
    virtual const SageWorkflowTab& GetTab(int nVisualTabIndex) const;

    virtual int GetResultColumnCount(int nTaskType) const;
    virtual const SageWorkflowColumn& GetResultColumn(int nTaskType, int nColumnIndex) const;
    virtual SageWorkflowResultStyle GetResultStyle(int nTaskType) const;
    virtual BOOL UsesCustomResultTable(int nTaskType) const;

    virtual int GetFilterCriteriaCount() const;
    virtual const SageWorkflowFilterCriteria& GetFilterCriteria(int nIndex) const;

    virtual LPCWSTR GetInputDialogTitle() const;
    virtual BOOL UsesInputTable() const;
    virtual LPCWSTR FindGenerateCompletedMessage() const;

    virtual BOOL ValidateSelectedRows(int nSelectedCount, BOOL bHasSelectedRowNums, BOOL bOnePage, CString& strError) const;
};
