#pragma once

class TaechangHwpCompareService
{
public:
    CString BuildOpenMultiFileDialogResponse(
        const CString& strRequestId,
        const CString& strPayloadJson);

    CString BuildRunCompareResponse(
        const CString& strRequestId,
        const CString& strPayloadJson);

    CString BuildExportCsvResponse(
        const CString& strRequestId,
        const CString& strPayloadJson);
};
