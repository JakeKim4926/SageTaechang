#include "pch.h"
#include "app/infra/db/SqlInitializer.h"
#include "TaechangDefine.h"
#include "app/infra/db/TaechangUserRepository.h"
#include "app/core/auth/TaechangUserService.h"

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

BOOL SqlInitializer::CreateTables(CString& strError) {
    if (CreateTaechangPriceTable(strError) == FALSE)
        return FALSE;

    if (CreateTaechangUserTable(strError) == FALSE)
        return FALSE;

    if (CreateTaechangReceivableCompanyOrderTable(strError) == FALSE)
        return FALSE;

    if (CreateTaechangPriceCompanyTable(strError) == FALSE)
        return FALSE;

    if (SeedTaechangPriceCompanies(strError) == FALSE)
        return FALSE;

    return TRUE;
}

BOOL SqlInitializer::CreateTaechangPriceTable(CString& strError) {
    CString strSql;

    strSql =
        _T("CREATE TABLE IF NOT EXISTS TaechangPrice (")
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
        _T("CREATE INDEX IF NOT EXISTS idx_TaechangPrice_company ")
        _T("ON TaechangPrice(company_name, report_type);");

    if (m_pSqlContext->Execute(strSql, strError) == FALSE) {
        return FALSE;
    }

    strSql =
        _T("CREATE INDEX IF NOT EXISTS idx_TaechangPrice_copies ")
        _T("ON TaechangPrice(company_name, report_type, min_copies, max_copies);");

    if (m_pSqlContext->Execute(strSql, strError) == FALSE) {
        return FALSE;
    }

    strSql =
        _T("CREATE INDEX IF NOT EXISTS idx_TaechangPrice_price ")
        _T("ON TaechangPrice(print_price, cover_price);");

    if (m_pSqlContext->Execute(strSql, strError) == FALSE) {
        return FALSE;
    }

    strSql =
        _T("CREATE INDEX IF NOT EXISTS idx_TaechangReceivableCompanyOrder_sort ")
        _T("ON TaechangReceivableCompanyOrder(sort_order, company_name);");

    if (m_pSqlContext->Execute(strSql, strError) == FALSE) {
        return FALSE;
    }

    return TRUE;
}

BOOL SqlInitializer::CreateTaechangPriceCompanyTable(CString& strError) {
    CString strSql;

    strSql =
        _T("CREATE TABLE IF NOT EXISTS TaechangPriceCompany (")
        _T("    company_id INTEGER PRIMARY KEY AUTOINCREMENT,")
        _T("    company_name TEXT NOT NULL,")
        _T("    report_type INTEGER NOT NULL,")
        _T("    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,")
        _T("    CHECK (report_type > 0),")
        _T("    UNIQUE (company_name, report_type)")
        _T(");");

    return m_pSqlContext->Execute(strSql, strError);
}

BOOL SqlInitializer::SeedTaechangPriceCompanies(CString& strError) {
    CString strSql;

    strSql =
        _T("INSERT OR IGNORE INTO TaechangPriceCompany (company_name, report_type) ")
        _T("SELECT DISTINCT company_name, report_type FROM TaechangPrice;");

    return m_pSqlContext->Execute(strSql, strError);
}

BOOL SqlInitializer::CreateTaechangUserTable(CString& strError) {
    CString strSql;

    strSql =
        _T("CREATE TABLE IF NOT EXISTS TaechangUser (")
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
        "SELECT COUNT(*) FROM TaechangUser WHERE login_id = ?;",
        -1, &pStatement, NULL
    );

    if (nResult != SQLITE_OK) {
        strError = _T("기본 관리자 계정 확인 실패");
        return FALSE;
    }

    CStringA strAdminIdA(CT2A(TAECHANG_DEFAULT_ADMIN_ID, CP_UTF8));
    sqlite3_bind_text(pStatement, 1, strAdminIdA.GetString(), -1, SQLITE_STATIC);

    nResult = sqlite3_step(pStatement);
    int nCount = (nResult == SQLITE_ROW) ? sqlite3_column_int(pStatement, 0) : 0;
    sqlite3_finalize(pStatement);

    if (nCount > 0)
        return TRUE;

    TaechangUserRepository repo(m_pSqlContext);
    TaechangUserDto adminDto;
    adminDto.strLoginId = TAECHANG_DEFAULT_ADMIN_ID;
    adminDto.strPwHash = TaechangUserService::HashPassword(TAECHANG_DEFAULT_ADMIN_PW);
    adminDto.nRole = USER_ROLE_ADMIN;

    int nNewId = 0;
    return repo.Insert(adminDto, nNewId, strError);
}

BOOL SqlInitializer::CreateTaechangReceivableCompanyOrderTable(CString& strError) {
    CString strSql;

    strSql =
        _T("CREATE TABLE IF NOT EXISTS TaechangReceivableCompanyOrder (")
        _T("    order_id INTEGER PRIMARY KEY AUTOINCREMENT,")
        _T("    company_name TEXT NOT NULL UNIQUE,")
        _T("    sort_order INTEGER NOT NULL,")
        _T("    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,")
        _T("    updated_at TEXT,")
        _T("    CHECK (sort_order >= 0)")
        _T(");");

    return m_pSqlContext->Execute(strSql, strError);
}
