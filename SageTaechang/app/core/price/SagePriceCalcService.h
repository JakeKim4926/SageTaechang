#pragma once

#include "pch.h"

class TaechangPriceService;

enum SagePriceCalcFailure {
    SAGE_PRICE_CALC_NONE,
    SAGE_PRICE_CALC_COPIES_BELOW_MIN,
    SAGE_PRICE_CALC_COPIES_ABOVE_MAX,
    SAGE_PRICE_CALC_PAGES_OUT_OF_RANGE,
    SAGE_PRICE_CALC_NO_DATA,
    SAGE_PRICE_CALC_LOAD_ERROR
};

struct SagePriceCalcResult {
    int nUnitPrice;
    int nCoverPrice;
    int nFreight;
    int nRangeMinCopies;
    int nRangeMaxCopies;
    BOOL bRangeHasMaxCopies;
    LONGLONG nPrintPrice;
    LONGLONG nSubtotal;
    LONGLONG nTotal;

    SagePriceCalcResult()
        : nUnitPrice(0)
        , nCoverPrice(0)
        , nFreight(0)
        , nRangeMinCopies(0)
        , nRangeMaxCopies(0)
        , bRangeHasMaxCopies(FALSE)
        , nPrintPrice(0)
        , nSubtotal(0)
        , nTotal(0) {}
};

class SagePriceCalcService {
public:
    SagePriceCalcService(TaechangPriceService* pPriceService);

public:
    BOOL Calculate(
        const CString& strCompanyName,
        int nCopies,
        int nPages,
        int nFreight,
        SagePriceCalcResult& outResult,
        SagePriceCalcFailure& outFailure,
        CString& strError
    );

    BOOL ValidateCopies(int nCopies, SagePriceCalcFailure& outFailure) const;

    void ApplyFreight(int nFreight, SagePriceCalcResult& result) const;

private:
    int ClampFreight(int nFreight) const;

private:
    TaechangPriceService* m_pPriceService;
};
