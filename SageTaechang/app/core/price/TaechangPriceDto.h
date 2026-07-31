#pragma once

#include "pch.h"

enum ReportType {
    REPORT_TYPE_NONE = 0,
    REPORT_TYPE_AUDIT_REPORT = 1,
    REPORT_TYPE_TAX_ADJUSTMENT = 2
};

struct TaechangPriceDto {
    int nPriceId;

    CString strCompanyName;
    int nReportType;

    int nMinCopies;
    int nMaxCopies;
    BOOL bHasMaxCopies;

    int nPrintPrice;
    int nCoverPrice;
    int nTotalPrice;

    CString strMemo;

    TaechangPriceDto() {
        nPriceId = 0;

        nReportType = REPORT_TYPE_AUDIT_REPORT;

        nMinCopies = 1;
        nMaxCopies = 0;
        bHasMaxCopies = FALSE;

        nPrintPrice = 0;
        nCoverPrice = 0;
        nTotalPrice = 0;
    }
};