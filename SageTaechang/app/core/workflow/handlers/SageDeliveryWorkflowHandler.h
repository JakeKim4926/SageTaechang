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
};
