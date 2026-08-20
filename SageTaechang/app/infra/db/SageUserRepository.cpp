#include "pch.h"
#include "app/infra/db/SageUserRepository.h"
#include "app/infra/db/RepositoryHelper.h"

SageUserRepository::SageUserRepository(SqlContext* pSqlContext) {
    m_pSqlContext = pSqlContext;
}

SageUserRepository::~SageUserRepository() {}

BOOL SageUserRepository::Insert(const SageUserDto& dto, int& nNewUserId, CString& strError) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    nNewUserId = 0;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "INSERT INTO SageUser "
        "(login_id, pw_hash, role) "
        "VALUES (?, ?, ?);";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, dto.strLoginId, strError) == FALSE ||
        RepositoryHelper::BindText(pStatement, 2, dto.strPwHash, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 3, dto.nRole, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nNewUserId = (int)sqlite3_last_insert_rowid(pDb);

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL SageUserRepository::SelectByLoginId(
    const CString& strLoginId,
    SageUserDto& dto,
    BOOL& bFound,
    CString& strError
) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    dto = SageUserDto();
    bFound = FALSE;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "SELECT user_id, login_id, pw_hash, role "
        "FROM SageUser "
        "WHERE login_id = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, strLoginId, strError) == FALSE) {
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

BOOL SageUserRepository::SelectAll(
    CArray<SageUserDto, SageUserDto&>& arrUsers,
    CString& strError
) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    arrUsers.RemoveAll();

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "SELECT user_id, login_id, pw_hash, role "
        "FROM SageUser "
        "ORDER BY user_id ASC;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    while ((nResult = sqlite3_step(pStatement)) == SQLITE_ROW) {
        SageUserDto dto;
        FillDto(pStatement, dto);
        arrUsers.Add(dto);
    }

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL SageUserRepository::Delete(int nUserId, CString& strError) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA = "DELETE FROM SageUser WHERE user_id = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindInt(pStatement, 1, nUserId, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL SageUserRepository::UpdatePassword(int nUserId, const CString& strPwHash, CString& strError) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "UPDATE SageUser SET pw_hash = ? "
        "WHERE user_id = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, strPwHash, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, nUserId, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL SageUserRepository::UpdateRole(int nUserId, int nRole, CString& strError) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "UPDATE SageUser SET role = ? "
        "WHERE user_id = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindInt(pStatement, 1, nRole, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, nUserId, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL SageUserRepository::ExistsByLoginId(const CString& strLoginId, BOOL& bExists, CString& strError) {
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
        "SELECT COUNT(*) FROM SageUser WHERE login_id = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, strLoginId, strError) == FALSE) {
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

BOOL SageUserRepository::FillDto(sqlite3_stmt* pStatement, SageUserDto& dto) {
    dto.nUserId = sqlite3_column_int(pStatement, 0);
    dto.strLoginId = RepositoryHelper::GetColumnText(pStatement, 1);
    dto.strPwHash = RepositoryHelper::GetColumnText(pStatement, 2);
    dto.nRole = sqlite3_column_int(pStatement, 3);

    return TRUE;
}
