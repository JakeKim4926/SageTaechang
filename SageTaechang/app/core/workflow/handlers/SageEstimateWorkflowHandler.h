#pragma once

#include "app/core/workflow/ISageWorkflowHandler.h"

class SageEstimateWorkflowHandler : public ISageWorkflowHandler
{
public:
    virtual int GetWorkflowType() const;
};
