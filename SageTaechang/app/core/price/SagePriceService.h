#pragma once

#include "pch.h"
#include "app/core/price/SagePriceDto.h"
#include "app/infra/db/SagePriceRepository.h"

class SagePriceService {
public:
    SagePriceService(SagePriceRepository* pRepository);
    ~SagePriceService();

public:
    BOOL AddPrice(const SagePriceDto& dto, int& nNewPriceId, CString& strError);

    BOOL LoadByCompany(
        const CString& strCompanyName,
        CArray<SagePriceDto, SagePriceDto&>& arrPrice,
        CString& strError
    );

    BOOL LoadByCompanyAndCopies(
        const CString& strCompanyName,
        int nCopies,
        SagePriceDto& dto,
        BOOL& bFound,
        CString& strError
    );

    BOOL LoadByPrice(
        int nPrice,
        CArray<SagePriceDto, SagePriceDto&>& arrPrice,
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

    BOOL AddCompany(const CString& strCompanyName, int& nNewCompanyId, CString& strError);
    BOOL LoadAllCompanyNames(CStringArray& arrNames, CString& strError);

    BOOL ModifyPriceById(const SagePriceDto& dto, CString& strError);

private:
    BOOL ValidateForInsert(const SagePriceDto& dto, CString& strError);
    BOOL ValidateCompanyName(const CString& strCompanyName, CString& strError);
    BOOL ValidateCopies(int nCopies, CString& strError);
    BOOL ValidatePriceValue(int nPrintPrice, int nCoverPrice, CString& strError);

private:
    SagePriceRepository* m_pRepository;
};
