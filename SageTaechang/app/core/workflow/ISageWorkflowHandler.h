#pragma once

class ISageWorkflowHandler
{
public:
    virtual ~ISageWorkflowHandler() {}

    virtual int GetWorkflowType() const = 0;

    virtual LPCWSTR GetHeaderTitle() const = 0;
    virtual LPCWSTR GetInputSectionLabel() const = 0;
    virtual LPCWSTR GetActionButtonLabel() const = 0;
    virtual LPCWSTR GetDetailSectionLabel() const = 0;
};
