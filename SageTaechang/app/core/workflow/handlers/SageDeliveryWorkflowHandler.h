#pragma once

#include "app/core/workflow/ISageWorkflowHandler.h"

class SageDeliveryWorkflowHandler : public ISageWorkflowHandler
{
public:
    virtual int GetWorkflowType() const;
};
