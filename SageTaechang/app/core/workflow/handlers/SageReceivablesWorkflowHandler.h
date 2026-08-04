#pragma once

#include "app/core/workflow/ISageWorkflowHandler.h"

class SageReceivablesWorkflowHandler : public ISageWorkflowHandler
{
public:
    virtual int GetWorkflowType() const;

    virtual LPCWSTR GetHeaderTitle() const;
    virtual LPCWSTR GetInputSectionLabel() const;
    virtual LPCWSTR GetActionButtonLabel() const;
    virtual LPCWSTR GetDetailSectionLabel() const;

    virtual int GetTabCount() const;
    virtual const SageWorkflowTab& GetTab(int nVisualTabIndex) const;
};
