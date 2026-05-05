#include "pch.h"
#include "TaechangPriceRepository.h"
#include "RepositoryHelper.h"

TaechangPriceRepository::TaechangPriceRepository(SqlContext* pSqlContext) {
    m_pSqlContext = pSqlContext;
}

TaechangPriceRepository::~TaechangPriceRepository() {}

BOOL TaechangPriceRepository::Insert(const TaechangPriceDto& dto, int& nNewPriceId, CString& strError) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    nNewPriceId = 0;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "INSERT INTO TaechangPrice "
        "("
        "    company_name, "
        "    report_type, "
        "    min_copies, "
        "    max_copies, "
        "    print_price, "
        "    cover_price, "
        "    memo "
        ") "
        "VALUES "
        "("
        "    ?, ?, ?, ?, ?, ?, ? "
        ");";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, dto.strCompanyName, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, dto.nReportType, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 3, dto.nMinCopies, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    if (dto.bHasMaxCopies == TRUE) {
        if (RepositoryHelper::BindInt(pStatement, 4, dto.nMaxCopies, strError) == FALSE) {
            sqlite3_finalize(pStatement);
            return FALSE;
        }
    } else {
        if (RepositoryHelper::BindNull(pStatement, 4, strError) == FALSE) {
            sqlite3_finalize(pStatement);
            return FALSE;
        }
    }

    if (RepositoryHelper::BindInt(pStatement, 5, dto.nPrintPrice, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 6, dto.nCoverPrice, strError) == FALSE ||
        RepositoryHelper::BindText(pStatement, 7, dto.strMemo, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nNewPriceId = (int)sqlite3_last_insert_rowid(pDb);

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL TaechangPriceRepository::SelectByCompany(
    const CString& strCompanyName,
    int nReportType,
    CArray<TaechangPriceDto, TaechangPriceDto&>& arrPrice,
    CString& strError
) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    arrPrice.RemoveAll();

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "SELECT "
        "    price_id, "
        "    company_name, "
        "    report_type, "
        "    min_copies, "
        "    max_copies, "
        "    print_price, "
        "    cover_price, "
        "    memo "
        "FROM TaechangPrice "
        "WHERE company_name = ? "
        "  AND report_type = ? "
        "ORDER BY min_copies ASC;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, strCompanyName, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, nReportType, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    while ((nResult = sqlite3_step(pStatement)) == SQLITE_ROW) {
        TaechangPriceDto dto;

        FillDto(pStatement, dto);
        arrPrice.Add(dto);
    }

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL TaechangPriceRepository::SelectByCompanyAndCopies(
    const CString& strCompanyName,
    int nReportType,
    int nCopies,
    TaechangPriceDto& dto,
    BOOL& bFound,
    CString& strError
) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    dto = TaechangPriceDto();
    bFound = FALSE;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "SELECT "
        "    price_id, "
        "    company_name, "
        "    report_type, "
        "    min_copies, "
        "    max_copies, "
        "    print_price, "
        "    cover_price, "
        "    memo "
        "FROM TaechangPrice "
        "WHERE company_name = ? "
        "  AND report_type = ? "
        "  AND ? >= min_copies "
        "  AND (max_copies IS NULL OR ? <= max_copies) "
        "ORDER BY min_copies DESC "
        "LIMIT 1;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, strCompanyName, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, nReportType, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 3, nCopies, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 4, nCopies, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult == SQLITE_ROW) {
        FillDto(pStatement, dto);
        bFound = TRUE;
    } else if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL TaechangPriceRepository::SelectByPrice(
    int nPrice,
    int nReportType,
    CArray<TaechangPriceDto, TaechangPriceDto&>& arrPrice,
    CString& strError
) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    arrPrice.RemoveAll();

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "SELECT "
        "    price_id, "
        "    company_name, "
        "    report_type, "
        "    min_copies, "
        "    max_copies, "
        "    print_price, "
        "    cover_price, "
        "    memo "
        "FROM TaechangPrice "
        "WHERE report_type = ? "
        "  AND (print_price = ? OR cover_price = ? OR print_price + cover_price = ?) "
        "ORDER BY company_name ASC, min_copies ASC;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindInt(pStatement, 1, nReportType, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, nPrice, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 3, nPrice, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 4, nPrice, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    while ((nResult = sqlite3_step(pStatement)) == SQLITE_ROW) {
        TaechangPriceDto dto;

        FillDto(pStatement, dto);
        arrPrice.Add(dto);
    }

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL TaechangPriceRepository::UpdatePriceByCompany(
    const CString& strCompanyName,
    int nReportType,
    int nPrintPrice,
    int nCoverPrice,
    int& nAffectedCount,
    CString& strError
) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    nAffectedCount = 0;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "UPDATE TaechangPrice SET "
        "    print_price = ?, "
        "    cover_price = ?, "
        "    updated_at = CURRENT_TIMESTAMP "
        "WHERE company_name = ? "
        "  AND report_type = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindInt(pStatement, 1, nPrintPrice, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, nCoverPrice, strError) == FALSE ||
        RepositoryHelper::BindText(pStatement, 3, strCompanyName, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 4, nReportType, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nAffectedCount = sqlite3_changes(pDb);

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL TaechangPriceRepository::UpdatePriceByCompanyAndCopies(
    const CString& strCompanyName,
    int nReportType,
    int nCopies,
    int nPrintPrice,
    int nCoverPrice,
    int& nAffectedCount,
    CString& strError
) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    nAffectedCount = 0;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "UPDATE TaechangPrice SET "
        "    print_price = ?, "
        "    cover_price = ?, "
        "    updated_at = CURRENT_TIMESTAMP "
        "WHERE company_name = ? "
        "  AND report_type = ? "
        "  AND ? >= min_copies "
        "  AND (max_copies IS NULL OR ? <= max_copies);";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindInt(pStatement, 1, nPrintPrice, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, nCoverPrice, strError) == FALSE ||
        RepositoryHelper::BindText(pStatement, 3, strCompanyName, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 4, nReportType, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 5, nCopies, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 6, nCopies, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nAffectedCount = sqlite3_changes(pDb);

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL TaechangPriceRepository::ExistsOverlap(
    const CString& strCompanyName,
    int nReportType,
    int nMinCopies,
    BOOL bHasMaxCopies,
    int nMaxCopies,
    int nExceptPriceId,
    BOOL& bExists,
    CString& strError
) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;
    int nCount;

    bExists = FALSE;
    nCount = 0;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "SELECT COUNT(*) "
        "FROM TaechangPrice "
        "WHERE company_name = ? "
        "  AND report_type = ? "
        "  AND price_id <> ? "
        "  AND min_copies <= COALESCE(?, 999999999) "
        "  AND COALESCE(max_copies, 999999999) >= ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, strCompanyName, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, nReportType, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 3, nExceptPriceId, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    if (bHasMaxCopies == TRUE) {
        if (RepositoryHelper::BindInt(pStatement, 4, nMaxCopies, strError) == FALSE) {
            sqlite3_finalize(pStatement);
            return FALSE;
        }
    } else {
        if (RepositoryHelper::BindNull(pStatement, 4, strError) == FALSE) {
            sqlite3_finalize(pStatement);
            return FALSE;
        }
    }

    if (RepositoryHelper::BindInt(pStatement, 5, nMinCopies, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult == SQLITE_ROW) {
        nCount = sqlite3_column_int(pStatement, 0);
        bExists = nCount > 0;
    } else {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL TaechangPriceRepository::FillDto(sqlite3_stmt* pStatement, TaechangPriceDto& dto) {
    dto.nPriceId = sqlite3_column_int(pStatement, 0);
    dto.strCompanyName = RepositoryHelper::GetColumnText(pStatement, 1);
    dto.nReportType = sqlite3_column_int(pStatement, 2);

    dto.nMinCopies = sqlite3_column_int(pStatement, 3);

    dto.bHasMaxCopies = RepositoryHelper::IsColumnNull(pStatement, 4) == FALSE;
    dto.nMaxCopies = dto.bHasMaxCopies == TRUE ? sqlite3_column_int(pStatement, 4) : 0;

    dto.nPrintPrice = sqlite3_column_int(pStatement, 5);
    dto.nCoverPrice = sqlite3_column_int(pStatement, 6);
    dto.nTotalPrice = dto.nPrintPrice + dto.nCoverPrice;

    dto.strMemo = RepositoryHelper::GetColumnText(pStatement, 7);

    return TRUE;
}