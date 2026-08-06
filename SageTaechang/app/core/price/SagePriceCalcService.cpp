#include "pch.h"
#include "app/core/price/SagePriceCalcService.h"
#include "app/core/price/TaechangPriceService.h"
#include "TaechangDefine.h"

SagePriceCalcService::SagePriceCalcService(TaechangPriceService* pPriceService) {
    m_pPriceService = pPriceService;
}

BOOL SagePriceCalcService::Calculate(
    const CString& strCompanyName,
    int nCopies,
    int nPages,
    int nFreight,
    SagePriceCalcResult& outResult,
    SagePriceCalcFailure& outFailure,
    CString& strError
) {
    TaechangPriceDto dto;
    BOOL bFound;

    outResult = SagePriceCalcResult();
    outFailure = SAGE_PRICE_CALC_NONE;
    bFound = FALSE;

    if (ValidateCopies(nCopies, outFailure) == FALSE) {
        return FALSE;
    }

    if (nPages < 1 || nPages > TAECHANG_PRICE_COPIES_MAX) {
        outFailure = SAGE_PRICE_CALC_PAGES_OUT_OF_RANGE;
        return FALSE;
    }

    if (m_pPriceService->LoadByCompanyAndCopies(strCompanyName, nCopies, dto, bFound, strError) == FALSE) {
        outFailure = SAGE_PRICE_CALC_LOAD_ERROR;
        return FALSE;
    }

    if (bFound == FALSE) {
        outFailure = SAGE_PRICE_CALC_NO_DATA;
        return FALSE;
    }

    outResult.nUnitPrice = dto.nPrintPrice;
    outResult.nCoverPrice = dto.nCoverPrice;
    outResult.nRangeMinCopies = dto.nMinCopies;
    outResult.nRangeMaxCopies = dto.nMaxCopies;
    outResult.bRangeHasMaxCopies = dto.bHasMaxCopies;
    outResult.nPrintPrice = static_cast<LONGLONG>(dto.nPrintPrice) * nPages;
    outResult.nSubtotal = outResult.nPrintPrice + dto.nCoverPrice;
    ApplyFreight(nFreight, outResult);
    return TRUE;
}

BOOL SagePriceCalcService::ValidateCopies(int nCopies, SagePriceCalcFailure& outFailure) const {
    outFailure = SAGE_PRICE_CALC_NONE;

    if (nCopies < 1) {
        outFailure = SAGE_PRICE_CALC_COPIES_BELOW_MIN;
        return FALSE;
    }

    if (nCopies > TAECHANG_PRICE_COPIES_MAX) {
        outFailure = SAGE_PRICE_CALC_COPIES_ABOVE_MAX;
        return FALSE;
    }

    return TRUE;
}

void SagePriceCalcService::ApplyFreight(int nFreight, SagePriceCalcResult& result) const {
    result.nFreight = ClampFreight(nFreight);
    result.nTotal = result.nSubtotal + result.nFreight;
}

int SagePriceCalcService::ClampFreight(int nFreight) const {
    if (nFreight < 0)
        return 0;
    if (nFreight > TAECHANG_PRICE_AMOUNT_MAX)
        return TAECHANG_PRICE_AMOUNT_MAX;
    return nFreight;
}
