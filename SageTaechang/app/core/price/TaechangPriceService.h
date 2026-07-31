#pragma once

#include "pch.h"
#include "app/core/price/TaechangPriceDto.h"
#include "app/infra/db/TaechangPriceRepository.h"

class TaechangPriceService {
public:
    TaechangPriceService(TaechangPriceRepository* pRepository);
    ~TaechangPriceService();

public:
    BOOL AddPrice(const TaechangPriceDto& dto, int& nNewPriceId, CString& strError);

    BOOL LoadByCompany(
        const CString& strCompanyName,
        CArray<TaechangPriceDto, TaechangPriceDto&>& arrPrice,
        CString& strError
    );

    BOOL LoadByCompanyAndCopies(
        const CString& strCompanyName,
        int nCopies,
        TaechangPriceDto& dto,
        BOOL& bFound,
        CString& strError
    );

    BOOL LoadByPrice(
        int nPrice,
        CArray<TaechangPriceDto, TaechangPriceDto&>& arrPrice,
        CString& strError
    );

    BOOL ChangePriceByCompany(
        const CString& strCompanyName,
        int nPrintPrice,
        int nCoverPrice,
        int& nAffectedCount,
        CString& strError
    );

    BOOL ChangePriceByCompanyAndCopies(
        const CString& strCompanyName,
        int nCopies,
        int nPrintPrice,
        int nCoverPrice,
        int& nAffectedCount,
        CString& strError
    );

    BOOL RenameCompany(
        const CString& strOldCompanyName,
        const CString& strNewCompanyName,
        int& nAffectedCount,
        CString& strError
    );

    BOOL ChangeCoverPriceByCompany(
        const CString& strCompanyName,
        int nCoverPrice,
        int& nAffectedCount,
        CString& strError
    );

    BOOL RemovePrice(int nPriceId, CString& strError);

    BOOL RemoveCompany(
        const CString& strCompanyName,
        int& nAffectedCount,
        CString& strError
    );

    BOOL LoadAllCompanyNames(CStringArray& arrNames, CString& strError);

    BOOL ModifyPriceById(const TaechangPriceDto& dto, CString& strError);

private:
    BOOL ValidateForInsert(const TaechangPriceDto& dto, CString& strError);
    BOOL ValidateCompanyName(const CString& strCompanyName, CString& strError);
    BOOL ValidateCopies(int nCopies, CString& strError);
    BOOL ValidatePriceValue(int nPrintPrice, int nCoverPrice, CString& strError);

private:
    TaechangPriceRepository* m_pRepository;
};
