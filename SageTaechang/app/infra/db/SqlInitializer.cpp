#include "pch.h"
#include "app/infra/db/SqlInitializer.h"
#include "SageDefine.h"
#include "app/infra/db/SageUserRepository.h"
#include "app/core/auth/SageUserService.h"
#include "app/infra/db/RepositoryHelper.h"

namespace {

constexpr LPCWSTR SAGE_LEGACY_TABLE_PRICE = L"TaechangPrice";
constexpr LPCWSTR SAGE_LEGACY_TABLE_USER = L"TaechangUser";
constexpr LPCWSTR SAGE_LEGACY_TABLE_RECEIVABLE_ORDER = L"TaechangReceivableCompanyOrder";
constexpr LPCWSTR SAGE_TABLE_PRICE = L"SagePrice";
constexpr LPCWSTR SAGE_TABLE_USER = L"SageUser";
constexpr LPCWSTR SAGE_TABLE_RECEIVABLE_ORDER = L"SageReceivableCompanyOrder";
constexpr LPCWSTR SAGE_MIGRATION_RENAME_SQL_FORMAT = L"ALTER TABLE %s RENAME TO %s;";
constexpr LPCWSTR SAGE_MIGRATION_COPY_SQL_FORMAT = L"INSERT INTO %s SELECT * FROM %s;";
constexpr LPCWSTR SAGE_MIGRATION_COUNT_SQL_FORMAT = L"SELECT COUNT(*) FROM %s;";
constexpr int SAGE_MIGRATION_TABLE_COUNT = 3;

}

SqlInitializer::SqlInitializer(SqlContext* pSqlContext) {
    m_pSqlContext = pSqlContext;
}

SqlInitializer::~SqlInitializer() {}

BOOL SqlInitializer::Initialize(CString& strError) {
    CString strRollbackError;

    if (m_pSqlContext == NULL) {
        strError = _T("SqlContext가 NULL입니다.");
        return FALSE;
    }

    if (m_pSqlContext->IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    if (m_pSqlContext->BeginTransaction(strError) == FALSE) {
        return FALSE;
    }

    if (MigrateLegacyTableNames(strError) == FALSE) {
        m_pSqlContext->Rollback(strRollbackError);
        return FALSE;
    }

    if (CreateTables(strError) == FALSE) {
        m_pSqlContext->Rollback(strRollbackError);
        return FALSE;
    }

    if (CreateIndexes(strError) == FALSE) {
        m_pSqlContext->Rollback(strRollbackError);
        return FALSE;
    }

    if (m_pSqlContext->Commit(strError) == FALSE) {
        m_pSqlContext->Rollback(strRollbackError);
        return FALSE;
    }

    return TRUE;
}

BOOL SqlInitializer::HasTable(const CString& strTableName, BOOL& bExists, CString& strError) {
    sqlite3_stmt* pStatement;
    CStringA strSqlA;
    int nResult;

    bExists = FALSE;
    pStatement = NULL;

    strSqlA = "SELECT name FROM sqlite_master WHERE type = 'table' AND name = ?;";

    nResult = sqlite3_prepare_v2(m_pSqlContext->GetDb(), strSqlA.GetString(), -1, &pStatement, NULL);
    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(m_pSqlContext->GetDb());
        return FALSE;
    }

    if (RepositoryHelper::BindText(pStatement, 1, strTableName, strError) == FALSE) {
        sqlite3_finalize(pStatement);
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);
    sqlite3_finalize(pStatement);

    if (nResult == SQLITE_ROW) {
        bExists = TRUE;
        return TRUE;
    }

    if (nResult != SQLITE_DONE) {
        strError = RepositoryHelper::GetLastError(m_pSqlContext->GetDb());
        return FALSE;
    }

    return TRUE;
}

BOOL SqlInitializer::IsTableEmpty(const CString& strTableName, BOOL& bEmpty, CString& strError) {
    sqlite3_stmt* pStatement;
    CString strSql;
    CStringA strSqlA;
    int nResult;

    bEmpty = TRUE;
    pStatement = NULL;

    strSql.Format(SAGE_MIGRATION_COUNT_SQL_FORMAT, strTableName.GetString());
    strSqlA = CStringA(CT2A(strSql, CP_UTF8));

    nResult = sqlite3_prepare_v2(m_pSqlContext->GetDb(), strSqlA.GetString(), -1, &pStatement, NULL);
    if (nResult != SQLITE_OK) {
        strError = RepositoryHelper::GetLastError(m_pSqlContext->GetDb());
        return FALSE;
    }

    nResult = sqlite3_step(pStatement);
    if (nResult == SQLITE_ROW)
        bEmpty = (sqlite3_column_int(pStatement, 0) == 0) ? TRUE : FALSE;

    sqlite3_finalize(pStatement);

    if (nResult != SQLITE_ROW) {
        strError = RepositoryHelper::GetLastError(m_pSqlContext->GetDb());
        return FALSE;
    }

    return TRUE;
}

BOOL SqlInitializer::MigrateLegacyTableNames(CString& strError) {
    LPCWSTR arrLegacyNames[SAGE_MIGRATION_TABLE_COUNT] = {
        SAGE_LEGACY_TABLE_PRICE,
        SAGE_LEGACY_TABLE_USER,
        SAGE_LEGACY_TABLE_RECEIVABLE_ORDER
    };
    LPCWSTR arrCurrentNames[SAGE_MIGRATION_TABLE_COUNT] = {
        SAGE_TABLE_PRICE,
        SAGE_TABLE_USER,
        SAGE_TABLE_RECEIVABLE_ORDER
    };

    for (int nIndex = 0; nIndex < SAGE_MIGRATION_TABLE_COUNT; ++nIndex) {
        CString strLegacy = arrLegacyNames[nIndex];
        CString strCurrent = arrCurrentNames[nIndex];
        CString strSql;
        BOOL bLegacyExists = FALSE;
        BOOL bCurrentExists = FALSE;
        BOOL bCurrentEmpty = FALSE;

        if (HasTable(strLegacy, bLegacyExists, strError) == FALSE)
            return FALSE;

        if (bLegacyExists == FALSE)
            continue;

        if (HasTable(strCurrent, bCurrentExists, strError) == FALSE)
            return FALSE;

        if (bCurrentExists == FALSE) {
            strSql.Format(SAGE_MIGRATION_RENAME_SQL_FORMAT,
                strLegacy.GetString(), strCurrent.GetString());
            if (m_pSqlContext->Execute(strSql, strError) == FALSE)
                return FALSE;
            continue;
        }

        if (IsTableEmpty(strCurrent, bCurrentEmpty, strError) == FALSE)
            return FALSE;

        if (bCurrentEmpty == FALSE)
            continue;

        strSql.Format(SAGE_MIGRATION_COPY_SQL_FORMAT,
            strCurrent.GetString(), strLegacy.GetString());
        if (m_pSqlContext->Execute(strSql, strError) == FALSE)
            return FALSE;
    }

    return TRUE;
}

BOOL SqlInitializer::CreateTables(CString& strError) {
    if (CreateSagePriceTable(strError) == FALSE)
        return FALSE;

    if (CreateSageUserTable(strError) == FALSE)
        return FALSE;

    if (CreateSageReceivableCompanyOrderTable(strError) == FALSE)
        return FALSE;

    if (CreateSagePriceCompanyTable(strError) == FALSE)
        return FALSE;

    if (SeedSagePriceCompanies(strError) == FALSE)
        return FALSE;

    return TRUE;
}

BOOL SqlInitializer::CreateSagePriceTable(CString& strError) {
    CString strSql;

    strSql =
        _T("CREATE TABLE IF NOT EXISTS SagePrice (")
        _T("    price_id INTEGER PRIMARY KEY AUTOINCREMENT,")
        _T("    company_name TEXT NOT NULL,")
        _T("    report_type INTEGER NOT NULL,")
        _T("    min_copies INTEGER NOT NULL,")
        _T("    max_copies INTEGER,")
        _T("    print_price INTEGER NOT NULL,")
        _T("    cover_price INTEGER NOT NULL,")
        _T("    memo TEXT,")
        _T("    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,")
        _T("    updated_at TEXT,")
        _T("    CHECK (report_type > 0),")
        _T("    CHECK (min_copies >= 1),")
        _T("    CHECK (max_copies IS NULL OR max_copies >= min_copies),")
        _T("    CHECK (print_price >= 0),")
        _T("    CHECK (cover_price >= 0)")
        _T(");");

    return m_pSqlContext->Execute(strSql, strError);
}

BOOL SqlInitializer::CreateIndexes(CString& strError) {
    CString strSql;

    strSql =
        _T("CREATE INDEX IF NOT EXISTS idx_SagePrice_company ")
        _T("ON SagePrice(company_name, report_type);");

    if (m_pSqlContext->Execute(strSql, strError) == FALSE) {
        return FALSE;
    }

    strSql =
        _T("CREATE INDEX IF NOT EXISTS idx_SagePrice_copies ")
        _T("ON SagePrice(company_name, report_type, min_copies, max_copies);");

    if (m_pSqlContext->Execute(strSql, strError) == FALSE) {
        return FALSE;
    }

    strSql =
        _T("CREATE INDEX IF NOT EXISTS idx_SagePrice_price ")
        _T("ON SagePrice(print_price, cover_price);");

    if (m_pSqlContext->Execute(strSql, strError) == FALSE) {
        return FALSE;
    }

    strSql =
        _T("CREATE INDEX IF NOT EXISTS idx_SageReceivableCompanyOrder_sort ")
        _T("ON SageReceivableCompanyOrder(sort_order, company_name);");

    if (m_pSqlContext->Execute(strSql, strError) == FALSE) {
        return FALSE;
    }

    return TRUE;
}

BOOL SqlInitializer::CreateSagePriceCompanyTable(CString& strError) {
    CString strSql;

    strSql =
        _T("CREATE TABLE IF NOT EXISTS SagePriceCompany (")
        _T("    company_id INTEGER PRIMARY KEY AUTOINCREMENT,")
        _T("    company_name TEXT NOT NULL,")
        _T("    report_type INTEGER NOT NULL,")
        _T("    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,")
        _T("    CHECK (report_type > 0),")
        _T("    UNIQUE (company_name, report_type)")
        _T(");");

    return m_pSqlContext->Execute(strSql, strError);
}

BOOL SqlInitializer::SeedSagePriceCompanies(CString& strError) {
    CString strSql;

    strSql =
        _T("INSERT OR IGNORE INTO SagePriceCompany (company_name, report_type) ")
        _T("SELECT DISTINCT company_name, report_type FROM SagePrice;");

    return m_pSqlContext->Execute(strSql, strError);
}

BOOL SqlInitializer::CreateSageUserTable(CString& strError) {
    CString strSql;

    strSql =
        _T("CREATE TABLE IF NOT EXISTS SageUser (")
        _T("    user_id   INTEGER PRIMARY KEY AUTOINCREMENT,")
        _T("    login_id  TEXT NOT NULL UNIQUE,")
        _T("    pw_hash   TEXT NOT NULL,")
        _T("    role      INTEGER NOT NULL DEFAULT 0,")
        _T("    CHECK (role >= 0)")
        _T(");");

    if (m_pSqlContext->Execute(strSql, strError) == FALSE)
        return FALSE;

    if (SeedDefaultAdmin(strError) == FALSE)
        return FALSE;

    return TRUE;
}

BOOL SqlInitializer::SeedDefaultAdmin(CString& strError) {
    sqlite3* pDb = m_pSqlContext->GetDb();
    sqlite3_stmt* pStatement = NULL;

    int nResult = sqlite3_prepare_v2(
        pDb,
        "SELECT COUNT(*) FROM SageUser WHERE login_id = ?;",
        -1, &pStatement, NULL
    );

    if (nResult != SQLITE_OK) {
        strError = _T("기본 관리자 계정 확인 실패");
        return FALSE;
    }

    CStringA strAdminIdA(CT2A(SAGE_DEFAULT_ADMIN_ID, CP_UTF8));
    sqlite3_bind_text(pStatement, 1, strAdminIdA.GetString(), -1, SQLITE_STATIC);

    nResult = sqlite3_step(pStatement);
    int nCount = (nResult == SQLITE_ROW) ? sqlite3_column_int(pStatement, 0) : 0;
    sqlite3_finalize(pStatement);

    if (nCount > 0)
        return TRUE;

    SageUserRepository repo(m_pSqlContext);
    SageUserDto adminDto;
    adminDto.strLoginId = SAGE_DEFAULT_ADMIN_ID;
    adminDto.strPwHash = SageUserService::HashPassword(SAGE_DEFAULT_ADMIN_PW);
    adminDto.nRole = USER_ROLE_ADMIN;

    int nNewId = 0;
    return repo.Insert(adminDto, nNewId, strError);
}

BOOL SqlInitializer::CreateSageReceivableCompanyOrderTable(CString& strError) {
    CString strSql;

    strSql =
        _T("CREATE TABLE IF NOT EXISTS SageReceivableCompanyOrder (")
        _T("    order_id INTEGER PRIMARY KEY AUTOINCREMENT,")
        _T("    company_name TEXT NOT NULL UNIQUE,")
        _T("    sort_order INTEGER NOT NULL,")
        _T("    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,")
        _T("    updated_at TEXT,")
        _T("    CHECK (sort_order >= 0)")
        _T(");");

    return m_pSqlContext->Execute(strSql, strError);
}
