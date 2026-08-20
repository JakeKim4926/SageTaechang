#include "pch.h"
#include "app/infra/db/SageReceivableCompanyOrderRepository.h"
#include "app/infra/db/RepositoryHelper.h"

SageReceivableCompanyOrderRepository::SageReceivableCompanyOrderRepository(SqlContext* pSqlContext) {
    m_pSqlContext = pSqlContext;
}

SageReceivableCompanyOrderRepository::~SageReceivableCompanyOrderRepository() {}

BOOL SageReceivableCompanyOrderRepository::Insert(const SageReceivableCompanyOrderDto& dto, int& nNewOrderId, CString& strError) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    nNewOrderId = 0;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "INSERT INTO SageReceivableCompanyOrder "
        "("
        "    company_name, "
        "    sort_order "
        ") "
        "VALUES "
        "("
        "    ?, ? "
        ");";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, dto.strCompanyName, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, dto.nSortOrder, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nNewOrderId = (int)sqlite3_last_insert_rowid(pDb);

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL SageReceivableCompanyOrderRepository::SelectAll(
    CArray<SageReceivableCompanyOrderDto, SageReceivableCompanyOrderDto&>& arrOrder,
    CString& strError
) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    arrOrder.RemoveAll();

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "SELECT order_id, company_name, sort_order "
        "FROM SageReceivableCompanyOrder "
        "ORDER BY sort_order ASC, company_name ASC;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    while ((nResult = sqlite3_step(pStatement)) == SQLITE_ROW) {
        SageReceivableCompanyOrderDto dto;
        FillDto(pStatement, dto);
        arrOrder.Add(dto);
    }

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL SageReceivableCompanyOrderRepository::SelectByCompanyName(
    const CString& strCompanyName,
    SageReceivableCompanyOrderDto& dto,
    BOOL& bFound,
    CString& strError
) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    dto = SageReceivableCompanyOrderDto();
    bFound = FALSE;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "SELECT order_id, company_name, sort_order "
        "FROM SageReceivableCompanyOrder "
        "WHERE company_name = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, strCompanyName, strError) == FALSE) {
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

BOOL SageReceivableCompanyOrderRepository::Update(
    const SageReceivableCompanyOrderDto& dto,
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
        "UPDATE SageReceivableCompanyOrder SET "
        "    company_name = ?, "
        "    sort_order = ?, "
        "    updated_at = CURRENT_TIMESTAMP "
        "WHERE order_id = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, dto.strCompanyName, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, dto.nSortOrder, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 3, dto.nOrderId, strError) == FALSE) {
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

BOOL SageReceivableCompanyOrderRepository::SwapSortOrder(
    const SageReceivableCompanyOrderDto& dtoFirst,
    const SageReceivableCompanyOrderDto& dtoSecond,
    CString& strError
) {
    SageReceivableCompanyOrderDto dtoLower;
    SageReceivableCompanyOrderDto dtoUpper;
    CString strRollbackError;
    int nAffectedCount;
    int nSortOrder;

    nAffectedCount = 0;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    dtoLower = dtoFirst;
    dtoUpper = dtoSecond;
    nSortOrder = dtoLower.nSortOrder;
    dtoLower.nSortOrder = dtoUpper.nSortOrder;
    dtoUpper.nSortOrder = nSortOrder;

    if (m_pSqlContext->BeginTransaction(strError) == FALSE) {
        return FALSE;
    }

    if (Update(dtoLower, nAffectedCount, strError) == FALSE ||
        Update(dtoUpper, nAffectedCount, strError) == FALSE) {
        m_pSqlContext->Rollback(strRollbackError);
        return FALSE;
    }

    return m_pSqlContext->Commit(strError);
}

BOOL SageReceivableCompanyOrderRepository::DeleteByOrderId(int nOrderId, int& nAffectedCount, CString& strError) {
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
        "DELETE FROM SageReceivableCompanyOrder "
        "WHERE order_id = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindInt(pStatement, 1, nOrderId, strError) == FALSE) {
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

BOOL SageReceivableCompanyOrderRepository::DeleteByCompanyName(
    const CString& strCompanyName,
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
        "DELETE FROM SageReceivableCompanyOrder "
        "WHERE company_name = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, strCompanyName, strError) == FALSE) {
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

BOOL SageReceivableCompanyOrderRepository::ExistsByCompanyName(
    const CString& strCompanyName,
    int nExceptOrderId,
    BOOL& bExists,
    CString& strError
) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    bExists = FALSE;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "SELECT COUNT(*) "
        "FROM SageReceivableCompanyOrder "
        "WHERE company_name = ? "
        "  AND order_id <> ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, strCompanyName, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, nExceptOrderId, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult == SQLITE_ROW) {
        bExists = sqlite3_column_int(pStatement, 0) > 0;
    } else {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL SageReceivableCompanyOrderRepository::ExistsBySortOrder(
    int nSortOrder,
    int nExceptOrderId,
    BOOL& bExists,
    CString& strError
) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    bExists = FALSE;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB媛 ?대젮 ?덉? ?딆뒿?덈떎.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "SELECT COUNT(*) "
        "FROM SageReceivableCompanyOrder "
        "WHERE sort_order = ? "
        "  AND order_id <> ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindInt(pStatement, 1, nSortOrder, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, nExceptOrderId, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult == SQLITE_ROW) {
        bExists = sqlite3_column_int(pStatement, 0) > 0;
    } else {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL SageReceivableCompanyOrderRepository::FillDto(sqlite3_stmt* pStatement, SageReceivableCompanyOrderDto& dto) {
    dto.nOrderId = sqlite3_column_int(pStatement, 0);
    dto.strCompanyName = RepositoryHelper::GetColumnText(pStatement, 1);
    dto.nSortOrder = sqlite3_column_int(pStatement, 2);

    return TRUE;
}
