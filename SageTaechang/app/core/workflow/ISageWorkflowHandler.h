#pragma once

class ISageWorkflowHandler
{
public:
    virtual ~ISageWorkflowHandler() {}

    virtual int GetWorkflowType() const = 0;
};
