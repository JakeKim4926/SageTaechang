#pragma once

class SageDeliveryExcelService
{
public:
    CString BuildOpenInputDialogResponse(
        const CString& strRequestId,
        const CString& strPayloadJson);

    CString BuildLoadInputDataResponse(
        const CString& strRequestId,
        const CString& strPayloadJson);

    CString BuildSelectOutputFolderResponse(
        const CString& strRequestId,
        const CString& strPayloadJson);

    CString BuildGenerateResponse(
        const CString& strRequestId,
        const CString& strPayloadJson);
};
