#include "pch.h"
#include "app/core/workflow/SageWorkflowRegistry.h"
#include "app/core/workflow/handlers/SageReceivablesWorkflowHandler.h"
#include "app/core/workflow/handlers/SageDeliveryWorkflowHandler.h"
#include "app/core/workflow/handlers/SageEstimateWorkflowHandler.h"
#include "app/core/workflow/handlers/SagePdfCompareWorkflowHandler.h"
#include "app/core/workflow/handlers/SageHwpCompareWorkflowHandler.h"

namespace {

SageReceivablesWorkflowHandler g_handlerReceivables;
SageDeliveryWorkflowHandler g_handlerDelivery;
SageEstimateWorkflowHandler g_handlerEstimate;
SagePdfCompareWorkflowHandler g_handlerPdfCompare;
SageHwpCompareWorkflowHandler g_handlerHwpCompare;

ISageWorkflowHandler* const g_handlers[] = {
	&g_handlerReceivables,
	&g_handlerDelivery,
	&g_handlerEstimate,
	&g_handlerPdfCompare,
	&g_handlerHwpCompare
};

constexpr int SAGE_WORKFLOW_HANDLER_COUNT = sizeof(g_handlers) / sizeof(g_handlers[0]);

}

namespace SageWorkflowRegistry {

ISageWorkflowHandler* FindHandler(int nWorkflowType) {
	for (int i = 0; i < SAGE_WORKFLOW_HANDLER_COUNT; ++i) {
		if (g_handlers[i]->GetWorkflowType() == nWorkflowType)
			return g_handlers[i];
	}
	return NULL;
}

}
