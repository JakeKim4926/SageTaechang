#pragma once

#include "app/core/workflow/ISageWorkflowHandler.h"

class SageReceivablesWorkflowHandler : public ISageWorkflowHandler
{
public:
    virtual int GetWorkflowType() const;
};
