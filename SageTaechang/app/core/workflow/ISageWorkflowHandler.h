#pragma once

#include "app/core/workflow/SageWorkflowTab.h"

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
};
