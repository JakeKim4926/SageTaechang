#pragma once

CString BuildSuccessResponse(const CString& strRequestId, const CString& strPayloadJson);
CString BuildErrorResponse(
    const CString& strRequestId,
    const CString& strErrorCode,
    const CString& strErrorMessage);
