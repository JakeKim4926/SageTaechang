#pragma once

#include "app/core/workflow/ISageWorkflowHandler.h"

class SageHwpCompareWorkflowHandler : public ISageWorkflowHandler
{
public:
    virtual int GetWorkflowType() const;
};
