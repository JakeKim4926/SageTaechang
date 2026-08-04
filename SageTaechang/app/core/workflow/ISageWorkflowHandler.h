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
    virtual LPCWSTR GetDetailSectionLabel() const = 0;

    virtual int GetTabCount() const = 0;
    virtual const SageWorkflowTab& GetTab(int nVisualTabIndex) const = 0;

    virtual int GetResultColumnCount(int nTaskType) const = 0;
    virtual const SageWorkflowColumn& GetResultColumn(int nTaskType, int nColumnIndex) const = 0;
    virtual SageWorkflowResultStyle GetResultStyle(int nTaskType) const = 0;
};
