#pragma once

#include "pch.h"
#include "app/infra/db/SqlContext.h"
#include "app/core/price/TaechangPriceDto.h"

class TaechangPriceRepository {
public:
    TaechangPriceRepository(SqlContext* pSqlContext);
    ~TaechangPriceRepository();

public:
    BOOL Insert(const TaechangPriceDto& dto, int& nNewPriceId, CString& strError);

    BOOL SelectByCompany(
        const CString& strCompanyName,
        int nReportType,
        CArray<TaechangPriceDto, TaechangPriceDto&>& arrPrice,
        CString& strError
    );

    BOOL SelectByCompanyAndCopies(
        const CString& strCompanyName,
        int nReportType,
        int nCopies,
        TaechangPriceDto& dto,
        BOOL& bFound,
        CString& strError
    );

    BOOL SelectByPrice(
        int nPrice,
        int nReportType,
        CArray<TaechangPriceDto, TaechangPriceDto&>& arrPrice,
        CString& strError
    );

    BOOL UpdatePriceByCompany(
        const CString& strCompanyName,
        int nReportType,
        int nPrintPrice,
        int nCoverPrice,
        int& nAffectedCount,
        CString& strError
    );

    BOOL UpdatePriceByCompanyAndCopies(
        const CString& strCompanyName,
        int nReportType,
        int nCopies,
        int nPrintPrice,
        int nCoverPrice,
        int& nAffectedCount,
        CString& strError
    );

    BOOL UpdateCompanyName(
        const CString& strOldCompanyName,
        const CString& strNewCompanyName,
        int nReportType,
        int& nAffectedCount,
        CString& strError
    );

    BOOL UpdateCoverPriceByCompany(
        const CString& strCompanyName,
        int nReportType,
        int nCoverPrice,
        int& nAffectedCount,
        CString& strError
    );

    BOOL ExistsOverlap(
        const CString& strCompanyName,
        int nReportType,
        int nMinCopies,
        BOOL bHasMaxCopies,
        int nMaxCopies,
        int nExceptPriceId,
        BOOL& bExists,
        CString& strError
    );

    BOOL DeleteByPriceId(int nPriceId, CString& strError);

    BOOL DeleteByCompany(
        const CString& strCompanyName,
        int nReportType,
        int& nAffectedCount,
        CString& strError
    );

    BOOL InsertCompany(
        const CString& strCompanyName,
        int nReportType,
        int& nNewCompanyId,
        CString& strError
        );

    BOOL SelectAllCompanyNames(
        int nReportType,
        CStringArray& arrNames,
        CString& strError
    );

    BOOL UpdateByPriceId(
        int nPriceId,
        int nMinCopies,
        BOOL bHasMaxCopies,
        int nMaxCopies,
        int nPrintPrice,
        int nCoverPrice,
        CString& strError
    );

private:
    BOOL ExecuteCompanyStatement(
        const CStringA& strSqlA,
        const CString& strFirstText,
        const CString& strSecondText,
        int nReportType,
        CString& strError
        );

    BOOL FillDto(sqlite3_stmt* pStatement, TaechangPriceDto& dto);

private:
    SqlContext* m_pSqlContext;
};
