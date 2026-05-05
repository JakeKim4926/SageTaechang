#pragma once

#include "pch.h"
#include "SqlContext.h"
#include "TaechangPriceDto.h"

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

private:
    BOOL FillDto(sqlite3_stmt* pStatement, TaechangPriceDto& dto);

private:
    SqlContext* m_pSqlContext;
};