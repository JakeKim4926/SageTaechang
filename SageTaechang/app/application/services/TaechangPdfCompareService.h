#pragma once

class TaechangPdfCompareService
{
public:
    CString BuildOpenFileDialogResponse(
        const CString& strRequestId,
        const CString& strPayloadJson);

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
