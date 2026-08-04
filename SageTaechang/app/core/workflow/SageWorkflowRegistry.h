#pragma once

#include "app/core/workflow/ISageWorkflowHandler.h"

namespace SageWorkflowRegistry {
    ISageWorkflowHandler* FindHandler(int nWorkflowType);
}
