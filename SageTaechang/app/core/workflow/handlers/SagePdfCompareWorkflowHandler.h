#pragma once

#include "app/core/workflow/ISageWorkflowHandler.h"

class SagePdfCompareWorkflowHandler : public ISageWorkflowHandler
{
public:
    virtual int GetWorkflowType() const;
};
