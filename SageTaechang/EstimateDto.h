#pragma once

#include "pch.h"

struct CompanyDto {
    int nCompanyId;
    CString strCompanyName;
    CString strBusinessNo;
    CString strMemo;

    CompanyDto() {
        nCompanyId = 0;
    }
};

struct ReportTypeDto {
    int nReportTypeId;
    CString strReportCode;
    CString strReportName;
    CString strMemo;

    ReportTypeDto() {
        nReportTypeId = 0;
    }
};

struct PriceBookDto {
    int nPriceBookId;
    int nCompanyId;
    int nReportTypeId;

    CString strPriceBookName;
    CString strEffectiveFrom;
    CString strEffectiveTo;
    BOOL bHasEffectiveTo;

    BOOL bActive;
    CString strMemo;

    CString strCompanyName;
    CString strReportCode;
    CString strReportName;

    PriceBookDto() {
        nPriceBookId = 0;
        nCompanyId = 0;
        nReportTypeId = 0;
        bHasEffectiveTo = FALSE;
        bActive = TRUE;
    }
};

struct CopyPriceTierDto {
    int nTierId;
    int nPriceBookId;

    int nMinCopies;
    int nMaxCopies;
    BOOL bHasMaxCopies;

    int nPriceAmount;
    CString strMemo;

    CopyPriceTierDto() {
        nTierId = 0;
        nPriceBookId = 0;
        nMinCopies = 1;
        nMaxCopies = 0;
        bHasMaxCopies = FALSE;
        nPriceAmount = 0;
    }
};

struct FixedPriceItemDto {
    int nFixedItemId;
    int nPriceBookId;

    CString strItemCode;
    CString strItemName;

    int nPriceAmount;
    CString strApplyType;
    CString strMemo;

    FixedPriceItemDto() {
        nFixedItemId = 0;
        nPriceBookId = 0;
        nPriceAmount = 0;
        strApplyType = _T("ONCE");
    }
};

struct EstimateResultDto {
    int nCompanyId;
    int nPriceBookId;
    int nCopies;

    CString strCompanyName;
    CString strReportCode;
    CString strReportName;

    int nCopyPrice;
    int nFixedPrice;
    int nTotalPrice;

    int nMinCopies;
    int nMaxCopies;
    BOOL bHasMaxCopies;

    EstimateResultDto() {
        nCompanyId = 0;
        nPriceBookId = 0;
        nCopies = 0;

        nCopyPrice = 0;
        nFixedPrice = 0;
        nTotalPrice = 0;

        nMinCopies = 0;
        nMaxCopies = 0;
        bHasMaxCopies = FALSE;
    }
};