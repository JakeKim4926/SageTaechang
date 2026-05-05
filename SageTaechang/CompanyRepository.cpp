#include "pch.h"
#include "CompanyRepository.h"
#include "RepositoryHelper.h"

CompanyRepository::CompanyRepository(SqlContext* pSqlContext) {
    m_pSqlContext = pSqlContext;
}

CompanyRepository::~CompanyRepository() {}

BOOL CompanyRepository::Insert(const CompanyDto& dto, int& nNewCompanyId, CString& strError) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    nNewCompanyId = 0;

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "INSERT INTO companies "
        "("
        "    company_name, "
        "    business_no, "
        "    memo "
        ") "
        "VALUES "
        "("
        "    ?, ?, ? "
        ");";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, dto.strCompanyName, strError) == FALSE ||
        RepositoryHelper::BindText(pStatement, 2, dto.strBusinessNo, strError) == FALSE ||
        RepositoryHelper::BindText(pStatement, 3, dto.strMemo, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nNewCompanyId = (int)sqlite3_last_insert_rowid(pDb);

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL CompanyRepository::Update(const CompanyDto& dto, CString& strError) {
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
        "UPDATE companies SET "
        "    company_name = ?, "
        "    business_no = ?, "
        "    memo = ?, "
        "    updated_at = CURRENT_TIMESTAMP "
        "WHERE company_id = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, dto.strCompanyName, strError) == FALSE ||
        RepositoryHelper::BindText(pStatement, 2, dto.strBusinessNo, strError) == FALSE ||
        RepositoryHelper::BindText(pStatement, 3, dto.strMemo, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 4, dto.nCompanyId, strError) == FALSE) {
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

BOOL CompanyRepository::Delete(int nCompanyId, CString& strError) {
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

    strSqlA = "DELETE FROM companies WHERE company_id = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindInt(pStatement, 1, nCompanyId, strError) == FALSE) {
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

BOOL CompanyRepository::SelectAll(CArray<CompanyDto, CompanyDto&>& arrCompany, CString& strError) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    arrCompany.RemoveAll();

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "SELECT "
        "    company_id, "
        "    company_name, "
        "    business_no, "
        "    memo "
        "FROM companies "
        "ORDER BY company_name ASC;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    while ((nResult = sqlite3_step(pStatement)) == SQLITE_ROW) {
        CompanyDto dto;

        dto.nCompanyId = sqlite3_column_int(pStatement, 0);
        dto.strCompanyName = RepositoryHelper::GetColumnText(pStatement, 1);
        dto.strBusinessNo = RepositoryHelper::GetColumnText(pStatement, 2);
        dto.strMemo = RepositoryHelper::GetColumnText(pStatement, 3);

        arrCompany.Add(dto);
    }

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL CompanyRepository::SelectById(int nCompanyId, CompanyDto& dto, BOOL& bFound, CString& strError) {
    sqlite3* pDb;
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    bFound = FALSE;
    dto = CompanyDto();

    if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pDb = m_pSqlContext->GetDb();
    pStatement = NULL;

    strSqlA =
        "SELECT "
        "    company_id, "
        "    company_name, "
        "    business_no, "
        "    memo "
        "FROM companies "
        "WHERE company_id = ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindInt(pStatement, 1, nCompanyId, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);

    if (nResult == SQLITE_ROW) {
        dto.nCompanyId = sqlite3_column_int(pStatement, 0);
        dto.strCompanyName = RepositoryHelper::GetColumnText(pStatement, 1);
        dto.strBusinessNo = RepositoryHelper::GetColumnText(pStatement, 2);
        dto.strMemo = RepositoryHelper::GetColumnText(pStatement, 3);

        bFound = TRUE;
    } else if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(pDb);
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    sqlite3_finalize(pStatement);

    return TRUE;
}

BOOL CompanyRepository::ExistsByName(const CString& strCompanyName, int nExceptCompanyId, BOOL& bExists, CString& strError) {
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
        "FROM companies "
        "WHERE company_name = ? "
        "  AND company_id <> ?;";

    nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(pDb);
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, strCompanyName, strError) == FALSE ||
        RepositoryHelper::BindInt(pStatement, 2, nExceptCompanyId, strError) == FALSE) {
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