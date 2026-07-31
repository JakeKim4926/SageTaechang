#include "pch.h"
#include "app/common/TaechangJson.h"
#include "app/core/workflow/TaechangWorkflowResponse.h"

CString BuildSuccessResponse(const CString& strRequestId, const CString& strPayloadJson) {
	return L"{\"type\":\"response\",\"requestId\":\"" + JsonEscapeString(strRequestId) +
		L"\",\"success\":true,\"payload\":" +
		(strPayloadJson.IsEmpty() ? CString(L"{}") : strPayloadJson) +
		L",\"error\":null}";
}

CString BuildErrorResponse(
	const CString& strRequestId,
	const CString& strErrorCode,
	const CString& strErrorMessage) {
	return L"{\"type\":\"response\",\"requestId\":\"" + JsonEscapeString(strRequestId) +
		L"\",\"success\":false,\"payload\":null,\"error\":{\"code\":\"" + JsonEscapeString(strErrorCode) +
		L"\",\"message\":\"" + JsonEscapeString(strErrorMessage) + L"\"}}";
}
