#pragma once

class TaechangCompareCsvExportService
{
public:
    BOOL ExportCompareResult(
        const CString& strResponseJson,
        const CString& strOutputPath,
        CString& strError);
};
